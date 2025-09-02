/*
This is the helper for the Quantum Application
*/

#include "ns3/quantum-helper.h"
#include "ns3/quantum-application.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"

namespace ns3
{
	QuantumHelper::QuantumHelper()
		: ApplicationHelper(QuantumApplication::GetTypeId())
	{

	}

	QuantumHelper::QuantumHelper(uint16_t id)
		: ApplicationHelper(QuantumApplication::GetTypeId())
	{
		SetAttribute("ID", UintegerValue(id));
	}
	
	QuantumHelper::QuantumHelper(uint16_t id, uint16_t port)
		: ApplicationHelper(QuantumApplication::GetTypeId())
	{
		SetAttribute("ID", UintegerValue(id));
		SetAttribute("Port", UintegerValue(port));
	}

	QuantumHelper::QuantumHelper(uint16_t id, const Address& addr)
		: ApplicationHelper(QuantumApplication::GetTypeId())
	{
		SetAttribute("ID", UintegerValue(id));
		SetAttribute("Local", AddressValue(addr));
	}
}
