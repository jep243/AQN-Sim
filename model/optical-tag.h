#ifndef OPTICAL_TAG_H
#define OPTICAL_TAG_H

#include "ns3/tag.h"

namespace ns3
{
	class OpticalTag : public Tag
	{
		public:
			static TypeId GetTypeId();
			TypeId GetInstanceTypeId() const override;
			OpticalTag();
			/*
			Instead of listing specific wavelengths, will have index for
			which channel the packet is on. Index of 0 is control.
			*/
			OpticalTag(uint8_t channel); 
			~OpticalTag() override;
			void Serialize(TagBuffer i) const override;
			void Deserialize(TagBuffer i) override;
			uint32_t GetSerializedSize() const override;
			void Print(std::ostream& os) const override;
			void SetChannel(uint8_t channel);
			void SetMsgId(uint32_t id);
			uint8_t GetChannel() const;
			uint32_t GetMsgId() const;
			void DropPacket();
			bool IsDropped();
		private:
			uint8_t m_channel;
			uint32_t m_msg_id;
			uint8_t m_dropped = 0;
	};
}

#endif
