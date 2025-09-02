#ifndef TIME_NODE_H
#define TIME_NODE_H

#include "ns3/node.h"
#include "ns3/nstime.h"

namespace ns3
{
	class TimeNode : public Node
	{
		public:
			static TypeId GetTypeId();
			TimeNode();
			TimeNode(uint32_t systemId);
			~TimeNode() override;
			Time GetLocalTime();
			Time GetGlobalTime() const;
			double GetSkew() const;
			void SetLocalTime(Time time);
		private:
			Time m_lastLocal;
			Time m_lastGlobal;
			double m_skew;
	};
}

#endif
