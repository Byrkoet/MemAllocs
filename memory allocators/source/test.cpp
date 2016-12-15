#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "FreeListAllocator.h"
#include "PoolAllocator.h"
#include "ProxyAllocator.h"

#include "MyCounter.h"

#include <iostream>
#include <cstdarg>
#include <stack>

#define SIZE_1MB 1048576
#define SIZE_2MB 2097152
#define SIZE_150MB 157286400
#define SIZE_400MB 419430400
#define SIZE_800MB 838860800
#define SIZE_ALLOC SIZE_150MB

#define NUM_16B_ALLOCS 10000
#define NUM_256B_ALLOCS 1000
#define NUM_2MB_ALLOCS 50

using std::cout;
using std::endl;

struct MyClass
{
	MyClass(size_t size = 0llu) :
		m_size(size)
	{}

	size_t m_size;
};

struct MyClass2
{
	MyClass2(size_t size = 0llu) :
		m_size(size)
	{}

	size_t m_size;

	void* operator new(size_t size, Allocator *alloc)
	{
		return alloc->Allocate(size, alignof(MyClass));
	}
};

// Prints base allocator variables.
inline void PrintAllocatorValues(Allocator *alloc)
{
	const size_t allocs = alloc->GetNumAllocations();

	printf("   Allocations: %llu\n", allocs);
	printf("Allocator Size: %llu bytes\n", alloc->GetSize());
	printf("Allocator Used: %llu bytes\n", alloc->GetUsedMemory());
	printf(" Average alloc: %llu bytes\n", alloc->GetUsedMemory() / (allocs > 0 ? allocs : 1));
}

// Prints a minimum of 3 memory addresses.
void PrintAddressValues(uintptr_t start, void*...)
{
	va_list list;

	const unsigned n_args = 3;

	va_start(list, n_args);

	printf("\n    Starting address: %llu (0x%p)\n\n", start, reinterpret_cast<void*>(start));

	for (unsigned i = 0; i < n_args; i++)
	{
		uintptr_t address = va_arg(list, uintptr_t);
		printf("Address nr%u: +%llu (0x%p)\n", i + 1, address - start, reinterpret_cast<void*>(address));
	}

	va_end(list);
}

void BenchmarkMalloc()
{
	std::stack<void*> allocations;

	MyCounter counter;
	counter.Start();

	for (unsigned i = 0; i < NUM_16B_ALLOCS; i++)
	{
		allocations.push(malloc(16));
	}

	for (unsigned i = 0; i < NUM_256B_ALLOCS; i++)
	{
		allocations.push(malloc(256));
	}

	for (unsigned i = 0; i < NUM_2MB_ALLOCS; i++)
	{
		allocations.push(malloc(SIZE_2MB));
	}

	const size_t blocks_allocated = allocations.size();

	while (!allocations.empty())
	{
		free(allocations.top());
		allocations.pop();
	}

	const double elapsed = counter.Elapsed();

	printf("\nMalloc: %.2fms\n  Allocated: %llu blocks\n", elapsed, blocks_allocated);
}

void BenchmarkLinearAllocator()
{
	std::stack<void*> allocations;
	LinearAllocator *alloc = new LinearAllocator(SIZE_ALLOC);

	MyCounter counter;
	counter.Start();

	for (unsigned i = 0; i < NUM_16B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(16, 8));
	}

	for (unsigned i = 0; i < NUM_256B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(256, 8));
	}
	
	for (unsigned i = 0; i < NUM_2MB_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(SIZE_2MB, 8));
	}

	const size_t kilobytes_used = alloc->GetUsedMemory() / 1024llu;
	const size_t blocks_allocated = allocations.size();

	alloc->Clear();

	const double elapsed = counter.Elapsed();

	printf("\nLinear Allocator: %.2fms\n  Allocated: %llu blocks\n  Memory used: %lluKB\n", elapsed, blocks_allocated, kilobytes_used);

	// Clean up.
	allocations.empty();
	delete alloc;
}

