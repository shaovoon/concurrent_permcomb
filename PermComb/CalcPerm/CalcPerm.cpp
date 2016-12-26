#include <iostream>
#include <numeric>
#include <string>
//#include <intrin.h>
//#include <boost/multiprecision/cpp_int.hpp>
#include "../permcomb/concurrent_perm.h"
#include "../common/timer.h"

void test_find_perm(uint32_t PermSetSize);
void unit_test();
void unit_test_threaded();
void unit_test_threaded_predicate();
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
bool test_threaded_perm_predicate(int_type thread_cnt, uint32_t set_size)
{
	std::cout << "test_threaded_perm_predicate(" << thread_cnt << ", " << set_size << ") starting" << std::endl;

	std::vector<char> results(set_size);
	std::iota(results.begin(), results.end(), 'A');

	std::vector<std::vector< std::vector<char> > > vecvecvec((size_t)thread_cnt);

	concurrent_perm::compute_all_perm(thread_cnt, results,
		[&vecvecvec](const int thread_index, const std::vector<char>& cont) -> bool
	{
		vecvecvec[thread_index].push_back(cont);
		return true;
	},
		[](char a, char b)
	{
		return a < b;
	}
	);

	std::vector< std::vector<char> > vecvec;
	do
	{
		vecvec.push_back(std::vector<char>(results.begin(), results.end()));
	} while (std::next_permutation(results.begin(), results.end(), [](char a, char b) { return a < b; }));

	// compare results
	size_t cnt = 0;
	bool error = false;
	for (size_t i = 0; i < vecvecvec.size(); ++i)
	{
		for (size_t j = 0; j < vecvecvec[i].size(); ++j, ++cnt)
		{
			if (!compare_vec(vecvec[cnt], vecvecvec[i][j]))
			{
				error = true;
				std::cerr << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}
	std::cout << "test_threaded_perm_predicate(" << thread_cnt << ", " << set_size << ") finished with" << ((error) ? " errors" : " no errors") << std::endl;

	return true;
}

template<typename int_type>
bool test_threaded_perm(int_type thread_cnt, uint32_t set_size)
{
	std::cout << "test_threaded_perm(" << thread_cnt << ", " << set_size << ") starting" << std::endl;

	std::vector<char> results(set_size);
	std::iota(results.begin(), results.end(), 'A');

	std::vector<std::vector< std::vector<char> > > vecvecvec((size_t)thread_cnt);

	concurrent_perm::compute_all_perm(thread_cnt, results,
		[&vecvecvec] (const int thread_index, const std::vector<char>& cont) -> bool
	{
		vecvecvec[thread_index].push_back(cont);
		return true;
	});

	std::vector< std::vector<char> > vecvec;
	do
	{
		vecvec.push_back(std::vector<char>(results.begin(), results.end()));
	} while (std::next_permutation(results.begin(), results.end()));

	// compare results
	size_t cnt = 0;
	bool error = false;
	for (size_t i = 0; i < vecvecvec.size(); ++i)
	{
		for (size_t j = 0; j < vecvecvec[i].size(); ++j, ++cnt)
		{
			if (!compare_vec(vecvec[cnt], vecvecvec[i][j]))
			{
				error = true;
				std::cerr << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}
	std::cout << "test_threaded_perm(" << thread_cnt << ", " << set_size << ") finished with" << ((error) ? " errors" : " no errors") << std::endl;

	return true;
}

// return false to stop processing
template<typename container_type>
struct empty_callback_t
{
	bool operator()(const int thread_index, const container_type& cont)
	{
		return true;
	}
};

//typedef boost::multiprecision::cpp_int int_type;
//typedef boost::multiprecision::int256_t int_type;
typedef int64_t int_type;

int main(int argc, char* argv[])
{
	//benchmark_perm();

	//unit_test();

	//unit_test_threaded();

	unit_test_threaded_predicate();

	return 0;
}

void benchmark_perm()
{
	std::string results(11, 'A');
	std::iota(results.begin(), results.end(), 'A');

	timer stopwatch;
	{
		std::string results2(results.begin(), results.end());

		stopwatch.start("next_permutation");
		while (std::next_permutation(results2.begin(), results2.end()))
		{

		}
		stopwatch.stop();
	}

	int_type thread_cnt = 1;

	typedef empty_callback_t<decltype(results)> callback_t;

	stopwatch.start("1 thread(s)");
	thread_cnt = 1;
	concurrent_perm::compute_all_perm(thread_cnt, results, callback_t());
	stopwatch.stop();

	stopwatch.start("2 thread(s)");
	thread_cnt = 2;
	concurrent_perm::compute_all_perm(thread_cnt, results, callback_t());
	stopwatch.stop();

	stopwatch.start("3 thread(s)");
	thread_cnt = 3;
	concurrent_perm::compute_all_perm(thread_cnt, results, callback_t());
	stopwatch.stop();

	stopwatch.start("4 thread(s)");
	thread_cnt = 4;
	concurrent_perm::compute_all_perm(thread_cnt, results, callback_t());
	stopwatch.stop();
}

void test_find_perm(uint32_t set_size)
{
	std::cout << "test_find_perm(" << set_size << ") starting" << std::endl;
	std::vector<uint32_t> results1;
	std::vector<uint32_t> results2(set_size);
	std::iota(results2.begin(), results2.end(), 0);

	int factorial=0; 
	concurrent_perm::compute_factorial( set_size, factorial );

	bool error = false;
	for(int j=0; j<factorial; ++j )
	{
		if(concurrent_perm::find_perm(set_size, j, results1))
		{
			if(compare_vec(results1, results2)==false)
			{
				error = true;
				std::cerr << "Perm at " << j << " is not the same!" << std::endl;
				display(results1);
				display(results2);
				break;
			}
			else
			{
				//display(results1);
			}
		}
		std::next_permutation(results2.begin(), results2.end());
	}
	std::cout << "test_find_perm(" << set_size << ") finished with" << ((error)?" errors":" no errors") <<  std::endl;
}

void unit_test()
{
	test_find_perm(3);
	test_find_perm(4);
	test_find_perm(5);
	test_find_perm(6);
	test_find_perm(7);
	test_find_perm(8);
	test_find_perm(9);
	test_find_perm(10);
}

void unit_test_threaded()
{
	int_type thread_cnt = 4;
	test_threaded_perm(thread_cnt, 5);
	test_threaded_perm(thread_cnt, 6);
	test_threaded_perm(thread_cnt, 7);
	test_threaded_perm(thread_cnt, 8);
	test_threaded_perm(thread_cnt, 9);
	test_threaded_perm(thread_cnt, 10);
}

void unit_test_threaded_predicate()
{
	int_type thread_cnt = 4;
	test_threaded_perm_predicate(thread_cnt, 5);
	test_threaded_perm_predicate(thread_cnt, 6);
	test_threaded_perm_predicate(thread_cnt, 7);
	test_threaded_perm_predicate(thread_cnt, 8);
	test_threaded_perm_predicate(thread_cnt, 9);
	test_threaded_perm_predicate(thread_cnt, 10);
}
