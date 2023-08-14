#pragma once

#include "CookieKat/Core/Containers/Containers.h"

#include "IDs.h"
#include "Archetype.h"
#include "EntityQuery.h"
#include "Iterators/ComponentIterator.h"
#include "Iterators/IteratorCommon.h"
#include "Iterators/MultiComponentIterator.h"
#include "Iterators/EntityComponentIterator.h"
#include "CookieKat/Core/Memory/PoolAllocator.h"

#include <typeinfo>
#include <functional>

namespace CKE {
	// We require all components to be trivially copyable so
	// that we can memcpy their data when necessary
	// NOTE: This requirement is in quite arbitrary and could be changed if needed
	template <typename T>
	concept CanBeComponent = std::is_trivially_copyable_v<T>;
}

namespace CKE {
	// Main ECS Database
	// Manages all of the entity component data
	class EntityDatabase
	{
	public:
		// Create an uninitialized entity database
		EntityDatabase() = default;

		// Constructs the database and implicitly calls Initialize(...)
		explicit EntityDatabase(u64 maxEntities);

		//-----------------------------------------------------------------------------
		// Lifetime
		//-----------------------------------------------------------------------------

		// Initializes the database and sets the maximum number of entities
		void Initialize(u64 maxEntities);

		// Shutdowns the system and releases its resources
		void Shutdown();

		//-----------------------------------------------------------------------------
		// Component Registration
		//-----------------------------------------------------------------------------

		// Registers a component with the given description.
		// If the component has size = 0 then it works as a Tag
		ComponentTypeID RegisterComponent(const char* name, u64 sizeInBytes, u32 alignment);

		// Registers the component of type T with the database
		template <typename T>
			requires CanBeComponent<T>
		ComponentTypeID RegisterComponent();

		// Use this if you registered a component using the un-templated method but you
		// want to assign the component ID to the static type so you can still use
		// the rest of the template API.
		template <typename T>
		void AssignComponentIDToStaticType(ComponentTypeID id);

		//-----------------------------------------------------------------------------
		// Iterators
		//-----------------------------------------------------------------------------

		// Type-less
		//-----------------------------------------------------------------------------

		// Returns an iterator that iterates over a single component type
		//
		// Example:
		//   for(void* pPos : db.GetSingleCompIter(positionID)){
		//     DoSomething((Position*)pPos);
		//   }
		ComponentIter GetSingleCompIter(ComponentTypeID componentID);

		// Returns an iterator that iterates over component data of entities that contain, at least
		// the supplied component set
		//
		// Example:
		//   for(ComponentTuple* pTuple : db.GetMultiCompIter(positionID, velocityID)){
		//	   Position* pPos = (Position*)compTuple->GetComponent(0);
		//     Velocity* pVel = compTuple->GetComponent<Velocity>(1);
		//     DoSomething(pPos, pVel);
		//   }
		MultiComponentIter GetMultiCompIter(ComponentSet componentID);

		// Returns an iterator that iterates over a single component type and provides
		// the EntityID of the current entity. If you don't need access to the entityID
		// we recommend using other iterators as this adds some overhead.
		//
		// Example:
		//   for(EntityComponentPair* pPair : db.GetEntityIterator(positionID)){
		//     DoSomething(pPair->m_EntityID, (Position*)pPair->m_pComponent);
		//   }
		EntityComponentIterator GetEntityIterator(ComponentTypeID componentID);

		// Templated
		//-----------------------------------------------------------------------------

		// Returns a templated single-component iterator
		//
		// Example:
		//   for(Position* pPos : db.GetSingleCompIter<Position>()){
		//     DoSomething(pPos);
		//   }
		template <typename T>
		TComponentIterator<T> GetSingleCompIter();

		// Returns a templated multi-component iterator
		//
		// Example:
		//   for(ComponentTuple* pTuple : db.GetMultiCompIter<Position, Velocity>()){
		//	   Position* pPos = compTuple->GetComponent<Position>(0);
		//     Velocity* pVel = compTuple->GetComponent<Velocity>(1);
		//     DoSomething(pPos, pVel);
		//   }
		template <typename T, typename... Other>
		MultiComponentIter GetMultiCompIter();

		// Returns a templated multi-component iterator that, on each iteration,
		// provides a tuple that can be unpacked.
		//
		// Example:
		//   for(auto [pos, vel] : db.GetMultiCompTupleIter<Position, Velocity>()){
		//	   DoSomething(pos, vel);
		//   }
		template <typename T, typename... Other>
		TMultiComponentIter<T, Other...> GetMultiCompTupleIter();

