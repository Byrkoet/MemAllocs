#pragma once

#include "gtest/gtest.h"
#include "Allocator.h"

namespace tests
{
	inline testing::AssertionResult AssertAdjustmentInFormat2(const char *address_expr, const char *alignment_expr, void *address, uint8_t alignment)
	{
		if (alloc::IsAdjusted(address, alignment))
		{
			return testing::AssertionSuccess();
		}

		return testing::AssertionFailure()
			<< address_expr << " and " << alignment_expr
			<< " (" << reinterpret_cast<uintptr_t>(address) << " is not aligned by " << alignment_expr << ")";
	}
}
