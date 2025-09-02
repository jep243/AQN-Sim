#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/quantum-network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-module.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

using namespace ns3;

int drop_count = 0;
int collision_count = 0;
NS_LOG_COMPONENT_DEFINE("QUANTUM_SIM");

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
	//std::cout << "Dropped" << std::endl;
	drop_count++;
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
	//std::cout << "Collision" << std::endl;
	collision_count++;
}

int
main(int argc, char* argv[])
{
	// Setup command line arguments
	int num_qubits = 20;
	float q_error = 0.5;
	float c_error = 0.5;
	int ave_send_time = 9000000;
	int send_rng = 1000;
	float skew = 0.0;
	int reconfigure_time = 500;
	int timeslot = 10000;
	int packet_delay = 1000;
    int max_tx_queue = 10000;
	int num_channels = 10;
	int debug = 0;
	int nodes_per_switch = 2;
	int cluster_size = 2;
	int num_clusters = 2;
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
	cmd.AddValue("packet-delay", "Time between control and data.", packet_delay);
	cmd.AddValue("max-tx-queue", "Maximum size of tx_queue before app closes.", 
				 max_tx_queue);
	cmd.AddValue("num-channels", "The number of optical channels used",
				 num_channels);
	cmd.AddValue("nodes-per-switch", "The number of nodes attached to a switch",
				 nodes_per_switch);
	cmd.AddValue("cluster-size", "Each cluster will have 2 * cluster-size "
				 "switches and cluster-size * nodes-per-switch of nodes.",
				 cluster_size);
	cmd.AddValue("num-clusters", "The number of clusters.",
				 num_clusters);
	cmd.AddValue("debug", "Debug level 0-none, 1-app, 2-app+optical", debug);
    cmd.Parse(argc, argv);

	// Setup logging
	if (debug > 0)
	{
		LogComponentEnable("QuantumApplication", LOG_LEVEL_ALL);
	}
	if (debug > 1)
	{
		LogComponentEnable("OpticalDevice", LOG_LEVEL_ALL);
    }

	/*Setup the Nodes*/
	int num_nodes = nodes_per_switch * cluster_size * num_clusters;
	int num_layer1 = cluster_size * num_clusters;
	int num_layer2 = cluster_size * num_clusters;
	int num_layer3 = cluster_size;
	NodeContainer nodes;
	for (int i = 0; i < num_nodes; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		nodes.Add(node);
	}
	NodeContainer layer1;
	for (int i = 0; i < num_layer1; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		layer1.Add(node);
	}
	NodeContainer layer2;
	for (int i = 0; i < num_layer2; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		layer2.Add(node);
	}
	NodeContainer layer3;
	for (int i = 0; i < num_layer3; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(skew));
		layer3.Add(node);
	}
	NodeContainer all;
	all.Add(nodes);
	all.Add(layer1);
	all.Add(layer2);
	all.Add(layer3);

	/*Setup the optical network*/
	OpticalHelper helper;
	helper.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1024p"));
	helper.SetDeviceAttribute("FailureRate", DoubleValue(0));
	helper.SetDeviceAttribute("ControlDataRate",
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("DataDataRate",
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("ControlFrameGap", TimeValue(NanoSeconds(5)));
	helper.SetDeviceAttribute("DataFrameGap", TimeValue(NanoSeconds(5)));
	helper.SetDeviceAttribute("PacketDelay", TimeValue(NanoSeconds(packet_delay)));
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
	helper.SetChannelAttribute("NumChannels", UintegerValue(num_channels));

	/*Setup nodes/layer1*/
	int delay;
	int distance;
	int index1;
	int index2;
	int index3;
	NetDeviceContainer *node_devs = new NetDeviceContainer[num_nodes];
	//Loop through layer1 switches
	for (int l1 = 0; l1 < num_layer1; l1++)
	{
		//Loop through nodes attached to switch
		distance = 3;
		for (int n = 0; n < nodes_per_switch; n++)
		{
			if (n % 2 == 1) distance += 3;
			delay = distance * 5;
			helper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
			index1 = (l1 * nodes_per_switch) + n;
			node_devs[index1] = helper.Install(nodes.Get(index1), 
											   layer1.Get(l1)); 
		}
	}

	/*Setup layer1/layer2*/
	int total_switch_devs = (num_clusters * (cluster_size * cluster_size)) + 
							(num_clusters * cluster_size);
	index3 = 0;
	NetDeviceContainer *switch_devs = new NetDeviceContainer[total_switch_devs];
	distance = 3 * (nodes_per_switch - 1);
	//Loop through the blocks of switches
	for (int c = 0; c < num_clusters; c++)
	{
		for (int l1 = 0; l1 < cluster_size; l1++)
		{
			for (int l2 = 0; l2 < cluster_size; l2++)
			{
				delay = (3 + (distance * std::abs(l1 - l2))) * 5;
				helper.SetChannelAttribute("Delay", 
										   TimeValue(NanoSeconds(delay)));
				index1 = (c * cluster_size) + l1;
				index2 = (c * cluster_size) + l2;
				switch_devs[index3++] = helper.Install(layer1.Get(index1), 
											  		   layer2.Get(index2));
			}
		}
	}

	/*Setup layer2/layer3*/
	for (int c = 0; c < num_clusters; c++)
	{
		distance = 3;
		for (int i = 0; i < cluster_size; i++)
		{
			if (i % 2 == 1) distance += 5;
			delay = distance * 5;
			helper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
			index1 = (c * cluster_size) + i;
			switch_devs[index3++] = helper.Install(layer2.Get(index1), 
												   layer3.Get(i));
		}
	}
	
	/*Add Internet Stack*/
	InternetStackHelper stack;
	stack.Install(all);

	/*Set addresses*/
	Ipv4AddressHelper address;
	index1 = 0;
	for (int i = 0; i < total_switch_devs; i++)
	{
		std::stringstream ss;
		ss << "10.1." << index1++ << ".0";
		std::string ip = ss.str();
		const char *ip_c = ip.c_str();
		address.SetBase(Ipv4Address(ip_c), Ipv4Mask("255.255.255.0"));
		address.Assign(switch_devs[i]);
	}

	Ipv4InterfaceContainer *node_addr = new Ipv4InterfaceContainer[num_nodes];
	for (int i = 0; i < num_nodes; i++)
	{
		std::stringstream ss;
		ss << "10.1." << index1++ << ".0";
		std::string ip = ss.str();
		const char *ip_c = ip.c_str();
		address.SetBase(Ipv4Address(ip_c), Ipv4Mask("255.255.255.0"));
		node_addr[i] = address.Assign(node_devs[i]);
	}

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
	q_helper.SetAttribute("MaxTxQueue", UintegerValue(max_tx_queue));
	for (int i = 0; i < num_nodes; i++)
	{
		Address addr = InetSocketAddress(node_addr[i].GetAddress(0), i + 1);
		Ptr<Node> node = nodes.Get(i);
		q_helper.SetAttribute("ID", UintegerValue(i));
		q_helper.SetAttribute("Local", AddressValue(addr));
		apps.Add(q_helper.Install(node));
		Ptr<QuantumApplication> app = 
			node->GetApplication(0)->GetObject<QuantumApplication>();
		for (int j = 0; j < num_nodes; j++)
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
	delete[] node_devs;
	delete[] switch_devs;
	delete[] node_addr;
	std::cout << "CollisionCount," << collision_count << std::endl;
	std::cout << "DropCount," << drop_count << std::endl;
    return 0;
}
