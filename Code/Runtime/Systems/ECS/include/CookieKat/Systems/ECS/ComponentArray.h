#pragma once

#include "CookieKat/Core/Memory/Memory.h"
#include "CookieKat/Core/Platform/Asserts.h"

#include "IDs.h"

namespace CKE {
	// Works as a packed unordered array
	class ComponentArray
	{
	public:
		ComponentArray(ComponentTypeID componentID, u64 numMaxElements, u64 compSizeInBytes, u32 compAlignment);
		~ComponentArray();
		ComponentArray(const ComponentArray& other) = delete;
		ComponentArray(ComponentArray&& other) noexcept;
		ComponentArray& operator=(const ComponentArray& other) = delete;
		ComponentArray& operator=(ComponentArray&& other) noexcept;

		//-----------------------------------------------------------------------------

		// Adds a new component to the end and returns its index
		inline u64 AppendComponentToEnd();

		// Returns a pointer to the component at the given position
		inline u8* GetCompAtIndex(u64 index);

		// Removes a component and returns the index of the element that has been moved
		// to fill the hole left when packing the array
		inline u64 RemoveCompAt(u64 index);

		// Returns the size in bytes of the component type that is stored in this array
		inline u64 GetCompSizeInBytes() const;

	private:
		u8*             m_pData;
		u64             m_NumComponents;
		u64             m_NumMaxComponents;
		u64             m_ComponentSize;
		u32             m_ComponentAlignment;
		ComponentTypeID m_ComponentTypeID;
	};
}

// Template implementation
//-----------------------------------------------------------------------------

namespace CKE {
	inline ComponentArray::ComponentArray(ComponentTypeID componentID, u64 numMaxElements, u64 compSizeInBytes, u32 compAlignment) {
		m_ComponentTypeID = componentID;
		m_NumMaxComponents = numMaxElements;
		m_NumComponents = 0;
		m_ComponentSize = compSizeInBytes;
		m_ComponentAlignment = compAlignment;
		m_pData = static_cast<u8*>(CKE::Alloc(compSizeInBytes * numMaxElements, m_ComponentAlignment));
	}

	inline ComponentArray::~ComponentArray() {
		if (m_pData != nullptr) {
			CKE::Free(m_pData);
		}
	}

	inline ComponentArray::ComponentArray(ComponentArray&& other) noexcept {
		m_ComponentTypeID = other.m_ComponentTypeID;
		m_NumMaxComponents = other.m_NumMaxComponents;
		m_NumComponents = other.m_NumComponents;
		m_ComponentSize = other.m_ComponentSize;
		m_ComponentAlignment = other.m_ComponentAlignment;
		CKE_ASSERT(other.m_pData != nullptr);
		m_pData = other.m_pData;
		other.m_pData = nullptr;
	}

	inline ComponentArray& ComponentArray::operator=(ComponentArray&& other) noexcept {
		m_ComponentTypeID = other.m_ComponentTypeID;
		m_NumMaxComponents = other.m_NumMaxComponents;
		m_NumComponents = other.m_NumComponents;
		m_ComponentSize = other.m_ComponentSize;
		m_ComponentAlignment = other.m_ComponentAlignment;
		CKE_ASSERT(other.m_pData != nullptr);
		m_pData = other.m_pData;
		other.m_pData = nullptr;
		return *this;
	}

	u64 ComponentArray::AppendComponentToEnd() {
		CKE_ASSERT(m_NumComponents < m_NumMaxComponents); // Ran out of space
		u64 rowIndex = m_NumComponents;
		m_NumComponents++;
		return rowIndex;
	}

	u8* ComponentArray::GetCompAtIndex(u64 index) {
		CKE_ASSERT(index < m_NumComponents); // Trying to access a component that doesn't exist
		return m_ComponentSize * index + m_pData;
	}

	u64 ComponentArray::RemoveCompAt(u64 index) {
		CKE_ASSERT(index < m_NumComponents);
		u8* elementToRemove = GetCompAtIndex(index);
		u64 lastIndex = m_NumComponents != 0 ? m_NumComponents - 1 : 0;
		u8* lastElement = GetCompAtIndex(lastIndex);

		memcpy(elementToRemove, lastElement, m_ComponentSize);
		m_NumComponents--;
		return lastIndex;
	}

	u64 ComponentArray::GetCompSizeInBytes() const { return m_ComponentSize; }
}
