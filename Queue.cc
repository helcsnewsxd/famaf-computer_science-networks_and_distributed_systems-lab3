#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;

    cOutVector bufferSizeVector;
    cOutVector packetDropVector;

    void scheduleSendPacketWithDelay(simtime_t time);
    void sendPacket();
    void addPacket(cMessage *message);
    bool isFullQueue();
    void activeQueue();
public:
    Queue();
    virtual ~Queue();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *message);
};
Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetsDropped");

    endServiceEvent = new cMessage("endService");
}

void Queue::finish() {
}

void Queue::scheduleSendPacketWithDelay(simtime_t delay) {
    scheduleAt(simTime() + delay, endServiceEvent);
}

void Queue::sendPacket() { // Only if there is any packet
    if (!buffer.isEmpty()) {
        cPacket *packet = (cPacket*) buffer.pop();
        send(packet, "out");
        scheduleSendPacketWithDelay(packet->getDuration());
    }
}

void Queue::addPacket(cMessage *message) {
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

bool Queue::isFullQueue() {
    return buffer.getLength() >= par("bufferSize").intValue();
}

void Queue::activeQueue() {
    if (!endServiceEvent->isScheduled()) { // Queue is inactive
        scheduleSendPacketWithDelay(0);
    }
}

void Queue::handleMessage(cMessage *message) {
    if (message == endServiceEvent) {
        sendPacket();
    } else {
        addPacket(message);
    }
}

#endif
