//#pragma once
//
//#include <iostream>
//
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//
//extern "C"
//{
//#include <libavutil/frame.h>
//#include <libavcodec/avcodec.h>
//#include <libswscale/swscale.h>
//#include <libavutil/imgutils.h>
//}
//
//
//namespace ns3 {
//	class FrameConverter {
//	public:
//		FrameConverter();
//		~FrameConverter();
//		cv::Mat* transferFrame(AVFrame* inputFrame, AVPixelFormat fmt);
//		cv::Mat* convert(int width, int height, SwsContext* img_convert_ctx, AVFrame* pFrame);
//
//
//		AVFrame* transfer2YUV(AVFrame* inputFrame, AVPixelFormat fmt);
//
//		cv::Mat frame2Mat(AVFrame* frame);
//
//	private:
//		SwsContext* swsCtx = nullptr;
//	};
//}