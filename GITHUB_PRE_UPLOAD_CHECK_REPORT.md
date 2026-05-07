# GitHub 上传前检查报告

检查日期：2026-05-07

## 结论

当前 NexusChat 项目适合作为 C++ / Qt 分布式即时通信系统学习项目上传到 GitHub。已对发现的本地敏感配置进行脱敏处理，但上传前仍需删除或排除构建产物。当前目录还不是 Git 仓库，建议先确认 `.gitignore` 生效，再执行首次 `git add`。

## 应保留的内容

- 源码文件：`*.cpp`、`*.h`、`*.js`
- Qt 客户端文件：`*.ui`、`*.qrc`、`*.qss`、图片资源、`client/chat.pro`
- Visual Studio 工程文件：`*.sln`、`*.vcxproj`、`*.vcxproj.filters`
- 协议文件：`*.proto`
- Node.js 依赖声明：`VarifyServer/package.json`、`VarifyServer/package-lock.json`
- 示例配置：`config.example.ini`、`VarifyServer/config.example.json`
- 数据库结构脚本：`sql/schema.sql`
- 项目说明文件：`README.md`

## 不应上传的构建产物

已发现以下类型的构建产物或本地缓存，应删除或确保不被 Git 追踪：

- Visual Studio 缓存和输出：`.vs/`、`x64/`、`Debug/`
- Qt 构建输出：`client/build/`、`client/debug/`、`client/release/`、`client/bin/`
- Node.js 依赖目录：`VarifyServer/node_modules/`
- 二进制和调试文件：`*.exe`、`*.dll`、`*.pdb`、`*.obj`、`*.ilk`、`*.lib`、`*.exp`
- 自动生成或本地文件：`ui_*.h`、`Makefile`、`Makefile.Debug`、`Makefile.Release`、`.qmake.stash`、`.qtcreator/`
- 备份文件：`*.bak`

重点路径：

- `ChatServer/.vs/`
- `ChatServer/x64/`
- `ChatServer2/.vs/`
- `ChatServer2/x64/`
- `GateServer/.vs/`
- `GateServer/x64/`
- `StatusServer/.vs/`
- `StatusServer/Debug/`
- `StatusServer/x64/`
- `client/build/`
- `client/debug/`
- `client/release/`
- `client/bin/`
- `VarifyServer/node_modules/`

另外，服务根目录下存在 MySQL Connector DLL，例如：

- `ChatServer/mysqlcppconn-9-vs14.dll`
- `ChatServer/mysqlcppconn8-2-vs14.dll`
- `ChatServer2/mysqlcppconn-9-vs14.dll`
- `ChatServer2/mysqlcppconn8-2-vs14.dll`
- `GateServer/mysqlcppconn-9-vs14.dll`
- `GateServer/mysqlcppconn8-2-vs14.dll`
- `StatusServer/mysqlcppconn-9-vs14.dll`
- `StatusServer/mysqlcppconn8-2-vs14.dll`

这些 DLL 不建议上传，应通过依赖安装说明、vcpkg、系统库或发布包方式处理。

## 敏感配置处理

未输出任何完整密码或授权码。以下文件原本包含或可能包含本地连接信息、密码、邮箱授权码或服务地址，已替换为占位符或本地默认地址：

- `ChatServer/config.ini`
- `ChatServer2/config.ini`
- `GateServer/config.ini`
- `StatusServer/config.ini`
- `client/config.ini`
- `VarifyServer/config.json`
- `VarifyServer/server.js`

同时已新增示例配置文件：

- `ChatServer/config.example.ini`
- `ChatServer2/config.example.ini`
- `GateServer/config.example.ini`
- `StatusServer/config.example.ini`
- `client/config.example.ini`
- `VarifyServer/config.example.json`

`VarifyServer/email.js` 和 `VarifyServer/redis.js` 只是读取配置字段，本身可保留。`VarifyServer/server.js` 原本硬编码发件邮箱，已改为从配置模块读取。

重要提醒：如果旧邮箱授权码曾经用于真实邮箱，建议到邮箱服务商后台撤销或重置授权码。即使本地文件已脱敏，已经暴露过的授权码也不应继续使用。

## SQL 数据检查

`sql/schema.sql` 主要是建表结构，适合保留。文件中包含 `AUTO_INCREMENT` 值，可能反映开发库历史行数，但未发现真实用户记录插入语句。

`sql/chat.sql` 中的 `INSERT INTO user` 位于 `reg_user` 存储过程内部，是注册逻辑的一部分，不是实际用户数据导出。

未发现聊天记录类 `INSERT INTO chat_message` 数据。

## README 检查

已更新 `README.md`，现在能更准确说明：

- 项目定位：C++ / Qt 分布式即时通信学习与实践项目
- 模块职责：client、GateServer、StatusServer、ChatServer、ChatServer2、VarifyServer、sql
- 技术栈：Qt、Boost.Asio、gRPC、MySQL、Redis、Node.js
- 上传前应保留和忽略的文件类型
- 配置文件和 SQL 数据的安全注意事项

## .gitignore 检查

已完善 `.gitignore`，覆盖：

- VS / CMake / Qt / qmake 构建产物
- Qt Creator、VS Code、JetBrains 等本地 IDE 文件
- Node.js `node_modules`
- `.env`、`config.ini`、`config.json` 等本地配置
- `config.example.ini`、`config.example.json` 示例配置会被保留
- 日志、临时文件、备份文件、数据库本地文件
- Windows / macOS 系统文件

## 上传前建议操作

1. 初始化 Git 前，确认 `.gitignore` 已保存。
2. 删除或不要加入构建产物目录：`.vs/`、`x64/`、`Debug/`、`Release/`、`build/`、`bin/`、`obj/`、`node_modules/`。
3. 不提交真实配置文件：`config.ini`、`config.json`、`.env`；只提交示例配置。
4. 如果旧邮箱授权码真实可用，立即撤销或重置。
5. 首次提交前运行 `git status --ignored`，确认忽略规则符合预期。
