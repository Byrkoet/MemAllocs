#include "StackAllocator.h"

using alloc::math::AdjustmentFromAlignWithHeader;
using alloc::math::Add;
using alloc::math::Subtract;

StackAllocator::StackAllocator(size_t size) :
	Allocator(size),
	m_CurrentPosition(m_Start)
{
	assert(size > 0);

#if _DEBUG
	m_PreviousPosition = nullptr;
#endif
}

StackAllocator::~StackAllocator()
{
	assert(m_Allocations == 0 && m_UsedMemory == 0);

	m_CurrentPosition = nullptr;
#if _DEBUG
	m_PreviousPosition = nullptr;
#endif
}

void* StackAllocator::Allocate(size_t size, uint8_t alignment)
{
	assert(size != 0);

	const uint8_t adjustment = AdjustmentFromAlignWithHeader(m_CurrentPosition, alignment, sizeof Header);

	if (m_UsedMemory + adjustment + size > m_Size)
	{
		return nullptr;
	}

	void *aligned_address = Add(m_CurrentPosition, adjustment);

	Header *const header = reinterpret_cast<Header*>(Subtract(aligned_address, sizeof Header));

	header->adjustment = adjustment;
#if _DEBUG
	header->prev_address = m_PreviousPosition;

	m_PreviousPosition = aligned_address;
#endif

	// Current top of the stack.
	m_CurrentPosition = Add(aligned_address, size);

	// adjustment holds the size needed to fit the header so simply add it with the size requested.
	m_UsedMemory += adjustment + size;
	m_Allocations++;

	return aligned_address;
}

void StackAllocator::Deallocate(void *address)
{
	assert(address == m_PreviousPosition);

	Header *header = reinterpret_cast<Header*>(Subtract(address, sizeof Header));

	// m_CurrentPosition holds the aligned address and size, so subtract to get size and combine with adjustment.
	m_UsedMemory -= reinterpret_cast<uintptr_t>(m_CurrentPosition) - reinterpret_cast<uintptr_t>(address) + header->adjustment;

	// Set current pos to previous by subtracting the adjusted needed to get the next aligned address.
	m_CurrentPosition = Subtract(address, header->adjustment);

#if _DEBUG
	m_PreviousPosition = header->prev_address;
#endif

	m_Allocations--;
}
