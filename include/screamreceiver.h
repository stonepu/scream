#ifndef SCREAM_RECEIVER_H
#define SCREAM_RECEIVER_H
#include <functional>
#include "screamRx.h"
#include "decoder.h"
#include "converter.h"
#include "jitter-buffer.h"
#include "packet-decoder.h"
#include "media-packet.h"
#include "udpchannel.h"
namespace shining
{
class ScreamReceiver
{
public:
	ScreamReceiver();
	~ScreamReceiver();
	typedef std::function<void(uint8_t*, size_t, int64_t, uint16_t, std::vector<uint32_t>, bool)> RTP_CALLBACK;
	typedef std::function<void(uint64_t)> DelayCallback;
	void SetDelayCallback(DelayCallback cb){m_delayCb=cb;}
	void SetChannel(Channel* rtpChannel_, Channel* rtcpChannel_);
	void RecvPacket(uint8_t* data, int size);
	void SetRtpCallback(RTP_CALLBACK callback_) { rtpCallback = callback_; }


	void SetDisplayRecvLog(bool display) { displayRecvNum = display; }
	void SetUseNack(bool use) { useNack = use; }
	void SetUseFec(bool use) { useFec = use; }
	void SetJitBufferRensendTime(uint16_t time);
	void SetTest(bool test_) { isTest = test_; }
private:
	void Send(uint8_t* data, int size);
	void Process();
	void StartSendFeedback();
    uint16_t m_peerPort;
	uint64_t m_rtcpFbInterval{1};
	ScreamRx *m_screamRx;
	uint16_t m_port;
	bool  m_running{false};
	bool m_first{true};
	DelayCallback m_delayCb;

	Decoder* decoder;
	JitterBuffer* jBuffer;
	PacketDecoder* pDecoder;

	Channel* rtpChannel;
	Channel* rtcpChannel;
	RTP_CALLBACK rtpCallback;

	bool displayRecvNum = false;
	bool useNack = false;
	bool useFec = false;
	bool isTest = true;
};
}
#endif
