#include <msgClient.hpp>
#include <zmq.h>
#include <iostream>
#include <string>
#include <QDebug>
#include <QThread>

msgClient::msgClient(const QString &server_ip) {
    // 初始化ZMQ上下文
    context = zmq_ctx_new();
    if (!context) {
        qDebug() << "Failed to create ZMQ context";
        emit errorOccurred("Failed to create ZMQ context");
        return;
    }
    
    // 创建RTSP地址订阅者
    rtsp_subscriber = zmq_socket(context, ZMQ_SUB);
    if (!rtsp_subscriber) {
        qDebug() << "Failed to create RTSP subscriber socket";
        emit errorOccurred("Failed to create RTSP subscriber socket");
        zmq_ctx_destroy(context);
        return;
    }
    
    // 创建报警信息订阅者
    alarm_subscriber = zmq_socket(context, ZMQ_SUB);
    if (!alarm_subscriber) {
        qDebug() << "Failed to create alarm subscriber socket";
        emit errorOccurred("Failed to create alarm subscriber socket");
        zmq_close(rtsp_subscriber);
        zmq_ctx_destroy(context);
        return;
    }
    
    // 设置接收超时
    int timeout = 1000; // 1秒超时
    zmq_setsockopt(rtsp_subscriber, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(alarm_subscriber, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    
    // 连接到RTSP服务器
    std::string rtsp_endpoint = "tcp://" + server_ip.toStdString() + ":5555";
    int rc = zmq_connect(rtsp_subscriber, rtsp_endpoint.c_str());
    if (rc != 0) {
        qDebug() << "Failed to connect to RTSP server:" << zmq_strerror(zmq_errno());
        emit errorOccurred(QString("Failed to connect to RTSP server: %1").arg(zmq_strerror(zmq_errno())));
        zmq_close(alarm_subscriber);
        zmq_close(rtsp_subscriber);
        zmq_ctx_destroy(context);
        return;
    }
    
    // 连接到报警服务器
    std::string alarm_endpoint = "tcp://" + server_ip.toStdString() + ":5556";
    rc = zmq_connect(alarm_subscriber, alarm_endpoint.c_str());
    if (rc != 0) {
        qDebug() << "Failed to connect to alarm server:" << zmq_strerror(zmq_errno());
        emit errorOccurred(QString("Failed to connect to alarm server: %1").arg(zmq_strerror(zmq_errno())));
        zmq_close(alarm_subscriber);
        zmq_close(rtsp_subscriber);
        zmq_ctx_destroy(context);
        return;
    }
    
    // 订阅所有消息
    zmq_setsockopt(rtsp_subscriber, ZMQ_SUBSCRIBE, "", 0);
    zmq_setsockopt(alarm_subscriber, ZMQ_SUBSCRIBE, "", 0);
    
    qDebug() << "ZMQ客户端初始化成功";
    qDebug() << "连接到RTSP服务器:" << QString::fromStdString(rtsp_endpoint);
    qDebug() << "连接到报警服务器:" << QString::fromStdString(alarm_endpoint);
}

msgClient::~msgClient() {
    stop();
    
    if (alarm_subscriber) zmq_close(alarm_subscriber);
    if (rtsp_subscriber) zmq_close(rtsp_subscriber);
    if (context) zmq_ctx_destroy(context);
    
    qDebug() << "ZMQ客户端已关闭";
}

void msgClient::start() {
    if (running.load()) {
        qDebug() << "客户端已经在运行";
        return;
    }
    
    running = true;
    
    // 创建RTSP工作线程
    rtsp_thread = new QThread();
    ZmqWorker *rtsp_worker = new ZmqWorker(rtsp_subscriber, "RTSP", running);
    rtsp_worker->moveToThread(rtsp_thread);
    
    // 连接RTSP工作线程信号
    connect(rtsp_thread, &QThread::started, rtsp_worker, &ZmqWorker::run);
    connect(rtsp_worker, &ZmqWorker::messageReceived, this, &msgClient::rtspUrlReceived);
    connect(rtsp_worker, &ZmqWorker::errorOccurred, this, &msgClient::errorOccurred);
    connect(rtsp_thread, &QThread::finished, rtsp_worker, &ZmqWorker::deleteLater);
    connect(rtsp_thread, &QThread::finished, rtsp_thread, &QThread::deleteLater);
    
    // 创建报警工作线程
    alarm_thread = new QThread();
    ZmqWorker *alarm_worker = new ZmqWorker(alarm_subscriber, "ALARM", running);
    alarm_worker->moveToThread(alarm_thread);
    
    // 连接报警工作线程信号
    connect(alarm_thread, &QThread::started, alarm_worker, &ZmqWorker::run);
    connect(alarm_worker, &ZmqWorker::messageReceived, this, &msgClient::msgReceived);
    connect(alarm_worker, &ZmqWorker::errorOccurred, this, &msgClient::errorOccurred);
    connect(alarm_thread, &QThread::finished, alarm_worker, &ZmqWorker::deleteLater);
    connect(alarm_thread, &QThread::finished, alarm_thread, &QThread::deleteLater);
    
    // 启动线程
    rtsp_thread->start();
    alarm_thread->start();
    
    qDebug() << "ZMQ客户端已启动";
}

void msgClient::stop() {
    if (!running.load()) {
        return;
    }
    
    running = false;
    
    // 等待线程结束
    if (rtsp_thread) {
        rtsp_thread->quit();
        rtsp_thread->wait();
        rtsp_thread = nullptr;
    }
    
    if (alarm_thread) {
        alarm_thread->quit();
        alarm_thread->wait();
        alarm_thread = nullptr;
    }
    
    qDebug() << "ZMQ客户端已停止";
}

QString msgClient::receiveMessage(void *socket, const QString &socket_name) {
    char buffer[1024];
    int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
    
    if (size == -1) {
        if (zmq_errno() == EAGAIN) {
            // 超时，没有消息
            return QString();
        } else {
            qDebug() << QString("接收%1消息失败: %2").arg(socket_name).arg(zmq_strerror(zmq_errno()));
            emit errorOccurred(QString("接收%1消息失败: %2").arg(socket_name).arg(zmq_strerror(zmq_errno())));
            return QString();
        }
    }
    
    if (size > 0) {
        buffer[size] = '\0';
        qDebug() << QString("接收到%1消息，长度: %2").arg(socket_name).arg(size);
        return QString::fromUtf8(buffer, size);
    }
    
    return QString();
}

void msgClient::receiveRtspAddresses() {
    qDebug() << "RTSP地址接收线程已启动";
    
    int rtsp_count = 0;
    while (running.load()) {
        QString msg = receiveMessage(rtsp_subscriber, "RTSP");
        if (!msg.isEmpty()) {
            rtsp_count++;
            qDebug() << QString("[RTSP #%1] 接收到: %2").arg(rtsp_count).arg(msg);
            emit rtspUrlReceived(msg);
        }
        
        QThread::msleep(100); // 短暂延时
    }
    
    qDebug() << QString("RTSP地址接收线程已停止，共接收 %1 条消息").arg(rtsp_count);
}

void msgClient::receiveAlarmMessages() {
    qDebug() << "报警信息接收线程已启动";
    
    int alarm_count = 0;
    while (running.load()) {
        QString msg = receiveMessage(alarm_subscriber, "ALARM");
        if (!msg.isEmpty()) {
            alarm_count++;
            qDebug() << QString("[ALARM #%1] 接收到: %2").arg(alarm_count).arg(msg);
            emit msgReceived(msg);
        }
        
        QThread::msleep(100); // 短暂延时
    }
    
    qDebug() << QString("报警信息接收线程已停止，共接收 %1 条消息").arg(alarm_count);
}

// ZmqWorker 实现
ZmqWorker::ZmqWorker(void *socket, const QString &socket_name, std::atomic<bool> &running_flag)
    : m_socket(socket)
    , m_socket_name(socket_name)
    , m_running(running_flag)
{
}

void ZmqWorker::run() {
    qDebug() << QString("%1工作线程已启动").arg(m_socket_name);
    
    int count = 0;
    while (m_running.load()) {
        char buffer[1024];
        int size = zmq_recv(m_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (size == -1) {
            if (zmq_errno() == EAGAIN) {
                // 超时，没有消息
                QThread::msleep(100);
                continue;
            } else {
                qDebug() << QString("接收%1消息失败: %2").arg(m_socket_name).arg(zmq_strerror(zmq_errno()));
                emit errorOccurred(QString("接收%1消息失败: %2").arg(m_socket_name).arg(zmq_strerror(zmq_errno())));
                break;
            }
        }
        
        if (size > 0) {
            buffer[size] = '\0';
            count++;
            QString msg = QString::fromUtf8(buffer, size);
            qDebug() << QString("[%1 #%2] 接收到: %3").arg(m_socket_name).arg(count).arg(msg);
            emit messageReceived(msg);
        }
        
        QThread::msleep(100); // 短暂延时
    }
    
    qDebug() << QString("%1工作线程已停止，共接收 %2 条消息").arg(m_socket_name).arg(count);
}
