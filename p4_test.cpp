//
// Created by taichen on 2020/10/27.
//

#include <iostream>
#include <zconf.h>
#include <bitset>
#include "opencv2/opencv.hpp"
extern "C"{
#include <libavformat/avformat.h>
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}


/*
 *
add_executable(p4 p4_test.cpp)
target_link_libraries(p4 avcodec avformat  swresample  avutil swscale ${CVLIB})
 * */
using namespace std;
int main(){


    int ret = 0;
//    string path("/home/taichen/Desktop/19project/aaa.mp4");
    string path("/home/taichen/Desktop/19project/1.mp4");
//    string path("ditie.mp4");

    //ffmpeg attr
    AVFormatContext *pFormatCtx = nullptr;//必须空指针
    int i, video_id=-1, audio_id=-1;



    av_register_all();


    //打开媒体文件
    ret = avformat_open_input(&pFormatCtx, path.c_str(), nullptr, nullptr);
    if (ret != 0) {
        cout << "Couldn't open input stream. code :" << ret << std::endl;
        //若文件名读取失败，对UI发送时长为0的信号
        return 0;
    }

    //解析媒体信息
    avformat_find_stream_info(pFormatCtx, nullptr);
    int durations = pFormatCtx->duration/AV_TIME_BASE;







    //解码过滤器
    const AVBitStreamFilter *absFilter = nullptr;
    AVBSFContext *absCtx = nullptr;

    for(i = 0;i<pFormatCtx->nb_streams;i++){

        switch(pFormatCtx->streams[i]->codecpar->codec_type){
            case AVMEDIA_TYPE_VIDEO:{
                switch(pFormatCtx->streams[i]->codecpar->codec_id){
                    case AV_CODEC_ID_HEVC:{
                        absFilter = av_bsf_get_by_name("hevc_mp4toannexb");
                        av_bsf_alloc(absFilter, &absCtx);
                        avcodec_parameters_copy(absCtx->par_in, pFormatCtx->streams[i]->codecpar);
                        av_bsf_init(absCtx);
                        break;
                    }
                    case AV_CODEC_ID_H264:{
                        absFilter = av_bsf_get_by_name("h264_mp4toannexb");
                        av_bsf_alloc(absFilter, &absCtx);
                        avcodec_parameters_copy(absCtx->par_in, pFormatCtx->streams[i]->codecpar);
                        av_bsf_init(absCtx);
                        break;
                    }
                    default:
                        break;
                }
                video_id = i;
                break;
            }

            case AVMEDIA_TYPE_AUDIO:{
                switch(pFormatCtx->streams[i]->codecpar->codec_id){
                    case AV_CODEC_ID_AAC:{
                        absFilter = av_bsf_get_by_name("hevc_mp4toannexb");
                        break;
                    }
                    default:
                        break;
                }
                audio_id = i;

            }


            default:
                break;
        }
    }


    //创建解码器
    avcodec_register_all();
    //查找解码器
    AVCodec *codec = avcodec_find_decoder(pFormatCtx->streams[video_id]->codecpar->codec_id);
    if(!codec)
        cout<<"find decoder err"<<endl;
    AVCodecContext *codectx = avcodec_alloc_context3(codec);
    if(!codectx){
        cout<<"codectx alloc err"<<endl;
        return 0;
    }

    //配置解码输出参数
    if (avcodec_open2(codectx, codec, nullptr) < 0) {
        printf("Could not open codec\n");
        return 0;
    }



    //初始化img_convert_ctx结构 转码器
//    struct SwsContext *sws_ctx = sws_getContext(codectx->width, codectx->height, (enum AVPixelFormat)AV_PIX_FMT_YUVJ420P,
//                                                codectx->width, codectx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);


    //读取内容
    int count=1;
    AVPacket *packet = av_packet_alloc();

//    FILE *fd = fopen("ditie.h264","w");

    AVFrame *frame = av_frame_alloc();


    while(1){
        if(av_read_frame(pFormatCtx, packet) >= 0){

            if(packet->stream_index == video_id){
                //解码







                if (av_bsf_send_packet(absCtx, packet) != 0) {
                    cout<<"not total package"<<endl;
                }
                usleep(100);

                if((av_bsf_receive_packet(absCtx, packet) != 0)){
                    cout<<"av_bsf_receive_packet err"<<endl;
                    return 0;
                }


//                fwrite(packet->data,packet->size,1,fd);

//                char name[128];
//                sprintf(name,"./testfile/%d",count);
//                FILE *fd = fopen(name,"w");
//                fwrite(packet->data,packet->size,1,fd);
//                fclose(fd);

                ret = avcodec_send_packet(codectx,packet);
                if(0!=ret){
                    cout<<"avcodec_send_packet err"<<endl;
                    continue;
                }
                ret = avcodec_receive_frame(codectx,frame);
                if(0!=ret){
                    cout<<"avcodec_receive_frame err"<<endl;
                    continue;
                }



                //AV_PIX_FMT_YUVJ420P
                cv::Mat img(frame->height,frame->width,CV_8UC1,frame->data[0]);

                cv::imshow("1",img);
                cv::imwrite("testfile/a.jpg",img);
//                pause();


                count++;
            }

            if(count>30)
                break;
        }
        else{
            break;
        }

    }


//    fclose(fd);

    av_frame_free(&frame);
    av_packet_unref(packet);

    return 0;
}