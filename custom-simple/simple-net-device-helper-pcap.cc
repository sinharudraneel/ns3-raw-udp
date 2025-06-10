#include "ns3/abort.h"
 #include "ns3/log.h"
 #include "ns3/simulator.h"
 #include "ns3/object-factory.h"
 #include "ns3/queue.h"
 #include "ns3/net-device-queue-interface.h"
 #include "simple-net-device-pcap.h"
 #include "simple-channel-pcap.h"
 #include "ns3/config.h"
 #include "ns3/packet.h"
 #include "ns3/names.h"
 #include "ns3/boolean.h"
  
 #include "ns3/trace-helper.h"
 #include "simple-net-device-helper-pcap.h"
  
 #include <string>
  
 namespace ns3 {
  
 NS_LOG_COMPONENT_DEFINE ("SimpleNetDeviceHelperPcap");
  
 SimpleNetDeviceHelperPcap::SimpleNetDeviceHelperPcap ()
 {
   m_queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
   m_deviceFactory.SetTypeId ("ns3::SimpleNetDevicePcap");
   m_channelFactory.SetTypeId ("ns3::SimpleChannelPcap");
   m_pointToPointMode = false;
 }
  
 void 
 SimpleNetDeviceHelperPcap::SetQueue (std::string type,
                                  std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4)
 {
   QueueBase::AppendItemTypeIfNotPresent (type, "Packet");
  
   m_queueFactory.SetTypeId (type);
   m_queueFactory.Set (n1, v1);
   m_queueFactory.Set (n2, v2);
   m_queueFactory.Set (n3, v3);
   m_queueFactory.Set (n4, v4);
 }
  
 void
 SimpleNetDeviceHelperPcap::SetChannel (std::string type,
                                    std::string n1, const AttributeValue &v1,
                                    std::string n2, const AttributeValue &v2,
                                    std::string n3, const AttributeValue &v3,
                                    std::string n4, const AttributeValue &v4)
 {
   m_channelFactory.SetTypeId (type);
   m_channelFactory.Set (n1, v1);
   m_channelFactory.Set (n2, v2);
   m_channelFactory.Set (n3, v3);
   m_channelFactory.Set (n4, v4);
 }
  
 void
 SimpleNetDeviceHelperPcap::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
 {
   m_deviceFactory.Set (n1, v1);
 }
  
 void
 SimpleNetDeviceHelperPcap::SetChannelAttribute (std::string n1, const AttributeValue &v1)
 {
   m_channelFactory.Set (n1, v1);
 }
  
 void
 SimpleNetDeviceHelperPcap::SetNetDevicePointToPointMode (bool pointToPointMode)
 {
   m_pointToPointMode = pointToPointMode;
 }
  
 NetDeviceContainer
 SimpleNetDeviceHelperPcap::Install (Ptr<Node> node) const
 {
   Ptr<SimpleChannelPcap> channel = m_channelFactory.Create<SimpleChannelPcap> ();
   return Install (node, channel);
 }
  
 NetDeviceContainer
 SimpleNetDeviceHelperPcap::Install (Ptr<Node> node, Ptr<SimpleChannelPcap> channel) const
 {
   return NetDeviceContainer (InstallPriv (node, channel));
 }
  
 NetDeviceContainer 
 SimpleNetDeviceHelperPcap::Install (const NodeContainer &c) const
 {
   Ptr<SimpleChannelPcap> channel = m_channelFactory.Create<SimpleChannelPcap> ();
  
   return Install (c, channel);
 }
  
 NetDeviceContainer 
 SimpleNetDeviceHelperPcap::Install (const NodeContainer &c, Ptr<SimpleChannelPcap> channel) const
 {
   NetDeviceContainer devs;
  
   for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
     {
       devs.Add (InstallPriv (*i, channel));
     }
  
   return devs;
 }
  
 Ptr<NetDevice>
 SimpleNetDeviceHelperPcap::InstallPriv (Ptr<Node> node, Ptr<SimpleChannelPcap> channel) const
 {
   Ptr<SimpleNetDevicePcap> device = m_deviceFactory.Create<SimpleNetDevicePcap> ();
   device->SetAttribute ("PointToPointMode", BooleanValue (m_pointToPointMode));
   device->SetAddress (Mac48Address::Allocate ());
   node->AddDevice (device);
   device->SetChannel (channel);
   Ptr<Queue<Packet> > queue = m_queueFactory.Create<Queue<Packet> > ();
   device->SetQueue (queue);
   NS_ASSERT_MSG (!m_pointToPointMode || (channel->GetNDevices () <= 2), "Device set to PointToPoint and more than 2 devices on the channel.");
   // Aggregate a NetDeviceQueueInterface object
   Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface> ();
   ndqi->GetTxQueue (0)->ConnectQueueTraces (queue);
   device->AggregateObject (ndqi);
   return device;
 }

 void 
 SimpleNetDeviceHelperPcap::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
 {
   //
   // All of the Pcap enable functions vector through here including the ones
   // that are wandering through all of devices on perhaps all of the nodes in
   // the system.  We can only deal with devices of type CsmaNetDevice.
   //
   Ptr<SimpleNetDevicePcap> device = nd->GetObject<SimpleNetDevicePcap> ();
   if (device == 0)
     {
       NS_LOG_INFO ("SimpleNetDevicePcap::EnablePcapInternal(): Device " << device << " not of type ns3::SimpleNetDevicePcap");
       return;
     }
  
   PcapHelper pcapHelper;
  
   std::string filename;
   if (explicitFilename)
     {
       filename = prefix;
     }
   else
     {
       filename = pcapHelper.GetFilenameFromDevice (prefix, device);
     }
  
   Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, 
                                                      PcapHelper::DLT_EN10MB);
   
     
    pcapHelper.HookDefaultSink<SimpleNetDevicePcap> (device, "Sniffer", file);
     
 }
 } // namespace ns3