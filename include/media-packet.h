#pragma once

#include <WinSock2.h>
#include <string>

namespace shining {

	class MediaPacket {
	public:
		MediaPacket() = default;

		virtual int toBuffer(uint8_t** out) = 0;
	};

	class RtpPacket : public MediaPacket {
	public:
		RtpPacket() = default;

		RtpPacket(uint8_t* data, uint16_t size);

		uint16_t rtpInfo = 0x80;
		uint16_t sequenceNum;
		uint32_t timestamp;
		uint32_t ssrc;
		uint8_t* payload;
		uint16_t payloadLen;

		int toBuffer(uint8_t** buffer) override;
	};

	/*
	* Create feedback according to the format below. It is up to the
	* wrapper application to prepend this RTCP with SR or RR when needed
	* BT = 255, means that this is experimental use
	* The code currently only handles one SSRC source per IP packet
	*
	* 0                   1                   2                   3
	* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |V=2|P|reserved |   PT=XR=207   |           length=6            |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |                              SSRC                             |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |     BT=255    |    reserved   |         block length=4        |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |                        SSRC of source                         |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* | Highest recv. seq. nr. (16b)  |         ECN_CE_bytes          |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |                     Ack vector (b0-31)                        |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |                     Ack vector (b32-63)                       |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	* |                    Timestamp (32bits)                         |
	* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/

	enum RTCP_TYPE {
		rr = 1,
		nack = 2,
	};

	class RtcpPacket : public MediaPacket {
	public:
		RtcpPacket() = default;
		RtcpPacket(uint8_t* data, uint16_t size);
		uint8_t type;
		uint32_t timeStamp;
		uint16_t seqNr;
		uint64_t ackVector;
		uint16_t ecnCeMarkedBytes;
		uint32_t ssrc;
		uint16_t lossSeq;

		int toBuffer(uint8_t** buffer) override;
	};
}