#include <iostream>
#include <numeric>
#include <string>
//#include <intrin.h>
//#include <boost/multiprecision/cpp_int.hpp>
#include "../permcomb/concurrent_perm.h"
#include "../common/timer.h"

void test_find_perm(uint32_t PermSetSize);
void benchmark_perm();

template<typename T>
bool compare_vec(std::vector<T>& results1, std::vector<T>& results2)
{
	if (results1.size() != results2.size())
		return false;

	for (size_t i = 0; i<results1.size(); ++i)
	{
		if (results1[i] != results2[i])
			return false;
	}
	return true;
}

template<typename T>
void display(std::vector<T>& results)
{
	for (size_t i = 0; i<results.size(); ++i)
	{
		std::cout << results[i] << ", ";
	}
	std::cout << std::endl;
}

template<typename int_type>
bool how_to_use_thread_index_perm(int_type thread_cnt, uint32_t set_size)
{
	std::vector<char> results(set_size);
	std::iota(results.begin(), results.end(), 'A');

	std::vector<std::vector< std::vector<char> > > vecvecvec((size_t)thread_cnt);

	concurrent_perm::compute_all_perm(thread_cnt, results.cbegin(), results.cend(),
		[&vecvecvec] (const int_type thread_index, auto begin, auto end) -> bool
	{
		vecvecvec[(size_t)thread_index].push_back(std::vector<char>(begin, end));
		return true;
	});

	std::vector< std::vector<char> > vecvec;
	do
	{
		vecvec.push_back(std::vector<char>(results.begin(), results.end()));
	} while (std::next_permutation(results.begin(), results.end()));

	// compare results
	size_t cnt = 0;
	for (size_t i = 0; i < vecvecvec.size(); ++i)
	{
		for (size_t j = 0; j < vecvecvec[i].size(); ++j, ++cnt)
		{
			if (!compare_vec(vecvec[cnt], vecvecvec[i][j]))
			{
				std::cout << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}
	std::cout << "how_to_use_thread_index_perm done! thread_cnt: " << thread_cnt << ", set_size: " << set_size << std::endl;

	return true;
}

// return false to stop processing
template<typename int_type, typename bidirectional_iterator>
struct empty_callback_t
{
	bool operator()(const int_type thread_index, bidirectional_iterator begin, bidirectional_iterator end)
	{
		return true;
	}
};

//typedef boost::multiprecision::cpp_int int_type;
//typedef boost::multiprecision::int256_t int_type;
typedef int64_t int_type;

int main(int argc, char* argv[])
{
	int_type thread_cnt = 4;
	how_to_use_thread_index_perm(thread_cnt, 10);

	benchmark_perm();

	//test_find_perm(3);

	return 0;
}

void benchmark_perm()
{
	std::string results(11, 'A');
	std::iota(results.begin(), results.end(), 'A');

	timer stopwatch;
	{
		std::string results2(results.begin(), results.end());

		stopwatch.start_timing("Plain vanilla");
		while (std::next_permutation(results2.begin(), results2.end()))
		{

		}
		stopwatch.stop_timing();
	}

	int_type thread_cnt = 1;

	typedef empty_callback_t<int_type, std::vector<char>::const_iterator> callback_t;

	stopwatch.start_timing("1 thread(s)");
	thread_cnt = 1;
	concurrent_perm::compute_all_perm(thread_cnt, results.cbegin(), results.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("2 thread(s)");
	thread_cnt = 2;
	concurrent_perm::compute_all_perm(thread_cnt, results.cbegin(), results.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("3 thread(s)");
	thread_cnt = 3;
	concurrent_perm::compute_all_perm(thread_cnt, results.cbegin(), results.cend(), callback_t());
	stopwatch.stop_timing();

	stopwatch.start_timing("4 thread(s)");
	thread_cnt = 4;
	concurrent_perm::compute_all_perm(thread_cnt, results.cbegin(), results.cend(), callback_t());
	stopwatch.stop_timing();
}

void test_find_perm(uint32_t set_size)
{
	std::vector<uint32_t> results1;
	std::vector<uint32_t> results2(set_size);
	std::iota(results2.begin(), results2.end(), 0);

	int factorial=0; 
	concurrent_perm::compute_factorial( set_size, factorial );

	for(int j=0; j<factorial; ++j )
	{
		if(concurrent_perm::find_perm(set_size, j, results1))
		{
			if(compare_vec(results1, results2)==false)
			{
				std::cout << "Perm at " << j << " is not the same!" << std::endl;
				display(results1);
				display(results2);
			}
			else
			{
				display(results1);
			}
		}
		std::next_permutation(results2.begin(), results2.end());
	}
}


