// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "PCore/StringView.h"
#include "PMath/Math.h"

#include <array>
#include <iterator>
#include <ostream>


#define CPP20_SPACESHIP_OPERATOR_PRESENT __cpp_lib_three_way_comparison

#define CPP20_CONSTEXPR_ALGORITHMS_PRESENT (__cpp_lib_constexpr_algorithms)


namespace p::core
{
	namespace Details
	{
		template<typename InputIterator, typename OutputIterator>
		constexpr OutputIterator copy(
		    InputIterator first, InputIterator last, OutputIterator d_first)
		{
#if CPP20_CONSTEXPR_ALGORITHMS_PRESENT
			return std::copy(first, last, d_first);
#else
			while (first != last)
			{
				*d_first++ = *first++;
			}
			return d_first;
#endif    // CPP20_CONSTEXPR_ALGORITHMS_PRESENT
		}

		template<typename ForwardIterator, typename T>
		constexpr void fill(ForwardIterator first, ForwardIterator last, const T& value)
		{
#if CPP20_CONSTEXPR_ALGORITHMS_PRESENT
			std::fill(first, last, value);
#else
			for (; first != last; ++first)
			{
				*first = value;
			}
#endif    // CPP20_CONSTEXPR_ALGORITHMS_PRESENT
		}

	}    // namespace Details

	template<sizet N, typename CharType = TChar, typename TTraits = std::char_traits<CharType>>
	struct TFixedString
	{
		// exposition only
		using storage_type = std::array<CharType, N + 1>;
		storage_type _data{};
		size_t _size = N;

		using traits_type            = TTraits;
		using value_type             = CharType;
		using pointer                = value_type*;
		using const_pointer          = const value_type*;
		using reference              = value_type&;
		using const_reference        = const value_type&;
		using iterator               = typename storage_type::iterator;
		using const_iterator         = typename storage_type::const_iterator;
		using reverse_iterator       = typename storage_type::reverse_iterator;
		using const_reverse_iterator = typename storage_type::const_reverse_iterator;
		using sizetype               = sizet;
		using difference_type        = ptrdiff_t;
		using string_view_type       = std::basic_string_view<value_type, traits_type>;
		static constexpr auto npos   = string_view_type::npos;

		constexpr TFixedString() noexcept
		{
			Details::fill(_data.begin(), _data.end(), static_cast<value_type>(0));
		}

		constexpr TFixedString(
		    const value_type (&array)[N + 1]) noexcept    // NOLINT(google-explicit-constructor)
		{
			Details::copy(std::begin(array), std::end(array), _data.begin());
		}

		constexpr TFixedString(TFixedString const& other) noexcept
		{
			Details::copy(other._data.begin(), other._data.end(), _data.begin());
			_size = other._size;
		}

		constexpr TFixedString(TStringView<value_type> other) noexcept
		{
			_size = math::Min(other.size(), N);
			Details::copy(other.begin(), other.begin() + _size, _data.begin());
		}

		constexpr TFixedString& operator=(const TFixedString& other) noexcept
		{
			Details::copy(other._data.begin(), other._data.end(), _data.begin());
			_size = N;
			return *this;
		}

		template<sizet M>
		constexpr TFixedString& operator=(const value_type (&array)[M]) noexcept requires(M <= N)
		{
			Details::copy(std::begin(array), std::end(array), _data.begin());
			_size = M;
			return *this;
		}

