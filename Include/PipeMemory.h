// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Limits.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"


namespace p
{

	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;


#pragma region Memory Ops
	namespace Details
	{
		template<typename T>
		struct TIsCharacterByteOrBool : std::false_type
		{};
		template<>
		struct TIsCharacterByteOrBool<char> : std::true_type
		{};    // chars are characters
		template<>
		struct TIsCharacterByteOrBool<signed char> : std::true_type
		{};    // signed chars are also characters
		template<>
		struct TIsCharacterByteOrBool<unsigned char> : std::true_type
		{};    // unsigned chars are also characters
		template<>
		struct TIsCharacterByteOrBool<char8_t> : std::true_type
		{};
		template<>
		struct TIsCharacterByteOrBool<std::byte> : std::true_type
		{};
		template<>
		struct TIsCharacterByteOrBool<bool> : std::true_type
		{};

		template<typename T>
		struct TIsMemsetZeroConstructible
		{
			static constexpr bool value =
			    IsScalar<T> && TIsCharacterByteOrBool<T>::value && !IsVolatile<T>;
		};

		template<typename T>
		struct TIsMemsetValueConstructible
		{
			static constexpr bool value = IsScalar<T> && !IsVolatile<T> && !IsMemberPointer<T>;
		};

		template<typename T>
		bool AreBitsZero(const T& value) requires(IsScalar<T> && !IsMemberPointer<T>)
		{
			if constexpr (IsSame<T, std::nullptr_t>)
			{
				return true;
			}
			else
			{
				constexpr T zero{};
				return CmpMem(&value, &zero, sizeof(T)) == 0;
			}
		}
	}    // namespace Details

	namespace Memory
	{
		// Kilobyte, Megabyte, Gigabyte...
		constexpr sizet B  = 1;
		constexpr sizet KB = 1024;
		constexpr sizet MB = KB * 1024;
		constexpr sizet GB = MB * 1024;
		constexpr sizet TB = GB * 1024;
		constexpr sizet PB = TB * 1024;

		// Kilobit, Megabit, Gigabit...
		constexpr sizet Kb = KB / 8;
		constexpr sizet Mb = MB / 8;
		constexpr sizet Gb = GB / 8;
		constexpr sizet Tb = TB / 8;
		constexpr sizet Pb = PB / 8;
	}    // namespace Memory


	P_API void MoveMem(void* dest, void* src, sizet size);
	P_API void CopyMem(void* dest, const void* src, sizet size);
	P_API void SwapMem(void* a, void* b, sizet size);
	P_API void SetMem(void* dest, u8 value, sizet size);
	P_API void SetZeroMem(void* dest, sizet size);
	P_API i32 CmpMem(const void* a, const void* b, sizet size);

	/**
	 * @return the previous pointer of 'ptr' aligned to blockSize
	 */
	void* GetAlignedBlock(void* ptr, const sizet blockSize);

	/**
	 * @return the number of bytes needed for p to be aligned in 'align'
	 */
	P_API sizet GetAlignmentPadding(const void* p, sizet align);

