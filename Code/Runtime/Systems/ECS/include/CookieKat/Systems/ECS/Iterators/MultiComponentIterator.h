#pragma once

#include "IDs.h"

namespace CKE {
	// Forward Declarations
	class EntityDatabase;
	class Archetype;
	class ComponentArray;
}

namespace CKE {
	// Tuple of components returned by a MultiComponentIter
	class ComponentTuple
	{
		friend class MultiComponentIter;

	public:
		// Returns a pointer to the component at the given position index.
		// The component ordering is the same as the iterators
		inline void* GetComponent(u64 componentIndex);

		template <typename T>
		inline T* GetComponent(u64 componentIndex);

	private:
		Vector<void*> m_Components;
	};

	//-----------------------------------------------------------------------------

	struct IteratorSetupData
	{
		struct ArchetypeInfo
		{
			Archetype*              m_pArchetype;
			u64                     m_TotalRows;
			Vector<ComponentArray*> m_pComponentArrays; // Component order is the same as the query order
		};

		Vector<ArchetypeInfo> m_ArchetypesToIterate;
		u64                   m_TotalEntities;
	};

	struct IterationData
	{
		Archetype*              m_pArchetype;
		u64                     m_TotalRows;
		Vector<ComponentArray*> m_pComponentArrays; // Component order is the same as the query order
	};

	// Iterator for multi-component queries
	class MultiComponentIter
	{
	public:
		// Setup
		//-----------------------------------------------------------------------------

		MultiComponentIter(Vector<IterationData> const& iterationData);

		// Utility Accessors
		//-----------------------------------------------------------------------------

		// Returns the total number of elements/entities in the iterator
		inline u64 GetNumElements() const;

		// Range-for iterator
		//-----------------------------------------------------------------------------

		inline MultiComponentIter begin();
		inline ComponentIterEnd   end();
		inline bool               operator!=(const ComponentIterEnd& other) const;
		inline void               operator++();
		inline void               operator+(int i);
		inline ComponentTuple*    operator*();

	protected:
		Vector<IterationData> m_IterationData;
		u64                   m_IterationIdx = 0; // Current Idx in the iteration data
		u64                   m_RowInArch = 0;

		u32 m_ComponentsToIterate = 0;
		u64 m_NumEntitiesIterated = 0; // Total number of components already iterated
		u64 m_NumEntitiesTotal = 0;    // Total number of components to iterate in all archetypes

		ComponentTuple m_OutCompTuple{}; // Cached tuple returned with each iteration to avoid allocating it constantly
	};

	//-----------------------------------------------------------------------------

	// Template iterator for multi-component queries
	template <typename Comp, typename... OtherComp>
	class TMultiComponentIter : public MultiComponentIter
	{
	public:
		TMultiComponentIter(Vector<IterationData> const& iterationData);

		// Range-for iterator
		inline TMultiComponentIter               begin();
		inline std::tuple<Comp*, OtherComp*...>& operator*();

	private:
		// Cached component tuple that the iterator returns
		std::tuple<Comp*, OtherComp*...> m_CompTuple;
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace CKE {
	inline void* ComponentTuple::GetComponent(u64 componentIndex) {
		return m_Components[componentIndex];
	}

	template <typename T>
	T* ComponentTuple::GetComponent(u64 componentIndex) {
		return reinterpret_cast<T*>(GetComponent(componentIndex));
	}
}

namespace CKE {
	u64 MultiComponentIter::GetNumElements() const {
		return m_NumEntitiesTotal;
	}

	MultiComponentIter MultiComponentIter::begin() {
		m_IterationIdx = 0;
		m_RowInArch = 0;
		m_NumEntitiesIterated = 0;
		return *this;
	}

	ComponentIterEnd MultiComponentIter::end() {
		ComponentIterEnd iterEnd{m_NumEntitiesTotal};
		return iterEnd;
	}

	bool MultiComponentIter::operator!=(const ComponentIterEnd& other) const {
		return m_NumEntitiesIterated != other.m_NumEntitiesTotal;
	}

	void MultiComponentIter::operator++() {
		m_RowInArch++;

		// If we exhausted the current archetype, go to the next one
		if (m_RowInArch >= m_IterationData[m_IterationIdx].m_TotalRows) {
			// Update the entities processed counter
			m_NumEntitiesIterated += m_IterationData[m_IterationIdx].m_TotalRows;

			// Advance to the next archetype
			m_RowInArch = 0;
			m_IterationIdx++;

			// If this is true then we finished iterating all of the archetypes
			if (m_IterationIdx >= m_IterationData.size()) {
				m_IterationIdx = 0;
			}
		}
	}

	inline void MultiComponentIter::operator+(int i) {
		//m_RowInArch += i;
		//while (m_RowInArch >= m_NumCompsInCurrArch) {
		//	m_IterationIdx++;
		//	m_RowInArch -= m_NumCompsInCurrArch;

		//	if (m_IterationIdx >= m_MatchedArchIDs.size()) {
		//		m_IterationIdx = 0;
		//	}

		//	m_pCurrArch = m_IDToArchetype->at(m_MatchedArchIDs[m_IterationIdx]);
		//	m_NumCompsInCurrArch = m_pCurrArch->m_NumEntities;
		//}

		//SetArchetypeToIterate(m_MatchedArchIDs[m_IterationIdx]);
	}

	ComponentTuple* MultiComponentIter::operator*() {
		Vector<void*>& compTupleArr = m_OutCompTuple.m_Components;
		compTupleArr.clear();

		Vector<ComponentArray*>& componentArrays = m_IterationData[m_IterationIdx].m_pComponentArrays;
		for (int i = 0; i < m_ComponentsToIterate; ++i) {
			compTupleArr.push_back(componentArrays[i]->GetCompAtIndex(m_RowInArch));
		}
		return &m_OutCompTuple;
	}
}

namespace CKE {
	template <typename Comp, typename... Other>
	TMultiComponentIter<
		Comp, Other...>::TMultiComponentIter(Vector<IterationData> const& iterationData) : MultiComponentIter{iterationData} { }

	template <typename Comp, typename... Other>
	TMultiComponentIter<Comp, Other...> TMultiComponentIter<Comp, Other...>::begin() {
		return *this;
	}

	template <typename Comp, typename... Other>
	std::tuple<Comp*, Other*...>& TMultiComponentIter<Comp, Other...>::operator*() {
		IteratorsUtilities::PopulateTupleWithComponents(m_CompTuple, m_IterationData[m_IterationIdx].m_pComponentArrays, m_RowInArch);
		return m_CompTuple;
	}
}
