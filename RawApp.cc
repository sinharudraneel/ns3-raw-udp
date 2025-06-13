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
    m_byteTest = false;
}

// Destructor for RawApp, sets socket to nullptr
RawApp::~RawApp() 
{
    m_socket = nullptr;
}

// Sets up all arguments for app
void RawApp::Setup(uint32_t pktSize, uint32_t pktCount, Time interval, bool isSender, Ptr<Node> destNode, bool byteTest) 
{
    m_pktSize = pktSize;
    m_pktCount = pktCount;
    m_interval = interval;
    m_isSender = isSender;
    m_destNode = destNode;
    m_byteTest = byteTest;
}

// Internally called method to start RawApp application
void RawApp::StartApplication()
{
    m_running = true;
    
    Ptr<NetDevice> device = GetNode()->GetDevice(1); // CSMA device resides at index 1 instead of 0, 0 occupied by Bridge Device (why?)
   
   

    if (!m_isSender)
    {
        device->SetReceiveCallback(MakeCallback(&RawApp::ReceivePacket, this));
    }
    else
    {
        RawApp::SendPacket();
    }
}

void RawApp::StopApplication() 
{
    m_running = false;
    // Cancel any pending event as of the time of calling this function
    if (m_sendEvent.IsPending()) {
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
        Ptr<Packet> pkt;
        if (!m_byteTest) {
            pkt = Create<Packet>(m_pktSize); 


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

            Ptr<NetDevice> device = GetNode()->GetDevice(1);
            if (!device->SendFrom(pkt, Mac48Address::ConvertFrom(GetNode()->GetDevice(1)->GetAddress()), Mac48Address::ConvertFrom(m_destNode->GetDevice(1)->GetAddress()), 0x0800)) {
                NS_LOG_UNCOND("Error in Sending");
                return;
            }
        }
        else 
        {
            uint8_t pktBytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x08, 0x00, 0x45, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 0x66, \
                0x9C, 0x0A, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x03, 0x1F, 0x90, 0x1F, 0x90, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            uint8_t srcBytes[6];
            uint8_t destBytes[6];
            uint16_t protocol = (uint16_t)((pktBytes[12] << 8) | pktBytes[13]);
            for (int i = 0; i < 6; i++) {
                srcBytes[i] = pktBytes[i];
                destBytes[i] = pktBytes[i + 6];
            }
            Mac48Address srcAddr;
            srcAddr.CopyFrom(srcBytes);
            Mac48Address destAddr;
            destAddr.CopyFrom(destBytes);

            pkt = Create<Packet>(pktBytes, 92);
            pkt = pkt->CreateFragment(14, 78);

            Ptr<NetDevice> device = GetNode()->GetDevice(1);
            if (!device->SendFrom(pkt, srcAddr, destAddr, protocol)) {
                NS_LOG_UNCOND("Error in Sending");
                return;
            }

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

bool RawApp::ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> pkt, uint16_t protocol, const Address& sender)
{
   
    NS_LOG_UNCOND("Node " << GetNode()->GetId() << " received packet " << m_PSN << " of size " << pkt->GetSize() << " at time " << Simulator::Now().GetSeconds() << "s");
    m_PSN++; // Only local to one channel, need to implement global packet counting 
    return true;
    
}
}