#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpUdpSimulation");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);

  NodeContainer nodes;
  nodes.Create (4); // Node0, Node1, Node2, Node3

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer d01 = p2p.Install (nodes.Get(0), nodes.Get(1));
  NetDeviceContainer d23 = p2p.Install (nodes.Get(2), nodes.Get(3));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = address.Assign (d01);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = address.Assign (d23);

  // TCP Setup
  uint16_t tcpPort = 8080;
  Address tcpAddress (InetSocketAddress (i01.GetAddress (1), tcpPort));
  PacketSinkHelper tcpSink ("ns3::TcpSocketFactory", tcpAddress);
  ApplicationContainer tcpServerApp = tcpSink.Install (nodes.Get (1));
  tcpServerApp.Start (Seconds (1.0));
  tcpServerApp.Stop (Seconds (10.0));

  OnOffHelper tcpClient ("ns3::TcpSocketFactory", tcpAddress);
  tcpClient.SetAttribute ("DataRate", StringValue ("5Mbps"));
  tcpClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer tcpClientApp = tcpClient.Install (nodes.Get (0));
  tcpClientApp.Start (Seconds (2.0));
  tcpClientApp.Stop (Seconds (10.0));

  // UDP Setup
  uint16_t udpPort = 8081;
  UdpServerHelper udpServer (udpPort);
  ApplicationContainer udpServerApp = udpServer.Install (nodes.Get (3));
  udpServerApp.Start (Seconds (1.0));
  udpServerApp.Stop (Seconds (10.0));

  UdpClientHelper udpClient (i23.GetAddress (1), udpPort);
  udpClient.SetAttribute ("MaxPackets", UintegerValue (320));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer udpClientApp = udpClient.Install (nodes.Get (2));
  udpClientApp.Start (Seconds (2.0));
  udpClientApp.Stop (Seconds (10.0));

  // Flow Monitor
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (11.0));
  Simulator::Run ();

  monitor->SerializeToXmlFile ("flowmon_results.xml", true, true);

  Simulator::Destroy ();
  return 0;
}
