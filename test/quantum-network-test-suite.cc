#include "ns3/time-node.h"
#include "ns3/test.h"
#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/packet.h"
#include "ns3/optical-device.h"
#include "ns3/optical-tag.h"
#include "ns3/optical-helper.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/address.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/udp-socket.h"

#include <string>

using namespace ns3;

/**
 * @defgroup quantum-network-tests Tests for quantum-network
 * @ingroup quantum-network
 * @ingroup tests
 */

/**
 * @ingroup quantum-network-tests
 * Test case for time nodes
 */
class TimeNodeTests : public TestCase
{
  public:
    TimeNodeTests();
    virtual ~TimeNodeTests();
  private:
    void DoRun() override;
	void CheckTimes();
	NodeContainer m_nodes;
};
TimeNodeTests::TimeNodeTests()
    : TestCase("Will Test skew and reset of TimeNode clocks."){}
TimeNodeTests::~TimeNodeTests(){}

void
TimeNodeTests::CheckTimes()
{
	for (int i = 0; i < 4; i++)
	{
		Ptr<TimeNode> node = DynamicCast<TimeNode>(m_nodes.Get(i));
		double skew = node->GetSkew();
		NS_TEST_ASSERT_MSG_EQ(skew, i * 10, "Time Node incorrect skew.");
		Time global = Simulator::Now();
		Time n_global = node->GetGlobalTime();
		NS_TEST_ASSERT_MSG_EQ(n_global, global, "Global time is incorrect.");
		Time expected = Seconds(5 + (5 * (i * 10) / 1000000.0));
		Time local = node->GetLocalTime();
		NS_TEST_ASSERT_MSG_EQ(local, expected, "Local time is not corret.");
		node->SetLocalTime(Seconds(0));
		local = node->GetLocalTime();
		NS_TEST_ASSERT_MSG_EQ(local, Seconds(0), "Time reset did not work.");
	}
}

void
TimeNodeTests::DoRun()
{
	for (int i = 0; i < 4; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(i * 10));
		m_nodes.Add(node);
	}
	Simulator::Schedule(Seconds(5), &TimeNodeTests::CheckTimes, this);
	Simulator::Schedule(Seconds(10), &TimeNodeTests::CheckTimes, this);
	Simulator::Run();
	Simulator::Destroy();
}

NodeContainer GetTestNetwork(int endpoints, uint8_t channels)
{
	NodeContainer container;
	NodeContainer ends;
	for (int i = 0; i <= endpoints; i++)
	{
		Ptr<TimeNode> node = CreateObject<TimeNode>();
		node->SetAttribute("Skew", DoubleValue(0));
		container.Add(node);
		if (i > 0)
		{
			ends.Add(node);
		}
	}
	
	OpticalHelper helper;
	helper.SetDeviceAttribute("FailureRate", DoubleValue(0.0));
	helper.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("5p"));
	helper.SetDeviceAttribute("ControlDataRate", 
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("DataDataRate", 
		DataRateValue(DataRate("40Gbps")));
	helper.SetDeviceAttribute("ControlFrameGap", TimeValue(NanoSeconds(50)));
	helper.SetDeviceAttribute("DataFrameGap", TimeValue(NanoSeconds(50)));
	helper.SetDeviceAttribute("PacketDelay", TimeValue(NanoSeconds(738)));
	helper.SetDeviceAttribute("SwitchPropagationDelay", 
		TimeValue(NanoSeconds(2)));
	helper.SetDeviceAttribute("TotalPropagationDelay", 
		TimeValue(NanoSeconds(12)));
	helper.SetDeviceAttribute("ReconfigureTime", TimeValue(NanoSeconds(500)));
	helper.SetDeviceAttribute("TimeslotDuration", TimeValue(MicroSeconds(10)));
	helper.SetDeviceAttribute("PacketProcessing", TimeValue(NanoSeconds(350)));
	helper.SetDeviceAttribute("OpticalProcessing", TimeValue(NanoSeconds(350)));
	helper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(5)));
	helper.SetChannelAttribute("NumChannels", UintegerValue(channels));
	
	InternetStackHelper stack;
	Ipv4AddressHelper address;
	for (int i = 0; i < endpoints; i++)
	{
		Ptr<TimeNode> src = DynamicCast<TimeNode>(ends.Get(i));
		Ptr<TimeNode> dest = DynamicCast<TimeNode>(container.Get(0));
		NetDeviceContainer dev = helper.Install(src, dest);
		stack.Install(src);
		stack.Install(dest);
		std::string addr = "10.1." + std::to_string(i + 1) + ".0";
		address.SetBase(Ipv4Address(&addr[0]), Ipv4Mask("255.255.255.0"));
		address.Assign(dev);
	}
	helper.SetEndpoints(ends);
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	helper.Initialize(container);
	return ends;
}


