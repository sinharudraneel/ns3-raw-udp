#include "RawApp.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RawApp");

// Mostly boilerplate
TypeId RawApp::GetTypeId()
{
    static TypeId tid = TypeId("ns3::RawApp")
                        .SetParent<Application>()
                        .AddConstructor<RawApp>()
                        ;
    return tid;
}

// Constructor for RawApp, sets socket, running, size, count and sent to default values
RawApp::RawApp() 
{
    m_socket = nullptr;
    m_running = false;
    m_pktSize = 0;
    m_pktCount = 0;
    m_sent = 0;
    m_PSN = 0;
}

// Destructor for RawApp, sets socket to nullptr
RawApp::~RawApp() 
{
    m_socket = nullptr;
}

// Sets up all arguments for app
void RawApp::Setup(uint32_t pktSize, uint32_t pktCount, Time interval, bool isSender, Ptr<Node> destNode) 
{
    m_pktSize = pktSize;
    m_pktCount = pktCount;
    m_interval = interval;
    m_isSender = isSender;
    m_destNode = destNode;
}

// Internally called method to start RawApp application
void RawApp::StartApplication()
{
    m_running = true;
    // Create Raw Socket
    m_socket = Socket::CreateSocket(GetNode(), PacketSocketFactory::GetTypeId());
    Ptr<NetDevice> device = GetNode()->GetDevice(1); // CSMA device resides at index 1 instead of 0, 0 occupied by Bridge Device (why?)
   
    PacketSocketAddress localAddress;
    localAddress.SetSingleDevice(device->GetIfIndex());
    localAddress.SetProtocol(0x0800);

    m_socket->Bind(localAddress);

    if (!m_isSender)
    {
        m_socket->SetRecvCallback(MakeCallback(&RawApp::ReceivePacket, this));
    }
    else
    {
        PacketSocketAddress socketAddress;
        socketAddress.SetSingleDevice(device->GetIfIndex());
        socketAddress.SetPhysicalAddress(Mac48Address::ConvertFrom(m_destNode->GetDevice(1)->GetAddress()));
        socketAddress.SetProtocol(0x0800);
        m_socket->Connect(socketAddress);
        RawApp::SendPacket();
    }
}

void RawApp::StopApplication() 
{
    m_running = false;
    // Cancel any pending event as of the time of calling this function
    if (m_sendEvent.IsRunning()) {
        Simulator::Cancel(m_sendEvent);
    }
    // Close Socket
    if (m_socket) 
    {
        m_socket->Close();   
    }
}

void RawApp::SendPacket() 
{
    if (m_running && m_pktCount && m_sent < m_pktCount) 
    {   
        //LogComponentEnable("RawApp", LOG_LEVEL_DEBUG);
        Ptr<Packet> pkt = Create<Packet>(m_pktSize);

        // EthernetHeader ethHeader;
        // ethHeader.SetSource(Mac48Address::ConvertFrom(GetNode()->GetDevice(1)->GetAddress()));
        // ethHeader.SetDestination(Mac48Address::ConvertFrom(m_destNode->GetDevice(1)->GetAddress()));


        Ipv4Header ipheader;
        ipheader.SetSource(GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
        ipheader.SetDestination(m_destNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal());
        ipheader.SetProtocol(17);
        ipheader.SetPayloadSize(m_pktSize + 8);
        ipheader.SetTtl(64);
        ipheader.EnableChecksum();


        // Must manually add UDP header, set both src and dest ports to 8080
        UdpHeader udpheader;
        udpheader.SetSourcePort(8080);
        udpheader.SetDestinationPort(8080);
        pkt->AddHeader(udpheader);
        pkt->AddHeader(ipheader);
        //pkt->AddHeader(ethHeader);

        if (!m_socket->Send(pkt)) {
            NS_LOG_UNCOND("Error in Sending");
            return;
        }

        NS_LOG_UNCOND("Node " << GetNode()->GetId() << " sent Packet " << m_sent << " at time " << Simulator::Now().GetSeconds() << "s");

        m_sent++;
        
        // Schedule the next event where this function must be called
        // Do it as long as the number of sent packets is less than the count to be sent
        if (m_sent < m_pktCount)
        {
            m_sendEvent = Simulator::Schedule(m_interval, &RawApp::SendPacket, this);
        }
    }
}

void RawApp::ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> pkt;
    while ((pkt = socket->Recv()))
    {
        NS_LOG_UNCOND("Node " << GetNode()->GetId() << " received packet " << m_PSN << " of size " << pkt->GetSize() << " at time " << Simulator::Now().GetSeconds() << "s");
        m_PSN++; // Only local to one channel, need to implement global packet counting 
    }
}
}