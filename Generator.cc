#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
    cMessage *sendMessageEvent;

    cStdDev transmissionStats;
    cOutVector packetTxVector; // transmitted packet count

    simtime_t newDepartureTime();
    void scheduleSendPacket(simtime_t departureTime);
    void sendPacket();
public:
    Generator();
    virtual ~Generator();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *message);
};
Define_Module(Generator);

Generator::Generator() {
    sendMessageEvent = NULL;
}

Generator::~Generator() {
    cancelAndDelete(sendMessageEvent);
}

simtime_t Generator::newDepartureTime() {
    return simTime() + par("generationInterval");
}

void Generator::scheduleSendPacket(simtime_t departureTime) {
    scheduleAt(departureTime, sendMessageEvent);
}

void Generator::sendPacket() {
    cPacket *packet = new cPacket("packet");
    packet->setByteLength(par("packetByteSize"));
    send(packet, "out");

    packetTxVector.record(1);
}

void Generator::initialize() {
    transmissionStats.setName("TotalTransmissions");
    packetTxVector.setName("packetsTransmitted");

    sendMessageEvent = new cMessage("sendEvent");
    scheduleSendPacket(par("generationInterval"));
}

void Generator::finish() {
}

void Generator::handleMessage(cMessage *message) {
    sendPacket();
    scheduleSendPacket(newDepartureTime());
}

#endif
