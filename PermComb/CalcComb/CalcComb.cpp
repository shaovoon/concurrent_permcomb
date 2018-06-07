#include <iostream>
#include <string>
//#include <intrin.h>
//#include <boost/multiprecision/cpp_int.hpp>
#include "../permcomb/concurrent_comb.h"
#include "../common/timer.h"
#include "other_combination.h"

void test_find_comb(uint32_t fullset, uint32_t subset);
void unit_test();
void unit_test_threaded();
void unit_test_threaded_predicate();
void unit_test_threaded_shard();
void unit_test_comb_by_idx();
void usage_of_comb_by_idx();
void usage_of_next_comb();
void usage_of_next_comb_with_state();
void usage_of_other_next_comb();
void usage_of_other_next_pcomb();
void benchmark_comb();

template<typename T>
bool compare_vec(T& results1, T& results2)
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
void display(T& results)
{
	for (size_t i = 0; i<results.size(); ++i)
	{
		std::cout << results[i] << ", ";
	}
	std::cout << std::endl;
}

template<typename int_type>
bool test_threaded_comb_predicate(int_type thread_cnt, uint32_t fullset_size, uint32_t subset_size)
{
	std::cout << "test_threaded_comb_predicate(" << thread_cnt << ", " << fullset_size << ", " << subset_size << ") starting" << std::endl;

	std::vector<uint32_t> fullset(fullset_size);
	std::iota(fullset.begin(), fullset.end(), 0);

	std::vector<std::vector< std::vector<uint32_t> > > vecvecvec((size_t)thread_cnt);

	concurrent_comb::compute_all_comb(thread_cnt, subset_size, fullset,
		[&vecvecvec](const int thread_index,
			const size_t fullset_cnt,
			const std::vector<uint32_t>& cont) -> bool
	{
		vecvecvec[(size_t)thread_index].push_back(cont);
		return true;
	},
		[](const int thread_index,
			const size_t fullset_cnt,
			const std::vector<uint32_t>& cont, 
            const std::string& error) -> void
	{
		std::cerr << error;
	},
		[](uint32_t a, uint32_t b) { return a == b; });

	std::vector<uint32_t> subset(subset_size);
	std::iota(subset.begin(), subset.end(), 0);
	std::vector< std::vector<uint32_t> > vecvec;
	do
	{
		vecvec.push_back(std::vector<uint32_t>(subset.begin(), subset.end()));
	} while (stdcomb::next_combination(fullset.begin(), fullset.end(), subset.begin(), subset.end(), [](uint32_t a, uint32_t b) { return a == b; }));

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

				std::cout << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}

	std::cout << "test_threaded_comb_predicate(" << thread_cnt << ", " << fullset_size << ", " << subset_size <<
		") finished with" << ((error) ? " errors" : " no errors") << std::endl;

	return true;
}

template<typename int_type>
bool test_threaded_comb(int_type thread_cnt, uint32_t fullset_size, uint32_t subset_size)
{
	std::cout << "test_threaded_comb(" << thread_cnt << ", " << fullset_size << ", " << subset_size << ") starting" << std::endl;

	std::vector<uint32_t> fullset(fullset_size);
	std::iota(fullset.begin(), fullset.end(), 0);

	std::vector<std::vector< std::vector<uint32_t> > > vecvecvec((size_t)thread_cnt);

	concurrent_comb::compute_all_comb(thread_cnt, subset_size, fullset,
		[&vecvecvec](const int thread_index, 
			const size_t fullset_cnt,  
			const std::vector<uint32_t>& cont) -> bool
		{
			vecvecvec[(size_t)thread_index].push_back(cont);
			return true;
		},
        [](const int thread_index,
			const size_t fullset_cnt,
			const std::vector<uint32_t>& cont, 
            const std::string& error) -> void
        {
            std::cerr << error;
        });

	std::vector<uint32_t> subset(subset_size);
	std::iota(subset.begin(), subset.end(), 0);
	std::vector< std::vector<uint32_t> > vecvec;
	do
	{
		vecvec.push_back(std::vector<uint32_t>(subset.begin(), subset.end()));
	} while (stdcomb::next_combination(fullset.begin(), fullset.end(), subset.begin(), subset.end()));

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

				std::cout << "Perm at " << cnt << " is not the same!" << std::endl;

				display(vecvec[cnt]);
				display(vecvecvec[i][j]);

				return false;
			}
		}
	}

	std::cout << "test_threaded_comb(" << thread_cnt << ", " << fullset_size << ", " << subset_size << 
		") finished with" << ((error) ? " errors" : " no errors") << std::endl;

	return true;
}

