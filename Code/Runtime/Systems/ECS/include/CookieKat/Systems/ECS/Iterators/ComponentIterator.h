#pragma once

#include "IteratorCommon.h"
#include "../IDs.h"

namespace CKE {
	// Forward Declarations
	class EntityDatabase;
	class Archetype;
	class ComponentArray;
}

namespace CKE {
	// Iterator for single-component queries
	class ComponentIter
	{
	public:
		// Setup
		//-----------------------------------------------------------------------------

		ComponentIter(Vector<ArchetypeColumnPair> const& accessData, u64 totalEntitiesCount);

		// DEPRECATED
		ComponentIter(EntityDatabase* pEntityAdmin, ComponentTypeID componentID);

		// Utility Accessors
		//-----------------------------------------------------------------------------

		// Returns the total number of elements/entities in the iterator
		inline u64 GetNumElements() const;

		// Range-for iterator
		//-----------------------------------------------------------------------------

		inline ComponentIter    begin();
		inline ComponentIterEnd end();
		inline bool             operator!=(const ComponentIterEnd& other) const;
		inline void             operator++();
		inline void*            operator*();

	protected:
		// Base setup function that must be called to begin the component iterator
		inline void BeginIteratorSetup();

	protected:
		u64 m_CurrRowInArch = 0;
		u64 m_CurrCompColumn = 0;

		u64 m_CurrArchAccessDataIndex = 0; // Current index in the archetype component access data
		u64 m_NumRowsInCurrArch = 0;       // Total number of components in current archetype

		u64 m_NumEntitiesTotal = 0;     // Total number of entities to iterate in all archetypes
		u64 m_NumEntitiesProcessed = 0; // Total number of entities already iterated

		// Cached variables to avoid constant lookups
		Archetype*      m_pCurrArch = nullptr;
		ComponentArray* m_pCurrCompArray = nullptr;

		Vector<ArchetypeColumnPair> m_CompArchAccessData; // Data to access a component in a given archetype
	};

	//-----------------------------------------------------------------------------

	// Template iterator for single-component queries
	template <typename T>
	class TComponentIterator : public ComponentIter
	{
	public:
		explicit TComponentIterator(EntityDatabase* pEntityAdmin) : ComponentIter{pEntityAdmin, ComponentStaticTypeID<T>::s_CompID} {}

		// Range-for iterator
		inline TComponentIterator begin();
		inline T*                 operator*();
	};
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace CKE {
	u64 ComponentIter::GetNumElements() const {
		return m_NumEntitiesTotal;
	}

	void ComponentIter::BeginIteratorSetup() {
		if (m_NumEntitiesTotal > 0) {
			CKE_ASSERT(!m_CompArchAccessData.empty());

			m_CurrArchAccessDataIndex = 0;
			m_CurrCompColumn = m_CompArchAccessData[m_CurrArchAccessDataIndex].m_Column;
			m_pCurrArch = m_CompArchAccessData[m_CurrArchAccessDataIndex].m_pArch;
			m_NumRowsInCurrArch = m_pCurrArch->m_NumEntities;
			m_pCurrCompArray = &m_pCurrArch->m_ArchTable[m_CurrCompColumn];
		}
	}

	ComponentIter ComponentIter::begin() {
		BeginIteratorSetup();
		return *this;
	}

	ComponentIterEnd ComponentIter::end() {
		ComponentIterEnd iter{};
		iter.m_NumEntitiesTotal = m_NumEntitiesTotal;
		return iter;
	}

	bool ComponentIter::operator!=(const ComponentIterEnd& other) const {
		return m_NumEntitiesProcessed != other.m_NumEntitiesTotal;
	}

	void ComponentIter::operator++() {
		m_CurrRowInArch++;

		// If we exhausted the current archetype, go to the next one
		[[unlikely]]
		if (m_CurrRowInArch >= m_NumRowsInCurrArch) {
			// Update the entities processed counter
			m_NumEntitiesProcessed += m_NumRowsInCurrArch;

			// Advance to the next archetype
			m_CurrRowInArch = 0;
			m_CurrArchAccessDataIndex++;

			// Check if we finished iterating all of the archetypes
			if (m_CurrArchAccessDataIndex >= m_CompArchAccessData.size()) {
				m_CurrArchAccessDataIndex = 0;
			}

			// Get the new archetype column that we will be iterating
			m_CurrCompColumn = m_CompArchAccessData[m_CurrArchAccessDataIndex].m_Column;

			// Cache the archetype and component array ptr because
			// these only change when changing archetypes
			m_pCurrArch = m_CompArchAccessData[m_CurrArchAccessDataIndex].m_pArch;
			m_pCurrCompArray = &m_pCurrArch->m_ArchTable[m_CurrCompColumn];

			// Save new iter archetype total size so we don't have to pass through
			// the Archetype* pointer to get it.
			m_NumRowsInCurrArch = m_pCurrArch->m_NumEntities;
		}
	}

	void* ComponentIter::operator*() {
		return m_pCurrCompArray->GetCompAtIndex(m_CurrRowInArch);
	}

	//-----------------------------------------------------------------------------

	template <typename T>
	TComponentIterator<T> TComponentIterator<T>::begin() {
		BeginIteratorSetup();
		return *this;
	}

	template <typename T>
	T* TComponentIterator<T>::operator*() {
		return reinterpret_cast<T*>(ComponentIter::operator*());
	}
}
