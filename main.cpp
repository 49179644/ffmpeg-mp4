#include <iostream>

#include "opencv2/opencv.hpp"
extern "C" {
#include "libavcodec/mediacodec.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
int ret = 0;
int chnStream;
int main(){
    const char file[128] = "./test0.mp4";

    av_register_all();

    AVFormatContext *pFormatCtx =  avformat_alloc_context();
    ret = avformat_open_input(&pFormatCtx, file, nullptr, nullptr);
    if (ret!= 0) {
        printf("Couldn't open input stream. code :%d\n",ret);
        return 0;
    }

    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    if (ret <0){
        std::cout<<"find stream info erro "<<std::endl;
    }

    std::cout<<pFormatCtx->nb_streams<<std::endl;

    for(int i = 0;i<pFormatCtx->nb_streams;i++){
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            chnStream = i;
            break;
        }
    }

    //查找获取编码器
    AVCodec *avcodec = avcodec_find_decoder(pFormatCtx->streams[chnStream]->codec->codec_id);

    //打开解码器 用解码信息初始化解码器
    std::cout<<"is open"<<avcodec_is_open(pFormatCtx->streams[chnStream]->codec)<<std::endl;
    ret = avcodec_open2(pFormatCtx->streams[chnStream]->codec, avcodec,NULL);
    if (ret !=0){
        std::cout<<"avcodec_open2 erro "<<std::endl;
    }


    //初始化转码器
    SwsContext *swscontext = sws_getContext(pFormatCtx->streams[chnStream]->codec->width,

                                            pFormatCtx->streams[chnStream]->codec->height,

                                            pFormatCtx->streams[chnStream]->codec->pix_fmt,

                                            pFormatCtx->streams[chnStream]->codec->width,

                                            pFormatCtx->streams[chnStream]->codec->height,

                                            AV_PIX_FMT_YUV420P,

                                            SWS_BICUBIC,

                                            NULL,

                                            NULL,

                                            NULL);


    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P,

                                               pFormatCtx->streams[chnStream]->codec->width,

                                               pFormatCtx->streams[chnStream]->codec->height,

                                               1);

    //开辟　frame buf
    AVFrame* avframe_buf = av_frame_alloc();
    AVFrame* avframe_out = av_frame_alloc();

    uint8_t *out_buffer = (uint8_t *)av_malloc(buffer_size);
    av_image_fill_arrays(avframe_out->data,

                         avframe_out->linesize,

                         out_buffer,

                         AV_PIX_FMT_YUVJ420P,

                         pFormatCtx->streams[chnStream]->codec->width,

                         pFormatCtx->streams[chnStream]->codec->height,

                         1);

    int y_size, u_size, v_size;
    auto packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    while (av_read_frame(pFormatCtx, packet) >=0){

        if (packet->stream_index == AVMEDIA_TYPE_VIDEO){

            avcodec_send_packet(pFormatCtx->streams[chnStream]->codec, packet);
            ret = avcodec_receive_frame(pFormatCtx->streams[chnStream]->codec, avframe_buf);
            if (ret ==0){
                std::cout<<"解码成功"<<std::endl;

                sws_scale(swscontext,

                          (const uint8_t *const *)avframe_buf->data,

                          avframe_buf->linesize,

                          0,

                          pFormatCtx->streams[chnStream]->codec->height,

                          avframe_out->data,

                          avframe_out->linesize);
//                cv::Mat yuv(pFormatCtx->streams[chnStream]->codec->height,pFormatCtx->streams[chnStream]->codec->width, CV_8UC3,avframe_out->data);
                cv::Mat rgbimg(pFormatCtx->streams[chnStream]->codec->height/2, pFormatCtx->streams[chnStream]->codec->width, CV_8UC1,avframe_out->data[1]);
//                cvtColor(yuv, rgbimg, cv::COLOR_YUV2RGB_NV12);
                cv::imshow("img",rgbimg);
                cv::imwrite("test.jpg",rgbimg);


            }

        }
    }


    av_packet_free(&packet);

    av_frame_free(&avframe_buf);
    av_frame_free(&avframe_out);
    avcodec_close(pFormatCtx->streams[chnStream]->codec);
    avformat_free_context(pFormatCtx);
//    avcodec_close(pFormatCtx);
    return 0;
}

int main0() {
    //QAudioFormat
    int ret = 0;
    const char file[128] = "./sample.mp4";
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx; //视频编码
    AVPacket *packet;
    av_register_all();
    //解码过滤器
    const AVBitStreamFilter *absFilter = nullptr;
    AVBSFContext *absCtx = nullptr;
    AVCodecParameters *codecpar = nullptr;
    pFormatCtx =  avformat_alloc_context();

    char codecName[12] = "h265";
    if (strcasecmp(codecName, "h264") == 0) {
        absFilter = av_bsf_get_by_name("h264_mp4toannexb");
    } else if (strcasecmp(codecName, "h265") == 0) {
        absFilter = av_bsf_get_by_name("hevc_mp4toannexb");
    }
    //过滤器分配内存
    av_bsf_alloc(absFilter, &absCtx);

    //初始化编解码库
//    av_register_all();
    pFormatCtx = avformat_alloc_context();
    //初始化pFormatCtx结构
    ret = avformat_open_input(&pFormatCtx, file, nullptr, nullptr);
    if (ret!= 0) {
        printf("Couldn't open input stream. code :%d\n",ret);
        return 0;
    }
    //获取音视频流数据信息
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        printf("Couldn't find stream information.\n");
        return 0;
    }
    videoindex = -1;
    //nb_streams视音频流的个数，获取音视频个数。
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;

            codecpar = pFormatCtx->streams[i]->codecpar;
            avcodec_parameters_copy(absCtx->par_in, codecpar);
            av_bsf_init(absCtx);
            break;
        }
    }
    if (videoindex == -1) {
        printf("Didn't find a video stream.\n");
        return 0;
    }
    //获取视频流编码结构
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    packet = av_packet_alloc();


    while (av_read_frame(pFormatCtx, packet) >= 0) {

        //如果是视频数据
        if (packet->stream_index == videoindex) {
//            std::cout<<"----------------------------------------------------------------------------------------"<<std::endl;
            if (av_bsf_send_packet(absCtx, packet) != 0) {
                av_free_packet(packet);
                continue;
            }
            while (av_bsf_receive_packet(absCtx, packet) == 0) {


//                char file[128];
//                sprintf(file,"./bary/%d",packet->size);
//                FILE *fd = fopen(file,"w");
//                fwrite(packet->data,packet->size,1,fd);
//                fwrite(packet->data,packet->size,1,h265);
//                fclose(fd);

//                vdec.send_frame(packet->data, packet->size, false, packet->pts);



//                while (!vdec.get_frame(vdec.m_chn, &VideoFrame, 0)) {
//                    //保存yuv 到队列
////                    std::cout<<"get yuv"<<std::endl;
//                    vdec.release_frame(vdec.m_chn, &VideoFrame);
//                }
//                vdec.release_frame(vdec.m_chn, &VideoFrame);

            }


        }
        av_free_packet(packet);
    }
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0;
}

