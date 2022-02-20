#include <chrono>
#include "config.h"
#include "seeker/loggerApi.h"
#include <thread>
#include "screamsender.h"
#include "seeker/common.h"
#include "media-packet.h"
namespace shining
{
	static int64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	static bool usingAlgorithm = true;
	ScreamSender::ScreamSender()
	{
		m_running = true;
		m_screamTx = new ScreamTx();
		m_rtpQueue = new RtpQueue();
		m_videoEnc = nullptr;
		m_nextFrameTime = 0;
		m_ssrc = 1234;
	}
	ScreamSender::~ScreamSender()
	{
		delete m_screamTx;
		delete m_rtpQueue;
		delete m_videoEnc;
	}

	void ScreamSender::Start()
	{
		std::thread sendThread{ &ScreamSender::Process, this };
		sendThread.detach();
	}

	void ScreamSender::Stop()
	{
		m_running = false;
	}

	void ScreamSender::SetTraceFilePath(char* trace)
	{
		m_videoEnc = new VideoEnc(m_rtpQueue, m_frameRate, trace, 0, isTest);
		m_screamTx->registerNewStream(m_rtpQueue, m_ssrc, 1.0f, 256e3f, 2 * 1024e3f, 4 * 8192e3f, 2e6f);//, 1e6f, 0.2f, 0.1f, 0.1f);
	}
	void ScreamSender::SetChannel(Channel* rtpChannel_, Channel* rtcpChannel_)
	{
		rtpChannel = rtpChannel_;
		rtcpChannel = rtcpChannel_;
	}

	static int lossTest = 0;
	static bool testLoss = true;
	void ScreamSender::Send(uint8_t* data, int size, bool resend)
	{
		if (!resend) {
			if (testLoss) {
				++lossTest;
				if (lossTest % 20 == 0) {
					lossTest = 0;
					//I_LOG("not send");
					return;
				}
				else {
					//I_LOG("send packet {}", lossTest);
				}
			}
		}
		if (rtpChannel) {
			rtpChannel->SendMsg(data, size);
			//I_LOG("rtp channel send msg");
		}
		else {
			E_LOG("rtp channel is null");
		}
	}

	void ScreamSender::Process()
	{
		I_LOG("start sending");
		while (m_running) {
			int64_t now = seeker::Time::currentTime();
			float time_s = 1.0 * now / 1e3;
			uint64_t time_us = now * 1000;
			float retVal = -1.0;
			//I_LOG("now: {}, m_nextFrame_time: {}", now, m_nextFrameTime);
			if (now >= m_nextFrameTime)//generate a frame
			{
				m_nextFrameTime = now + m_videoTick;
				if (currFrameIndex >= GOP) {
					m_videoEnc->setTargetBitrate(m_screamTx->getTargetBitrate(m_ssrc));
					currFrameIndex = 0;
				}
				else {
					currFrameIndex++;
				}
				int bytes = m_videoEnc->encode(time_s);
				m_screamTx->newMediaFrame(time_us, m_ssrc, bytes);
				retVal = m_screamTx->isOkToTransmit(time_us, m_ssrc);
			}
			if (now >= m_nextCallN && retVal != 0.0f)
			{
				retVal = m_screamTx->isOkToTransmit(time_us, m_ssrc);
				if (retVal > 0)
				{
					if (usingAlgorithm) {
						m_nextCallN = now + retVal;
						I_LOG("ret {}", retVal);
					}
					else {
						retVal = 0;
					}
				}
			}
			if (printRetVal) {
				I_LOG("retVal: {}", retVal);
			}
			//retVal = 0;		//直接发送，不需要算法控制
			if (retVal == 0) {
				/*
				* RTP packet can be transmitted
				*/
				void* rtpPayload = 0;
				int size;
				uint16_t seqNr;
				auto sendPack = m_rtpQueue->sendPacket(&rtpPayload, size, seqNr);
				//std::cout << "send seq: " << seqNr << std::endl;
				if (sendPack) {
					//std::cout << now << " send packet: " << std::endl;
					retVal = m_screamTx->addTransmitted(time_us, m_ssrc, size, seqNr);
					m_nextCallN = now + retVal;
					//std::cout << "next interval: " << retVal << std::endl;
					RtpPacket rtpPacket;
					rtpPacket.payload = (uint8_t*)rtpPayload;
					rtpPacket.payloadLen = size;
					rtpPacket.sequenceNum = seqNr;
					rtpPacket.ssrc = m_ssrc;
					//D_LOG("send ssrc: {}", m_ssrc);
					rtpPacket.timestamp = (uint32_t)now;
					uint8_t* buffer;
					int size = rtpPacket.toBuffer(&buffer);
					Send(buffer, size, false);
					//delete buffer;

				}
				//D_LOG("{} send {}", now, seqNr);
			}
			else {
				//D_LOG("not send, retval = {}", retVal);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(m_tick));
		}
		I_LOG("stop sream sender");
	}

	void ScreamSender::RecvPacket(uint8_t* data, int size)
	{
		//I_LOG("recv rtcp msg, size: {}", size);
		RtcpPacket rtcpPacket(data, size);
		int64_t now = seeker::Time::currentTime();
		int64_t time_us = now * 1000;
		uint32_t rxTimestamp = rtcpPacket.timeStamp;
		uint16_t aseq = rtcpPacket.seqNr;
		uint64_t ack_vec = rtcpPacket.ackVector;
		auto ecn_bytes = rtcpPacket.ecnCeMarkedBytes;
		if (rtcpPacket.type == RTCP_TYPE::rr) {
			//D_LOG("recv rtcp aseq: {}", aseq);
			m_screamTx->incomingFeedback(time_us, m_ssrc, rxTimestamp, aseq, ack_vec, ecn_bytes);
		}
		else {
			uint16_t nackSeq = rtcpPacket.lossSeq;
			I_LOG("loss seqNum: {}", rtcpPacket.lossSeq);
			uint8_t* nackData;
			int nackLen = 0;
			m_rtpQueue->popNackPacket((void**)&nackData, nackLen, nackSeq);
			if (nackLen <= 0) {
				return;
			}
			RtpPacket rtpPacket;
			rtpPacket.payload = (uint8_t*)nackData;
			rtpPacket.payloadLen = nackLen;
			rtpPacket.sequenceNum = nackSeq;
			rtpPacket.ssrc = m_ssrc;
			rtpPacket.timestamp = (uint32_t)now;
			uint8_t* nackBuffer;
			int nackBufLen = rtpPacket.toBuffer(&nackBuffer);
			I_LOG("resend, seq {}", rtpPacket.sequenceNum);
			Send(nackBuffer, nackBufLen, true);
		}
	}
}

