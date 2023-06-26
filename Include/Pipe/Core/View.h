// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Checks.h"

#include <iterator>


namespace p::core
{
	template<typename T>
	struct TView
	{
		using Iterator             = T*;
		using ConstIterator        = const T*;
		using ReverseIterator      = std::reverse_iterator<T*>;
		using ConstReverseIterator = std::reverse_iterator<const T*>;


		T* const data  = nullptr;
		const i32 size = 0;


		constexpr TView() {}
		constexpr TView(T& value) : data{&value}, size{1} {}
		constexpr TView(T* first, T* last) : data{first}, size{i32(std::distance(first, last))} {}
		constexpr TView(T* data, i32 size) : data{data}, size{size} {}

		template<sizet N>
		constexpr TView(T (&value)[N]) : data{value}, size{N}
		{}
		constexpr TView(std::initializer_list<T> value)
		    : data{value.begin()}, size{i32(value.size())}
		{}

		TView(const TArray<Mut<T>>& value) : data{value.Data()}, size{value.Size()} {}
		TView(const TArray<Mut<T>>& value, i32 firstN)
		    : data{value.Data()}, size{math::Min(value.Size(), firstN)}
		{}

		TView(const TView<T>& other) : data{other.data}, size{other.size} {}
		TView& operator=(const TView<T>& other)
		{
			data = other.data;
			size = other.size;
			return *this;
		}
		TView(const TView<Mut<T>>& other) requires(IsSame<T, const T>)
		    : data{other.data}, size{other.size}
		{}
		TView& operator=(const TView<Mut<T>>& other) requires(IsSame<T, const T>)
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

		template<typename OtherType>
		bool operator==(const TView<OtherType>& other) const
		{
			if (Size() != other.Size() || IsEmpty())
			{
				return false;
			}

			for (i32 i = 0; i < Size(); ++i)
			{
				if (Data()[i] != other[i])
				{
					return false;
				}
			}
			return true;
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
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
