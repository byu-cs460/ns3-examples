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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

#include <string>

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Link Test");

int
main (int argc, char *argv[])
{
    uint32_t queueSize = 100;
    DataRate linkRate("10Mbps");
    double udpRate = 1.0;
    
    CommandLine cmd;
    cmd.AddValue("queueSize","queue size",queueSize);
    cmd.AddValue("udpRate","UDP rate",udpRate);
    cmd.AddValue("linkRate","link rate",linkRate);
    cmd.Parse(argc,argv);
    
    LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    
    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue(linkRate));
    pointToPoint.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(queueSize));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    uint16_t port = 4000;
    UdpServerHelper server (port);

    ApplicationContainer serverApps = server.Install (nodes.Get (1));
    serverApps.Start (Seconds (0.0));
    serverApps.Stop (Seconds (100.0));

    double interval = linkRate.CalculateTxTime(1500)/udpRate;

    UdpClientHelper client (interfaces.GetAddress (1), port);
    client.SetAttribute ("MaxPackets", UintegerValue (1000000));
    client.SetAttribute ("Interval", TimeValue (Seconds(interval)));
    client.SetAttribute ("PacketSize", UintegerValue (1472));

    ApplicationContainer clientApps = client.Install (nodes.Get (0));
    clientApps.Start (Seconds (0.0));
    clientApps.Stop (Seconds (10.0));

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("link.tr"));
    /* pointToPoint.EnablePcapAll ("link"); */

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
