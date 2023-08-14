#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Memory/Memory.h"

namespace CKE {
	// Handles the provided block of memory using a linear allocation scheme
	//
	// Example
	//     usize const pMemBlockSize = sizeof(u32)*10; // Buffer of 40 Bytes
	//     void* pMemBlock = CKE::Alloc(pMemBlockSize);
	//     LinearAllocator linearAlloc{pMemBlock, pMemBlockSize};
	//     u32* a = linearAlloc.Alloc<u32>();     // Allocated 4 of 40 bytes
	//     f64* a = linearAlloc.Alloc<f64>();     // Allocated 12 of 40 bytes
	//     linearAlloc.FreeAll();                 // Reset entire buffer
	class LinearAllocator
	{
	public:
		LinearAllocator(void* pMemoryBlock, u64 sizeInBytes);

		//-----------------------------------------------------------------------------

		[[nodiscard]] inline void* Alloc(u64 sizeInBytes);

		// Allocates a block of memory the size of T
		// (Doesn't call any constructor)
		template <typename T>
		[[nodiscard]] inline T* Alloc();

		//-----------------------------------------------------------------------------

		void FreeAll();

	private:
		u8* m_pBuffer;       // Ptr to the memory block managed by the allocator
		u64 m_SizeInBytes;   // Total size of the buffer
		u64 m_OffsetInBytes; // Current offset
	};
}


namespace CKE {
	template <typename T>
	T* LinearAllocator::Alloc() {
		return static_cast<T*>(Alloc(sizeof(T)));
	}

	inline LinearAllocator::LinearAllocator(void* pMemoryBlock, u64 sizeInBytes) {
		m_pBuffer = static_cast<u8*>(pMemoryBlock);
		m_SizeInBytes = sizeInBytes;
		m_OffsetInBytes = 0;
	}

	inline void* LinearAllocator::Alloc(u64 sizeInBytes) {
		CKE_ASSERT(m_OffsetInBytes + sizeInBytes <= m_SizeInBytes);
		void* pReturnMemory = m_pBuffer + m_OffsetInBytes;
		m_OffsetInBytes += sizeInBytes;
		return pReturnMemory;
	}

	inline void LinearAllocator::FreeAll() {
		m_OffsetInBytes = 0;
	}
}