template<typename int_type>
bool test_threaded_comb_shard(int_type thread_cnt, uint32_t fullset_size, uint32_t subset_size)
{
	std::cout << "test_threaded_comb_shard(" << thread_cnt << ", " << fullset_size << ", " << subset_size << ") starting" << std::endl;

	std::vector<uint32_t> fullset(fullset_size);
	std::iota(fullset.begin(), fullset.end(), 0);

	std::vector<std::vector< std::vector<uint32_t> > > vecvecvec((size_t)thread_cnt);

	int_type cpu_cnt = 4;
	std::vector<std::vector<std::vector< std::vector<uint32_t> > > > vecvecvecvec;
	for (int_type i = 0; i < cpu_cnt; ++i)
		vecvecvecvec.push_back(vecvecvec);

	for (int_type i = 0; i < cpu_cnt; ++i)
	{
		int_type cpu_index = i;
		int cpu_index_n = static_cast<int>(cpu_index);

		concurrent_comb::compute_all_comb_shard(cpu_index, cpu_cnt, thread_cnt, subset_size, fullset,
			[&vecvecvecvec, cpu_index_n](const int thread_index,
				const size_t fullset_cnt,
				const std::vector<uint32_t>& cont) -> bool
		{
			vecvecvecvec[cpu_index_n][(size_t)thread_index].push_back(cont);
			return true;
		},
			[](const int thread_index,
				const size_t fullset_cnt,
				const std::vector<uint32_t>& cont, 
                const std::string& error) -> void
		{
            std::cerr << error;
		});
	}
	std::vector<uint32_t> subset(subset_size);
	std::iota(subset.begin(), subset.end(), 0);
	std::vector< std::vector<uint32_t> > vecvec;
	do
	{
		vecvec.push_back(std::vector<uint32_t>(subset.begin(), subset.end()));
	} while (stdcomb::next_combination(fullset.begin(), fullset.end(), subset.begin(), subset.end()));

	// compare results
	size_t cnt = 0;
	bool error = false;
	for (size_t k = 0; k < vecvecvecvec.size(); ++k)
	{
		for (size_t i = 0; i < vecvecvecvec[k].size(); ++i)
		{
			for (size_t j = 0; j < vecvecvecvec[k][i].size(); ++j, ++cnt)
			{
				if (!compare_vec(vecvec[cnt], vecvecvecvec[k][i][j]))
				{
					error = true;

					std::cout << "Comb at " << cnt << " is not the same!" << std::endl;

					display(vecvec[cnt]);
					display(vecvecvecvec[k][i][j]);

					return false;
				}
			}
		}
	}
	std::cout << "test_threaded_comb_shard(" << thread_cnt << ", " << fullset_size << ", " << subset_size <<
		") finished with" << ((error) ? " errors" : " no errors") << std::endl;

	return true;
}

// return false to stop processing
template<typename container_type>
struct empty_callback_t
{
	bool operator()(const int thread_index, const size_t fullset_size, const container_type& cont)
	{
		return true;
	}
};

template<typename container_type>
struct error_callback_t
{
	void operator()(const int thread_index, const size_t fullset_size, const container_type& cont, const std::string& error)
	{
		std::cerr << error << std::endl;
	}
};

//typedef boost::multiprecision::cpp_int int_type;
//typedef boost::multiprecision::int128_t int_type;
typedef int64_t int_type;

int main(int argc, char* argv[])
{
	//benchmark_comb();

	//unit_test();

	//unit_test_threaded();

	//unit_test_threaded_predicate();

	//unit_test_threaded_shard();

	//unit_test_comb_by_idx();

	//usage_of_comb_by_idx();

	usage_of_next_comb();

	usage_of_next_comb_with_state();

	usage_of_other_next_comb();

	usage_of_other_next_pcomb();

	return 0;
}

void benchmark_comb()
{
	std::vector<int> fullset_vec(20);
	std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
	uint32_t subset = 10;

	timer stopwatch;
	{
		std::vector<int> fullset_vec2(fullset_vec.begin(), fullset_vec.end());
		std::vector<int> subset_vec(fullset_vec.begin(), fullset_vec.begin()+ subset);

		stopwatch.start("next_combination");
		while (stdcomb::next_combination(fullset_vec2.begin(), fullset_vec2.end(), subset_vec.begin(), subset_vec.end()))
		{

		}
		stopwatch.stop();
	}


	int_type thread_cnt = 1;

	typedef empty_callback_t<decltype(fullset_vec)> callback_t;
	typedef error_callback_t<decltype(fullset_vec)> err_callback_t;

	stopwatch.start("1 thread(s)");
	thread_cnt = 1;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, callback_t(), err_callback_t());
	stopwatch.stop();

	stopwatch.start("2 thread(s)");
	thread_cnt = 2;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, callback_t(), err_callback_t());
	stopwatch.stop();

	stopwatch.start("3 thread(s)");
	thread_cnt = 3;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, callback_t(), err_callback_t());
	stopwatch.stop();

	stopwatch.start("4 thread(s)");
	thread_cnt = 4;
	concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, callback_t(), err_callback_t());
	stopwatch.stop();
}

