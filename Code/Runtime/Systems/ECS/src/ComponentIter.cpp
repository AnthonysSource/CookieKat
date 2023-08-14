#include "CookieKat/Core/Platform/Asserts.h"
#include "EntityDatabase.h"

#include <ranges>

namespace CKE {
	ComponentIter::ComponentIter(EntityDatabase* pEntityAdmin, ComponentTypeID componentID) {
		Map<ArchetypeID, Archetype*> const* pIDToArchetype = &pEntityAdmin->m_IDToArchetype;

		// Early exit if there isn't any archetype that contains the component
		if (!pEntityAdmin->m_ComponentToArchetypes.contains(componentID)) {
			m_NumEntitiesTotal = 0;
			return;
		}

		// Calculate the total num of entities that have the given component
		// and cache all of the archetype ids with the column where the component is located
		// into an array to iterate later
		Map<ArchetypeID, ArchetypeComponentColumn> const& archetypeColumnMap =
			pEntityAdmin->m_ComponentToArchetypes.at(componentID);
		for (auto const& [archetypeID, compColumn] : archetypeColumnMap) {
			m_CompArchAccessData.emplace_back(ArchetypeColumnPair{ pIDToArchetype->at(archetypeID), compColumn });
			m_NumEntitiesTotal += pIDToArchetype->at(archetypeID)->m_NumEntities;
		}
	}

	ComponentIter::ComponentIter(Vector<ArchetypeColumnPair> const& accessData, u64 totalEntitiesCount) {
		m_CompArchAccessData = accessData;
		m_NumEntitiesTotal = totalEntitiesCount;
	}

	MultiComponentIter::MultiComponentIter(EntityDatabase* pEntityAdmin, ComponentSet& componentID) {
		Initialize(pEntityAdmin, componentID);
	}

	void MultiComponentIter::Initialize(EntityDatabase* pEntityAdmin, Vector<ComponentTypeID>& componentID) {
		m_IDToArchetype = &pEntityAdmin->m_IDToArchetype;
		m_pComponentToArchetypes = &pEntityAdmin->m_ComponentToArchetypes;
		m_CompsToIterate = componentID;

		CKE_ASSERT(!componentID.empty());
		// Early exit
		if (!pEntityAdmin->m_ComponentToArchetypes.contains(componentID[0])) {
			m_NumEntitiesTotal = 0;
			return;
		}

		// We have a vector where we will store the matched
		// archetypes that contain the given components
		// Fill the vector with archetypes that contain the first component
		for (ArchetypeID const& archetypeID : pEntityAdmin->m_ComponentToArchetypes.at(componentID[0]) | std::views::keys) {
			m_MatchedArchIDs.emplace_back(archetypeID);
		}

		// For each other component, we check if the other component is used 
		// in the same archetype as the first component and if its not
		// the case then we eliminate the archetype from the match list
		for (u64 compIndex = 1; compIndex < componentID.size(); ++compIndex) {
			for (i64 matchedArchIndex = m_MatchedArchIDs.size() - 1; matchedArchIndex >= 0; --matchedArchIndex) {
				if (!pEntityAdmin->m_ComponentToArchetypes.at(componentID[compIndex]).contains(m_MatchedArchIDs[matchedArchIndex])) {
					m_MatchedArchIDs[matchedArchIndex] = m_MatchedArchIDs[m_MatchedArchIDs.size() - 1];
					m_MatchedArchIDs.pop_back();
				}
			}
		}

		// Calculate total num of entities that match the component requirements
		for (ArchetypeID archID : m_MatchedArchIDs) {
			m_NumEntitiesTotal += m_IDToArchetype->at(archID)->m_NumEntities;
		}
	}
}
