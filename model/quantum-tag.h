#ifndef QUANTUM_TAG_H
#define QUANTUM_TAG_H

#include "ns3/tag.h"

namespace ns3
{
	class QuantumTag : public Tag
	{
		public:
			static TypeId GetTypeId();
			TypeId GetInstanceTypeId() const override;
			QuantumTag();
			/*
			Instead of listing specific wavelengths, will have index for
			which channel the packet is on. Index of 0 is control.
			*/
			QuantumTag(bool is_quantum); 
			~QuantumTag() override;
			void Serialize(TagBuffer i) const override;
			void Deserialize(TagBuffer i) override;
			uint32_t GetSerializedSize() const override;
			void Print(std::ostream& os) const override;
			bool IsQuantum() const;
		private:
			bool m_is_quantum;
	};
}

#endif
