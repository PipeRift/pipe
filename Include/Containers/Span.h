// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Misc/Checks.h"


namespace Rift
{
	template<typename T>
	struct TSpan
	{
		using Iterator             = T*;
		using ConstIterator        = const T*;
		using ReverseIterator      = T*;
		using ConstReverseIterator = const T*;


		T* const data  = nullptr;
		const i32 size = 0;

		constexpr TSpan() {}
		constexpr TSpan(T& value) : data{&value}, size{1} {}
		constexpr TSpan(T* first, T* last) : data{first}, size{i32(std::distance(first, last))} {}

		TSpan(TArray<T>& value) : data{value.Data()}, size{value.Size()} {}
		TSpan(TArray<Mut<T>>& value) requires(IsSame<T, const T>)
		    : data{value.Data()}, size{value.Size()}
		{}
		TSpan(TArray<T>& value, i32 firstN)
		    : data{value.Data()}, size{Math::Min(value.Size(), firstN)}
		{}
		TSpan(TArray<Mut<T>>& value, i32 firstN) requires(IsSame<T, const T>)
		    : data{value.Data()}, size{Math::Min(value.Size(), firstN)}
		{}

		TSpan(const TSpan<T>& other) : data{other.data}, size{other.size} {}
		TSpan& operator=(const TSpan<T>& other)
		{
			data = other.data;
			size = other.size;
			return *this;
		}
		TSpan(const TSpan<Mut<T>>& other) requires(IsSame<T, const T>)
		    : data{other.data}, size{other.size}
		{}
		TSpan& operator=(const TSpan<Mut<T>>& other) requires(IsSame<T, const T>)
		{
			data = other.data;
			size = other.size;
			return *this;
		}


		T& operator[](i32 index)
		{
			Ensure(IsValidIndex(index));
			return data[index];
		}

		const T& operator[](i32 index) const
		{
			Ensure(IsValidIndex(index));
			return data[index];
		}

		T* Data() const
		{
			return data;
		}

		i32 Size() const
		{
			return size;
		}

		bool IsEmpty() const
		{
			return Size() == 0;
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}

		bool Contains(const T& key) const
		{
			for (i32 i = 0; i < size; ++i)
			{
				if (data[i] == key)
				{
					return true;
				}
			}
			return false;
		}

		template<typename Callback>
		void Each(Callback cb) const
		{
			for (i32 i = 0; i < size; ++i)
			{
				cb(data[i]);
			}
		}

		template<typename Callback>
		void EachReverse(Callback cb) const
		{
			for (i32 i = size - 1; i >= 0; --i)
			{
				cb(data[i]);
			}
		}

		Iterator begin()
		{
			return data;
		};
		ConstIterator begin() const
		{
			return data;
		};
		ConstIterator cbegin() const
		{
			return begin();
		};

		Iterator end()
		{
			return data + size;
		};
		ConstIterator end() const
		{
			return data + size;
		};
		ConstIterator cend() const
		{
			return end();
		};

		ReverseIterator rbegin()
		{
			return ReverseIterator(end());
		};
		ConstReverseIterator rbegin() const
		{
			return ConstReverseIterator(end());
		};
		ConstReverseIterator crbegin() const
		{
			return rbegin();
		};

		ReverseIterator rend()
		{
			return ReverseIterator(begin());
		};
		ConstReverseIterator rend() const
		{
			return ConstReverseIterator(begin());
		};
		ConstReverseIterator crend() const
		{
			return rend();
		};
	};

	template<typename T>
	struct TSpan<const T>
	{
		using Iterator             = T*;
		using ConstIterator        = const T*;
		using ReverseIterator      = T*;
		using ConstReverseIterator = const T*;


		const T* const data = nullptr;
		const i32 size      = 0;

		constexpr TSpan() {}
		constexpr TSpan(const T& value) : data{&value}, size{1} {}
		TSpan(const TArray<T>& value) : data{value.Data()}, size{value.Size()} {}
		TSpan(const TSpan<T>& other) : data{other.data}, size{other.size} {}
		TSpan& operator=(const TSpan<T>& other)
		{
			data = other.data;
			size = other.size;
			return *this;
		}

		const T& operator[](i32 index) const
		{
			Ensure(IsValidIndex(index));
			return data[index];
		}

		const T* Data() const
		{
			return data;
		}

		i32 Size() const
		{
			return size;
		}

		bool IsEmpty() const
		{
			return Size() == 0;
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}

		bool Contains(const T& key) const
		{
			for (i32 i = 0; i < size; ++i)
			{
				if (data[i] == key)
				{
					return true;
				}
			}
			return false;
		}

		template<typename Callback>
		void Each(Callback cb) const
		{
			for (i32 i = 0; i < size; ++i)
			{
				cb(data[i]);
			}
		}

		template<typename Callback>
		void EachReverse(Callback cb) const
		{
			for (i32 i = size - 1; i >= 0; --i)
			{
				cb(data[i]);
			}
		}

		ConstIterator begin() const
		{
			return data;
		};
		ConstIterator cbegin() const
		{
			return begin();
		};

		ConstIterator end() const
		{
			return data + size;
		};
		ConstIterator cend() const
		{
			return end();
		};

		ConstReverseIterator rbegin() const
		{
			return ConstReverseIterator(end());
		};
		ConstReverseIterator crbegin() const
		{
			return rbegin();
		};

		ConstReverseIterator rend() const
		{
			return ConstReverseIterator(begin());
		};
		ConstReverseIterator crend() const
		{
			return rend();
		};
	};
}    // namespace Rift
