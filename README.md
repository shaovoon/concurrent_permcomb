## Boost Concurrent Permutations and Combinations on CPU

### Primary motivation

Upcoming C++17 Concurrent STL algorithms does not include parallel `next_permutation` and `next_combination`. `compute_all_perm` and `compute_all_comb` make use of `next_permutation` and `next_combination` underneath to find all the permutations and combinations. Many years ago, I had written parallel library which only deals with integer type but it exposes many internal workings. This one encapsulates the details and can permute/combine with any data types.

**Note**: Function overload with predicate is available.

**Note**: Work is still ongoing. Library is not yet submitted for Boost review and is not part of Boost Library.

### Requirement

**Required**: C++11

**Optional**: [Boost Multiprecision](http://www.boost.org/doc/libs/1_62_0/libs/multiprecision/doc/html/index.html)
**Note**: Library need Boost Multiprecision for factorial(n) where n > 20. You either use the arbitrary integer(most safe) or fixed width big integer to accommodate the largest factorial. 

```cpp
// arbitrary integer
boost::multiprecision::cpp_int;
// 256 bit integer
boost::multiprecision::int256_t;
```

### Compiler tested
- Visual C++ 2015
- GCC 5.4 Ubuntu 16.04
- Clang 3.8 Ubuntu 16.04

### Build examples with GCC and Clang

```
g++     CalcPerm.cpp -std=c++11 -lpthread -O2
g++     CalcComb.cpp -std=c++11 -lpthread -O2

clang++ CalcPerm.cpp -std=c++11 -lpthread -O2
clang++ CalcComb.cpp -std=c++11 -lpthread -O2
```

### No CMakeList?

This is header-only library.

### Formulae to calculate total results returned.

Calculate this in a calculator and use sum to determine the largest integer type to be used.

```cpp
Total Permutation: n!
Total Combination: n! / (r! (n - r)!)
```

Use `compute_factorial`  to calculate total permutation count.

Use `compute_total_comb` to calculate total combination count.

### Limitation

`next_permutation` supports duplicate elements but `compute_all_perm` and `compute_all_comb` do not. Make sure every element is unique. Also make sure total results are greater than number of threads spawned.

### Examples

`compute_all_perm` function and callback signatures shown below. Callback should catch all exceptions and return false. If exception propagate outside callback, error_callback will be invoked and processing will be stopped prematurely for the thread.

```cpp
// compute_all_perm function signature
template<typename int_type, typename container_type, typename callback_type, 
    typename error_callback_type, typename predicate_type = no_predicate_type>
bool compute_all_perm(int_type thread_cnt, const container_type& cont, callback_type callback, 
    error_callback_type err_callback, predicate_type pred = predicate_type());

// callback_type signature
template<typename container_type>
struct callback_t
{
    bool operator()(const int thread_index, const container_type& cont)
    {
        return true; // can return false to cancel processing in current thread
    }
};

// error_callback_type example
template<typename container_type>
struct error_callback_t
{
    void operator()(const int thread_index, const container_type& cont, const std::string& error)
    {
        std::cerr << error << std::endl;
    }
};

// predicate_type example
template<typename T>
struct predicate_t
{
    bool operator()(T a, T b)
    {
        return a < b;
    }
};
```

Example on how to use `compute_all_perm`

```cpp
#include "../permcomb/concurrent_perm.h"

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    int64_t thread_cnt = 2;

    concurrent_perm::compute_all_perm(thread_cnt, results, 
        [](const int thread_index, const std::string& cont) 
            { return true; } /* evaluation callback */,
        [](const int thread_index, const std::string& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */           
        );
}
```

Example on how to use `compute_all_perm` with predicate

```cpp
#include "../permcomb/concurrent_perm.h"

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    int64_t thread_cnt = 2;

    concurrent_perm::compute_all_perm(thread_cnt, results, 
        [](const int thread_index, const std::string& cont) 
            { return true; } /* evaluation callback */,
        [](const int thread_index, const std::string& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */,            
        [](char a, char b) 
            { return a < b; } /* predicate */
        );
}
```

`compute_all_comb` function and callback signatures shown below. Callback should catch all exceptions and return false. If exception propagate outside callback, error_callback will be invoked and processing will be stopped prematurely for the thread.

```cpp
// compute_all_comb function signature
template<typename int_type, typename container_type, typename callback_type, 
    typename error_callback_type, typename predicate_type = no_predicate_type>
bool compute_all_comb(int_type thread_cnt, uint32_t subset, const container_type& cont, 
    callback_type callback, error_callback_type err_callback, predicate_type pred = predicate_type())

// callback_type signature
template<typename container_type>
struct callback_t
{
    bool operator()(const int thread_index, const size_t fullset_size, const container_type& cont)
    {
        return true; // can return false to cancel processing in current thread
    }
};

// error_callback_type example
template<typename container_type>
struct error_callback_t
{
    void operator()(const int thread_index, const size_t fullset_size, 
        const container_type& cont, const std::string& error)
    {
        std::cerr << error << std::endl;
    }
};


// predicate_type example
template<typename T>
struct predicate_t
{
    bool operator()(T a, T b)
    {
        return a == b;
    }
};
```

Example on how to use `compute_all_comb`

```cpp
#include "../permcomb/concurrent_comb.h"

void main()
{
    std::vector<int> fullset_vec(20);
    std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
    uint32_t subset = 10;
    
    int64_t thread_cnt = 2;
    
    concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, 
        [] (const int thread_index, uint32_t fullset, const std::vector<int>& cont) 
            { return true; } /* evaluation callback */,
        [] (const int thread_index, uint32_t fullset, const std::vector<int>& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */,
        );
}
```

Example on how to use `compute_all_comb` with predicate

```cpp
#include "../permcomb/concurrent_comb.h"

void main()
{
    std::vector<int> fullset_vec(20);
    std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
    uint32_t subset = 10;
    
    int64_t thread_cnt = 2;
    
    concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, 
        [] (const int thread_index, uint32_t fullset, const std::vector<int>& cont) 
            { return true; } /* evaluation callback */,
        [] (const int thread_index, uint32_t fullset, const std::vector<int>& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */,
        [] (int a, int b) 
            { return a == b; } /* predicate */
        );
}
```

### Why not pass in begin and end iterators?

Library need to know the container type to instantiate a copy in the worker thread. From the iterator type, we have no way to know the container. Iterator type is not compatible: for example `string` and `vector` iterator are not interchangeable; It is not right that user pass `string` iterator but library pass `vector` iterator to callback.

### How to make use of thread_index parameter in callback?

`thread_index` is a zero based and consecutive number. For example when `thread_cnt` is 4, then `thread_index` would be [0..3]. Data type of `thread_cnt` has to be a type large enough to hold the largest factorial required.

```cpp
#include "../permcomb/concurrent_perm.h"

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    int64_t thread_cnt = 4;
    int matched[4] = {0,0,0,0};

    concurrent_perm::compute_all_perm(thread_cnt, results, 
        [&matched](const int thread_index, const std::string& cont) /* evaluation callback */
            {
                if(...) 
                    ++matched[thread_index];
                return true;
            },
        [] (const int thread_index, const std::string& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */, 
            
        );
            
    int total_matched = matched[0] + matched[1] + matched[2] + matched[3];
    // display total_matched
}
```

I'll leave to the reader to fix false-sharing in the above example.

### Cancellation

Cancellation is not directly supported but every callback can return `false` to cancel processing.

### How many threads are spawned?

**Answer**: `thread_cnt` - 1. For `thread_cnt` = 4, 3 threads will be spawned while main thread is used to compute the 4th batch. For `thread_cnt` = 1, no threads is spawned, all work is done in the main thread.

### How to split the work across physically separate processors?

Say you have more than 1 computer at home or can access cloud of computers, Work can be split using `compute_all_perm_shard`. In fact `compute_all_perm` calls `compute_all_perm_shard` to do the work as well. `compute_all_perm_shard` has 2 extra parameters which are `cpu_index` and `cpu_cnt`. Value of `cpu_index` can be [0..`cpu_cnt`).

```cpp
#include "../permcomb/concurrent_perm.h"

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    int64_t thread_cnt = 4;
    
    int_type cpu_cnt = 2;
    int_type cpu_index = 0; /* 0 or 1 */
    int cpu_index_n = static_cast<int>(cpu_index);

    concurrent_perm::compute_all_perm_shard(cpu_index, cpu_cnt, thread_cnt, results, 
        [](const int thread_index, const std::string& cont) /* evaluation callback */
            {
                return true;
            },
        [] (const int thread_index, const std::string& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */, 
            
        );
}
```

```cpp
#include "../permcomb/concurrent_comb.h"

void main()
{
    std::vector<uint32_t> fullset(fullset_size);
    std::iota(fullset.begin(), fullset.end(), 0);
    
    int64_t thread_cnt = 4;
    
    int_type cpu_cnt = 2;
    int_type cpu_index = 0; /* 0 or 1 */
    int cpu_index_n = static_cast<int>(cpu_index);

    concurrent_comb::compute_all_comb_shard(cpu_index, cpu_cnt, thread_cnt, subset_size, fullset, 
        [](const int thread_index, const size_t fullset_cnt, const std::vector<uint32_t>& cont) 
            { /* evaluation callback */
                return true;
            },
        [] (const int thread_index, const std::vector<uint32_t>& cont, const std::string& error) 
            { std::cerr << error; } /* error callback */, 
            
        );
}
```

### Benchmark results

Intel i7 6700 CPU with 16 GB RAM with Visual C++ on Windows 10

```
Results for permutation of 11 elements:
Total permutations computed: 39,916,800 - 1
int64_t type used.

next_permutation:  163ms
     1 thread(s):  175ms
     2 thread(s):   95ms
     3 thread(s):   48ms
     4 thread(s):   50ms
```

```
Results for combination of 14 out of 28 elements:
Total combinations computed: 40,116,600 - 1
int128_t type used.
 
next_combination:  789ms
     1 thread(s):  808ms
     2 thread(s):  434ms
     3 thread(s):  316ms
     4 thread(s):  242ms
```

### Diminishing returns on 4 threads

Main suspect is the Intel i7 6700 CPU is a 4 core processor where other applications are running. Need a multicore CPU with more than 4 cores to see whether diminishing perf gain issue persist!
