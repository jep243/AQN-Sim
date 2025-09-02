#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/quantum-network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-module.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("QAUNTUM_NETWORK_EXAMPLE");

uint16_t
BytesToUint16(uint8_t* buffer, int offset)
{
	uint16_t low = static_cast<uint16_t>(buffer[offset]);
	uint16_t high = static_cast<uint16_t>(buffer[offset + 1]);
	uint16_t value = high << 8 | low;
	return value;
}

uint32_t
BytesToUint32(uint8_t* buffer, int offset)
{
	uint32_t low = static_cast<uint32_t>(BytesToUint16(buffer, offset));
	uint32_t high = static_cast<uint32_t>(BytesToUint16(buffer, offset + 2));
	uint32_t value = high << 16 | low;
	return value;
}

void
DropSink(std::string context, Ptr<const Packet> packet)
{
	/*
	Ptr<Packet> copy = packet->Copy();
	OpticalHeader op_header;
	copy->RemoveHeader(op_header);
	Ipv4Header ip_header;
	copy->RemoveHeader(ip_header);
	UdpHeader udp_header;
	copy->RemoveHeader(udp_header);
	uint32_t data_size = copy->GetSize();
	NS_ASSERT_MSG(data_size > 5, "Invalid packet size.");
	uint8_t *buffer = new uint8_t[data_size];
	copy->CopyData(buffer, data_size); 
	uint32_t id = BytesToUint32(buffer, 1);
	uint8_t protocol = buffer[5];
	std::cout << "Dropped," << id << "," << (int)protocol << std::endl;
	delete[] buffer;
	*/
	std::cout << "Dropped" << std::endl;
}

void
CollisionSink(std::string context, Ptr<const OpticalDevice> device, 
			  Ptr<const Packet> packet)
{
	/*
	Ptr<Packet> copy = packet->Copy();
	OpticalHeader op_header;
	copy->RemoveHeader(op_header);
	Ipv4Header ip_header;
	copy->RemoveHeader(ip_header);
	UdpHeader udp_header;
	copy->RemoveHeader(udp_header);
	uint32_t data_size = copy->GetSize();
	NS_ASSERT_MSG(data_size > 5, "Invalid packet size.");
	uint8_t *buffer = new uint8_t[data_size];
	copy->CopyData(buffer, data_size); 
	uint32_t id = BytesToUint32(buffer, 1);
	uint8_t protocol = buffer[5];
	std::cout << "Collision," << id << "," << (int)protocol << std::endl;
	delete[] buffer;
	*/
	std::cout << "Collision" << std::endl;
}

