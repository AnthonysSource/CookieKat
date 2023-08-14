#include "EntityDatabase.h"

#include <algorithm>
#include <iomanip>
#include <ranges>

namespace CKE {
	EntityDatabase::EntityDatabase(u64 maxEntities) {
		Initialize(maxEntities);
	}

	void EntityDatabase::Initialize(u64 maxEntities) {
		m_MaxNumEntities = maxEntities;
		m_Entities.reserve(maxEntities);
		m_ComponentTypes.reserve(25'000);
		m_Archetypes.reserve(250'000);
	}

	void EntityDatabase::Shutdown() { }

	ComponentTypeID EntityDatabase::RegisterComponent(const char* name, u64 sizeInBytes, u32 alignment) {
		m_LastComponentID++;

		ComponentTypeData typeData{};
		typeData.m_Name = name;
		typeData.m_SizeInBytes = sizeInBytes;
		typeData.m_Alignment = alignment;

		m_ComponentTypeData.insert({m_LastComponentID, typeData});
		m_ComponentTypes.push_back(m_LastComponentID);
		return m_LastComponentID;
	}

	ComponentSetID EntityDatabase::CalculateComponentSetID(Vector<ComponentTypeID> const& componentSet) {
		Vector<ComponentTypeID> compVec = componentSet;
		std::sort(compVec.begin(), compVec.end());

		ComponentSetID id = 254633;
		for (int i = 0; i < compVec.size(); ++i) {
			id ^= std::hash<ComponentSetID>()(compVec[i]) << i;
		}
		return id;
	}

	ArchetypeComponentColumn EntityDatabase::GetComponentColumnInArchetype(
		ComponentTypeID component, ArchetypeID archetypeID) const {
		CKE_ASSERT(m_ComponentToArchetypes.contains(component));
		CKE_ASSERT(m_ComponentToArchetypes.at(component).contains(archetypeID));

		return m_ComponentToArchetypes.at(component).at(archetypeID);
	}

	void EntityDatabase::CreateArchetype(Vector<ComponentTypeID> const& componentSet) {
		// Generate a new archetype ID
		m_LastArchetypeID++;

		// Calculate an unique ID for the given component set
		ComponentSetID componentSetID = CalculateComponentSetID(componentSet);

		// Create the archetype and initialize some basic data
		Archetype& archetype = m_Archetypes.emplace_back(Archetype{ m_LastArchetypeID, componentSet, m_MaxNumEntities });

		// Set data relationships
		m_ComponentSetToArchetype.insert({componentSetID, &archetype});
		m_IDToArchetype.insert({m_LastArchetypeID, &archetype});

		// Initialize the archetype component table
		int componentColumn = 0;
		for (ComponentTypeID componentID : componentSet) {
			CKE_ASSERT(m_ComponentTypeData.contains(componentID));
			ComponentTypeData const& typeData = m_ComponentTypeData.at(componentID);

			if (typeData.m_SizeInBytes == 0) { continue; } // If a component has size 0 don't create an array for it

			// Create component array
			archetype.m_ArchTable.push_back(ComponentArray{componentID, m_MaxNumEntities, typeData.m_SizeInBytes, typeData.m_Alignment});

			// If we find the component doesn't have a relationship
			// with any archetype then we create it
			if (!m_ComponentToArchetypes.contains(componentID)) {
				m_ComponentToArchetypes.insert({componentID, Map<ArchetypeID, ArchetypeComponentColumn>()});
			}

			// Update component to archetypes relationship
			// The component column is given by its position in the component set array
			m_ComponentToArchetypes.at(componentID).insert({m_LastArchetypeID, componentColumn});

			componentColumn++;
		}
	}

	bool EntityDatabase::HasComponent(EntityID entity, ComponentTypeID componentID) {
		CKE_ASSERT(m_ComponentTypeData.contains(componentID)); // Check that the component has been registered
		CKE_ASSERT(m_EntityToRecord.contains(entity));         // Check that the entity exists;

		ArchetypeID entityArchID = m_EntityToRecord[entity].m_pArchetype->m_ID;
		auto&&      compArchetypes = m_ComponentToArchetypes[componentID];
		return compArchetypes.contains(entityArchID);
	}

	void EntityDatabase::AddSingletonComponent(ComponentTypeID componentID, void* pComponentData) {
		CKE_ASSERT(!m_IDToSingletonComponents.contains(componentID));
		CKE_ASSERT(pComponentData != nullptr);

		SingletonComponentRecord record{};
		record.m_SizeInBytes = m_ComponentTypeData.at(componentID).m_SizeInBytes;
		record.m_pComponentData = CKE::Alloc(m_ComponentTypeData.at(componentID).m_SizeInBytes);
		memcpy(record.m_pComponentData, pComponentData, sizeof(SingletonComponentRecord));
		m_IDToSingletonComponents.insert({componentID, record});
	}

	void EntityDatabase::RemoveSingletonComponent(ComponentTypeID componentID) {
		CKE_ASSERT(m_IDToSingletonComponents.contains(componentID));
		m_IDToSingletonComponents.erase(componentID);
	}

	void* EntityDatabase::GetSingletonComponent(ComponentTypeID componentID) {
		CKE_ASSERT(m_IDToSingletonComponents.contains(componentID));
		return m_IDToSingletonComponents.at(componentID).m_pComponentData;
	}

	void* EntityDatabase::GetComponent(EntityID entity, ComponentTypeID componentID) {
		CKE_ASSERT(m_ComponentTypeData.contains(componentID)); // Check that the component has been registered
		CKE_ASSERT(m_EntityToRecord.contains(entity));         // Check that the entity exists;

		auto&      entityRecord = m_EntityToRecord[entity];
		Archetype* pArchetype = entityRecord.m_pArchetype;
		u64        row = entityRecord.m_EntityArchetypeRow;

		if (HasComponent(entity, componentID)) {
			u64             compColumnInArchTable = GetComponentColumnInArchetype(componentID, pArchetype->m_ID);
			ComponentArray* componentArray = &pArchetype->m_ArchTable.at(compColumnInArchTable);
			u8*             pComponentData = componentArray->GetCompAtIndex(row);
			return pComponentData;
		}
		CKE_UNREACHABLE_CODE();
		return nullptr;
	}

	void EntityDatabase::AddComponent(EntityID entityID, ComponentTypeID componentID, void* pComponentData) {
		CKE_ASSERT(m_ComponentTypeData.contains(componentID)); // Check that the component has been registered
		CKE_ASSERT(m_EntityToRecord.contains(entityID));       // Check that the entity exists

		// Cache data from soon to be old entity archetype
		EntityRecord& record = m_EntityToRecord[entityID];
		Archetype*    pOldArchetype = record.m_pArchetype;
		u64           oldArchetypeRow = record.m_EntityArchetypeRow;

		// Define new Entity Component Set and calculate its ID
		Vector<ComponentTypeID> newComponentSet = pOldArchetype->m_ComponentSet;
		newComponentSet.push_back(componentID);
		ComponentSetID newComponentSetID = CalculateComponentSetID(newComponentSet);

		// Find or create the new archetype for the component set
		if (!m_ComponentSetToArchetype.contains(newComponentSetID)) {
			CreateArchetype(newComponentSet);
		}
		Archetype* pNewArchetype = m_ComponentSetToArchetype.at(newComponentSetID);

		// Create a new row in the new archetype
		u64 newArchetypeRow = pNewArchetype->AddEntityRow(entityID);

		// Copy old component data into new archetype
		// At this point there is a row for the entity in both archetypes
		for (ComponentTypeID& compID : pOldArchetype->m_ComponentSet) {
			MoveComponentDataFromToArch(pOldArchetype, oldArchetypeRow, pNewArchetype, newArchetypeRow, compID);
		}

		// Copy newly added component data into new archetype
		CKE_ASSERT(m_ComponentTypeData.contains(componentID)); // Component not registered
		// Only copy if size is bigger than 0, if its 0 it means its just a tag component
		u64 compSizeInBytes = m_ComponentTypeData.at(componentID).m_SizeInBytes;
		if (compSizeInBytes > 0) {
			CKE_ASSERT(pComponentData != nullptr); // Check that we have passed actual data to copy
			u64 newCompColumn = GetComponentColumnInArchetype(componentID, pNewArchetype->m_ID);
			u8* pComp = static_cast<u8*>(pNewArchetype->GetComponentAt(newCompColumn, newArchetypeRow));
			memcpy(pComp, pComponentData, compSizeInBytes);
		}

		// Update Record to point to new archetype
		record.m_EntityArchetypeRow = newArchetypeRow;
		record.m_pArchetype = pNewArchetype;

		// Remove entity it from the previous archetype
		EntityID movedEntityID = pOldArchetype->RemoveEntityRow(oldArchetypeRow);
		// When we remove a row from an archetype, we need to fill the hole left in that position so we move
		// the last element of the array to the removed row.
		// We have to find the entity that points to that moved row and update it.
		if (entityID != movedEntityID) {
			m_EntityToRecord[movedEntityID].m_EntityArchetypeRow = oldArchetypeRow;
		}
	}

	void EntityDatabase::MoveComponentDataFromToArch(Archetype*       pOldArchetype, u64 oldArchetypeRow,
	                                                 Archetype*       pNewArchetype, u64 newArchetypeRow,
	                                                 ComponentTypeID& compID) {
		// Get the column of the current component in each archetype
		u64 oldCompColumnInOldArchetype = GetComponentColumnInArchetype(compID, pOldArchetype->m_ID);
		u64 oldCompColumnInNewArchetype = GetComponentColumnInArchetype(compID, pNewArchetype->m_ID);

		// Get the size of the component in bytes
		u64 compSizeInBytes = m_ComponentTypeData[compID].m_SizeInBytes;

		// Get the pointers of the component locations and copy the data
		u8* pOldComp = (u8*)pOldArchetype->GetComponentAt(oldCompColumnInOldArchetype, oldArchetypeRow);
		u8* pNewComp = (u8*)pNewArchetype->GetComponentAt(oldCompColumnInNewArchetype, newArchetypeRow);
		memcpy(pNewComp, pOldComp, compSizeInBytes);
	}

	EntityDatabaseDebugger EntityDatabase::GetDebugger() {
		return EntityDatabaseDebugger{*this};
	}

	void EntityDatabase::PrintAdminState() {
		u32 numArchetypesInUse = 0;
		for (Archetype& arch : m_Archetypes) { if (arch.m_NumEntities != 0) { numArchetypesInUse++; } }

		std::cout << "-------------------------------------------------------------------" << std::endl;
		std::cout << "	Entity Admin State" << std::endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;
		std::cout << "Num Entities: " << m_Entities.size() << std::endl;
		std::cout << "Num Component Types: " << m_ComponentTypes.size() << std::endl;
		std::cout << "Num Archetypes: " << m_Archetypes.size() << " (" << numArchetypesInUse << " In Use)" <<
				std::endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;

		for (auto& compTypePair : m_ComponentTypeData) {
			std::cout << "ID: " << compTypePair.first << " - " << compTypePair.second.m_Name << " - " <<
					compTypePair.second.m_SizeInBytes << " Bytes" << std::endl;
		}

		std::cout << "-------------------------------------------------------------------" << std::endl;

		for (Archetype& arch : m_Archetypes) {
			if (arch.m_NumEntities == 0) { continue; }

			std::cout << "Archetype " << arch.m_ID << " - " << arch.m_NumEntities << " Entities" << std::endl;
			Vector<ComponentTypeID>& sortedSet = arch.m_ComponentSet;
			std::sort(sortedSet.begin(), sortedSet.end());
			for (auto compID : sortedSet) {
				std::cout << "    " << m_ComponentTypeData.at(compID).m_Name << std::endl;
			}
			std::cout << std::endl;
		}
	}

	void EntityDatabase::PrintEntityState(EntityID entityID) {
		EntityRecord& record = m_EntityToRecord[entityID];
		Archetype*    pArch = record.m_pArchetype;

		std::cout << "Entity " << entityID.GetValue() << std::endl;

		std::ios_base::fmtflags f(std::cout.flags());

		for (ComponentTypeID archCompID : record.m_pArchetype->m_ComponentSet) {
			ArchetypeComponentColumn compCol = m_ComponentToArchetypes.at(archCompID).at(record.m_pArchetype->m_ID);
			String&                  compName = m_ComponentTypeData[archCompID].m_Name;
			u64                      compSize = m_ComponentTypeData[archCompID].m_SizeInBytes;
			u8*                      compData = pArch->m_ArchTable[compCol].GetCompAtIndex(record.m_EntityArchetypeRow);

			std::cout << "    " << compName << " " << compSize << " Bytes - ";
			for (int i = 0; i < compSize; ++i) {
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)compData[i];
			}
			std::cout << std::dec << std::endl;
		}

		std::cout.flags(f);
	}

	EntityDatabaseStateSnapshot EntityDatabaseDebugger::GetStateSnapshot() const {
		EntityDatabaseStateSnapshot s{};
		s.m_NumEntities = m_Db->m_Entities.size();
		s.m_NumComponentTypes = m_Db->m_ComponentTypes.size();
		s.m_NumArchetypes = m_Db->m_Archetypes.size();
		return s;
	}

	void EntityDatabaseDebugger::PrintGeneralState() const {
		u32 numArchetypesInUse = 0;
		for (Archetype& arch : m_Db->m_Archetypes) { if (arch.m_NumEntities != 0) { numArchetypesInUse++; } }

		std::cout << "-------------------------------------------------------------------" << std::endl;
		std::cout << "	Entity Admin State" << std::endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;
		std::cout << "Num Entities: " << m_Db->m_Entities.size() << std::endl;
		std::cout << "Num Component Types: " << m_Db->m_ComponentTypes.size() << std::endl;
		std::cout << "Num Archetypes: " << m_Db->m_Archetypes.size() << " (" << numArchetypesInUse << " In Use)" <<
				std::endl;
		std::cout << "-------------------------------------------------------------------" << std::endl;

		for (auto& compTypePair : m_Db->m_ComponentTypeData) {
			std::cout << "ID: " << compTypePair.first << " - " << compTypePair.second.m_Name << " - " <<
					compTypePair.second.m_SizeInBytes << " Bytes" << std::endl;
		}

		std::cout << "-------------------------------------------------------------------" << std::endl;

		for (Archetype& arch : m_Db->m_Archetypes) {
			if (arch.m_NumEntities == 0) { continue; }

			std::cout << "Archetype " << arch.m_ID << " - " << arch.m_NumEntities << " Entities" << std::endl;
			Vector<ComponentTypeID>& sortedSet = arch.m_ComponentSet;
			std::sort(sortedSet.begin(), sortedSet.end());
			for (auto compID : sortedSet) {
				std::cout << "    " << m_Db->m_ComponentTypeData.at(compID).m_Name << std::endl;
			}
			std::cout << std::endl;
		}
	}

	void EntityDatabase::RemoveComponent(EntityID entityID, ComponentTypeID componentID) {
		CKE_ASSERT(m_ComponentTypeData.contains(componentID)); // Check that the component has been registered
		CKE_ASSERT(m_EntityToRecord.contains(entityID));       // Check that the entity exists;

		// Cache data from soon to be old entity archetype
		EntityRecord& entityRecord = m_EntityToRecord[entityID];
		Archetype*    pOldArchetype = entityRecord.m_pArchetype;
		u64           oldArchetypeRow = entityRecord.m_EntityArchetypeRow;

		// Define new Entity Component Set and calculate its ID
		Vector<ComponentTypeID> newComponentSet = pOldArchetype->m_ComponentSet;
		for (int i = 0; i < newComponentSet.size(); ++i) {
			if (newComponentSet[i] == componentID) {
				newComponentSet[i] = newComponentSet[newComponentSet.size() - 1];
				newComponentSet.pop_back();
			}
		}
		ComponentSetID newComponentSetID = CalculateComponentSetID(newComponentSet);

		// Find or create the new archetype for the component set
		// (High chances of not creating a new archetype)
		if (!m_ComponentSetToArchetype.contains(newComponentSetID)) { CreateArchetype(newComponentSet); }
		Archetype* pNewArchetype = m_ComponentSetToArchetype.at(newComponentSetID);

		// Create a new row in the new archetype
		u64 newArchetypeRow = pNewArchetype->AddEntityRow(entityID);

		// Copy old component data into new archetype
		// At this point there is a row for the entity in both archetypes
		// Only copy the components present in the new archetype
		for (ComponentTypeID& compID : pNewArchetype->m_ComponentSet) {
			MoveComponentDataFromToArch(pOldArchetype, oldArchetypeRow, pNewArchetype, newArchetypeRow, compID);
		}

		// Update Record to point to new archetype
		entityRecord.m_EntityArchetypeRow = newArchetypeRow;
		entityRecord.m_pArchetype = pNewArchetype;

		// Remove entity it from the previous archetype
		EntityID movedEntityID = pOldArchetype->RemoveEntityRow(oldArchetypeRow);
		m_EntityToRecord[movedEntityID].m_EntityArchetypeRow = oldArchetypeRow;
	}

	EntityComponentIterator EntityDatabase::GetEntityIterator(ComponentTypeID componentID) {
		EntityComponentIterator componentIterator(this, componentID);
		return componentIterator;
	}

	void EntityDatabase::QueryArchetypes(QueryInfo const& queryInfo) {
		Vector<ArchetypeID> matchedArchetypes;
		u64                 totalEntitiesCount = 0;

		CKE_ASSERT(queryInfo.m_QueryElementsCount > 0);

		// Acquire entities that match the first component
		for (ArchetypeID const& archetypeID : m_ComponentToArchetypes.at(queryInfo.m_QueryElements[0].m_ComponentID) |
		     std::ranges::views::keys) {
			matchedArchetypes.push_back(archetypeID);
		}

		// Find and remove archetypes that don't match the rest of the components
		for (i32 i = 1; i < queryInfo.m_QueryElementsCount; ++i) {
			QueryElement const& q = queryInfo.m_QueryElements[i];

			for (i64 matchedArchIndex = matchedArchetypes.size() - 1; matchedArchIndex >= 0; --matchedArchIndex) {
				if (!m_ComponentToArchetypes.at(q.m_ComponentID).contains(matchedArchetypes[matchedArchIndex])) {
					// Remove it and fill the gap
					matchedArchetypes[matchedArchIndex] = matchedArchetypes[matchedArchetypes.size() - 1];
					matchedArchetypes.pop_back();
				}
			}
		}

		// Calculate total entity count of all matched archetypes
		for (ArchetypeID archID : matchedArchetypes) {
			totalEntitiesCount += m_IDToArchetype.at(archID)->m_NumEntities;
		}
	}

	void EntityDatabase::QueryArchetypesSingleComponent(ComponentTypeID compTypeID, Vector<ArchetypeColumnPair>& accessData,
	                                                    u64&            totalEntitiesCount) {
		Map<ArchetypeID, Archetype*> const* pIDToArchetype = &m_IDToArchetype;

		// Early exit if there isn't any archetype that contains the component
		if (!m_ComponentToArchetypes.contains(compTypeID)) {
			totalEntitiesCount = 0;
			return;
		}

		// Calculate the total num of entities that have the given component
		// and cache all of the archetype ids with the column where the component is located
		// into an array to iterate later
		accessData.clear();
		Map<ArchetypeID, ArchetypeComponentColumn> const& archetypeColumnMap = m_ComponentToArchetypes.at(compTypeID);
		for (auto const& [archetypeID, compColumn] : archetypeColumnMap) {
			accessData.emplace_back(ArchetypeColumnPair{pIDToArchetype->at(archetypeID), compColumn});
			totalEntitiesCount += pIDToArchetype->at(archetypeID)->m_NumEntities;
		}
	}

	ComponentIter EntityDatabase::GetSingleCompIter(ComponentTypeID componentID) {
		Vector<ArchetypeColumnPair> accessData;
		u64                         totalEntitiesCount = 0;
		QueryArchetypesSingleComponent(componentID, accessData, totalEntitiesCount);
		ComponentIter compIterator(accessData, totalEntitiesCount);
		return compIterator;
	}

	MultiComponentIter EntityDatabase::GetMultiCompIter(ComponentSet componentID) {
		MultiComponentIter compIter(this, componentID);
		return compIter;
	}

	EntityID EntityDatabase::CreateEntity() {
		CKE_ASSERT(m_Entities.size() < m_MaxNumEntities);         // Check that we didn't run out of space
		m_NextEntityID = EntityID{m_NextEntityID.GetValue() + 1}; // Advance unique entity ID counter
		m_Entities.push_back(m_NextEntityID);                     // Add entity to global entity list for tracking

		// Note: Having to create an empty component set and an empty archetype
		// here is a bit weird, maybe we could just generate this special archetype
		// at startup and give it a special identifier, although its not really a big deal

		// Generate empty component set
		Vector<ComponentTypeID> componentSet{};
		ComponentSetID          componentSetID = CalculateComponentSetID(componentSet);

		// Generate archetype for entities with 0 components if necessary
		// We don't create a row in this empty archetype because it doesn't make sense
		if (!m_ComponentSetToArchetype.contains(componentSetID)) { CreateArchetype(componentSet); }
		Archetype* arch = m_ComponentSetToArchetype.at(componentSetID);
		arch->m_NumEntities++;

		// Generate the relationship between entity and archetype
		EntityRecord entityRecord{};
		entityRecord.m_EntityArchetypeRow = arch->m_NumEntities - 1;
		entityRecord.m_pArchetype = arch;
		m_EntityToRecord.insert({m_NextEntityID, entityRecord});

		return m_NextEntityID;
	}

	void EntityDatabase::DeleteEntity(EntityID entity) {
		CKE_ASSERT(m_EntityToRecord.contains(entity));

		// Remove entity from entities array
		// TODO: This is insanely slow
		for (u64 i = 0; i < m_Entities.size(); ++i) {
			if (m_Entities[i] == entity) {
				m_Entities[i] = m_Entities[m_Entities.size() - 1];
				m_Entities.pop_back();
				break;
			}
		}

		// Start removing entity component row from its archetype table
		EntityRecord& record = m_EntityToRecord[entity];
		EntityID      movedEntityID = record.m_pArchetype->RemoveEntityRow(record.m_EntityArchetypeRow);
		// Update the record of the moved entity
		m_EntityToRecord[movedEntityID].m_EntityArchetypeRow = record.m_EntityArchetypeRow;

		// Erase entity to record relationship
		m_EntityToRecord.erase(entity);
	}
}
