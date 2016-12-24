///////////////////////////////////////////////////////////////////////////////
// concurrent_comb.h header file
//
// Concurrent Combination version 0.1.0
// Copyright 2016 Wong Shao Voon
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// See http://www.boost.org/libs/foreach for documentation


#pragma once

#include <list>
#include <vector>
#include <iterator>
#include <memory>
#include <thread>
#include <functional>
#include <algorithm>
#include <numeric> // for iota
#include <cstdint>
#include "combination.h"

namespace concurrent_comb
{
template<typename int_type>
void compute_factorial( uint32_t num, int_type& factorial )
{
	factorial = 1;

	for( uint32_t i=2; i<=num; ++i )
	{
		factorial = factorial * i;
	}
}

template<typename int_type>
void find_range( const uint32_t &Min, const uint32_t &Max, int_type& result )
{
	if(Min == Max)
	{
		result = 1;
		return;
	}

	result = Min+1;

	for( int_type i=Min+2; i<=Max; ++i )
	{
		result = result * i;
	}
}

template<typename int_type>
bool find_total_comb( const uint32_t fullset, const uint32_t subset, int_type& total )
{
	if (subset > fullset)
		return false;

	if( subset == fullset )
	{
		total = 1;
		return true;
	}

	uint32_t acomb = fullset - subset;

	int_type factorial = 0;
	int_type range = 0;
	if( acomb >= subset )
	{
		compute_factorial(subset, factorial );
		find_range( acomb, fullset, range );
	}
	else
	{
		compute_factorial( acomb, factorial );
		find_range( subset, fullset, range );
	}
	total = range / factorial;

	return true;
}

template<typename int_type>
bool find_comb(const uint32_t fullset, 
			   const uint32_t subset, 
			   int_type index_to_find,
			   std::vector<uint32_t>& results )
{
	if( subset > fullset || fullset == 0 || subset == 0 )
		return false;

	// Actual Processing
	uint32_t remaining_set = fullset - 1;  
	uint32_t remaining_comb = subset - 1;  

	for( uint32_t x=0; x<subset; ++x )
	{

		if( x == subset-1 ) // Last Element
		{
			while( true )
			{
				if( 0 == index_to_find )
				{
					results[x] = fullset - remaining_set - 1;
					break;			
				}
				else
				{
					index_to_find -= 1;
					--remaining_set;
				}
			}
		}
		else
		{
			int_type total_comb=0;
			int_type prev=0;

			uint32_t loop = remaining_set-remaining_comb;
			bool found = false;
			uint32_t x_prev = 0;

			if( x > 0 )
				x_prev = results[ x-1 ] + 1;

			uint32_t y;
			for( y=0; y<loop; ++y )
			{
				if (!find_total_comb(remaining_set, remaining_comb, total_comb))
					return false;

				total_comb += prev;
				if( total_comb > index_to_find ) // prev is the found one
				{
					index_to_find -= prev;

					results[x] = y + x_prev;

					found = true;
					break;
				}
				prev = total_comb;
				--remaining_set;
			}

			if( !found )
			{
				index_to_find -= total_comb;

				results[x] = y + x_prev;

			}

			--remaining_set;
			--remaining_comb;
		}
	}

	return true;
};

template<typename int_type, typename container_type, typename callback_type>
void worker_thread_proc(const int_type thread_index, 
						const container_type& cont,
						int_type start_index, 
						int_type end_index, 
						uint32_t subset, 
						callback_type callback)
{
	const int thread_index_n = static_cast<const int>(thread_index);

	std::vector<uint32_t> results(subset);
	std::iota(results.begin(), results.end(), 0);

	if(start_index>0)
	{
		find_comb(cont.size(), subset, start_index, results);
	}
	container_type vec;
	for(size_t i=0; i<results.size(); ++i)
	{
		vec.push_back(cont[results[i]]);
	}
	container_type cont2(cont.begin(), cont.end());
	if(end_index <= std::numeric_limits<int>::max()) // use POD counter when possible
	{ 
		const int start_i = static_cast<int>(start_index);
		const int end_i = static_cast<int>(end_index);
		for (int j = start_i; j < end_i; ++j)
		{
			if (!callback(thread_index_n, cont.size(), subset, vec))
				return;
			stdcomb::next_combination(cont2.begin(), cont2.end(), vec.begin(), vec.end());
		}
	}
	else if (end_index <= std::numeric_limits<int64_t>::max()) // use POD counter when possible
	{
		const int64_t start_i = static_cast<int64_t>(start_index);
		const int64_t end_i = static_cast<int64_t>(end_index);
		for (int64_t j = start_i; j < end_i; ++j)
		{
			if (!callback(thread_index_n, cont.size(), subset, vec))
				return;
			stdcomb::next_combination(cont2.begin(), cont2.end(), vec.begin(), vec.end());
		}
	}
	else
	{
		for (int_type j = start_index; j < end_index; ++j)
		{
			if (!callback(thread_index_n, cont.size(), subset, vec))
				return;
			stdcomb::next_combination(cont2.begin(), cont2.end(), vec.begin(), vec.end());
		}
	}
}

template<typename int_type, typename container_type, typename callback_type>
bool compute_all_comb(int_type thread_cnt, uint32_t subset, const container_type& cont, callback_type callback)
{
	int_type total_comb=0; 
	if (!find_total_comb(cont.size(), subset, total_comb))
		return false;

	int_type each_thread_elem_cnt = total_comb / thread_cnt;
	int_type remainder = total_comb % thread_cnt;

	std::vector<std::shared_ptr<std::thread> > threads;

	int_type bulk = each_thread_elem_cnt;
	for(int_type i=1; i<thread_cnt; ++i)
	{
		// test for last thread
		bulk = each_thread_elem_cnt;
		if( i == (thread_cnt-1) && remainder > 0 )
		{
			bulk += remainder;
		}
		int_type start_index = i*each_thread_elem_cnt; 
		int_type end_index = start_index + bulk;
		threads.push_back( std::shared_ptr<std::thread>(new std::thread(
			std::bind(worker_thread_proc<int_type, container_type, callback_type>, i, cont, start_index, end_index, subset, callback))));
	}

	bulk = each_thread_elem_cnt;
	if( thread_cnt == 1 && remainder > 0 )
	{
		bulk += remainder;
	}

	int_type start_index = 0; 
	int_type end_index = bulk;
	int_type thread_index=0;
	worker_thread_proc<int_type, container_type, callback_type>( thread_index, cont, start_index, end_index, subset, callback );

	for(size_t i=0; i<threads.size(); ++i)
	{
		threads[i]->join();
	}

	return true;
}

}