#include <algorithm>
 #include "simple-channel-pcap.h"
 #include "simple-net-device-pcap.h"
 #include "ns3/simulator.h"
 #include "ns3/packet.h"
 #include "ns3/node.h"
 #include "ns3/log.h"
  
 namespace ns3 {
  
 NS_LOG_COMPONENT_DEFINE ("SimpleChannelPcap");
  
 NS_OBJECT_ENSURE_REGISTERED (SimpleChannelPcap);
  
 TypeId 
 SimpleChannelPcap::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::SimpleChannelPcap")
     .SetParent<Channel> ()
     .SetGroupName("Network")
     .AddConstructor<SimpleChannelPcap> ()
     .AddAttribute ("Delay", "Transmission delay through the channel",
                    TimeValue (Seconds (0)),
                    MakeTimeAccessor (&SimpleChannelPcap::m_delay),
                    MakeTimeChecker ())
   ;
   return tid;
 }
  
 SimpleChannelPcap::SimpleChannelPcap ()
 {
   NS_LOG_FUNCTION (this);
 }
  
 void
 SimpleChannelPcap::Send (Ptr<Packet> p, uint16_t protocol,
                      Mac48Address to, Mac48Address from,
                      Ptr<SimpleNetDevicePcap> sender)
 {
   NS_LOG_FUNCTION (this << p << protocol << to << from << sender);
   for (std::vector<Ptr<SimpleNetDevicePcap> >::const_iterator i = m_devices.begin (); i != m_devices.end (); ++i)
     {
       Ptr<SimpleNetDevicePcap> tmp = *i;
       if (tmp == sender)
         {
           continue;
         }
       if (m_blackListedDevices.find (tmp) != m_blackListedDevices.end ())
         {
           if (find (m_blackListedDevices[tmp].begin (), m_blackListedDevices[tmp].end (), sender) !=
               m_blackListedDevices[tmp].end () )
             {
               continue;
             }
         }
       Simulator::ScheduleWithContext (tmp->GetNode ()->GetId (), m_delay,
                                       &SimpleNetDevicePcap::Receive, tmp, p->Copy (), protocol, to, from);
     }
 }
  
 void
 SimpleChannelPcap::Add (Ptr<SimpleNetDevicePcap> device)
 {
   NS_LOG_FUNCTION (this << device);
   m_devices.push_back (device);
 }
  
 std::size_t
 SimpleChannelPcap::GetNDevices (void) const
 {
   NS_LOG_FUNCTION (this);
   return m_devices.size ();
 }
  
 Ptr<NetDevice>
 SimpleChannelPcap::GetDevice (std::size_t i) const
 {
   NS_LOG_FUNCTION (this << i);
   return m_devices[i];
 }
  
 void
 SimpleChannelPcap::BlackList (Ptr<SimpleNetDevicePcap> from, Ptr<SimpleNetDevicePcap> to)
 {
   if (m_blackListedDevices.find (to) != m_blackListedDevices.end ())
     {
       if (find (m_blackListedDevices[to].begin (), m_blackListedDevices[to].end (), from) ==
           m_blackListedDevices[to].end () )
         {
           m_blackListedDevices[to].push_back (from);
         }
     }
   else
     {
       m_blackListedDevices[to].push_back (from);
     }
 }
  
 void
 SimpleChannelPcap::UnBlackList (Ptr<SimpleNetDevicePcap> from, Ptr<SimpleNetDevicePcap> to)
 {
   if (m_blackListedDevices.find (to) != m_blackListedDevices.end ())
     {
       std::vector<Ptr<SimpleNetDevicePcap> >::iterator iter;
       iter = find (m_blackListedDevices[to].begin (), m_blackListedDevices[to].end (), from);
       if (iter != m_blackListedDevices[to].end () )
         {
           m_blackListedDevices[to].erase (iter);
         }
     }
 }
  
  
 } // namespace ns3
