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
		// Create an UNINITIALIZED allocator. MUST be initialized before using it.
		PoolAllocator() = default;

		// Constructs an initialized pool
		PoolAllocator(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		void Initialize(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		//-----------------------------------------------------------------------------

		// Returns a pointer to an available block of memory.
		[[nodiscard]] void* Alloc();

		// Templated version of AllocChunk that casts the returned pointer to the requested type.
		template <typename T>
		[[nodiscard]] T* Alloc();

		//-----------------------------------------------------------------------------

		// Returns to the pool a previously requested chunk
		void Free(void* pChunkPtr);

		// Releases all of the allocated objects
		void Reset();

		void*      GetUnderlyingMemoryBlock() { return m_pBuffer; }
		inline u64 GetTotalSize() const { return m_TotalSize; }
		inline u64 GetBlockSize() const { return m_BlockSize; }
		inline u64 GetFreeBlocksCount() const { return m_BlockOffsetsFree.size(); }

	private:
		// TODO: Replace the heavy Set block tracking implementation with a more compact & efficient one
		char*    m_pBuffer = nullptr;   // Ptr to the memory buffer managed by this allocator
		u64      m_TotalSize = 0;       // Total size of the memory buffer managed by the allocator
		u64      m_BlockSize = 0;       // Size of each block/chunk
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
			: PoolAllocator{CKE::Alloc(sizeof(T) * maxElements, alignof(T)), sizeof(T), maxElements * sizeof(T)} {}

		template <typename... ConstructorArgs>
		[[nodiscard]] T* New(ConstructorArgs&&... args) {
			void* pMemory = this->Alloc<T>();
			void* a = new(pMemory) T{std::forward<ConstructorArgs>(args)...};
			CKE_ASSERT(pMemory == a);
			return (T*)a;
		}

		void Delete(T* pAddress) {
			pAddress->~T();
			Free(pAddress);
		}
	};
}

// Template implementations
//-----------------------------------------------------------------------------

namespace CKE {
	template <typename T>
	T* PoolAllocator::Alloc() {
		CKE_ASSERT(sizeof(T) <= m_BlockSize);
		return static_cast<T*>(Alloc());
	}
}
