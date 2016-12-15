#pragma once

#include "Allocator.h"

// The pointer is moved by requested amount of bytes and aligned to store the address and header.
// Also holds the last allocation for debugging purposes, which is disabled in Release builds.
class StackAllocator : public Allocator
{
	StackAllocator(StackAllocator const&);
public:
	StackAllocator(size_t size);
	~StackAllocator();
private:
	struct Header
	{
		#if _DEBUG
		void *prev_address;
		#endif
		uint8_t adjustment;
	};
#if _DEBUG
	// Last allocation made.
	void *m_PreviousPosition;
#endif
	void *m_CurrentPosition;
public:
	void* Allocate(size_t size, uint8_t alignment) override;
	void Deallocate(void *addresss) override;
};
