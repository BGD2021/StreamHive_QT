#include "streamPlayer.h"
#include <QDebug>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include <libavutil/imgutils.h>
}

StreamPlayer::StreamPlayer(const QString &url, QObject *parent)
    : QThread(parent), streamUrl(url) ,isStop(false){
    avformat_network_init();
}

StreamPlayer::~StreamPlayer() {
    avformat_network_deinit();
}

void StreamPlayer::stop() {
    isStop.store(true);  // 设置停止标志位
}

void StreamPlayer::run() {
    AVFormatContext *fmtCtx = nullptr;
    AVCodecContext *codecCtx = nullptr;
    const AVCodec *codec = nullptr;
    AVFrame *frame = nullptr, *rgbFrame = nullptr;
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
        qWarning() << "Could not allocate packet";
        emit errorSignal(2);
        return;
    }
    uint8_t *rgbBuffer = nullptr;
    SwsContext *swsCtx = nullptr;

    AVDictionary *opts = nullptr;
    fmtCtx = avformat_alloc_context();
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "5000000", 0); // 5秒超时

    if (avformat_open_input(&fmtCtx, streamUrl.toStdString().c_str(), nullptr, &opts) < 0) {
        qWarning() << "Could not open input";
        emit errorSignal(1);
        return;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        qWarning() << "Could not find stream info";
        emit errorSignal(3);
        return;
    }

    int videoStreamIndex = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            qDebug() << "time base:" << fmtCtx->streams[i]->time_base.num << "/" << fmtCtx->streams[i]->time_base.den;
            qDebug() << "frame rate:" << fmtCtx->streams[i]->r_frame_rate.num << "/" << fmtCtx->streams[i]->r_frame_rate.den;
            qDebug() << "start time:" << fmtCtx->streams[i]->start_time;
            break;
        }
    }
    if (videoStreamIndex == -1) return;

    codec = avcodec_find_decoder(fmtCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if(!codec){
        qWarning()<<"decoder not found";
        emit errorSignal(4);
        return;
    }
    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx){
        qWarning()<<"codecctx is wrong";
        return;
    }
    avcodec_parameters_to_context(codecCtx, fmtCtx->streams[videoStreamIndex]->codecpar);
    avcodec_open2(codecCtx, codec, nullptr);

    frame = av_frame_alloc();
    rgbFrame = av_frame_alloc();

    int w = codecCtx->width, h = codecCtx->height;
    rgbBuffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, w, h, 1));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbBuffer, AV_PIX_FMT_RGB24, w, h, 1);

    swsCtx = sws_getContext(w, h, codecCtx->pix_fmt, w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx) {
        qWarning() << "sws_getContext failed";
        emit errorSignal(5);
        return;
    }
    while (!isStop.load()) {
        if (av_read_frame(fmtCtx, pkt) < 0) break;
        if (pkt->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecCtx, pkt) == 0) {
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, h, rgbFrame->data, rgbFrame->linesize);
                    QImage img(rgbFrame->data[0], w, h, rgbFrame->linesize[0], QImage::Format_RGB888);
                    emit frameReady(img.copy());
                }
            }
        }
        av_packet_unref(pkt);
        msleep(0.02);
    }

    // 清理资源
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_packet_free(&pkt);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
    sws_freeContext(swsCtx);
    av_free(rgbBuffer);
    qDebug() << "播放线程安全退出";
}
