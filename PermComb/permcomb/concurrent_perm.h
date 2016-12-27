///////////////////////////////////////////////////////////////////////////////
// concurrent_perm.h header file
//
// Concurrent Permutation version 0.1.0
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
#include <vector>
#include <cstdint>

namespace concurrent_perm
{

struct no_predicate_type
{
};

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

template<typename container_type, typename index_type, typename callback_type, typename predicate_type>
typename std::enable_if<!std::is_same<predicate_type, no_predicate_type>::value>::type 
perm_loop(const int thread_index, container_type& cont, const index_type& start, const index_type& end, callback_type callback, predicate_type pred)
{
	for (index_type j = start; j < end; ++j)
	{
		if (!callback(thread_index, cont))
			return;
		std::next_permutation(cont.begin(), cont.end(), pred);
	}
}

template<typename container_type, typename index_type, typename callback_type, typename predicate_type>
typename std::enable_if<std::is_same<predicate_type, no_predicate_type>::value>::type
perm_loop(const int thread_index, container_type& cont, const index_type& start, const index_type& end, callback_type callback, predicate_type pred)
{
	for (index_type j = start; j < end; ++j)
	{
		if (!callback(thread_index, cont))
			return;
		std::next_permutation(cont.begin(), cont.end());
	}
}

template<typename int_type, typename container_type, typename callback_type, typename predicate_type>
void worker_thread_proc(const int_type& thread_index, 
	const container_type& cont,
	int_type start_index, 
	int_type end_index, 
	callback_type callback,
	predicate_type pred)
{
	const int thread_index_n = static_cast<const int>(thread_index);
	std::vector<uint32_t> results;
	container_type vec(cont.cbegin(), cont.cend());
	if(start_index>0)
	{
		if(concurrent_perm::find_perm(cont.size(), start_index, results))
		{
			container_type vecTemp(cont.cbegin(), cont.cend());
			for(size_t i=0; i<results.size(); ++i)
			{
				vec[i] = vecTemp[ results[i] ];
			}
		}
	}

	if (end_index <= std::numeric_limits<int>::max()) // use POD counter when possible
	{
		const int start_i = static_cast<int>(start_index);
		const int end_i   = static_cast<int>(end_index);
		perm_loop(thread_index_n, vec, start_i, end_i, callback, pred);
	}
	else if (end_index <= std::numeric_limits<int64_t>::max()) // use POD counter when possible
	{
		const int64_t start_i = static_cast<int64_t>(start_index);
		const int64_t end_i = static_cast<int64_t>(end_index);
		perm_loop(thread_index_n, vec, start_i, end_i, callback, pred);
	}
	else
	{
		perm_loop(thread_index_n, vec, start_index, end_index, callback, pred);
	}
}

template<typename int_type, typename container_type, typename callback_type, typename predicate_type=no_predicate_type>
bool compute_all_perm_shard(int_type cpu_index, int_type cpu_cnt, int_type thread_cnt, const container_type& cont, callback_type callback, predicate_type pred=predicate_type())
{
	int_type factorial=0; 
	compute_factorial(cont.size(), factorial );

	int_type each_cpu_elem_cnt = factorial / cpu_cnt;
	int_type cpu_remainder = factorial % cpu_cnt;
	int_type offset = cpu_index*each_cpu_elem_cnt;
	if (cpu_index == (cpu_cnt - 1) && cpu_remainder > 0)
	{
		each_cpu_elem_cnt += cpu_remainder;
	}

	int_type each_thread_elem_cnt = each_cpu_elem_cnt / thread_cnt;
	int_type remainder = each_cpu_elem_cnt % thread_cnt;

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
		int_type start_index = i * each_thread_elem_cnt + offset;
		int_type end_index = start_index + bulk;
		threads.push_back( std::shared_ptr<std::thread>(new std::thread(
			std::bind(worker_thread_proc<int_type, container_type, callback_type, predicate_type>, i, cont, start_index, end_index, callback, pred))));
	}

	bulk = each_thread_elem_cnt; // reset remainder
	int_type start_index = offset;
	int_type end_index = start_index + bulk;
	int_type thread_index = 0;
	worker_thread_proc<int_type, container_type, callback_type, predicate_type>(thread_index, cont, start_index, end_index, callback, pred);

	for(size_t i=0; i<threads.size(); ++i)
	{
		threads[i]->join();
	}

	return true;
}

template<typename int_type, typename container_type, typename callback_type, typename predicate_type = no_predicate_type>
bool compute_all_perm(int_type thread_cnt, const container_type& cont, callback_type callback, predicate_type pred = predicate_type())
{
	int_type cpu_index = 0; 
	int_type cpu_cnt = 1;
	return compute_all_perm_shard(cpu_index, cpu_cnt, thread_cnt, cont, callback, pred);
}

}