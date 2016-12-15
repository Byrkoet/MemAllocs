#include "LinearAllocator.h"

using alloc::math::AdjustmentFromAlign;

LinearAllocator::LinearAllocator(size_t size) :
	Allocator(size),
	m_CurrentPosition(m_Start)
{
	assert(size > 0);
}

LinearAllocator::~LinearAllocator()
{
	assert(m_Allocations == 0 && m_UsedMemory == 0);

	m_CurrentPosition = nullptr;
}

void* LinearAllocator::Allocate(size_t size, uint8_t alignment)
{
	assert(size != 0);

	const uint8_t adjustment = AdjustmentFromAlign(m_CurrentPosition, alignment);

	if (m_UsedMemory + adjustment + size > m_Size)
	{
		return nullptr;
	}

	const uintptr_t aligned_address = reinterpret_cast<uintptr_t>(m_CurrentPosition) + adjustment;

	m_CurrentPosition = reinterpret_cast<void*>(aligned_address + size);

	m_UsedMemory += size + adjustment;
	m_Allocations++;

	return reinterpret_cast<void*>(aligned_address);
}

void LinearAllocator::Deallocate(void *address_not_used)
{
	return void();
}

void LinearAllocator::Clear()
{
	m_Allocations = 0;
	m_UsedMemory = 0;
	m_CurrentPosition = m_Start;
}
