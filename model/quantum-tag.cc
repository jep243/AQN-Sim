#include "quantum-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED(QuantumTag);

	TypeId
	QuantumTag::GetTypeId()
	{
		static TypeId tid = 
			TypeId("ns3::QuantumTag")
				.SetParent<Tag>()
				.SetGroupName("QuantumNetwork")
				.AddConstructor<QuantumTag>();
		return tid;
	}

	TypeId
	QuantumTag::GetInstanceTypeId() const
	{
		return GetTypeId();
	}

	QuantumTag::QuantumTag()
	{
	}

	QuantumTag::QuantumTag(bool is_quantum)
		: m_is_quantum(is_quantum)
	{
	}

	QuantumTag::~QuantumTag()
	{
	}

	uint32_t
	QuantumTag::GetSerializedSize() const
	{
		return 1;
	}

	void
	QuantumTag::Serialize(TagBuffer i) const
	{
		i.WriteU8(m_is_quantum);
	}

	void
	QuantumTag::Deserialize(TagBuffer i)
	{
		m_is_quantum = i.ReadU8();
	}

	void
	QuantumTag::Print(std::ostream& os) const
	{
		os << m_is_quantum;
	}

	bool
	QuantumTag::IsQuantum() const
	{
		return m_is_quantum;
	}
}
