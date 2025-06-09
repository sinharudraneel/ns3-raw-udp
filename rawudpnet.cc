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
#include <unordered_set>
#include "ns3/pyviz.h"
#include "external/popl.hpp"

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

template <typename T>
std::vector<T> parse(const std::string &input)
{
    std::vector<T> output;
    std::istringstream stream(input);
    std::string value;
    while (stream >> value)
    {   
        T val;
        if(std::is_same<T, int>()) {
            val = std::stoi(value);
        }
        else if (std::is_same<T, double>()) {
            val = std::stod(value);
        }
        output.push_back(val);
    }

    return output;
}

struct runConfig
{
    double interval;
    double start;
    int numPkts;
    int pktSize;
    Ptr<Node> src;
    Ptr<Node> dst;
};


int main(int argc, char *argv[])
{
    auto op = popl::OptionParser("Allowed Options");

    auto initiatorOpt = op.add<popl::Value<std::string>>("i", "initiator", "String (int): Space separated integers indicating the nodes from which packets must be sent", "0");
    auto targetOpt = op.add<popl::Value<std::string>>("t", "target", "String (int): Space separated integers indicating the nodes to which packets must be sent", "4");
    auto startOpt = op.add<popl::Value<std::string>>("s", "start", "String (double): Space separated floats indicating the start time for each transmission (s)", "1.0");
    auto numPktsOpt = op.add<popl::Value<std::string>>("n", "numPkts", "String (int): Space separated integers indicating the number of packets to be sent from initiator to target", "5");
    auto pktSizeOpt = op.add<popl::Value<std::string>>("p", "pktSize", "String (int): Space separated integers indicating the individual packet size (in bytes)", "1024");
    auto intervalOpt = op.add<popl::Value<std::string>>("l", "interval", "String (double): Space separated floats indicating the interval between packet sends (s)", "1.0");
    auto simEndOpt = op.add<popl::Value<double>>("e", "simEnd", "double: end time for the simulation", 10.0);
    auto helpOpt = op.add<popl::Switch>("h", "help", "Print this help message");

    

    try {
        op.parse(argc, argv);
        if (helpOpt->is_set()) {
            std::cout << op << std::endl;
            return -1;
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        std::cout << op << std::endl;
        return 1; 
    }

    std::vector<int> initiatorVec = parse<int>(initiatorOpt->value());
    std::vector<int> targetVec = parse<int>(targetOpt->value());
    std::vector<double> startVec = parse<double>(startOpt->value());
    std::vector<int> numPktsVec = parse<int>(numPktsOpt->value());
    std::vector<int> pktSizeVec = parse<int>(pktSizeOpt->value());
    std::vector<double> intervalVec = parse<double>(intervalOpt->value());

    int numConfigs = initiatorVec.size();
    if (targetVec.size() != numConfigs) {
        std::cout << "ERROR: Make sure the number of targets match the number of initiators" << std::endl;
        return 1;
    }
    std::unordered_set allowed = {0, 1, 4, 5};
    for (int i = 0; i < numConfigs; i++) {
        for (int num : initiatorVec) {
            if (allowed.find(num) == allowed.end()) {
                std::cout << "ERROR: Only 0, 1, 4, 5 can be initiators or targets" << std::endl;
            }
        }
        for (int num : targetVec) {
            if (allowed.find(num) == allowed.end()) {
                std::cout << "ERROR: Only 0, 1, 4, 5 can be initiators or targets" << std::endl;
                return 1;
            }
        }
    }
    if (startVec.size() != numConfigs) {
        for (int i = 0; i < (numConfigs - startVec.size()); i++) {
            startVec.push_back(1.0);
        }
    }
    if (numPktsVec.size() != numConfigs) {
        for (int i = 0; i < (numConfigs - numPktsVec.size()); i++) {
            numPktsVec.push_back(5);
        }
    }
    if (pktSizeVec.size() != numConfigs) {
        for (int i = 0; i < (numConfigs - pktSizeVec.size()); i++) {
            pktSizeVec.push_back(1024);
        }
    }
    if (intervalVec.size() != numConfigs) {
        for (int i = 0; i < (numConfigs - intervalVec.size()); i++) {
            intervalVec.push_back(1.0);
        }
    }

    Time::SetResolution(Time::NS);
    // Log component enable

    // Create Nodes
    NodeContainer nodes, endpoints, bridges;
    nodes.Create(6);

    struct runConfig configs[numConfigs];
    for (int i = 0; i < numConfigs; i++)
    {
        configs[i].src = nodes.Get(initiatorVec[i]);
        configs[i].dst = nodes.Get(targetVec[i]);
        configs[i].interval = intervalVec[i];
        configs[i].numPkts = numPktsVec[i];
        configs[i].pktSize = pktSizeVec[i];
        configs[i].start = startVec[i];
    }

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

    for (int i = 0; i < numConfigs; i++)
    {

        Ptr<RawApp> rcvAppLoop = CreateObject<RawApp>();
        rcvAppLoop->Setup(configs[i].pktSize, 0, Seconds(0), false, configs[i].src);
        configs[i].dst->AddApplication(rcvAppLoop);
        rcvAppLoop->SetStartTime(Seconds(configs[i].start));
        rcvAppLoop->SetStopTime(Seconds(simEndOpt->value()));

        Ptr<RawApp> sndAppLoop = CreateObject<RawApp>();
        sndAppLoop->Setup(configs[i].pktSize, configs[i].numPkts, Seconds(configs[i].interval), true, configs[i].dst);
        configs[i].src->AddApplication(sndAppLoop);
        sndAppLoop->SetStartTime(Seconds(configs[i].start));
        sndAppLoop->SetStopTime(Seconds(simEndOpt->value()));
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