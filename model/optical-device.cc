#include "ns3/optical-device.h"
#include "ns3/optical-channel.h"
#include "ns3/optical-control-header.h"
#include "ns3/optical-data-header.h"
#include "ns3/optical-header.h"
#include "ns3/optical-tag.h"
#include "ns3/time-node.h"

#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/socket.h"
#include "ns3/ipv4-packet-info-tag.h"

#include <map>
#include <vector>
#include <random>
#include <ctime>

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("OpticalDevice");
	NS_OBJECT_ENSURE_REGISTERED(OpticalDevice);

	TypeId
	OpticalDevice::GetTypeId()
	{
		static TypeId tid =
			TypeId("ns3::OpticalDevice")
				.SetParent<NetDevice>()
				.SetGroupName("QuantumNetwork")
				.AddConstructor<OpticalDevice>()
				.AddAttribute("Mtu",
							  "The MAC-level maximum transmission unit.",
							  UintegerValue(1500),
							  MakeUintegerAccessor(&OpticalDevice::SetMtu,
							  					   &OpticalDevice::GetMtu),
							  MakeUintegerChecker<uint16_t>())
				.AddAttribute("ControlDataRate",
							  "The data rate to be used for control channel.",
							  DataRateValue(DataRate("32768b/s")),
							  MakeDataRateAccessor(
							  		&OpticalDevice::m_control_bps),
							  MakeDataRateChecker())
				.AddAttribute("DataDataRate",
							  "The data rate to be used for data channels.",
							  DataRateValue(DataRate("32768b/s")),
							  MakeDataRateAccessor(
							  		&OpticalDevice::m_data_bps),
							  MakeDataRateChecker())
				.AddAttribute("ControlFrameGap",
							  "The time between control frame transmissions.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_control_frame_gap),
							  MakeTimeChecker())
				.AddAttribute("DataFrameGap",
							  "The time between data frame transmissions.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_data_frame_gap),
							  MakeTimeChecker())
				.AddAttribute("ControlQueue",
							  "The transmit queue for the control channel.",
							  PointerValue(),
							  MakePointerAccessor(
							  		&OpticalDevice::m_control_queue),
							  MakePointerChecker<Queue<Packet>>())
				.AddAttribute("PacketDelay",
							  "The delay between the control and data packets.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_packet_delay),
							  MakeTimeChecker())
				.AddAttribute("ReconfigureTime",
							  "The time it takes the device to reconfigure.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_reconfigure_time),
							  MakeTimeChecker())
				.AddAttribute("TimeslotDuration",
							  "The time between reconfigures that data" 
							  " can be transmitted.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_timeslot_duration),
							  MakeTimeChecker())
				.AddAttribute("SwitchPropagationDelay",
							  "How long a packet takes to traverse the switch.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_switch_propagation_delay),
							  MakeTimeChecker())
				.AddAttribute("TotalPropagationDelay",
							  "How long a packet takes to traverse the "
							  "longest network path.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_total_propagation_delay),
							  MakeTimeChecker())
				.AddAttribute("PacketProcessing",
							  "How long it takes for a control packet "
							  "to be processed by the device.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_packet_processing),
							  MakeTimeChecker())
				.AddAttribute("OpticalProcessing",
							  "How long it takes for a optical packet "
							  "to be processed by the device.",
							  TimeValue(Seconds(0)),
							  MakeTimeAccessor(
							  		&OpticalDevice::m_optical_processing),
							  MakeTimeChecker())
				.AddAttribute("ScheduleSize",
							  "The number of timeslots in schedule.",
							  UintegerValue(5),
							  MakeUintegerAccessor(
							  		&OpticalDevice::m_schedule_size),
							  MakeUintegerChecker<uint16_t>())
				.AddAttribute("FailureRate",
							  "The percent chance [0-1] a packet will be dropped.",
							  DoubleValue(0.0),
							  MakeDoubleAccessor(
							  		&OpticalDevice::m_failure_rate),
							  MakeDoubleChecker<double>())
				.AddTraceSource("DropTrace",
								"Trace for when a packet is dropped",
								MakeTraceSourceAccessor(
										&OpticalDevice::m_dropTrace),
								"ns3::TracedCallback<Ptr<const Packet>>")
				.AddTraceSource("RxTrace",
								"Trace for when a packet is received",
								MakeTraceSourceAccessor(
										&OpticalDevice::m_rxTrace),
								"ns3::Packet::TracedCallback")
				.AddTraceSource("TxTrace",
								"Trace for when a packet is transmitted",
								MakeTraceSourceAccessor(
										&OpticalDevice::m_txTrace),
								"ns3::Packet::TracedCallback")
				.AddTraceSource("PassThroughTrace",
								"Trace for when a packet is sent through "
								"the optical channel",
								MakeTraceSourceAccessor(
										&OpticalDevice::m_passThroughTrace),
								"ns3::Packet::TracedCallback")
				.AddTraceSource("CollisionTrace",
								"Trace source indicating packet collision",
								MakeTraceSourceAccessor(
									&OpticalDevice::m_collisionTrace),
								"ns3::OpticalDevice::TracedCallback");
		return tid;
	}

	OpticalDevice::OpticalDevice()
		: m_msg_count(0),
		  m_channel(nullptr),
		  m_is_endpoint(false),
		  m_is_link_up(false),
		  m_is_transmitting_control(false),
		  m_is_transmitting_data(false),
		  m_is_reconfiguring(false)
	{
		NS_LOG_FUNCTION(this);
	}

	OpticalDevice::~OpticalDevice()
	{
		NS_LOG_FUNCTION(this);
	}

	Ptr<Channel>
	OpticalDevice::GetChannel() const
	{
		return m_channel;
	}

	void
	OpticalDevice::SetIfIndex(const uint32_t index)
	{
		NS_LOG_FUNCTION(this << index);
		m_if_index = index;
	}

	uint32_t
	OpticalDevice::GetIfIndex() const
	{
		NS_LOG_FUNCTION(this);
		return m_if_index;
	}

	void
	OpticalDevice::SetAddress(Address address)
	{
		NS_LOG_FUNCTION(this << address);
		m_address = Mac48Address::ConvertFrom(address);
	}

	Address
	OpticalDevice::GetAddress() const
	{
		return m_address;
	}

	bool
	OpticalDevice::SetMtu(const uint16_t mtu)
	{
		NS_LOG_FUNCTION(this << mtu);
		m_mtu = mtu;
		return true;
	}

	uint16_t
	OpticalDevice::GetMtu() const
	{
		return m_mtu;
	}

	bool
	OpticalDevice::IsLinkUp() const
	{
		return m_is_link_up;
	}

	void
	OpticalDevice::NotifyLinkUp()
	{
		NS_LOG_FUNCTION(this);
		m_is_link_up = true;
		m_linkChangeCallbacks();
	}

	void
	OpticalDevice::AddLinkChangeCallback(Callback<void> callback)
	{
		NS_LOG_FUNCTION(this);
		m_linkChangeCallbacks.ConnectWithoutContext(callback);
	}

	bool
	OpticalDevice::IsBroadcast() const
	{
		return true;
	}

	Address
	OpticalDevice::GetBroadcast() const
	{
		return Mac48Address::GetBroadcast();
	}

	bool
	OpticalDevice::IsMulticast() const
	{
		return true;
	}

	Address
	OpticalDevice::GetMulticast(Ipv4Address multicastGroup) const
	{
		return Mac48Address("01:00:5e:00:00:00");
	}

	bool
	OpticalDevice::IsPointToPoint() const
	{
		return true;
	}

	bool
	OpticalDevice::IsBridge() const
	{
		return false;
	}

	void
	OpticalDevice::SetDeviceId(uint16_t id)
	{
		m_dev_id = id;
	}

	uint16_t
	OpticalDevice::GetDeviceId() const
	{
		return m_dev_id;
	}
	
	bool
	OpticalDevice::Send(Ptr<Packet> packet,
						const Address& dest,
						uint16_t protocolNumber)
	{
		NS_LOG_FUNCTION(this << dest << protocolNumber);
		bool success = true;
		bool control_full = m_control_queue->GetCurrentSize() >= 
							m_control_queue->GetMaxSize();
		//If endpoint seperate data and control
		if (m_is_endpoint)
		{
			uint32_t id = ((uint32_t) m_dev_id << 16) | m_msg_count;
			m_msg_count++;
			success = InternalSend(packet, protocolNumber, id);
		}
		//If it is not endpoint only control needs handling
		else
		{
			Ipv4Header ipv4_header;
			Ptr<Packet> copy = packet->Copy();
			uint32_t full_size = packet->GetSize();
			uint32_t read = packet->RemoveHeader(ipv4_header);
			NS_ASSERT_MSG(read > 0, "Sent message has no ipv4.");
			UdpHeader udp_header;
			read = packet->RemoveHeader(udp_header);
			NS_ASSERT_MSG(read > 0, "Send message has no udp.");
			uint32_t data_size = packet->GetSize();
			uint8_t* buffer = new uint8_t[data_size];
			NS_ASSERT_MSG(data_size == 26, "Data is wrong size.");
			packet->CopyData(buffer, data_size);
			uint8_t msg_type = buffer[0];
			uint32_t id = BytesToUint32(buffer, 1);
			uint64_t message_sent = BytesToUint64(buffer, 5);
			uint64_t duration = BytesToUint64(buffer, 13);
			uint8_t channel = buffer[21];
			int dev = BytesToUint32(buffer, 22);
			Time propagation_delay = m_channel->GetDelay();
			Time arrival = 
				Time::FromInteger(message_sent, Time::NS) + propagation_delay;
			Time tx_delay = Time::FromInteger(duration, Time::NS);
			bool result = true;
			if (msg_type == 1)
			{
				result = ScheduleMessage(arrival, GetRemote(), id, 
										 channel, tx_delay, dev);
			}
			delete[] buffer;
		
			Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
			Time current = node->GetLocalTime();
			uint32_t queue_size = m_control_queue->GetNPackets();
			if (m_is_transmitting_control)
			{
				queue_size++;
			}
			Time tx_time = m_control_bps.CalculateBytesTxTime(full_size);
			Time control_tx = m_packet_processing + (tx_time + 
				m_packet_processing + m_control_frame_gap) * queue_size;
			
			uint64_t ctrl_send_time = control_tx.GetNanoSeconds();
			uint64_t data_send_time = (arrival + m_switch_propagation_delay)
				.GetNanoSeconds();
			
			uint32_t cur_dev = GetIfIndex();
			NS_ASSERT_MSG(dev >= 0, "Invalid device id.");
			if (result)
			{
				if (control_full)
				{
					success = false;
				}
				else
				{
					//Update packet data
					uint8_t buffer[26];
					std::memcpy(&buffer[0], &msg_type, 1);
					std::memcpy(&buffer[1], &id, 4);
					std::memcpy(&buffer[5], &data_send_time, 8);
					std::memcpy(&buffer[13], &duration, 8);
					std::memcpy(&buffer[21], &channel, 1);
					std::memcpy(&buffer[22], &cur_dev, 4);
					Ptr<Packet> new_packet = Create<Packet>(buffer, 26);
					
					new_packet->AddHeader(udp_header);
					new_packet->AddHeader(ipv4_header);
					AddOpticalHeader(new_packet, id, protocolNumber, 
									 ctrl_send_time);
					CopyTags(packet, new_packet);
					
					OpticalTag tag;
					tag.SetChannel(0);
					tag.SetMsgId(id);
					new_packet->AddPacketTag(tag);
					
					m_control_queue->Enqueue(new_packet);

					if (!m_is_transmitting_control)
					{
						Ptr<Packet> control = m_control_queue->Dequeue();
						ControlTransmitStart(control);
					}
				}
			}
			else
			{
				success = false;
			}
			
			// If message not successful send NACK
			if (!success && msg_type == 1)
			{
				Ptr<NetDevice> base_dev = m_node->GetDevice(dev);
				Ptr<OpticalDevice> from_dev = 
					DynamicCast<OpticalDevice>(base_dev);
				from_dev->SendCTRL(copy, protocolNumber, id, 2);
			}
		}
		
		return success;
	}

	bool
	OpticalDevice::InternalSend(Ptr<Packet> packet, uint16_t protocol, 
								uint32_t id)
	{
		NS_LOG_FUNCTION(this << packet << protocol << id);
		Ptr<Packet> copy = packet->Copy();
		Ptr<Packet> control;
		Time message_send = SplitPacket(packet, control, protocol, id);
		OpticalTag tag;
		bool found_tag = packet->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Packet did not have optical tag.");
		uint8_t channel = tag.GetChannel();
		AddOpticalHeader(copy, id, protocol, 0);
		NS_ASSERT_MSG(channel > 0, "Should not be control channel");
		bool success = ControlSend(control);
		if (success)
		{
			Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
			Time local = node->GetLocalTime();
			Time tx_time = m_data_bps.CalculateBytesTxTime(packet->GetSize());
			Time check_time = message_send + tx_time + 
				m_total_propagation_delay + m_optical_processing + 
				m_packet_delay + m_packet_processing + NanoSeconds(1); 
			EventId sched_event = Simulator::Schedule(check_time - local,
										&OpticalDevice::CheckSent,
										this,
										id);
			EventId check_event = Simulator::Schedule(message_send - local,
										&OpticalDevice::DataTransmitStart,
										this,
										packet);
			ScheduleItem sched_item;
			sched_item.packet = copy;
			sched_item.schedule_event = sched_event;
			sched_item.check_event = check_event;
			m_sent_table.insert({id, sched_item});
		}
		return success;
	}

	bool
	OpticalDevice::ControlSend(Ptr<Packet> p)
	{
		NS_LOG_FUNCTION(this << p);
		bool control_full = m_control_queue->GetCurrentSize() >= 
							m_control_queue->GetMaxSize();
		if (!control_full)
		{
			m_control_queue->Enqueue(p);

			if (!m_is_transmitting_control)
			{
				Ptr<Packet> control = m_control_queue->Dequeue();
				ControlTransmitStart(control);
			}
		}

		return !control_full;
	}

	void
	OpticalDevice::CheckSent(uint32_t id)
	{	
		NS_LOG_FUNCTION(this << id);
		auto iter = m_sent_table.find(id);
		if (iter != m_sent_table.end())
		{
			ScheduleItem sched_item = iter->second;
			Ptr<Packet> packet = sched_item.packet;
			Simulator::Cancel(sched_item.schedule_event);
			Simulator::Cancel(sched_item.check_event);
			m_sent_table.erase(iter);
			OpticalHeader header;
			uint32_t read = packet->RemoveHeader(header);
			NS_ASSERT_MSG(read > 0, "Saved packet had no header.");
			uint16_t protocol = header.GetProtocol();
			InternalSend(packet, protocol, id);
		}
	}

	uint64_t
	OpticalDevice::BytesToUint64(uint8_t* buffer, int offset)
	{
		NS_LOG_FUNCTION(this << buffer << offset);
		uint64_t low = static_cast<uint64_t>(BytesToUint32(buffer, offset));
		uint64_t high = static_cast<uint64_t>(BytesToUint32(buffer, offset + 4));
		uint64_t value = high << 32 | low;
		return value;
	}

	uint32_t
	OpticalDevice::BytesToUint32(uint8_t* buffer, int offset)
	{
		NS_LOG_FUNCTION(this << buffer << offset);
		uint32_t low = static_cast<uint32_t>(BytesToUint16(buffer, offset));
		uint32_t high = static_cast<uint32_t>(BytesToUint16(buffer, offset + 2));
		uint32_t value = high << 16 | low;
		return value;
	}

	uint16_t
	OpticalDevice::BytesToUint16(uint8_t* buffer, int offset)
	{
		NS_LOG_FUNCTION(this << buffer << offset);
		uint16_t low = static_cast<uint16_t>(buffer[offset]);
		uint16_t high = static_cast<uint16_t>(buffer[offset + 1]);
		uint16_t value = high << 8 | low;
		return value;
	}

	Time
	OpticalDevice::SplitPacket(Ptr<Packet> data, Ptr<Packet>& control, 
							   uint16_t protocol, uint32_t id)
	{
		NS_LOG_FUNCTION(this << data << control << protocol << id);

		Ptr<Packet> copy = data->Copy();
		OpticalHeader optical_header;
		Ipv4Header ipv4_header;
		uint32_t read = copy->RemoveHeader(ipv4_header);
		NS_ASSERT_MSG(read > 0, "Split copy no ipv4.");
		ipv4_header.SetPayloadSize(34);
		UdpHeader udp_header;
		read = copy->RemoveHeader(udp_header);
		NS_ASSERT_MSG(read > 0, "Split copy no udp.");
		uint8_t channel = GetRandomChannel();
		
		Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
		uint64_t current = node->GetLocalTime().GetNanoSeconds();
		AddOpticalHeader(data, id, protocol, current);
		Time tx_data = m_data_bps.CalculateBytesTxTime(data->GetSize());
		uint32_t ctrl_size = 26 + optical_header.GetSerializedSize() + 
								  ipv4_header.GetSerializedSize() + 
								  udp_header.GetSerializedSize();
		Time tx_ctrl = m_control_bps.CalculateBytesTxTime(ctrl_size);
		uint64_t duration = tx_data.GetNanoSeconds();	
		uint64_t message_send = GetPacketTransmitTime(tx_ctrl, tx_data)
			.GetNanoSeconds();
		uint8_t msg_type = 1;
		
		int dev = GetIfIndex();
		NS_ASSERT_MSG(dev >= 0, "Invalid device id.");

		uint8_t buffer[26];
		std::memcpy(&buffer[0], &msg_type, 1); 
		std::memcpy(&buffer[1], &id, 4);
		std::memcpy(&buffer[5], &message_send, 8);
		std::memcpy(&buffer[13], &duration, 8);
		std::memcpy(&buffer[21], &channel, 1);
		std::memcpy(&buffer[22], &dev, 4);
		control = Create<Packet>(buffer, 26);
		control->AddHeader(udp_header);
		control->AddHeader(ipv4_header);
		AddOpticalHeader(control, id, protocol, current);

		OpticalTag tag1;
		tag1.SetChannel(channel);
		tag1.SetMsgId(id);
		NS_ASSERT_MSG(!tag1.IsDropped(), 
			"Packet should not be dropped by default.");
		data->AddPacketTag(tag1);
		OpticalTag tag2;
		tag2.SetChannel(0);
		tag2.SetMsgId(id);
		NS_ASSERT_MSG(!tag2.IsDropped(), 
			"Packet should not be dropped by default.");
		control->AddPacketTag(tag2);
		CopyTags(copy, control);

		return Time::FromInteger(message_send, Time::NS);
	}

	void
	OpticalDevice::CopyTags(Ptr<Packet> original, Ptr<Packet> copy)
	{
		NS_LOG_FUNCTION(this);
		Ptr<Packet> spare_parts = original->Copy();
		
		SocketSetDontFragmentTag ssdft;
		bool found = spare_parts->RemovePacketTag(ssdft);
		if (found)
		{
			copy->AddPacketTag(ssdft);
		}

		Ipv4PacketInfoTag ipit;
		found = spare_parts->RemovePacketTag(ipit);
		if (found)
		{
			copy->AddPacketTag(ipit);
		}

		SocketIpTtlTag sitt;
		found = spare_parts->RemovePacketTag(sitt);
		if (found)
		{
			copy->AddPacketTag(sitt);
		}
	}

	Time
	OpticalDevice::GetPacketTransmitTime(Time& tx_ctrl, Time& tx_data)
	{
		NS_LOG_FUNCTION(this);
		Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
		Time current = node->GetLocalTime();
		uint32_t queue_size = m_control_queue->GetNPackets();
		if (m_is_transmitting_control)
		{
			queue_size++;
		}
		Time control_tx = m_packet_processing + 
			(tx_ctrl + m_packet_processing + m_control_frame_gap) * queue_size;
		Time estimate_send = current + control_tx + m_packet_delay;
		Time send_time = (estimate_send >= m_next_transmit) ? estimate_send : 
			m_next_transmit;
		Time total_time = send_time + tx_data + m_total_propagation_delay;
		for (const auto& pair : m_schedule_table)
		{
			const Time timeslot = pair.first;
			if ((total_time >= timeslot - m_reconfigure_time) &&
				(total_time < timeslot))
			{
				send_time = timeslot;
				break;
			}
		}
		m_next_transmit = send_time + tx_data + m_data_frame_gap + 
			Time::FromInteger(1, Time::NS);
		return send_time;
	}

	uint8_t
	OpticalDevice::GetRandomChannel()
	{
		NS_LOG_FUNCTION(this);
		static std::mt19937 rng(std::time(nullptr));
		uint8_t num_channels = m_channel->GetNChannels();
		std::uniform_int_distribution<uint8_t> dist(1, num_channels);
		uint8_t channel = dist(rng);
		NS_ASSERT_MSG(channel > 0 && channel <= num_channels,
					  "Random channel is outside bounds.");
		return channel;
	}
	
	bool
	OpticalDevice::SendFrom(Ptr<Packet> packet,
							const Address& source,
							const Address& dest,
							uint16_t protocolNumber)
	{
		NS_LOG_FUNCTION(this << packet << source << dest << protocolNumber);
		return false;
	}

	Ptr<Node>
	OpticalDevice::GetNode() const
	{
		return m_node;
	}

	void
	OpticalDevice::SetNode(Ptr<Node> node)
	{
		NS_LOG_FUNCTION(this << node);
		m_node = node;
	}

	bool
	OpticalDevice::NeedsArp() const
	{
		return false;
	}

	void
	OpticalDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
	{
		m_rxCallback = cb;
	}

	Address
	OpticalDevice::GetMulticast(Ipv6Address addr) const
	{
		return Mac48Address("33:33:00:00:00:00");
	}

	void
	OpticalDevice::SetPromiscReceiveCallback(
			NetDevice::PromiscReceiveCallback cb)
	{
		m_promiscCallback = cb;
	}

	bool
	OpticalDevice::SupportsSendFrom() const
	{
		return false;
	}

	void
	OpticalDevice::SetControlDataRate(DataRate bps)
	{
		NS_LOG_FUNCTION(this << bps);
		m_control_bps = bps;
	}

	void
	OpticalDevice::SetDataDataRate(DataRate bps)
	{
		NS_LOG_FUNCTION(this << bps);
		m_data_bps = bps;
	}

	uint64_t
	OpticalDevice::GetControlDataRate()
	{
		return m_control_bps.GetBitRate();
	}

	uint64_t
	OpticalDevice::GetDataDataRate()
	{
		return m_data_bps.GetBitRate();
	}

	void
	OpticalDevice::SetControlFrameGap(Time t)
	{
		m_control_frame_gap = t;
	}

	void
	OpticalDevice::SetDataFrameGap(Time t)
	{
		m_data_frame_gap = t;
	}

	void
	OpticalDevice::SetPacketDelay(Time t)
	{
		m_packet_delay = t;
	}

	void
	OpticalDevice::SetReconfigureTime(Time t)
	{
		m_reconfigure_time = t;
	}

	void
	OpticalDevice::SetTimeslotDuration(Time t)
	{
		m_timeslot_duration = t;
	}

	bool
	OpticalDevice::Attach(Ptr<OpticalChannel> ch)
	{
		NS_LOG_FUNCTION(this << &ch);
		m_channel = ch;
		m_channel->Attach(this);
		UpdateChannels();
		return true;
	}

	void
	OpticalDevice::SetControlQueue(Ptr<Queue<Packet>> queue)
	{
		m_control_queue = queue;
	}

	void
	OpticalDevice::Receive(Ptr<Packet> p)
	{
		NS_LOG_FUNCTION(this << p);
		m_rxTrace(p->Copy());
		OpticalTag tag;
		OpticalHeader header;
		bool found_tag = p->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Packet did not have optical tag.");
		uint8_t channel = tag.GetChannel();
		NS_ASSERT_MSG(channel >= 0 && channel <= m_channel->GetNChannels(),
					  "Channel is invalid.");
		if (m_is_endpoint)
		{
			if (channel == 0)
			{
				uint32_t read = p->RemoveHeader(header);
				Ptr<Packet> copy = p->Copy();
				NS_ASSERT_MSG(read > 0, "Packet did not have optical header.");
				uint16_t protocol = header.GetProtocol();
				Ipv4Header ipv4_header;
				read = p->RemoveHeader(ipv4_header);
				NS_ASSERT_MSG(read > 0, "Sent message has no ipv4.");
				UdpHeader udp_header;
				read = p->RemoveHeader(udp_header);
				NS_ASSERT_MSG(read > 0, "Send message has no udp.");
				uint32_t data_size = p->GetSize();
				uint8_t* buffer = new uint8_t[data_size];
				NS_ASSERT_MSG(data_size == 26, "Data is wrong size.");
				p->CopyData(buffer, data_size);
				
				uint32_t id = header.GetMsgId();
				uint8_t msg_type = buffer[0];
				// Control Message
				if (msg_type == 1)
				{
					Simulator::Schedule(m_packet_processing,
										&OpticalDevice::UpdateReceived,
										this,
										copy,
										protocol,
										id);
				}
				else if (msg_type == 2 || msg_type == 3)
				{
					OpticalHeader copy_header;
					ScheduleItem sched_item;
					Ptr<Packet> data;
					bool found = false;
					for (uint32_t i = 0; i < m_node->GetNDevices(); i++)
					{
						Ptr<NetDevice> device = m_node->GetDevice(i);
						Ptr<OpticalDevice> optical = 
							device->GetObject<OpticalDevice>();
						if (optical)
						{
							auto iter = optical->m_sent_table.find(id);
							if (iter != optical->m_sent_table.end())
							{
								sched_item = iter->second;
								data = sched_item.packet;
								optical->m_sent_table.erase(iter);
								read = data->RemoveHeader(copy_header);
								NS_ASSERT_MSG(read, "Saved msg no header.");
								NS_ASSERT_MSG(copy_header.GetMsgId() == id,
									"Saved message did not match AWK/NACK.");
								found = true;
								break;
							}
						}
					}
					
					// NACK
					if (found && msg_type == 2)
					{
						Simulator::Cancel(sched_item.schedule_event);
						Simulator::Cancel(sched_item.check_event);
						InternalSend(data, protocol, id);
					}
					// AWK
					else if (msg_type == 3)
					{
						//Do Nothing, message succeeded.
					}
				}
				// Invalid
				else
				{
					m_dropTrace(p);
				}
				return;
			}
			else
			{
				uint32_t read = p->RemoveHeader(header);
				Ptr<Packet> copy = p->Copy();
				NS_ASSERT_MSG(read > 0, "Packet did not have optical header.");
				uint32_t id = header.GetMsgId();
				uint16_t protocol = header.GetProtocol(); 
				auto it_e = std::find(m_expected.begin(), m_expected.end(), id);
				auto it_r = std::find(m_received.begin(), m_received.end(), id);
				bool dropped = tag.IsDropped();
				static std::random_device rd;
				static std::mt19937 gen(rd());
				std::uniform_real_distribution<float> dis(0.0, 1.0);
				bool failed = dis(gen) < m_failure_rate;
				if (it_e != m_expected.end() && it_r == m_received.end() &&
					!dropped && !failed)
				{
					m_expected.erase(it_e);
					m_received.push_back(id);
					if (!m_promiscCallback.IsNull())
					{
						m_promiscCallback(this,
										  p,
										  protocol,
										  GetRemote(),
										  GetAddress(),
										  NetDevice::PACKET_HOST);
					}
					Simulator::Schedule(m_optical_processing,
										&OpticalDevice::FinalCallback,
										this,
										p,
										protocol);
					Simulator::Schedule(m_optical_processing,
										&OpticalDevice::SendCTRL,
										this,
										copy,
										protocol,
										id,
										3);
				}
				else
				{
					if (it_r == m_received.end())
					{
						SendCTRL(copy, protocol, id, 2);
					}
					m_dropTrace(p);
				}
			}
		}
		else
		{
			if (channel == 0)
			{
				p->RemoveHeader(header);
				Ipv4Header ipv4_header;
				uint32_t read = p->RemoveHeader(ipv4_header);
				NS_ASSERT_MSG(read > 0, "Sent message has no ipv4.");
				UdpHeader udp_header;
				read = p->RemoveHeader(udp_header);
				NS_ASSERT_MSG(read > 0, "Send message has no udp.");
				uint32_t data_size = p->GetSize();
				uint8_t* buffer = new uint8_t[data_size];
				NS_ASSERT_MSG(data_size == 26, "Data is wrong size.");
				p->CopyData(buffer, data_size);
				uint16_t protocol = header.GetProtocol();
				
				int dev = GetIfIndex();
				NS_ASSERT_MSG(dev >= 0, "Invalid device id.");
				std::memcpy(&buffer[22], &dev, 4);
				
				Ptr<Packet> new_packet = Create<Packet>(buffer, 26);
				new_packet->AddHeader(udp_header);
				new_packet->AddHeader(ipv4_header);
				CopyTags(p, new_packet);
				delete[] buffer;
				
				if (!m_promiscCallback.IsNull())
				{
					m_promiscCallback(this,
									  new_packet,
									  protocol,
									  GetRemote(),
									  GetAddress(),
									  NetDevice::PACKET_HOST);
				}
				m_rxCallback(this, new_packet, protocol, GetRemote());
			}
			else
			{
				NS_ASSERT_MSG(!m_schedule_table.empty(),
							  "Schedule should not be empty.");
				std::vector<int> entry = m_schedule_table.begin()->second;
				int dev_idx = entry[channel];
				if (dev_idx >= 0)
				{
					Ptr<NetDevice> ndev = m_node->GetDevice(dev_idx);
					Ptr<OpticalDevice> dev = DynamicCast<OpticalDevice>(ndev);
					dev->PassThrough(p, this);
				}
				else
				{
					m_dropTrace(p->Copy());
				}
			}
		}
	}

	void
	OpticalDevice::UpdateReceived(Ptr<Packet> copy, uint16_t protocol,
								  uint32_t id)
	{
		NS_LOG_FUNCTION(this << copy << protocol << id);
		auto it_e = std::find(m_expected.begin(),
			m_expected.end(), id);
		auto it_r = std::find(m_received.begin(), 
			m_received.end(), id);
		if (it_r == m_received.end())
		{
			if (it_e == m_expected.end())
			{
				m_expected.push_back(id);
			}
		}
		else
		{
			SendCTRL(copy, protocol, id, 3);
		}
	}

	void
	OpticalDevice::FinalCallback(Ptr<Packet> packet, uint16_t protocol)
	{
		NS_LOG_FUNCTION(this << packet << protocol);
		m_rxCallback(this, packet, protocol, GetRemote());
	}

	void
	OpticalDevice::SendCTRL(Ptr<Packet> copy, uint16_t protocol, uint32_t id,
							uint8_t msg_type)
	{
		NS_LOG_FUNCTION(this);
		Ipv4Header ipv4_header;
		uint32_t read = copy->RemoveHeader(ipv4_header);
		NS_ASSERT_MSG(read > 0, "Sent message has no ipv4.");
		UdpHeader udp_header;
		read = copy->RemoveHeader(udp_header);
		NS_ASSERT_MSG(read > 0, "Send message has no udp.");

		int dev = GetIfIndex();
		NS_ASSERT_MSG(dev >= 0, "Invalid device id.");
		uint64_t message_send = 0;
		uint64_t duration = 0;
		uint64_t channel = 1;

		uint8_t buffer[26];
		std::memcpy(&buffer[0], &msg_type, 1); 
		std::memcpy(&buffer[1], &id, 4);
		std::memcpy(&buffer[5], &message_send, 8);
		std::memcpy(&buffer[13], &duration, 8);
		std::memcpy(&buffer[21], &channel, 1);
		std::memcpy(&buffer[22], &dev, 4);
		Ptr<Packet> ctrl = Create<Packet>(buffer, 26);
		
		Ipv4Address p_src = ipv4_header.GetSource();
		Ipv4Address p_dest = ipv4_header.GetDestination();
		ipv4_header.SetSource(p_dest);
		ipv4_header.SetDestination(p_src);
		ipv4_header.SetPayloadSize(34);
		
		ctrl->AddHeader(udp_header);
		ctrl->AddHeader(ipv4_header);

		Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
		uint64_t current = node->GetLocalTime().GetNanoSeconds();
		AddOpticalHeader(ctrl, id, protocol, current);
		
		OpticalTag tag;
		tag.SetChannel(0);
		tag.SetMsgId(id);
		ctrl->AddPacketTag(tag);
		CopyTags(copy, ctrl);

		ControlSend(ctrl);
	}

	void
	OpticalDevice::SetIsEndpoint(bool is_endpoint)
	{
		NS_LOG_FUNCTION(this << is_endpoint);
		m_is_endpoint = is_endpoint;
	}

	bool
	OpticalDevice::IsEndpoint() const
	{
		return m_is_endpoint;
	}

	void
	OpticalDevice::DoDispose()
	{
		NS_LOG_FUNCTION(this);
		m_node = nullptr;
		m_channel = nullptr;
		NetDevice::DoDispose();
	}

	Address
	OpticalDevice::GetRemote() const
	{
		Ptr<NetDevice> dev = (m_channel->GetDevice(0) == this) ? 
				m_channel->GetDevice(1) : m_channel->GetDevice(0);
		return dev->GetAddress();
	}
	void
	OpticalDevice::AddControlHeader(Ptr<Packet> p,
									uint16_t protocol,
									uint32_t id,
									uint64_t send_timestamp,
									uint64_t message_timestamp,
									uint8_t channel)
	{
		NS_LOG_FUNCTION(this << id << send_timestamp << message_timestamp <<
						channel);
		OpticalControlHeader header;
		header.SetMsgId(id);
		header.SetSendTimestamp(send_timestamp);
		header.SetMessageTimestamp(message_timestamp);
		header.SetChannel(channel);
		header.SetProtocol(protocol);
		header.SetDuration(
			m_data_bps.CalculateBytesTxTime(p->GetSize()).GetNanoSeconds());
		p->AddHeader(header);
	}

	void
	OpticalDevice::AddDataHeader(Ptr<Packet> p,
								 uint16_t protocol,
								 uint32_t id,
								 uint64_t send_timestamp)
	{
		NS_LOG_FUNCTION(this << p << id << send_timestamp);
		OpticalDataHeader header;
		header.SetMsgId(id);
		header.SetSendTimestamp(send_timestamp);
		header.SetProtocol(protocol);
		p->AddHeader(header);
	}

	void
	OpticalDevice::AddOpticalHeader(Ptr<Packet> p,
									uint32_t id,
									uint16_t protocol,
									uint64_t send_timestamp)
	{
		NS_LOG_FUNCTION(this << p << id << protocol << send_timestamp);
		OpticalHeader header;
		header.SetProtocol(protocol);
		header.SetTimestamp(send_timestamp);
		header.SetMsgId(id);
		p->AddHeader(header);
	}
	
	void
	OpticalDevice::ControlTransmitStart(Ptr<Packet> p)
	{
		NS_LOG_FUNCTION(this << p);
		NS_ASSERT_MSG(!m_is_transmitting_control, "Should not be transmitting");
		m_is_transmitting_control = true;
		Time tx_time = m_control_bps.CalculateBytesTxTime(p->GetSize());
		Time total_time = tx_time + m_packet_processing + 
			m_control_frame_gap;
		m_txTrace(p->Copy());
		Simulator::Schedule(m_packet_processing,
							&OpticalChannel::TransmitStart,
							m_channel,
							p,
							this,
							tx_time);
		Simulator::Schedule(total_time, 
							&OpticalDevice::ControlTransmitComplete,
							this);
	}

	void
	OpticalDevice::ControlTransmitComplete()
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(m_is_transmitting_control, "Should be transmitting");
		m_is_transmitting_control = false;
		Ptr<Packet> p = m_control_queue->Dequeue();
		if (p)
		{
			ControlTransmitStart(p);
		}
	}

	void
	OpticalDevice::DataTransmitStart(Ptr<Packet> p)
	{

		NS_LOG_FUNCTION(this << p);
		OpticalTag tag;
		bool found_tag = p->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Packet did not have optical tag.");
		uint8_t channel = tag.GetChannel();
		NS_ASSERT_MSG(channel > 0, "There should be no control messages.");
		if (!m_is_transmitting_data)
		{
			m_is_transmitting_data = true;
			Time tx_time = m_data_bps.CalculateBytesTxTime(p->GetSize());
			Time total_time = tx_time + m_data_frame_gap;
			Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
			
			m_txTrace(p->Copy());
			m_channel->PassThrough(p, this, tx_time);
			Simulator::Schedule(total_time, 
								&OpticalDevice::DataTransmitComplete,
								this);
		}
	}

	void
	OpticalDevice::DataTransmitComplete()
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(m_is_transmitting_data, "Should be transmitting");
		m_is_transmitting_data = false;
	}

	void
	OpticalDevice::PassThrough(Ptr<Packet> p, Ptr<OpticalDevice> src)
	{
		NS_LOG_FUNCTION(this << p);
		if (!m_is_reconfiguring)
		{
			OpticalTag tag;
			bool found_tag = p->PeekPacketTag(tag);
			NS_ASSERT_MSG(found_tag, "Packet should have optical tag.");
			uint8_t channel = tag.GetChannel();
			uint32_t id = tag.GetMsgId();
			Time tx_time = m_data_bps.CalculateBytesTxTime(p->GetSize());
			NS_ASSERT_MSG(channel >= 0 && channel <= m_channel->GetNChannels(),
						  "Channel is not valid.");
			
			m_packet_map[channel].insert({id, p});
			if (m_channels[channel] != 0)
			{
				m_collisionTrace(src, p);
				for (uint32_t i = 0; i < m_node->GetNDevices(); i++)
				{
					Ptr<NetDevice> device = m_node->GetDevice(i);
					Ptr<OpticalDevice> optical = 
						device->GetObject<OpticalDevice>();
					if (optical)
					{
						for (auto& item : optical->m_packet_map[channel])
						{
							found_tag = item.second->RemovePacketTag(tag);
							NS_ASSERT_MSG(found_tag, 
								"Packet has no optical tag.");
							tag.DropPacket();
							item.second->AddPacketTag(tag);
						}
					}
				}
			}
			src->m_channels[channel]++;
			
			Simulator::Schedule(m_switch_propagation_delay + tx_time,
								&OpticalDevice::PassThroughFinish,
								this,
								p,
								src);
			Simulator::Schedule(m_switch_propagation_delay,
								&OpticalChannel::PassThrough,
								m_channel,
								p,
								this,
								tx_time);
		}
		else
		{
			m_dropTrace(p->Copy());
		}
	}

	void
	OpticalDevice::PassThroughFinish(Ptr<Packet> p, Ptr<OpticalDevice> src)
	{
		NS_LOG_FUNCTION(this << p);
		OpticalTag tag;
		bool found_tag = p->PeekPacketTag(tag);
		NS_ASSERT_MSG(found_tag, "Packet did not have optial tag.");
		uint32_t id = tag.GetMsgId();
		uint8_t channel = tag.GetChannel();
		NS_ASSERT_MSG(src->m_channels[channel] > 0,
					  "Error: Pass Through on unused channel.");
		src->m_channels[channel]--;
		NS_ASSERT_MSG(!m_packet_map[channel].empty(),
					  "Should have packets on queue.");
		auto iter = m_packet_map[channel].find(id);
		NS_ASSERT_MSG(iter != m_packet_map[channel].end(),
					  "Did not find packet in map.");
		m_packet_map[channel].erase(iter);
		// erase from dest
		auto& tx_list = m_tx_list.begin()->second[channel];
		for (auto item = tx_list.begin(); item != tx_list.end(); ++item)
		{
			if (item->id == id)
			{	
				item = tx_list.erase(item);
				break;
			}
		}
		// erase from src
		tx_list = src->m_tx_list.begin()->second[channel];
		for (auto item = tx_list.begin(); item != tx_list.end(); ++item)
		{
			if (item->id == id)
			{	
				item = tx_list.erase(item);
				break;
			}
		}
	}

	void
	OpticalDevice::UpdateRoutes(bool first)
	{
		NS_LOG_FUNCTION(this << first);
		m_route_table.clear();
		for (uint32_t i = 0; i < m_node->GetNDevices(); i++)
		{
			Ptr<NetDevice> device = m_node->GetDevice(i);
			if (device->GetInstanceTypeId() == OpticalDevice::GetTypeId())
			{
				Ptr<OpticalDevice> src_dev = 
					DynamicCast<OpticalDevice>(device);
				Mac48Address dest = 
					Mac48Address::ConvertFrom(src_dev->GetRemote());
				int dev = static_cast<int>(i);
				m_route_table.insert({dest, dev});

				if (first && src_dev != this)
				{
					src_dev->UpdateRoutes(false);
				}
			}
		}
		BeginReconfigure();
	}

	bool
	OpticalDevice::ScheduleMessage(Time arrival, Address dest, uint32_t id, 
								   uint8_t channel, Time tx_delay, int from)
	{
		NS_LOG_FUNCTION(this << arrival << dest << channel);
		bool success = false;
		Ptr<NetDevice> base_dev = m_node->GetDevice(from);
		Ptr<OpticalDevice> from_dev = DynamicCast<OpticalDevice>(base_dev);
		Mac48Address mac = Mac48Address::ConvertFrom(dest);
		auto iter = from_dev->m_route_table.find(mac);
		NS_ASSERT_MSG(iter != from_dev->m_route_table.end(), 
					  "Address not found.");
		int dev = iter->second;
		NS_ASSERT_MSG(dev >= 0, "Invalid device id.");
		for (auto& pair : from_dev->m_schedule_table)
		{
			Time timeslot = pair.first;
			if (arrival >= timeslot && 
				arrival + tx_delay <= timeslot + m_timeslot_duration)
			{
				// Check if source clear
				auto src_iter = from_dev->m_tx_list.find(timeslot);
				NS_ASSERT_MSG(src_iter != from_dev->m_tx_list.end(), 
							  "Tx list should have same timeslots as sched.");
				for(TransmissionItem& item : src_iter->second[channel])
				{
					Time a1 = item.arrival;
					Time e1 = item.exit;
					Time a2 = arrival;
					Time e2 = arrival + tx_delay;
					if ((a1 >= a2 && a1 <= e2) || (e1 >= a2 && e1 <= e2))
					{
						return false;
					}
				}
				// Check if dest clear
				auto dest_iter = m_tx_list.find(timeslot);
				NS_ASSERT_MSG(dest_iter != m_tx_list.end(), 
							  "Tx list should have same timeslots as sched.");
				for(TransmissionItem& item : dest_iter->second[channel])
				{
					Time a1 = item.arrival;
					Time e1 = item.exit;
					Time a2 = arrival;
					Time e2 = arrival + tx_delay;
					if ((a1 >= a2 && a1 <= e2) || (e1 >= a2 && e1 <= e2))
					{
						return false;
					}
				}
				
				auto& entry = pair.second;
				if (entry[channel] == -1)
				{
					entry[channel] = dev;
				}
				success = entry[channel] == dev;
				
				// Add packet to dest and src
				if (success)
				{
					TransmissionItem item;
					item.id = id;
					item.arrival = arrival;
					item.exit = arrival + tx_delay;
					src_iter->second[channel].push_back(item);
					dest_iter->second[channel].push_back(item);
				}
				break;
			}
		}
		return success;
	}

	int
	OpticalDevice::GetOpticalRoute(uint8_t channel)
	{
		NS_LOG_FUNCTION(this << channel);
		return m_schedule_table.begin()->second[channel];
	}

	void
	OpticalDevice::BeginReconfigure()
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(!m_is_reconfiguring, "Should not be reconfiguring");
		m_is_reconfiguring = true;
		Ptr<TimeNode> node = DynamicCast<TimeNode>(m_node);
		Time current = node->GetLocalTime() + m_reconfigure_time;
		if (m_schedule_table.empty())
		{
			for (int t = 0; t < m_schedule_size; t++)
			{
				Time timeslot = 
					current + ((m_reconfigure_time + m_timeslot_duration) * t);
				std::vector<int> sched_entry;
				std::vector<std::vector<TransmissionItem>> tx_entry;
				for (size_t d = 0; d <= m_channel->GetNChannels(); d++)
				{
					sched_entry.push_back(-1);
					std::vector<TransmissionItem> tx_item;
					tx_entry.push_back(tx_item);
				}
				m_schedule_table.insert({timeslot, sched_entry});
				m_tx_list.insert({timeslot, tx_entry});
			}
		}
		else
		{
			m_schedule_table.erase(m_schedule_table.begin());
			m_tx_list.erase(m_tx_list.begin());
			for (const auto& it : m_schedule_table)
			{
				Time slot = it.first;
				auto values = it.second;
				NS_ASSERT_MSG(slot == slot, "slot exists");
				NS_ASSERT_MSG(values == values, "values exist");
			}
			Time timeslot = current + ((m_reconfigure_time + 
				m_timeslot_duration) * (m_schedule_size - 1));
			std::vector<int> sched_entry;
			std::vector<std::vector<TransmissionItem>> tx_entry;
			for (size_t d = 0; d <= m_channel->GetNChannels(); d++)
			{
				sched_entry.push_back(-1);
				std::vector<TransmissionItem> tx_item;
				tx_entry.push_back(tx_item);
			}
			m_schedule_table.insert({timeslot, sched_entry});
			m_tx_list.insert({timeslot, tx_entry});
		}

		if (m_next_transmit < m_schedule_table.begin()->first)
		{
			m_next_transmit = m_schedule_table.begin()->first;
		}

		Simulator::Schedule(m_reconfigure_time, 
							&OpticalDevice::CompleteReconfigure,
							this);
		Simulator::Schedule(m_timeslot_duration + m_reconfigure_time,
							&OpticalDevice::BeginReconfigure,
							this);
	}

	void
	OpticalDevice::CompleteReconfigure()
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(m_is_reconfiguring, "Should be reconfiguring.");
		m_is_reconfiguring = false;
	}

	void
	OpticalDevice::UpdateChannels()
	{
		NS_LOG_FUNCTION(this);
		uint8_t channels = m_channel->GetNChannels();
		m_channels.clear();
		m_packet_map.clear();
		for (int i = 0; i <= channels; i++)
		{
			m_channels.push_back(0);
			std::map<uint32_t, Ptr<Packet>> packet_map;
			m_packet_map.push_back(packet_map);
		}
	}
}
