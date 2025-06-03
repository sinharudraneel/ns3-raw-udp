/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h" 
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/csma-net-device.h"
#include "ns3/bridge-module.h"
#include "ns3/log.h"
#include "RawApp.h"

/*
 *  UDP Raw Socket Network Topology
 *
 *              10Mb/s, 3ms         10.0.0.3             10.0.1.1      10Mb/s, 5ms
 *      n0------------------------------|                    |-----------------------------n4
 *      10.0.0.1                        |    1.5Mbps, 20ms   |                       10.0.1.2
 *                                      n2------------------n3
 *              10Mb/s, 3ms             |                    |         10Mb/s, 5ms
 *      n1------------------------------|                    |-----------------------------n5
 *      10.0.0.2                    10.0.2.1             10.0.2.2                   10.0.1.3       
 * 
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UDPTestScript");

struct runConfig {
    bool send;
    double interval;
    double start;
    int numPkts;
    int pktSize;
    Ptr<Node> src;
    Ptr<Node> dst;
};

enum channelNum {
    channel04,
    channel40,
    channel05,
    channel50,
    channel14,
    channel41,
    channel15,
    channel51
};

int main(int argc, char* argv[]) {
    int pktSize = 1024;
    struct runConfig configs[8] = {true, 1.0, 1.0, 5, pktSize, nullptr, nullptr};
    for (int i = 1; i < 8; i++) {
        configs[i].send = false;
        configs[i].interval = 1.0;
        configs[i].start = 1.0;
        configs[i].numPkts = 5;
        configs[i].pktSize = pktSize;
        configs[i].src = nullptr;
        configs[i].dst = nullptr;
    }
    double simEnd = 10.0;
    CommandLine cmd(__FILE__);
    cmd.Usage("Simulator for raw socket UDP packet transmission over a simple network topology\nHere are your options for the command line\n");
    cmd.AddValue("send04", "bool: send packets from n0 to n4", configs[channel04].send);
    cmd.AddValue("send40", "bool: send packets from n4 to n0", configs[channel40].send);
    cmd.AddValue("send05", "bool: send packets from n0 to n5", configs[channel05].send);
    cmd.AddValue("send50", "bool: send packets from n5 to n0", configs[channel50].send);
    cmd.AddValue("send14", "bool: send packets from n1 to n4", configs[channel14].send);
    cmd.AddValue("send41", "bool: send packets from n4 to n1", configs[channel41].send);
    cmd.AddValue("send15", "bool: send packets from n1 to n5", configs[channel15].send);
    cmd.AddValue("send51", "bool: send packets from n5 to n1", configs[channel51].send);

    cmd.AddValue("start04", "double: start time for n0 to n4 (s)", configs[channel04].start);
    cmd.AddValue("start40", "double: start time for n4 to n0 (s)", configs[channel40].start);
    cmd.AddValue("start05", "double: start time for n0 to n5 (s)", configs[channel05].start);
    cmd.AddValue("start50", "double: start time for n5 to n0 (s)", configs[channel50].start);
    cmd.AddValue("start14", "double: start time for n1 to n4 (s)", configs[channel14].start);
    cmd.AddValue("start41", "double: start time for n4 to n1 (s)", configs[channel41].start);
    cmd.AddValue("start15", "double: start time for n1 to n5 (s)", configs[channel15].start);
    cmd.AddValue("start51", "double: start time for n5 to n1 (s)", configs[channel51].start);
    
    cmd.AddValue("int04", "double: interval between packets for n0 to n4 (s)", configs[channel04].interval);
    cmd.AddValue("int40", "double: interval between packets for n4 to n0 (s)", configs[channel40].interval);
    cmd.AddValue("int05", "double: interval between packets for n0 to n5 (s)", configs[channel05].interval);
    cmd.AddValue("int50", "double: interval between packets for n5 to n0 (s)", configs[channel50].interval);
    cmd.AddValue("int14", "double: interval between packets for n1 to n4 (s)", configs[channel14].interval);
    cmd.AddValue("int41", "double: interval between packets for n4 to n1 (s)", configs[channel41].interval);
    cmd.AddValue("int15", "double: interval between packets for n1 to n5 (s)", configs[channel15].interval);
    cmd.AddValue("int51", "double: interval between packets for n5 to n1 (s)", configs[channel51].interval);

    cmd.AddValue("num04", "int: number of packets to be sent from n0 to n4", configs[channel04].numPkts);
    cmd.AddValue("num40", "int: number of packets to be sent from n4 to n0", configs[channel40].numPkts);
    cmd.AddValue("num05", "int: number of packets to be sent from n0 to n5", configs[channel05].numPkts);
    cmd.AddValue("num50", "int: number of packets to be sent from n5 to n0", configs[channel50].numPkts);
    cmd.AddValue("num14", "int: number of packets to be sent from n1 to n4", configs[channel14].numPkts);
    cmd.AddValue("num41", "int: number of packets to be sent from n4 to n1", configs[channel41].numPkts);
    cmd.AddValue("num15", "int: number of packets to be sent from n1 to n5", configs[channel15].numPkts);
    cmd.AddValue("num51", "int: number of packets to be sent from n5 to n1", configs[channel51].numPkts);

    cmd.AddValue("pktSize", "int: size of packets to be sent (bytes)", pktSize);
    cmd.AddValue("simEnd", "double: end time for the simulation", simEnd);
    cmd.Parse(argc, argv);

    if (pktSize != 1024) {
        for (int i = 0; i < 8; i++) {
            configs[i].pktSize = pktSize;
        }
    }

    Time::SetResolution(Time::NS);
    // Log component enable

    // Create Nodes
    NodeContainer nodes, endpoints, bridges;
    nodes.Create(6);

    configs[channel04].src = nodes.Get(0);
    configs[channel04].dst = nodes.Get(4);
    configs[channel40].src = nodes.Get(4);
    configs[channel40].dst = nodes.Get(0);
    configs[channel05].src = nodes.Get(0);
    configs[channel05].dst = nodes.Get(5);
    configs[channel14].src = nodes.Get(1);
    configs[channel14].dst = nodes.Get(4);
    configs[channel50].src = nodes.Get(5);
    configs[channel50].dst = nodes.Get(0);
    configs[channel41].src = nodes.Get(4);
    configs[channel41].dst = nodes.Get(1);
    configs[channel15].src = nodes.Get(1);
    configs[channel15].dst = nodes.Get(5);
    configs[channel51].src = nodes.Get(5);
    configs[channel51].dst = nodes.Get(1);

    LogComponentEnable("UDPTestScript", LOG_LEVEL_LOGIC);

    // We categorise nodes into endpoints and bridges. The bridge nodes act as switches
    // The internet stack is only installed on the endpoints, n2 and n3 simply forward packets from one side
    // of the subnet to the other.
    endpoints.Add(nodes.Get(0));
    endpoints.Add(nodes.Get(1));
    endpoints.Add(nodes.Get(4));
    endpoints.Add(nodes.Get(5));

    bridges.Add(nodes.Get(2));
    bridges.Add(nodes.Get(3));

    InternetStackHelper internet;
    internet.Install(endpoints);

    // We use CSMA channels to emulate the behaviour of ethernet. This is paired with the installation
    // of CSMA net devices on all the nodes in the network. These are the devices that grant each node
    // their unique MAC addresses. We set up channels of three types, one between switches and one between endpoints
    // and their respective switch. These have latency and bandwidth parameters as described in the network topology.

    // CSMA Channel for Switch 1 System
    NS_LOG_LOGIC("Setting up Ethernet(CSMA) Channel for Switch 1 (n0, n1, n2)");
    CsmaHelper csmaSwitch1;
    csmaSwitch1.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csmaSwitch1.SetChannelAttribute("Delay", TimeValue(MilliSeconds(3)));
    NetDeviceContainer n0n2 = csmaSwitch1.Install(NodeContainer(nodes.Get(0), nodes.Get(2)));
    NetDeviceContainer n1n2 = csmaSwitch1.Install(NodeContainer(nodes.Get(1), nodes.Get(2)));
    
    // CSMA Channel for Switch 2 System
    NS_LOG_LOGIC("Setting up Ethernet(CSMA) Channel for Switch 2 (n3, n4, n5)");
    CsmaHelper csmaSwitch2;
    csmaSwitch2.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csmaSwitch2.SetChannelAttribute("Delay", TimeValue(MilliSeconds(5)));
    NetDeviceContainer n3n4 = csmaSwitch2.Install(NodeContainer(nodes.Get(3), nodes.Get(4)));
    NetDeviceContainer n3n5 = csmaSwitch2.Install(NodeContainer(nodes.Get(3), nodes.Get(5)));

    // CSMA Channel for n2 -- n3
    NS_LOG_LOGIC("Setting up Ethernet(CSMA) Channel between Switches (n2, n3)");
    CsmaHelper csmaInter;
    csmaInter.SetChannelAttribute("DataRate", StringValue("1.5Mbps"));
    csmaInter.SetChannelAttribute("Delay", StringValue("15ms"));
    NetDeviceContainer interDevices = csmaInter.Install(NodeContainer(nodes.Get(2), nodes.Get(3)));

    // Set up ports for Switch 1 (n2) 
    // i.e. connect the CSMA net devices of all direct
    // connections on that switch
    NetDeviceContainer bridge1ports;
    bridge1ports.Add(n0n2.Get(1));
    bridge1ports.Add(n1n2.Get(1));
    bridge1ports.Add(interDevices.Get(0));

    // Set up ports for Switch 2 (n3) 
    // i.e. connect the CSMA net devices of all direct
    // connections on that switch
    NetDeviceContainer bridge2ports;
    bridge2ports.Add(n3n4.Get(0));
    bridge2ports.Add(n3n5.Get(0));
    bridge2ports.Add(interDevices.Get(1));

    BridgeHelper bridgehelper;
    NetDeviceContainer bridge1device = bridgehelper.Install(nodes.Get(2), bridge1ports);
    NetDeviceContainer bridge2device = bridgehelper.Install(nodes.Get(3), bridge2ports);
    // NS_LOG_INFO("Bridge 1 (n2) MAC Address: " << nodes.Get(2)->GetDevice(0)->GetAddress());
    // NS_LOG_INFO("Bridge 2 (n3) MAC Address: " << nodes.Get(3)->GetDevice(0)->GetAddress());
    // NS_LOG_INFO("Source (n0) MAC Address: " << nodes.Get(0)->GetDevice(1)->GetAddress());
    // NS_LOG_INFO("Dest (n4) MAC Address: " << nodes.Get(4)->GetDevice(1)->GetAddress());

    // Declare all nodes to be in the same subnet with base 10.0.0.0
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces;
    interfaces.Add(ipv4.Assign(n0n2.Get(0)));
    interfaces.Add(ipv4.Assign(n1n2.Get(0)));
    interfaces.Add(ipv4.Assign(n3n4.Get(1)));
    interfaces.Add(ipv4.Assign(n3n5.Get(1)));

    // Set up sending and receiving applicataions for each channel
    // activated by the user.

    for (int i = 0; i < 8; i++) {
        if (configs[i].send) {

            Ptr<RawApp> rcvAppLoop = CreateObject<RawApp>();
            rcvAppLoop->Setup(configs[i].pktSize, 0, Seconds(0.1), false, configs[i].src);
            configs[i].dst->AddApplication(rcvAppLoop);
            rcvAppLoop->SetStartTime(Seconds(configs[i].start));
            rcvAppLoop->SetStopTime(Seconds(simEnd));

            Ptr<RawApp> sndAppLoop = CreateObject<RawApp>();
            sndAppLoop->Setup(configs[i].pktSize, configs[i].numPkts, Seconds(configs[i].interval) ,true, configs[i].dst);
            configs[i].src->AddApplication(sndAppLoop);
            sndAppLoop->SetStartTime(Seconds(configs[i].start));
            sndAppLoop->SetStopTime(Seconds(simEnd));
            
        }
    }

    // Enable packet capture for each endpoint.
    
    csmaSwitch1.EnablePcap("endpoint-n0", n0n2.Get(0), true);
    csmaSwitch1.EnablePcap("endpoint-n1", n1n2.Get(0), true);
    csmaSwitch2.EnablePcap("endpoint-n4", n3n4.Get(1), true);
    csmaSwitch2.EnablePcap("endpoint-n5", n3n5.Get(1), true);
    
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}