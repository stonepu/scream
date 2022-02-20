#include <iostream>
#include <sstream>
#include <thread>
#include "config.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include "screamreceiver.h"

namespace shining
{
	ScreamReceiver::ScreamReceiver() {
		m_screamRx = new ScreamRx(0);
		decoder = new Decoder();
		decoder->setPixFmt(AV_PIX_FMT_BGR24);
		decoder->init();
		jBuffer = new JitterBuffer();
		pDecoder = new PacketDecoder();

		m_running = true;
	}

	ScreamReceiver::~ScreamReceiver() {
		delete m_screamRx;
		delete decoder;
	}

	void ScreamReceiver::SetChannel(Channel* rtpChannel_, Channel* rtcpChannel_)
	{
		rtpChannel = rtpChannel_;
		rtcpChannel = rtcpChannel_;
	}
	void ScreamReceiver::SetJitBufferRensendTime(uint16_t time)
	{
		if (jBuffer) {
			jBuffer->SetResenTime(time);
		}
	}

	void ScreamReceiver::RecvPacket(uint8_t* data, int size)
	{
		//I_LOG("recv rtp msg, size: {}", size);
		int64_t now = seeker::Time::currentTime();
		uint64_t time_us = now * 1000;

		if (!m_running) { return; }
		RtpPacket rtp(data, size);
		uint32_t ssrc = rtp.ssrc;
		uint16_t seqNr = rtp.sequenceNum;
		uint32_t tmp = rtp.timestamp;
		uint64_t gap = now - tmp;
		//D_LOG("recv rtp msg, ssrc: {}, seqNr: {}", ssrc, seqNr);
		if (m_delayCb) { m_delayCb(gap); }
		if (displayRecvNum) {
			D_LOG("mrecv seq {}", rtp.sequenceNum);
		}
		if (useNack) {
			jBuffer->push(rtp.payload, rtp.payloadLen, time_us, seqNr);
			auto lossNum = jBuffer->getLossSeqNum();
			if (lossNum < 65536) {
				W_LOG("loss num {}", (int)lossNum);
				RtcpPacket rtcp;
				rtcp.type = RTCP_TYPE::nack;
				rtcp.lossSeq = lossNum;
				uint8_t* rtcpBuffer;
				int bufferSize = rtcp.toBuffer(&rtcpBuffer);
				Send(rtcpBuffer, bufferSize);
			}
			std::vector<VideoPacket*> pktVec{};
			jBuffer->pop(pktVec);
			if (displayRecvNum) {
				for (int i = 0; i < pktVec.size(); i++) {
					std::cout << "pop: " << pktVec[i]->seqNum << std::endl;
				}
			}

			for (int i = 0; i < pktVec.size(); i++) {
				auto pkt = pktVec[i];
				if (rtpCallback) {
					std::vector<uint32_t> ssrcVec{};
					ssrcVec.push_back(ssrc);
					rtpCallback(pkt->data, pkt->len, now, seqNr, ssrcVec, false);
				}
				else {
					W_LOG("rtp callback is empty");
				}
			}
			//pDecoder->push(pktVec);
		}
		else { // not use nack;
			//std::vector<VideoPacket*> pktVec{};
			//auto pktPtr = new VideoPacket(rtp.payload, rtp.payloadLen, time_us, seqNr);
			//pktPtr->ssrc = ssrc;
			//pktVec.emplace_back(pktPtr);
			//pDecoder->push(pktVec);

			if (rtpCallback) {
				std::vector<uint32_t> ssrcVec{};
				ssrcVec.push_back(ssrc);
				rtpCallback(rtp.payload, rtp.payloadLen, now, seqNr, ssrcVec, false);
			}
			else {
				W_LOG("rtp callback is empty");
			}
		}
		m_screamRx->receive(time_us, data, ssrc, size, seqNr);
		if (m_first) {
			StartSendFeedback();
			m_first = false;
		}
	}

	void ScreamReceiver::Process()
	{
		while (m_running) {
			int64_t now = seeker::Time::currentTime();
			uint64_t time_us = now * 1000;
			bool isFeedback = false;

			isFeedback |= m_screamRx->isFeedback(time_us);
			if (isFeedback)
			{
				uint32_t ssrc = 0;
				uint32_t rxTimestamp = 0;
				uint16_t aseqNr = 0;
				uint64_t aackVector = 0;
				uint16_t ecnCeMarkedBytes = 0;
				if (m_screamRx->getFeedback(time_us, ssrc, rxTimestamp, aseqNr, aackVector, ecnCeMarkedBytes))
				{
					RtcpPacket rtcp;
					rtcp.timeStamp = rxTimestamp;
					rtcp.ssrc = ssrc;
					rtcp.ackVector = aackVector;
					rtcp.ecnCeMarkedBytes = ecnCeMarkedBytes;
					rtcp.seqNr = aseqNr;
					rtcp.type = RTCP_TYPE::rr;
					uint8_t* rtcpBuffer;
					int rtcpBufferSize = rtcp.toBuffer(&rtcpBuffer);
					Send(rtcpBuffer, rtcpBufferSize);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(m_rtcpFbInterval));
		}
	}

	void ScreamReceiver::StartSendFeedback()
	{
		std::thread feedbackThread{ &ScreamReceiver::Process, this };
		feedbackThread.detach();
	}

	void ScreamReceiver::Send(uint8_t* data, int size)
	{
		if (rtcpChannel) {
			rtcpChannel->SendMsg(data, size);
			I_LOG("rtcp channel send msg");
		}
		else {
			E_LOG("rtp channel is null");
		}
	}
}
