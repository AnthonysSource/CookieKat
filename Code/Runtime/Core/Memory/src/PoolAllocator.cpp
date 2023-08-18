#include "PoolAllocator.h"

namespace CKE {
	PoolAllocator::PoolAllocator(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes) {
		Initialize(pMemoryBlock, chunkSizeInBytes, totalSizeInBytes);
	}

	void PoolAllocator::Initialize(void* pMemoryBlock, u64 chunkSizeInBytes, u64 totalSizeInBytes) {
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

	void* PoolAllocator::Alloc() {
		CKE_ASSERT(!m_BlockOffsetsFree.empty());

		u64   blockOffset = *m_BlockOffsetsFree.begin();
		char* pBlock = m_pBuffer + blockOffset;

		m_BlockOffsetsFree.erase(blockOffset);
		m_BlockOffsetsInUse.insert(blockOffset);

		return pBlock;
	}

	void PoolAllocator::Free(void* pChunkPtr) {
		memset(pChunkPtr, 0, m_BlockSize);

		u64 blockOffset = static_cast<char*>(pChunkPtr) - m_pBuffer;
		m_BlockOffsetsInUse.erase(blockOffset);
		m_BlockOffsetsFree.insert(blockOffset);
	}

	void PoolAllocator::Reset() {
		for (unsigned long long inUse : m_BlockOffsetsInUse) {
			m_BlockOffsetsFree.insert(inUse);
		}
		m_BlockOffsetsInUse.clear();
	}
}
