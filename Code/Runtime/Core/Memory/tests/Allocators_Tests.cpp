#include "CookieKat/Core/Platform/PlatformTime.h"

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/Asserts.h"

#include "CookieKat/Core/Memory/Memory.h"
#include "CookieKat/Core/Memory/LinearAllocator.h"
#include "CookieKat/Core/Memory/StackAllocator.h"
#include "CookieKat/Core/Memory/PoolAllocator.h"

#include <gtest/gtest.h>

using namespace CKE;


TEST(Core_Memory, NewArray_DeleteArray)
{
	// Data type to check if the constructor and destructor
	// are called correctly on the NewArray operation
	// The constructor of this class populates its elements and receives a ptr to an array
	// and the destructor writes data to the ptr
	struct NewArrayElem
	{
		NewArrayElem(i32* p) { ptr = p; i = 1; }
		~NewArrayElem() { *ptr = i; }

		i32* ptr;
		i32  i;
	};

	usize constexpr elemCount = 17;

	i32  destructorArray[elemCount];
	auto allocatedArray = CKE::NewArray<NewArrayElem>(elemCount, &destructorArray[0]);

	for (usize i = 0; i < elemCount; ++i)
	{
		// Check constructor was called and that the alignment was correct
		EXPECT_EQ(allocatedArray[i].ptr, &destructorArray[0]);
		EXPECT_EQ(allocatedArray[i].i, 1);
		EXPECT_TRUE(CKE::IsAligned(&allocatedArray[i]));

		allocatedArray[i].ptr = &destructorArray[i];
		allocatedArray[i].i = i;
	}

	CKE::DeleteArray(allocatedArray);

	// Check destructor was called
	for (int i = 0; i < elemCount; ++i)
	{
		EXPECT_EQ(destructorArray[i], i);
	}
}

TEST(Core_Allocators, Linear_Allocator_Alloc)
{
	constexpr u64   BLOCK_SIZE = 32;
	void* pMemoryBlock = CKE::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{ pMemoryBlock, BLOCK_SIZE };

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

	linearAllocator.FreeAll();
	CKE::Free(pMemoryBlock);
}

TEST(Core_Allocators, Linear_Allocator_OverflowThrows)
{
	constexpr u64   BLOCK_SIZE = 1;
	void* pMemoryBlock = CKE::Alloc(BLOCK_SIZE);
	LinearAllocator linearAllocator{ pMemoryBlock, BLOCK_SIZE };

#ifdef CKE_BUILDSYSTEM_ASSERTS_ENABLE
	EXPECT_DEATH({ i32 * pI32 = linearAllocator.Alloc<i32>(); }, "Assertion failed");
#endif

	linearAllocator.FreeAll();
}

TEST(Core_Allocators, Pool_Allocator_Alloc)
{
	constexpr u64 BLOCK_SIZE = 32;
	void* pMemoryBlock = CKE::Alloc(BLOCK_SIZE);
	PoolAllocator poolAllocator{ static_cast<char*>(pMemoryBlock), 8, BLOCK_SIZE };

	u64* pA = poolAllocator.AllocChunk<u64>();
	u64* pB = poolAllocator.AllocChunk<u64>();
	u64* pC = poolAllocator.AllocChunk<u64>();

	*pA = 1;
	*pB = 2;
	*pC = 3;

	EXPECT_EQ(*pA, 1);
	EXPECT_EQ(*pB, 2);
	EXPECT_EQ(*pC, 3);

	poolAllocator.FreeChunk(pB);
	poolAllocator.FreeChunk(pA);
	poolAllocator.FreeChunk(pC);

	Free(pMemoryBlock);
}
