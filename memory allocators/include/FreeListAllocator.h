#pragma once

#include "Allocator.h"

// Linked list of free blocks of memory:
// Every free block contains the next free block.
class FreeListAllocator : public Allocator
{
	FreeListAllocator(FreeListAllocator const&);
public:
	FreeListAllocator(size_t size);
	~FreeListAllocator();
private:
	// Holds size and adjustment.
	struct Header
	{
		// Allocation size.
		size_t size;
		// Allocation adjustment.
		uint8_t adjustment;
	};

	// Holds size and next FreeBlock.
	struct FreeBlock
	{
		// Memory size.
		size_t size;
		// Next FreeBlock.
		FreeBlock *next;
	};

	FreeBlock *m_FreeBlock;
public:
	void* Allocate(size_t size, uint8_t alignment) override;
	void Deallocate(void *address) override;
};
