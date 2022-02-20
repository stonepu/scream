#include "config.h"
#include "seeker/loggerApi.h"
#include "media-packet.h"
#pragma comment(lib,"ws2_32.lib")

namespace shining {
    RtpPacket::RtpPacket(uint8_t* data, uint16_t size)
    {
        uint8_t* lineArr = new uint8_t[10];
        for (int i = 0; i < 10; i++) {
            lineArr[i] = data[i];
        }
        //D_LOG("recv side: {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, ", lineArr[0], lineArr[1], lineArr[2], lineArr[3], lineArr[4], lineArr[5], lineArr[6], lineArr[7], lineArr[8], lineArr[9]);
        delete[] lineArr;
        uint16_t tmp_s;
        uint32_t tmp_l;
        memcpy(&tmp_s, data, 2);
        rtpInfo = ntohs(tmp_s);
        memcpy(&tmp_s, data + 2, 2);
        sequenceNum = ntohs(tmp_s);
        //I_LOG("recv side, recv num: {}, transferNum: {}, size: {}", tmp_s, sequenceNum, size);
        memcpy(&tmp_l, data + 4, 4);
        timestamp = ntohs(tmp_l);

        memcpy(&tmp_l, data + 8, 4);
        ssrc = tmp_l;

        payloadLen = size - 12;
        payload = new uint8_t[payloadLen];
        memcpy(payload, data + 12, payloadLen);
    }

    int RtpPacket::toBuffer(uint8_t** buffer)
    {
        //I_LOG("rtp 2 buffer");
        (*buffer) = new uint8_t[payloadLen + 12];

        uint16_t temps;
        uint32_t templ;
        //temps = htons(rtpInfo);
        temps = rtpInfo;
        memcpy(*buffer, &temps, 2);
        temps = htons(sequenceNum);
        //temps = sequenceNum;
        //I_LOG("send side, sqn: {}, transfer: {}, size: {}", sequenceNum, temps, payloadLen + 12);
        memcpy((*buffer) + 2, &temps, 2);
        templ = htonl(timestamp);
        memcpy((*buffer) + 4, &templ, 4);
        templ = htonl(ssrc);
        memcpy((*buffer) + 8, &templ, 4);
        memcpy((*buffer) + 12, payload, payloadLen);
        uint8_t* lineArr = new uint8_t[10];
        for (int i = 0; i < 10; i++) {
            lineArr[i] = (*buffer)[i];
        }
        //D_LOG("send side: {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, ", lineArr[0], lineArr[1], lineArr[2], lineArr[3], lineArr[4], lineArr[5], lineArr[6], lineArr[7], lineArr[8], lineArr[9]);
        delete[] lineArr;
        return payloadLen + 12;
    }

    RtcpPacket::RtcpPacket(uint8_t* data, uint16_t size)
    {
        uint8_t type_ = data[9];
        if (type_ == 0x00) {
            type = RTCP_TYPE::rr;
        }
        else if (type_ == 0x01) {
            type = RTCP_TYPE::nack;
        }
        uint16_t tmp_s;
        uint32_t tmp_l;
        uint64_t tmp_ll;
        if (type == RTCP_TYPE::rr) {
            memcpy(&tmp_l, data + 28, 4);
            timeStamp = ntohl(tmp_l);
            memcpy(&tmp_s, data + 16, 2);
            seqNr = ntohs(tmp_s);
            memcpy(&tmp_l, data + 20, 4);
            ackVector |= ((uint64_t)ntohl(tmp_l)) << 32;
            memcpy(&tmp_l, data + 24, 4);
            ackVector |= ntohl(tmp_l);
            memcpy(&tmp_s, data + 18, 2);
            ecnCeMarkedBytes = ntohs(tmp_s);
            memcpy(&tmp_l, data + 12, 4);
            ssrc = ntohl(tmp_l);
        }
        else if (type == RTCP_TYPE::nack) {
            memcpy(&tmp_s, data + 32, 2);
            lossSeq = ntohs(tmp_s);
        }


    }

    int RtcpPacket::toBuffer(uint8_t** buf)
    {
        //I_LOG("rtcpPacket to buffer");
        (*buf) = new uint8_t[34];
        uint16_t tmp_s;
        uint32_t tmp_l;
        (*buf)[0] = 0x80;
        (*buf)[1] = 207;
        uint16_t t = 0x06;
        //tmp_s = (uint16_t)htons(t);
        tmp_s = 1536;
        memcpy((*buf) + 2, &tmp_s, 2);
        tmp_l = htonl(ssrc);
        memcpy((*buf) + 4, &tmp_l, 4);
        (*buf)[8] = 0xFF; // BT=255
        if (type == (uint8_t)RTCP_TYPE::rr) {
            (*buf)[9] = 0x00;
        }
        else if (type == (uint8_t)RTCP_TYPE::nack) {
            (*buf)[9] = 0x01;
        }
        tmp_s = htons(4);
        memcpy((*buf) + 10, &tmp_s, 2);
        tmp_l = htonl(ssrc);
        memcpy((*buf) + 12, &tmp_l, 4);
        tmp_s = htons(seqNr);
        memcpy((*buf) + 16, &tmp_s, 2);
        tmp_s = htons(ecnCeMarkedBytes);
        memcpy((*buf) + 18, &tmp_s, 2);
        tmp_l = uint32_t((ackVector >> 32) & 0xFFFFFFFF);
        tmp_l = htonl(tmp_l);
        memcpy((*buf) + 20, &tmp_l, 4);
        tmp_l = uint32_t(ackVector & 0xFFFFFFFF);
        tmp_l = htonl(tmp_l);
        memcpy((*buf) + 24, &tmp_l, 4);
        tmp_l = htonl(timeStamp);
        memcpy((*buf) + 28, &tmp_l, 4);
        tmp_s = htons(lossSeq);
        memcpy((*buf) + 32, &tmp_s, 2);
        return 34;
    }
}
