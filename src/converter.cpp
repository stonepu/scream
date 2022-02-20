//#include "converter.h"
//
//namespace ns3 {
//    FrameConverter::FrameConverter() {
//        //init;
//    }
//
//    FrameConverter::~FrameConverter() {
//        //release'
//    }
//
//    cv::Mat* FrameConverter::transferFrame(AVFrame* inputFrame, AVPixelFormat fmt) {
//        swsCtx = sws_getCachedContext(nullptr, inputFrame->width, inputFrame->height, fmt, inputFrame->width,
//            inputFrame->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
//        return convert(inputFrame->width, inputFrame->height, swsCtx, inputFrame);
//    }
//
//    AVFrame* FrameConverter::transfer2YUV(AVFrame* inputFrame, AVPixelFormat fmt) {
//        swsCtx = sws_getCachedContext(nullptr, inputFrame->width, inputFrame->height, fmt, inputFrame->width,
//            inputFrame->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
//
//        int frameSize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, inputFrame->width, inputFrame->height, 2);
//        auto* outBuffer = (uint8_t*)av_malloc(frameSize * sizeof(uint8_t));
//
//        auto frameYUV = av_frame_alloc();
//        av_image_fill_arrays(frameYUV->data, frameYUV->linesize, outBuffer,
//            AV_PIX_FMT_YUV420P, inputFrame->width, inputFrame->height, 2);
//        frameYUV->format = AV_PIX_FMT_YUV420P;
//        frameYUV->width = inputFrame->width;
//        frameYUV->height = inputFrame->height;
//
//        int ret = sws_scale(swsCtx, inputFrame->data, inputFrame->linesize, 0, inputFrame->height, frameYUV->data, frameYUV->linesize);
//        if (ret <= 0) {
//            std::cout << "error" << std::endl;
//            av_frame_free(&frameYUV);
//            return nullptr;
//        }
//        //av_frame_free(&inputFrame);
//        return frameYUV;
//    }
//
//    cv::Mat FrameConverter::frame2Mat(AVFrame* frame)
//    {
//        int width = frame->width;
//        int height = frame->height;
//        cv::Mat pCvMat;       
//        if (pCvMat.empty())
//        {
//            pCvMat.create(cv::Size(width, height), CV_8UC3);
//        }
//        int size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 2);
//        memcpy(pCvMat.data, frame->data[0], size);
//        return pCvMat;
//    }
//
//    cv::Mat* FrameConverter::convert(int width, int height, SwsContext* img_convert_ctx, AVFrame* pFrame)
//    {
//        cv::Mat* pCvMat = new cv::Mat();
//        if (pCvMat->empty())
//        {
//            pCvMat->create(cv::Size(width, height), CV_8UC3);
//        }
//
//        AVFrame* pFrameRGB = av_frame_alloc();
//        uint8_t* out_bufferRGB = NULL;
//        //给pFrameRGB帧加上分配的内存;
//        int size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 2);
//        out_bufferRGB = new uint8_t[size];
//        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_bufferRGB, AV_PIX_FMT_BGR24, width, height, 2);
//
//        //YUV to BGR
//        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, height, pFrameRGB->data, pFrameRGB->linesize);
//
//        memcpy(pCvMat->data, out_bufferRGB, size);
//
//        delete[] out_bufferRGB;
//        av_free(pFrameRGB);
//        return pCvMat;
//    }
//}