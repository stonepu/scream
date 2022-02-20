#pragma once

#include <list>
#include <mutex>
#include <map>
#include <memory>
#include <iostream>

extern "C"
{
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "parser.h"

constexpr auto ALIGN = 1;

namespace shining {
	class Decoder {
	public:

		Decoder();
		~Decoder();
        virtual void init();
        virtual void push(uint8_t* data, int len, int64_t timestamp);
        virtual int poll(AVFrame* outData);
        virtual void setPixFmt(AVPixelFormat fmt);
        virtual void stop();
        virtual void scale(AVFrame* frame, int width, int height);
        void setAVFormatContext(AVFormatContext* formatContext);

    private:
        //±‡¬Îœ‡πÿ
        const AVCodec* codec = nullptr;
        AVFormatContext* formatCtx = nullptr;
        AVCodecParserContext* parser = NULL;
        AVCodecContext* c = NULL;
        AVPacket* pkt = NULL;
        int inputWidth = 640;
        int inputHeight = 480;
        int outputWidth = 640;
        int outputHeight = 480;
        uint8_t* outBuff = nullptr;
        uint8_t* h264Data = nullptr;

        SwsContext* sws_ctx = nullptr;
        AVPixelFormat dstPixFmt = AV_PIX_FMT_NONE;

        std::list<AVFrame*> playList{};
        std::shared_ptr<Parser> decodeParser = nullptr;

        //locker
        std::mutex sendMtx{};
        //std::condition_variable sendCv{};
        bool decodeReady = false;

        void decode(AVPacket* pkt, uint64_t ts);
        void changeFmtAndSave(AVFrame* frameYUV);
        void initSws();
	};
}