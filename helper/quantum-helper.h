/*
This will be my helper for the Teledata Application
*/

#ifndef QUANTUM_HELPER_H
#define QUANTUM_HELPER_H

#include "ns3/application-helper.h"

namespace ns3
{
	class QuantumHelper : public ApplicationHelper
	{
		public:
			QuantumHelper();
			QuantumHelper(uint16_t id);
			QuantumHelper(uint16_t id, uint16_t port);
			QuantumHelper(uint16_t id, const Address& addr);
	};
}

#endif
