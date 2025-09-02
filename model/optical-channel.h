#ifndef OPTICAL_CHANNEL_H
#define OPTICAL_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <vector>
#include <list>
#include <map>

namespace ns3
{
	class OpticalDevice;

	class OpticalChannel : public Channel
	{
		public:
			static TypeId GetTypeId();
			OpticalChannel();
			~OpticalChannel();
			void Attach(Ptr<OpticalDevice> device);
			void TransmitStart(Ptr<Packet> p,
							   Ptr<OpticalDevice> src,
							   Time txTime);
			void PassThrough(Ptr<Packet> p,
							 Ptr<OpticalDevice> src,
							 Time tx_time);
			std::size_t GetNDevices() const override;
			Ptr<NetDevice> GetDevice(std::size_t i) const override;
			uint8_t GetNChannels() const;
			void SetNChannels(uint8_t channels);
			Time GetDelay() const;
		protected:
			void PassThroughFinished(Ptr<OpticalDevice> src,
									 Ptr<Packet> packet,
									 uint8_t channel);
		private:
			Time m_delay;
			std::size_t m_num_devices;
			uint8_t m_num_channels;
			Ptr<OpticalDevice> m_dev0;
			Ptr<OpticalDevice> m_dev1;
			std::vector<std::map<uint32_t, Ptr<Packet>>> m_packet_map;
			std::vector<uint8_t> m_dev0_channels;
			std::vector<uint8_t> m_dev1_channels;

			TracedCallback<Ptr<const OpticalDevice>, Ptr<const Packet>> 
				m_collisionTrace;
	};
}

#endif
