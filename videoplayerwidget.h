#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFrame>
#include <QPixmap>
#include <QImage>

// 前向声明
class StreamPlayer;

class VideoPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget *parent = nullptr);
    ~VideoPlayerWidget();

public slots:
    void playStream(const QString &streamName, const QString &streamUrl);
    void stopStream();
    void addAlarmMessage(const QString &message);

signals:
    void backToMain();

private slots:
    void onBackButtonClicked();
    void onFrameReady(const QImage &img);
    void onStreamError(int stopCode);
    void updateAlarmInfo();

private:
    void setupUI();
    void applyStyles();


    // 主布局
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_contentLayout;
    
    // 顶部区域
    QHBoxLayout *m_topLayout;
    QPushButton *m_backButton;
    QLabel *m_streamTitleLabel;
    
    // 内容区域
    QVBoxLayout *m_videoLayout;
    QLabel *m_videoDisplayLabel;  // 替换QVideoWidget，用于显示QImage
    StreamPlayer *m_streamPlayer; // FFmpeg播放器
    
    // 右侧报警信息区域
    QVBoxLayout *m_alarmLayout;
    QLabel *m_alarmTitleLabel;
    QTextEdit *m_alarmTextEdit;
    
    // 定时器用于模拟报警信息
    QTimer *m_alarmTimer;
    
    // 当前播放的流信息
    QString m_currentStreamName;
    QString m_currentStreamUrl;
    
    // 样式常量
    static const int VIDEO_WIDTH = 800;
    static const int ALARM_WIDTH = 200;
    static const int TOP_HEIGHT = 60;
};

#endif // VIDEOPLAYERWIDGET_H 
