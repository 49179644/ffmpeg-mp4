//
// Created by taichen on 2020/7/2.
//

#include <zconf.h>
#include "ffmpeg_mp4_dec.h"


mp4Dec::~mp4Dec() {
    close();
}




int mp4Dec::close() {

    working_ = false;
    usleep(30000);


    if(formate_ctx_)
        avformat_close_input(&formate_ctx_);
    if(abs_ctx_)
        av_bsf_free(&abs_ctx_);

    if(packet_)
        av_packet_unref(packet_);

    if(frame_buf_)
        av_frame_free(&frame_buf_);
    if(work_){
        work_->join();
        delete work_;
        work_ = nullptr;
        cout<<"work quit"<<endl;
    }


    return 0;
}


int mp4Dec::play() {
    if (!work_) {
        working_ = true;
        work_ = new std::thread(&mp4Dec::work, this);
    }

    return 0;
}


int mp4Dec::open(const char *path) {
    if(formate_ctx_){
        cout<<"file alreadly open"<<endl;
        return -1;
    }

    av_register_all();
    //打开媒体文件
    int ret = avformat_open_input(&formate_ctx_,path, nullptr, nullptr);
    if (ret != 0) {
        cout << "Couldn't open input stream. code :" << ret << std::endl;
        formate_ctx_ = nullptr;
        //若文件名读取失败，对UI发送时长为0的信号
        return -1;
    }

    //解析媒体信息
    avformat_find_stream_info(formate_ctx_, nullptr);
    durations_ = formate_ctx_->duration/AV_TIME_BASE;

    for(int i = 0;i<formate_ctx_->nb_streams;i++){

        switch(formate_ctx_->streams[i]->codecpar->codec_type){
            case AVMEDIA_TYPE_VIDEO:{
                switch(formate_ctx_->streams[i]->codecpar->codec_id){
                    case AV_CODEC_ID_HEVC:{
                        abs_filter_ = av_bsf_get_by_name("hevc_mp4toannexb");
                        av_bsf_alloc(abs_filter_, &abs_ctx_);
                        avcodec_parameters_copy(abs_ctx_->par_in, formate_ctx_->streams[i]->codecpar);
                        av_bsf_init(abs_ctx_);
                        break;
                    }
                    case AV_CODEC_ID_H264:{
                        abs_filter_ = av_bsf_get_by_name("h264_mp4toannexb");
                        av_bsf_alloc(abs_filter_, &abs_ctx_);
                        avcodec_parameters_copy(abs_ctx_->par_in, formate_ctx_->streams[i]->codecpar);
                        av_bsf_init(abs_ctx_);
                        break;
                    }
                    default:
                        break;
                }
                video_index_ = i;
                break;
            }

            case AVMEDIA_TYPE_AUDIO:{
                switch(formate_ctx_->streams[i]->codecpar->codec_id){
                    case AV_CODEC_ID_AAC:{
                        abs_filter_ = av_bsf_get_by_name("hevc_mp4toannexb");
                        break;
                    }
                    default:
                        break;
                }
                audio_index_ = i;

            }

            default:
                break;
        }
    }



    return 0;
}



void mp4Dec::work() {
    int ret = 0;

    speed_ = 1000000/formate_ctx_->streams[video_index_]->codecpar->bits_per_coded_sample;
    cout<<"play speed_   :"<<speed_<<endl;
    while(1){

        if(pause_){
            if(!working_)
                break;
            usleep(20000);
            continue;
        }
        if(av_read_frame(formate_ctx_, packet_) >= 0 && working_){

            if(packet_->stream_index == video_index_){

                //ffmpeg packet 可用本身的接口直接解码
                avcodec_send_packet(formate_ctx_->streams[video_index_]->codec, packet_);
                ret = avcodec_receive_frame(formate_ctx_->streams[video_index_]->codec, frame_buf_);
                if (ret ==0) {
                    std::cout << "解码成功" << std::endl;

                }

                //还原最初的码流封装
                if (av_bsf_send_packet(abs_ctx_, packet_) != 0) {
                    cout<<"not total package"<<endl;
                }
                usleep(100);
                if((av_bsf_receive_packet(abs_ctx_, packet_) != 0)){
                    cout<<"av_bsf_receive_packet err"<<endl;
                    break;
                }

                //do something
            } else {
//                    SAMPLE_PRT("vdec ok  %d\n",packet->size);
            }



        }

        if(packet_->stream_index == audio_index_){
            //do something
        }

    }

}


void mp4Dec::pause() {
    pause_ = !pause_;
}