	P_API sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize);


	/** Constructs a number of contiguous items with the default constructor */
	template<typename T>
	constexpr void ConstructItems(T* data, sizet count)
	{
		if (!std::is_constant_evaluated() && Details::TIsMemsetZeroConstructible<T>::value)
		{
			SetMem(data, 0, count * sizeof(T));
		}
		else
		{
			const T* const end = data + count;
			while (data < end)
			{
				new (data) T();
				++data;
			}
		}
	}

	/** Constructs a number of contiguous items by copying from a single source item */
	template<typename T>
	constexpr void ConstructItems(T* data, sizet count, const T& value)
	{
		if (!std::is_constant_evaluated())
		{
			if constexpr (Details::TIsMemsetValueConstructible<T>::value)
			{
				SetMem(data, static_cast<unsigned char>(value), count * sizeof(T));
				return;
			}
			else if constexpr (Details::TIsMemsetZeroConstructible<T>::value)
			{
				if (Details::AreBitsZero(value))
				{
					SetMem(data, 0, count * sizeof(T));
					return;
				}
			}
		}

		const T* const end = data + count;
		while (data < end)
		{
			new (data) T(value);
			++data;
		}
	}

	/** Constructs a number of contiguous items by copying from source items */
	template<typename T, bool destroySourceInPlace = false>
	constexpr void CopyConstructItems(T* data, sizet count, const T* values)
	{
		if (!std::is_constant_evaluated() && IsTriviallyCopyConstructible<T>)
		{
			CopyMem(data, values, count * sizeof(T));
		}
		else
		{
			const T* const end = values + count;
			while (values < end)
			{
				new (data) T(*values);
				if constexpr (destroySourceInPlace)
				{
					values->T::~T();
				}
				++values;
				++data;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveConstructItems(T* data, sizet count, T* values)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveConstructible<T>)
		{
			MoveMem(data, values, count * sizeof(T));
		}
		else
		{
			const T* const end = values + count;
			while (values < end)
			{
				new (data) T(p::Forward<T>(*values));
				if constexpr (destroySourceInPlace)
				{
					values->T::~T();
				}
				++values;
				++data;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveOrCopyConstructItems(T* data, sizet count, T* values)
	{
		if constexpr (IsMoveConstructible<T> || !IsCopyConstructible<T>)
		{
			MoveConstructItems<T, destroySourceInPlace>(data, count, values);
		}
		else
		{
			CopyConstructItems<T, destroySourceInPlace>(data, count, values);
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void CopyItems(T* dest, sizet count, const T* src)
	{
		if (!std::is_constant_evaluated() && IsTriviallyCopyAssignable<T>)
		{
			CopyMem(dest, src, count * sizeof(T));
		}
		else
		{
			const T* const end = src + count;
			while (src < end)
			{
				*dest = *src;
				if constexpr (destroySourceInPlace)
				{
					src->T::~T();
				}
				++src;
				++dest;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveItems(T* dest, sizet count, T* source)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveAssignable<T>)
		{
			MoveMem(dest, source, count * sizeof(T));
		}
		else
		{
			const T* const end = source + count;
			while (source < end)
			{
				*dest = p::Forward<T>(*source);
				if constexpr (destroySourceInPlace)
				{
					source->T::~T();
				}
				++source;
				++dest;
			}
		}
	}

	// Move items [source, source + count) backwards to [..., dest)
	template<typename T>
	void MoveItemsBackwards(T* dest, sizet count, T* source)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveAssignable<T>)
		{
			// MoveMem (or std::memmove) is safe in overlapping memory
			p::MoveMem(dest, source, count * sizeof(T));
		}
		else
		{
			T* lastDest   = dest + count;
			T* lastSource = source + count;
			while (lastSource != source)
			{
				*--lastDest = p::Forward<T>(*--lastSource);
			}
		}
	}

	template<typename T>
	constexpr void DestroyItems(T* data, sizet count)
	{
		if (!std::is_constant_evaluated() && IsTriviallyDestructible<T>)
		{
			// Do nothing. No destruction needed.
		}
		else
		{
			const T* const end = data + count;
			while (data < end)
			{
				data->T::~T();
				++data;
			}
		}
	}
#pragma endregion Memory Ops

#pragma region Allocation
	class Arena;
	class HeapArena;

	void InitializeMemory();

	// Native allocation functions
	P_API void* HeapAlloc(sizet size);
	P_API void* HeapAlloc(sizet size, sizet align);
	P_API void* HeapRealloc(void* ptr, sizet size);
	P_API void HeapFree(void* ptr);


	P_API HeapArena& GetHeapArena();
	P_API Arena& GetCurrentArena();
	P_API void SetCurrentArena(Arena& arena);

	// Arena allocation functions (Find current arena)
	P_API void* Alloc(sizet size);
	P_API void* Alloc(sizet size, sizet align);
	P_API bool Realloc(void* ptr, sizet ptrSize, sizet size);
	P_API void Free(void* ptr, sizet size);


	// Templated arena allocation functions:

	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count = 1) requires(!IsVoid<T>)
	{
		return static_cast<T*>(arena.Alloc(sizeof(T) * count, alignof(T)));
	}
	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count, sizet align) requires(!IsVoid<T>)
	{
		return static_cast<T*>(arena.Alloc(sizeof(T) * count, align));
	}
	template<typename T, Derived<Arena> ArenaT>
	bool Realloc(ArenaT& arena, T* ptr, sizet ptrCount, sizet newCount) requires(!IsVoid<T>)
	{
		return arena.Realloc(ptr, sizeof(T) * ptrCount, sizeof(T) * newCount);
	}
	template<typename T, Derived<Arena> ArenaT>
	void Free(ArenaT& arena, T* ptr, u32 count = 1) requires(!IsVoid<T>)
	{
		arena.Free(static_cast<void*>(ptr), sizeof(T) * count);
	}
#pragma endregion Allocation

#pragma region Arena
	struct P_API ArenaBlock
	{
		void* data = nullptr;
		sizet size = 0;


		ArenaBlock() = default;
		ArenaBlock(void* data, sizet size) : data{data}, size{size} {}
		ArenaBlock(ArenaBlock&& other) noexcept;
		ArenaBlock& operator=(ArenaBlock&& other) noexcept;
		ArenaBlock(const ArenaBlock& other)            = default;
		ArenaBlock& operator=(const ArenaBlock& other) = default;


		bool IsAllocated() const
		{
			return data != nullptr;
		}

		void* Begin() const
		{
			return data;
		}
		void* End() const
		{
			return static_cast<u8*>(data) + size;
		}

		bool Contains(void* ptr) const
		{
			return data <= ptr && static_cast<u8*>(data) + size > ptr;
		}

		const void* operator*() const
		{
			return data;
		}

		void* operator*()
		{
			return data;
		}
	};


	/** Arena defines the API used on all other arena types */
	class P_API Arena
	{
	public:
		using AllocSignature        = void*(Arena*, sizet size);
		using AllocAlignedSignature = void*(Arena*, sizet size, sizet align);
		using ResizeSignature       = bool(Arena*, void* ptr, sizet ptrSize, sizet size);
		using FreeSignature         = void(Arena*, void* ptr, sizet size);

	private:
		AllocSignature* doAlloc               = nullptr;
		AllocAlignedSignature* doAllocAligned = nullptr;
		ResizeSignature* doRealloc            = nullptr;
		FreeSignature* doFree                 = nullptr;


		template<typename T>
		static consteval bool ImplementsAlloc()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return (void* (T::*)(sizet size))(&T::Alloc)
			    != (void* (T::*)(sizet size))(&Arena::Alloc);
#endif
		}
		template<typename T>
		static consteval bool ImplementsAllocAligned()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return (void* (T::*)(sizet size, sizet align))(&T::Alloc)
			    != (void* (T::*)(sizet size, sizet align))(&Arena::Alloc);
#endif
		}
		template<typename T>
		static consteval bool ImplementsRealloc()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return &T::Realloc != &Arena::Realloc;
#endif
		}
		template<typename T>
		static consteval bool ImplementsFree()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return &T::Free != &Arena::Free;
#endif
		}

	protected:

		template<Derived<Arena, false> T>
		void Interface()
		{
			doAlloc = [](Arena* self, sizet size) {
				static_assert(ImplementsAlloc<T>() && "Alloc is not implemented");
				return static_cast<T*>(self)->Alloc(size);
			};
			doAllocAligned = [](Arena* self, sizet size, sizet align) {
				static_assert(ImplementsAllocAligned<T>() && "Alloc (aligned) is not implemented");
				return static_cast<T*>(self)->Alloc(size, align);
			};
			doRealloc = [](Arena* self, void* ptr, sizet ptrSize, sizet size) {
				static_assert(ImplementsRealloc<T>() && "Realloc is not implemented");
				return static_cast<T*>(self)->Realloc(ptr, ptrSize, size);
			};
			doFree = [](Arena* self, void* ptr, sizet size) {
				static_assert(ImplementsFree<T>() && "Free is not implemented");
				return static_cast<T*>(self)->Free(ptr, size);
			};
		}

	public:
		Arena() = default;
		virtual ~Arena() {}
		Arena(const Arena&)            = delete;
		Arena& operator=(const Arena&) = delete;

		Arena(Arena&&)            = default;
		Arena& operator=(Arena&&) = default;


		void* Alloc(sizet size)
		{
			return doAlloc(this, size);
		}
		void* Alloc(sizet size, sizet align)
		{
			return doAllocAligned(this, size, align);
		}
		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return doRealloc(this, ptr, ptrSize, size);
		}
		void Free(void* ptr, sizet size)
		{
			doFree(this, ptr, size);
		}

		virtual sizet GetAvailableMemory() const
		{
			return 0;
		}
		virtual void GetBlocks(TInlineArray<ArenaBlock, 0>& /**outBlocks*/) const {}

		virtual const struct MemoryStats* GetStats() const
		{
			return nullptr;
		}
	};

	class P_API ChildArena : public Arena
	{
	protected:
		Arena* parent = nullptr;

	public:
		ChildArena(Arena* parent);

		Arena& GetParentArena() const
		{
			return *parent;
		}
	};
