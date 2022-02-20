#pragma once

#include <string>
#include <vector>
#include <map>



namespace shining {
	class VideoPacket
	{
	public:
		VideoPacket(uint8_t* data, uint16_t len, uint64_t timestamp, uint32_t seqNum);
		void setExpireTime(uint32_t expireTime);
	
		uint8_t* data;
		uint16_t len;
		uint64_t timestamp;
		uint32_t seqNum;
		uint32_t expireTime;
		uint32_t ssrc;
	};

	class JitterBuffer {
	public:
		void push(uint8_t* data, uint16_t len, uint64_t timestamp, uint32_t seqNum);
		void push(VideoPacket* pkt);
		void pop(std::vector<VideoPacket*>& data);
		uint32_t getLossSeqNum();
		void SetResenTime(uint16_t time) { resendTime = time; }

	private:
		std::map<uint32_t, VideoPacket*> dataMap{};
		uint32_t currSeq;

		uint32_t highestSeq;
		std::map<uint32_t, uint16_t> countMap{};
		//uint32_t lowestAckedSeq;
		uint16_t resendTime;
	};
}