void test_find_comb(uint32_t fullset, uint32_t subset)
{
	std::cout << "test_find_comb(" << fullset << "," << subset << ") starting" << std::endl;
	std::vector<uint32_t> fullset_vec(fullset);
	std::vector<uint32_t> results1(subset);
	std::vector<uint32_t> results2(subset);

	std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
	std::iota(results1.begin(), results1.end(), 0);
	std::iota(results2.begin(), results2.end(), 0);

	uint32_t nTotal = 0;
	if (!concurrent_comb::compute_total_comb(fullset, subset, nTotal))
	{
		std::cerr << "compute_total_comb() returns false" << std::endl;
		return;
	}

	bool error = false;
	for(uint32_t j=0; j<nTotal; ++j )
	{
		if(concurrent_comb::find_comb(fullset, subset, j, results1))
		{
			if(compare_vec(results1, results2)==false)
			{
				error = true;
				std::cout << "Perm at " << j << " is not the same!" << std::endl;
				display(results1);
				display(results2);
			}
			else
			{
				//display(results1);
			}
		}
		stdcomb::next_combination(fullset_vec.begin(), fullset_vec.end(), results2.begin(), results2.end());
	}
	std::cout << "test_find_comb(" << fullset << "," << subset << ") finished with" << ((error) ? " errors" : " no errors") << std::endl;
}

void unit_test()
{
	test_find_comb(3,2);
	test_find_comb(4,2);
	test_find_comb(5,3);
	test_find_comb(6,3);
	test_find_comb(7,4);
	test_find_comb(8,4);
	test_find_comb(9,5);
	test_find_comb(10,5);
	test_find_comb(11,6);
	test_find_comb(12,6);
	test_find_comb(13,7);
	test_find_comb(14,7);
	test_find_comb(15,8);
	test_find_comb(16,8);
	test_find_comb(17,9);
	test_find_comb(18,9);
}

void unit_test_threaded()
{
	int_type thread_cnt = 4;
	test_threaded_comb(thread_cnt, 5, 3);
	test_threaded_comb(thread_cnt, 6, 3);
	test_threaded_comb(thread_cnt, 7, 4);
	test_threaded_comb(thread_cnt, 8, 4);
	test_threaded_comb(thread_cnt, 9, 5);
	test_threaded_comb(thread_cnt, 10, 5);
	thread_cnt = 10;
	test_threaded_comb(thread_cnt, 2, 1);
}

void unit_test_threaded_predicate()
{
	int_type thread_cnt = 4;
	test_threaded_comb_predicate(thread_cnt, 5, 3);
	test_threaded_comb_predicate(thread_cnt, 6, 3);
	test_threaded_comb_predicate(thread_cnt, 7, 4);
	test_threaded_comb_predicate(thread_cnt, 8, 4);
	test_threaded_comb_predicate(thread_cnt, 9, 5);
	test_threaded_comb_predicate(thread_cnt, 10, 5);
}

void unit_test_threaded_shard()
{
	int_type thread_cnt = 2;
	test_threaded_comb_shard(thread_cnt, 5, 3);
	test_threaded_comb_shard(thread_cnt, 6, 3);
	test_threaded_comb_shard(thread_cnt, 7, 4);
	test_threaded_comb_shard(thread_cnt, 8, 4);
	test_threaded_comb_shard(thread_cnt, 9, 5);
	test_threaded_comb_shard(thread_cnt, 10, 5);
	//thread_cnt = 10;
	//test_threaded_comb_shard(thread_cnt, 2, 1); // should fail
}

void unit_test_comb_by_idx()
{
	uint64_t index_to_find = 0;
	std::string original_text = "123456";
	std::string std_combined = "123";
	while (true)
	{
		std::string combined = concurrent_comb::find_comb_by_idx(std_combined.size(), index_to_find, original_text);
		if (combined.size() == 0)
		{
			std::cout << "Ended at index: " << index_to_find << std::endl;
			break;
		}
		if (compare_vec(combined, std_combined) == false)
		{
			std::cout << "string not equal at index: " << index_to_find << std::endl;
			break;
		}
		++index_to_find;
		stdcomb::next_combination(original_text.begin(), original_text.end(), std_combined.begin(), std_combined.end());
	}

}

