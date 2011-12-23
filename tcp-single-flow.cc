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
//  queue size: 20
//
// - Flow from n0 to n1 using BulkSendApplication.
// - Tracing of queues and packet receptions to file "tcp-single-flow.tr"
// - Tracing of congestion window in "tcp-single-flow.cwnd"
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

NS_LOG_COMPONENT_DEFINE ("TCPSingleFlow");

// CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
static void 
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}

static void
TraceCwnd ()
{
  // Trace changes to the congestion window
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("tcp-single-flow.cwnd");
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream));
}

int
main (int argc, char *argv[])
{
  uint32_t maxBytes = 0;
  uint32_t queueSize = 20;
  DataRate linkRate("10Mbps");
  std::string protocol = "TcpNewReno";
    
  CommandLine cmd;
  cmd.AddValue("queueSize","queue size",queueSize);
  cmd.AddValue("linkRate","link rate",linkRate);
  cmd.AddValue("protocol","protocol",protocol);
  cmd.Parse(argc,argv);

    // Set TCP defaults
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  // Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (524288));
  // Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (524288));
  // Config::SetDefault("ns3::TcpSocket::SlowStartThreshold", UintegerValue (1000000));
  if (protocol == "TcpTahoe")
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));

  // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpTahoe", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpReno", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);
  // LogComponentEnable("TCPSingleFlow", LOG_LEVEL_ALL);
  
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

  // Create a BulkSendApplication and install it on node 0
  uint16_t port = 9;  // well-known echo port number

  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (i.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  // Set the segment size
  source.SetAttribute ("SendSize", UintegerValue (10000));
  ApplicationContainer sourceApps = source.Install (nodes.Get (0));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));

  // Create a PacketSinkApplication and install it on node 1
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));

  // Set up tracing
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-single-flow.tr"));
  // pointToPoint.EnablePcapAll ("tcp-single-flow", false);
  // Setup tracing for cwnd
  Simulator::Schedule(Seconds(0.00001),&TraceCwnd);
  
  // Run the simulation
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
}
