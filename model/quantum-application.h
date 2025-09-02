#ifndef QUANTUM_APPLICATION_H
#define QUANTUM_APPLICATION_H

/**
 * @defgroup quantum-network
 * This module will add optical burst switching functionality and a quantum
 * application that will use the optical network. The quantum application
 * will use uniform traffic, teledata, and telegate protocols. The QNIC
 * is implemented in the quantum application, the optical and quantum
 * portions of the application should be seperate. The optical device is
 * based on the point-to-point device.
 */

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"

#include <vector>
#include <map>

namespace ns3
{
	class ProtocolItem
	{
		public:
			uint32_t id;
			int peer;
			bool sender;
			bool has_qubit;
			bool operating;
			int protocol;
			Time init;
			Time sent;
			uint8_t x_result;
			uint8_t z_result;
	};

	class DataItem
	{
		public:
			uint32_t id;
			bool sender;
			uint8_t protocol;
			int nacks;
			int awks;
			uint64_t rx_queue_time;
			uint64_t tx_queue_time;
			uint64_t total_time;
	};

	/**
	 * @ingroup quantum-network
	 * @class QuantumApplication
	 * @brief A application simulating network traffic for quantum applications.
	 *
	 * This class is designed to run on a TimeNode with optical devices. This
	 * application will generate uniform traffic with teledata and telegate
	 * protocols.
	 */
	class QuantumApplication : public Application
	{
		public:
			/**
			 * @brief Get the TypeId.
			 * @return The TypeId for this class.
			 */
			static TypeId GetTypeId();
			QuantumApplication();
			~QuantumApplication() override;
			/**
			 * @brief Add another application this application will communicate with.
			 * @param addr the Address of the peer node.
			 */
			void AddPeer(const Address& addr);
			void RemovePeer(const Address& ip);
		protected:
			uint16_t m_msg_count = 0;
			uint16_t m_id;
			uint16_t m_port;
			Address m_local;
			std::vector<Address> m_peers;
		private:
			std::map<uint32_t, DataItem> m_data;

			uint16_t m_max_tx_queue;
			uint8_t m_tos;
			Ptr<Socket> m_sock;
			void StartApplication() override;
			void StopApplication() override;
			void DataReceiveCallback(Ptr<Socket> sock);
			void Run();
			void Send();
			uint32_t BytesToUint32(uint8_t *buffer, int offset);
			uint16_t BytesToUint16(uint8_t *buffer, int offest);
			void ApplyConditional(uint32_t id, bool sender);
			void SendClassical(uint32_t id, bool sender);
			void SendNACK(uint32_t id, uint8_t protocol, int peer);
			void SendAWK(uint32_t id, uint8_t protocol, int peer);
			void ReleaseQubit(uint32_t id, bool sender);
			void ReleaseClassic(uint32_t id);

			float m_q_failure_rate;
			float m_c_failure_rate;

			Time m_single_op_time;
			Time m_two_op_time;
			Time m_measurement_time;
			uint64_t m_entanglement_time;
			uint64_t m_buff_size;
			uint64_t m_ave_send_time;
			uint64_t m_send_rng;
			uint16_t m_num_qubits;
			std::vector<ProtocolItem> m_rx_queue;
			std::vector<ProtocolItem> m_tx_queue;
			std::vector<ProtocolItem> m_rx_qubits;
			std::vector<ProtocolItem> m_tx_qubits;
			std::vector<ProtocolItem> m_classic_storage;
			EventId m_run_event;

	};
} // namespace ns3

#endif
