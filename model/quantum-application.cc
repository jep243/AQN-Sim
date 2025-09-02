#include "ns3/quantum-application.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket.h"
#include "ns3/optical-device.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <random>
#include <iostream>

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("QuantumApplication");
	NS_OBJECT_ENSURE_REGISTERED(QuantumApplication);

	TypeId
	QuantumApplication::GetTypeId()
	{
		static TypeId tid = TypeId("ns3::QuantumApplication")
			.SetParent<Application>()
			.SetGroupName("QuantumNetwork")
			.AddConstructor<QuantumApplication>()
			.AddAttribute("Port",
						  "Port the application will listen on.",
						  UintegerValue(9),
						  MakeUintegerAccessor(&QuantumApplication::m_port),
						  MakeUintegerChecker<uint16_t>())
			.AddAttribute("Tos",
						  "The Type of Service used to send Ipv4 packets.",
						  UintegerValue(0),
						  MakeUintegerAccessor(&QuantumApplication::m_tos),
						  MakeUintegerChecker<uint8_t>())
			.AddAttribute("Local",
						  "The Address the listening socket will bind to",
						  AddressValue(),
						  MakeAddressAccessor(&QuantumApplication::m_local),
						  MakeAddressChecker())
			.AddAttribute("NumQubits",
						  "The number of qubits the QNIC will use.",
						  UintegerValue(1),
						  MakeUintegerAccessor(
						  	&QuantumApplication::m_num_qubits),
						  MakeUintegerChecker<uint16_t>())
			.AddAttribute("MaxTxQueue",
						  "The maximum length of the tx queue.",
						  UintegerValue(100),
						  MakeUintegerAccessor(
						  	&QuantumApplication::m_max_tx_queue),
						  MakeUintegerChecker<uint16_t>())
			.AddAttribute("EntanglementTime",
						  "How long it takes to generate entanglement.",
						  UintegerValue(50),
						  MakeUintegerAccessor(
						  	&QuantumApplication::m_entanglement_time),
						  MakeUintegerChecker<uint64_t>())
			.AddAttribute("AverageSendTime",
						  "The average amount of time between messages.",
						  UintegerValue(10),
						  MakeUintegerAccessor(
						  	&QuantumApplication::m_ave_send_time),
						  MakeUintegerChecker<uint64_t>())
			.AddAttribute("SendTimeRange",
						  "The range between send times (+ and -).",
						  UintegerValue(5),
						  MakeUintegerAccessor(
						  	&QuantumApplication::m_send_rng),
						  MakeUintegerChecker<uint64_t>())
			.AddAttribute("ID",
						  "The id the node will use.",
						  UintegerValue(0),
						  MakeUintegerAccessor(&QuantumApplication::m_id),
						  MakeUintegerChecker<uint16_t>())
			.AddAttribute("QuantumFailureRate",
						  "The quantum failure rate.",
						  DoubleValue(0),
						  MakeDoubleAccessor(
						  	&QuantumApplication::m_q_failure_rate),
						  MakeDoubleChecker<double>())
			.AddAttribute("ClassicalFailureRate",
						  "The classical failure rate.",
						  DoubleValue(0),
						  MakeDoubleAccessor(
						  	&QuantumApplication::m_c_failure_rate),
						  MakeDoubleChecker<double>())
			.AddAttribute("SingleQubitTime",
						  "The time required for single qubit operation.",
						  TimeValue(NanoSeconds(60)),
						  MakeTimeAccessor(
						  	&QuantumApplication::m_single_op_time),
						  MakeTimeChecker())
			.AddAttribute("TwoQubitTime",
						  "The time required for two qubit operation.",
						  TimeValue(NanoSeconds(560)),
						  MakeTimeAccessor(
						  	&QuantumApplication::m_two_op_time),
						  MakeTimeChecker())
			.AddAttribute("MeasureTime",
						  "The time required for measurement operation.",
						  TimeValue(NanoSeconds(1400)),
						  MakeTimeAccessor(
						  	&QuantumApplication::m_measurement_time),
						  MakeTimeChecker());
		return tid;
	}

	QuantumApplication::QuantumApplication()
		: m_peers{std::vector<Address>()},
		  m_sock{nullptr}
	{
		NS_LOG_FUNCTION(m_id);
	}

	QuantumApplication::~QuantumApplication()
	{
		NS_LOG_FUNCTION(m_id);
		m_sock = nullptr;
	}

	void
	QuantumApplication::AddPeer(const Address& addr)
	{
		NS_LOG_FUNCTION(m_id << addr);
		if (!std::count(m_peers.begin(), m_peers.end(), addr))
		{
			m_peers.push_back(addr);
			NS_LOG_DEBUG("Adding address to " << (int)m_id << ", IP: " << 
						 InetSocketAddress::ConvertFrom(addr).GetIpv4() << 
						 ", Port: " << 
						 InetSocketAddress::ConvertFrom(addr).GetPort());
		}
	}

	void
	QuantumApplication::RemovePeer(const Address& ip)
	{
		NS_LOG_FUNCTION(m_id << ip);
		if (std::count(m_peers.begin(), m_peers.end(), ip))
		{
			m_peers.erase(std::remove(m_peers.begin(), m_peers.end(), ip), 
						  m_peers.end());
		}
	}

	void
	QuantumApplication::StartApplication()
	{
		NS_LOG_FUNCTION(m_id);
		if (!m_sock)
		{
			TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
			m_sock = Socket::CreateSocket(GetNode(), tid);
			if (!InetSocketAddress::IsMatchingType(m_local))
			{
				NS_LOG_DEBUG("Supplied Address did not match");
				m_local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
			}
			NS_LOG_DEBUG("Starting ID: " << (int)m_id << ", IP: " << 
						 InetSocketAddress::ConvertFrom(m_local).GetIpv4() << 
						 ", Port: " << 
						 InetSocketAddress::ConvertFrom(m_local).GetPort());
			if (m_sock->Bind(m_local) == -1)
			{
				NS_FATAL_ERROR("Failed to bind socket");
			}
			m_sock->SetIpTos(m_tos);
			m_sock->SetRecvCallback(
				MakeCallback(&QuantumApplication::DataReceiveCallback, this));
		}
		Ptr<Node> node = GetNode();
		for (uint32_t i = 0; i < node->GetNDevices(); i++)
		{
			Ptr<NetDevice> device = node->GetDevice(i);
			Ptr<OpticalDevice> optical = device->GetObject<OpticalDevice>();
			if (optical)
			{
				uint64_t bps = optical->GetDataDataRate();
				uint64_t bpns = bps / 1000000000;
				m_buff_size = bpns * m_entanglement_time / 8;
				m_buff_size = m_buff_size > 8 ? m_buff_size : 8;
				break;
			}
		}
		Run();
	}

	void
	QuantumApplication::StopApplication()
	{
		NS_LOG_FUNCTION(m_id);
		if (m_sock)
		{
			m_sock->Close();
			m_sock->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
		}
		Simulator::Cancel(m_run_event);
		std::cout << "TotalSent: " << m_msg_count << "," << m_id << std::endl;
	}

	void
	QuantumApplication::DataReceiveCallback(Ptr<Socket> sock)
	{
		Address from;
		Ptr<Packet> packet;
		int buff_size = m_buff_size > 64 ? m_buff_size : 64;
		uint8_t *buffer = new uint8_t[buff_size];
		int read = 0;
		while ((packet = sock->RecvFrom(from)) && read < buff_size)
		{
			read = read + packet->CopyData(&buffer[0], buff_size);
		}
		uint8_t msg_type = buffer[0];
		uint32_t id = BytesToUint32(buffer, 1);
		uint8_t protocol = buffer[5];
		NS_LOG_FUNCTION(m_id << id << msg_type);
		Time current = Simulator::Now();
		int peer = -1;
		for (int i = 0; i < (int)m_peers.size(); i++)
		{	
			if (from == m_peers[i])
			{
				peer = i;
				break;
			}
		}
		NS_ASSERT_MSG(peer >= 0, "Received message from non-peer.");

		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0, 1.0);
		int index = -1;
		bool sender = false;
		uint8_t x_result = 255;
		uint8_t z_result = 255;
		bool classical = false;
		Time runtime;
		switch (msg_type)
		{
			// Received Quantum packet
			case 0:
				if (dis(gen) < m_q_failure_rate)
				{
					SendNACK(id, protocol, peer);
					break;
				}
				for (int i = 0; i < (int)m_rx_queue.size(); i++)
				{
					if (m_rx_queue[i].id == id)
					{
						NS_ASSERT_MSG(false, "Should not receive another qubit.");
						break;
					}
				}
				if (index < 0)
				{
					for (int i = 0; i < (int)m_rx_qubits.size(); i++)
					{
						if (m_rx_qubits[i].id == id)
						{
							index = i;
							bool operating = m_rx_qubits[i].operating;
							bool has_qubit = m_rx_qubits[i].has_qubit;
							NS_ASSERT_MSG(!operating && !has_qubit,
								"Should not recieve a qubit when existing qubit.");
							m_rx_qubits[i].has_qubit = true;
							break;
						}
					}
				}
				if (index < 0)
				{
					ProtocolItem new_item;
					new_item.id = id;
					new_item.peer = peer;
					new_item.protocol = protocol;
					new_item.init = current;
					new_item.x_result = 255;
					new_item.z_result = 255;
					new_item.has_qubit = false;
					new_item.operating = false;
		
					// For data results
					DataItem data;
					data.id = id;
					data.sender = false;
					data.protocol = protocol;
					data.nacks = 0;
					data.awks = 0;
					data.rx_queue_time = 0;
					data.tx_queue_time = 0;
					data.total_time = 0;
					m_data.insert({id, data});

					if (m_rx_qubits.size() < m_num_qubits)
					{
						new_item.has_qubit = true;
						m_rx_qubits.push_back(new_item);
						index = m_rx_qubits.size() - 1;
						NS_ASSERT_MSG(m_data.find(id) != m_data.end(), 
									  "Rx queue for unsaved packet.");
						m_data[id].rx_queue_time = 0;
						NS_LOG_DEBUG("RxQueue-ID: " << id << "Time: 0, " <<
									 "Queue Size: " << (int)m_rx_queue.size());
					}
					else
					{
						m_rx_queue.push_back(new_item);
					}
				}

				// Entanglement achieved, start protocol.
				if (index >= 0 && m_rx_qubits[index].has_qubit)
				{
					NS_ASSERT_MSG(!m_rx_qubits[index].operating, 
								  "Qubit should not be operating.");
					bool is_good = m_rx_qubits[index].id == id;
					NS_ASSERT_MSG(is_good, "Incorrect index.");
					m_rx_qubits[index].operating = true;
					switch (protocol)
					{
						case 0:
							runtime = m_two_op_time + m_single_op_time + 
									  m_measurement_time;
							break;
						case 1:
							runtime = m_two_op_time + m_measurement_time;
							break;
						default:
							NS_ASSERT_MSG(false, "Invalid protocol.");
							break;
					}
					Simulator::Schedule(runtime,
										&QuantumApplication::SendClassical,
										this,
										id,
										false);
				}
				break;
			// Received Classical packet
			case 1:
				if (dis(gen) < m_c_failure_rate)
				{
					SendNACK(id, protocol, peer);
					break;
				}
				x_result = buffer[6];
				z_result = buffer[7];
				for (int i = 0; i < (int)m_rx_qubits.size(); i++)
				{
					if (m_rx_qubits[i].id == id)
					{
						index = i;
						m_rx_qubits[i].x_result = x_result;
						m_rx_qubits[i].z_result = z_result;
						break;
					}
				}
				if (index < 0)
				{
					for (int i = 0; i < (int)m_tx_qubits.size(); i++)
					{
						if (m_tx_qubits[i].id == id)
						{
							index = i;
							sender = true;
							m_tx_qubits[i].x_result = x_result;
							m_tx_qubits[i].z_result = z_result;
							break;
						}
					}
				}
				NS_ASSERT_MSG(index >= 0, "Classical arrived, no qubit.");
				switch(protocol)
				{
					case 0:
						NS_ASSERT_MSG(sender,
									  "Teledata only sender get classical.");
						ApplyConditional(id, true);
						break;
					case 1:
						if (sender)
						{
							//Run local, send classical, apply conditional
							bool is_good = m_tx_qubits[index].id == id;
							NS_ASSERT_MSG(is_good, "Incorrect index.");
							m_tx_qubits[index].operating = true;
							runtime = m_two_op_time + m_single_op_time + 
									  m_measurement_time;
							Simulator::Schedule(runtime,
											&QuantumApplication::SendClassical,
											this,
											id,
											true);
							Simulator::Schedule(runtime,
										&QuantumApplication::ApplyConditional,
										this,
										id,
										true);
						}
						else
						{
							ApplyConditional(id, false);
						}
						break;
					default:
						NS_ASSERT_MSG(false, "Invalide protocol.");
						break;
				}
				break;
			// Recevied NACK
			case 2:
				for (int i = 0; i < (int)m_tx_qubits.size(); i++)
				{
					if (m_tx_qubits[i].id == id)
					{
						index = i;
						classical = false;
						NS_ASSERT_MSG(!m_tx_qubits[i].operating,
									  "Received NACK, already operating.");
						break;
					}
				}
				for (int i = 0; i < (int)m_classic_storage.size(); i++)
				{
					if (m_classic_storage[i].id == id)
					{
						index = i;
						ProtocolItem& item = m_classic_storage[index];
						classical = (item.x_result < 255 || 
									 item.z_result < 255);
						NS_ASSERT_MSG(classical,
									  "Classic storage, no classical.");
					}
				}
				NS_ASSERT_MSG(index >= 0, "NACK arrived, no qubit.");
				// Resend classical packet
				if (classical)
				{
					ProtocolItem& item = m_classic_storage[index];
					NS_ASSERT_MSG(item.id == id, "ID does not match.");
					Address addr = m_peers[item.peer];
					uint8_t out_buffer[8];
					out_buffer[0] = 1;
					std::memcpy(&out_buffer[1], &(item.id), 4);
					out_buffer[5] = item.protocol;
					out_buffer[6] = item.x_result;
					out_buffer[7] = item.z_result;
					m_sock->SendTo(&out_buffer[0], 8, 0, addr);
				}
				// Resend quantum packet
				else
				{
					ProtocolItem& item = m_tx_qubits[index];
					Address peer = m_peers[item.peer];
					uint8_t *out_buffer = new uint8_t[m_buff_size];
					out_buffer[0] = 0;
					std::memcpy(&out_buffer[1], &(item.id), 4);
					out_buffer[5] = item.protocol;
					m_sock->SendTo(out_buffer, m_buff_size , 0, peer);
					delete[] out_buffer;
				}
				break;
			// Received AWK
			case 3:
				ReleaseClassic(id);
				break;
			default:
				NS_ASSERT_MSG(false, "Invalid message type.");
				break;
		}

		delete[] buffer;
	}

	void 
	QuantumApplication::ApplyConditional(uint32_t id, bool sender)
	{
		NS_LOG_FUNCTION(m_id << id << sender);
		std::vector<ProtocolItem>& qubits = sender ? m_tx_qubits : m_rx_qubits;
		int index = 0;
		for (size_t i = 0; i < qubits.size(); i++)
		{
			if (qubits[i].id == id)
			{
				index = i;
				break;
			}
		}
		ProtocolItem& item = qubits[index];
		NS_ASSERT_MSG(item.id == id, "Item not found.");
		Time runtime;
		uint8_t ops;
		switch (item.protocol)
		{
			case 0:
				// Teledata
				if (sender)
				{
					ops = item.x_result + item.z_result;
					NS_ASSERT_MSG(ops >= 0 && ops <= 2, "Invalid operation values.");
					runtime = ops * m_single_op_time;
					ReleaseClassic(item.id);
					SendAWK(item.id, item.protocol, true);
					Simulator::Schedule(runtime, 
										&QuantumApplication::ReleaseQubit,
										this,
										id,
										sender);
				}
				else
				{
					NS_ASSERT_MSG(false, "Teledata sender should not have local.");
				}
				break;
			case 1:
				// Telegate
				if (sender)
				{
					ops = item.x_result;
					NS_ASSERT_MSG(ops >= 0 && ops < 2, "Invalid operation value.");
				}
				else
				{
					ops = item.z_result;
					NS_ASSERT_MSG(ops >= 0 && ops < 2, "Invalid operation value.");
					ReleaseClassic(item.id);
					SendAWK(item.id, item.protocol, item.peer);
				}
				runtime = ops * m_single_op_time;
				Simulator::Schedule(runtime, 
									&QuantumApplication::ReleaseQubit,
									this,
									id,
									sender);
				break;
			default:
				// Invalid
				NS_ASSERT_MSG(false, "Invalid protocol number.");
				break;
		}
	}

	void
	QuantumApplication::ReleaseClassic(uint32_t id)
	{
		NS_LOG_FUNCTION(m_id << id);
		for (auto it = m_classic_storage.begin(); 
			 it != m_classic_storage.end(); ++it)
		{
			if (it->id == id)
			{
				m_classic_storage.erase(it);
				break;
			}
		}
	}

	void
	QuantumApplication::SendClassical(uint32_t id, bool sender)
	{
		NS_LOG_FUNCTION(m_id << id);
		std::vector<ProtocolItem>& qubits = sender ? m_tx_qubits : m_rx_qubits;
		int index = 0;
		for (size_t i = 0; i < qubits.size(); i++)
		{
			if (qubits[i].id == id)
			{
				index = i;
				break;
			}
		}
		ProtocolItem& item = qubits[index];
		NS_ASSERT_MSG(item.id == id, "Item not found.");
		
		if (item.operating)
		{
			item.operating = false;
			Address addr = m_peers[item.peer];
			uint8_t buffer[8];
			buffer[0] = 1;
			std::memcpy(&buffer[1], &(item.id), 4);
			buffer[5] = item.protocol;
			uint8_t x_result = 255;
			uint8_t z_result = 255;
			switch(item.protocol)
			{
				case 0:
					x_result = std::rand() % 2;
					z_result = std::rand() % 2;
					buffer[6] = x_result;
					buffer[7] = z_result;
					m_classic_storage.push_back(item);
					index = m_classic_storage.size() - 1;
					m_classic_storage[index].x_result = x_result;
					m_classic_storage[index].z_result = z_result;
					ReleaseQubit(id, sender);
					break;
				case 1:
					if (sender)
					{
						x_result = 255;
						z_result = std::rand() % 2;
					}
					else
					{
						x_result = std::rand() % 2;
						z_result = 255;
					}
					buffer[6] = x_result;
					buffer[7] = z_result;
					m_classic_storage.push_back(item);
					index = m_classic_storage.size() - 1;
					m_classic_storage[index].x_result = x_result;
					m_classic_storage[index].z_result = z_result;
					break;
				default:
					NS_ASSERT_MSG(false, "Invalid protocol number.");
					break;
			}
			m_sock->SendTo(&buffer[0], 8 , 0, addr);
		}
	}

	void
	QuantumApplication::SendNACK(uint32_t id, uint8_t protocol, int peer)
	{
		NS_LOG_FUNCTION(m_id << id);
		Address addr = m_peers[peer];
		uint8_t buffer[6];
		buffer[0] = 2;
		std::memcpy(&buffer[1], &(id), 4);
		buffer[5] = protocol;
		m_sock->SendTo(&buffer[0], 6 , 0, addr);
		//NS_ASSERT_MSG(m_data.find(id) != m_data.end(), "NACK for unsaved packet.");
		m_data[id].nacks++;
		NS_LOG_DEBUG("NACK-ID: " << id);
	}
	
	void
	QuantumApplication::SendAWK(uint32_t id, uint8_t protocol, int peer)
	{
		NS_LOG_FUNCTION(m_id << id);
		Address addr = m_peers[peer];
		uint8_t buffer[6];
		buffer[0] = 3;
		std::memcpy(&buffer[1], &(id), 4);
		buffer[5] = protocol;
		m_sock->SendTo(&buffer[0], 6 , 0, addr);
		NS_ASSERT_MSG(m_data.find(id) != m_data.end(), "AWK for unsaved packet.");
		m_data[id].awks++;
		NS_LOG_DEBUG("AWK-ID: " << id);
	}

	void
	QuantumApplication::ReleaseQubit(uint32_t id, bool sender)
	{
		NS_LOG_FUNCTION(m_id << id);
		std::vector<ProtocolItem>& qubits = sender ? m_tx_qubits : m_rx_qubits;
		int index = 0;
		for (size_t i = 0; i < qubits.size(); i++)
		{
			if (qubits[i].id == id)
			{
				index = i;
				break;
			}
		}
		ProtocolItem& item = qubits[index];
		NS_ASSERT_MSG(item.id == id, "Item not found.");
		
		Time current = Simulator::Now();
		Time elapsed = current - item.init;
		if (sender)
		{
			m_tx_qubits.erase(m_tx_qubits.begin() + index);
			Send();
		}
		else
		{
			m_rx_qubits.erase(m_rx_qubits.begin() + index);
			if (m_rx_queue.size() > 0)
			{
				item = m_rx_queue[0];
				m_rx_queue.erase(m_rx_queue.begin());
				NS_ASSERT_MSG(m_data.find(item.id) != m_data.end(), 
							  "Rx queue for unsaved packet.");
				Time rx_elapsed = current - item.init;
				m_data[item.id].rx_queue_time = rx_elapsed.GetNanoSeconds();
				NS_LOG_DEBUG("RxQueue-ID: " << id << "Time: " << 
							 rx_elapsed.GetNanoSeconds() << ", Queue Size: " 
							 << (int)m_rx_queue.size());
				item.init = current;
				SendNACK(item.id, item.protocol, item.peer);
			}
		}
		NS_ASSERT_MSG(m_data.find(id) != m_data.end(), 
					  "Finished for unsaved packet.");
		DataItem& di = m_data[id];
		di.total_time = elapsed.GetNanoSeconds();
		std::cout << (int)di.id << "," << di.sender << "," << (int)di.protocol
				  << "," << di.nacks << "," << di.awks << ","
				  << di.rx_queue_time << "," << di.tx_queue_time 
				  << "," << di.total_time << "," << m_id << std::endl;
		m_data.erase(id);
	}

	void
	QuantumApplication::Run()
	{
		ProtocolItem item;
		item.id = ((uint32_t)m_id << 16) | m_msg_count;
		m_msg_count++;
		NS_LOG_FUNCTION(m_id << item.id);
		int choice = std::rand() % m_peers.size();
		item.peer = choice;
		item.sender = true;
		int protocol = std::rand() % 2;
		//int protocol = 1;
		item.protocol = protocol;
		item.init = Simulator::Now();
		item.x_result = -1;
		item.z_result = -1;
		item.operating = false;
		m_tx_queue.push_back(item);
		
		// For data results
		DataItem data;
		data.id = item.id;
		data.sender = true;
		data.protocol = protocol;
		data.nacks = 0;
		data.awks = 0;
		data.rx_queue_time = 0;
		data.tx_queue_time = 0;
		data.total_time = 0;
		m_data.insert({item.id, data});
		NS_LOG_DEBUG("Send-ID: " << item.id);

		if (m_tx_qubits.size() < m_num_qubits)
		{
			Send();
		}
		uint64_t delay_nano = (rand() % (2 * m_send_rng)) - m_send_rng + 
							   m_ave_send_time;
		NS_LOG_DEBUG("Next Send: " << delay_nano);
		Time send_delay = NanoSeconds(delay_nano);
		m_run_event = Simulator::Schedule(send_delay, 
										  &QuantumApplication::Run, this);
		if (m_tx_queue.size() >= m_max_tx_queue)
		{
			std::cout << "TxFull," << m_id << "," << m_rx_qubits.size() << ","
					  << m_tx_qubits.size() << "," 
					  << item.init.GetNanoSeconds() << std::endl;
			StopApplication();
		}
	}

	void
	QuantumApplication::Send()
	{
		if (m_tx_queue.size() > 0 && m_tx_qubits.size() < m_num_qubits)
		{
			ProtocolItem item = m_tx_queue[0];
			NS_LOG_FUNCTION(m_id << item.id);
			NS_ASSERT_MSG(item.sender, "Called send from receiver.");
			m_tx_queue.erase(m_tx_queue.begin());
			m_tx_qubits.push_back(item);
			Time current = Simulator::Now();
			item.sent = current;
			Address peer = m_peers[item.peer];
			uint8_t *buffer = new uint8_t[m_buff_size];
			buffer[0] = 0;
			std::memcpy(&buffer[1], &(item.id), 4);
			buffer[5] = item.protocol;
			m_sock->SendTo(buffer, m_buff_size , 0, peer);
			delete[] buffer;
			Time elapsed = current - item.init;
			NS_ASSERT_MSG(m_data.find(item.id) != m_data.end(),
						  "Sending a packet that was not saved.");
			m_data[item.id].tx_queue_time = elapsed.GetNanoSeconds();
			NS_LOG_DEBUG("TxQueue-ID: " << item.id << "Time: " << 
						 elapsed.GetNanoSeconds() << "Queue Size: " << 
						 (int)m_rx_queue.size());
			Send();
		}
	} 

	uint32_t
    QuantumApplication::BytesToUint32(uint8_t* buffer, int offset)
  	{
	  	NS_LOG_FUNCTION(m_id);
	  	uint32_t low = static_cast<uint32_t>(BytesToUint16(buffer, offset));
	  	uint32_t high = static_cast<uint32_t>(BytesToUint16(buffer, offset + 2));
	  	uint32_t value = high << 16 | low;
	  	return value;
  	}
 
    uint16_t
   	QuantumApplication::BytesToUint16(uint8_t* buffer, int offset)
    {
        NS_LOG_FUNCTION(m_id);
        uint16_t low = static_cast<uint16_t>(buffer[offset]);
        uint16_t high = static_cast<uint16_t>(buffer[offset + 1]);
        uint16_t value = high << 8 | low;
        return value;
    }  
} // namespace ns3
