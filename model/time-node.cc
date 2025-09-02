#include "time-node.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/log.h"

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("TimeNode");
	NS_OBJECT_ENSURE_REGISTERED(TimeNode);

	TypeId
	TimeNode::GetTypeId()
	{
		static TypeId tid = 
			TypeId("ns3::TimeNode")
				.SetParent<Node>()
				.SetGroupName("QuantumNetwork")
				.AddConstructor<TimeNode>()
				.AddAttribute("Skew",
							  "The skew this nodes clock will have.",
							  DoubleValue(0.0),
							  MakeDoubleAccessor(&TimeNode::m_skew),
							  MakeDoubleChecker<double>());
		return tid;
	}

	TimeNode::TimeNode()
		: Node()
	{
		NS_LOG_FUNCTION(this);
		m_lastGlobal = Simulator::Now();
		m_lastLocal = m_lastGlobal;
	}

	TimeNode::TimeNode(uint32_t systemId)
		: Node(systemId)
	{
		NS_LOG_FUNCTION(this << systemId);
		m_lastGlobal = Simulator::Now();
		m_lastLocal = m_lastGlobal;
	}

	TimeNode::~TimeNode()
	{
		NS_LOG_FUNCTION(this);
	}

	Time
	TimeNode::GetLocalTime()
	{
		Time current = Simulator::Now();
		Time elapsed = current - m_lastGlobal;
		Time elapsed_local = elapsed + (elapsed * (m_skew / 1000000.0));
		Time local = m_lastLocal + elapsed_local;
		NS_LOG_DEBUG("Current: " << current <<
					 ", Skew: " << m_skew <<
					 ", Last Local: " << m_lastLocal <<
					 ", Last Global: " << m_lastGlobal <<
					 ", Elapsed: " << elapsed <<
					 ", Elapsed Local: " << elapsed_local <<
					 ", Local: " << local);
		m_lastLocal = local;
		m_lastGlobal = current;
		return local;
	}

	Time
	TimeNode::GetGlobalTime() const
	{
		return Simulator::Now();
	}

	double
	TimeNode::GetSkew() const
	{
		return m_skew;
	}

	void
	TimeNode::SetLocalTime(Time time)
	{
		m_lastLocal = time;
		m_lastGlobal = Simulator::Now();
	}
}
