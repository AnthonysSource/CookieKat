#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/Asserts.h"

#include "CookieKat/Core/Memory/Memory.h"
#include "CookieKat/Core/Memory/LinearAllocator.h"
#include "CookieKat/Core/Memory/StackAllocator.h"
#include "CookieKat/Core/Memory/PoolAllocator.h"

#include <gtest/gtest.h>

using namespace CKE;

//-----------------------------------------------------------------------------
// Auxiliary Data Types
//-----------------------------------------------------------------------------

struct NewDeleteDataType
{
	u64 a = 2;
	i32 b = 6;
};

//-----------------------------------------------------------------------------
// Base Memory Operations
//-----------------------------------------------------------------------------

TEST(Memory, AllocFree) {
	u64* pMemory = (u64*)Memory::Alloc(sizeof(u64) * 100, alignof(u64));

	for (u64 i = 0; i < 100; ++i) {
		pMemory[i] = i;
	}

	Memory::Free(pMemory);
	ASSERT_TRUE(true);
}

TEST(Memory, NewDelete) {
	NewDeleteDataType* pObject = Memory::New<NewDeleteDataType>();
	EXPECT_EQ(pObject->a, 2);
	EXPECT_EQ(pObject->b, 6);
	Memory::Free(pObject);
}

TEST(Memory, NewDeleteArray) {
	// Data type to check if the constructor and destructor
	// are called correctly on the NewArray operation
	// The constructor of this class populates its elements and receives a ptr to an array
	// and the destructor writes data to the ptr
	struct NewArrayElem
	{
		NewArrayElem(i32* p) {
			ptr = p;
			i = 1;
		}

		~NewArrayElem() { *ptr = i; }

		i32* ptr;
		i32  i;
	};

	usize constexpr elemCount = 17;

	i32  destructorArray[elemCount];
	auto allocatedArray = Memory::NewArray<NewArrayElem>(elemCount, &destructorArray[0]);

	for (usize i = 0; i < elemCount; ++i) {
		// Check constructor was called and that the alignment was correct
		EXPECT_EQ(allocatedArray[i].ptr, &destructorArray[0]);
		EXPECT_EQ(allocatedArray[i].i, 1);
		EXPECT_TRUE(Memory::IsAligned(&allocatedArray[i]));

		allocatedArray[i].ptr = &destructorArray[i];
		allocatedArray[i].i = i;
	}

	Memory::DeleteArray(allocatedArray);

	// Check destructor was called
	for (int i = 0; i < elemCount; ++i) {
		EXPECT_EQ(destructorArray[i], i);
	}
}

//-----------------------------------------------------------------------------
// Allocators
//-----------------------------------------------------------------------------


// Linear Allocator
//-----------------------------------------------------------------------------

TEST(Allocators, LinearAllocator_AllocReset) {
	constexpr u64   BLOCK_SIZE = 32;
	void*           pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{pMemoryBlock, BLOCK_SIZE};

	{
		i32* pI32 = linearAllocator.Alloc<i32>();
		u16* pu16 = linearAllocator.Alloc<u16>();
		i16* pI16 = linearAllocator.Alloc<i16>();
		i64* pI64 = linearAllocator.Alloc<i64>();

		*pI32 = -4;
		*pu16 = 2;
		*pI16 = -2;
		*pI64 = 16;

		EXPECT_EQ(*pI32, -4);
		EXPECT_EQ(*pu16, 2);
		EXPECT_EQ(*pI16, -2);
		EXPECT_EQ(*pI64, 16);
	}

	linearAllocator.Reset();

	{
		i32* pI32 = linearAllocator.Alloc<i32>();
		u16* pu16 = linearAllocator.Alloc<u16>();
		i16* pI16 = linearAllocator.Alloc<i16>();
		i64* pI64 = linearAllocator.Alloc<i64>();

		*pI32 = -64;
		*pu16 = 75;
		*pI16 = -4;
		*pI64 = 643;

		EXPECT_EQ(*pI32, -64);
		EXPECT_EQ(*pu16, 75);
		EXPECT_EQ(*pI16, -4);
		EXPECT_EQ(*pI64, 643);
	}

	linearAllocator.Reset();
	Memory::Free(pMemoryBlock);
}

TEST(Allocators, LinearAllocator_OverflowThrows) {
	constexpr u64   BLOCK_SIZE = 1;
	void*           pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{pMemoryBlock, BLOCK_SIZE};

#ifdef CKE_BUILDSYSTEM_ASSERTS_ENABLE
	EXPECT_DEATH({ i32 * pI32 = linearAllocator.Alloc<i32>(); }, "Assertion failed");
#endif

	linearAllocator.Reset();
}

TEST(Allocators, LinearAllocator_GetTotalSize) {
	constexpr u64   BLOCK_SIZE = 32;
	void* pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{ pMemoryBlock, BLOCK_SIZE };

	EXPECT_EQ(linearAllocator.GetTotalSizeInBytes(), 32);
}

