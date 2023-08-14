#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	// Subdivides a memory block into fixed - size chunks and provides allocation and deallocation functionality.
	class PoolAllocator
	{
	public:
		// Constructs an uninitialized pool
		PoolAllocator() = default;
		// Constructs an initialized pool
		PoolAllocator(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		void Initialize(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes);

		//-----------------------------------------------------------------------------

		// Returns a pointer to an available block of memory.
		[[nodiscard]] void* AllocChunk();

		// Templated version of AllocChunk that casts the returned pointer to the requested type.
		template <typename T>
		[[nodiscard]] T* AllocChunk();

		//-----------------------------------------------------------------------------

		// Returns to the pool a previously requested chunk
		void FreeChunk(void* pChunkPtr);

		void* GetUnderlyingMemoryBlock() { return m_pBuffer; }

	private:
		// TODO: Replace the heavy Set block tracking implementation with a more compact & efficient one
		char*    m_pBuffer = nullptr;   // Ptr to the memory buffer managed by this allocator
		u64      m_TotalSize = 0;       // Total size of the memory buffer managed by the allocator
		u64      m_BlockSize = 0;       // Size of each block/chunk
		Set<u64> m_BlockOffsetsFree{};  // Array of the start offset of every available block
		Set<u64> m_BlockOffsetsInUse{}; // Array of the start offset of every in-use block
	};

	// TODO: Should we extract the templated functions to a derived class like this?
	template <typename T>
	class TPoolAllocator : public PoolAllocator
	{
	public:
		TPoolAllocator() : PoolAllocator{} {}
		TPoolAllocator(void* pMemoryBlock, u32 maxElements) : PoolAllocator{pMemoryBlock, sizeof(T), maxElements * sizeof(T)} {}

		[[nodiscard]] T* Alloc() {
			return this->AllocChunk<T>();
		}

		template <typename... ConstructorArgs>
		[[nodiscard]] T* New(ConstructorArgs&&... args) {
			void* pMemory = this->AllocChunk<T>();
			void* a = new(pMemory) T{ std::forward<ConstructorArgs>(args)... };
			CKE_ASSERT(pMemory == a);
			return (T*)a;
		}

		void Delete(T* pAddress) {
			pAddress->~T();
			FreeChunk(pAddress);
		}
	};

	template <typename T>
	class TObjectPool
	{
	public:
		TObjectPool(u32 maxElements);

		template <typename... ConstructorArgs>
		[[nodiscard]] T* New(ConstructorArgs&&... args) {
			T* pObject = m_MemoryPool.New(args);
			return pObject;
		}

		void Delete(T* pObject) {
			m_MemoryPool.Delete(pObject);
		}

	private:
		TPoolAllocator<T> m_MemoryPool;
	};
}

// Template implementations
//-----------------------------------------------------------------------------

namespace CKE {
	template <typename T>
	T* PoolAllocator::AllocChunk() {
		CKE_ASSERT(sizeof(T) <= m_BlockSize);
		return static_cast<T*>(AllocChunk());
	}

	inline PoolAllocator::PoolAllocator(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes) {
		Initialize(pMemoryBlock, chunkSizeInBytes, totalSizeInBytes);
	}

	inline void PoolAllocator::Initialize(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes) {
		m_pBuffer = (char*)pMemoryBlock;
		m_TotalSize = totalSizeInBytes;
		m_BlockSize = chunkSizeInBytes;

		CKE_ASSERT(totalSizeInBytes % chunkSizeInBytes == 0);
		u64 const chunkCount = totalSizeInBytes / chunkSizeInBytes;
		for (u64 i = 0; i < chunkCount; ++i) {
			u64 blockOffset = i * chunkSizeInBytes;
			m_BlockOffsetsFree.insert(blockOffset);
		}
	}

	inline void* PoolAllocator::AllocChunk() {
		CKE_ASSERT(!m_BlockOffsetsFree.empty());

		u64   blockOffset = *m_BlockOffsetsFree.begin();
		char* pBlock = m_pBuffer + blockOffset;

		m_BlockOffsetsFree.erase(blockOffset);
		m_BlockOffsetsInUse.insert(blockOffset);

		return pBlock;
	}

	inline void PoolAllocator::FreeChunk(void* pChunkPtr) {
		memset(pChunkPtr, 0, m_BlockSize);

		u64 blockOffset = static_cast<char*>(pChunkPtr) - m_pBuffer;
		m_BlockOffsetsInUse.erase(blockOffset);
		m_BlockOffsetsFree.insert(blockOffset);
	}
}
