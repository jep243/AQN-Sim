#include "ns3/optical-data-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED(OpticalDataHeader);
	
	TypeId
	OpticalDataHeader::GetTypeId()
	{
		static TypeId tid = TypeId("ns3::OpticalDataHeader")
								.SetParent<Header>()
								.SetGroupName("QuantumNetwork")
								.AddConstructor<OpticalDataHeader>();
		return tid;
	}

	TypeId
	OpticalDataHeader::GetInstanceTypeId() const
	{
		return GetTypeId();
	}

	OpticalDataHeader::OpticalDataHeader()
	{

	}

	OpticalDataHeader::~OpticalDataHeader()
	{

	}

	void
	OpticalDataHeader::Print(std::ostream& os) const
	{
		os << "ID: " << m_msg_id << ", Send Timestamp: " << m_send_timestamp <<
		", Protocol: " << m_protocol;
	}

	uint32_t
	OpticalDataHeader::GetSerializedSize() const
	{
		return 13;
	}

	void
	OpticalDataHeader::Serialize(Buffer::Iterator start) const
	{
		start.WriteU32(m_msg_id);
		start.WriteU64(m_send_timestamp);
		start.WriteU16(m_protocol);
	}

	uint32_t
	OpticalDataHeader::Deserialize(Buffer::Iterator start)
	{
		m_msg_id = start.ReadU32();
		m_send_timestamp = start.ReadU64();
		m_protocol = start.ReadU16();
		return 13;
	}

	void
	OpticalDataHeader::SetMsgId(uint32_t id)
	{
		m_msg_id = id;
	}

	void
	OpticalDataHeader::SetSendTimestamp(uint64_t timestamp)
	{
		m_send_timestamp = timestamp;
	}

	void
	OpticalDataHeader::SetProtocol(uint16_t protocol)
	{
		m_protocol = protocol;
	}

	uint32_t
	OpticalDataHeader::GetMsgId()
	{
		return m_msg_id;
	}

	uint64_t
	OpticalDataHeader::GetSendTimestamp()
	{
		return m_send_timestamp;
	}

	uint16_t
	OpticalDataHeader::GetProtocol()
	{
		return m_protocol;
	}
}
