/*
MIT License

Concurrent Combination version 1.0.0

Copyright (c) 2016 Wong Shao Voon

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

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

template<typename int_type, typename bidirectional_iterator, typename callback_type>
void worker_thread_proc(const int_type thread_index, 
						bidirectional_iterator begin, 
						bidirectional_iterator end, 
						int_type start_index, 
						int_type end_index, 
						uint32_t fullset, 
						uint32_t subset, 
						callback_type callback)
{
	std::vector<uint32_t> results(subset);
	std::iota(results.begin(), results.end(), 0);

	std::vector< std::iterator_traits<bidirectional_iterator>::value_type> fullset_vec(begin, end);
	if(start_index>0)
	{
		find_comb(fullset, subset, start_index, results);
	}
	std::vector< std::iterator_traits<bidirectional_iterator>::value_type> vec;
	for(size_t i=0; i<results.size(); ++i)
	{
		vec.push_back(fullset_vec[results[i]]);
	}
	for(int_type j=start_index; j<end_index; ++j)
	{
		if(!callback(thread_index, fullset, subset, vec.cbegin(), vec.cend()))
			return;
		stdcomb::next_combination(fullset_vec.begin(), fullset_vec.end(), vec.begin(), vec.end());
	}
}

template<typename int_type, typename bidirectional_iterator, typename callback_type>
bool compute_all_comb(int_type thread_cnt, uint32_t subset, bidirectional_iterator begin, bidirectional_iterator end, callback_type callback)
{
	int_type total_comb=0; 
	typename std::iterator_traits<bidirectional_iterator>::difference_type fullset = std::distance(begin, end);
	if (!find_total_comb(fullset, subset, total_comb))
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
			std::bind(worker_thread_proc<int_type, bidirectional_iterator, callback_type>, i, begin, end, start_index, end_index, fullset, subset, callback))));
	}

	bulk = each_thread_elem_cnt;
	if( thread_cnt == 1 && remainder > 0 )
	{
		bulk += remainder;
	}

	int_type start_index = 0; 
	int_type end_index = bulk;
	int_type thread_index=0;
	worker_thread_proc<int_type, bidirectional_iterator, callback_type>( thread_index, begin, end, start_index, end_index, fullset, subset, callback );

	for(size_t i=0; i<threads.size(); ++i)
	{
		threads[i]->join();
	}

	return true;
}

}