/**
 * @ingroup quantum-network-tests
 * Test case send packet down empty channel
 */
class OpticalDeviceEmptyTest : public TestCase
{
  public:
    OpticalDeviceEmptyTest();
    virtual ~OpticalDeviceEmptyTest();
  private:
    void DoRun() override;
	void RxCallback(Ptr<Socket> sock);
	void Setup();
	void SendFunc();
	bool m_result = false;
	Ptr<Socket> m_sock;
	Address m_dest;
};
OpticalDeviceEmptyTest::OpticalDeviceEmptyTest()
    : TestCase("Will test sending packet down empty channel."){}
OpticalDeviceEmptyTest::~OpticalDeviceEmptyTest(){}

void
OpticalDeviceEmptyTest::RxCallback(Ptr<Socket> sock)
{
	Time current = Simulator::Now();
	bool time = current == NanoSeconds(1472);
	Ptr<Packet> packet = sock->Recv();
	uint32_t size = packet->GetSize();
	uint8_t *buffer = new uint8_t[size];
	packet->CopyData(buffer, size);
	std::ostringstream convert;
	for (uint32_t i = 0; i < size; i++)
	{
		convert << buffer[i];
	}
	std::string msg = convert.str();
	bool value = msg == "Hello from node1.";
	m_result = time && value;
	delete[] buffer;
}

void
OpticalDeviceEmptyTest::Setup()
{
	TypeId sock_tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	NodeContainer endpoints = GetTestNetwork(2, 1);
	
	Ptr<Node> node0 = endpoints.Get(0);
	Ptr<Ipv4> ipv40 = node0->GetObject<Ipv4>();
	Ipv4Address addr0 = ipv40->GetAddress(1,0).GetLocal();
	Ptr<Socket> sock0 = Socket::CreateSocket(node0, sock_tid);
	m_dest = InetSocketAddress(addr0, 80);
	sock0->Bind(m_dest);
	sock0->SetRecvCallback(
		MakeCallback(&OpticalDeviceEmptyTest::RxCallback, this));

	Ptr<Node> node1 = endpoints.Get(1);
	Ptr<Ipv4> ipv41 = node1->GetObject<Ipv4>();
	Ipv4Address addr1 = ipv41->GetAddress(1,0).GetLocal();
	m_sock = Socket::CreateSocket(node1, sock_tid);
	Address local1 = InetSocketAddress(addr1, 80);
	m_sock->Bind(local1);
}

void
OpticalDeviceEmptyTest::SendFunc()
{
	std::string msg = "Hello from node1.";
	auto sent = m_sock->SendTo(
		reinterpret_cast<const uint8_t*>(&msg[0]), 17, 0, m_dest);
	NS_TEST_ASSERT_MSG_EQ(sent, 17, "Did not send all bytes.");
}

void
OpticalDeviceEmptyTest::DoRun()
{
	Setup();
	Simulator::Schedule(NanoSeconds(10), &OpticalDeviceEmptyTest::SendFunc,
						this);
	Simulator::Stop(Seconds(5));
	Simulator::Run();
	Simulator::Destroy();
	NS_TEST_ASSERT_MSG_EQ(m_result, true, 
		"Transmission took wrong amount of time.");
}

/**
 * @ingroup quantum-network-tests
 * Test case will time packets so they get sent before the other finishes
 * The Optical Device should schedule them so no collision occurs
 */
