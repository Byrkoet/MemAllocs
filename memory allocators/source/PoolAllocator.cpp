#include "PoolAllocator.h"

using alloc::math::AdjustmentFromAlign;
using alloc::math::Add;

PoolAllocator::PoolAllocator(size_t size, size_t obj_size, uint8_t obj_alignment) :
	Allocator(size),
	m_ObjectSize(obj_size),
	m_ObjectAlignment(obj_alignment)
{
	// When blocks are freed, they store a pointer to the next free block.
	assert(obj_size >= sizeof(void*));

	// For keeping the alloc properly aligned.
	uint8_t adjustment = AdjustmentFromAlign(m_Start, obj_alignment);

	// Start of the FreeList.
	// Essentially adding a pointer for the aligned address to hold.
	m_FreeList = reinterpret_cast<void**>(Add(m_Start, adjustment));

	size_t object_count = (size - adjustment) / obj_size;

	void **free_address = m_FreeList;

	// Create list of free blocks.
	for (size_t i = 0; i < object_count - 1; i++)
	{
		// Add next free block to current address.
		// Dereferencing free_address gives u the next address.
		*free_address = Add(free_address, obj_size);

		// Allow the next free address to hold its own pointer.
		// We will use it in the next loop to store an address.
		free_address = reinterpret_cast<void**>(*free_address);
	}

	// End list similar to cstings.
	*free_address = nullptr;
}

PoolAllocator::~PoolAllocator()
{
	assert(m_Allocations == 0 && m_UsedMemory == 0);

	m_FreeList = nullptr;
}

void* PoolAllocator::Allocate(size_t size, uint8_t alignment)
{
	assert(size == m_ObjectSize && alignment == m_ObjectAlignment);

	// If no list, allocation impossible.
	if (!m_FreeList)
	{
		return nullptr;
	}

	// Next free address is current pointer of freelist.
	void *next_free_address = m_FreeList;

	// Set current pointer to next free address, by dereferencing.
	m_FreeList = reinterpret_cast<void**>(*m_FreeList);

	m_UsedMemory += size;
	m_Allocations++;

	return next_free_address;
}

void PoolAllocator::Deallocate(void *address)
{
	assert(address && "Address is a nullptr, usually indicating not enough memory on PoolAllocator");

	// Set current free address as next one for our deallocated block.
	*reinterpret_cast<void**>(address) = m_FreeList;

	// Set deallocated block as first free address available.
	m_FreeList = reinterpret_cast<void**>(address);

	m_UsedMemory -= m_ObjectSize;
	m_Allocations--;
}
