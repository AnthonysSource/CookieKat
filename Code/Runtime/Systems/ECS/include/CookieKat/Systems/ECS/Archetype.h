#pragma once

#include "CookieKat/Core/Containers/Containers.h"

#include "IDs.h"
#include "ComponentArray.h"

namespace CKE {
	// Contains component data of all of the entities that have the exact
	// component signature as the archetype, conceptually works as a 2D table
	class Archetype
	{
	public:
		Archetype(ArchetypeID id, ComponentSet const& componentSet, u64 maxEntities);

		ArchetypeID             m_ID{0};              // Unique ID of the archetype
		Vector<ComponentTypeID> m_ComponentSet{};     // Unique set of component IDs used by the archetype
		Vector<ComponentArray>  m_ArchTable{};        // Table with the component data
		u64                     m_NumEntities{0};     // Number of entities in the component table
		Vector<EntityID>        m_RowIndexToEntity{}; // Given a row, returns the associated entity ID

	public:
		// Add a new row to the archetype table
		// Returns the index of the added row
		u64 AddEntityRow(EntityID associatedEntity);

		// Removes the given row from the archetype table
		// Returns the associated entity ID of the row that has been moved to fill the gap
		EntityID RemoveEntityRow(u64 entityRow);

		// Returns a pointer to the component data of the given position in the archetype table
		inline void* GetComponentAt(u64 componentColumn, u64 entityRow);
	};
}

//-----------------------------------------------------------------------------

namespace CKE {
	void* Archetype::GetComponentAt(u64 componentColumn, u64 entityRow) {
		return m_ArchTable[componentColumn].GetCompAtIndex(entityRow);
	}
}
