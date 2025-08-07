# StreamHive_QT

<div align="center">

![Qt](https://img.shields.io/badge/Qt-5.12.9-green.svg)
![FFmpeg](https://img.shields.io/badge/FFmpeg-4.4+-blue.svg)
![ZeroMQ](https://img.shields.io/badge/ZeroMQ-4.3.4-orange.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

**基于FFmpeg的多路RTSP视频流播放器**

*专为StreamHive项目设计的Qt桌面客户端*

</div>

---

## 📖 项目简介

StreamHive_QT是一个基于Qt框架开发的多路RTSP视频流播放器，专门为StreamHive项目设计。该播放器支持实时接收和播放多路RTSP视频流，具备现代化的用户界面和强大的视频处理能力。

### 🎯 主要特性

- **🔴 多路RTSP流播放**: 支持同时接收和播放多路RTSP视频流
- **🎨 现代化界面**: 简洁美观的双层界面设计，适配7寸1024×600分辨率
- **⚡ 高性能播放**: 基于FFmpeg的高效视频解码和渲染
- **📡 实时通信**: 集成ZeroMQ实现实时消息接收和报警信息显示
- **🔄 动态更新**: 支持实时添加和移除视频流

## 🏗️ 技术架构

### 核心技术栈

- **Qt 5.12.9**: 跨平台GUI框架
- **FFmpeg 4.4+**: 视频解码和处理
- **ZeroMQ 4.3.4**: 高性能消息队列
- **C++**: 主要开发语言

### 系统架构

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   MainWindow    │    │ StreamListWidget│    │VideoPlayerWidget│
│   (主窗口管理)   │    │  (流列表界面)   │    │  (视频播放界面) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   msgClient     │    │  StreamPlayer   │    │   Alarm Display │
│  (ZeroMQ客户端) │    │ (FFmpeg播放器)  │    │   (报警显示)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 快速开始

### 环境要求

- **操作系统**: Windows 10/11, Linux, macOS
- **Qt版本**: 5.12.9 或更高版本
- **编译器**: MinGW 7.3.0 (Windows) 或 GCC 7+
- **FFmpeg**: 4.4+ 版本
- **ZeroMQ**: 4.3.4 版本

### 安装步骤

1. **克隆项目**
   ```bash
   git clone https://github.com/your-username/StreamHive_QT.git
   cd StreamHive_QT
   ```

2. **配置依赖库**
   - 将FFmpeg库文件放置在 `ffmpeg/` 目录下
   - 将ZeroMQ库文件放置在 `zmq/` 目录下

3. **编译项目**
   ```bash
   qmake StreamHive_QT.pro
   make
   ```

4. **运行程序**
   ```bash
   ./StreamHive_QT
   ```

## 📱 界面说明

### 主界面 (Level 1)
- **垂直列表**: 显示所有可用的RTSP视频流
- **刷新按钮**: 清空当前列表
- **点击跳转**: 点击列表项进入播放界面

### 播放界面 (Level 2)
- **视频播放区**: 大尺寸视频显示区域
- **报警信息框**: 右侧实时显示报警信息
- **返回按钮**: 顶部返回主界面

## 🔧 配置说明

### 项目文件配置

```pro
# StreamHive_QT.pro
QT += core gui widgets multimedia multimediawidgets network

# FFmpeg配置
INCLUDEPATH += $$PWD/ffmpeg/include
LIBS += -L$$PWD/ffmpeg/lib/ -lavcodec -lavformat -lavutil -lswscale

# ZeroMQ配置
INCLUDEPATH += $$PWD/zmq/include
LIBS += -L$$PWD/zmq/lib -llibzmq-v140-mt-4_3_4
```

### 网络配置

- **RTSP流端口**: 5555
- **报警消息端口**: 5556
- **服务器IP**: 192.168.10.107 (可配置)

## 📊 功能特性详解

### 1. 多路流管理
- 支持JSON格式的流信息解析
- 自动去重机制，防止重复添加
- 动态添加和移除视频流

### 2. 视频播放
- 基于FFmpeg的高效解码
- 支持多种视频格式
- 实时帧率控制
- 错误处理和重连机制

### 3. 消息通信
- ZeroMQ订阅模式
- 实时报警信息接收
- 线程安全的消息处理
- 自动重连机制

### 4. 用户界面
- 响应式设计
- 现代化样式
- 流畅的动画效果
- 直观的操作体验

## 🐛 故障排除

### 常见问题

1. **编译错误**: `zmq.h: No such file or directory`
   - 确保ZeroMQ库文件正确放置在 `zmq/` 目录下

2. **链接错误**: `cannot find -lzmq`
   - 检查库文件路径和版本兼容性

3. **播放问题**: 视频无法播放
   - 检查RTSP URL的有效性
   - 确认网络连接正常

4. **消息接收问题**: 无法接收报警信息
   - 检查ZeroMQ服务器连接
   - 确认端口配置正确

## 🤝 贡献指南

我们欢迎社区贡献！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request


## 📞 联系方式

- **邮箱**: 1239503460@qq.com

---

