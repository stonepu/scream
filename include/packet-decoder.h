#pragma once

#include <mutex>


#include "jitter-buffer.h"
#include "decoder.h"

namespace shining {
	class PacketDecoder {
	public:
		PacketDecoder();
		void Start();
		void push(std::vector<VideoPacket*>& vec);
		void decode();

	private:

		std::list<VideoPacket*> dataList{};

		Decoder* decoder;
		std::mutex mtx{};

	};
}