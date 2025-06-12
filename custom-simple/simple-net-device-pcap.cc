#include "simple-net-device-pcap.h"
 
#include "ns3/error-model.h"
#include "ns3/queue.h"
#include "simple-channel-pcap.h"
 
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tag.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ethernet-header.h"
 
namespace ns3
{
 
NS_LOG_COMPONENT_DEFINE("SimpleNetDevicePcap");
 
/**
 * @brief SimpleNetDevicePcap tag to store source, destination and protocol of each packet.
 */
class SimpleTag : public Tag
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
 
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
 
    /**
     * Set the source address
     * @param src source address
     */
    void SetSrc(Mac48Address src);
    /**
     * Get the source address
     * @return the source address
     */
    Mac48Address GetSrc() const;
 
    /**
     * Set the destination address
     * @param dst destination address
     */
    void SetDst(Mac48Address dst);
    /**
     * Get the destination address
     * @return the destination address
     */
    Mac48Address GetDst() const;
 
    /**
     * Set the protocol number
     * @param proto protocol number
     */
    void SetProto(uint16_t proto);
    /**
     * Get the protocol number
     * @return the protocol number
     */
    uint16_t GetProto() const;
 
    void Print(std::ostream& os) const override;
 
  private:
    Mac48Address m_src;        //!< source address
    Mac48Address m_dst;        //!< destination address
    uint16_t m_protocolNumber; //!< protocol number
};
 
NS_OBJECT_ENSURE_REGISTERED(SimpleTag);
 
TypeId
SimpleTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SimpleTag")
                            .SetParent<Tag>()
                            .SetGroupName("Network")
                            .AddConstructor<SimpleTag>();
    return tid;
}
 
TypeId
SimpleTag::GetInstanceTypeId() const
{
    return GetTypeId();
}
 
uint32_t
SimpleTag::GetSerializedSize() const
{
    return 8 + 8 + 2;
}
 
void
SimpleTag::Serialize(TagBuffer i) const
{
    uint8_t mac[6];
    m_src.CopyTo(mac);
    i.Write(mac, 6);
    m_dst.CopyTo(mac);
    i.Write(mac, 6);
    i.WriteU16(m_protocolNumber);
}
 
void
SimpleTag::Deserialize(TagBuffer i)
{
    uint8_t mac[6];
    i.Read(mac, 6);
    m_src.CopyFrom(mac);
    i.Read(mac, 6);
    m_dst.CopyFrom(mac);
    m_protocolNumber = i.ReadU16();
}
 
void
SimpleTag::SetSrc(Mac48Address src)
{
    m_src = src;
}
 
Mac48Address
SimpleTag::GetSrc() const
{
    return m_src;
}
 
void
SimpleTag::SetDst(Mac48Address dst)
{
    m_dst = dst;
}
 
Mac48Address
SimpleTag::GetDst() const
{
    return m_dst;
}
 
void
SimpleTag::SetProto(uint16_t proto)
{
    m_protocolNumber = proto;
}
 
uint16_t
SimpleTag::GetProto() const
{
    return m_protocolNumber;
}
 
void
SimpleTag::Print(std::ostream& os) const
{
    os << "src=" << m_src << " dst=" << m_dst << " proto=" << m_protocolNumber;
}
 
NS_OBJECT_ENSURE_REGISTERED(SimpleNetDevicePcap);
 
TypeId
SimpleNetDevicePcap::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SimpleNetDevicePcap")
            .SetParent<NetDevice>()
            .SetGroupName("Network")
            .AddConstructor<SimpleNetDevicePcap>()
            .AddTraceSource("Sniffer", 
                          "Trace source for packets passing through the device",
                          MakeTraceSourceAccessor(&SimpleNetDevicePcap::m_snifferTrace),
                          "ns3::Packet::TracedCallback") // Edit 1: Add Trace Source for Packet Sniffer
            .AddTraceSource("MacTxDrop",
                            "Trace source indicating a packet has been "
                            "dropped by the device before transmission",
                            MakeTraceSourceAccessor(&SimpleNetDevicePcap::m_macTxDropTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("PhyTxDrop",
                            "Trace source indicating a packet has been "
                            "dropped by the device during transmission",
                            MakeTraceSourceAccessor(&SimpleNetDevicePcap::m_phyTxDropTrace),
                            "ns3::Packet::TracedCallback")
            .AddAttribute("ReceiveErrorModel",
                          "The receiver error model used to simulate packet loss",
                          PointerValue(),
                          MakePointerAccessor(&SimpleNetDevicePcap::m_receiveErrorModel),
                          MakePointerChecker<ErrorModel>())
            .AddAttribute("PointToPointMode",
                          "The device is configured in Point to Point mode",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SimpleNetDevicePcap::m_pointToPointMode),
                          MakeBooleanChecker())
            .AddAttribute("TxQueue",
                          "A queue to use as the transmit queue in the device.",
                          StringValue("ns3::DropTailQueue<Packet>"),
                          MakePointerAccessor(&SimpleNetDevicePcap::m_queue),
                          MakePointerChecker<Queue<Packet>>())
            .AddAttribute("DataRate",
                          "The default data rate for point to point links. Zero means infinite",
                          DataRateValue(DataRate("0b/s")),
                          MakeDataRateAccessor(&SimpleNetDevicePcap::m_bps),
                          MakeDataRateChecker())
            .AddTraceSource("PhyRxDrop",
                            "Trace source indicating a packet has been dropped "
                            "by the device during reception",
                            MakeTraceSourceAccessor(&SimpleNetDevicePcap::m_phyRxDropTrace),
                            "ns3::Packet::TracedCallback");
    return tid;
}
 
