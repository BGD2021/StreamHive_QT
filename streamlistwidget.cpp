#include "streamlistwidget.h"
#include <QMouseEvent>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QSizePolicy>
#include <QDebug>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

StreamListWidget::StreamListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
    
    // 清空示例数据，改为动态添加
    // addStreamItem("前门监控", "rtsp://admin:qweasdzxc12@192.168.10.105/h264/ch1/main/av_stream", "在线");
    // addStreamItem("后门监控", "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8", "在线");
    // addStreamItem("停车场监控", "rtsp://192.168.1.102:554/stream1", "离线");
    // addStreamItem("大厅监控", "rtsp://192.168.1.103:554/stream1", "在线");
    // addStreamItem("电梯监控", "rtsp://192.168.1.104:554/stream1", "在线");
    // addStreamItem("走廊监控", "rtsp://192.168.1.105:554/stream1", "在线");
}

void StreamListWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(20);
    
    // 创建头部布局
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setSpacing(20);
    
    // 创建标题
    m_titleLabel = new QLabel("RTSP流监控列表", this);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // 创建刷新按钮
    m_refreshButton = new QPushButton("刷新列表", this);
    m_refreshButton->setFixedSize(100, 40);
    connect(m_refreshButton, &QPushButton::clicked, this, &StreamListWidget::onRefreshButtonClicked);
    
    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // 创建流列表
    m_streamList = new QListWidget(this);
    m_streamList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_streamList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_streamList->setSpacing(ITEM_MARGIN);
    m_streamList->setViewMode(QListView::ListMode);
    m_streamList->setResizeMode(QListView::Adjust);
    m_streamList->setUniformItemSizes(false);
    m_streamList->setSelectionMode(QAbstractItemView::NoSelection);
    m_streamList->setFocusPolicy(Qt::NoFocus);
    
    m_mainLayout->addWidget(m_streamList);
    
    // 连接信号槽
    connect(m_streamList, &QListWidget::itemClicked,
            this, &StreamListWidget::onItemClicked);
}

void StreamListWidget::addRtspStream(const QString &rtspUrl)
{
    // 尝试解析JSON格式的流信息
    QString streamName, url, id;
    if (parseJsonStreamInfo(rtspUrl, streamName, url, id)) {
        // JSON解析成功，使用解析出的信息
        qDebug() << "解析JSON成功:" << streamName << "->" << url << "ID:" << id;
        
        // 检查解析后的URL是否已经存在
        if (m_existingUrls.contains(url)) {
            qDebug() << "RTSP URL已存在，跳过:" << url;
            return;
        }
        
        // 添加到列表
        addStreamItem(streamName, url, "在线");
        
        // 添加到已存在URL集合
        m_existingUrls.insert(url);
        
        qDebug() << "添加新的RTSP流:" << streamName << "->" << url;
    } else {
        // JSON解析失败，尝试作为普通URL处理
        qDebug() << "JSON解析失败，作为普通URL处理:" << rtspUrl;
        
        // 检查原始URL是否已经存在
        if (m_existingUrls.contains(rtspUrl)) {
            qDebug() << "RTSP URL已存在，跳过:" << rtspUrl;
            return;
        }
        
        // 提取流名称
        QString streamName = extractStreamName(rtspUrl);
        
        // 添加到列表
        addStreamItem(streamName, rtspUrl, "在线");
        
        // 添加到已存在URL集合
        m_existingUrls.insert(rtspUrl);
        
        qDebug() << "添加新的RTSP流:" << streamName << "->" << rtspUrl;
    }
}

bool StreamListWidget::parseJsonStreamInfo(const QString &jsonData, QString &name, QString &url, QString &id)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << error.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qDebug() << "JSON数据不是对象格式";
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // 检查必需的字段
    if (!obj.contains("name") || !obj.contains("url")) {
        qDebug() << "JSON缺少必需字段 (name 或 url)";
        return false;
    }
    
    // 提取字段值
    name = obj["name"].toString();
    url = obj["url"].toString();
    id = obj["id"].toString(); // id是可选的
    
    // 验证数据有效性
    if (name.isEmpty() || url.isEmpty()) {
        qDebug() << "JSON字段值为空";
        return false;
    }
    
    qDebug() << "JSON解析成功 - 名称:" << name << "URL:" << url << "ID:" << id;
    return true;
}

void StreamListWidget::clearStreamList()
{
    qDebug() << "清空流列表";
    
    // 清空列表控件
    m_streamList->clear();
    
    // 清空URL集合
    m_existingUrls.clear();
    
    // 清空ID集合
    m_existingIds.clear();
    
    qDebug() << "流列表已清空";
}

void StreamListWidget::onRefreshButtonClicked()
{
    qDebug() << "刷新按钮被点击";
    clearStreamList();
}

QString StreamListWidget::extractStreamName(const QString &rtspUrl)
{
    // 尝试从URL中提取有意义的名称
    QUrl url(rtspUrl);
    
    // 如果有主机名，使用主机名
    if (!url.host().isEmpty()) {
        QString host = url.host();
        
        // 根据IP地址或主机名生成名称
        if (host.contains("192.168.10.105")) {
            return "前门监控";
        } else if (host.contains("192.168.10.106")) {
            return "后门监控";
        } else if (host.contains("192.168.10.107")) {
            return "yf403";
        } else if (host.contains("192.168.10.108")) {
            return "大厅监控";
        } else if (host.contains("192.168.10.109")) {
            return "电梯监控";
        } else if (host.contains("192.168.10.110")) {
            return "走廊监控";
        } else {
            // 使用IP地址作为名称
            return QString("监控-%1").arg(host);
        }
    }
    
    // 如果无法解析，使用默认名称
    return QString("RTSP流-%1").arg(m_streamList->count() + 1);
}

