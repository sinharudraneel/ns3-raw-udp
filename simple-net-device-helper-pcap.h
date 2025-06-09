#ifndef SIMPLE_NETDEVICE_HELPER_PCAP_H
 #define SIMPLE_NETDEVICE_HELPER_PCAP_H
  
 #include <string>
  
 #include "ns3/attribute.h"
 #include "ns3/object-factory.h"
 #include "ns3/net-device-container.h"
 #include "ns3/node-container.h"
 #include "simple-channel-pcap.h"
  
 namespace ns3 {
  
 class SimpleNetDeviceHelperPcap : public PcapHelperForDevice
 {
 public:
   SimpleNetDeviceHelperPcap ();
   virtual ~SimpleNetDeviceHelperPcap () {}
  
   void SetQueue (std::string type,
                  std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                  std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                  std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                  std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());
  
   void SetChannel (std::string type,
                    std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                    std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                    std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                    std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());
  
  
   void SetDeviceAttribute (std::string n1, const AttributeValue &v1);
  
   void SetChannelAttribute (std::string n1, const AttributeValue &v1);
  
   void SetNetDevicePointToPointMode (bool pointToPointMode);
  
   NetDeviceContainer Install (Ptr<Node> node) const;
  
   NetDeviceContainer Install (Ptr<Node> node, Ptr<SimpleChannelPcap> channel) const;
  
   NetDeviceContainer Install (const NodeContainer &c) const;
  
   NetDeviceContainer Install (const NodeContainer &c, Ptr<SimpleChannelPcap> channel) const;
  
 private:
   Ptr<NetDevice> InstallPriv (Ptr<Node> node, Ptr<SimpleChannelPcap> channel) const;
   virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);
  
   ObjectFactory m_queueFactory; 
   ObjectFactory m_deviceFactory; 
   ObjectFactory m_channelFactory; 
   bool m_pointToPointMode; 
  
 };
  
 } // namespace ns3
  
 #endif /* SIMPLE_NETDEVICE_HELPER_H */
