#include <math.h>
#include "config.h"
#include "seeker/loggerApi.h"
#include "jitter-buffer.h"

namespace shining {
	VideoPacket::VideoPacket(uint8_t* data, uint16_t len, uint64_t timestamp, uint32_t seqNum) : data(data), len(len), timestamp(timestamp), seqNum(seqNum)
	{
	}
	void VideoPacket::setExpireTime(uint32_t expireTime_)
	{
		expireTime = expireTime_;
	}
	void JitterBuffer::push(uint8_t* data, uint16_t len, uint64_t timestamp, uint32_t seqNum)
	{
		VideoPacket* pkt = new VideoPacket(data, len, timestamp, seqNum);
		push(pkt);
	}
	void JitterBuffer::push(VideoPacket* pkt)
	{
		//I_LOG("push {}", pkt->seqNum);
		dataMap[pkt->seqNum] = pkt;
		highestSeq = std::max(pkt->seqNum, highestSeq);
		highestSeq %= 65536;
	}
	void JitterBuffer::pop(std::vector<VideoPacket*>& dataVec)
	{
		while (dataMap[currSeq]) {
			dataVec.emplace_back(dataMap[currSeq]);
			dataMap.erase(currSeq);
			//lowestAckedSeq = std::max(lowestAckedSeq, currSeq);
			currSeq++;
			currSeq %= 65536;
		}
	}

	uint32_t JitterBuffer::getLossSeqNum()
	{
		int temp = currSeq;
		//I_LOG("temp before check: {}", temp );
		while (dataMap.find(temp) != dataMap.end() && dataMap.find(temp)->second != 0 && temp < highestSeq) {
			++temp;
		}
		//I_LOG("temp: {}, hi: {}", temp, highestSeq);
		if (temp < highestSeq) {

			if (countMap.find(temp) == countMap.end()) {
				countMap[temp] = 1;
			}
			else {
				if (countMap[temp] >= resendTime) {
					while ((dataMap.find(temp) == dataMap.end() || dataMap.find(temp)->second == 0) && temp < highestSeq) {
						I_LOG("loss num {}", temp);
						++temp;
					}
					currSeq = temp;
					return 65536;
				}
				else {
					countMap[temp]++;
				}
			}
			return temp;
		}
		return 65536;
	}

}