class OpticalDeviceQueueTest : public TestCase
{
  public:
    OpticalDeviceQueueTest();
    virtual ~OpticalDeviceQueueTest();
  private:
    void DoRun() override;
	void RxCallback(Ptr<Socket> sock);
	void Setup();
	void SendFunc();
	Ptr<Socket> m_sock;
	Address m_dest;
	uint64_t m_values[8] = {1472, 1887, 2302, 2717, 3132, 3547, 11462, 11877};
	bool m_success[8] = {false, false, false, false, false, false, false, false};
	int m_rx_count = 0;
};
OpticalDeviceQueueTest::OpticalDeviceQueueTest()
    : TestCase("Will test sending packets faster than the tx rate."){}
OpticalDeviceQueueTest::~OpticalDeviceQueueTest(){}

void
OpticalDeviceQueueTest::RxCallback(Ptr<Socket> sock)
{
	if (m_rx_count > 7)
	{
		m_success[0] = false;
		return;
	}
	Time current = Simulator::Now();
	uint64_t expected = m_values[m_rx_count];
	bool time = current == NanoSeconds(expected);
	Ptr<Packet> packet = sock->Recv();
	uint32_t size = packet->GetSize();
	uint8_t *buffer = new uint8_t[size];
	packet->CopyData(buffer, size);
	std::ostringstream convert;
	for (uint32_t i = 0; i < size; i++)
	{
		convert << buffer[i];
	}
	std::string msg = convert.str();
	bool value = msg == "Hello from node1.";
	m_success[m_rx_count++] = time && value;
	delete[] buffer;
}

void
OpticalDeviceQueueTest::Setup()
{
	TypeId sock_tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	NodeContainer endpoints = GetTestNetwork(2, 1);
	
	Ptr<Node> node0 = endpoints.Get(0);
	Ptr<Ipv4> ipv40 = node0->GetObject<Ipv4>();
	Ipv4Address addr0 = ipv40->GetAddress(1,0).GetLocal();
	Ptr<Socket> sock0 = Socket::CreateSocket(node0, sock_tid);
	m_dest = InetSocketAddress(addr0, 80);
	sock0->Bind(m_dest);
	sock0->SetRecvCallback(
		MakeCallback(&OpticalDeviceQueueTest::RxCallback, this));

	Ptr<Node> node1 = endpoints.Get(1);
	Ptr<Ipv4> ipv41 = node1->GetObject<Ipv4>();
	Ipv4Address addr1 = ipv41->GetAddress(1,0).GetLocal();
	m_sock = Socket::CreateSocket(node1, sock_tid);
	Address local1 = InetSocketAddress(addr1, 80);
	m_sock->Bind(local1);
}

void
OpticalDeviceQueueTest::SendFunc()
{
	std::string msg = "Hello from node1.";
	auto sent = m_sock->SendTo(
		reinterpret_cast<const uint8_t*>(&msg[0]), 17, 0, m_dest);
	NS_TEST_ASSERT_MSG_EQ(sent, 17, "Did not send all bytes.");
}

