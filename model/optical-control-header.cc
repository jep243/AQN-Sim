#include "ns3/optical-control-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED(OpticalControlHeader);
	
	TypeId
	OpticalControlHeader::GetTypeId()
	{
		static TypeId tid = TypeId("ns3::OpticalControlHeader")
								.SetParent<Header>()
								.SetGroupName("QuantumNetwork")
								.AddConstructor<OpticalControlHeader>();
		return tid;
	}

	TypeId
	OpticalControlHeader::GetInstanceTypeId() const
	{
		return GetTypeId();
	}

	OpticalControlHeader::OpticalControlHeader()
	{

	}

	OpticalControlHeader::~OpticalControlHeader()
	{

	}

	void
	OpticalControlHeader::Print(std::ostream& os) const
	{
		os << "ID: " << m_msg_id << ", Send Timestamp: " << m_send_timestamp <<
			", Message Timestamp: " << m_message_timestamp <<
			", Channel: " << m_channel << ", Protocol: " << m_protocol <<
			", Duration: " << m_message_duration;
	}

	uint32_t
	OpticalControlHeader::GetSerializedSize() const
	{
		return 31;
	}

	void
	OpticalControlHeader::Serialize(Buffer::Iterator start) const
	{
		start.WriteU32(m_msg_id);
		start.WriteU64(m_send_timestamp);
		start.WriteU64(m_message_timestamp);
		start.WriteU8(m_channel);
		start.WriteU16(m_protocol);
		start.WriteU64(m_message_duration);
	}

	uint32_t
	OpticalControlHeader::Deserialize(Buffer::Iterator start)
	{
		m_msg_id = start.ReadU32();
		m_send_timestamp = start.ReadU64();
		m_message_timestamp = start.ReadU64();
		m_channel = start.ReadU8();
		m_protocol = start.ReadU16();
		m_message_duration = start.ReadU64();
		return 31;
	}

	void
	OpticalControlHeader::SetMsgId(uint32_t id)
	{
		m_msg_id = id;
	}

	void
	OpticalControlHeader::SetSendTimestamp(uint64_t timestamp)
	{
		m_send_timestamp = timestamp;
	}

	void
	OpticalControlHeader::SetMessageTimestamp(uint64_t timestamp)
	{
		m_message_timestamp = timestamp;
	}

	void
	OpticalControlHeader::SetChannel(uint8_t channel)
	{
		m_channel = channel;
	}

	void
	OpticalControlHeader::SetProtocol(uint16_t protocol)
	{
		m_protocol = protocol;
	}

	void
	OpticalControlHeader::SetDuration(uint64_t duration)
	{
		m_message_duration = duration;
	}

	uint32_t
	OpticalControlHeader::GetMsgId() const
	{
		return m_msg_id;
	}

	uint64_t
	OpticalControlHeader::GetSendTimestamp() const
	{
		return m_send_timestamp;
	}

	uint64_t
	OpticalControlHeader::GetMessageTimestamp() const
	{
		return m_message_timestamp;
	}

	uint8_t
	OpticalControlHeader::GetChannel() const
	{
		return m_channel;
	}

	uint16_t
	OpticalControlHeader::GetProtocol() const
	{
		return m_protocol;
	}

	uint64_t
	OpticalControlHeader::GetDuration() const
	{
		return m_message_duration;
	}
}
