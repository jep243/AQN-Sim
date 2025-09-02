#ifndef OPTICAL_CONTROL_HEADER_H
#define OPTICAL_CONTROL_HEADER_H

#include "ns3/header.h"

namespace ns3
{
	class OpticalControlHeader : public Header
	{
		public:
			OpticalControlHeader();
			~OpticalControlHeader();
			static TypeId GetTypeId();
			TypeId GetInstanceTypeId() const override;
			void Print(std::ostream& os) const override;
			uint32_t GetSerializedSize() const override;
			void Serialize(Buffer::Iterator start) const override;
			uint32_t Deserialize(Buffer::Iterator start) override;
			
			void SetMsgId(uint32_t id);
			void SetSendTimestamp(uint64_t timestamp);
			void SetMessageTimestamp(uint64_t timestamp);
			void SetChannel(uint8_t channel);
			void SetProtocol(uint16_t protocol);
			void SetDuration(uint64_t duration);
			uint32_t GetMsgId() const;
			uint64_t GetSendTimestamp() const;
			uint64_t GetMessageTimestamp() const;
			uint8_t GetChannel() const;
			uint16_t GetProtocol() const;
			uint64_t GetDuration() const;
		private:
			uint32_t m_msg_id;
			uint64_t m_send_timestamp;
			uint64_t m_message_timestamp;
			uint64_t m_message_duration;
			uint8_t m_channel;
			uint16_t m_protocol;
	};
}

#endif
