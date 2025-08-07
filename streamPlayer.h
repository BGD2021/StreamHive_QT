#ifndef FFMPEGWORKER_H
#define FFMPEGWORKER_H

#include <QThread>
#include <QImage>
#include <QString>

//extern "C" {
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libavcodec/avcodec.h>
//}

class StreamPlayer : public QThread
{
    Q_OBJECT
public:
    StreamPlayer(const QString &url, QObject *parent = nullptr);
    ~StreamPlayer();
    void stop();

signals:
    void frameReady(const QImage &img);
    void errorSignal(int stopCode);

protected:
    void run() override;

private:
    QString streamUrl;
    std::atomic<bool> isStop;
};


#endif // FFMPEGWORKER_H
