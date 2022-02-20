#ifndef VIDEO_ENC
#define VIDEO_ENC
#include <functional>
#include "config.h"
#include "seeker/loggerApi.h"
#include <functional>
#include "grabber.h"
#include "encoder.h"
#include "fec.h"
namespace shining
{
    class RtpQueue;
#define MAX_FRAMES 10000
    class VideoEnc {
    public:
        VideoEnc(RtpQueue* rtpQueue, float frameRate, char* fname, int ixOffset = 0, bool test = true);
        ~VideoEnc();
        int encode(float time);
        void setEncoder(Encoder* encoder_) { encoder = encoder_; }
        void push(uint8_t* data, int w, int h, int pixFormat);
        void setTargetBitrate(float targetBitrate);
        typedef std::function<void(float)> RateCallback;
        void SetRateCallback(RateCallback cb) { m_rateCb = cb; }
        RtpQueue* rtpQueue;
        float frameSize[MAX_FRAMES];
        int nFrames;
        float targetBitrate = 0;
        float lastBitrate = -1000;
        float frameRate;
        float nominalBitrate;
        unsigned int seqNr;
        int ix;
        float m_fs{ 40 };
        RateCallback m_rateCb;
        float lastLogTime; //¥Ú”°»’÷æ


    private:
        Grabber* grabber;
        Encoder* encoder;
        PacketFec* packetFec;
        bool isTest;

        bool useFile = false;

        uint8_t* AVFrame2Img(AVFrame* pFrame);
    };
}
#endif
