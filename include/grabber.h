#pragma once

#include <string>
#include <iostream>
#include "util.h"
#include <exception>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

using std::cout;
using std::endl;

namespace shining {

	class Grabber {
	public:
		Grabber();
		int open(const std::string& filename);
		AVCodecContext* getVideoCtx() { return vctx; }
		AVCodecContext* getAudioCtx() { return actx; }
		double getFrameRate() { return frameRate; }
		int getVideoIndex() { return videoIndex; }
		int getAudioIndex() { return audioIndex; }
		AVFrame* decode(AVPacket* pkt);
		int receiveFrame(AVFrame* frame);
		AVPacket* grab();

		AVFrame* grabAndDecode();

		AVPixelFormat getPixFormat() {
			if (!vctx) {
				return AV_PIX_FMT_NONE;
			}
			return vctx->pix_fmt;
		}

	private:
		AVFormatContext* fctx;
		AVCodecContext* vctx, * actx;
		int					videoIndex, audioIndex;
		double				frameRate;
	};
}