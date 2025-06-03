# ns3-raw-udp

## Overview
This repository contains a simulation to send UDP packets over ethernet using raw (layer 2) sockets, using the NS3 simulation library. The network topology is as follows:

```
/*
 *  UDP Raw Socket Network Topology
 *
 *              10Mb/s, 3ms                                            10Mb/s, 5ms
 *      n0------------------------------|                    |-----------------------------n4
 *      10.0.0.1                        |    1.5Mbps, 20ms   |                       10.0.0.3
 *                                      n2------------------n3
 *              10Mb/s, 3ms             |                    |         10Mb/s, 5ms
 *      n1------------------------------|                    |-----------------------------n5
 *      10.0.0.2                                                                     10.0.0.4       
 * 
 */
```

Packets can be sent from any endpoint to any other endpoint, where the endpoints are the nodes n0, n1, n4 and n5. The nodes n2 and n3 act as switches and do not have an IP address of their own. All endpoints are part of the same subnet. All the channels used are designed to act as ethernet, implemented as CSMA channels in NS3. To enable switch like behaviour on n3 and n4, bridge network devices have been installed on them as well. Look at rawudpnet.cc for further details.

## Prerequisites
### Basic Requirements
This project is configured to run on a linux system. The system it was programmed on runs Ubuntu 22.04.5 LTS (Jammy Jellyfish). Ensure that these are installed on your system before you proceed:
1. A C++ compiler such as g++ or clang, that supports the C++17 standard
2. CMake, version 13.0 or higher
3. make
If not, please install these on your system before you can proceed.
### NS3 Installation
This project, as mentioned before, is a simulation run on ns3, a discrete-event network simulator. There are a couple of installation options:

**Option 1: Using apt**\
On Ubuntu, install ns3 using 
```
sudo apt update
sudo apt install libns3-dev
```
This would install the libraries and headers in (similar folders if not exactly the same):
- Libraries: `/usr/lib/x86_64-linux-gnu/libns3*.so`
- Headers: `/usr/include/ns3.35/`\
The version might differ in the example include path.


**Option 2: Cloning the GitHub Repository** 

## Compiling the Simulation
### Clone the Repository:
```
git clone https://github.com/sinharudraneel/ns3-raw-udp.git
cd ns3-raw-udp
```
### Configure Build Environment Variables
Check that ns3 is installed in the default paths mentioned in Option 1 in the previous section regarding ns3 installation. If so, there is no setup required. If not, make sure to define the following environment variables:
- `NS3_INCLUDE_PATH`: Path to the directory containing the ns3 headers
- `NS3_LIB_PATH`: Path to the directory containing compiled ns3 libraries

### Build the Project

```
mkdir build
cd build
cmake ..

make
```
After compiling the project, you will find an executable in `./build/rawudpnet`.

## Running the Simulation
Let us look at the executable's help option that can be called by `./build/rawudpnet --help`:
```
rawudpnet [Program Options] [General Arguments]

Simulator for raw socket UDP packet transmission over a simple network topology
Here are your options for the command line


Program Options:
    --send04:   bool: send packets from n0 to n4 [true]
    --send40:   bool: send packets from n4 to n0 [false]
    --send05:   bool: send packets from n0 to n5 [false]
    --send50:   bool: send packets from n5 to n0 [false]
    --send14:   bool: send packets from n1 to n4 [false]
    --send41:   bool: send packets from n4 to n1 [false]
    --send15:   bool: send packets from n1 to n5 [false]
    --send51:   bool: send packets from n5 to n1 [false]
    --start04:  double: start time for n0 to n4 (s) [1]
    --start40:  double: start time for n4 to n0 (s) [1]
    --start05:  double: start time for n0 to n5 (s) [1]
    --start50:  double: start time for n5 to n0 (s) [1]
    --start14:  double: start time for n1 to n4 (s) [1]
    --start41:  double: start time for n4 to n1 (s) [1]
    --start15:  double: start time for n1 to n5 (s) [1]
    --start51:  double: start time for n5 to n1 (s) [1]
    --int04:    double: interval between packets for n0 to n4 (s) [1]
    --int40:    double: interval between packets for n4 to n0 (s) [1]
    --int05:    double: interval between packets for n0 to n5 (s) [1]
    --int50:    double: interval between packets for n5 to n0 (s) [1]
    --int14:    double: interval between packets for n1 to n4 (s) [1]
    --int41:    double: interval between packets for n4 to n1 (s) [1]
    --int15:    double: interval between packets for n1 to n5 (s) [1]
    --int51:    double: interval between packets for n5 to n1 (s) [1]
    --num04:    int: number of packets to be sent from n0 to n4 [5]
    --num40:    int: number of packets to be sent from n4 to n0 [5]
    --num05:    int: number of packets to be sent from n0 to n5 [5]
    --num50:    int: number of packets to be sent from n5 to n0 [5]
    --num14:    int: number of packets to be sent from n1 to n4 [5]
    --num41:    int: number of packets to be sent from n4 to n1 [5]
    --num15:    int: number of packets to be sent from n1 to n5 [5]
    --num51:    int: number of packets to be sent from n5 to n1 [5]
    --pktSize:  int: size of packets to be sent (bytes) [1024]
    --simEnd:   double: end time for the simulation [10]

General Arguments:
    --PrintGlobals:              Print the list of globals.
    --PrintGroups:               Print the list of groups.
    --PrintGroup=[group]:        Print all TypeIds of group.
    --PrintTypeIds:              Print all TypeIds.
    --PrintAttributes=[typeid]:  Print all attributes of typeid.
    --PrintVersion:              Print the ns-3 version.
    --PrintHelp:                 Print this help message.
```
Let us look at the type of arguments one by one:
1. The --sendXY arguments accept boolean true/false values that enable packet transmission from node X to node Y, where X and Y are either 0, 1, 4 or 5.
2. The --startXY arguments accept double precision floating point values that specify when to start packet transmission (in seconds) from node X to node Y.
3. The --intXY arguments accept double precision floating point values that specify how long to wait between sending packets (in seconds), if multiple are to be sent from node X to node Y.
4. The --numXY arguments accept integer values that specify how many packet to send from node X to node Y. 
5. --pktSize accepts an integer value that specifies the size of the packets that are to be sent between nodes in the simulation.
6. --simEnd accepts a double precision floating point value that specifies the time, in seconds at which to end the simulation.