#pragma endregion Arena

#pragma region STL Allocator
	template<typename T>
	struct STLAllocator
	{
		using value_type      = T;
		using size_type       = sizet;
		using difference_type = std::ptrdiff_t;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;

		template<typename U>
		struct rebind
		{
			using other = STLAllocator<U>;
		};

		Arena* arena = nullptr;


		STLAllocator(Arena& arena = GetCurrentArena()) noexcept : arena{&arena} {}
		STLAllocator(const STLAllocator& other) noexcept : arena{other.arena} {}
		template<typename U>
		STLAllocator(const STLAllocator<U>& other) noexcept : arena{other.arena}
		{}
		STLAllocator select_on_container_copy_construction() const
		{
			return *this;
		}

#if (__cplusplus >= 201703L)    // C++17
		P_NODISCARD T* allocate(size_type count)
		{
			return p::Alloc<T>(*arena, count);
		}
		P_NODISCARD T* allocate(size_type count, const void*)
		{
			return allocate(count);
		}
#else
		P_NODISCARD pointer allocate(size_type count, const void* = 0)
		{
			return p::Alloc<T>(*arena, count);
		}
#endif
		constexpr void deallocate(T* p, size_type n)
		{
			p::Free<T>(*arena, p, n);
		}


#if ((__cplusplus >= 201103L) || (_MSC_VER > 1900))    // C++11
		using is_always_equal                        = std::true_type;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap            = std::true_type;
		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new (p) U(p::Forward<Args>(args)...);
		}
		template<typename U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}
#else
		void construct(pointer p, value_type const& val)
		{
			::new (p) value_type(val);
		}
		void destroy(pointer p)
		{
			p->~value_type();
		}
#endif

		size_type max_size() const noexcept
		{
			return (Limits<difference_type>::Max() / sizeof(value_type));
		}
		pointer address(reference x) const
		{
			return &x;
		}
		const_pointer address(const_reference x) const
		{
			return &x;
		}
	};

	template<typename T1, typename T2>
	bool operator==(const STLAllocator<T1>& a, const STLAllocator<T2>& b) noexcept
	{
		return &a.arena == &b.arena;
	}
	template<typename T1, typename T2>
	bool operator!=(const STLAllocator<T1>& a, const STLAllocator<T2>& b) noexcept
	{
		return &a.arena != &b.arena;
	}
#pragma endregion STL Allocator

}    // namespace p
