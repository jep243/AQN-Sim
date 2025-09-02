#include "ns3/optical-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED(OpticalHeader);

	TypeId
	OpticalHeader::GetTypeId()
	{
		static TypeId tid = TypeId("ns3::OpticalHeader")
								.SetParent<Header>()
								.SetGroupName("QuantumNetwork")
								.AddConstructor<OpticalHeader>();
		return tid;
	}

	TypeId
	OpticalHeader::GetInstanceTypeId() const
	{
		return GetTypeId();
	}

	OpticalHeader::OpticalHeader()
	{

	}

	OpticalHeader::~OpticalHeader()
	{

	}

	void
	OpticalHeader::Print(std::ostream& os) const
	{
		os << "Protocol: " << m_protocol << ", Timestamp: " << m_timestamp <<
			", ID: " << m_id;
	}

	uint32_t
	OpticalHeader::GetSerializedSize() const
	{
		return 14;
	}

	void
	OpticalHeader::Serialize(Buffer::Iterator start) const
	{
		start.WriteU16(m_protocol);
		start.WriteU64(m_timestamp);
		start.WriteU32(m_id);
	}

	uint32_t
	OpticalHeader::Deserialize(Buffer::Iterator start)
	{
		m_protocol = start.ReadU16();
		m_timestamp = start.ReadU64();
		m_id = start.ReadU32();
		return 14;
	}

	void
	OpticalHeader::SetProtocol(uint16_t protocol)
	{
		m_protocol = protocol;
	}

	uint16_t
	OpticalHeader::GetProtocol() const
	{
		return m_protocol;
	}

	void
	OpticalHeader::SetTimestamp(uint64_t timestamp)
	{
		m_timestamp = timestamp;
	}

	uint64_t
	OpticalHeader::GetTimestamp() const
	{
		return m_timestamp;
	}

	void
	OpticalHeader::SetMsgId(uint32_t id)
	{
		m_id = id;
	}

	uint32_t
	OpticalHeader::GetMsgId() const
	{
		return m_id;
	}
}
