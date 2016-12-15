#pragma once

#include "Allocator.h"

class ProxyAllocator : public Allocator
{
	ProxyAllocator(ProxyAllocator const&);
public:
	ProxyAllocator(Allocator &alloc);
	~ProxyAllocator();
private:
	Allocator &m_Allocator;
public:
	void* Allocate(size_t size, uint8_t alignment) override;
	void Deallocate(void *address) override;
};
