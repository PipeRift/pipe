// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/STDFormat.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wcharacter-conversion"
#endif
#include "Pipe/Extern/utf8/unchecked.h"
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
#include "PipeContainers.h"
#include "PipeMemory.h"
#include "PipePlatform.h"
#include "PipeSerializeFwd.h"

#include <algorithm>
#include <string>


namespace p
{
	/**
	 * A std-independent dynamic string built on top of TArray.
	 * Mirrors the std::basic_string API.
	 * The internal array always contains a null terminator as its last element.
	 *
	 * @param CharType of the characters stored
	 * @param InlineCapacity of characters stored inside the object itself (SSO).
	 * Defaults to ~24 bytes worth of characters.
	 */
	template<typename CharType, u32 InlineCapacity = 24 / sizeof(CharType)>
	struct TString
	{
	public:
		using traits_type            = std::char_traits<CharType>;
		using value_type             = CharType;
		using size_type              = sizet;
		using difference_type        = ptrdiff_t;
		using reference              = CharType&;
		using const_reference        = const CharType&;
		using pointer                = CharType*;
		using const_pointer          = const CharType*;
		using iterator               = CharType*;
		using const_iterator         = const CharType*;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using View = TStringView<CharType>;

		static constexpr sizet npos = sizet(-1);


	private:
		/** Contains [chars..., '\0']. Size() is always Len() + 1 */
		TArray<CharType, InlineCapacity> chars;


	public:
		TString()
		{
			InitEmpty();
		}

		/** Arena constructors. Allocations are done on the given arena */
		explicit TString(Arena& arena)
		{
			chars.arena = &arena;
			InitEmpty();
		}
		TString(Arena& arena, size_type count, CharType ch)
		{
			chars.arena = &arena;
			AppendCount(count, ch);
		}
		explicit TString(Arena& arena, const CharType* str)
		{
			chars.arena = &arena;
			SetFrom(str, traits_type::length(str));
		}
		TString(Arena& arena, const CharType* str, size_type count)
		{
			chars.arena = &arena;
			SetFrom(str, count);
		}
		explicit TString(Arena& arena, View view)
		{
			chars.arena = &arena;
			SetFrom(view.data(), view.size());
		}
		TString(Arena& arena, const TString& other)
		{
			chars.arena = &arena;
			SetFrom(other.Buf(), other.Len());
		}

		TString(size_type count, CharType ch)
		{
			AppendCount(count, ch);
		}

		TString(const TString& other, size_type pos)
		{
			P_Check(pos <= other.Len());
			SetFrom(other.Buf() + pos, other.Len() - i32(pos));
		}
		TString(const TString& other, size_type pos, size_type count)
		{
			P_Check(pos <= other.Len());
			SetFrom(other.Buf() + pos, Min(count, other.Len() - pos));
		}

		TString(const CharType* str, size_type count)
		{
			SetFrom(str, count);
		}
		TString(const CharType* str)
		{
			SetFrom(str, traits_type::length(str));
		}

		template<typename It>
		TString(It first, It last)
		{
			AppendIter(first, last);
		}

		TString(std::initializer_list<CharType> initList)
		{
			AppendIter(initList.begin(), initList.end());
		}

