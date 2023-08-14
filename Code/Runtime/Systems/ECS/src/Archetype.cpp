#include "Archetype.h"
#include "CookieKat/Core/Platform/Asserts.h"

namespace CKE {
	Archetype::Archetype(ArchetypeID id, ComponentSet const& componentSet, u64 maxEntities) {
		m_ComponentSet = componentSet;
		m_ID = id;
		m_NumEntities = 0;
		m_RowIndexToEntity.resize(maxEntities);
		std::fill(m_RowIndexToEntity.begin(),
		          m_RowIndexToEntity.end(),
		          EntityID::Invalid());

		//for (ComponentTypeID componentID : componentSet) {
		//	// Create component array
		//	m_ArchTable.push_back(ComponentArray{ componentID, maxEntities, typeData.m_SizeInBytes, typeData.m_Alignment });
		//}
	}

	EntityID Archetype::RemoveEntityRow(u64 entityRow) {
		CKE_ASSERT(m_NumEntities > 0);

		EntityID movedEntityID = m_RowIndexToEntity[m_NumEntities - 1];
		m_RowIndexToEntity[m_NumEntities - 1] = EntityID::Invalid();
		m_RowIndexToEntity[entityRow] = movedEntityID;

		for (auto&& componentArray : m_ArchTable) {
			componentArray.RemoveCompAt(entityRow);
		}

		m_NumEntities--;
		return movedEntityID;
	}

	u64 Archetype::AddEntityRow(EntityID associatedEntity) {
		m_NumEntities++;

		// Create a row in all of the component arrays
		// for the entity data
		u64 entityArchetypeRow = -1;
		for (auto&& componentArray : m_ArchTable) {
			entityArchetypeRow = componentArray.AppendComponentToEnd();
		}
		m_RowIndexToEntity[entityArchetypeRow] = associatedEntity;

		return entityArchetypeRow;
	}
}
