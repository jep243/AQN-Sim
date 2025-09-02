#include "ns3/optical-channel.h"
#include "ns3/optical-device.h"
#include "ns3/optical-tag.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"

#include <vector>

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("OpticalChannel");
	NS_OBJECT_ENSURE_REGISTERED(OpticalChannel);

	TypeId
	OpticalChannel::GetTypeId()
	{
		static TypeId tid = TypeId("ns3::OpticalChannel")
				.SetParent<Channel>()
				.SetGroupName("QuantumNetwork")
				.AddConstructor<OpticalChannel>()
				.AddAttribute("Delay",
							  "Propagation delay through the channel",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(&OpticalChannel::m_delay),
							  MakeTimeChecker())
				.AddAttribute("NumChannels",
							  "The number of channels this connection supports",
							  UintegerValue(1),
							  MakeUintegerAccessor(
							  		&OpticalChannel::SetNChannels,
							  		&OpticalChannel::GetNChannels),
							  MakeUintegerChecker<uint8_t>())
				.AddTraceSource("CollisionTrace",
								"Trace Source indicating a packet collision",
								MakeTraceSourceAccessor(
									&OpticalChannel::m_collisionTrace),
								"ns3::OpticalChannel::TracedCallback");
		return tid;
	}

	OpticalChannel::OpticalChannel()
		: Channel(),
		  m_delay(),
		  m_num_devices(0),
		  m_num_channels(1)
	{
		NS_LOG_FUNCTION(this);
		for (int i = 0; i <= m_num_channels; i++)
		{
			m_dev0_channels.push_back(0);
			m_dev1_channels.push_back(0);
			std::map<uint32_t, Ptr<Packet>> packet_map;
			m_packet_map.push_back(packet_map);
		}
	}

	OpticalChannel::~OpticalChannel()
	{
		NS_LOG_FUNCTION(this);
	}

	void
	OpticalChannel::Attach(Ptr<OpticalDevice> device)
	{
		NS_LOG_FUNCTION(this << device);
		NS_ASSERT_MSG(m_num_devices < 2, "Only two devices permitted");
		NS_ASSERT_MSG(device, "Device did not exist");

		if (m_num_devices == 0)
		{
			m_dev0 = device;
		}
		else
		{
			m_dev1 = device;
		}
		m_num_devices++;

		if (m_num_devices == 2)
		{
			m_dev0->NotifyLinkUp();
			m_dev1->NotifyLinkUp();
		}
	}

	void
	OpticalChannel::TransmitStart(Ptr<Packet> p,
								  Ptr<OpticalDevice> src,
								  Time txTime)
	{
		NS_LOG_FUNCTION(this << p << src << txTime);
		NS_ASSERT_MSG(m_num_devices == 2, "Channel should have 2 devices");

		OpticalTag tag;
		bool found_tag = p->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Did not find the optical tag.");
		uint8_t channel = tag.GetChannel();
		NS_ASSERT_MSG(channel == 0, "Should be control channel.");

		Ptr<OpticalDevice> dest_dev = m_dev0 == src ? m_dev1 : m_dev0;

		Simulator::ScheduleWithContext(dest_dev->GetNode()->GetId(),
									   txTime + m_delay,
									   &OpticalDevice::Receive,
									   dest_dev,
									   p);
	}

	void
	OpticalChannel::PassThrough(Ptr<Packet> p,
								Ptr<OpticalDevice> src,
								Time tx_time)
	{
		NS_LOG_FUNCTION(this << p << src << tx_time);
		NS_ASSERT_MSG(m_num_devices == 2, "Channel should have 2 devices");
		OpticalTag tag;
		bool found_tag = p->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Did not find the optical tag.");
		uint8_t channel = tag.GetChannel();
		uint32_t id = tag.GetMsgId();
		NS_ASSERT_MSG(channel <= m_num_channels && channel > 0, 
				  	  "That channel does not exist");

		std::vector<uint8_t>& src_channels = 
				m_dev0 == src ? m_dev0_channels : m_dev1_channels;
		std::vector<uint8_t>& dest_channels = 
				m_dev0 == src ? m_dev1_channels : m_dev0_channels;
		Ptr<OpticalDevice> dest_dev = m_dev0 == src ? m_dev1 : m_dev0;

		m_packet_map[channel].insert({id, p});
		if (dest_channels[channel] != 0)
		{
			m_collisionTrace(src, p);
			for (auto& item : m_packet_map[channel])
			{
				found_tag = item.second->RemovePacketTag(tag);
				NS_ASSERT_MSG(found_tag, "Packet has no optical tag.");
				tag.DropPacket();
				item.second->AddPacketTag(tag);
			}
		}
		src_channels[channel]++;
		
		Simulator::Schedule(tx_time + m_delay,
							&OpticalChannel::PassThroughFinished,
							this,
							src,
							p,
							channel);
		if (dest_dev->IsEndpoint())
		{
			Simulator::ScheduleWithContext(dest_dev->GetNode()->GetId(),
										   tx_time + m_delay,
										   &OpticalDevice::Receive,
										   dest_dev,
										   p);
		}
		else
		{
			Simulator::ScheduleWithContext(dest_dev->GetNode()->GetId(),
										   m_delay,
										   &OpticalDevice::Receive,
										   dest_dev,
										   p);
		}
	}

	std::size_t
	OpticalChannel::GetNDevices() const
	{
		NS_LOG_FUNCTION(this);
		return m_num_devices;
	}

	Ptr<NetDevice>
	OpticalChannel::GetDevice(std::size_t i) const
	{
		NS_LOG_FUNCTION(this << i);
		if (i == 0)
			return m_dev0;
		else
			return m_dev1;
	}

	uint8_t
	OpticalChannel::GetNChannels() const
	{
		NS_LOG_FUNCTION(this);
		return m_num_channels;
	}

	void
	OpticalChannel::SetNChannels(uint8_t channels)
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(channels > 0, "Requires at least one channel");
		m_dev0_channels.clear();
		m_dev1_channels.clear();
		m_packet_map.clear();
		m_num_channels = channels;
		for (int i = 0; i <= channels; i++)
		{
			m_dev0_channels.push_back(0);
			m_dev1_channels.push_back(0);
			std::map<uint32_t, Ptr<Packet>> packet_map;
			m_packet_map.push_back(packet_map);
		}
	}

	Time
	OpticalChannel::GetDelay() const
	{
		NS_LOG_FUNCTION(this);
		return m_delay;
	}

	void
	OpticalChannel::PassThroughFinished(Ptr<OpticalDevice> src, 
									 	Ptr<Packet> packet,
									 	uint8_t channel)
	{
		NS_LOG_FUNCTION(this << packet << src << channel);
		OpticalTag tag;
		bool found_tag = packet->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Did not find the optical tag.");
		uint32_t id = tag.GetMsgId();
		std::vector<uint8_t>& src_channels = 
				m_dev0 == src ? m_dev0_channels : m_dev1_channels;
		NS_ASSERT_MSG(src_channels[channel] > 0, 
				  "Error, transmission finished on unused channel");
		src_channels[channel]--;
		NS_ASSERT_MSG(!m_packet_map[channel].empty(), 
					  "Should have packets in queue.");
		auto iter = m_packet_map[channel].find(id);
		NS_ASSERT_MSG(iter != m_packet_map[channel].end(), 
					  "Did not find packet in map.");
		m_packet_map[channel].erase(iter);
	}
}
