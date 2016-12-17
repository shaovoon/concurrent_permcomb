/*
MIT License

Concurrent Permutation version 1.0.0

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
#include <vector>
#include <cstdint>

namespace concurrent_perm
{
template<typename int_type>
void compute_factorial(uint32_t num, int_type& factorial )
{
	factorial = 1;

	for( uint32_t i=2; i<=num; ++i )
	{
		factorial = factorial * i;
	}
}

bool remove_element( uint32_t elem, uint32_t& remove_value, std::list<uint32_t>& leftovers )
{
	if( leftovers.empty() )
		return false;

	std::list<uint32_t>::iterator it = leftovers.begin();

	uint32_t cnt=0;
	for( ; it!=leftovers.end(); ++it )
	{
		if( cnt == elem )
		{
			remove_value = *it;
			leftovers.erase( it );
			return true;
		}

		++cnt;
	}

	return false;
}

template<typename int_type>
bool find_perm(uint32_t set_size, 
			  int_type index_to_find, 
			  std::vector<uint32_t>& results )
{
	results.clear();

	std::list<uint32_t> leftovers;
	for( uint32_t i=0; i<set_size; ++i )
		leftovers.push_back( i );

	uint32_t prev_size=0;
	++index_to_find;
	int_type remaining_index = index_to_find;
	bool processed = false;
	while( set_size )
	{
		prev_size = set_size;
		--set_size;

		int_type factorial;
		compute_factorial( set_size, factorial );
		int_type prev_mult = 0;
		for( uint32_t i=1; i<=prev_size; ++i )
		{
			int_type pos = factorial * i;

			if( remaining_index < pos || remaining_index == pos )
			{
				if( prev_mult < remaining_index || prev_mult == remaining_index )
				{
					processed = true;
					remaining_index = remaining_index - prev_mult;
				}

				uint32_t removed_value=0;
				remove_element( i-1, removed_value, leftovers );
				results.push_back( removed_value );
				break;
			}

			prev_mult = pos;
		}
	}

	return processed;
}

template<typename int_type, typename bidirectional_iterator, typename callback_type>
void worker_thread_proc(const int_type thread_index, 
	bidirectional_iterator begin, 
	bidirectional_iterator end, 
	int_type start_index, 
	int_type end_index, 
	uint32_t set_size, 
	callback_type callback)
{
	std::vector<uint32_t> results;
	std::vector< typename std::iterator_traits<bidirectional_iterator>::value_type> vec(begin, end);
	if(start_index>0)
	{
		if(concurrent_perm::find_perm(set_size, start_index, results))
		{
			std::vector< typename std::iterator_traits<bidirectional_iterator>::value_type> vecTemp(begin, end);
			for(size_t i=0; i<results.size(); ++i)
			{
				vec[i] = vecTemp[ results[i] ];
			}
		}
	}

	for (int_type j = start_index; j<end_index; ++j)
	{
		if (!callback(thread_index, vec.cbegin(), vec.cend()))
			return;
		std::next_permutation(vec.begin(), vec.end());
	}
}

template<typename int_type, typename bidirectional_iterator, typename callback_type>
bool compute_all_perm(int_type thread_cnt, bidirectional_iterator begin, bidirectional_iterator end, callback_type callback)
{
	int_type factorial=0; 
	typename std::iterator_traits<bidirectional_iterator>::difference_type set_size = std::distance(begin, end);
	compute_factorial( set_size, factorial );

	int_type each_thread_elem_cnt = factorial / thread_cnt;
	int_type remainder = factorial % thread_cnt;

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
			std::bind(worker_thread_proc<int_type, bidirectional_iterator, callback_type>, i, begin, end, start_index, end_index, set_size, callback))));
	}

	bulk = each_thread_elem_cnt;
	if( thread_cnt == 1 && remainder > 0 )
	{
		bulk += remainder;
	}

	int_type start_index = 0; 
	int_type end_index = bulk;
	worker_thread_proc<int_type, bidirectional_iterator, callback_type>( 0, begin, end, start_index, end_index, set_size, callback );

	for(size_t i=0; i<threads.size(); ++i)
	{
		threads[i]->join();
	}

	return true;
}

}