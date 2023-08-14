#pragma once

#include "IDs.h"
#include "Iterators/ComponentIterator.h"

namespace CKE {
	// Data returned by an EntityComponentIterator
	struct EntityComponentPair
	{
		EntityID m_EntityID;
		void*    m_pComponent = nullptr;
	};

	class EntityComponentIterator : public ComponentIter
	{
	public:
		EntityComponentIterator(EntityDatabase* pEntityAdmin, ComponentTypeID componentID)
			: ComponentIter{pEntityAdmin, componentID} {}

		inline EntityComponentIterator begin();
		inline EntityComponentPair*    operator*();

	private:
		EntityComponentPair m_EntityCompPair;
	};
}

namespace CKE {
	EntityComponentIterator EntityComponentIterator::begin() {
		BeginIteratorSetup();
		return *this;
	}

	EntityComponentPair* EntityComponentIterator::operator*() {
		m_EntityCompPair.m_EntityID = m_pCurrArch->m_RowIndexToEntity[m_CurrRowInArch];
		m_EntityCompPair.m_pComponent = m_pCurrCompArray->GetCompAtIndex(m_CurrRowInArch);
		return &m_EntityCompPair;
	}
}
