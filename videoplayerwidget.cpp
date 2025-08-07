#include "videoplayerwidget.h"
#include "streamPlayer.h"
#include <QApplication>
#include <QFont>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug> // Added for qDebug

VideoPlayerWidget::VideoPlayerWidget(QWidget *parent)
    : QWidget(parent)
    , m_streamPlayer(nullptr)
{
    setupUI();
    applyStyles();
    
//    // 初始化报警定时器
//    m_alarmTimer = new QTimer(this);
//    connect(m_alarmTimer, &QTimer::timeout, this, &VideoPlayerWidget::updateAlarmInfo);
//    m_alarmTimer->start(5000); // 每5秒更新一次报警信息
    
    // 添加初始报警信息
    addAlarmMessage("系统启动完成");
    addAlarmMessage("FFmpeg播放器初始化完成");
}

VideoPlayerWidget::~VideoPlayerWidget()
{
    // 确保在析构时停止播放
    if (m_streamPlayer) {
        disconnect(m_streamPlayer, &StreamPlayer::frameReady, this, &VideoPlayerWidget::onFrameReady);
        disconnect(m_streamPlayer, &StreamPlayer::errorSignal, this, &VideoPlayerWidget::onStreamError);
        
        m_streamPlayer->stop();
        if (!m_streamPlayer->wait(1000)) {
            m_streamPlayer->terminate();
            m_streamPlayer->wait();
        }
        m_streamPlayer->deleteLater();
    }
}

void VideoPlayerWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 顶部区域
    QWidget *topWidget = new QWidget(this);
    m_topLayout = new QHBoxLayout(topWidget);
    m_topLayout->setContentsMargins(20, 10, 20, 10);
    m_topLayout->setSpacing(20);
    
    // 返回按钮
    m_backButton = new QPushButton("← 返回", this);
    m_backButton->setFixedSize(100, 40);
    connect(m_backButton, &QPushButton::clicked, this, &VideoPlayerWidget::onBackButtonClicked);
    
    // 流标题标签
    m_streamTitleLabel = new QLabel("视频播放", this);
    m_streamTitleLabel->setAlignment(Qt::AlignCenter);
    
    m_topLayout->addWidget(m_backButton);
    m_topLayout->addWidget(m_streamTitleLabel);
    m_topLayout->addStretch();
    
    m_mainLayout->addWidget(topWidget);
    
    // 内容区域
    QWidget *contentWidget = new QWidget(this);
    m_contentLayout = new QHBoxLayout(contentWidget);
    m_contentLayout->setContentsMargins(20, 10, 20, 20);
    m_contentLayout->setSpacing(20);
    
    // 视频播放区域
    QWidget *videoContainer = new QWidget(this);
    m_videoLayout = new QVBoxLayout(videoContainer);
    m_videoLayout->setContentsMargins(0, 0, 0, 0);
    m_videoLayout->setSpacing(10);
    
    // 使用QLabel替换QVideoWidget来显示QImage
    m_videoDisplayLabel = new QLabel(this);
    m_videoDisplayLabel->setFixedSize(VIDEO_WIDTH, 450);
    m_videoDisplayLabel->setAlignment(Qt::AlignCenter);
    m_videoDisplayLabel->setText("等待视频流...");
    m_videoDisplayLabel->setStyleSheet("QLabel { background-color: #000000; color: #ffffff; }");
    
    m_videoLayout->addWidget(m_videoDisplayLabel);
    
    m_contentLayout->addWidget(videoContainer);
    
    // 右侧报警信息区域
    QWidget *alarmContainer = new QWidget(this);
    m_alarmLayout = new QVBoxLayout(alarmContainer);
    m_alarmLayout->setContentsMargins(0, 0, 0, 0);
    m_alarmLayout->setSpacing(10);
    
    // 报警标题
    m_alarmTitleLabel = new QLabel("报警信息", this);
    m_alarmTitleLabel->setAlignment(Qt::AlignCenter);
    
    // 报警信息文本框
    m_alarmTextEdit = new QTextEdit(this);
    m_alarmTextEdit->setFixedSize(ALARM_WIDTH, 400);
    m_alarmTextEdit->setReadOnly(true);
    m_alarmTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_alarmLayout->addWidget(m_alarmTitleLabel);
    m_alarmLayout->addWidget(m_alarmTextEdit);
    m_alarmLayout->addStretch();
    
    m_contentLayout->addWidget(alarmContainer);
    
    m_mainLayout->addWidget(contentWidget);
}

void VideoPlayerWidget::applyStyles()
{
    // 设置整体样式
    setStyleSheet(R"(
        QWidget {
            background-color: #1e1e1e;
            color: #ffffff;
        }
    )");
    
    // 设置顶部区域样式
    m_backButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3d3d3d;
            border: 1px solid #4d4d4d;
            border-radius: 6px;
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #4d4d4d;
            border: 1px solid #5d5d5d;
        }
        QPushButton:pressed {
            background-color: #2d2d2d;
        }
    )");
    
    m_streamTitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18px;
            font-weight: bold;
            color: #ffffff;
        }
    )");
    
    // 设置视频显示区域样式
    m_videoDisplayLabel->setStyleSheet(R"(
        QLabel {
            background-color: #000000;
            border: 2px solid #3d3d3d;
            border-radius: 8px;
            color: #ffffff;
            font-size: 16px;
        }
    )");
    
    // 设置报警信息区域样式
    m_alarmTitleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 16px;
            font-weight: bold;
            color: #ffffff;
            padding: 8px;
            background-color: #2d2d2d;
            border-radius: 6px;
        }
    )");
    
    m_alarmTextEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 6px;
            color: #ffffff;
            font-size: 12px;
            padding: 8px;
        }
        QScrollBar:vertical {
            background-color: #1e1e1e;
            width: 10px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background-color: #4a4a4a;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #5a5a5a;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
}

