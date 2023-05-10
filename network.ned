simple Generator
{
    parameters:
        volatile double generationInterval;
        volatile int packetByteSize;
        @display("i=block/source");
    gates:
        output out;
}

simple TransportTx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

simple Queue
{
    parameters:
        volatile int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple TransportRx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        traTx: TransportTx {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> traTx.toApp;

        in --> traTx.toOut$i;
        traTx.toOut$o --> out;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        traRx: TransportRx {
            parameters:
                @display("p=75,190");
        }
    connections:
        // Case study 1
        traRx.toApp --> { datarate = 0.5Mbps; } --> sink.in;
        // Case study 2
        // traRx.toApp --> { datarate = 1Mbps; } --> sink.in;

        in --> traRx.toOut$i;
        traRx.toOut$o --> out;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,30");
        }
        queue0: Queue {
            @display("p=130,30");
        }
        queue1: Queue {
            @display("p=130,100");
        }
        nodeRx: NodeRx {
            @display("p=230,30");
        }
    connections:
        nodeTx.out --> { datarate = 1Mbps; delay = 100us; } --> queue0.in;

        // Case study 1
        queue0.out --> { datarate = 1Mbps; delay = 100us; } --> nodeRx.in;
        // Case study 2
        // queue0.out --> { datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in;

        nodeRx.out --> { datarate = 1Mbps; delay = 100us; } --> queue1.in;
        queue1.out --> { datarate = 1Mbps; delay = 100us; } --> nodeTx.in;
}
