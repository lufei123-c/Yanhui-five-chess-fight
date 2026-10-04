# Qt 网络五子棋

一个使用 Qt/C++ 编写的双人网络五子棋项目。项目采用客户端—服务器结构，服务器负责连接管理、回合控制和胜负判断，客户端负责棋盘显示与玩家交互。

## 功能

- 两名玩家通过 TCP 连接进行对战
- 自动分配黑棋和白棋，黑棋先行
- 服务端校验落子、维护回合并判断胜负
- 客户端显示棋盘、连接状态和对局结果
- 默认监听端口：`8888`

## 项目结构

```text
.
├── Client/             # Qt Widgets 客户端
├── Server/             # 命令行服务器
├── libs/
│   ├── GameEngine/     # 棋盘状态及胜负判断
│   └── Protocol/       # 客户端与服务器通信协议
└── Yanhui_five_chess_fight.pro
```

## 开发环境

- Qt 6
- qmake
- 支持 C++17 的编译器（例如 MSVC 2022）

## 编译

使用 Qt Creator 打开根目录的 `Yanhui_five_chess_fight.pro`，配置合适的 Qt Kit 后构建整个项目。

也可以在已经配置好 Qt 环境的命令行中执行：

```bash
qmake Yanhui_five_chess_fight.pro
make
```

在 Windows 的 MSVC Kit 中，构建命令通常为 `nmake` 或由 Qt Creator 自动执行。

## 运行

1. 先启动 `Server`，服务器会监听 `8888` 端口。
2. 启动两个 `Client` 实例。
3. 在客户端中输入服务器 IP（本机测试使用 `127.0.0.1`）和端口 `8888`。
4. 两名玩家都连接后即可开始对局。
