#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
    cPacket *sendPacketEvent;
    cStdDev transmissionStats;
public:
    Generator();
    virtual ~Generator();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};
Define_Module(Generator);

Generator::Generator() {
    sendPacketEvent = NULL;

}

Generator::~Generator() {
    cancelAndDelete(sendPacketEvent);
}

void Generator::initialize() {
    transmissionStats.setName("TotalTransmissions");
// create the send packet
    sendPacketEvent = new cPacket("sendPacket"); // Change packet name and data type
    sendPacketEvent->setByteLength(par("packetByteSize")); // Set byte length of the packet
    // schedule the first event at random time
    scheduleAt(par("generationInterval"), sendPacketEvent); 
}

void Generator::finish() {
}

void Generator::handleMessage(cMessage *msg) {

    // create new packet
    cPacket *pkt = new cPacket("packet"); // Change data type to cPacket*
    pkt->setByteLength(par("packetByteSize")); // Set byte length of the packet
    // send to the output
    send(pkt, "out");
    // compute the new departure time
    simtime_t departureTime = simTime() + par("generationInterval");
    // schedule the new packet generation
    scheduleAt(departureTime, sendPacketEvent);
}

#endif /* GENERATOR */