		/** StringView-like constructor (not convertible to const CharType*) */
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		explicit TString(const SVLike& view) : TString{View(view)}
		{}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		explicit TString(const SVLike& view, size_type pos)
		{
			View v = view;
			P_Check(pos <= v.size());
			SetFrom(v.data() + pos, v.size() - pos);
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		explicit TString(const SVLike& view, size_type pos, size_type count)
		{
			View v = view;
			P_Check(pos <= v.size());
			SetFrom(v.data() + pos, Min(count, v.size() - pos));
		}
		explicit TString(View view)
		{
			SetFrom(view.data(), view.size());
		}

		TString(std::nullptr_t) = delete;

		TString(const TString& other) : chars{other.chars} {}
		TString(TString&& other) noexcept : chars{Move(other.chars)}
		{
			other.InitEmpty();
		}

		~TString() = default;


#pragma region Assignment
		TString& operator=(const TString& other)
		{
			if (this != &other)
			{
				SetFrom(other.Buf(), other.Len());
			}
			return *this;
		}
		TString& operator=(TString&& other) noexcept
		{
			if (this != &other)
			{
				chars = Move(other.chars);
				other.InitEmpty();
			}
			return *this;
		}
		TString& operator=(const CharType* str)
		{
			SetFrom(str, traits_type::length(str));
			return *this;
		}
		TString& operator=(CharType ch)
		{
			SetFrom(&ch, 1);
			return *this;
		}
		TString& operator=(std::nullptr_t) = delete;
		TString& operator=(std::initializer_list<CharType> initList)
		{
			clear();
			AppendIter(initList.begin(), initList.end());
			return *this;
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& operator=(const SVLike& view)
		{
			View v = view;
			SetFrom(v.data(), v.size());
			return *this;
		}

		TString& assign(const TString& other)
		{
			return *this = other;
		}
		TString& assign(const TString& other, size_type pos, size_type count = npos)
		{
			P_Check(pos <= other.Len());
			SetFrom(other.Buf() + pos, Min(count, other.Len() - pos));
			return *this;
		}
		TString& assign(TString&& other) noexcept
		{
			return *this = Move(other);
		}
		TString& assign(const CharType* str, size_type count)
		{
			SetFrom(str, count);
			return *this;
		}
		TString& assign(const CharType* str)
		{
			SetFrom(str, traits_type::length(str));
			return *this;
		}
		TString& assign(size_type count, CharType ch)
		{
			clear();
			AppendCount(count, ch);
			return *this;
		}
		template<typename It>
		TString& assign(It first, It last)
		{
			clear();
			AppendIter(first, last);
			return *this;
		}
		TString& assign(std::initializer_list<CharType> initList)
		{
			clear();
			AppendIter(initList.begin(), initList.end());
			return *this;
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& assign(const SVLike& view)
		{
			View v = view;
			SetFrom(v.data(), v.size());
			return *this;
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& assign(const SVLike& view, size_type pos, size_type count = npos)
		{
			View v = view;
			P_Check(pos <= v.size());
			SetFrom(v.data() + pos, Min(count, v.size() - pos));
			return *this;
		}
#pragma endregion Assignment


#pragma region Element access
		reference at(size_type pos)
		{
			P_Check(pos < Len());
			return Buf()[pos];
		}
		const_reference at(size_type pos) const
		{
			P_Check(pos < Len());
			return Buf()[pos];
		}

		reference operator[](size_type pos)
		{
			P_Check(pos <= Len());
			return Buf()[pos];
		}
		const_reference operator[](size_type pos) const
		{
			P_Check(pos <= Len());
			return Buf()[pos];
		}

		reference front()
		{
			P_Check(!IsEmpty());
			return Buf()[0];
		}
		const_reference front() const
		{
			P_Check(!IsEmpty());
			return Buf()[0];
		}

		reference back()
		{
			P_Check(!IsEmpty());
			return Buf()[Len() - 1];
		}
		const_reference back() const
		{
			P_Check(!IsEmpty());
			return Buf()[Len() - 1];
		}

		pointer data()
		{
			return Buf();
		}
		const_pointer data() const
		{
			return Buf();
		}
		const_pointer c_str() const
		{
			return Buf();
		}

		operator View() const
		{
			return {Buf(), size()};
		}
#pragma endregion Element access


#pragma region Iterators
		iterator begin()
		{
			return Buf();
		}
		const_iterator begin() const
		{
			return Buf();
		}
		const_iterator cbegin() const
		{
			return Buf();
		}

		iterator end()
		{
			return Buf() + Len();
		}
		const_iterator end() const
		{
			return Buf() + Len();
		}
		const_iterator cend() const
		{
			return Buf() + Len();
		}

		reverse_iterator rbegin()
		{
			return reverse_iterator{end()};
		}
		const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator{end()};
		}
		const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator{end()};
		}

		reverse_iterator rend()
		{
			return reverse_iterator{begin()};
		}
		const_reverse_iterator rend() const
		{
			return const_reverse_iterator{begin()};
		}
		const_reverse_iterator crend() const
		{
			return const_reverse_iterator{begin()};
		}
#pragma endregion Iterators


#pragma region Capacity
		bool empty() const
		{
			return IsEmpty();
		}
		size_type size() const
		{
			return sizet(Len());
		}
		size_type length() const
		{
			return sizet(Len());
		}
		size_type max_size() const
		{
			return sizet(INT32_MAX - 1);
		}

		void reserve(size_type newCapacity)
		{
			P_CheckMsg(newCapacity <= sizet(max_size()), "String capacity exceeds max_size");
			chars.Reserve(i32(newCapacity) + 1);
		}
		size_type capacity() const
		{
			return sizet(Max(1, chars.Capacity()) - 1);
		}

		void shrink_to_fit()
		{
			chars.Shrink(Len() + 1);
		}
#pragma endregion Capacity


#pragma region Modifiers
		void clear()
		{
			chars.Resize(1, Shrink::No);
			Buf()[0] = CharType{};
		}

		void push_back(CharType ch)
		{
			EnsureInit();
			const i32 len = Len();
			chars.AddUninitialized(1);
			Buf()[len]     = ch;
			Buf()[len + 1] = CharType{};
		}
		void pop_back()
		{
			P_Check(!IsEmpty());
			chars.RemoveLast(1, Shrink::No);
			Buf()[Len()] = CharType{};
		}

		TString& append(size_type count, CharType ch)
		{
			AppendCount(count, ch);
			return *this;
		}
		TString& append(const TString& other)
		{
			Append(other.Buf(), other.Len());
			return *this;
		}
		TString& append(const TString& other, size_type pos, size_type count = npos)
		{
			P_Check(pos <= other.Len());
			Append(other.Buf() + pos, Min(count, other.Len() - pos));
			return *this;
		}
		TString& append(const CharType* str, size_type count)
		{
			Append(str, count);
			return *this;
		}
		TString& append(const CharType* str)
		{
			Append(str, traits_type::length(str));
			return *this;
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& append(const SVLike& view)
		{
			View v = view;
			Append(v.data(), v.size());
			return *this;
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& append(const SVLike& view, size_type pos, size_type count = npos)
		{
			View v = view;
			P_Check(pos <= v.size());
			Append(v.data() + pos, Min(count, v.size() - pos));
			return *this;
		}
		template<typename It>
		TString& append(It first, It last)
		{
			AppendIter(first, last);
			return *this;
		}
		TString& append(std::initializer_list<CharType> initList)
		{
			AppendIter(initList.begin(), initList.end());
			return *this;
		}

		TString& operator+=(const TString& other)
		{
			return append(other);
		}
		TString& operator+=(const CharType* str)
		{
			return append(str);
		}
		TString& operator+=(CharType ch)
		{
			push_back(ch);
			return *this;
		}
		TString& operator+=(std::initializer_list<CharType> initList)
		{
			return append(initList);
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& operator+=(const SVLike& view)
		{
			return append(view);
		}

		TString& insert(size_type pos, size_type count, CharType ch)
		{
			P_Check(pos <= Len());
			P_CheckMsg(count <= sizet(max_size()), "String size exceeds max_size");
			const i32 num    = i32(count);
			const i32 oldLen = Len();
			chars.AddUninitialized(num);
			traits_type::move(Buf() + pos + num, Buf() + pos, i32(oldLen - pos) + 1);
			traits_type::assign(Buf() + pos, num, ch);
			return *this;
		}
		TString& insert(size_type pos, const CharType* str)
		{
			return insert(pos, str, traits_type::length(str));
		}
		TString& insert(size_type pos, const CharType* str, size_type count)
		{
			P_Check(pos <= Len());
			P_CheckMsg(count <= sizet(max_size()), "String size exceeds max_size");
			const i32 num            = i32(count);
			TArray<CharType, 0> temp = ResolveAliased(str, num);
			const i32 oldLen         = Len();
			chars.AddUninitialized(num);
			traits_type::move(Buf() + pos + num, Buf() + pos, i32(oldLen - pos) + 1);
			traits_type::copy(Buf() + pos, str, num);
			return *this;
		}
		TString& insert(size_type pos, const TString& other)
		{
			return insert(pos, other.Buf(), other.Len());
		}
		TString& insert(
		    size_type pos, const TString& other, size_type otherPos, size_type count = npos)
		{
			P_Check(otherPos <= other.Len());
			return insert(pos, other.Buf() + otherPos, Min(count, other.Len() - otherPos));
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& insert(size_type pos, const SVLike& view)
		{
			View v = view;
			return insert(pos, v.data(), v.size());
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& insert(
		    size_type pos, const SVLike& view, size_type otherPos, size_type count = npos)
		{
			View v = view;
			P_Check(otherPos <= v.size());
			return insert(pos, v.data() + otherPos, Min(count, v.size() - otherPos));
		}

		iterator insert(const_iterator pos, CharType ch)
		{
			const i32 index = i32(pos - Buf());
			insert(sizet(index), 1, ch);
			return Buf() + index;
		}
		/** Integral first parameters are excluded so that insert(0, 3, '=') is not ambiguous */
		template<typename T>
		requires(!std::is_integral_v<T>)
		iterator insert(T pos, size_type count, CharType ch)
		{
			const i32 index = i32(pos - Buf());
			insert(sizet(index), count, ch);
			return Buf() + index;
		}
		template<typename It>
		iterator insert(const_iterator pos, It first, It last)
		{
			if constexpr (requires(It a, It b) { a - b; })    // Random access (or pointer)
			{
				const i32 index = i32(pos - Buf());
				const i32 count = i32(last - first);
				if (count > 0)
				{
					const i32 oldLen = Len();
					chars.AddUninitialized(count);
					traits_type::move(
					    Buf() + index + count, Buf() + index, i32(oldLen - index) + 1);
					CharType* dest = Buf() + index;
					for (It it = first; it != last; ++it, ++dest)
					{
						*dest = *it;
					}
				}
				return Buf() + index;
			}
			else    // Single pass iterators (input iterators)
			{
				const i32 index = i32(pos - Buf());
				TString temp(first, last);
				insert(sizet(index), temp.Buf(), temp.Len());
				return Buf() + index;
			}
		}
		iterator insert(const_iterator pos, std::initializer_list<CharType> initList)
		{
			return insert(pos, initList.begin(), initList.end());
		}

		TString& erase(size_type pos = 0, size_type count = npos)
		{
			P_Check(pos <= Len());
			const i32 len = Len();
			const i32 num = i32(Min(count, len - pos));
			if (num > 0)
			{
				traits_type::move(
				    Buf() + pos, Buf() + pos + num, i32(len - pos - num) + 1);    // + terminator
				chars.RemoveLast(num, Shrink::No);
				Buf()[Len()] = CharType{};
			}
			return *this;
		}
		iterator erase(const_iterator pos)
		{
			P_Check(pos >= begin() && pos < end());
			const i32 index = i32(pos - Buf());
			erase(sizet(index), 1);
			return Buf() + index;
		}
		iterator erase(const_iterator first, const_iterator last)
		{
			P_Check(first >= begin() && first <= end() && last >= first && last <= end());
			const i32 index = i32(first - Buf());
			erase(sizet(index), sizet(last - first));
			return Buf() + index;
		}

		TString& replace(size_type pos, size_type count, const TString& other)
		{
			return ReplaceRange(pos, count, other.Buf(), other.Len());
		}
		TString& replace(size_type pos, size_type count, const TString& other, size_type otherPos,
		    size_type otherCount = npos)
		{
			P_Check(otherPos <= other.Len());
			return ReplaceRange(
			    pos, count, other.Buf() + otherPos, i32(Min(otherCount, other.Len() - otherPos)));
		}
		TString& replace(size_type pos, size_type count, const CharType* str)
		{
			return ReplaceRange(pos, count, str, traits_type::length(str));
		}
		TString& replace(size_type pos, size_type count, const CharType* str, size_type strCount)
		{
			return ReplaceRange(pos, count, str, strCount);
		}
		TString& replace(size_type pos, size_type count, size_type num, CharType ch)
		{
			P_Check(pos <= Len());
			const TString temp(num, ch);
			return ReplaceRange(pos, count, temp.Buf(), temp.Len());
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& replace(size_type pos, size_type count, const SVLike& view)
		{
			View v = view;
			return ReplaceRange(pos, count, v.data(), v.size());
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& replace(size_type pos, size_type count, const SVLike& view, size_type otherPos,
		    size_type otherCount = npos)
		{
			View v = view;
			P_Check(otherPos <= v.size());
			return ReplaceRange(
			    pos, count, v.data() + otherPos, Min(otherCount, v.size() - otherPos));
		}
		TString& replace(const_iterator first, const_iterator last, const TString& other)
		{
			return ReplaceRange(first - Buf(), last - first, other.Buf(), other.Len());
		}
		TString& replace(const_iterator first, const_iterator last, const CharType* str)
		{
			return ReplaceRange(first - Buf(), last - first, str, traits_type::length(str));
		}
		TString& replace(
		    const_iterator first, const_iterator last, const CharType* str, size_type count)
		{
			return ReplaceRange(first - Buf(), last - first, str, count);
		}
		TString& replace(const_iterator first, const_iterator last, size_type num, CharType ch)
		{
			const TString temp(num, ch);
			return ReplaceRange(first - Buf(), last - first, temp.Buf(), temp.Len());
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		TString& replace(const_iterator first, const_iterator last, const SVLike& view)
		{
			View v = view;
			return ReplaceRange(first - Buf(), last - first, v.data(), v.size());
		}
		template<typename It>
		TString& replace(const_iterator first, const_iterator last, It first2, It last2)
		{
			const i32 pos   = i32(first - Buf());
			const i32 count = i32(last - first);
			const TString temp(first2, last2);
			return ReplaceRange(sizet(pos), sizet(count), temp.Buf(), temp.Len());
		}

		void resize(size_type newSize, CharType ch = CharType{})
		{
			P_CheckMsg(newSize <= sizet(max_size()), "String size exceeds max_size");
			const i32 num = i32(newSize);
			const i32 len = Len();
			if (num < len)
			{
				chars.RemoveLast(len - num, Shrink::No);
				Buf()[num] = CharType{};
			}
			else if (num > len)
			{
				append(num - len, ch);
			}
		}

		void swap(TString& other)
		{
			p::Swap(chars, other.chars);
		}

		friend void swap(TString& a, TString& b)
		{
			a.swap(b);
		}
#pragma endregion Modifiers


#pragma region String operations
		TString substr(size_type pos = 0, size_type count = npos) const
		{
			P_Check(pos <= Len());
			return TString{*this, pos, count};
		}

		size_type copy(CharType* dest, size_type count, size_type pos = 0) const
		{
			P_Check(pos <= Len());
			const i32 num = i32(Min(count, Len() - pos));
			traits_type::copy(dest, Buf() + pos, num);
			return sizet(num);
		}

		i32 compare(const TString& other) const
		{
			return AsView().compare(other.AsView());
		}
		i32 compare(size_type pos, size_type count, const TString& other) const
		{
			return substr(pos, count).compare(other);
		}
		i32 compare(size_type pos, size_type count, const TString& other, size_type otherPos,
		    size_type otherCount = npos) const
		{
			return substr(pos, count).compare(other.substr(otherPos, otherCount));
		}
		i32 compare(const CharType* str) const
		{
			return AsView().compare(View{str});
		}
		i32 compare(size_type pos, size_type count, const CharType* str) const
		{
			return substr(pos, count).compare(View{str});
		}
		i32 compare(size_type pos, size_type count, const CharType* str, size_type strCount) const
		{
			return substr(pos, count).compare(View{str, strCount});
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		i32 compare(const SVLike& view) const
		{
			return AsView().compare(view);
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		i32 compare(size_type pos, size_type count, const SVLike& view) const
		{
			return substr(pos, count).compare(view);
		}
		template<typename SVLike>
		requires(std::convertible_to<const SVLike&, View>
		         && !std::convertible_to<const SVLike&, const CharType*>)
		i32 compare(size_type pos, size_type count, const SVLike& view, size_type otherPos,
		    size_type otherCount = npos) const
		{
			View v = view;
			P_Check(otherPos <= v.size());
			return substr(pos, count)
			    .compare(v.substr(otherPos, Min(otherCount, v.size() - otherPos)));
		}

		bool starts_with(View view) const
		{
			return Strings::StartsWith(AsView(), view);
		}
		bool starts_with(CharType ch) const
		{
			return !IsEmpty() && front() == ch;
		}
		bool starts_with(const CharType* str) const
		{
			return starts_with(View{str});
		}

		bool ends_with(View view) const
		{
			return Strings::EndsWith(AsView(), view);
		}
		bool ends_with(CharType ch) const
		{
			return !IsEmpty() && back() == ch;
		}
		bool ends_with(const CharType* str) const
		{
			return ends_with(View{str});
		}

		bool contains(View view) const
		{
			return find(view) != npos;
		}
		bool contains(CharType ch) const
		{
			return traits_type::find(Buf(), Len(), ch) != nullptr;
		}
		bool contains(const CharType* str) const
		{
			return contains(View{str});
		}

		size_type find(View view, size_type pos = 0) const
		{
			return AsView().find(view, pos);
		}
		size_type find(CharType ch, size_type pos = 0) const
		{
			return AsView().find(ch, pos);
		}
		size_type find(const CharType* str, size_type pos = 0) const
		{
			return AsView().find(View{str}, pos);
		}
		size_type find(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().find(View{str, count}, pos);
		}

		size_type rfind(View view, size_type pos = npos) const
		{
			return AsView().rfind(view, pos);
		}
		size_type rfind(CharType ch, size_type pos = npos) const
		{
			return AsView().rfind(ch, pos);
		}
		size_type rfind(const CharType* str, size_type pos = npos) const
		{
			return AsView().rfind(View{str}, pos);
		}
		size_type rfind(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().rfind(View{str, count}, pos);
		}

		size_type find_first_of(View view, size_type pos = 0) const
		{
			return AsView().find_first_of(view, pos);
		}
		size_type find_first_of(CharType ch, size_type pos = 0) const
		{
			return AsView().find_first_of(ch, pos);
		}
		size_type find_first_of(const CharType* str, size_type pos = 0) const
		{
			return AsView().find_first_of(View{str}, pos);
		}
		size_type find_first_of(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().find_first_of(View{str, count}, pos);
		}

		size_type find_last_of(View view, size_type pos = npos) const
		{
			return AsView().find_last_of(view, pos);
		}
		size_type find_last_of(CharType ch, size_type pos = npos) const
		{
			return AsView().find_last_of(ch, pos);
		}
		size_type find_last_of(const CharType* str, size_type pos = npos) const
		{
			return AsView().find_last_of(View{str}, pos);
		}
		size_type find_last_of(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().find_last_of(View{str, count}, pos);
		}

		size_type find_first_not_of(View view, size_type pos = 0) const
		{
			return AsView().find_first_not_of(view, pos);
		}
		size_type find_first_not_of(CharType ch, size_type pos = 0) const
		{
			return AsView().find_first_not_of(ch, pos);
		}
		size_type find_first_not_of(const CharType* str, size_type pos = 0) const
		{
			return AsView().find_first_not_of(View{str}, pos);
		}
		size_type find_first_not_of(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().find_first_not_of(View{str, count}, pos);
		}

		size_type find_last_not_of(View view, size_type pos = npos) const
		{
			return AsView().find_last_not_of(view, pos);
		}
		size_type find_last_not_of(CharType ch, size_type pos = npos) const
		{
			return AsView().find_last_not_of(ch, pos);
		}
		size_type find_last_not_of(const CharType* str, size_type pos = npos) const
		{
			return AsView().find_last_not_of(View{str}, pos);
		}
		size_type find_last_not_of(const CharType* str, size_type pos, size_type count) const
		{
			return AsView().find_last_not_of(View{str, count}, pos);
		}
#pragma endregion String operations


		View AsView() const
		{
			return {Buf(), size()};
		}

		/** @return the arena this string allocates from */
		Arena& GetArena() const
		{
			return *chars.arena;
		}

#pragma region Operators
		friend bool operator==(const TString& a, const TString& b)
		{
			return a.size() == b.size() && traits_type::compare(a.Buf(), b.Buf(), a.Len()) == 0;
		}
		friend auto operator<=>(const TString& a, const TString& b)
		{
			const i32 result = traits_type::compare(a.Buf(), b.Buf(), Min(a.Len(), b.Len()) + 1);
			if (result != 0) [[likely]]
			{
				return result <=> 0;
			}
			return a.Len() <=> b.Len();
		}

		friend bool operator==(const TString& a, const CharType* str)
		{
			return a.AsView() == View{str};
		}
		friend auto operator<=>(const TString& a, const CharType* str)
		{
			return a.AsView() <=> View{str};
		}

		friend bool operator==(const TString& a, View b)
		{
			return a.AsView() == b;
		}
		friend auto operator<=>(const TString& a, View b)
		{
			return a.AsView() <=> b;
		}

		friend TString operator+(const TString& a, const TString& b)
		{
			TString result{a};
			result += b;
			return result;
		}
		friend TString operator+(const TString& a, const CharType* str)
		{
			TString result{a};
			result += str;
			return result;
		}
		friend TString operator+(const CharType* str, const TString& b)
		{
			TString result;
			result.reserve(traits_type::length(str) + b.size());
			result += str;
			result += b;
			return result;
		}
		friend TString operator+(const TString& a, CharType ch)
		{
			TString result{a};
			result += ch;
			return result;
		}
		friend TString operator+(CharType ch, const TString& b)
		{
			TString result;
			result.reserve(b.size() + 1);
			result += ch;
			result += b;
			return result;
		}
		friend TString operator+(const TString& a, View b)
		{
			TString result{a};
			result += b;
			return result;
		}
		friend TString operator+(View a, const TString& b)
		{
			TString result;
			result.reserve(a.size() + b.size());
			result += a;
			result += b;
			return result;
		}

		friend TString operator+(TString&& a, const TString& b)
		{
			a += b;
			return Move(a);
		}
		friend TString operator+(TString&& a, const CharType* str)
		{
			a += str;
			return Move(a);
		}
		friend TString operator+(TString&& a, CharType ch)
		{
			a += ch;
			return Move(a);
		}
		friend TString operator+(TString&& a, View b)
		{
			a += b;
			return Move(a);
		}
		friend TString operator+(const CharType* str, TString&& b)
		{
			b.insert(sizet(0), str);
			return Move(b);
		}
		friend TString operator+(CharType ch, TString&& b)
		{
			b.insert(b.begin(), ch);
			return Move(b);
		}
		friend TString operator+(View a, TString&& b)
		{
			b.insert(sizet(0), a);
			return Move(b);
		}
#pragma endregion Operators


	private:
		constexpr i32 Len() const
		{
			return chars.Size() - 1;
		}
		constexpr bool IsEmpty() const
		{
			return chars.Size() <= 1;
		}

		constexpr CharType* Buf()
		{
			return chars.Data();
		}
		constexpr const CharType* Buf() const
		{
			return chars.Data();
		}

		/** Ensures the string contains its terminator. Allows safe Len() on fresh objects */
		void EnsureInit()
		{
			if (chars.Size() == 0)
			{
				chars.Add(CharType{});
			}
		}

		/** Ensures the string starts with its terminator, empty */
		void InitEmpty()
		{
			chars.Clear(Shrink::No);
			chars.Add(CharType{});
		}

		/**
		 * Self-referential sources must be copied out before any buffer reallocation.
		 * Replaces the source with a pointer to a safe temporary buffer if it aliases this string.
		 * @return the temporary buffer, kept alive by the caller until the source is consumed
		 */
		TArray<CharType, 0> ResolveAliased(const CharType*& str, i32 count)
		{
			TArray<CharType, 0> temp;
			if (count > 0 && str && chars.Size() > 0 && str >= Buf() && str < Buf() + chars.Size())
			{
				temp.arena = chars.arena;
				temp.AddUninitialized(count);
				traits_type::copy(temp.Data(), str, count);
				str = temp.Data();
			}
			return temp;
		}

		void SetFrom(const CharType* str, size_type count)
		{
			P_CheckMsg(count <= sizet(max_size()), "String size exceeds max_size");
			const i32 num = i32(count);
			if (str && num > 0)
			{
				TArray<CharType, 0> temp = ResolveAliased(str, num);
				chars.Clear(Shrink::No);
				chars.Reserve(num + 1);
				chars.AddUninitialized(num + 1);
				traits_type::copy(Buf(), str, num);
				Buf()[num] = CharType{};
			}
			else
			{
				clear();
			}
		}

		void Append(const CharType* str, size_type count)
		{
			P_CheckMsg(count <= sizet(max_size()), "String size exceeds max_size");
			const i32 num = i32(count);
			if (str && num > 0)
			{
				TArray<CharType, 0> temp = ResolveAliased(str, num);
				EnsureInit();
				const i32 len = Len();
				chars.AddUninitialized(num);
				traits_type::copy(Buf() + len, str, num);
				Buf()[len + num] = CharType{};
			}
		}

		void AppendCount(size_type count, CharType ch)
		{
			P_CheckMsg(count <= sizet(max_size()), "String size exceeds max_size");
			const i32 num = i32(count);
			if (num > 0)
			{
				EnsureInit();
				const i32 len = Len();
				chars.AddUninitialized(num);
				traits_type::assign(Buf() + len, num, ch);
				Buf()[len + num] = CharType{};
			}
		}

		template<typename It>
		void AppendIter(It first, It last)
		{
			if constexpr (requires(It a, It b) { a - b; })    // Random access (or pointer)
			{
				const i32 num = i32(last - first);
				if (num > 0)
				{
					EnsureInit();
					const i32 len = Len();
					chars.AddUninitialized(num);
					CharType* dest = Buf() + len;
					for (It it = first; it != last; ++it, ++dest)
					{
						*dest = *it;
					}
					Buf()[len + num] = CharType{};
				}
			}
			else    // Single pass iterators (input iterators)
			{
				for (; first != last; ++first)
				{
					push_back(*first);
				}
			}
		}

		TString& ReplaceRange(size_type pos, size_type count, const CharType* str, size_type strNum)
		{
			P_Check(pos <= Len());
			P_CheckMsg(strNum <= sizet(max_size()), "String size exceeds max_size");
			const i32 n              = i32(strNum);
			TArray<CharType, 0> temp = ResolveAliased(str, n);
			const i32 len            = Len();
			const i32 num            = i32(Min(count, len - pos));
			const i32 tail           = len - i32(pos) - num;
			if (n > num)    // Grow first so we don't overlap while moving
			{
				chars.AddUninitialized(n - num);
				traits_type::move(Buf() + pos + n, Buf() + pos + num, tail + 1);    // + terminator
			}
			else if (n < num)
			{
				traits_type::move(Buf() + pos + n, Buf() + pos + num, tail + 1);
				chars.RemoveLast(num - n, Shrink::No);
				Buf()[Len()] = CharType{};
			}
			traits_type::move(Buf() + pos, str, n);
			return *this;
		}
	};


	using String  = TString<char>;
	using WString = TString<WideChar>;

	template<typename... Args>
	using FormatString = std::basic_format_string<char, Args...>;

	template<typename StringType = String, typename... Args>
	inline StringType Format(StringView format, Args... args)
	{
		StringType str;
		std::vformat_to(std::back_inserter(str), format, std::make_format_args(args...));
		return Move(str);
	}

	template<typename StringType, typename... Args>
	inline void FormatTo(StringType& buffer, StringView format, Args... args)
	{
		std::vformat_to(std::back_inserter(buffer), format, std::make_format_args(args...));
	}

	// Format a string using a compile time format
	template<typename StringType = String, typename... Args>
	inline StringType Format(FormatString<Args...> format, Args... args)
	{
		StringType str;
		std::format_to(std::back_inserter(str), format, p::Fwd<Args>(args)...);
		return Move(str);
	}

	// Format into an existing string using a compile time format
	template<typename StringType, typename... Args>
	inline void FormatTo(StringType& buffer, FormatString<Args...> format, Args... args)
	{
		std::format_to(std::back_inserter(buffer), format, p::Fwd<Args>(args)...);
	}

	namespace Strings
	{

		template<typename StringType, typename T>
		inline void ToString(StringType& buffer, T value, FormatString<T> format = "{}")
		{
			std::format_to(std::back_inserter(buffer), format, p::Fwd<T>(value));
		}

		template<typename StringType, typename T>
		inline StringType ToString(T value)
		{
			StringType str;
			ToString(str, value);
			return str;
		}

		P_API String ToSentenceCase(StringView value);

		/**
		 * Breaks up a delimited string into elements of a string array.
		 *
		 * @param	InArray		The array to fill with the string pieces
		 * @param	pchDelim	The string to delimit on
		 * @param	InCullEmpty	If 1, empty strings are not added to the array
		 *
		 * @return	The number of elements in InArray
		 */
		P_API i32 ParseIntoArray(const String& str, TArray<String>& OutArray, const char* pchDelim,
		    bool InCullEmpty = true);

		P_API void RemoveFromStart(String& str, sizet size);
		P_API void RemoveFromEnd(String& str, sizet size);
		P_API void RemoveFromEnd(String& str, StringView subStr);

		P_API bool RemoveCharFromEnd(String& str, char c);

		P_API i32 Split(const String& str, TArray<String>& tokens, const char delim);

		P_API bool Split(const String& str, String& a, String& b, const char* delim);

		P_API bool IsNumeric(const String& str);
		P_API bool IsNumeric(const char* str);

		P_API String ParseMemorySize(sizet size, bool asBits = false);
		P_API void ParseMemorySizeTo(String& str, sizet size, bool asBits = false);

		template<typename ToStringType, typename FromChar>
		inline void ConvertTo(TStringView<FromChar> source, ToStringType& dest)
		{
			using ToChar = typename ToStringType::value_type;
			static_assert(
			    std::is_integral_v<FromChar>, "FromChar is not integral (so it is not a char)");
			static_assert(
			    std::is_integral_v<ToChar>, "ToChar is not integral (so it is not a char)");

			if constexpr (IsSame<FromChar, ToChar>)
			{
				dest += source;
			}
			else if constexpr (sizeof(FromChar) == 1 && sizeof(ToChar) == 2)
			{
				utf8::unchecked::utf8to16(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 2 && sizeof(ToChar) == 1)
			{
				utf8::unchecked::utf16to8(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 1 && sizeof(ToChar) == 4)
			{
				utf8::unchecked::utf8to32(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 4 && sizeof(ToChar) == 1)
			{
				utf8::unchecked::utf32to8(source.begin(), source.end(), std::back_inserter(dest));
			}
			else
			{
				// TODO: Find a way to assert at compile time except on the previous cases
				// static_assert(false, "Unknown char conversion");
			}
		}

		template<typename ToStringType, typename FromChar>
		inline ToStringType Convert(TStringView<FromChar> source)
		{
			ToStringType dest;
			ConvertTo(source, dest);
			return Move(dest);
		}
		template<typename ToStringType, typename FromChar>
		inline ToStringType Convert(const TString<FromChar>& source)
		{
			ToStringType dest;
			ConvertTo(TStringView<FromChar>{source}, dest);
			return Move(dest);
		}
	};    // namespace Strings


	inline sizet GetHash(const String& str)
	{
		return GetStringHash(str.data());
	}

	template<typename CharType, u32 InlineCapacity>
	inline std::basic_ostream<CharType, std::char_traits<CharType>>& operator<<(
	    std::basic_ostream<CharType, std::char_traits<CharType>>& os,
	    const TString<CharType, InlineCapacity>& str)
	{
		os << str.AsView();
		return os;
	}
}    // namespace p

template<typename CharType, p::u32 InlineCapacity>
struct std::formatter<p::TString<CharType, InlineCapacity>, CharType>
    : std::formatter<std::basic_string_view<CharType>, CharType>
{
	template<typename FormatContext>
	auto format(const p::TString<CharType, InlineCapacity>& str, FormatContext& ctx) const
	{
		return std::formatter<std::basic_string_view<CharType>, CharType>::format(
		    std::basic_string_view<CharType>{str.data(), str.size()}, ctx);
	}
};

template<typename CharType, p::u32 InlineCapacity>
struct std::hash<p::TString<CharType, InlineCapacity>>
{
	size_t operator()(const p::TString<CharType, InlineCapacity>& str) const noexcept
	{
		return p::GetStringHash(str.data());
	}
};
