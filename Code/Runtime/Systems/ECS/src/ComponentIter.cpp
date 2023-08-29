#include "EntityDatabase.h"

namespace CKE {
	ComponentIter::ComponentIter(Vector<ArchetypeColumnPair> const& accessData, u64 totalEntitiesCount) {
		m_CompArchAccessData = accessData;
		m_NumEntitiesTotal = totalEntitiesCount;
	}

	ComponentIter::ComponentIter(ComponentIteratorConfiguration& config) {
		
	}

	MultiComponentIter::MultiComponentIter(Vector<IterationData> const& iterationData) {
		m_IterationData = iterationData;

		for (IterationData const& data : iterationData) {
			m_NumEntitiesTotal += data.m_TotalRows;
		}

		if (!m_IterationData.empty()) {
			m_ComponentsToIterate = m_IterationData[0].m_pComponentArrays.size();
		}else {
			m_ComponentsToIterate = 0;
		}
	}
}