void BenchmarkStackAllocator()
{
	std::stack<void*> allocations;
	StackAllocator *alloc = new StackAllocator(SIZE_ALLOC);

	MyCounter counter;
	counter.Start();

	for (unsigned i = 0; i < NUM_16B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(16, 8));
	}

	for (unsigned i = 0; i < NUM_256B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(256, 8));
	}

	for (unsigned i = 0; i < NUM_2MB_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(SIZE_2MB, 8));
	}

	const size_t kilobytes_used = alloc->GetUsedMemory() / 1024llu;
	const size_t blocks_allocated = allocations.size();

	while (!allocations.empty())
	{
		alloc->Deallocate(allocations.top());
		allocations.pop();
	}

	const double elapsed = counter.Elapsed();

	printf("\nStack Allocator: %.2fms\n  Allocated: %llu blocks\n  Memory used: %lluKB\n", elapsed, blocks_allocated, kilobytes_used);

	// Clean up.
	delete alloc;
}

void BenchmarkFreeListAllocator()
{
	std::stack<void*> allocations;
	FreeListAllocator *alloc = new FreeListAllocator(SIZE_ALLOC);

	MyCounter counter;
	counter.Start();

	for (unsigned i = 0; i < NUM_16B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(16, 8));
	}

	for (unsigned i = 0; i < NUM_256B_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(256, 8));
	}

	for (unsigned i = 0; i < NUM_2MB_ALLOCS; i++)
	{
		allocations.push(alloc->Allocate(SIZE_2MB, 8));
	}

	const size_t kilobytes_used = alloc->GetUsedMemory() / 1024llu;
	const size_t blocks_allocated = allocations.size();

	while (!allocations.empty())
	{
		alloc->Deallocate(allocations.top());
		allocations.pop();
	}

	const double elapsed = counter.Elapsed();

	printf("\nFreeList Allocator: %.2fms\n  Allocated: %llu blocks\n  Memory used: %lluKB\n", elapsed, blocks_allocated, kilobytes_used);

	// Clean up.
	delete alloc;
}

void BenchmarkPoolAllocator()
{
	const size_t obj_size = 256;
	const unsigned total_allocations = NUM_256B_ALLOCS + NUM_16B_ALLOCS + NUM_2MB_ALLOCS;

	std::stack<void*> allocations;
	PoolAllocator *alloc = new PoolAllocator(SIZE_150MB, obj_size, 8);

	MyCounter counter;
	counter.Start();

	for (unsigned i = 0; i < total_allocations; i++)
	{
		allocations.push(alloc->Allocate(256, 8));
	}

	const size_t kilobytes_used = alloc->GetUsedMemory() / 1024llu;
	const size_t blocks_allocated = allocations.size();

	while (!allocations.empty())
	{
		alloc->Deallocate(allocations.top());
		allocations.pop();
	}

	const double elapsed = counter.Elapsed();

	printf("\nPool Allocator: %.2fms\n  Allocated: %llu blocks\n  Memory used: %lluKB\n", elapsed, blocks_allocated, kilobytes_used);

	// Clean up.
	delete alloc;
}

void TestLinearAlloc()
{
	LinearAllocator *alloc = new LinearAllocator(32);
	const uintptr_t start = reinterpret_cast<uintptr_t>(alloc->GetStart());

	// Allocating 1 is a quick way to get the next aligned address.
	void *test_mem = alloc->Allocate(1, 4);
	void *test_mem2 = alloc->Allocate(1, 4);
	void *test_mem3 = alloc->Allocate(1, 4);

	// Print to console.
	PrintAllocatorValues(alloc);
	PrintAddressValues(start, test_mem, test_mem2, test_mem3);

	alloc->Clear();

	delete alloc;
}

void TestStackAlloc()
{
	StackAllocator *alloc = new StackAllocator(128);
	const uintptr_t start = reinterpret_cast<uintptr_t>(alloc->GetStart());

	// Allocating 1 is a quick way to get the next aligned address.
	void *test_mem = alloc->Allocate(1, 4);
	void *test_mem2 = alloc->Allocate(1, 4);
	void *test_mem3 = alloc->Allocate(1, 4);

	alloc->Deallocate(test_mem3);
	alloc->Deallocate(test_mem2);
	alloc->Deallocate(test_mem);

	// Print to console.
	PrintAllocatorValues(alloc);
	PrintAddressValues(start, test_mem, test_mem2, test_mem3);

	delete alloc;
}