SimpleNetDevicePcap::SimpleNetDevicePcap()
    : m_channel(nullptr),
      m_node(nullptr),
      m_mtu(0xffff),
      m_ifIndex(0),
      m_linkUp(false)
{
    NS_LOG_FUNCTION(this);
}
 
void
SimpleNetDevicePcap::Receive(Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from)
{
    NS_LOG_FUNCTION(this << packet << protocol << to << from);
    NetDevice::PacketType packetType;
    
    // Edit 2: Add ethernet header for the purposes of packet capture
   EthernetHeader ethHeader;
   ethHeader.SetSource(Mac48Address::ConvertFrom(from));
   ethHeader.SetSource(Mac48Address::ConvertFrom(to));
   ethHeader.SetLengthType(protocol);
   packet->AddHeader(ethHeader);
   m_snifferTrace(packet);
   // Remove Ethernet Header promptly after
   packet->RemoveHeader(ethHeader);
 
    if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt(packet))
    {
        m_phyRxDropTrace(packet);
        return;
    }
 
    if (to == m_address)
    {
        packetType = NetDevice::PACKET_HOST;
    }
    else if (to.IsBroadcast())
    {
        packetType = NetDevice::PACKET_BROADCAST;
    }
    else if (to.IsGroup())
    {
        packetType = NetDevice::PACKET_MULTICAST;
    }
    else
    {
        packetType = NetDevice::PACKET_OTHERHOST;
    }
 
    if (packetType != NetDevice::PACKET_OTHERHOST)
    {
        m_rxCallback(this, packet, protocol, from);
    }
 
    if (!m_promiscCallback.IsNull())
    {
        m_promiscCallback(this, packet, protocol, from, to, packetType);
    }
}
 
void
SimpleNetDevicePcap::SetChannel(Ptr<SimpleChannelPcap> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
    m_channel->Add(this);
    m_linkUp = true;
    m_linkChangeCallbacks();
}
 
Ptr<Queue<Packet>>
SimpleNetDevicePcap::GetQueue() const
{
    NS_LOG_FUNCTION(this);
    return m_queue;
}
 
void
SimpleNetDevicePcap::SetQueue(Ptr<Queue<Packet>> q)
{
    NS_LOG_FUNCTION(this << q);
    m_queue = q;
}
 
void
SimpleNetDevicePcap::SetReceiveErrorModel(Ptr<ErrorModel> em)
{
    NS_LOG_FUNCTION(this << em);
    m_receiveErrorModel = em;
}
 
void
SimpleNetDevicePcap::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    m_ifIndex = index;
}
 
uint32_t
SimpleNetDevicePcap::GetIfIndex() const
{
    NS_LOG_FUNCTION(this);
    return m_ifIndex;
}
 
Ptr<Channel>
SimpleNetDevicePcap::GetChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_channel;
}
 
void
SimpleNetDevicePcap::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = Mac48Address::ConvertFrom(address);
}
 
Address
SimpleNetDevicePcap::GetAddress() const
{
    //
    // Implicit conversion from Mac48Address to Address
    //
    NS_LOG_FUNCTION(this);
    return m_address;
}
 
bool
SimpleNetDevicePcap::SetMtu(const uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}
 
uint16_t
SimpleNetDevicePcap::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}
 
bool
SimpleNetDevicePcap::IsLinkUp() const
{
    NS_LOG_FUNCTION(this);
    return m_linkUp;
}
 
