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
//   n2
//     |
//     n0 ----------- n1
//     |    10 Mbps
//   n3      10 ms
//
//  queue size: 100
//
// - A flow from n2 to n1 and another from n3 to n1, using BulkSendApplication
// - Vary the delay on the link from n3 to n0
// - Tracing of queues and packet receptions to file "tcp-rtt.tr"
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

NS_LOG_COMPONENT_DEFINE ("TcpRtt");

int
main (int argc, char *argv[])
{
  uint32_t maxBytes = 0;
  uint32_t queueSize = 100;
  DataRate linkRate("10Mbps");
  int delay = 10;
  std::string protocol = "TcpNewReno";
    
  CommandLine cmd;
  cmd.AddValue("queueSize","queue size",queueSize);
  cmd.AddValue("linkRate","link rate",linkRate);
  cmd.AddValue("delay","delay",delay);
  cmd.AddValue("protocol","protocol",protocol);
  cmd.Parse(argc,argv);

    // Set TCP defaults
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
  if (protocol == "TcpTahoe")
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));

  // LogComponentEnable("TcpRtt", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpTahoe", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpReno", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);
  
  // Create the nodes required by the topology
    NodeContainer nodes;
  nodes.Create (4);
  NodeContainer n0n1 (nodes.Get (0), nodes.Get (1));
  NodeContainer n2n0 (nodes.Get (2), nodes.Get (0));
  NodeContainer n3n0 (nodes.Get (3), nodes.Get (0));

  // Create the point-to-point links required by the topology
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue(linkRate));
  p2p.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
  p2p.SetChannelAttribute ("Delay", TimeValue(MilliSeconds (10)));

  PointToPointHelper p2pup;
  p2pup.SetDeviceAttribute ("DataRate", DataRateValue(linkRate));
  p2pup.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pup.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
  p2pup.SetChannelAttribute ("Delay", TimeValue(MilliSeconds (10)));

  PointToPointHelper p2pdown;
  p2pdown.SetDeviceAttribute ("DataRate", DataRateValue(linkRate));
  p2pdown.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pdown.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
  p2pdown.SetChannelAttribute ("Delay", TimeValue(MilliSeconds (delay)));

  NetDeviceContainer dev0 = p2p.Install (n0n1);
  NetDeviceContainer dev1 = p2pup.Install (n2n0);
  NetDeviceContainer dev2 = p2pdown.Install (n3n0);

  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (nodes);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (dev0);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  ipv4.Assign (dev1);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  ipv4.Assign (dev2);

  // Turn on global static routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create two BulkSendApplications
  uint16_t port;
  for (port=10; port < 12; port++ ) {
    BulkSendHelper source ("ns3::TcpSocketFactory",
                           InetSocketAddress (i.GetAddress(1), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    // Set the segment size
    source.SetAttribute ("SendSize", UintegerValue (1460));
    // Install the application on a node
    ApplicationContainer sourceApps = source.Install (nodes.Get (port-8));
    // Set the start time
    int start = 0.0;
    int stop = 10.0;
    sourceApps.Start (Seconds (start));
    sourceApps.Stop (Seconds (stop));

    // Create PacketSinkApplications and install it on node 1
    PacketSinkHelper sink ("ns3::TcpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
    sinkApps.Start (Seconds (start));
    sinkApps.Stop (Seconds (stop));
  }

    // Set up packet tracing
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("tcp-rtt.tr"));
  p2pup.EnableAsciiAll (ascii.CreateFileStream ("tcp-rtt2.tr"));
  p2pdown.EnableAsciiAll (ascii.CreateFileStream ("tcp-rtt3.tr"));
  
  // Run the simulation
  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();
}
