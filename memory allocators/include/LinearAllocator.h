#pragma once

#include "Allocator.h"

// Linear allocator simply moves the pointer one free address forward.
// Individual deallocations aren't possible, instead use Clear() to clear member values.
//
// Maintains the starting address, the first free address and the total size.
class LinearAllocator : public Allocator
{
	LinearAllocator(LinearAllocator const&);
public:
	LinearAllocator(size_t size);
	~LinearAllocator();
private:
	void *m_CurrentPosition;
public:
	void* Allocate(size_t size, uint8_t alignment) override;
	void Deallocate(void *address_not_used) override;
	void Clear();
};