When run without any command line arguments, the program sends 5 packets from n0 to n4 at intervals of 1s each.\

Let us look at the sample output when the program is run without any command line arguments:
```
./build/rawudpnet
 
Setting up Ethernet(CSMA) Channel for Switch 1 (n0, n1, n2)
Setting up Ethernet(CSMA) Channel for Switch 2 (n3, n4, n5)
Setting up Ethernet(CSMA) Channel between Switches (n2, n3)
Node 0 sent Packet at time 1s
Node 4 received packet of size 1052 at time 1.03042s
Node 0 sent Packet at time 2s
Node 4 received packet of size 1052 at time 2.03042s
Node 0 sent Packet at time 3s
Node 4 received packet of size 1052 at time 3.03042s
Node 0 sent Packet at time 4s
Node 4 received packet of size 1052 at time 4.03042s
Node 0 sent Packet at time 5s
Node 4 received packet of size 1052 at time 5.03042s
```

On each run, the files `endpoint-n0-0-1.pcap`, `endpoint-n1-1-1,pcap`, `endpoint-n4-4-1.pcap`, `endpoint-n5-5-1.pcap` will be generated. These can be inspected on the command line via `tshark`, or through a GUI using Wireshark. In particular, we can view the packet traffic using:
```
tshark -r endpoint-n0-0-1.pcap

/*  Sample output:
 *  1   0.000000     10.0.0.1 → 10.0.0.3     UDP 1070 8080 → 8080 Len=1024
 *  2   0.062862 00:00:00_00:00:06 → Broadcast    ARP 64 Who has 10.0.0.1? Tell 10.0.0.3
 *  3   0.062862 00:00:00_00:00:01 → 00:00:00_00:00:06 ARP 64 10.0.0.1 is at 00:00:00:00:00:01
 *  4   0.109821     10.0.0.3 → 10.0.0.1     ICMP 74 Destination unreachable (Port unreachable)
 *  5   1.000000     10.0.0.1 → 10.0.0.3     UDP 1070 8080 → 8080 Len=1024
 *  6   1.053932     10.0.0.3 → 10.0.0.1     ICMP 74 Destination unreachable (Port unreachable)
 *  7   2.000000     10.0.0.1 → 10.0.0.3     UDP 1070 8080 → 8080 Len=1024
 *  8   2.053932     10.0.0.3 → 10.0.0.1     ICMP 74 Destination unreachable (Port unreachable)
 *  9   3.000000     10.0.0.1 → 10.0.0.3     UDP 1070 8080 → 8080 Len=1024
 * 10   3.053932     10.0.0.3 → 10.0.0.1     ICMP 74 Destination unreachable (Port unreachable)
 * 11   4.000000     10.0.0.1 → 10.0.0.3     UDP 1070 8080 → 8080 Len=1024
 * 12   4.053932     10.0.0.3 → 10.0.0.1     ICMP 74 Destination unreachable (Port unreachable)
 */
```

Or one can view packet header and hex breakdowns using:
```
tshark -x -V -r endpoint-n0-0-1.pcap
```

## Notes
Note that currently each packet transmission incurs an ICMP 74 Destination Unreachable (Port unreachable) packet in response. This is because there is no UDP socket listening ont the destination port. The aim was to use raw sockets and therefore this is happening. This can be fixed by installing dummy UDP sockets to receive the packets, but I have not found a workaround yet that solves it without having to use dummy UDP sockets.