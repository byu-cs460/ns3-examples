/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//       n0 ----------- n1
//            10 Mbps
//             10 ms
//
//  queue size: 1000/100
//
// - Multiple flows from n0 to n1 using BulkSendApplication.
// - Tracing of queues and packet receptions to file "tcp-multiple-flow.tr"
// - Tracing in pcap format available when uncommented

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpMultipleFlow");

int
main (int argc, char *argv[])
{
  uint32_t maxBytes = 0;
  uint32_t queueSize = 1000;
  DataRate linkRate("10Mbps");
  std::string protocol = "TcpNewReno";
    
  CommandLine cmd;
  cmd.AddValue("queueSize","queue size",queueSize);
  cmd.AddValue("linkRate","link rate",linkRate);
  cmd.AddValue("protocol","protocol",protocol);
  cmd.Parse(argc,argv);

    // Set TCP defaults
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
  if (protocol == "TcpTahoe")
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));

  // LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TCPMultipleFlow", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpTahoe", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpReno", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);
  
  // Create the nodes required by the topology
    NodeContainer nodes;
  nodes.Create (2);

  // Create the point-to-point link required by the topology
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue(linkRate));
  pointToPoint.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  pointToPoint.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (nodes);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  // Create a set of BulkSendApplications
  uint16_t port;
  for (port=10; port < 15; port++ ) {
    BulkSendHelper source ("ns3::TcpSocketFactory",
                           InetSocketAddress (i.GetAddress (1), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    // Set the segment size
    source.SetAttribute ("SendSize", UintegerValue (1460));
    // Install the application on node 0
    ApplicationContainer sourceApps = source.Install (nodes.Get (0));
    // Set the start time
    int start = 0.0 + 5*(port - 10);
    int stop = 50.0 - 5*(port - 10);
    sourceApps.Start (Seconds (start));
    sourceApps.Stop (Seconds (stop));

    // Create a PacketSinkApplication and install it on node 1
    PacketSinkHelper sink ("ns3::TcpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
    sinkApps.Start (Seconds (start));
    sinkApps.Stop (Seconds (stop));
  }

  // Set up packet tracing
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-multiple-flow.tr"));
  // pointToPoint.EnablePcapAll ("tcp-multiple-flow", false);

  // Run the simulation
  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();
}
