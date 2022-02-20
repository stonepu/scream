#pragma once

#include <memory>
#include <list>
#include <vector>
#include <mutex>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};

#include "parser.h"

#define MAX_ENCODE_WIDTH 1280
#define MAX_ENCODE_HEIGHT 960

namespace shining {
    class Encoder {
    private:
        //...
        AVCodecContext* encodeCtx = nullptr;
        AVCodec* encode = nullptr;
        SwsContext* swsCtx = nullptr;
        uint8_t* outBuff = nullptr;
        AVPixelFormat inputFmt = AV_PIX_FMT_YUV420P;
        std::shared_ptr<Parser> encodeParser = nullptr;

        //输出
        AVFormatContext* pFormatCtx = nullptr;
        AVOutputFormat* fmt = nullptr;
        AVStream* video_st = nullptr;
        bool encodeReady = false;
        bool isSaveToFile = false;
        std::string out_file_path = "test_okk_87.h264";
        int count = 0;



        //lock
        std::mutex sendMtx{};
        //std::condition_variable sendCv{};
        std::list<AVPacket*> sendList{}; //TODO 内存需要释放

        AVCodecID codecFmt = AV_CODEC_ID_H264;  //设置压缩格式，支持h264和vp8
        AVPixelFormat pixFmt = AV_PIX_FMT_YUV420P;  //设置输入图像的格式，支持yuv和rgba两种格式

        //输入的yuv avframe的尺寸
        int inputWidth = -1;
        int inputHeight = -1;

        //设置视频分辨率
        int outputWidth = -1;    //宽
        int outputHeight = -1;   //高
        bool isScale = false;

        int bitRate = 10000;  //设置视频码率
        int framerate = 30;     //设置视频帧率
        int gopsize = 15;       //设置关键帧之间的时间差
        int maxBframe = 0;      //设置最大B帧数

        std::string Preset = "medium";//设置preset取值
        std::string tune = "zerolatency"; //设置tune取值 根据输入视频的内容进行优化
        std::string Crf = "25.0";  //设置码率控制方法：恒定质量
        int Profile = 100; //设置限制输出视频流的profile

        uint64_t dts = 1;       //

        bool isCrf = true;
        void initSws();
        int bufferToAVFrame(uint8_t* data, AVFrame* yuvFrame, size_t width, size_t height);
        //判断是否需要修改分辨率以降低带宽
        bool needRescale();

        int updateOutSize();



    public:
        Encoder();
        ~Encoder();
        virtual void init();
        virtual void pushWatchFmt(uint8_t* data, int len, int width, int height, AVPixelFormat fmt);
        virtual void push(uint8_t* data, int len, int width, int height);
        virtual int poll(std::vector<std::pair<uint8_t*, int>>& outData);
        virtual void stop();
        virtual void setCodecFmt(std::string fmt);
        virtual void setPixFmt(AVPixelFormat fmt);
        virtual void setBitrate(int bitrate);
        virtual void setFrameRate(int frameRate);
        virtual void setFrameSize(int outWidth, int outHeight);
        virtual void setGopSize(int gopSize);
        virtual void setMaxBFrame(int maxBFrame);
        virtual void setPreset(std::string preset);
        virtual void setTune(std::string Tune);
        virtual void setCrf(std::string crf);
        virtual void setProfile(std::string profile);
        virtual void setIsCrf(bool setIsCrf);
        virtual void push(AVFrame* frame);


        void initOutput();

        void deleteOutput();

        void setOutput();
    };
}