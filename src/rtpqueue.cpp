#include "rtpqueue.h"
#include <iostream>
#include <string.h>
/*
* Implements a simple RTP packet queue
*/

const int RtpSize = 1500;
namespace shining
{
    using namespace std;
    RtpQueueItem::RtpQueueItem() {
        packet = 0;
        used = false;
        size = 0;
        seqNr = 0;
    }


    RtpQueue::RtpQueue() {
        for (int n = 0; n < RtpQueueSize; n++) {
            items[n] = new RtpQueueItem();
        }
        head = -1;
        tail = 0;
        nItems = 0;
        sizeOfLastFrame = 0;
        bytesInQueue_ = 0;
        sizeOfQueue_ = 0;
        sizeOfNextRtp_ = -1;
    }

    void RtpQueue::push(void* rtpPacket, int size, unsigned short seqNr, float ts) {
        head++; if (head == RtpQueueSize) head = 0;
        items[head]->packet = rtpPacket;
        items[head]->seqNr = seqNr;
        items[head]->size = size;
        items[head]->ts = ts;
        items[head]->used = true;
        bytesInQueue_ += size;
        sizeOfQueue_ += 1;
        computeSizeOfNextRtp();
    }

    bool RtpQueue::pop(void** rtpPacket, int& size, unsigned short& seqNr) {
        if (items[tail]->used == false) {
            sizeOfNextRtp_ = -1;
            return false;
        }
        else {
            *rtpPacket = items[tail]->packet;
            size = items[tail]->size;
            seqNr = items[tail]->seqNr;
            items[tail]->used = false;
            RtpQueueItem* item = new RtpQueueItem();
            item->packet = items[tail]->packet;
            item->size = items[tail]->size;
            item->seqNr = items[tail]->seqNr;
            uint16_t seqNum = items[tail]->seqNr;
            dataMap[seqNum] = item;
            tail++; if (tail == RtpQueueSize) tail = 0;
            bytesInQueue_ -= size;
            sizeOfQueue_ -= 1;
            computeSizeOfNextRtp();
            return true;
        }
    }

    void RtpQueue::computeSizeOfNextRtp() {
        if (!items[tail]->used) {
            sizeOfNextRtp_ = -1;
        }
        else {
            sizeOfNextRtp_ = items[tail]->size;
        }
    }

    void RtpQueue::popNackPacket(void** rtpPacket, int& size, unsigned short seqNr)
    {
        //std::cout << "seq: " << seqNr << std::endl;
        auto pkt = dataMap[seqNr];
        if (pkt) {
            *rtpPacket = dataMap[seqNr]->packet;
            size = dataMap[seqNr]->size;
        }
        else {
            //std::cout << "pkt is null" << std::endl;
        }

        //seqNr = dataMap[seqNr]->seqNr;
    }

    int RtpQueue::sizeOfNextRtp() {
        return sizeOfNextRtp_;
    }

    int RtpQueue::seqNrOfNextRtp() {
        if (!items[tail]->used) {
            return -1;
        }
        else {
            return items[tail]->seqNr;
        }
    }

    int RtpQueue::bytesInQueue() {
        return bytesInQueue_;
    }

    int RtpQueue::sizeOfQueue() {
        return sizeOfQueue_;
    }

    float RtpQueue::getDelay(float currTs) {
        if (items[tail]->used == false) {
            return 0;
        }
        else {
            return currTs - items[tail]->ts;
        }
    }

    bool RtpQueue::sendPacket(void** rtpPacket, int& size, unsigned short& seqNr) {
        if (sizeOfQueue() > 0) {
            return pop(rtpPacket, size, seqNr);
        }
        return false;
    }

    void RtpQueue::clear() {
        for (int n = 0; n < RtpQueueSize; n++) {
            items[n]->used = false;
        }
        head = -1;
        tail = 0;
        nItems = 0;
        bytesInQueue_ = 0;
        sizeOfQueue_ = 0;
        sizeOfNextRtp_ = -1;
    }

    int RtpQueue::size() {
        return head - tail;
    }
    void RtpQueue::deleteAckedData(unsigned short seqNum)
    {
        if (dataMap.find(seqNum) != dataMap.end()) {
            auto result = dataMap.find(seqNum);
            delete result->second;
            dataMap.erase(seqNum);
        }
    }
}