		// iterators
		[[nodiscard]] constexpr iterator begin() noexcept
		{
			return _data.begin();
		}
		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return _data.begin();
		}
		[[nodiscard]] constexpr iterator end() noexcept
		{
			return _data.end() - 1;
		}
		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return _data.begin() + _size;
		}
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return _data.cbegin();
		}
		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return _data.cend() - 1;
		}
		[[nodiscard]] constexpr iterator rbegin() noexcept
		{
			return _data.rbegin() + 1;
		}
		[[nodiscard]] constexpr const_iterator rbegin() const noexcept
		{
			return _data.rbegin() + 1;
		}
		[[nodiscard]] constexpr iterator rend() noexcept
		{
			return _data.rend();
		}
		[[nodiscard]] constexpr const_iterator rend() const noexcept
		{
			return _data.rend();
		}
		[[nodiscard]] constexpr const_iterator crbegin() const noexcept
		{
			return _data.crbegin() + 1;
		}
		[[nodiscard]] constexpr const_iterator crend() const noexcept
		{
			return _data.crend() + 1;
		}

		// capacity
		[[nodiscard]] constexpr sizetype size() const noexcept
		{
			return _size;
		}
		[[nodiscard]] constexpr sizetype length() const noexcept
		{
			return _size;
		}
		[[nodiscard]] constexpr sizetype max_size() const noexcept
		{
			return N;
		}
		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return N == 0 || _size == 0;
		}

		// element access
		[[nodiscard]] constexpr reference operator[](sizetype n)
		{
			return _data[n];
		}
		[[nodiscard]] constexpr const_reference operator[](sizetype n) const
		{
			return _data[n];
		}
		[[nodiscard]] constexpr reference at(sizetype n)
		{
			return _data.at(n);
		}
		[[nodiscard]] constexpr const_reference at(sizetype n) const
		{
			return _data.at(n);
		}
		[[nodiscard]] constexpr reference front()
		{
			return _data.front();
		}
		[[nodiscard]] constexpr const_reference front() const
		{
			return _data.front();
		}
		[[nodiscard]] constexpr reference back()
		{
			return _data.back();
		}
		[[nodiscard]] constexpr const_reference back() const
		{
			return _data.back();
		}

	private:
		template<sizet M>
		using same_with_other_size = TFixedString<M, value_type, traits_type>;

		template<sizetype pos, sizetype count, sizetype size>
		constexpr static sizetype calculate_substr_size()
		{
			if constexpr (pos >= size)
				return 0;

			constexpr sizetype rcount = std::min(count, size - pos);

			return rcount;
		}

		template<sizetype pos, sizetype count>
		using substr_result_type = same_with_other_size<calculate_substr_size<pos, count, N>()>;

	public:
		// string operations
		[[nodiscard]] constexpr pointer data() noexcept
		{
			return _data.data();
		}
		[[nodiscard]] constexpr const_pointer data() const noexcept
		{
			return _data.data();
		}
		[[nodiscard]] constexpr operator string_view_type()
		    const noexcept    // NOLINT(google-explicit-constructor)
		{
			return {data(), size()};
		}

		template<sizetype pos = 0, sizetype count = npos>
		[[nodiscard]] constexpr substr_result_type<pos, count> substr()
		{
			static_assert(pos <= N, "pos cannot be larger than size!");
			constexpr sizetype rcount = calculate_substr_size<pos, count, N>();
			substr_result_type<pos, count> result;
			Details::copy(begin() + pos, begin() + pos + rcount, result.begin());
			return result;
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype find(
		    const same_with_other_size<M>& str, sizetype pos = 0) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().find(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype find(string_view_type sv, sizetype pos = 0) const noexcept
		{
			return sv().find(sv, pos);
		}
		[[nodiscard]] constexpr sizetype find(const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().find(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype find(const value_type* s, sizetype pos = 0) const
		{
			return sv().find(s, pos);
		}
		[[nodiscard]] constexpr sizetype find(value_type c, sizetype pos = 0) const noexcept
		{
			return sv().find(c, pos);
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype rfind(
		    const same_with_other_size<M>& str, sizetype pos = npos) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().rfind(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype rfind(
		    string_view_type sv, sizetype pos = npos) const noexcept
		{
			return sv().rfind(sv, pos);
		}
		[[nodiscard]] constexpr sizetype rfind(const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().rfind(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype rfind(const value_type* s, sizetype pos = npos) const
		{
			return sv().rfind(s, pos);
		}
		[[nodiscard]] constexpr sizetype rfind(value_type c, sizetype pos = npos) const noexcept
		{
			return sv().rfind(c, pos);
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype find_first_of(
		    const same_with_other_size<M>& str, sizetype pos = 0) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().find_first_of(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype find_first_of(
		    string_view_type sv, sizetype pos = 0) const noexcept
		{
			return sv().find_first_of(sv, pos);
		}
		[[nodiscard]] constexpr sizetype find_first_of(
		    const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().find_first_of(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype find_first_of(const value_type* s, sizetype pos = 0) const
		{
			return sv().find_first_of(s, pos);
		}
		[[nodiscard]] constexpr sizetype find_first_of(
		    value_type c, sizetype pos = 0) const noexcept
		{
			return sv().find_first_of(c, pos);
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype find_last_of(
		    const same_with_other_size<M>& str, sizetype pos = npos) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().find_last_of(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype find_last_of(
		    string_view_type sv, sizetype pos = npos) const noexcept
		{
			return sv().find_last_of(sv, pos);
		}
		[[nodiscard]] constexpr sizetype find_last_of(
		    const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().find_last_of(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype find_last_of(
		    const value_type* s, sizetype pos = npos) const
		{
			return sv().find_last_of(s, pos);
		}
		[[nodiscard]] constexpr sizetype find_last_of(
		    value_type c, sizetype pos = npos) const noexcept
		{
			return sv().find_last_of(c, pos);
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype find_first_not_of(
		    const same_with_other_size<M>& str, sizetype pos = 0) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().find_first_of(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype find_first_not_of(
		    string_view_type sv, sizetype pos = 0) const noexcept
		{
			return sv().find_first_not_of(sv, pos);
		}
		[[nodiscard]] constexpr sizetype find_first_not_of(
		    const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().find_first_not_of(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype find_first_not_of(
		    const value_type* s, sizetype pos = 0) const
		{
			return sv().find_first_not_of(s, pos);
		}
		[[nodiscard]] constexpr sizetype find_first_not_of(
		    value_type c, sizetype pos = 0) const noexcept
		{
			return sv().find_first_not_of(c, pos);
		}

		template<sizet M>
		[[nodiscard]] constexpr sizetype find_last_not_of(
		    const same_with_other_size<M>& str, sizetype pos = npos) const noexcept
		{
			if constexpr (M > N)
				return npos;
			return sv().find_last_of(str.sv(), pos);
		}
		[[nodiscard]] constexpr sizetype find_last_not_of(
		    string_view_type sv, sizetype pos = npos) const noexcept
		{
			return sv().find_last_not_of(sv, pos);
		}
		[[nodiscard]] constexpr sizetype find_last_not_of(
		    const value_type* s, sizetype pos, sizetype n) const
		{
			return sv().find_last_not_of(s, pos, n);
		}
		[[nodiscard]] constexpr sizetype find_last_not_of(
		    const value_type* s, sizetype pos = npos) const
		{
			return sv().find_last_not_of(s, pos);
		}
		[[nodiscard]] constexpr sizetype find_last_not_of(
		    value_type c, sizetype pos = npos) const noexcept
		{
			return sv().find_last_not_of(c, pos);
		}

		[[nodiscard]] constexpr int compare(string_view_type v) const noexcept
		{
			return sv().compare(v);
		}
		[[nodiscard]] constexpr int compare(
		    sizetype pos1, sizetype count1, string_view_type v) const
		{
			return sv().compare(pos1, count1, v);
		}
		[[nodiscard]] constexpr int compare(sizetype pos1, sizetype count1, string_view_type v,
		    sizetype pos2, sizetype count2) const
		{
			return sv().compare(pos1, count1, v, pos2, count2);
		}
		[[nodiscard]] constexpr int compare(const value_type* s) const
		{
			return sv().compare(s);
		}
		[[nodiscard]] constexpr int compare(
		    sizetype pos1, sizetype count1, const value_type* s) const
		{
			return sv().compare(pos1, count1, s);
		}
		[[nodiscard]] constexpr int compare(
		    sizetype pos1, sizetype count1, const value_type* s, sizetype count2) const
		{
			return sv().compare(pos1, count1, s, count2);
		}

		[[nodiscard]] constexpr bool starts_with(string_view_type v) const noexcept
		{
			return sv().substr(0, v.size()) == v;
		}
		[[nodiscard]] constexpr bool starts_with(char c) const noexcept
		{
			return !empty() && traits_type::eq(front(), c);
		}
		[[nodiscard]] constexpr bool starts_with(const value_type* s) const noexcept
		{
			return starts_with(string_view_type(s));
		}

		[[nodiscard]] constexpr bool ends_with(string_view_type sv) const noexcept
		{
			return size() >= sv.size() && compare(size() - sv.size(), npos, sv) == 0;
		}
		[[nodiscard]] constexpr bool ends_with(value_type c) const noexcept
		{
			return !empty() && traits_type::eq(back(), c);
		}
		[[nodiscard]] constexpr bool ends_with(const value_type* s) const
		{
			return ends_with(string_view_type(s));
		}

		[[nodiscard]] constexpr bool contains(string_view_type sv) const noexcept
		{
			return find(sv) != npos;
		}
		[[nodiscard]] constexpr bool contains(value_type c) const noexcept
		{
			return find(c) != npos;
		}
		[[nodiscard]] constexpr bool contains(const value_type* s) const
		{
			return find(s) != npos;
		}

	private:
		constexpr string_view_type sv()
		{
			return *this;
		}
	};

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator==(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		if constexpr (M1 != M2)
			return false;
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type  = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) == rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator==(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type  = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) == rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator==(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type  = typename rhs_type::string_view_type;
		return lhs == static_cast<sv_type>(rhs);
	}

#if CPP20_SPACESHIP_OPERATOR_PRESENT

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr auto operator<=>(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type  = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) <=> rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr auto operator<=>(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type  = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) <=> rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr auto operator<=>(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type  = typename rhs_type::string_view_type;
		return lhs <=> static_cast<sv_type>(rhs);
	}

#else

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator!=(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		if constexpr (M1 != M2)
			return true;
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) != rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator!=(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) != rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator!=(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type = typename rhs_type::string_view_type;
		return lhs != static_cast<sv_type>(rhs);
	}

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator<(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) < rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator<(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) < rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator<(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type = typename rhs_type::string_view_type;
		return lhs < static_cast<sv_type>(rhs);
	}

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator<=(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) <= rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator<=(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) <= rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator<=(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type = typename rhs_type::string_view_type;
		return lhs <= static_cast<sv_type>(rhs);
	}

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator>(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) > rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator>(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) > rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator>(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type = typename rhs_type::string_view_type;
		return lhs > static_cast<sv_type>(rhs);
	}

	template<typename CharType, typename TTraits, sizet M1, sizet M2>
	[[nodiscard]] constexpr bool operator>=(const TFixedString<M1, CharType, TTraits>& lhs,
	    const TFixedString<M2, CharType, TTraits>& rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) >= rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator>=(const TFixedString<N, CharType, TTraits>& lhs,
	    std::basic_string_view<CharType, TTraits> rhs)
	{
		using lhs_type = std::decay_t<decltype(lhs)>;
		using sv_type = typename lhs_type::string_view_type;
		return static_cast<sv_type>(lhs) >= rhs;
	}

	template<typename CharType, typename TTraits, sizet N>
	[[nodiscard]] constexpr bool operator>=(std::basic_string_view<CharType, TTraits> lhs,
	    const TFixedString<N, CharType, TTraits>& rhs)
	{
		using rhs_type = std::decay_t<decltype(rhs)>;
		using sv_type = typename rhs_type::string_view_type;
		return lhs >= static_cast<sv_type>(rhs);
	}

#endif    // CPP20_SPACESHIP_OPERATOR_PRESENT

	template<sizet N, typename CharType = TChar>
	TFixedString(const CharType (&)[N]) -> TFixedString<N - 1, CharType>;

	template<sizet N>
	using FixedString = TFixedString<N, TChar>;

	// template <sizet N>
	// FixedString(const TChar (&)[N]) -> FixedString<N - 1>;


	template<sizet N, sizet M, typename CharType, typename TTraits>
	constexpr TFixedString<N + M, CharType, TTraits> operator+(
	    const TFixedString<N, CharType, TTraits>& lhs,
	    const TFixedString<M, CharType, TTraits>& rhs)
	{
		TFixedString<N + M, CharType, TTraits> result;
		Details::copy(lhs.begin(), lhs.end(), result.begin());
		Details::copy(rhs.begin(), rhs.end(), result.begin() + lhs.size());
		return result;
	}

	template<sizet N, sizet M, typename CharType, typename TTraits>
	constexpr TFixedString<N - 1 + M, CharType, TTraits> operator+(
	    const CharType (&lhs)[N], const TFixedString<M, CharType, TTraits>& rhs)
	{
		TFixedString lhs2 = lhs;
		return lhs2 + rhs;
	}

	template<sizet N, sizet M, typename CharType, typename TTraits>
	constexpr TFixedString<N + M - 1, CharType, TTraits> operator+(
	    const TFixedString<N, CharType, TTraits>& lhs, const CharType (&rhs)[M])
	{
		TFixedString rhs2 = rhs;
		return lhs + rhs2;
	}

	namespace Details
	{
		template<typename CharType>
		constexpr TFixedString<1, CharType> from_char(CharType ch)
		{
			TFixedString<1, CharType> fs;
			fs[0] = ch;
			return fs;
		}
	}    // namespace Details

	template<sizet N, typename CharType, typename TTraits>
	constexpr TFixedString<N + 1, CharType, TTraits> operator+(
	    CharType lhs, const TFixedString<N, CharType, TTraits>& rhs)
	{
		return Details::from_char(lhs) + rhs;
	}

	template<sizet N, typename CharType, typename TTraits>
	constexpr TFixedString<N + 1, CharType, TTraits> operator+(
	    const TFixedString<N, CharType, TTraits>& lhs, CharType rhs)
	{
		return lhs + Details::from_char(rhs);
	}

	template<sizet N, typename CharType, typename TTraits>
	std::basic_ostream<CharType, TTraits>& operator<<(
	    std::basic_ostream<CharType, TTraits>& out, const TFixedString<N, CharType, TTraits>& str)
	{
		out << str.data();
		return out;
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