		// Lambda-Based
		//-----------------------------------------------------------------------------

		// Executes the provided lambda over the supplied component set
		//
		// Example:
		//   db.ForEach([](Position* pos, Velocity* vel) {
		//     DoSomething(pos, vel);
		//   });
		template <typename Func, typename... pComps>
		void ForEach(Func&& callback);

		template <typename pComp, typename... pComps>
		void ForEach(std::function<void(pComp, pComps...)>&& callback);

		//-----------------------------------------------------------------------------
		// Entities
		//-----------------------------------------------------------------------------

		// Creates a new empty entity and returns its ID
		//
		// Asserts:
		//   Didn't reach the max entity count limit
		EntityID CreateEntity();

		// Deletes the given entity
		//
		// Asserts:
		//   Entity exists
		void DeleteEntity(EntityID entity);

		//-----------------------------------------------------------------------------
		// Entity Components
		//-----------------------------------------------------------------------------

		// Type-less
		//-----------------------------------------------------------------------------

		// Adds the given component type to an entity.
		// If pComponentData != nullptr, its data is used to initialize the component
		//
		// Asserts:
		//	 Entity Exists
		//   Component Type exists
		void AddComponent(EntityID entityID, ComponentTypeID componentID, void* pComponentData);

		// Removes the given component type from an entity
		//
		// Asserts:
		//	 Entity Exists
		//   Component Type exists
		void RemoveComponent(EntityID entityID, ComponentTypeID componentID);

		// Returns a pointer to an entity's component data of the given type
		//
		// Asserts:
		//	 Entity Exists
		//   Component Type exists
		//   Entity has component
		void* GetComponent(EntityID entity, ComponentTypeID componentID);

		// Checks if an entity has the associated component type
		//
		// Asserts:
		//	 Entity Exists
		//   Component Type exists
		bool HasComponent(EntityID entity, ComponentTypeID componentID);

		// Template
		//-----------------------------------------------------------------------------

		// TODO: Add multiple components in one call to reduce overhead in archetype changes
		// void AddComponents(EntityID entity, Vector<ComponentID>, void** pComponentData);
		// void RemoveComponents(EntityID entity, Vector<ComponentID>);

		// Adds to an entity a T component, calling its default constructor
		template <typename T>
			requires std::is_default_constructible_v<T>
		void AddComponent(EntityID entity);

		// Adds to an entity a T component, constructing it with the supplied arguments
		template <typename T, typename... ConstructorArgs>
			requires (std::is_constructible_v<T, ConstructorArgs...> && sizeof...(ConstructorArgs) >= 1)
		void AddComponent(EntityID entity, ConstructorArgs... args);

		// Adds to an entity a T component, creating it from the supplied T component values
		template <typename T>
		void AddComponent(EntityID entity, T component);

		// Removes a T component from the given entity
		// Its only a typed extension of RemoveComponent(...)
		template <typename T>
		void RemoveComponent(EntityID entity);

		// Returns a pointer to an entity's T component
		// Its only a typed extension of GetComponent(...)
		template <typename T>
		T* GetComponent(EntityID entity);

		// Checks if an entity has a T component
		// Its only a typed extension of HasComponent(...)
		template <typename T>
		bool HasComponent(EntityID entity);

		//-----------------------------------------------------------------------------
		// Singleton Components
		//-----------------------------------------------------------------------------

		// Adds an unique singleton component of the type given by the component ID
		// Utilizes the pComponentData pointer memory to initialize the singleton component
		//
		// Asserts:
		//   pComponentData is not null
		//   Component doesn't already exist as a singleton component
		void AddSingletonComponent(ComponentTypeID componentID, void* pComponentData);

		// Removes the singleton component associated to the given component ID
		//
		// Asserts:
		//   Component exists as a singleton component
		void RemoveSingletonComponent(ComponentTypeID componentID);

		// Returns a pointer to a singleton component with the given component ID
		//
		// Asserts:
		//   Component exists as a singleton component
		void* GetSingletonComponent(ComponentTypeID componentID);

		// Adds a unique singleton component of type T using its default constructor
		template <typename T>
			requires std::is_default_constructible_v<T>
		void AddSingletonComponent();

		// Adds a unique singleton component of type T using the supplied data
		template <typename T>
		void AddSingletonComponent(T component);

		// Returns a casted pointer to the singleton component of type T
		//
		// Asserts:
		//   Component exists as a singleton component
		template <typename T>
		T* GetSingletonComponent();

