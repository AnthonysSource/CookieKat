#pragma once

#include "IDs.h"

namespace CKE {
	class Archetype;
}

namespace CKE {
	struct ArchetypeColumnPair
	{
		Archetype* m_pArch;
		ArchetypeComponentColumn m_Column;
	};

	// Lightweight end of all component "range for" iterators
	// We use this custom struct to avoid copying the entire iterator to check
	// if it arrived to the end
	struct ComponentIterEnd
	{
		u64 m_NumEntitiesTotal;
	};

	class QueryComponentGroup
	{
	public:
		QueryComponentGroup(Vector<ComponentTypeID> id);

		void Add(Vector<ComponentTypeID> ids);
		void Remove(Vector<ComponentTypeID> ids);
		void CombineWith(QueryComponentGroup& other);

	private:
		Vector<ComponentSetID> m_Components;
	};
}
