#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>

#include "config.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include "videoenc.h"
#include "rtpqueue.h"


static const int kMaxRtpSize = 1200;
static const int kRtpOverHead = 12;
namespace shining
{
    //NS_LOG_COMPONENT_DEFINE("VideoEnc");
    VideoEnc::VideoEnc(RtpQueue* rtpQueue_, float frameRate_, char* fname, int ixOffset_, bool test_) {
        isTest = test_;
        rtpQueue = rtpQueue_;
        frameRate = frameRate_;
        ix = ixOffset_;
        nFrames = 0;
        seqNr = 0;
        nominalBitrate = 0.0;
        if (test_) {
            I_LOG("video enc start");
            FILE *fp=nullptr;
            fp= fopen(fname,"r");
            //NS_ASSERT_MSG(fp,"file open failed");
            char s[100];
            float sum = 0.0;
            while (fgets(s,99,fp)) {
                if (nFrames < MAX_FRAMES - 1) {
                    float x = atof(s);
                    frameSize[nFrames] = x;
                    nFrames++;
                    sum += x;
                }
            }
            float t = nFrames / frameRate;
            nominalBitrate = sum * 8 / t;
            fclose(fp);
            I_LOG("video enc over");

        }
        else if (useFile) {
            grabber = new Grabber();
            grabber->open(fname);
            encoder = new Encoder();
            //I_LOG("grabber: ", grabber->getPixFormat());
            encoder->setPixFmt(grabber->getPixFormat());
            encoder->init();
            packetFec = new PacketFec();
        }
        else {
            //encoder = new Encoder();
            //encoder->setPixFmt(AV_PIX_FMT_YUYV422);
            //encoder->init();
            //I_LOG("video encoder pix: AV_PIX_FMT_YUYV422");
            packetFec = new PacketFec();
        }
        I_LOG("video enc cons success");
    }

    VideoEnc::~VideoEnc() {
        if (grabber) {
            delete grabber;
        }
        if (encoder) {
            encoder->stop();
            delete encoder;
        }
        if (packetFec) {
            delete packetFec;
        }
    }

    void VideoEnc::push(uint8_t* data, int w, int h, int pixFormat)
    {
        if (encoder) {
            int len = av_image_get_buffer_size((AVPixelFormat)pixFormat, w, h, 2);
            encoder->push(data, len, w, h);
        }
        else {
            E_LOG("encoder is null");
        }
    }

    void VideoEnc::setTargetBitrate(float targetBitrate_) {
        //double now = Simulator::Now().GetSeconds();
        double now = 1.0 * seeker::Time::currentTime() / 1e3;

        if (targetBitrate_ > 1e-10) {
            if (now - lastLogTime >= 0.19) {
                //NS_LOG_INFO("enc " << now << " " << targetBitrate_ / 1000);
                lastLogTime = now;
            }
            if (m_rateCb) {
                double kbps = targetBitrate_ / 1000;
                m_rateCb(kbps);
            }
            //I_LOG("lastBitrate, {}, targetBitrate: {}, bitrate change: {}", lastBitrate, targetBitrate, std::abs((int)(lastBitrate - targetBitrate)));
            if (!isTest) {
                if (std::abs(lastBitrate - targetBitrate_) >= 1000 * 50) {
                    lastBitrate = targetBitrate;
                    targetBitrate = targetBitrate_;
                    //todo 更新编码器的码率
                    I_LOG("update encode bitrate: {}", targetBitrate);
                    //encoder->setBitrate(targetBitrate);
                    //encoder->init();
                }
            }
            else {
                targetBitrate = targetBitrate_;
            }

            //I_LOG("{} enc {} kbps", now , targetBitrate_ / 1000);


        }
    }

