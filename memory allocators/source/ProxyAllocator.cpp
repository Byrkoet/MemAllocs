#include "ProxyAllocator.h"

ProxyAllocator::ProxyAllocator(Allocator& alloc) :
	Allocator(alloc.GetSize()),
	m_Allocator(alloc)
{
}

ProxyAllocator::~ProxyAllocator()
{
}

void* ProxyAllocator::Allocate(size_t size, uint8_t alignment)
{
	assert(size != 0);

	m_Allocations++;

	size_t prev_mem = m_Allocator.GetUsedMemory();

	void* address = m_Allocator.Allocate(size, alignment);

	m_UsedMemory += m_Allocator.GetUsedMemory() - prev_mem;

	return address;
}

void ProxyAllocator::Deallocate(void *address)
{
	m_Allocations--;

	size_t prev_mem = m_Allocator.GetUsedMemory();

	m_Allocator.Deallocate(address);

	m_UsedMemory -= prev_mem - m_Allocator.GetUsedMemory();
}
