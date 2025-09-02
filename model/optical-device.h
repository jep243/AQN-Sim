#ifndef OPTICAL_DEVICE_H
#define OPTICAL_DEVICE_H

#include "ns3/address.h"
#include "ns3/callback.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/optical-channel.h"
#include "ns3/optical-control-header.h"
#include "ns3/queue.h"
#include "ns3/object-factory.h"

#include <map>
#include <vector>
#include <list>

namespace ns3
{
	class ScheduleItem
	{
		public:
			Ptr<Packet> packet;
			EventId schedule_event;
			EventId check_event;
	};

	class TransmissionItem
	{
		public:
			uint32_t id;
			Time arrival;
			Time exit;
	};

	class OpticalDevice : public NetDevice
	{
		public:
			static TypeId GetTypeId();
			OpticalDevice();
			~OpticalDevice();

			void SetIfIndex(const uint32_t index) override;
			uint32_t GetIfIndex() const override;
			Ptr<Channel> GetChannel() const override;
			void SetAddress(Address address) override;
			Address GetAddress() const override;
			bool SetMtu(const uint16_t mtu) override;
			uint16_t GetMtu() const override;
			bool IsLinkUp() const override;
			void NotifyLinkUp();
			void AddLinkChangeCallback(Callback<void> callback) override;
			bool IsBroadcast() const override;
			Address GetBroadcast() const override;
			bool IsMulticast() const override;
			Address GetMulticast(Ipv4Address multicastGroup) const override;
			bool IsPointToPoint() const override;
			bool IsBridge() const override;
			bool Send(Ptr<Packet> packet,
					  const Address& dest,
					  uint16_t protocolNumber) override;
			bool SendFrom(Ptr<Packet> packet,
						  const Address& source,
						  const Address& dest,
						  uint16_t protocolNumber) override;
			Ptr<Node> GetNode() const override;
			void SetNode(Ptr<Node> node) override;
			bool NeedsArp() const override;
			void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
			Address GetMulticast(Ipv6Address addr) const override;
			void SetPromiscReceiveCallback(
					NetDevice::PromiscReceiveCallback cb) override;
			bool SupportsSendFrom() const override;

			void SetControlDataRate(DataRate bps);
			void SetDataDataRate(DataRate bps);
			uint64_t GetControlDataRate();
			uint64_t GetDataDataRate();
			void SetControlFrameGap(Time t);
			void SetDataFrameGap(Time t);
			void SetPacketDelay(Time t);
			void SetReconfigureTime(Time t);
			void SetTimeslotDuration(Time t);
			bool Attach(Ptr<OpticalChannel> ch);
			void SetControlQueue(Ptr<Queue<Packet>> queue);
			void Receive(Ptr<Packet> p);
			void SetIsEndpoint(bool is_endpoint);
			bool IsEndpoint() const;

			void SetDeviceId(uint16_t id);
			uint16_t GetDeviceId() const;
			
			void UpdateRoutes(bool first);
			void UpdateChannels();
		private:
			void DoDispose() override;
			Address GetRemote() const;
			void AddControlHeader(Ptr<Packet> p,
								  uint16_t protocol,
								  uint32_t id,
								  uint64_t send_timestamp,
								  uint64_t message_timestamp,
								  uint8_t channel);
			void AddDataHeader(Ptr<Packet> p, 
							   uint16_t protocol,
							   uint32_t id, 
							   uint64_t send_timestamp);
			void AddOpticalHeader(Ptr<Packet> p,
								  uint32_t id,
								  uint16_t protocol,
								  uint64_t send_timestamp);
			void ControlTransmitStart(Ptr<Packet> p);
			void ControlTransmitComplete();
			void DataTransmitStart(Ptr<Packet> p);
			void DataTransmitComplete();
			void PassThrough(Ptr<Packet> p, Ptr<OpticalDevice> src);
			void PassThroughFinish(Ptr<Packet> p, Ptr<OpticalDevice> src);
			bool InternalSend(Ptr<Packet> packet, uint16_t protocol, 
							  uint32_t id);
			bool ControlSend(Ptr<Packet> packet);
			void CheckSent(uint32_t id);
			void FinalCallback(Ptr<Packet> p, uint16_t protocol);
			void SendCTRL(Ptr<Packet> copy, uint16_t protocol, uint32_t id,
						  uint8_t msg_type);
			Time SplitPacket(Ptr<Packet> data, Ptr<Packet>& control, 
							 uint16_t protocol, uint32_t id);
			void CopyTags(Ptr<Packet> original, Ptr<Packet> copy);
			Time GetPacketTransmitTime(Time& tx_ctrl, Time& tx_data);
			uint8_t GetRandomChannel();
			uint64_t BytesToUint64(uint8_t* buffer, int offset);
			uint32_t BytesToUint32(uint8_t* buffer, int offest);
			uint16_t BytesToUint16(uint8_t* buffer, int offest);
			void UpdateReceived(Ptr<Packet> copy, uint16_t protocol, 
								uint32_t id);

			uint16_t m_dev_id;
			uint16_t m_msg_count;
			
			float m_failure_rate;

			uint32_t m_if_index;
			Ptr<OpticalChannel> m_channel;
			bool m_is_endpoint;
			bool m_is_link_up;
			bool m_is_transmitting_control;
			bool m_is_transmitting_data; //Only for Endpoint
			bool m_is_reconfiguring;
			uint32_t m_mtu;
			DataRate m_control_bps;
			DataRate m_data_bps; //Only for Endpoint
			Time m_packet_delay;
			Time m_packet_processing;
			Time m_optical_processing;
			Time m_switch_propagation_delay;
			Time m_total_propagation_delay;
			Time m_reconfigure_time;
			Time m_timeslot_duration;
			Time m_control_frame_gap;
			Time m_data_frame_gap; //Only for Endpoint
			Ptr<Node> m_node;
			Mac48Address m_address;
			Ptr<Queue<Packet>> m_control_queue;

			TracedCallback<> m_linkChangeCallbacks;
			TracedCallback<Ptr<const Packet>> m_dropTrace;
			TracedCallback<Ptr<const Packet>> m_rxTrace;
			TracedCallback<Ptr<const Packet>> m_txTrace;
			TracedCallback<Ptr<const Packet>> m_passThroughTrace;
			TracedCallback<Ptr<const OpticalDevice>, Ptr<const Packet>>
				m_collisionTrace;
			NetDevice::ReceiveCallback m_rxCallback;
			NetDevice::PromiscReceiveCallback m_promiscCallback;

			std::vector<std::map<uint32_t, Ptr<Packet>>> m_packet_map;
			std::list<uint32_t> m_expected; //Only for Endpoint
			std::list<uint32_t> m_received; //Opnly for Endpoint
			std::map<uint32_t, ScheduleItem> m_sent_table; //Only for Endpoint
			std::vector<uint8_t> m_channels;
			Time m_next_transmit;
			uint16_t m_schedule_size;
			std::map<Mac48Address, int> m_route_table;
			std::map<Time, std::vector<int>> m_schedule_table;
			std::map<Time, std::vector<std::vector<TransmissionItem>>> 
				m_tx_list;
			bool ScheduleMessage(Time arrival, Address dest, uint32_t id, 
								 uint8_t channel, Time tx_delay, int from);
			int GetOpticalRoute(uint8_t channel);
			void BeginReconfigure();
			void CompleteReconfigure();
	};
}

#endif
