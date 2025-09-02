#ifndef OPTICAL_DATA_HEADER_H
#define OPTICAL_DATA_HEADER_H

#include "ns3/header.h"

namespace ns3
{
	class OpticalDataHeader : public Header
	{
		public:
			OpticalDataHeader();
			~OpticalDataHeader();
			static TypeId GetTypeId();
			TypeId GetInstanceTypeId() const override;
			void Print(std::ostream& os) const override;
			uint32_t GetSerializedSize() const override;
			void Serialize(Buffer::Iterator start) const override;
			uint32_t Deserialize(Buffer::Iterator start) override;
			
			void SetMsgId(uint32_t id);
			void SetSendTimestamp(uint64_t timestamp);
			void SetProtocol(uint16_t protocol);
			uint32_t GetMsgId();
			uint64_t GetSendTimestamp();
			uint16_t GetProtocol();
		private:
			uint32_t m_msg_id;
			uint16_t m_protocol;
			uint64_t m_send_timestamp;
	};
}

#endif
