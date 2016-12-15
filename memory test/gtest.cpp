#include "gtest/gtest.h"
#include "LinearAllocator.h"
#include "StackAllocator.h"

namespace
{
	struct LinearAllocatorTest : testing::Test
	{
		LinearAllocator *alloc;

		void SetUp() override
		{
			alloc = new LinearAllocator(1024);
		}

		void TearDown() override
		{
			alloc->Clear();
			delete alloc;
		}
	};

	TEST_F(LinearAllocatorTest, AllocatorStartsEmpty)
	{
		ASSERT_EQ(0llu, alloc->GetNumAllocations());
	}

	TEST_F(LinearAllocatorTest, AllocationTest)
	{
		void *mem = alloc->Allocate(512, 8);
		ASSERT_TRUE(mem != nullptr);
	}

	TEST_F(LinearAllocatorTest, UniquePointerTest)
	{
		alloc::unique_ptr<int> integer = alloc::make_unique<int>(alloc, 10);
		ASSERT_EQ(10, *integer);
	}

	struct StackAllocatorTest : testing::Test
	{
		StackAllocator *alloc;

		void SetUp() override
		{
			alloc = new StackAllocator(1024);
		}

		void TearDown() override
		{
			delete alloc;
		}
	};

	TEST_F(StackAllocatorTest, AllocatorStartsEmpty)
	{
		ASSERT_EQ(0llu, alloc->GetNumAllocations());
	}

	TEST_F(StackAllocatorTest, AllocationTest)
	{
		void *mem = alloc->Allocate(512, 8);
		ASSERT_TRUE(mem != nullptr);
		alloc->Deallocate(mem);
	}

	TEST_F(StackAllocatorTest, UniquePointerTest)
	{
		alloc::unique_ptr<int> integer = alloc::make_unique<int>(alloc, 123);
		ASSERT_EQ(123, *integer);
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
