#pragma once

#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	// Forward Declarations
	class Archetype;
}

namespace CKE {
	// General Purpose ID Types
	//-----------------------------------------------------------------------------

	template <typename T>
	class StronglyTypedID
	{
	public:
		StronglyTypedID() = default;
		explicit StronglyTypedID(u32 value) : m_Value{value} {}

		inline static StronglyTypedID Invalid() { return StronglyTypedID{}; } // Returns an invalid entity ID

		inline u32  GetValue() const { return m_Value; } // Returns the underlying value of the ID
		inline bool IsValid() const { return m_Value != 0; }

		inline StronglyTypedID& operator=(StronglyTypedID const& other) {
			m_Value = other.m_Value;
			return *this;
		}

		inline bool operator==(StronglyTypedID const& other) const { return m_Value == other.m_Value; }
		inline bool operator!=(StronglyTypedID const& other) const { return !(*this == other); }
		inline bool operator<(StronglyTypedID const& other) const { return m_Value < other.m_Value; }

	public:
		u32 m_Value = 0;
	};

	template <typename Tag, typename T>
	struct StronglyTypedValue
	{
		T m_Value{};
	};

	// Identifiers for many of the system structures
	//-----------------------------------------------------------------------------

	using EntityID = StronglyTypedID<struct EntityID_Tag>;
	using ComponentTypeID = StronglyTypedID<struct ComponentTypeID_Tag>;
	using ArchetypeID = StronglyTypedID<struct ArchetypeID_Tag>;
	using ComponentSetID = StronglyTypedID<struct ComponentSetID_Tag>;

	using ArchetypeComponentColumn = u32;

	// Todo: Improve the interface for this structure
	class ComponentSet2
	{
	public:
		void                                  AddComponent(ComponentTypeID componentTypeID);
		void                                  RemoveComponent(ComponentTypeID componentTypeID);
		void                                  MergeWith(ComponentSet2 const& otherSet);
		inline Vector<ComponentTypeID> const& GetVector() const { return m_Components; }

	private:
		Vector<ComponentTypeID> m_Components;
	};

	using ComponentSet = Vector<ComponentTypeID>;

	//-----------------------------------------------------------------------------

	// Explicit data assigned to an entity
	struct EntityRecord
	{
		Archetype* m_pArchetype;         // Ptr to the archetype of the entity
		u64        m_EntityArchetypeRow; // Index to the archetype table row where the entity components are located
	};

	// Type information of a component
	struct ComponentTypeData
	{
		String m_Name;
		u64    m_SizeInBytes;
		u32    m_Alignment;
	};

	// Data assigned to a singleton/global component
	struct SingletonComponentRecord
	{
		void* m_pComponentData;
		u64   m_SizeInBytes;
	};

	// Static global ID created for a given type
	// WARNING: Will crash if the component system is used across DLLs
	// TODO: Maybe use a component name hash to create its ID?
	template <typename T>
	struct ComponentStaticTypeID
	{
		inline static ComponentTypeID GetTypeID() { return s_CompID; }

		inline static ComponentTypeID s_CompID = ComponentTypeID::Invalid();
	};
}

namespace std {
	template <typename T>
	struct hash<CKE::StronglyTypedID<T>>
	{
		inline std::size_t operator()(const CKE::StronglyTypedID<T>& id) const noexcept {
			return id.GetValue();
		}
	};
}
