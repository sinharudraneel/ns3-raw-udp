#ifndef SIMPLE_CHANNEL_PCAP_H
 #define SIMPLE_CHANNEL_PCAP_H
  
 #include "ns3/channel.h"
 #include "ns3/nstime.h"
 #include "ns3/mac48-address.h"
 #include <vector>
 #include <map>
  
 namespace ns3 {
  
 class SimpleNetDevicePcap;
 class Packet;
  
 class SimpleChannelPcap : public Channel
 {
 public:
   static TypeId GetTypeId (void);
   SimpleChannelPcap ();
  
   virtual void Send (Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from,
                      Ptr<SimpleNetDevicePcap> sender);
  
   virtual void Add (Ptr<SimpleNetDevicePcap> device);
  
   virtual void BlackList (Ptr<SimpleNetDevicePcap> from, Ptr<SimpleNetDevicePcap> to);
  
   virtual void UnBlackList (Ptr<SimpleNetDevicePcap> from, Ptr<SimpleNetDevicePcap> to);
  
   // inherited from ns3::Channel
   virtual std::size_t GetNDevices (void) const;
   virtual Ptr<NetDevice> GetDevice (std::size_t i) const;
  
 private:
   Time m_delay; 
   std::vector<Ptr<SimpleNetDevicePcap> > m_devices; 
   std::map<Ptr<SimpleNetDevicePcap>, std::vector<Ptr<SimpleNetDevicePcap> > > m_blackListedDevices; 
 };
  
 } // namespace ns3
  
 #endif /* SIMPLE_CHANNEL_H */
