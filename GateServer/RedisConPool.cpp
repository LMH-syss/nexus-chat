#include "RedisConPool.h"

#include <iostream>

RedisConPool::RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
    : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
    for (size_t i = 0; i < poolSize_; ++i) {
        auto* context = redisConnect(host, port);
        if (context == nullptr || context->err != 0) {
            if (context != nullptr) {
                redisFree(context);
            }
            continue;
        }

        auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
        if (reply == nullptr) {
            redisFree(context);
            continue;
        }

        if (reply->type == REDIS_REPLY_ERROR) {
            std::cout << "认证失败" << std::endl;
            freeReplyObject(reply);
            redisFree(context);
            continue;
        }

        freeReplyObject(reply);
        //std::cout << "认证成功" << std::endl;
        connections_.push(context);
    }
}

RedisConPool::~RedisConPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        auto* context = connections_.front();
        connections_.pop();
        if (context != nullptr) {
            redisFree(context);
        }
    }
}

redisContext* RedisConPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        if (b_stop_) {
            return true;
        }
        return !connections_.empty();
        });

    if (b_stop_) {
        return nullptr;
    }

    auto* context = connections_.front();
    connections_.pop();
    return context;
}

void RedisConPool::returnConnection(redisContext* context) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
        if (context != nullptr) {
            redisFree(context);
        }
        return;
    }

    connections_.push(context);
    cond_.notify_one();
}

void RedisConPool::Close() {
    b_stop_ = true;
    cond_.notify_all();
}