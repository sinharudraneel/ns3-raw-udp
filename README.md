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
After compiling the project, you will find an executable in `build/raw-app`.