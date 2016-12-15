#pragma once

#include "Allocator.h"

class PoolAllocator : public Allocator
{
	PoolAllocator(PoolAllocator const&);
public:
	PoolAllocator(size_t size, size_t obj_size, uint8_t obj_alignment);
	~PoolAllocator();
private:
	size_t m_ObjectSize;
	uint8_t m_ObjectAlignment;
	void **m_FreeList;
public:
	void* Allocate(size_t size, uint8_t alignment) override;
	void Deallocate(void *address) override;
};