void VideoPlayerWidget::playStream(const QString &streamName, const QString &streamUrl)
{
    m_currentStreamName = streamName;
    m_currentStreamUrl = streamUrl;
    
    // 更新标题
    m_streamTitleLabel->setText(QString("正在播放: %1").arg(streamName));
    
    // 立即清理显示，避免残留
    m_videoDisplayLabel->clear();
    m_videoDisplayLabel->setText("正在连接...");
    
    // 停止当前播放
    stopStream();
    
    // 创建新的FFmpeg播放器
    m_streamPlayer = new StreamPlayer(streamUrl, this);
    
    // 连接信号槽
    connect(m_streamPlayer, &StreamPlayer::frameReady, this, &VideoPlayerWidget::onFrameReady);
    connect(m_streamPlayer, &StreamPlayer::errorSignal, this, &VideoPlayerWidget::onStreamError);
    
    // 开始播放
    m_streamPlayer->start();
    
    // 添加播放开始信息
    addAlarmMessage(QString("开始播放流: %1").arg(streamName));
    addAlarmMessage(QString("流地址: %1").arg(streamUrl));
}

void VideoPlayerWidget::stopStream()
{
    if (m_streamPlayer) {
        // 断开信号槽连接，避免在清理过程中继续接收帧
        disconnect(m_streamPlayer, &StreamPlayer::frameReady, this, &VideoPlayerWidget::onFrameReady);
        disconnect(m_streamPlayer, &StreamPlayer::errorSignal, this, &VideoPlayerWidget::onStreamError);
        
        // 停止播放器
        m_streamPlayer->stop();
        
        // 等待线程结束，设置超时避免无限等待
        if (!m_streamPlayer->wait(3000)) { // 等待3秒
            m_streamPlayer->terminate(); // 强制终止
            m_streamPlayer->wait(); // 再次等待
        }
        
        // 清理资源
        m_streamPlayer->deleteLater();
        m_streamPlayer = nullptr;
        
        // 清空显示
        m_videoDisplayLabel->clear();
        m_videoDisplayLabel->setText("视频已停止");
        
        addAlarmMessage(QString("停止播放流: %1").arg(m_currentStreamName));
    }
}

void VideoPlayerWidget::onFrameReady(const QImage &img)
{
    // 检查播放器是否还存在，避免在停止过程中继续显示
    if (!m_streamPlayer) {
        return;
    }
    
    // 将QImage转换为QPixmap并显示
    QPixmap pixmap = QPixmap::fromImage(img);
    
    // 保持宽高比缩放
    pixmap = pixmap.scaled(m_videoDisplayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    m_videoDisplayLabel->setPixmap(pixmap);
}

void VideoPlayerWidget::onStreamError(int stopCode)
{
    QString errorMsg;
    switch (stopCode) {
    case 1:
        errorMsg = "无法打开输入流";
        break;
    case 2:
        errorMsg = "无法分配数据包";
        break;
    case 3:
        errorMsg = "无法找到流信息";
        break;
    case 4:
        errorMsg = "找不到解码器";
        break;
    case 5:
        errorMsg = "图像格式转换失败";
        break;
    default:
        errorMsg = QString("未知错误，错误代码: %1").arg(stopCode);
        break;
    }
    
    addAlarmMessage(errorMsg);
    
    // 显示错误对话框
    QMessageBox::warning(this, "播放错误", errorMsg);
    
    // 清空显示
    m_videoDisplayLabel->clear();
    m_videoDisplayLabel->setText("播放出错");
}

void VideoPlayerWidget::onBackButtonClicked()
{
    stopStream();
    emit backToMain();
}

void VideoPlayerWidget::updateAlarmInfo()
{
    // 模拟报警信息更新
    static int counter = 0;
    counter++;
    
//    if (counter % 3 == 0) {
//        addAlarmMessage("系统运行正常");
//    }
    
//    if (counter % 5 == 0) {
//        addAlarmMessage("网络连接状态良好");
//    }
}

void VideoPlayerWidget::addAlarmMessage(const QString &message)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("hh:mm:ss");
    QString fullMessage = QString("[%1] %2").arg(timeStr, message);
    
    // 获取当前文本
    QString currentText = m_alarmTextEdit->toPlainText();
    QStringList lines = currentText.split('\n', QString::SkipEmptyParts);
    
    // 在开头插入新消息
    lines.prepend(fullMessage);
    
    // 限制消息数量，保持最新的50条
    const int maxMessages = 50;
    if (lines.size() > maxMessages) {
        lines = lines.mid(0, maxMessages); // 保留前50条（最新的）
    }
    
    // 重新设置文本
    QString newText = lines.join('\n');
    if (!newText.isEmpty()) {
        newText += '\n'; // 确保最后有换行符
    }
    
    // 设置新文本
    m_alarmTextEdit->setPlainText(newText);
    
    // 确保显示最新消息
    m_alarmTextEdit->moveCursor(QTextCursor::Start);
    
    qDebug() << "添加报警消息:" << fullMessage << "当前消息数量:" << lines.size();
} 
