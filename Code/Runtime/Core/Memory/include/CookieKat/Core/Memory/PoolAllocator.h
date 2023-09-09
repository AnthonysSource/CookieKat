#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Memory/Memory.h"

namespace CKE {
	// Subdivides a memory block into fixed - size chunks and provides allocation and deallocation functionality.
	class PoolAllocator
	{
	public:
		// Create an UNINITIALIZED allocator. MUST be initialized calling PoolAllocator:Initialize(...) before using.
		PoolAllocator() = default;

		// Construct and initialized the allocator
		PoolAllocator(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		void Initialize(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		//-----------------------------------------------------------------------------

		// Returns a pointer to an available block of memory.
		[[nodiscard]] void* Alloc();

		// Template version of AllocChunk that casts the returned pointer to the requested type.
		template <typename T>
		[[nodiscard]] T* Alloc();

		//-----------------------------------------------------------------------------

		// Returns to the pool a previously requested chunk
		void Free(void* pChunkPtr);

		// Releases all of the allocated objects
		void Reset();

		// Returns a pointer to the begining of the memory buffer used by the allocator
		//
		// WARNING: Only manipulate its contents if you know what you are doing
		inline void* GetUnderlyingMemoryBuffer() { return m_pBuffer; }

		// Returns the total size of the memory buffer used by the pool
		inline u64   GetTotalSize() const { return m_TotalSize; }

		// Returns the size of a single block in the pool
		inline u64   GetBlockSize() const { return m_BlockSize; }

		// Returns the available count of blocks
		inline u64   GetFreeBlocksCount() const { return m_BlockOffsetsFree.size(); }

		// Returns the in-use count of blocks
		inline u64   GetUsedBlocksCount() const { return m_BlockOffsetsInUse.size(); }

	private:
		u8*      m_pBuffer = nullptr;   // Ptr to the memory buffer managed by this allocator
		u64      m_TotalSize = 0;       // Total size of the memory buffer managed by the allocator
		u64      m_BlockSize = 0;       // Size of each block/chunk

		// TODO: Replace the heavy Set block tracking implementation with a more compact & efficient one
		Set<u64> m_BlockOffsetsFree{};  // Array of the start offset of every available block
		Set<u64> m_BlockOffsetsInUse{}; // Array of the start offset of every in-use block
	};

	template <typename T>
	class TPoolAllocator : public PoolAllocator
	{
	public:
		// Create an UNINITIALIZED allocator. MUST be initialized before using it.
		TPoolAllocator() : PoolAllocator{} {}

		TPoolAllocator(void* pMemoryBlock, u32 maxElements)
			: PoolAllocator{pMemoryBlock, sizeof(T), maxElements * sizeof(T)} {}

		// Create a pool allocator which uses a "default allocated" memory block
		// that can allocate at least the given count of type T elements.
		TPoolAllocator(u32 maxElements)
			: PoolAllocator{Memory::Alloc(sizeof(T) * maxElements, alignof(T)), sizeof(T), maxElements * sizeof(T)} {}

		template <typename... ConstructorArgs>
		[[nodiscard]] T* New(ConstructorArgs&&... args);

		void Delete(T* pAddress);
	};
}


//=======================================================================
//						Inline Definitions
//=======================================================================


namespace CKE {
	template <typename T>
	T* PoolAllocator::Alloc() {
		CKE_ASSERT(sizeof(T) <= m_BlockSize);
		return static_cast<T*>(Alloc());
	}

	template <typename T>
	template <typename... ConstructorArgs>
	T* TPoolAllocator<T>::New(ConstructorArgs&&... args) {
		void* pMemory = this->Alloc<T>();
		T*    pObject = Memory::NewInPlace<T>(pMemory, std::forward<ConstructorArgs>(args)...);
		return pObject;
	}

	template <typename T>
	void TPoolAllocator<T>::Delete(T* pAddress) {
		pAddress->~T();
		Free(pAddress);
	}
}