void
OpticalDeviceQueueTest::DoRun()
{
	Setup();
	Simulator::Schedule(NanoSeconds(10), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(11), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(12), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(13), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(14), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(15), &OpticalDeviceQueueTest::SendFunc,
						this);
	//Queue should overflow at this point
	Simulator::Schedule(NanoSeconds(16), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(10000), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Schedule(NanoSeconds(10001), &OpticalDeviceQueueTest::SendFunc,
						this);
	Simulator::Stop(Seconds(5));
	Simulator::Run();
	Simulator::Destroy();
	bool success = true;
	for (int i = 0; i < 8; i++)
	{
		if (!m_success[i])
		{
			success = false;
			break;
		}
	}
	NS_TEST_ASSERT_MSG_EQ(success, true, 
		"Transmission series did not arrive as expected.");
}

/**
 * @ingroup quantum-network-tests
 * Test case collisions in optical channel
 */
class OpticalDeviceCollisionTest : public TestCase
{
  public:
    OpticalDeviceCollisionTest();
    virtual ~OpticalDeviceCollisionTest();
  private:
    void DoRun() override;
	void RxCallback(Ptr<Socket> sock);
	void Setup();
	void SendFunc(int src, int dest);
	void CollisionSink(Ptr<const OpticalDevice> dev, Ptr<const Packet> p);
	Ptr<Socket> m_socks[2];
	Address m_addrs[2];

	uint64_t m_rx_values[7] = {3038, 3322, 
							   14028, 14312, 
							   22462, 22497, 22877};
	bool m_rx_success[7] = {false, false, 
							false, false, 
							false, false, false};
	int m_rx_count = 0;

	uint64_t m_c_values[2] = {1105, 12096};
	bool m_c_success[2] = {false, false};
	int m_c_count = 0;
	int m_c_test_count = 0;
};
OpticalDeviceCollisionTest::OpticalDeviceCollisionTest()
    : TestCase("Will test packet collisions in the channel."){}
OpticalDeviceCollisionTest::~OpticalDeviceCollisionTest(){}

void
OpticalDeviceCollisionTest::RxCallback(Ptr<Socket> sock)
{
	if (m_rx_count > 6)
	{
		return;
	}
	Time current = Simulator::Now();
	uint64_t expected = m_rx_values[m_rx_count];
	bool time = current == NanoSeconds(expected);
	Ptr<Packet> packet = sock->Recv();
	uint32_t size = packet->GetSize();
	uint8_t *buffer = new uint8_t[size];
	packet->CopyData(buffer, size);
	std::ostringstream convert;
	for (uint32_t i = 0; i < size; i++)
	{
		convert << buffer[i];
	}
	std::string msg = convert.str();
	bool value = msg == "Hello from node.";
	m_rx_success[m_rx_count++] = time && value;
	delete[] buffer;
}

void
OpticalDeviceCollisionTest::CollisionSink(Ptr<const OpticalDevice> dev,
										  Ptr<const Packet> p)
{
	if (m_c_count > 0)
	{
		return;
	}
	Time current = Simulator::Now();
	Time expected = NanoSeconds(m_c_values[m_c_test_count]);
	m_c_success[m_c_test_count] = current == expected;
	m_c_count++;
}

void
OpticalDeviceCollisionTest::Setup()
{
	TypeId sock_tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	NodeContainer endpoints = GetTestNetwork(2, 1);

	for (int i = 0; i < 2; i++)
	{
		Ptr<Node> node = endpoints.Get(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ipv4Address addr = ipv4->GetAddress(1,0).GetLocal();
		m_socks[i] = Socket::CreateSocket(node, sock_tid);
		m_addrs[i] = InetSocketAddress(addr, 80);
		m_socks[i]->Bind(m_addrs[i]);
		m_socks[i]->SetRecvCallback(
			MakeCallback(&OpticalDeviceCollisionTest::RxCallback, this));

		Ptr<NetDevice> dev = node->GetDevice(0);
		Ptr<Channel> channel = dev->GetChannel();
		Ptr<OpticalChannel> optical = DynamicCast<OpticalChannel>(channel);
		channel->TraceConnectWithoutContext("CollisionTrace", 
				MakeCallback(&OpticalDeviceCollisionTest::CollisionSink, this));
		if (i == 0)
		{
			for (size_t i = 0; i < channel->GetNDevices(); i++)
			{
				Ptr<OpticalDevice> other = DynamicCast<OpticalDevice>(
						channel->GetDevice(i));
				if (other != dev)
				{
					other->TraceConnectWithoutContext("CollisionTrace",
						MakeCallback(&OpticalDeviceCollisionTest::CollisionSink,
									 this));
					break;
				}
			}
		}
	}
}

void
OpticalDeviceCollisionTest::SendFunc(int src, int dest)
{
	std::string msg = "Hello from node.";
	auto sent = m_socks[src]->SendTo(
		reinterpret_cast<const uint8_t*>(&msg[0]), 16, 0, m_addrs[dest]);
	NS_TEST_ASSERT_MSG_EQ(sent, 16, "Did not send all bytes.");
}

void
OpticalDeviceCollisionTest::DoRun()
{
	
	Setup();
	// Test 1 basic collision
	Simulator::Schedule(NanoSeconds(10), &OpticalDeviceCollisionTest::SendFunc,
						this, 0, 1);
	Simulator::Schedule(NanoSeconds(10), &OpticalDeviceCollisionTest::SendFunc,
						this, 1, 0);
	Simulator::Stop(NanoSeconds(5000));
	Simulator::Run();
	Simulator::Destroy();
	m_c_test_count++;
	m_c_count = 0;
	
	Setup();
	// Test 2 offset collision
	Simulator::Schedule(NanoSeconds(11000), &OpticalDeviceCollisionTest::SendFunc,
						this, 0, 1);
	Simulator::Schedule(NanoSeconds(11008), &OpticalDeviceCollisionTest::SendFunc,
						this, 1, 0);
	Simulator::Stop(NanoSeconds(16000));
	Simulator::Run();
	Simulator::Destroy();
	m_c_test_count++;
	
	Setup();
	// Thread the needle
	Simulator::Schedule(NanoSeconds(21000), &OpticalDeviceCollisionTest::SendFunc,
						this, 0, 1);
	Simulator::Schedule(NanoSeconds(21001), &OpticalDeviceCollisionTest::SendFunc,
						this, 0, 1);
	Simulator::Schedule(NanoSeconds(21035), &OpticalDeviceCollisionTest::SendFunc,
						this, 1, 0);
	Simulator::Stop(NanoSeconds(26000));
	Simulator::Run();
	Simulator::Destroy();
	
	bool success = true;
	for (int i = 0; i < 7; i++)
	{
		if (!m_rx_success[i])
		{
			success = false;
			break;
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if (!m_c_success[i])
		{
			success = false;
			break;
		}
	}
	NS_TEST_ASSERT_MSG_EQ(success, true, 
		"Transmission series did not arrive as expected.");
}

/**
 * @ingroup quantum-network-tests
 * Test case route multiple outputs
 */
class OpticalDeviceRouteTest : public TestCase
{
  public:
    OpticalDeviceRouteTest();
    virtual ~OpticalDeviceRouteTest();
  private:
    void DoRun() override;
	void RxCallback(Ptr<Socket> sock);
	void Setup();
	void SendFunc(int src, int dest);
	void CollisionSink(Ptr<const OpticalDevice> dev, Ptr<const Packet> p);
	Ptr<Socket> m_socks[3];
	Address m_addrs[3];

	uint64_t m_rx_values[11] = {1472, 1502, 1542,
								11374, 11640, 12054,
								22962, 23700,
								11462, 11502, 11542};
	bool m_rx_success[11] = {false, false, false, 
							 false, false, false, 
							 false, false,
							 false, false, false};
	int m_rx_count = 0;

	bool m_c_success[1] = {true};
};
OpticalDeviceRouteTest::OpticalDeviceRouteTest()
    : TestCase("Will test routing to multiple outputs."){}
OpticalDeviceRouteTest::~OpticalDeviceRouteTest(){}

void
OpticalDeviceRouteTest::RxCallback(Ptr<Socket> sock)
{
	if (m_rx_count > 10)
	{
		m_rx_success[0] = false;
		return;
	}
	Time current = Simulator::Now();
	uint64_t expected = m_rx_values[m_rx_count];
	bool time = current == NanoSeconds(expected);
	Ptr<Packet> packet = sock->Recv();
	uint32_t size = packet->GetSize();
	uint8_t *buffer = new uint8_t[size];
	packet->CopyData(buffer, size);
	std::ostringstream convert;
	for (uint32_t i = 0; i < size; i++)
	{
		convert << buffer[i];
	}
	std::string msg = convert.str();
	bool value = msg == "Hello from node.";
	m_rx_success[m_rx_count++] = time && value;
	delete[] buffer;
}

void
OpticalDeviceRouteTest::CollisionSink(Ptr<const OpticalDevice> dev,
										  Ptr<const Packet> p)
{
}

void
OpticalDeviceRouteTest::Setup()
{
	TypeId sock_tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	NodeContainer endpoints = GetTestNetwork(3, 1);

	for (int i = 0; i < 3; i++)
	{
		Ptr<Node> node = endpoints.Get(i);
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ipv4Address addr = ipv4->GetAddress(1,0).GetLocal();
		m_socks[i] = Socket::CreateSocket(node, sock_tid);
		m_addrs[i] = InetSocketAddress(addr, 80);
		m_socks[i]->Bind(m_addrs[i]);
		m_socks[i]->SetRecvCallback(
			MakeCallback(&OpticalDeviceRouteTest::RxCallback, this));

		Ptr<NetDevice> dev = node->GetDevice(0);
		Ptr<Channel> channel = dev->GetChannel();
		Ptr<OpticalChannel> optical = DynamicCast<OpticalChannel>(channel);
		channel->TraceConnectWithoutContext("CollisionTrace", 
				MakeCallback(&OpticalDeviceRouteTest::CollisionSink, this));
		if (i == 0)
		{
			for (size_t i = 0; i < channel->GetNDevices(); i++)
			{
				Ptr<OpticalDevice> other = DynamicCast<OpticalDevice>(
						channel->GetDevice(i));
				if (other != dev)
				{
					other->TraceConnectWithoutContext("CollisionTrace",
						MakeCallback(&OpticalDeviceRouteTest::CollisionSink,
									 this));
					break;
				}
			}
		}
	}
}

void
OpticalDeviceRouteTest::SendFunc(int src, int dest)
{
	std::string msg = "Hello from node.";
	auto sent = m_socks[src]->SendTo(
		reinterpret_cast<const uint8_t*>(&msg[0]), 16, 0, m_addrs[dest]);
	NS_TEST_ASSERT_MSG_EQ(sent, 16, "Did not send all bytes.");
}

void
OpticalDeviceRouteTest::DoRun()
{
	
	Setup();
	// Test 1 basic routing
	Simulator::Schedule(NanoSeconds(10), &OpticalDeviceRouteTest::SendFunc,
						this, 0, 1);
	Simulator::Schedule(NanoSeconds(40), &OpticalDeviceRouteTest::SendFunc,
						this, 1, 2);
	Simulator::Schedule(NanoSeconds(80), &OpticalDeviceRouteTest::SendFunc,
						this, 2, 0);
	// Test 2 drop test
	Simulator::Schedule(NanoSeconds(11), &OpticalDeviceRouteTest::SendFunc,
						this, 0, 2);
	Simulator::Schedule(NanoSeconds(41), &OpticalDeviceRouteTest::SendFunc,
						this, 1, 0);
	Simulator::Schedule(NanoSeconds(81), &OpticalDeviceRouteTest::SendFunc,
						this, 2, 1);

	// Test 3 route to same endpoint
	Simulator::Schedule(NanoSeconds(21500), &OpticalDeviceRouteTest::SendFunc,
						this, 0, 2);
	Simulator::Schedule(NanoSeconds(21500), &OpticalDeviceRouteTest::SendFunc,
						this, 1, 2);
	Simulator::Stop(NanoSeconds(40000));
	Simulator::Run();
	Simulator::Destroy();
	
	Setup();
	// Test 4 collision from multiple sources
	Simulator::Schedule(NanoSeconds(10000), &OpticalDeviceRouteTest::SendFunc,
						this, 0, 2);
	Simulator::Schedule(NanoSeconds(10040), &OpticalDeviceRouteTest::SendFunc,
						this, 1, 0);
	Simulator::Schedule(NanoSeconds(10080), &OpticalDeviceRouteTest::SendFunc,
						this, 2, 1);
	Simulator::Stop(NanoSeconds(21000));
	Simulator::Run();
	Simulator::Destroy();
	
	bool success = true;
	for (int i = 0; i < 11; i++)
	{
		if (!m_rx_success[i])
		{
			success = false;
			break;
		}
	}
	for (int i = 0; i < 1; i++)
	{
		if (!m_c_success[i])
		{
			success = false;
			break;
		}
	}
	NS_TEST_ASSERT_MSG_EQ(success, true, 
		"Transmission series did not arrive as expected.");
}

/**
 * @ingroup quantum-network-tests
 * TestSuite for module quantum-network
 */
class QuantumNetworkTestSuite : public TestSuite
{
  public:
    QuantumNetworkTestSuite();
};

QuantumNetworkTestSuite::QuantumNetworkTestSuite()
    : TestSuite("quantum-network", Type::UNIT)
{
    AddTestCase(new TimeNodeTests(), TestCase::Duration::QUICK);

    AddTestCase(new OpticalDeviceEmptyTest(), TestCase::Duration::QUICK);
    AddTestCase(new OpticalDeviceQueueTest(), TestCase::Duration::QUICK);
    AddTestCase(new OpticalDeviceCollisionTest(), TestCase::Duration::QUICK);
    AddTestCase(new OpticalDeviceRouteTest(), TestCase::Duration::QUICK);
}
/**
 * @ingroup quantum-network-tests
 * Static variable for test initialization
 */
static QuantumNetworkTestSuite squantumNetworkTestSuite;
