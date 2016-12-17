#pragma once

#include <new.h>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>

// Interface for custom allocators.
// Allocator frees memory on delete.
class Allocator
{
public:
	Allocator(size_t size) :
		m_Size(size),
		m_Start(malloc(size))
	{
		m_UsedMemory = 0;
		m_Allocations = 0;
	}

	virtual ~Allocator()
	{
		free(m_Start);

		m_Start = nullptr;
		m_Size = 0;
	}
protected:
	size_t m_Size;
	void *m_Start;
	size_t m_UsedMemory;
	size_t m_Allocations;
public:
	virtual void* Allocate(size_t size, uint8_t alignment = 4) = 0;
	virtual void Deallocate(void *address) = 0;

	void* GetStart() const { return m_Start; }
	size_t GetSize() const { return m_Size; }
	size_t GetUsedMemory() const { return m_UsedMemory; }
	size_t GetNumAllocations() const { return m_Allocations; }
};

namespace alloc { namespace math
{
	// Add (@param size) to (@param address).
	inline void* Add(void *address, size_t size)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + size);
	}

	// Subtract (@param size) from (@param address).
	inline void* Subtract(void *address, size_t size)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) - size);
	}

	// Align memory address by (@param alignment) amount of bytes.
	// This masks the (@param alignment - 1) bit and adds to address.
	// @param alignment must be power of 2.
	inline void* Align(void *address, uint8_t alignment)
	{
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + static_cast<uintptr_t>(alignment - 1) & static_cast<uintptr_t>(~(alignment - 1)));
	}
	
	// NOTE: not suited for checking alignment, as it doesn't work as expected with addresses.
	// How many bytes needed to adjust address for alignment.
	// @param alignment must be power of 2.
	inline uint8_t AdjustmentFromAlign(void *address, uint8_t alignment)
	{
		const uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & static_cast<uintptr_t>(alignment - 1));

		// If already aligned, return 0.
		return (adjustment == alignment) ? 0 : adjustment;
	}

	// How many bytes are needed to adjust for alignment with a header.
	// @param alignment must be power of 2.
	inline uint8_t AdjustmentFromAlignWithHeader(void *address, uint8_t alignment, uint8_t header_size)
	{
		uint8_t adjustment = AdjustmentFromAlign(address, alignment);

		if (adjustment < header_size)
		{
			uint8_t needed_header_space = header_size - adjustment;

			// Make it so the adjustment fits the header.
			adjustment += alignment*(needed_header_space/alignment);

			// If needed space for header is not aligned, increase adjustment safely.
			if (needed_header_space % alignment > 0)
			{
				adjustment += alignment;
			}
		}

		return adjustment;
	}
}}

namespace alloc
{
	template<class T>
	void* Allocate(Allocator *allocator)
	{
		return allocator->Allocate(sizeof T, alignof(T));
	}

	template<class T>
	T* Allocate(Allocator *allocator, T *t)
	{
		return new(allocator->Allocate(sizeof T, alignof(T))) T(t);
	}

	template<class T>
	void Deallocate(Allocator *allocator, T *object)
	{
		object->~T();
		allocator->Deallocate(object);
	}

	template<class T>
	T* AllocateArray(Allocator *allocator, size_t array_length)
	{
		assert(array_length != 0);

		uint8_t header_size = sizeof size_t / sizeof T;

		if (sizeof size_t % sizeof T  > 0)
		{
			++header_size;
		}

		// Note: allocates extra memory to store array length before the array.
		T *array_pointer = static_cast<T*>(allocator->Allocate(sizeof T*(array_length + header_size), alignof(T))) + header_size;

		// Header holds array length.
		*(reinterpret_cast<uintptr_t*>(array_pointer) - 1) = array_length;

		// Initialize objects.
		for (size_t i = 0; i < array_length; i++)
		{
			// Initialize object on the address of array[i].
			new(&array_pointer[i]) T;
		}

		return array_pointer;
	}

	template<class T>
	void DeallocateArray(Allocator *allocator, T *array_object)
	{
		assert(array_object != nullptr);

		size_t length = *(reinterpret_cast<uintptr_t*>(array_object) - 1);

		for (size_t i = 0; i < length; i++)
		{
			array_object[i].~T();
		}

		// How much extra memory was allocated to store the length before the array.
		uint8_t header_size = sizeof size_t / sizeof T;

		if (sizeof size_t % sizeof T > 0)
		{
			++header_size;
		}

		allocator->Deallocate(array_object - header_size);
	}

	// Rarely used, since most object are naturally aligned.
	template<class T>
	bool IsAligned(T const* obj, size_t alignment = alignof(T))
	{
		return reinterpret_cast<uintptr_t>(obj) % alignment == 0;
	}

	// Test if an object is custom aligned.
	template<class T>
	bool IsAdjusted(T *obj, size_t alignment)
	{
		return !math::AdjustmentFromAlign(obj, alignment);
	}

	template<class T>
	struct destroy
	{
		Allocator *alloc = nullptr;
		
		void operator()(T *t) const
		{
			assert(t);
			t->~T();
			assert(alloc);
			alloc->Deallocate(t);
		}
	};

	// Alias template for our unique_ptr with our deleter type.
	template<class T>
	// Overshadows unique_ptr, and supports deallocation with custom allocators.
	using unique_ptr = std::unique_ptr<T, destroy<T>>;

	namespace factory
	{
		template<class T, class...Args>
		unique_ptr<T> make_unique(Allocator *alloc, Args&&...args)
		{
			void *p_data = alloc->Allocate(sizeof(T), alignof(T));

			try
			{
				T *ret = ::new(p_data) T(std::forward<Args>(args)...);
				return{ ret, destroy<T>{alloc} };
			}
			catch (...)
			{
				alloc->Deallocate(p_data);
				throw;
			}
		}
	}

	template<class T, class...Args>
	unique_ptr<T> make_unique(Allocator *alloc, Args&&...args)
	{
		return factory::make_unique<T>(alloc, std::forward<Args>(args)...);
	}

	// Overload for initializer_list.
	template<class T, class U, class...Args>
	unique_ptr<T> make_unique(Allocator *alloc, std::initializer_list<U> il, Args&&...args)
	{
		return factory::make_unique<T>(alloc, il, std::forward<Args>(args));
	}

	// Overload for braces construction.
	// Prevents template argument deduction.
	template<class T> struct tag_t { using type = T; };
	// Default Template Argument, for specifying which type is use.
	template<class T> using no_deduction = typename tag_t<T>::type;
	template<class T>
	unique_ptr<T> make_unique(Allocator *alloc, no_deduction<T> &&t)
	{
		return factory::make_unique<T>(alloc, std::move(t));
	}
}
