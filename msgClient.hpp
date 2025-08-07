#ifndef MSGCLIENT_H
#define MSGCLIENT_H

#include <QThread>
#include <QObject>
#include <QString>
#include <atomic>

class msgClient : public QObject {
    Q_OBJECT
private:
    void *context;
    void *rtsp_subscriber;    // 订阅5555端口的RTSP地址
    void *alarm_subscriber;   // 订阅5556端口的报警信息
    std::atomic<bool> running{false};
    
    QThread *rtsp_thread;
    QThread *alarm_thread;

public:
    msgClient(const QString &server_ip = "192.168.10.107");
    ~msgClient();
    
    void start();
    void stop();
    
    // 接收消息的通用函数
    QString receiveMessage(void *socket, const QString &socket_name);
    
    // 线程函数
    void receiveRtspAddresses();
    void receiveAlarmMessages();

signals:
    void msgReceived(const QString &msg);
    void rtspUrlReceived(const QString &msg);
    void errorOccurred(const QString &error_msg);
};

// 工作线程类
class ZmqWorker : public QObject {
    Q_OBJECT
public:
    ZmqWorker(void *socket, const QString &socket_name, std::atomic<bool> &running_flag);
    
public slots:
    void run();

signals:
    void messageReceived(const QString &msg);
    void errorOccurred(const QString &error_msg);

private:
    void *m_socket;
    QString m_socket_name;
    std::atomic<bool> &m_running;
};

#endif // MSGCLIENT_H
