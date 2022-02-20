#include <thread>
#include "config.h"
#include "seeker/loggerApi.h"
#include "packet-decoder.h"

namespace shining {
	PacketDecoder::PacketDecoder()
	{
		decoder = new Decoder();
		decoder->setPixFmt(AV_PIX_FMT_BGR24);
		decoder->init();
		Start();
	}
	void PacketDecoder::Start()
	{
		std::thread thread(&PacketDecoder::decode, this);
		thread.detach();
	}
	void PacketDecoder::push(std::vector<VideoPacket*>& pktVec)
	{
		std::unique_lock<std::mutex> lock(mtx);

		for (int i = 0; i < pktVec.size(); i++) {
			//decoder->push(pktVec[i]->data, pktVec[i]->len, pktVec[i]->timestamp);
			dataList.emplace_back(pktVec[i]);
		}
	}
	void PacketDecoder::decode()
	{
		while (1) {
			std::unique_lock<std::mutex> lock(mtx);
			if (dataList.empty()) {
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			auto pack = dataList.front();
			dataList.pop_front();
			lock.unlock();
			//std::cout << "pop front " << pack->seqNum << std::endl;
			decoder->push(pack->data, pack->len, pack->timestamp);
			int ret = 0;
			//while (ret >= 0)
			//{
				AVFrame* frame = av_frame_alloc();
				ret = decoder->poll(frame);
				//av_frame_free(&frame);

				if (ret >= 0) {
					//todo 
					I_LOG("decode a frame");
					//player->draw(frame);
				}
				else {
					av_frame_free(&frame);
				}
			//}
		}

	}
}