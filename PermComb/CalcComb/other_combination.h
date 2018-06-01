#pragma once

// This header consists of next_combination downloaded from the web, mainly used for benchmarking against mine.

#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>



namespace other
{
	// https://stackoverflow.com/questions/2211915/combination-and-permutation-in-c/2616837#2616837
	template<class RandIt>
	bool next_k_permutation(RandIt first, RandIt mid, RandIt last)
	{
		typedef typename std::iterator_traits<RandIt>::value_type value_type;

		std::sort(mid, last, std::greater< value_type >());
		return std::next_permutation(first, last);
	}
	template<class BiDiIt>
	bool other_next_combination1(BiDiIt first, BiDiIt mid, BiDiIt last)
	{
		typedef typename std::iterator_traits< BiDiIt >::value_type value_type;
		bool result;
		do
		{
			result = next_k_permutation(first, mid, last);
		} while (std::adjacent_find(first, mid,
			std::greater< value_type >())
			!= mid);
		return result;
	}


	//==========================
	// from https://codereview.stackexchange.com/questions/184586/generate-next-combination-in-lexicographic-order
	template <typename Iterator>
	Iterator find_next_increase_position(Iterator begin, Iterator combination_end, Iterator end);

	template <typename Iterator>
	bool other_next_combination2(Iterator begin, Iterator end, unsigned k) {
		const auto combination_end = begin + k;
		const auto next_move = find_next_increase_position(begin, combination_end, end);
		if (next_move == end) return false;
		const auto previous_value = *next_move;
		std::inplace_merge(next_move, combination_end, end);
		const auto next_rotation =
			std::rotate(next_move, std::upper_bound(next_move, end, previous_value), end);
		std::rotate(combination_end, next_rotation, end);
		return true;
	}

	template <typename Iterator>
	Iterator find_next_increase_position(Iterator begin, Iterator combination_end, Iterator end) {
		auto pos = std::upper_bound(typename std::reverse_iterator<Iterator>(combination_end),
			typename std::reverse_iterator<Iterator>(begin),
			*--end,
			std::greater<typename Iterator::value_type>());
		if (pos.base() == begin)
			return ++end;
		return --pos.base();
	}

}