void
SimpleNetDevicePcap::AddLinkChangeCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_linkChangeCallbacks.ConnectWithoutContext(callback);
}
 
bool
SimpleNetDevicePcap::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return !m_pointToPointMode;
}
 
Address
SimpleNetDevicePcap::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return Mac48Address::GetBroadcast();
}
 
bool
SimpleNetDevicePcap::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return !m_pointToPointMode;
}
 
Address
SimpleNetDevicePcap::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    return Mac48Address::GetMulticast(multicastGroup);
}
 
Address
SimpleNetDevicePcap::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    return Mac48Address::GetMulticast(addr);
}
 
bool
SimpleNetDevicePcap::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return m_pointToPointMode;
}
 
bool
SimpleNetDevicePcap::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return false;
}
 
bool
SimpleNetDevicePcap::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
 
    return SendFrom(packet, m_address, dest, protocolNumber);
}
 
bool
SimpleNetDevicePcap::SendFrom(Ptr<Packet> p,
                          const Address& source,
                          const Address& dest,
                          uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << p << source << dest << protocolNumber);
    if (p->GetSize() > GetMtu())
    {
        return false;
    }
 
    Mac48Address to = Mac48Address::ConvertFrom(dest);
    Mac48Address from = Mac48Address::ConvertFrom(source);
 
    SimpleTag tag;
    tag.SetSrc(from);
    tag.SetDst(to);
    tag.SetProto(protocolNumber);
 
    p->AddPacketTag(tag);

    // Edit 3: Add ethernet header for packet capture
   EthernetHeader ethHeader;
   ethHeader.SetSource(Mac48Address::ConvertFrom(source));
   ethHeader.SetSource(Mac48Address::ConvertFrom(dest));
   ethHeader.SetLengthType(protocolNumber);
   p->AddHeader(ethHeader);
   m_snifferTrace(p);
   // Remove ethernet header promptly after
   p->RemoveHeader(ethHeader);
 
    if (m_queue->Enqueue(p))
    {
        if (m_queue->GetNPackets() == 1 && !FinishTransmissionEvent.IsPending())
        {
            StartTransmission();
        }
        return true;
    }
    m_macTxDropTrace(p);
 
    return false;
}
 
void
SimpleNetDevicePcap::StartTransmission()
{
    if (m_queue->GetNPackets() == 0)
    {
        return;
    }
    NS_ASSERT_MSG(!FinishTransmissionEvent.IsPending(),
                  "Tried to transmit a packet while another transmission was in progress");
    Ptr<Packet> packet = m_queue->Dequeue();
 
    /**
     * SimpleChannelPcap will deliver the packet to the far end(s) of the link as soon as Send is called
     * (or after its fixed delay, if one is configured). So we have to handle the rate of the link
     * here, which we do by scheduling FinishTransmission (packetSize / linkRate) time in the
     * future. While that event is running, the transmit path of this NetDevice is busy, so we can't
     * send other packets.
     *
     * SimpleChannelPcap doesn't have a locking mechanism, and doesn't check for collisions, so there's
     * nothing we need to do with the channel until the transmission has "completed" from the
     * perspective of this NetDevice.
     */
    Time txTime = Time(0);
    if (m_bps > DataRate(0))
    {
        txTime = m_bps.CalculateBytesTxTime(packet->GetSize());
    }
    FinishTransmissionEvent =
        Simulator::Schedule(txTime, &SimpleNetDevicePcap::FinishTransmission, this, packet);
}
 
void
SimpleNetDevicePcap::FinishTransmission(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
 
    SimpleTag tag;
    packet->RemovePacketTag(tag);
 
    Mac48Address src = tag.GetSrc();
    Mac48Address dst = tag.GetDst();
    uint16_t proto = tag.GetProto();
 
    m_channel->Send(packet, proto, dst, src, this);
 
    StartTransmission();
}
 
Ptr<Node>
SimpleNetDevicePcap::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}
 
void
SimpleNetDevicePcap::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}
 
bool
SimpleNetDevicePcap::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return !m_pointToPointMode;
}
 
void
SimpleNetDevicePcap::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_rxCallback = cb;
}
 
void
SimpleNetDevicePcap::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_channel = nullptr;
    m_node = nullptr;
    m_receiveErrorModel = nullptr;
    m_queue->Dispose();
    if (FinishTransmissionEvent.IsPending())
    {
        FinishTransmissionEvent.Cancel();
    }
    NetDevice::DoDispose();
}
 
void
SimpleNetDevicePcap::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);
    m_promiscCallback = cb;
}
 
bool
SimpleNetDevicePcap::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return true;
}
 
} // namespace ns3