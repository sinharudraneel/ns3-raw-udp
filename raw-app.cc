#include "raw-app.h"

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
    m_running = false;
    m_pktSize = 0;
    m_pktCount = 0;
    m_sent = 0;
    m_PSN = 0;
    m_byteMode = false;
    m_byteArray = nullptr;
}

// Destructor for RawApp, sets socket to nullptr
RawApp::~RawApp() 
{
}

// Sets up all arguments for app
void RawApp::Setup(uint32_t pktSize, uint32_t pktCount, Time interval, bool isSender, Ptr<Node> destNode, bool byteMode, std::shared_ptr<std::vector<uint8_t>> byteArray) 
{
    m_pktSize = pktSize;
    m_pktCount = pktCount;
    m_interval = interval;
    m_isSender = isSender;
    m_destNode = destNode;
    m_byteMode = byteMode;
    m_byteArray = byteArray;
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
        if (m_byteMode) {
            RawApp::SendByteArray(m_byteArray);
        }
        else {
            RawApp::SendPacket();
        }
    }
}

void RawApp::StopApplication() 
{
    m_running = false;
    // Cancel any pending event as of the time of calling this function
    if (m_sendEvent.IsPending()) {
        Simulator::Cancel(m_sendEvent);
    }
}

void RawApp::SendByteArray(std::shared_ptr<std::vector<uint8_t>> payload) {
    if (m_running) {
        if ((payload->size() <= 14)) {
            NS_LOG_UNCOND("ERROR: INVALID HEADER LESS THAN 14 BYTES");
        }
        uint8_t destMacBytes[6];
        printf("Dest MAC: ");
        for (int i = 0; i < 6; i++) {
            destMacBytes[i] = (*payload)[i + 6];
            printf("%02x ", destMacBytes[i]);
        }
        printf("\n");
        Mac48Address destMac;
        destMac.CopyFrom(destMacBytes);

        uint16_t proto = (uint16_t)(((*payload)[12] << 8) | (*payload)[13]);
        printf("Protocol: %02x\n", proto);

        uint8_t* payloadraw = payload->data();
        payloadraw = payloadraw + 14;

        Ptr<Packet> pkt = Create<Packet>(payloadraw, (payload->size()));

        Ptr<NetDevice> device = GetNode()->GetDevice(1);
        if (!device->Send(pkt, destMac, proto)) {
            NS_LOG_UNCOND("Error in Sending");
            return;
        }

        NS_LOG_UNCOND("Node " << GetNode()->GetId() << " sent Packet " << m_sent << " at time " << Simulator::Now().GetSeconds() << "s");
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
        // ethHeader.SetLengthType(0x0800);


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

        Ptr<NetDevice> device = GetNode()->GetDevice(1);
        if (!device->Send(pkt, Mac48Address::ConvertFrom(m_destNode->GetDevice(1)->GetAddress()), 0x0800)) {
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

bool RawApp::ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> pkt, uint16_t protocol, const Address& sender)
{
    NS_LOG_UNCOND("Node " << GetNode()->GetId() << " received packet " << m_PSN << " of size " << pkt->GetSize() << " at time " << Simulator::Now().GetSeconds() << "s");
    m_PSN++; // Only local to one channel, need to implement global packet counting
    return true; 
    
}
}