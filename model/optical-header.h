#ifndef OPTICAL_HEADER_H
#define OPTICAL_HEADER_H

#include "ns3/header.h"

namespace ns3
{
	class OpticalHeader : public Header
	{
		public:
			OpticalHeader();
			~OpticalHeader();
			static TypeId GetTypeId();
			TypeId GetInstanceTypeId() const override;
			void Print(std::ostream& os) const override;
			uint32_t GetSerializedSize() const override;
			void Serialize(Buffer::Iterator start) const override;
			uint32_t Deserialize(Buffer::Iterator start) override;

			void SetProtocol(uint16_t protocol);
			uint16_t GetProtocol() const;
			void SetTimestamp(uint64_t timestamp);
			uint64_t GetTimestamp() const;
			void SetMsgId(uint32_t id);
			uint32_t GetMsgId() const;
		private:
			uint16_t m_protocol;
			uint64_t m_timestamp;
			uint32_t m_id;
	};
}

#endif