TEST(Allocators, LinearAllocator_GetAllocatedSize) {
	constexpr u64   BLOCK_SIZE = 32;
	void* pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{ pMemoryBlock, BLOCK_SIZE };

	EXPECT_EQ(linearAllocator.GetAllocatedSizeInBytes(), 0);
	void* pA = linearAllocator.Alloc(4);
	EXPECT_EQ(linearAllocator.GetAllocatedSizeInBytes(), 4);
	void* pB = linearAllocator.Alloc(16);
	EXPECT_EQ(linearAllocator.GetAllocatedSizeInBytes(), 20);
}

// Pool Allocator
//-----------------------------------------------------------------------------

TEST(Allocators, PoolAllocator_AllocReset) {
	constexpr u64 BLOCK_SIZE = 32;
	void*         pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	PoolAllocator poolAllocator{static_cast<char*>(pMemoryBlock), 8, BLOCK_SIZE};

	{
		u64* pA = poolAllocator.Alloc<u64>();
		u64* pB = poolAllocator.Alloc<u64>();
		u64* pC = poolAllocator.Alloc<u64>();
		*pA = 1;
		*pB = 2;
		*pC = 3;
		EXPECT_EQ(*pA, 1);
		EXPECT_EQ(*pB, 2);
		EXPECT_EQ(*pC, 3);
	}

	poolAllocator.Reset();

	{
		u64* pA = poolAllocator.Alloc<u64>();
		u64* pB = poolAllocator.Alloc<u64>();
		u64* pC = poolAllocator.Alloc<u64>();
		*pA = 6;
		*pB = 4;
		*pC = 8;
		EXPECT_EQ(*pA, 6);
		EXPECT_EQ(*pB, 4);
		EXPECT_EQ(*pC, 8);
	}

	poolAllocator.Reset();
	Memory::Free(pMemoryBlock);
}

TEST(Allocators, PoolAllocator_AllocFree) {
	constexpr u64 BLOCK_SIZE = sizeof(u64) * 3;
	void*         pMemoryBlock = Memory::Alloc(BLOCK_SIZE);
	PoolAllocator poolAllocator{pMemoryBlock, sizeof(u64), BLOCK_SIZE};

	u64* pA = poolAllocator.Alloc<u64>();
	u64* pB = poolAllocator.Alloc<u64>();
	*pA = 4;
	*pB = 24;
	EXPECT_EQ(*pA, 4);
	EXPECT_EQ(*pB, 24);

	poolAllocator.Free(pB);

	pB = poolAllocator.Alloc<u64>();
	*pB = 6;
	EXPECT_EQ(*pA, 4);
	EXPECT_EQ(*pB, 6);

	poolAllocator.Reset();
	Memory::Free(pMemoryBlock);
}

// Stack Allocator
//-----------------------------------------------------------------------------

TEST(Allocators, StackAllocator_AllocFree) {
	StackAllocator stackAllocator{ Memory::Alloc(sizeof(u64) * 3), sizeof(u64) * 3 };
}

//-----------------------------------------------------------------------------
// Memory Tracking
//-----------------------------------------------------------------------------

TEST(MemoryTrackingManager, Track_Alloc_Free) {
	void* pMemory = Memory::Alloc(64);

	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalAllocatedMemory(), 64);
	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalReleasedMemory(), 0);
	EXPECT_EQ(Memory::g_MemoryTracking.GetAllocatedMemorySize(), 64);

	Memory::Free(pMemory);

	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalAllocatedMemory(), 64);
	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalReleasedMemory(), 64);
	EXPECT_EQ(Memory::g_MemoryTracking.GetAllocatedMemorySize(), 0);

	Vector<Memory::MemoryOperationInfo> const& ops = Memory::g_MemoryTracking.GetOperationsHistory();

	EXPECT_EQ(ops.size(), 2);
	EXPECT_EQ(ops[0].m_Operation, Memory::MemoryOp::Alloc);
	EXPECT_EQ(ops[1].m_Operation, Memory::MemoryOp::Free);
}

TEST(MemoryTrackingManager, Track_New_Delete) {
	Memory::g_MemoryTracking.Reset();
	u64* pMemory = Memory::New<u64>(1);

	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalAllocatedMemory(), sizeof(u64));
	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalReleasedMemory(), 0);
	EXPECT_EQ(Memory::g_MemoryTracking.GetAllocatedMemorySize(), sizeof(u64));

	Memory::Delete(pMemory);

	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalAllocatedMemory(), sizeof(u64));
	EXPECT_EQ(Memory::g_MemoryTracking.GetTotalReleasedMemory(), sizeof(u64));
	EXPECT_EQ(Memory::g_MemoryTracking.GetAllocatedMemorySize(), 0);

	Vector<Memory::MemoryOperationInfo> const& ops = Memory::g_MemoryTracking.GetOperationsHistory();

	EXPECT_EQ(ops.size(), 2);
	EXPECT_EQ(ops[0].m_Operation, Memory::MemoryOp::New);
	EXPECT_EQ(ops[1].m_Operation, Memory::MemoryOp::Delete);
}