void TestFreeListAlloc()
{
	FreeListAllocator *alloc = new FreeListAllocator(128);
	const uintptr_t start = reinterpret_cast<uintptr_t>(alloc->GetStart());

	// Allocating 1 is a quick way to get the next aligned address.
	void *test_mem = alloc->Allocate(1, 4);
	void *test_mem2 = alloc->Allocate(1, 4);
	void *test_mem3 = alloc->Allocate(1, 4);

	// Print to console.
	PrintAllocatorValues(alloc);
	PrintAddressValues(start, test_mem, test_mem2, test_mem3);

	alloc->Deallocate(test_mem2);
	alloc->Deallocate(test_mem);
	alloc->Deallocate(test_mem3);

	delete alloc;
}

void TestPoolAllocator()
{
	const size_t mem_size = 33;
	const size_t obj_size = 11;
	const size_t obj_alignment = 4;
	PoolAllocator *alloc = new PoolAllocator(mem_size, obj_size, obj_alignment);
	const uintptr_t start = reinterpret_cast<uintptr_t>(alloc->GetStart());

	void *test_mem = alloc->Allocate(obj_size, obj_alignment);
	void *test_mem2 = alloc->Allocate(obj_size, obj_alignment);
	void *test_mem3 = alloc->Allocate(obj_size, obj_alignment);

	// Print to console.
	PrintAllocatorValues(alloc);
	PrintAddressValues(start, test_mem, test_mem2, test_mem3);

	alloc->Deallocate(test_mem3);
	alloc->Deallocate(test_mem2);
	alloc->Deallocate(test_mem);

	delete alloc;
}

void TestFreeListThroughProxy()
{
	const size_t mem_size = 128;
	FreeListAllocator *freelist_alloc = new FreeListAllocator(mem_size);
	ProxyAllocator *alloc = new ProxyAllocator(*freelist_alloc);
	const uintptr_t start = reinterpret_cast<uintptr_t>(alloc->GetStart());

	// Allocating 1 is a quick way to get the next aligned address.
	void *test_mem = alloc->Allocate(1, 4);
	void *test_mem2 = alloc->Allocate(1, 4);
	void *test_mem3 = alloc->Allocate(1, 4);

	// Print to console.
	PrintAllocatorValues(alloc);
	PrintAddressValues(start, test_mem, test_mem2, test_mem3);

	alloc->Deallocate(test_mem2);
	alloc->Deallocate(test_mem);
	alloc->Deallocate(test_mem3);

	delete alloc;
	delete freelist_alloc;
}

void TestArrays()
{
	FreeListAllocator *alloc = new FreeListAllocator(128);

	const size_t array_length = 3;

	int *integers = alloc::AllocateArray<int>(alloc, array_length);
	integers[0] = 2;
	integers[1] = 4;
	integers[2] = 6;

	for (int i = 0; i < array_length; i++)
	{
		printf("value %i: %i\n", i, integers[i]);
	}

	alloc::DeallocateArray(alloc, integers);

	delete alloc;
}

void Test()
{
	FreeListAllocator *alloc = new FreeListAllocator(128);

	alloc::unique_ptr<MyClass> obj_test = alloc::make_unique<MyClass>(alloc, 101);
	
	printf("Value: %llu\n", obj_test->m_size);
	 
	obj_test.reset();

	delete alloc;
}

int main(int agrc, char **agrv)
{
	Test();
	//TestArrays();

	//TestFreeListThroughProxy();
	//TestPoolAllocator();
	//TestFreeListAlloc();
	//TestStackAlloc();
	//TestLinearAlloc();
	
	//BenchmarkMalloc();
	//BenchmarkLinearAllocator();
	//BenchmarkStackAllocator();
	//BenchmarkFreeListAllocator();
	//BenchmarkPoolAllocator();
	
	cout << endl;
	system("pause");

	return 0;
}