void StreamListWidget::addStreamItem(const QString &name, const QString &url, const QString &status)
{
    StreamItemWidget *itemWidget = new StreamItemWidget(name, url, status);
    QListWidgetItem *item = new QListWidgetItem(m_streamList);
    
    // 设置合适的高度，确保文字能够完整显示
    int itemHeight = ITEM_HEIGHT;
    if (url.length() > 40) {
        itemHeight += 30; // 如果URL很长，增加更多高度
    }
    if (url.length() > 60) {
        itemHeight += 20; // 如果URL非常长，再增加高度
    }
    
    // 确保最小高度
    itemHeight = qMax(itemHeight, 100);
    
    item->setSizeHint(QSize(m_streamList->width() - 40, itemHeight));
    
    m_streamList->setItemWidget(item, itemWidget);
    
    // 连接点击信号
    connect(itemWidget, &StreamItemWidget::clicked, [this, name, url]() {
        emit streamSelected(name, url);
    });
}

void StreamListWidget::applyStyles()
{
    // 设置整体样式
    setStyleSheet(R"(
        QWidget {
            background-color: #1e1e1e;
            color: #ffffff;
        }
    )");
    
    // 设置标题样式
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 24px;
            font-weight: bold;
            color: #ffffff;
            padding: 10px;
            background-color: #2d2d2d;
            border-radius: 8px;
            font-family: Arial;
        }
    )");
    
    // 设置刷新按钮样式
    m_refreshButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            border: 1px solid #45a049;
            border-radius: 6px;
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #45a049;
            border: 1px solid #3d8b40;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
    )");
    
    // 设置列表样式
    m_streamList->setStyleSheet(R"(
        QListWidget {
            background-color: #1e1e1e;
            border: none;
            outline: none;
        }
        QListWidget::item {
            background-color: transparent;
            border: none;
            padding: 0px;
        }
        QListWidget::item:selected {
            background-color: transparent;
        }
        QListWidget QLabel {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #2d2d2d;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #4a4a4a;
            border-radius: 6px;
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

void StreamListWidget::onItemClicked(QListWidgetItem *item)
{
    // 点击事件已在addStreamItem中处理
    Q_UNUSED(item)
}

// StreamItemWidget 实现
StreamItemWidget::StreamItemWidget(const QString &name, const QString &url, 
                                   const QString &status, QWidget *parent)
    : QFrame(parent)
    , m_name(name)
    , m_url(url)
    , m_status(status)
{
    setupUI();
    applyStyles();
}

void StreamItemWidget::setupUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(15, 10, 15, 10);
    m_layout->setSpacing(15);
    
    // 信息布局
    m_infoLayout = new QVBoxLayout();
    m_infoLayout->setSpacing(8); // 增加间距
    
    // 流名称标签
    m_nameLabel = new QLabel(m_name, this);
    m_nameLabel->setFont(QFont("Arial", 14, QFont::Bold));
    
    // 流地址标签
    m_urlLabel = new QLabel(m_url, this);
    m_urlLabel->setFont(QFont("Courier New", 11));
    m_urlLabel->setWordWrap(true);
    m_urlLabel->setTextFormat(Qt::PlainText);
    m_urlLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_urlLabel->setMinimumHeight(35); // 确保有足够的高度显示URL
    m_urlLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_urlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse); // 允许选择文本
    
    // 状态标签
    m_statusLabel = new QLabel(m_status, this);
    m_statusLabel->setFont(QFont("Arial", 12, QFont::Bold));
    
    m_infoLayout->addWidget(m_nameLabel);
    m_infoLayout->addWidget(m_urlLabel);
    m_infoLayout->addWidget(m_statusLabel);
    
    m_layout->addLayout(m_infoLayout);
    m_layout->addStretch();
    
    // 箭头标签
    m_arrowLabel = new QLabel("▶", this);
    m_arrowLabel->setFont(QFont("Arial", 16));
    m_arrowLabel->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_arrowLabel);
}

void StreamItemWidget::applyStyles()
{
    setStyleSheet(R"(
        StreamItemWidget {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 8px;
        }
        StreamItemWidget:hover {
            background-color: #3d3d3d;
            border: 1px solid #4d4d4d;
        }
        StreamItemWidget QLabel {
            background-color: transparent;
            border: none;
        }
    )");
    
    // 设置流名称标签样式
    m_nameLabel->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-weight: bold;
            font-size: 14px;
            background-color: transparent;
            border: none;
        }
    )");
    
    // 设置URL标签样式
    m_urlLabel->setStyleSheet(R"(
        QLabel {
            color: #aaaaaa;
            font-size: 11px;
            padding: 2px;
            background-color: transparent;
            border: none;
        }
    )");
    
    // 设置状态颜色
    QString statusColor = (m_status == "在线") ? "#4CAF50" : "#F44336";
    m_statusLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-weight: bold;
            font-size: 12px;
            background-color: transparent;
            border: none;
        }
    )").arg(statusColor));
    
    // 设置箭头颜色
    m_arrowLabel->setStyleSheet(R"(
        QLabel {
            color: #888888;
            font-size: 16px;
            background-color: transparent;
            border: none;
        }
    )");
}

void StreamItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QFrame::mousePressEvent(event);
}

void StreamItemWidget::enterEvent(QEvent *event)
{
    m_arrowLabel->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
        }
    )");
    QFrame::enterEvent(event);
}

void StreamItemWidget::leaveEvent(QEvent *event)
{
    m_arrowLabel->setStyleSheet(R"(
        QLabel {
            color: #888888;
        }
    )");
    QFrame::leaveEvent(event);
} 
