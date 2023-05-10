#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include "ControlPacket_m.h"

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportRx: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;

    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector controlPacketSentVector;

    void checkBufferStatus();

    void sendControlPacket();
    void sendDataPacket();

    void scheduleSendPacketWithDelay(simtime_t delay);
    void addPacket(cMessage *message);

    bool isFullQueue();

    void activeQueue();

public:
    TransportRx();
    virtual ~TransportRx();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *message);
};
Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
}

void TransportRx::initialize() {
    buffer.setName("buffer");
    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetsDropped");
    controlPacketSentVector.setName("controlPacketSent");

    endServiceEvent = new cMessage("endService");
}

void TransportRx::finish() {
}

void TransportRx::checkBufferStatus() {
    if (rand()%101==0) { // A condition to send controlPackets
        sendControlPacket();
    }
}

void TransportRx::sendControlPacket() {
    ControlPacket* controlPacket = new ControlPacket("control packet");
    controlPacket->setByteLength(20);
    controlPacket->setKind(2);
    controlPacket->setTotalBuffer(par("bufferSize").intValue());
    controlPacket->setRemainingBuffer(par("bufferSize").intValue() - buffer.getLength());
    send(controlPacket, "toOut$o");

    controlPacketSentVector.record(1);
}

void TransportRx::sendDataPacket() { // Only if there is any packet
    if (!buffer.isEmpty()) {
        cPacket *packet = (cPacket*) buffer.pop();
        send(packet, "toApp");
        scheduleSendPacketWithDelay(packet->getDuration());
    }
}

void TransportRx::scheduleSendPacketWithDelay(simtime_t delay) {
    scheduleAt(simTime() + delay, endServiceEvent);
}

void TransportRx::addPacket(cMessage *message) {
    if (isFullQueue()) {
        // Drop packet
        delete(message);
        this->bubble("packet dropped");
        packetDropVector.record(1);

    } else {
        // Add packet
        buffer.insert(message);
        bufferSizeVector.record(buffer.getLength());

        activeQueue();
    }
}

bool TransportRx::isFullQueue() {
    return buffer.getLength() >= par("bufferSize").intValue();
}

void TransportRx::activeQueue() {
    if (!endServiceEvent->isScheduled()) { // TransportRx is inactive
        scheduleSendPacketWithDelay(0);
    }
}

void TransportRx::handleMessage(cMessage *message) {
    if (message == endServiceEvent) {
        sendDataPacket();
    } else {
        addPacket(message);
    }

    checkBufferStatus();
}

#endif
