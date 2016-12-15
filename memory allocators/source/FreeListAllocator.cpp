#include "FreeListAllocator.h"

using alloc::math::AdjustmentFromAlign;
using alloc::math::AdjustmentFromAlignWithHeader;
using alloc::math::Add;
using alloc::math::Subtract;

FreeListAllocator::FreeListAllocator(size_t size) :
	Allocator(size),
	m_FreeBlock(reinterpret_cast<FreeBlock*>(m_Start))
{
	assert(size > sizeof FreeBlock);

	m_FreeBlock->size = size;
	m_FreeBlock->next = nullptr;
}

FreeListAllocator::~FreeListAllocator()
{
	assert(m_Allocations == 0 && m_UsedMemory == 0);

	m_FreeBlock = nullptr;
}

void* FreeListAllocator::Allocate(size_t size, uint8_t alignment)
{
	assert(size != 0 && alignment != 0);

	FreeBlock *prev_free_block = nullptr;
	FreeBlock *free_block = m_FreeBlock; // Starting block atm.

	while (free_block)
	{
		// Adjustment to keep object aligned.
		const uint8_t adjustment = AdjustmentFromAlignWithHeader(free_block, alignment, sizeof Header);

		size_t allocation_size = size + adjustment;

		// If FreeBlock is too small, try the next one in the list.
		if (free_block->size < allocation_size)
		{
			prev_free_block = free_block;
			free_block = free_block->next;
			continue;
		}

		static_assert(sizeof Header >= sizeof FreeBlock, "Size of Header cannot be less than FreeBlock at this point");

		// If allocation is not possible in the remaining memory.
		if (free_block->size - allocation_size <= sizeof Header)
		{
			// Increase size, instead of creating new FreeBlock.
			allocation_size = free_block->size;

			if (prev_free_block)
			{
				prev_free_block->next = free_block->next;
			}
			else
			{
				m_FreeBlock = free_block->next;
			}
		}
		// If allocation is possible, create new Freeblock with remaining memory.
		else
		{
			FreeBlock *const new_block = reinterpret_cast<FreeBlock*>(Add(free_block, allocation_size));
			new_block->size = free_block->size - allocation_size;
			new_block->next = free_block->next;

			if (prev_free_block)
			{
				prev_free_block->next = new_block;
			}
			else
			{
				m_FreeBlock = new_block;
			}
		}

		const uintptr_t aligned_address = reinterpret_cast<uintptr_t>(free_block) + adjustment;

		Header *const header = reinterpret_cast<Header*>(aligned_address - sizeof Header);
		header->size = allocation_size;
		header->adjustment = adjustment;

		m_UsedMemory += allocation_size;
		m_Allocations++;

		// Check if properly aligned.
		assert(AdjustmentFromAlign(reinterpret_cast<void*>(aligned_address), alignment) == 0);

		return reinterpret_cast<void*>(aligned_address);
	}

	return nullptr;
}

void FreeListAllocator::Deallocate(void *address)
{
	assert(address);

	const Header *header = reinterpret_cast<Header*>(Subtract(address, sizeof Header));

	// Size of the FreeBlock.
	const size_t block_size = header->size;
	// Start of the FreeBlock, by removing adjustment used.
	const uintptr_t block_start = reinterpret_cast<uintptr_t>(address) - header->adjustment;
	// End of the FreeBlock.
	const uintptr_t block_end = block_start + block_size;

	FreeBlock *prev_free_block = nullptr;
	FreeBlock *free_block = m_FreeBlock;

	// Find previous FreeBlock.
	while (free_block)
	{
		// If we are beyond our current FreeBlock, break.
		if (reinterpret_cast<uintptr_t>(free_block) >= block_end)
		{
			break;
		}

		prev_free_block = free_block;
		free_block = free_block->next;
	}

	// If no previous block, assign one (handy if the block is the first in the list).
	if (!prev_free_block)
	{
		prev_free_block = reinterpret_cast<FreeBlock*>(block_start);
		prev_free_block->size = block_size;
		prev_free_block->next = m_FreeBlock;

		m_FreeBlock = prev_free_block;
	}
	// Simply set the size if the previous FreeBlock is the adjacent one.
	else if (reinterpret_cast<uintptr_t>(prev_free_block) + prev_free_block->size == block_start)
	{
		prev_free_block->size = block_size;
	}
	// If no previous block, probably last block, so simply replace the previous one with current FreeBlock.
	else
	{
		FreeBlock *temp_block = reinterpret_cast<FreeBlock*>(block_start);
		temp_block->size = block_size;
		temp_block->next = prev_free_block->next;
		
		prev_free_block->next = temp_block;
		prev_free_block = temp_block;
	}

	// If a free block has been found, merge with adjacent.
	if (free_block && reinterpret_cast<uintptr_t>(free_block) == block_end)
	{
		prev_free_block->size += free_block->size;
		prev_free_block->next = free_block->next;
	}

	m_UsedMemory -= block_size;
	m_Allocations--;
}