		// Removes the singleton component of type T
		//
		// Asserts:
		//   Component exists as a singleton component
		template <typename T>
		void RemoveSingletonComponent();

		//-----------------------------------------------------------------------------
		// Queries
		//-----------------------------------------------------------------------------

		void QueryArchetypes(QueryInfo const& queryInfo);

		void QueryArchetypesSingleComponent(ComponentTypeID compTypeID, Vector<ArchetypeColumnPair>& accessData, u64& totalEntitiesCount);

		//-----------------------------------------------------------------------------
		// Debugging
		//-----------------------------------------------------------------------------

		friend class EntityDatabaseDebugger;

		// Returns a debugger associated with the entity database
		EntityDatabaseDebugger GetDebugger();

		// Prints a general overview of the database
		void PrintAdminState();

		// Prints data associated with an entity
		void PrintEntityState(EntityID entityID);

	private:
		// Auxiliary
		//-----------------------------------------------------------------------------

		ComponentSetID CalculateComponentSetID(Vector<ComponentTypeID> const& componentSet);

		// Returns the component column of component in a given archetype
		ArchetypeComponentColumn GetComponentColumnInArchetype(ComponentTypeID component, ArchetypeID archetypeID) const;

		void MoveComponentDataFromToArch(Archetype*       pOldArchetype, u64 oldArchetypeRow,
		                                 Archetype*       pNewArchetype, u64 newArchetypeRow,
		                                 ComponentTypeID& compID);

		// Archetypes
		//-----------------------------------------------------------------------------

		// Create an archetype for the given component set
		void        CreateArchetype(Vector<ComponentTypeID> const& componentSet);
		//void        DeleteArchetype(Vector<ComponentTypeID> const& componentSet);
		inline bool ArchetypeExists(Vector<ComponentTypeID> const& componentSet);

	private:
		friend class ComponentIter;
		friend class MultiComponentIter;

	private:
		Vector<EntityID>        m_Entities;       // All the active entities in the world
		Vector<ComponentTypeID> m_ComponentTypes; // All of the component types

		Vector<Archetype> m_Archetypes; // All existing archetypes

		// Data Relationships
		//-----------------------------------------------------------------------------

		// Relationship between the ArchetypeID and its assigned Archetype
		Map<ArchetypeID, Archetype*> m_IDToArchetype;

		// Used to get the Archetype and the entity row
		Map<EntityID, EntityRecord> m_EntityToRecord;

		// Returns all the archetypes and columns that contain the component
		Map<ComponentTypeID, Map<ArchetypeID, ArchetypeComponentColumn>> m_ComponentToArchetypes;

		Map<ComponentSetID, Archetype*> m_ComponentSetToArchetype;

		Map<ComponentTypeID, SingletonComponentRecord> m_IDToSingletonComponents;

		Map<ComponentTypeID, ComponentTypeData> m_ComponentTypeData; // RTTI for components

		//-----------------------------------------------------------------------------

		u64 m_MaxNumEntities = 0;

		// ID Tracking
		//-----------------------------------------------------------------------------

		EntityID        m_NextEntityID{0};
		ComponentTypeID m_LastComponentID = 0;
		ArchetypeID     m_LastArchetypeID = 0;
	};
}

// ECS Debugging Tools
//-----------------------------------------------------------------------------

namespace CKE {
	struct EntityDatabaseStateSnapshot
	{
		u64 m_NumEntities;
		u64 m_NumComponentTypes;
		u64 m_NumArchetypes;
	};

	class EntityDatabaseDebugger
	{
	public:
		EntityDatabaseDebugger() = default;
		EntityDatabaseDebugger(EntityDatabase& db) : m_Db(&db) { }

		EntityDatabaseStateSnapshot GetStateSnapshot() const;

		void PrintGeneralState() const;

	private:
		EntityDatabase* m_Db;
	};
}

// Template and Inline implementations
//-----------------------------------------------------------------------------

namespace CKE {
	bool EntityDatabase::ArchetypeExists(Vector<ComponentTypeID> const& componentSet) {
		ComponentSetID setID = CalculateComponentSetID(componentSet);
		return m_ComponentSetToArchetype.contains(setID);
	}

	template <typename T>
	void EntityDatabase::AssignComponentIDToStaticType(ComponentTypeID id) {
		ComponentStaticTypeID<T>::s_CompID = id;
	}

