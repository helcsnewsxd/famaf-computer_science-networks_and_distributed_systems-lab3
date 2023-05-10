#ifndef SINK
#define SINK

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Sink : public cSimpleModule {
private:
    cStdDev delayStats;
    cOutVector delayVector;

    void computeStats(cMessage *message);
public:
    Sink();
    virtual ~Sink();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *message);
};
Define_Module(Sink);

Sink::Sink() {
}

Sink::~Sink() {
}

void Sink::initialize(){
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
}

void Sink::finish(){
    recordScalar("Avg delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
}

void Sink::computeStats(cMessage *message) {
    simtime_t queuingDelay = simTime() - message->getCreationTime();
    delayStats.collect(queuingDelay);
    delayVector.record(queuingDelay);
}

void Sink::handleMessage(cMessage *message) {
    computeStats(message);
    delete(message);
}

#endif