    int VideoEnc::encode(float time) {
        int rtpBytes = 0;

        if (isTest) {
            int bytes = ceil(0.001 * m_fs * targetBitrate / 8);
            //NS_LOG_INFO("frame size " << bytes);
            while (bytes > 0) {
                int rtpSize = std::min(kMaxRtpSize, bytes);
                bytes -= rtpSize;
                rtpSize += kRtpOverHead;
                rtpBytes += rtpSize;
                rtpQueue->push(0, rtpSize, seqNr, time);
                seqNr++;
            }

        }
        else if (useFile) {
            auto frame = grabber->grabAndDecode();
            if (!frame) {
                return 0;
            }
            //I_LOG("grab fmt:", frame->format);
            //I_LOG("grab width:", frame->width);
            //I_LOG("grab height:", frame->height);

            encoder->push(frame);
            //I_LOG("push ");
            std::vector<std::pair<uint8_t*, int>> dataVec{};
            int ret = encoder->poll(dataVec);
            //I_LOG("poll");
            if (ret < 0) {
                return 0;
            }

            //todo 将dataVec进行fec
            int bytes = 0;

            for (int i = 0; i < dataVec.size(); i++) {
                int rtpSize = std::get<1>(dataVec[i]);
                rtpBytes += rtpSize;
                rtpQueue->push(std::get<0>(dataVec[i]), rtpSize, seqNr, time);
                seqNr++;
            }
        }
        else {
            std::vector<std::pair<uint8_t*, int>> dataVec{};
            int ret = encoder->poll(dataVec);
            //I_LOG("poll from encoder");
            if (ret != 0) {
                //W_LOG("encoder is empty");
                return 0;
            }
            //todo 将dataVec进行fec
            int bytes = 0;
            for (int i = 0; i < dataVec.size(); i++) {
                int rtpSize = std::get<1>(dataVec[i]);
                rtpBytes += rtpSize;
                rtpQueue->push(std::get<0>(dataVec[i]), rtpSize, seqNr, time);
                seqNr++;
            }
        }

        rtpQueue->setSizeOfLastFrame(rtpBytes);
        return rtpBytes;
    }

    uint8_t* VideoEnc::AVFrame2Img(AVFrame* pFrame) {
        I_LOG("start transfer");
        //I_LOG("data", pFrame->data);
        //I_LOG("line0 ", pFrame->linesize[0]);
        //I_LOG("line1 ", pFrame->linesize[1]);
        //I_LOG("line2 ", pFrame->linesize[2]);

        int frame_height = pFrame->height;
        int frame_width = pFrame->width;
        int channels = 3;

        //反转图像
        //pFrame->data[0] += pFrame->linesize[0] * (frameHeight - 1);
        //pFrame->linesize[0] *= -1;
        //pFrame->data[1] += pFrame->linesize[1] * (frameHeight / 2 - 1);
        //pFrame->linesize[1] *= -1;
        //pFrame->data[2] += pFrame->linesize[2] * (frameHeight / 2 - 1);
        //pFrame->linesize[2] *= -1;

        //创建保存yuv数据的buffer
        uint8_t* yuv_buffer = (uint8_t*)malloc(
            frame_height * frame_width * sizeof(uint8_t) * channels);

        
    // 获取图片原始数据
    // Y
        //I_LOG("data[0]: {}", data[0]);
        for (int i = 0; i < frame_height; i++) {
            memcpy(yuv_buffer + frame_width * i,
                pFrame->data[0] + pFrame->linesize[0] * i,
                frame_width);
        }
        // U
        //I_LOG("data[1]: {}", data[1]);
        for (int j = 0; j < frame_height / 2; j++) {
            memcpy(yuv_buffer + frame_width * frame_height + frame_width / 2 * j,
                pFrame->data[1] + pFrame->linesize[1] * j,
                frame_width / 2);
        }
        // V
        //I_LOG("data[2]: {}", data[2]);
        for (int k = 0; k < frame_height / 2; k++) {
            memcpy(yuv_buffer + frame_width * frame_height + frame_width / 2 * (frame_height / 2) + frame_width / 2 * k,
                pFrame->data[2] + pFrame->linesize[2] * k,
                frame_width / 2);
        }
        //I_LOG("stop transfer");
        return yuv_buffer;
    }
}


