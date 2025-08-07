#include "mainwindow.h"
#include "streamlistwidget.h"
#include "videoplayerwidget.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include "msgClient.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupStreamData();
    applyStyles();
    
    // 设置窗口大小和位置
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // 居中显示窗口
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - WINDOW_WIDTH) / 2;
    int y = (screenGeometry.height() - WINDOW_HEIGHT) / 2;
    move(x, y);
    
    // 设置窗口标题
    setWindowTitle("StreamHive - RTSP流监控系统");
    
    // 连接信号槽
    connect(m_streamListWidget, &StreamListWidget::streamSelected,
            this, &MainWindow::onStreamSelected);
    connect(m_videoPlayerWidget, &VideoPlayerWidget::backToMain,
            this, &MainWindow::onBackToMain);
    
    // 创建并启动ZMQ客户端
    msgClient *client = new msgClient("192.168.10.107");
    connect(client, &msgClient::msgReceived,
            this, &MainWindow::onMsgReceived);
    connect(client, &msgClient::rtspUrlReceived,
            this, &MainWindow::onRtspUrlReceived);
    connect(client, &msgClient::errorOccurred,
            this, &MainWindow::onZmqError);
    
    // 启动客户端
    client->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建堆叠窗口部件
    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);
    
    // 创建流列表组件
    m_streamListWidget = new StreamListWidget(this);
    m_stackedWidget->addWidget(m_streamListWidget);
    
    // 创建视频播放器组件
    m_videoPlayerWidget = new VideoPlayerWidget(this);
    m_stackedWidget->addWidget(m_videoPlayerWidget);
    
    // 默认显示流列表
    m_stackedWidget->setCurrentWidget(m_streamListWidget);
}

void MainWindow::setupStreamData()
{
    // 这里可以添加实际的RTSP流数据
    // 目前使用示例数据
}

void MainWindow::applyStyles()
{
    // 设置窗口样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #ffffff;
        }
    )");
}

void MainWindow::onStreamSelected(const QString &streamName, const QString &streamUrl)
{
    // 切换到视频播放界面
    m_videoPlayerWidget->playStream(streamName, streamUrl);
    m_stackedWidget->setCurrentWidget(m_videoPlayerWidget);
}

void MainWindow::onBackToMain()
{
    // 停止当前视频播放
    m_videoPlayerWidget->stopStream();
    
    // 返回主界面
    m_stackedWidget->setCurrentWidget(m_streamListWidget);
}

void MainWindow::onMsgReceived(const QString &msg)
{
    qDebug() << "Received alarm message:" << msg;
    // 将报警消息添加到视频播放器的报警信息框
    m_videoPlayerWidget->addAlarmMessage(msg);
}

void MainWindow::onRtspUrlReceived(const QString &msg)
{
    qDebug() << "Received RTSP URL:" << msg;
    // 将接收到的RTSP URL添加到流列表中
    m_streamListWidget->addRtspStream(msg);
}

void MainWindow::onZmqError(const QString &error_msg)
{
    qDebug() << "ZMQ Error:" << error_msg;
    // 处理ZMQ错误
}