	// Template implementation of registering a component that
	// links the component ID with the C++ data type
	template <typename T>
		requires CanBeComponent<T>
	ComponentTypeID EntityDatabase::RegisterComponent() {
		// Check that the component has not been registered already
		// If it has not been registered, its type associated component id will
		// be 0
		// CKE_ASSERT(ComponentStaticTypeID<T>::s_CompID == 0);
		u64 sizeInBytes;

		// sizeof(T) will return 1 even if T is an empty datatype
		// so we need to check its emptiness with std::is_empty
		if constexpr (std::is_empty_v<T>) { sizeInBytes = 0; }
		else { sizeInBytes = sizeof(T); }

		// TODO: Replace typeid with custom RTTI
		ComponentTypeID compID = RegisterComponent(typeid(T).name(), sizeInBytes, alignof(T));
		ComponentStaticTypeID<T>::s_CompID = compID;

		return compID;
	}

	template <typename T>
	TComponentIterator<T> EntityDatabase::GetSingleCompIter() {
		TComponentIterator<T> compIterator(this);
		return compIterator;
	}

	template <typename T, typename... Other>
	TMultiComponentIter<T, Other...> EntityDatabase::GetMultiCompTupleIter() {
		TMultiComponentIter<T, Other...> iter(this);
		return iter;
	}

	template <typename Func, typename... pComps>
	void EntityDatabase::ForEach(Func&& callback) {
		ForEach(std::function{std::forward<Func>(callback)});
	}

	template <typename pComp, typename... pComps>
	inline void EntityDatabase::ForEach(std::function<void(pComp, pComps...)>&& callback) {
		for (std::tuple<pComp, pComps...>& compTuple : GetMultiCompTupleIter<
			     std::remove_pointer_t<pComp>, std::remove_pointer_t<pComps>...>()) {
			std::apply(callback, compTuple);
		}
	}

	template <size_t I, typename... Ts>
	constexpr void PopulateVectorWithComponentIDs(Vector<ComponentTypeID>& vec) {
		if constexpr (I == sizeof...(Ts)) { return; }
		else {
			vec.emplace_back(ComponentStaticTypeID<std::tuple_element_t<I, std::tuple<Ts...>>>::s_CompID);
			PopulateVectorWithComponentIDs<I + 1, Ts...>(vec);
		}
	}

	template <typename T, typename... Other>
	MultiComponentIter EntityDatabase::GetMultiCompIter() {
		Vector<ComponentTypeID> componentIDs;
		PopulateVectorWithComponentIDs<0, T, Other...>(componentIDs);
		return MultiComponentIter(this, componentIDs);
	}

	template <typename T>
	void EntityDatabase::AddSingletonComponent(T component) {
		AddSingletonComponent(ComponentStaticTypeID<T>::s_CompID, &component);
	}

	template <typename T>
		requires std::is_default_constructible_v<T>
	void EntityDatabase::AddSingletonComponent() {
		T comp{};
		AddSingletonComponent(comp);
	}

	template <typename T>
	T* EntityDatabase::GetSingletonComponent() {
		CKE_ASSERT(ComponentStaticTypeID<T>::s_CompID != 0);
		return reinterpret_cast<T*>(GetSingletonComponent(ComponentStaticTypeID<T>::s_CompID));
	}

	template <typename T>
	void EntityDatabase::RemoveSingletonComponent() {
		RemoveSingletonComponent(ComponentStaticTypeID<T>::s_CompID);
	}

	template <typename T>
		requires std::is_default_constructible_v<T>
	void EntityDatabase::AddComponent(EntityID entity) {
		T comp{};
		AddComponent(entity, comp);
	}

	template <typename T,
	          typename... ConstructorArgs>
		requires (std::is_constructible_v<T, ConstructorArgs...> && sizeof...(ConstructorArgs) >= 1)
	void EntityDatabase::AddComponent(EntityID entity, ConstructorArgs... args) {
		T comp{args...};
		AddComponent(entity, ComponentStaticTypeID<T>::s_CompID, &comp);
	}

	template <typename T>
	void EntityDatabase::AddComponent(EntityID entity, T component) {
		AddComponent(entity, ComponentStaticTypeID<T>::s_CompID, &component);
	}

	template <typename T>
	void EntityDatabase::RemoveComponent(EntityID entity) {
		RemoveComponent(entity, ComponentStaticTypeID<T>::s_CompID);
	}

	template <typename T>
	T* EntityDatabase::GetComponent(EntityID entity) {
		return reinterpret_cast<T*>(GetComponent(entity, ComponentStaticTypeID<T>::s_CompID));
	}

	template <typename T>
	bool EntityDatabase::HasComponent(EntityID entity) {
		return HasComponent(entity, ComponentStaticTypeID<T>::s_CompID);
	}
}
