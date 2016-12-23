## Boost Concurrent Permutations and Combinations on CPU

### Primary motivation

Upcoming C++17 Concurrent STL algorithms does not include parallel next_permutation and next_combination. compute_all_perm and compute_all_comb make use of next_permutation and next_combination underneath to find all the permutations and combinations. Right now, function overload with predicate is not available. So < operator and == operator must be provided for next_permutation and next_combination respectively.

**Note**: Work is still ongoing. Library is not yet submitted for Boost review and is not part of Boost Library.

### Requirement
C++11

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

### Formulae

```cpp
Total Permutation: n!
Total Combination: n! / r! (n - r)! 
```

Use compute_factorial for total permutation.
Use find_total_comb for total combination.

### Limitation

next_permutation supports duplicate elements but compute_all_perm and compute_all_comb do not. Make sure every element is unique.

### Examples

Example on compute_all_perm

```cpp
#include "../permcomb/concurrent_perm.h"

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    int64_t thread_cnt = 2;

    typedef empty_callback_t<decltype(results)> callback_t;
    concurrent_perm::compute_all_perm(thread_cnt, results, 
		[](const int thread_index, const std::string& cont) 
			{return true;} );
}
```

Example on compute_all_comb

```cpp
#include "../permcomb/concurrent_comb.h"

void main()
{
    std::vector<int> fullset_vec(20);
    std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
    uint32_t subset = 10;
    
    int64_t thread_cnt = 1;
    
    typedef empty_callback_t<decltype(fullset_vec)> callback_t;
    concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, 
		[] (const int thread_index, uint32_t fullset,
			uint32_t subset, const std::vector<int>& cont) 
			{return true;});
}
```

### Why not pass in begin and end iterators?

Library need to know the container type to instantiate a copy in the worker thread. From the iterator type, we have no way to know the container. iterator type is not compatible: for example string and vector iterator are not interchangeable.

### Cancellation

Cancellation is not directly supported but every callback can return false to stop processing.

### Benchmark results

Note: there is some overhead.

```
Results for permutation of 11 elements:
Total permutations computed: 39,916,800 - 1
int64_t type used.

next_permutation:  163ms
     1 thread(s):  175ms
     2 thread(s):   95ms
     3 thread(s):   48ms
     4 thread(s):   50ms

Results for combination of 14 out of 28 elements:
Total combinations computed: 40,116,600 - 1
int128_t type used.
 
next_combination:  828ms
     1 thread(s): 1271ms
     2 thread(s):  718ms
     3 thread(s):  401ms
     4 thread(s):  354ms
```