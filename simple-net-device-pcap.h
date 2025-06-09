#ifndef SIMPLE_NET_DEVICE_PCAP_H
#define SIMPLE_NET_DEVICE_PCAP_H
 
#include <stdint.h>
#include <string>
 
#include "ns3/traced-callback.h"
#include "ns3/net-device.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
 
#include "ns3/mac48-address.h"
 
namespace ns3 {
 
template <typename Item> class Queue;
class SimpleChannelPcap;
class Node;
class ErrorModel;
 
class SimpleNetDevicePcap : public NetDevice
{
public:
  static TypeId GetTypeId (void);
  SimpleNetDevicePcap ();
 
  void Receive (Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from);
  
  void SetChannel (Ptr<SimpleChannelPcap> channel);
 
  void SetQueue (Ptr<Queue<Packet> > queue);
 
  Ptr<Queue<Packet> > GetQueue (void) const;
 
  void SetReceiveErrorModel (Ptr<ErrorModel> em);
 
  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
 
  virtual Address GetMulticast (Ipv6Address addr) const;
 
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;
 
protected:
  virtual void DoDispose (void);
 
private:
  Ptr<SimpleChannelPcap> m_channel; 
  NetDevice::ReceiveCallback m_rxCallback; 
  NetDevice::PromiscReceiveCallback m_promiscCallback; 
  Ptr<Node> m_node; 
  uint16_t m_mtu;   
  uint32_t m_ifIndex; 
  Mac48Address m_address; 
  Ptr<ErrorModel> m_receiveErrorModel; 
 
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;
 
  void StartTransmission (void);
 
  void FinishTransmission (Ptr<Packet> packet);
 
  bool m_linkUp; 
 
  bool m_pointToPointMode;
 
  Ptr<Queue<Packet> > m_queue; 
  DataRate m_bps; 
  EventId FinishTransmissionEvent; 
 
  TracedCallback<> m_linkChangeCallbacks;

  // sniffer trace declaration
  TracedCallback<Ptr<const Packet>> m_snifferTrace;
};
 
} // namespace ns3
 
#endif /* SIMPLE_NET_DEVICE_PCAP_H */
