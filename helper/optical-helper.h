#ifndef OPTICAL_HELPER_H
#define OPTICAL_HELPER_H

#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/trace-helper.h"
#include "ns3/time-node.h"
#include "ns3/net-device.h"

namespace ns3
{
	class OpticalHelper
	{
		public:
			OpticalHelper();
			~OpticalHelper();
			
			template <typename... Ts>
			void SetQueue(std::string type, Ts&&... args);
			void SetDeviceAttribute(std::string name, 
									const AttributeValue& value);
			void SetChannelAttribute(std::string name, 
									 const AttributeValue& value);
			NetDeviceContainer Install(NodeContainer c);
			NetDeviceContainer Install(Ptr<TimeNode> a, Ptr<TimeNode> b);
			NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);
			NetDeviceContainer SetEndpoints(NodeContainer c);
			NetDeviceContainer SetEndpoints(Ptr<TimeNode> a);
			void Initialize(NodeContainer c);
		private:
			uint16_t m_dev_count;
			ObjectFactory m_queue_factory;
			ObjectFactory m_channel_factory;
			ObjectFactory m_device_factory;
	};

	template <typename... Ts>
	void
	OpticalHelper::SetQueue(std::string type, Ts&&...args)
	{
		QueueBase::AppendItemTypeIfNotPresent(type, "Packet");
		m_queue_factory.SetTypeId(type);
		m_queue_factory.Set(std::forward<Ts>(args)...);
	}
}

#endif
