//
// Created by taichen on 2020/10/27.
//

#include <iostream>
#include <zconf.h>
#include <bitset>

extern "C"{
#include <libavformat/avformat.h>
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
//    string path("/home/taichen/Desktop/19project/1.mp4");
    string path("ditie.mp4");

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



    //读取内容

    AVPacket *packet = av_packet_alloc();

//    FILE *fd = fopen("ditie.h264","w");


    while(1){
        if(av_read_frame(pFormatCtx, packet) >= 0){

            if(packet->stream_index == video_id){
                if (av_bsf_send_packet(absCtx, packet) != 0) {
                    cout<<"not total package"<<endl;
                }
                usleep(100);

                if((av_bsf_receive_packet(absCtx, packet) != 0)){
                    cout<<"av_bsf_receive_packet err"<<endl;
                    return 0;
                }
//                fwrite(packet->data,packet->size,1,fd);

            }


        }
        else{
            break;
        }

    }


//    fclose(fd);

    av_packet_unref(packet);

    return 0;
}