void usage_of_next_comb()
{
	std::string original_text = "1234567890ABCDEFGHIJKLMNO";
	std::string std_combined = "12345678";
	uint64_t total = 0;
	if(concurrent_comb::compute_total_comb(original_text.size(), std_combined.size(), total))
	{
		timer stopwatch;
		stopwatch.start("next_comb");

		//std::cout << std_combined << std::endl;
		for (uint64_t i = 1; i < total; ++i)
		{
			stdcomb::next_combination(original_text.begin(), original_text.end(), std_combined.begin(), std_combined.end());
			//std::cout << std_combined << std::endl;
		}
		stopwatch.stop();
	}
	std::cout << std_combined << std::endl;
	/* output
	123
	124
	125
	126
	134
	135
	136
	145
	146
	156
	234
	235
	236
	245
	246
	256
	345
	346
	356
	456
	*/
}

void usage_of_other_next_comb()
{
	std::string original_text = "1234567890ABCDEFGHIJKLMNO";
	std::string std_combined = "12345678";
	uint64_t total = 0;
	if (concurrent_comb::compute_total_comb(original_text.size(), std_combined.size(), total))
	{
		timer stopwatch;
		stopwatch.start("next_comb");

		//std::cout << std_combined << std::endl;
		for (uint64_t i = 1; i < total; ++i)
		{
			other::other_next_combination2(original_text.begin(), original_text.end(), std_combined.size());
			//std::cout << std_combined << std::endl;
		}
		stopwatch.stop();
	}
	std::cout << original_text << std::endl;
	/* output
	123
	124
	125
	126
	134
	135
	136
	145
	146
	156
	234
	235
	236
	245
	246
	256
	345
	346
	356
	456
	*/
}

void usage_of_other_next_pcomb()
{
	std::string original_text = "1234567890ABCDEF";
	std::string std_combined = "12345678";
	uint64_t total = 0;
	if (concurrent_comb::compute_total_comb(original_text.size(), std_combined.size(), total))
	{
		timer stopwatch;
		stopwatch.start("next_pcomb");

		//std::cout << std_combined << std::endl;
		for (uint64_t i = 1; i < total; ++i)
		{
			other::other_next_combination1(original_text.begin(), original_text.begin()+ std_combined.size(), original_text.end());
			//std::cout << std_combined << std::endl;
		}
		stopwatch.stop();
	}
	std::cout << original_text << std::endl;
	/* output
	123
	124
	125
	126
	134
	135
	136
	145
	146
	156
	234
	235
	236
	245
	246
	256
	345
	346
	356
	456
	*/
}

void usage_of_next_comb_with_state()
{
	std::string original_text = "1234567890ABCDEFGHIJKLMNO";
	std::string std_combined = "12345678";
	uint64_t total = 0;
	std::vector<std::string::iterator> state;
	std::string::iterator it = original_text.begin();
	for (size_t j = 0; j < std_combined.size(); ++j, ++it)
	{
		state.push_back(it);
	}
	if (concurrent_comb::compute_total_comb(original_text.size(), std_combined.size(), total))
	{
		timer stopwatch;
		stopwatch.start("next_comb_with_state");

		//std::cout << std_combined << std::endl;
		for (uint64_t i = 1; i < total; ++i)
		{
			stdcomb::next_combination_with_state(original_text.begin(), original_text.end(), state.begin(), state.end());
			//std::cout << std_combined << std::endl;
		}
		stopwatch.stop();

	}
	for (size_t j = 0; j < std_combined.size(); ++j, ++it)
	{
		std_combined[j] = *state[j];
	}
	std::cout << std_combined << std::endl;
	/* output
	123
	124
	125
	126
	134
	135
	136
	145
	146
	156
	234
	235
	236
	245
	246
	256
	345
	346
	356
	456
	*/
}

void usage_of_comb_by_idx()
{
	uint64_t index_to_find = 0;
	std::string original_text = "123456";
	std::string std_combined = "123";
	std::vector<std::string> all_results;
	size_t repeat_times = 5;
	for (size_t i = 0; i < 4; ++i)
	{
		std::string combined = concurrent_comb::find_comb_by_idx(std_combined.size(), index_to_find, original_text);

		std::thread th([original_text, combined, &all_results, repeat_times] () mutable {

			// do your work on the combined result, instead of pushing to vector
			all_results.push_back(combined);
			for (size_t j = 1; j < repeat_times; ++j)
			{
				stdcomb::next_combination(original_text.begin(), original_text.end(), combined.begin(), combined.end());
				// do your work on the combined result, instead of pushing to vector
				all_results.push_back(combined);
			}
		});
		th.join();

		index_to_find += repeat_times;
	}

	// Checking if all_results is correct
	std_combined = "123";
	for (size_t i = 0; i < all_results.size(); ++i)
	{
		std::cout << all_results[i] << std::endl;
		if (compare_vec(all_results[i], std_combined) == false)
		{
			std::cout << "string not equal at index: " << i << std::endl;
			break;
		}
		stdcomb::next_combination(original_text.begin(), original_text.end(), std_combined.begin(), std_combined.end());
	}

}
