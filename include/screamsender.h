#ifndef SCREAM_SENDER_H
#define SCREAM_SENDER_H
//#include "ns3/simulator.h"
//#include "ns3/socket.h"
//#include "ns3/network-module.h"
//#include "ns3/internet-module.h"
//#include "ns3/event-id.h"
#include "rtpqueue.h"
#include "videoenc.h"
#include "screamTx.h"
#include "udpchannel.h"
namespace shining
{
class ScreamSender
{
public:
	ScreamSender();
	~ScreamSender();
	void Start();
	void Stop();
	void SetTraceFilePath(char*trace);
	//void InitialSetup(Ipv4Address dest_ip,uint16_t dest_port);
	void SetChannel(Channel* rtpChannel_, Channel* rtcpChannel_);
	void RecvPacket(uint8_t* data, int size);

	VideoEnc* GetEncoder(){return m_videoEnc;}
	void SetPrintRetVal(bool print) { printRetVal = print; }
	void SetTest(bool test_) { isTest = test_; }
	void SetSsrc(uint32_t ssrc_) { m_ssrc = ssrc_; }
	void SetCwndCallback(CwndCallback callback) {
		m_screamTx->SetCwndCallback(callback);
	}
private:
	//void RecvPacket(Ptr<Socket>socket);
	//virtual void StartApplication();
	//virtual void StopApplication();
	//void Send(Ptr<Packet>packet);
	void Send(uint8_t* data, int size, bool resend);
	void Process();
    //Ipv4Address m_peerIp;
    uint16_t m_peerPort;
    //Ptr<Socket> m_socket;
	Channel* rtpChannel;
	Channel* rtcpChannel;
	int64_t m_videoTick{30};//40ms;
	int64_t m_nextFrameTime{0};
	int64_t m_tick{2};//5ms;
	float m_frameRate{25};
	RtpQueue *m_rtpQueue;
	VideoEnc *m_videoEnc;
	ScreamTx *m_screamTx;
	bool m_running = true;
	uint32_t m_ssrc;
	int m_nextCallN{-1};
	int GOP{ 0 };
	int currFrameIndex{ 0 };

	bool printRetVal = false;
	bool isTest = false;
};
}
#endif
