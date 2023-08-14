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

	// Iterator for multi-component queries
	class MultiComponentIter
	{
	public:
		// Setup
		//-----------------------------------------------------------------------------

		MultiComponentIter(EntityDatabase* pEntityAdmin, Vector<ComponentTypeID>& componentID);

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
		MultiComponentIter() = default;

		inline void SetArchetypeToIterate(ArchetypeID archID);
		inline void BeginIteratorSetup();
		void        Initialize(EntityDatabase* pEntityAdmin, Vector<ComponentTypeID>& componentID);

	protected:
		u64         m_CurrRowInArch = 0;
		Vector<u64> m_CurrCompColumnsInArch;

		// Cached Variables to avoid constant lookups
		Vector<ComponentArray*> m_pCurrCompArrays;
		Archetype*              m_pCurrArch = nullptr;

		u64 m_NumCompsInCurrArch = 0; // Total number of components in current archetype
		u64 m_CurrArchIDIndex = 0;    // Current Archetype index in the matched arch IDS

		Vector<ComponentTypeID> m_CompsToIterate;
		Vector<ArchetypeID>     m_MatchedArchIDs;

		Map<ComponentTypeID, Map<ArchetypeID, ArchetypeComponentColumn>>* m_pComponentToArchetypes = nullptr;
		Map<ArchetypeID, Archetype*>*                                     m_IDToArchetype = nullptr;

		u64 m_NumEntitiesTotal = 0;    // Total number of components to iterate in all archetypes
		u64 m_NumEntitiesIterated = 0; // Total number of components already iterated

		ComponentTuple m_OutCompTuple{}; // Tuple returned with each iteration
	};

	//-----------------------------------------------------------------------------

	// Template iterator for multi-component queries
	template <typename Comp, typename... OtherComp>
	class TMultiComponentIter : public MultiComponentIter
	{
	public:
		explicit TMultiComponentIter(EntityDatabase* pEntityAdmin);

		// Range-for iterator
		inline TMultiComponentIter               begin();
		inline std::tuple<Comp*, OtherComp*...>& operator*();

	private:
		// Auxiliary
		template <size_t I = 0, typename... Ts>
		constexpr inline void PopulateVectorWithComponentIDs(Vector<ComponentTypeID>& vec);
		template <size_t I = 0, typename... Ts>
		constexpr inline void PopulateTupleWithComponents(std::tuple<Ts...>& tuple);

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

	void MultiComponentIter::SetArchetypeToIterate(ArchetypeID archID) {
		// Setup necessary data to iterate the next archetype
		ArchetypeID newArchID = archID;
		m_pCurrArch = m_IDToArchetype->at(newArchID);
		m_NumCompsInCurrArch = m_pCurrArch->m_NumEntities;

		m_CurrCompColumnsInArch.clear();
		for (ComponentTypeID const& compID : m_CompsToIterate) {
			u64 compCol = m_pComponentToArchetypes->at(compID).at(newArchID);
			m_CurrCompColumnsInArch.emplace_back(compCol);
			m_pCurrCompArrays.emplace_back(&m_pCurrArch->m_ArchTable[compCol]);
		}
	}

	void MultiComponentIter::BeginIteratorSetup() {
		// Set the first archetype to start iterating
		if (!m_MatchedArchIDs.empty()) {
			SetArchetypeToIterate(m_MatchedArchIDs[0]);
		}
	}

	MultiComponentIter MultiComponentIter::begin() {
		BeginIteratorSetup();
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
		m_CurrRowInArch++;

		// If we exhausted the current archetype, go to the next one
		if (m_CurrRowInArch >= m_NumCompsInCurrArch) {
			// Update the entities processed counter
			m_NumEntitiesIterated += m_NumCompsInCurrArch;

			// Advance to the next archetype
			m_CurrRowInArch = 0;
			m_CurrArchIDIndex++;

			// If this is true then we finished iterating all of the archetypes
			if (m_CurrArchIDIndex >= m_MatchedArchIDs.size()) {
				m_CurrArchIDIndex = 0;
			}

			// Setup necesary data to iterate the next archetype
			SetArchetypeToIterate(m_MatchedArchIDs[m_CurrArchIDIndex]);
		}
	}

	inline void MultiComponentIter::operator+(int i) {
		m_CurrRowInArch += i;
		while (m_CurrRowInArch >= m_NumCompsInCurrArch) {
			m_CurrArchIDIndex++;
			m_CurrRowInArch -= m_NumCompsInCurrArch;

			if (m_CurrArchIDIndex >= m_MatchedArchIDs.size()) {
				m_CurrArchIDIndex = 0;
			}

			m_pCurrArch = m_IDToArchetype->at(m_MatchedArchIDs[m_CurrArchIDIndex]);
			m_NumCompsInCurrArch = m_pCurrArch->m_NumEntities;
		}

		SetArchetypeToIterate(m_MatchedArchIDs[m_CurrArchIDIndex]);
	}

	ComponentTuple* MultiComponentIter::operator*() {
		Vector<void*>& compTupleArr = m_OutCompTuple.m_Components;
		compTupleArr.clear();
		for (int i = 0; i < m_CompsToIterate.size(); ++i) {
			compTupleArr.push_back(m_pCurrCompArrays[i]->GetCompAtIndex(m_CurrRowInArch));
		}
		return &m_OutCompTuple;
	}
}

namespace CKE {
	template <typename Comp, typename... Other>
	template <size_t I, typename... Ts>
	constexpr void TMultiComponentIter<Comp, Other...>::PopulateVectorWithComponentIDs(Vector<ComponentTypeID>& vec) {
		if constexpr (I == sizeof...(Ts)) { return; }
		else {
			vec.emplace_back(ComponentStaticTypeID<std::tuple_element_t<I, std::tuple<Ts...>>>::s_CompID);
			PopulateVectorWithComponentIDs<I + 1, Ts...>(vec);
		}
	}

	template <typename Comp, typename... Other>
	template <size_t I, typename... Ts>
	constexpr void TMultiComponentIter<Comp, Other...>::PopulateTupleWithComponents(std::tuple<Ts...>& tuple) {
		if constexpr (I == sizeof...(Ts)) { return; }
		else {
			std::get<I>(tuple) = (std::tuple_element_t<I, std::tuple<Ts...>>)m_pCurrCompArrays.at(I)->
					GetCompAtIndex(m_CurrRowInArch);
			PopulateTupleWithComponents<I + 1>(tuple);
		}
	}

	template <typename Comp, typename... Other>
	TMultiComponentIter<Comp, Other...>::TMultiComponentIter(EntityDatabase* pEntityAdmin) {
		// Convert template tuple into a vector with the component IDs
		Vector<ComponentTypeID> componentIDs;
		PopulateVectorWithComponentIDs<0, Comp, Other...>(componentIDs);
		Initialize(pEntityAdmin, componentIDs);
	}

	template <typename Comp, typename... Other>
	TMultiComponentIter<Comp, Other...> TMultiComponentIter<Comp, Other...>::begin() {
		BeginIteratorSetup();
		return *this;
	}

	template <typename Comp, typename... Other>
	std::tuple<Comp*, Other*...>& TMultiComponentIter<Comp, Other...>::operator*() {
		PopulateTupleWithComponents(m_CompTuple);
		return m_CompTuple;
	}
}
