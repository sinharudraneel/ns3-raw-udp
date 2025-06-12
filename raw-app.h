#ifndef RAW_APP_H
#define RAW_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application.h"

namespace ns3 
{

class RawApp : public Application {
    public:
        static TypeId GetTypeId(void);

        RawApp();
        ~RawApp() override;

        // Setup necessary parameters for UDP packet transmission over a raw socket
        void Setup(uint32_t pktSize, uint32_t pktCount, Time interval, bool isSender, Ptr<Node> destNode, bool byteMode, std::shared_ptr<std::vector<uint8_t>> byteArray);

        
    private:
        void StartApplication() override;
        void StopApplication() override;

        // Method used by sending application
        void SendPacket();
        void SendByteArray(std::shared_ptr<std::vector<uint8_t>> payload);
        // Receiver method
        bool ReceivePacket(Ptr<NetDevice> device, Ptr<const Packet> pkt, uint16_t protocol, const Address& sender);

        uint32_t m_pktSize;     // Size of each packet to be sent
        uint32_t m_pktCount;    // Count of packets to be sent
        uint32_t m_sent;        // Count of packets already sent
        Time m_interval;        // Interval between packet sends
        bool m_running;         // Running status variable
        EventId m_sendEvent;    // Send Event
        bool m_isSender;        // Identify sender from receiver
        Ptr<Node> m_destNode;   // Destination node for MAC Address
        int m_PSN;              // Packet tracking num
        bool m_byteMode;         // Byte Array Sending Mode
        std::shared_ptr<std::vector<uint8_t>> m_byteArray;    // Byte Array to Send

};

}

#endif