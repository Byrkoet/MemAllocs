#include "tests.h"
#include "LinearAllocator.h"
#include "StackAllocator.h"

namespace testing_basic
{
	TEST(AlignmentTest, AdjustmentFromAlignTest)
	{
		void *mem_test = reinterpret_cast<void*>(0xb);
		uint8_t alignment = alloc::math::AdjustmentFromAlign(mem_test, 4);

		ASSERT_EQ(alignment, 1llu);
	}

	TEST(AlignmentTest, AlignTest)
	{
		void *mem_test = reinterpret_cast<void*>(0xc);
		void *mem_aligned = alloc::math::Align(mem_test, 8);
		uint8_t alignment = alloc::math::AdjustmentFromAlign(mem_aligned, 8);

		ASSERT_EQ(alignment, 0llu);
	}

	TEST(AlignmentTest, AdjustTest)
	{
		void *mem_test = reinterpret_cast<void*>(0xf);
		void *mem_aligned = alloc::math::Align(mem_test, 8);
		EXPECT_PRED_FORMAT2(tests::AssertAdjustmentInFormat2, mem_aligned, 16);
	}

	TEST(AlignmentTest, AdjustTest2)
	{
		void *mem_test = reinterpret_cast<void*>(0xf);
		void *mem_aligned = alloc::math::Align(mem_test, 8);
		EXPECT_PRED_FORMAT2(tests::AssertAdjustmentInFormat2, mem_aligned, 16);
	}
}

namespace testing_linear_alloc
{
	struct LinearAllocator_F : testing::Test
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

	TEST_F(LinearAllocator_F, AllocatorStartsEmpty)
	{
		ASSERT_EQ(0llu, alloc->GetNumAllocations());
	}

	TEST_F(LinearAllocator_F, AllocationTest)
	{
		void *mem = alloc->Allocate(512, 8);
		ASSERT_TRUE(mem != nullptr);
	}

	TEST_F(LinearAllocator_F, UniquePointerTest)
	{
		alloc::unique_ptr<int> integer = alloc::make_unique<int>(alloc, 10);
		ASSERT_EQ(10, *integer);
	}
}

namespace testing_stack_alloc
{
	struct StackAllocator_F : testing::Test
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

	TEST_F(StackAllocator_F, AllocatorStartsEmpty)
	{
		ASSERT_EQ(0llu, alloc->GetNumAllocations());
	}

	TEST_F(StackAllocator_F, AllocationTest)
	{
		void *mem = alloc->Allocate(512, 8);
		ASSERT_TRUE(mem != nullptr);
		alloc->Deallocate(mem);
	}

	TEST_F(StackAllocator_F, UniquePointerTest)
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
