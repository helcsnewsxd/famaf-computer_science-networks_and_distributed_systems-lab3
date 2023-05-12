#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include "ControlPacket_m.h"

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportRx: public cSimpleModule {
private:
    cQueue buffer, bufferControl;
    cMessage *endServiceEvent, *endControlServiceEvent;

    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    cOutVector controlPacketSentVector;
    cOutVector packetReceive;

    void sendControlPacket();

    void scheduleSendControlPacketWithDelay(simtime_t delay);
    void addControlPacket(cMessage *message);

    bool isFullControlQueue();


    void sendDataPacket();

    void scheduleSendPacketWithDelay(simtime_t delay);
    void addPacket(cMessage *message);

    bool isFullQueue();
    bool haveSpaceQueue();

    void activeControlQueue();
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
    endControlServiceEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
    cancelAndDelete(endControlServiceEvent);
}

void TransportRx::initialize() {
    buffer.setName("buffer");
    bufferControl.setName("bufferControl");
    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetsDropped");
    controlPacketSentVector.setName("controlPacketSent");
    packetReceive.setName("packetsReceived");

    endServiceEvent = new cMessage("endService");
    endControlServiceEvent = new cMessage("endControlService");
}

void TransportRx::finish() {
}

void TransportRx::sendControlPacket() {
    if (!bufferControl.isEmpty()) {
        ControlPacket *controlPacket = (ControlPacket *) bufferControl.pop();
        send(controlPacket, "toOut$o");
        controlPacketSentVector.record(1);

        scheduleSendControlPacketWithDelay(controlPacket->getDuration());
    }
}

void TransportRx::scheduleSendControlPacketWithDelay(simtime_t delay) {
    scheduleAt(simTime() + delay, endControlServiceEvent);
}

void TransportRx::addControlPacket(cMessage *message) {
    if (!isFullControlQueue()) {
        ControlPacket* controlPacket = new ControlPacket("control packet");
        controlPacket->setByteLength(20);
        controlPacket->setKind(2);
        controlPacket->setTotalBuffer(par("bufferSize").intValue());
        controlPacket->setRemainingBuffer(par("bufferSize").intValue() - buffer.getLength());
        controlPacket->setTimeElapsedToReceivePacket(message->getArrivalTime() - message->getCreationTime());

        bufferControl.insert(controlPacket);

        activeControlQueue();
    }
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
        packetReceive.record(1);

        addControlPacket(message);
        activeQueue();
    }
}

bool TransportRx::isFullControlQueue() {
    return bufferControl.getLength() >= par("bufferSize").intValue();
}

bool TransportRx::isFullQueue() {
    return buffer.getLength() >= par("bufferSize").intValue();
}

bool TransportRx::haveSpaceQueue() {
    return buffer.getLength()*2 <= par("bufferSize").intValue();
}

void TransportRx::activeControlQueue() {
    if (!endControlServiceEvent->isScheduled()) {
        scheduleSendControlPacketWithDelay(0);
    }
}

void TransportRx::activeQueue() {
    if (!endServiceEvent->isScheduled()) {
        scheduleSendPacketWithDelay(0);
    }
}

void TransportRx::handleMessage(cMessage *message) {
    if (message == endControlServiceEvent) {
        sendControlPacket();
    } else if (message == endServiceEvent) {
        sendDataPacket();
    } else {
        addPacket(message);
    }
}

#endif
