#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QTextEdit>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "msgClient.hpp"

class StreamListWidget;
class VideoPlayerWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStreamSelected(const QString &streamName, const QString &streamUrl);
    void onBackToMain();
    void onMsgReceived(const QString &msg);
    void onRtspUrlReceived(const QString &msg);
    void onZmqError(const QString &error_msg);

private:
    void setupUI();
    void setupStreamData();
    void applyStyles();

    QStackedWidget *m_stackedWidget;
    StreamListWidget *m_streamListWidget;
    VideoPlayerWidget *m_videoPlayerWidget;
    
    // 窗口尺寸常量
    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 600;
};

#endif // MAINWINDOW_H
