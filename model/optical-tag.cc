#include "optical-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED(OpticalTag);

	TypeId
	OpticalTag::GetTypeId()
	{
		static TypeId tid = 
			TypeId("ns3::OpticalTag")
				.SetParent<Tag>()
				.SetGroupName("QuantumNetwork")
				.AddConstructor<OpticalTag>();
		return tid;
	}

	TypeId
	OpticalTag::GetInstanceTypeId() const
	{
		return GetTypeId();
	}

	OpticalTag::OpticalTag()
	{
	}

	OpticalTag::OpticalTag(uint8_t channel)
		: m_channel(channel)
	{
		m_dropped = 0;
	}

	OpticalTag::~OpticalTag()
	{
	}

	uint32_t
	OpticalTag::GetSerializedSize() const
	{
		return 6;
	}

	void
	OpticalTag::Serialize(TagBuffer i) const
	{
		i.WriteU8(m_channel);
		i.WriteU32(m_msg_id);
		i.WriteU8(m_dropped);
	}

	void
	OpticalTag::Deserialize(TagBuffer i)
	{
		m_channel = i.ReadU8();
		m_msg_id = i.ReadU32();
		m_dropped = i.ReadU8();
	}

	void
	OpticalTag::Print(std::ostream& os) const
	{
		os << "Channel: " << m_channel << ", ID: " << m_msg_id << 
			", Dropped: " << m_dropped;
	}

	void
	OpticalTag::SetChannel(uint8_t channel)
	{
		m_channel = channel;
	}

	void
	OpticalTag::SetMsgId(uint32_t id)
	{
		m_msg_id = id;
	}

	uint8_t
	OpticalTag::GetChannel() const
	{
		return m_channel;
	}

	uint32_t
	OpticalTag::GetMsgId() const
	{
		return m_msg_id;
	}

	void
	OpticalTag::DropPacket()
	{
		m_dropped = 1;
	}

	bool
	OpticalTag::IsDropped()
	{
		return m_dropped;
	}
}
