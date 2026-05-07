# NexusChat

NexusChat 是一个基于 C++ / Qt 的分布式即时通信系统学习与实践项目。项目采用多服务协作架构，包含 Qt 桌面客户端、GateServer 网关服务、StatusServer 状态服务、ChatServer 聊天服务、VarifyServer 邮箱验证码服务，以及 MySQL、Redis 等基础组件。

本项目主要用于学习和实践 C++ 网络编程、Qt 客户端开发、Boost.Asio 异步通信、gRPC 服务调用、Redis 缓存、MySQL 数据持久化和即时通信系统中的注册、登录、验证码、状态分配、好友关系与消息处理流程。

## 技术栈

- C++17
- Qt6
- Boost.Asio
- gRPC / Protocol Buffers
- MySQL
- Redis
- Node.js

## 项目结构

```text
client/         Qt 桌面客户端
GateServer/     HTTP 网关服务，处理注册、登录、验证码等入口请求
StatusServer/   状态服务，用于分配或查询聊天服务节点
ChatServer/     TCP 聊天服务，处理客户端长连接和聊天业务
ChatServer2/    第二个聊天服务实例，用于模拟多节点部署
VarifyServer/   Node.js 邮箱验证码服务，目录名沿用项目原拼写
sql/            MySQL 数据库脚本
```

## 配置说明

各服务需要本地配置 MySQL、Redis、服务端口和邮箱授权信息。首次运行前，请在本地按各服务代码读取的字段创建配置文件，并使用自己的数据库、Redis 和邮箱授权信息。

各服务目录中的 `config.example.ini` 和 `VarifyServer/config.example.json` 提供了配置字段示例。运行前可参考示例文件创建本地 `config.ini` 或 `config.json`，并填入自己的服务地址、邮箱授权码、MySQL 和 Redis 配置。

## 数据库

`sql/schema.sql` 用于创建项目所需表结构。