int
main(int argc, char* argv[])
{
	// Setup logging
	//LogComponentEnable("QuantumApplication", LOG_LEVEL_ALL);
	//LogComponentEnable("OpticalDevice", LOG_LEVEL_ALL);

	// Setup command line arguments
	int num_qubits = 20;
	float q_error = 0.5;
	float c_error = 0.5;
	int ave_send_time = 9000000;
	int send_rng = 1000;
	float skew = 0.0;
	int reconfigure_time = 500;
	int timeslot = 10000;
    CommandLine cmd(__FILE__);
	cmd.AddValue("qubits", "The number of qubits in QNIC.", num_qubits);
	cmd.AddValue("qerror", "The error rate for quantum traffic.", q_error);
	cmd.AddValue("cerror", "The error rate for classical traffic.", c_error);
	cmd.AddValue("send-time", "The average time between messages.", 
				 ave_send_time);
	cmd.AddValue("send-range", "The range in time between send messages.",
				 send_rng);
	cmd.AddValue("skew", "The amount of skew clocks will have.", skew);
	cmd.AddValue("reconfigure", "The reconfigure time of switches.", 
				 reconfigure_time);
	cmd.AddValue("timeslot", "The duration of the timeslot (ns).", timeslot);
    cmd.Parse(argc, argv);

    /*Setup the Nodes*/
	NodeContainer nodes;
	for (int i = 0; i < 4; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		nodes.Add(node);
	}
	NodeContainer switches;
	for (int i = 0; i < 4; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		switches.Add(node);
	}
	NodeContainer all;
	all.Add(nodes);
	all.Add(switches);

	/*Setup the optical network*/
	OpticalHelper helper;
	helper.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("128p"));
	helper.SetDeviceAttribute("FailureRate", DoubleValue(0));
	helper.SetDeviceAttribute("ControlDataRate",
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("DataDataRate",
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("ControlFrameGap", TimeValue(NanoSeconds(5)));
	helper.SetDeviceAttribute("DataFrameGap", TimeValue(NanoSeconds(5)));
	helper.SetDeviceAttribute("PacketDelay", TimeValue(NanoSeconds(1486)));
	helper.SetDeviceAttribute("SwitchPropagationDelay",
		TimeValue(NanoSeconds(2)));
	helper.SetDeviceAttribute("TotalPropagationDelay",
		TimeValue(NanoSeconds(36)));
	helper.SetDeviceAttribute("ReconfigureTime", 
		TimeValue(NanoSeconds(reconfigure_time)));
	helper.SetDeviceAttribute("TimeslotDuration", 
		TimeValue(NanoSeconds(timeslot)));
	helper.SetDeviceAttribute("PacketProcessing", TimeValue(NanoSeconds(350)));
	helper.SetDeviceAttribute("OpticalProcessing", TimeValue(NanoSeconds(350)));
	helper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(5)));
	helper.SetChannelAttribute("NumChannels", UintegerValue(4));

	/*Setup switches*/
	Ipv4AddressHelper address;
	NetDeviceContainer dev0 = helper.Install(switches.Get(0), switches.Get(2));
	NetDeviceContainer dev1 = helper.Install(switches.Get(0), switches.Get(3));
	NetDeviceContainer dev2 = helper.Install(switches.Get(1), switches.Get(2));
	NetDeviceContainer dev3 = helper.Install(switches.Get(1), switches.Get(3));

	/*Setup nodes*/
	helper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(10)));
	NetDeviceContainer dev4 = helper.Install(nodes.Get(0), switches.Get(0));
	NetDeviceContainer dev5 = helper.Install(nodes.Get(1), switches.Get(0));
	NetDeviceContainer dev6 = helper.Install(nodes.Get(2), switches.Get(1));
	NetDeviceContainer dev7 = helper.Install(nodes.Get(3), switches.Get(1));

	/*Add Internet Stack*/
	InternetStackHelper stack;
	stack.Install(all);

	/*Set addresses*/
	Ipv4InterfaceContainer node_addr[4];
	address.SetBase(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"));
	address.Assign(dev0);
	address.SetBase(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"));
	address.Assign(dev1);
	address.SetBase(Ipv4Address("10.1.3.0"), Ipv4Mask("255.255.255.0"));
	address.Assign(dev2);
	address.SetBase(Ipv4Address("10.1.4.0"), Ipv4Mask("255.255.255.0"));
	address.Assign(dev3);
	address.SetBase(Ipv4Address("10.1.5.0"), Ipv4Mask("255.255.255.0"));
	node_addr[0] = address.Assign(dev4);
	address.SetBase(Ipv4Address("10.1.6.0"), Ipv4Mask("255.255.255.0"));
	node_addr[1] = address.Assign(dev5);
	address.SetBase(Ipv4Address("10.1.7.0"), Ipv4Mask("255.255.255.0"));
	node_addr[2] = address.Assign(dev6);
	address.SetBase(Ipv4Address("10.1.8.0"), Ipv4Mask("255.255.255.0"));
	node_addr[3] = address.Assign(dev7);

	/*Setup Routing and initialization*/
	helper.SetEndpoints(nodes);
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	helper.Initialize(all);

	/*Register trace sinks*/
	Config::Connect("/NodeList/*/DeviceList/*/$ns3::OpticalDevice/DropTrace", 
					MakeCallback(&DropSink));
	Config::Connect("/NodeList/*/DeviceList/*/$ns3::OpticalDevice/CollisionTrace",
					MakeCallback(&CollisionSink));
	Config::Connect("/ChannelList/*/$ns3::OpticalChannel/CollisionTrace",
					MakeCallback(&CollisionSink));

	/*Setup Quantum Application*/
	ApplicationContainer apps;
	QuantumHelper q_helper;
	q_helper.SetAttribute("NumQubits", UintegerValue(num_qubits));
	q_helper.SetAttribute("QuantumFailureRate", DoubleValue(q_error));
	q_helper.SetAttribute("ClassicalFailureRate", DoubleValue(c_error));
	q_helper.SetAttribute("EntanglementTime", UintegerValue(50));
	q_helper.SetAttribute("AverageSendTime", UintegerValue(ave_send_time));
	q_helper.SetAttribute("SendTimeRange", UintegerValue(send_rng));
	for (int i = 0; i < 4; i++)
	{
		Address addr = InetSocketAddress(node_addr[i].GetAddress(0), i + 1);
		Ptr<Node> node = nodes.Get(i);
		q_helper.SetAttribute("ID", UintegerValue(i));
		q_helper.SetAttribute("Local", AddressValue(addr));
		apps.Add(q_helper.Install(node));
		Ptr<QuantumApplication> app = 
			node->GetApplication(0)->GetObject<QuantumApplication>();
		for (int j = 0; j < 4; j++)
		{
			if (i == j) continue;
			addr = InetSocketAddress(node_addr[j].GetAddress(0), j + 1);
			app->AddPeer(addr);
		}
	}
	apps.Start(Seconds(1));
	apps.Stop(Seconds(10));

	Simulator::Stop(Seconds(11));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
