//
// Created by taichen on 2020/11/12.
//

#ifndef T_FFMEG_FFMPEG_MP4_DEC_H
#define T_FFMEG_FFMPEG_MP4_DEC_H


#include <atomic>
extern "C"{
#include <libavformat/avformat.h>
}

#include <mutex>
#include "iostream"
#include "thread"


using namespace  std;


class mp4Dec {
public:
    mp4Dec():formate_ctx_(nullptr),abs_filter_(nullptr), abs_ctx_(nullptr),video_index_(-1),audio_index_(-1),durations_(0),speed_(0),work_(
            nullptr),working_(false),pause_(false),packet_(av_packet_alloc()),frame_buf_( av_frame_alloc()){};
    ~mp4Dec();

    int play();
    int close();

    int open(const char* path);

    void work();

    int durations(){ return durations_;}
    void pause();

private:
    string path_;
    AVFormatContext *formate_ctx_;
    const AVBitStreamFilter *abs_filter_;
    AVBSFContext *abs_ctx_;

    int video_index_;
    int audio_index_;
    uint durations_;
    uint speed_;


    thread * work_ ;
    bool   working_;
    bool   pause_;
    AVPacket *packet_;
    AVFrame*  frame_buf_;
    mutex  mtx;

};

#endif //T_FFMEG_FFMPEG_MP4_DEC_H
