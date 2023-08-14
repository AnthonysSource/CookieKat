#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	// Allocates memory linearly from a given buffer and
	// allows releasing the last allocated block
	class StackAllocator
	{
	public:
		// Initialize the allocator with an existing memory block
		// that it will manage and its size
		StackAllocator(void* pMemoryBlock, u64 sizeInBytes);

		//-----------------------------------------------------------------------------

		// Allocates a memory block of the given size
		void* Alloc(u64 sizeInBytes);

		// Allocates a memory block the size of T
		template <typename T>
		T* Alloc();

		//-----------------------------------------------------------------------------

		// Frees the last allocated memory block
		void FreeLast();

	private:
		// TODO: Replace Stack with a container that doesn't use the heap
		u8*        m_pBuffer;           // Ptr to the memory block managed by the allocator
		u64        m_SizeInBytes;       // Total size of the buffer
		Stack<u64> m_AllocationOffsets; // Offset of the previously allocated blocks
	};
}

// Template implementations
//-----------------------------------------------------------------------------

namespace CKE {
	template <typename T>
	T* StackAllocator::Alloc() {
		return static_cast<T*>(Alloc(sizeof(T)));
	}

	inline StackAllocator::StackAllocator(void* pMemoryBlock, u64 sizeInBytes) {
		m_pBuffer = static_cast<u8*>(pMemoryBlock);
		m_SizeInBytes = sizeInBytes;
		m_AllocationOffsets.push(0);
	}

	inline void* StackAllocator::Alloc(u64 sizeInBytes) {
		CKE_ASSERT(m_AllocationOffsets.top() + sizeInBytes <= m_SizeInBytes);
		void*     pReturnMemory = m_pBuffer + m_AllocationOffsets.top();
		u64 const newOffset = m_AllocationOffsets.top() + sizeInBytes;
		m_AllocationOffsets.push(newOffset);
		return pReturnMemory;
	}

	inline void StackAllocator::FreeLast() {
		m_AllocationOffsets.pop();
	}
}
