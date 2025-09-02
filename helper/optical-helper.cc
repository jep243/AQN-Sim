#include "ns3/optical-helper.h"
#include "ns3/optical-device.h"
#include "ns3/optical-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/trace-helper.h"
#include "ns3/time-node.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/names.h"

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("OpticalHelper");

	OpticalHelper::OpticalHelper()
		: m_dev_count(0)
	{
		m_queue_factory.SetTypeId("ns3::DropTailQueue<Packet>");
		m_device_factory.SetTypeId("ns3::OpticalDevice");
		m_channel_factory.SetTypeId("ns3::OpticalChannel");
	}

	OpticalHelper::~OpticalHelper()
	{

	}

	void
	OpticalHelper::SetDeviceAttribute(std::string name,
									  const AttributeValue& value)
	{
		m_device_factory.Set(name, value);
	}

	void
	OpticalHelper::SetChannelAttribute(std::string name,
									   const AttributeValue& value)
	{
		m_channel_factory.Set(name, value);
	}

	NetDeviceContainer
	OpticalHelper::Install(NodeContainer c)
	{
		NS_ASSERT(c.GetN() == 2);
		Ptr<Node> node_a = c.Get(0);
		Ptr<TimeNode> tnode_a = node_a->GetObject<TimeNode>();
		Ptr<Node> node_b = c.Get(1);
		Ptr<TimeNode> tnode_b = node_b->GetObject<TimeNode>();
		NS_ASSERT(tnode_a && tnode_b);
		return Install(tnode_a, tnode_b);
	}

	NetDeviceContainer
	OpticalHelper::Install(Ptr<TimeNode> a, Ptr<TimeNode> b)
	{
		NetDeviceContainer container;

		Ptr<OpticalDevice> dev_a = m_device_factory.Create<OpticalDevice>();
		dev_a->SetAddress(Mac48Address::Allocate());
		Ptr<Queue<Packet>> control_queue_a = 
			m_queue_factory.Create<Queue<Packet>>();
		dev_a->SetControlQueue(control_queue_a);
		dev_a->SetDeviceId(m_dev_count++);
		a->AddDevice(dev_a);
		
		Ptr<OpticalDevice> dev_b = m_device_factory.Create<OpticalDevice>();
		dev_b->SetAddress(Mac48Address::Allocate());
		Ptr<Queue<Packet>> control_queue_b = 
			m_queue_factory.Create<Queue<Packet>>();
		dev_b->SetControlQueue(control_queue_b);
		dev_b->SetDeviceId(m_dev_count++);
		b->AddDevice(dev_b);

		Ptr<OpticalChannel> channel = 
			m_channel_factory.Create<OpticalChannel>();
		dev_a->Attach(channel);
		dev_b->Attach(channel);
		container.Add(dev_a);
		container.Add(dev_b);

		return container;
	}

	NetDeviceContainer
	OpticalHelper::Install(Ptr<Node> a, Ptr<Node> b)
	{
		Ptr<TimeNode> t_a = a->GetObject<TimeNode>();
		NS_ASSERT_MSG(t_a, "Should only pass in TimeNode.");
		Ptr<TimeNode> t_b = b->GetObject<TimeNode>();
		NS_ASSERT_MSG(t_b,"Should only pass in TimeNode.");
		return Install(t_a, t_b);
	}

	NetDeviceContainer
	OpticalHelper::SetEndpoints(NodeContainer c)
	{
		NetDeviceContainer container;
		NodeContainer::Iterator it;
		for (it = c.Begin(); it != c.End(); ++it)
		{
			Ptr<Node> node = *it;
			Ptr<TimeNode> tnode = node->GetObject<TimeNode>();
			if (tnode)
			{
				container.Add(SetEndpoints(tnode));
			}
		}
		return container;
	}

	NetDeviceContainer
	OpticalHelper::SetEndpoints(Ptr<TimeNode> a)
	{
		NetDeviceContainer container;
		for (uint32_t i = 0; i < a->GetNDevices(); i++)
		{
			Ptr<NetDevice> device = a->GetDevice(i);
			Ptr<OpticalDevice> optical = device->GetObject<OpticalDevice>();
			if (optical)
			{
				optical->SetIsEndpoint(true);
				container.Add(optical);
			}
		}
		return container;
	}

	void
	OpticalHelper::Initialize(NodeContainer c)
	{
		NodeContainer::Iterator it;
		for (it = c.Begin(); it != c.End(); ++it)
		{
			Ptr<Node> node = *it;
			for (uint32_t i = 0; i < node->GetNDevices(); i++)
			{
				Ptr<NetDevice> device = node->GetDevice(i);
				if (device->GetInstanceTypeId() == OpticalDevice::GetTypeId())
				{
					Ptr<OpticalDevice> dev = DynamicCast<OpticalDevice>(device);
					dev->UpdateRoutes(true);
					break;
				}
			}
		}
	}
}
