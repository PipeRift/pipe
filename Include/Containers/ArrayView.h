// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"


namespace Rift
{
	template<typename T>
	struct TArrayView
	{
		using Iterator             = T*;
		using ConstIterator        = const T*;
		using ReverseIterator      = T*;
		using ConstReverseIterator = const T*;


		T* const data  = nullptr;
		const i32 size = 0;

		constexpr TArrayView() {}
		constexpr TArrayView(T& value) : data{&value}, size{1} {}
		constexpr TArrayView(T* first, T* last) : data{first}, size{i32(std::distance(first, last))}
		{}
		TArrayView(TArray<T>& value) : data{value.Data()}, size{value.Size()} {}
		TArrayView(const TArrayView<T>& other) : data{other.data}, size{other.size} {}
		TArrayView& operator=(const TArrayView<T>& other)
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
	struct TArrayView<const T>
	{
		using Iterator             = T*;
		using ConstIterator        = const T*;
		using ReverseIterator      = T*;
		using ConstReverseIterator = const T*;


		const T* const data = nullptr;
		const i32 size      = 0;

		constexpr TArrayView() {}
		constexpr TArrayView(const T& value) : data{&value}, size{1} {}
		TArrayView(const TArray<T>& value) : data{value.Data()}, size{value.Size()} {}
		TArrayView(const TArrayView<T>& other) : data{other.data}, size{other.size} {}
		TArrayView& operator=(const TArrayView<T>& other)
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
