#pragma once

#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	// Forward Declarations
	class Archetype;
}

namespace CKE {
	// Identifiers for many of the system structures
	//-----------------------------------------------------------------------------

	// Strongly type integer to represent an Entity ID
	class EntityID
	{
	public:
		inline explicit EntityID() : m_Value(0) {}              // Constructs an invalid entity ID
		inline explicit EntityID(u32 value) : m_Value(value) {} // Constructs an entity ID with the given underlying value

		inline static EntityID Invalid() { return EntityID{}; } // Returns an invalid entity ID
		inline bool            IsValid() const { return m_Value != 0; }
		inline u32             GetValue() const { return m_Value; } // Returns the underlying value of the ID

		inline bool operator==(const EntityID& other) const { return m_Value == other.m_Value; }
		inline bool operator!=(const EntityID& other) const { return !(*this == other); }

	private:
		u32 m_Value;
	};

	// TODO: Implement these IDs as strongly-typed
	using ComponentTypeID = u64;
	using ArchetypeID = u64;
	using ComponentSetID = u64;
	using ArchetypeComponentColumn = u64;

	// Todo: Improve the interface for this structure
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
		inline static ComponentTypeID s_CompID = 0;
	};
}

template <>
struct std::hash<CKE::EntityID>
{
	inline std::size_t operator()(const CKE::EntityID& k) const noexcept {
		return k.GetValue();
	}
};
