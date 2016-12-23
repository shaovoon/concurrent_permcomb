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
typedef boost::multiprecision::cpp_int int_type;
// 256 bit integer
typedef boost::multiprecision::int256_t int_type;
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

// callback can return false to stop processing
template<typename int_type, typename container_type>
struct empty_callback_t
{
    // thread_index is zero based and consecutive.
    // Example for 2 threads, thread_index is 0 and 1
    bool operator()(const int thread_index, const container_type& cont)
    {
        return true;
    }
};

void main()
{
    std::string results(11, 'A');
    std::iota(results.begin(), results.end(), 'A');
    
    typedef int64_t int_type;
    int_type thread_cnt = 2;

    typedef empty_callback_t<int_type, decltype(results)> callback_t;
    concurrent_perm::compute_all_perm(thread_cnt, results, callback_t());
}
```

Example on compute_all_comb

```cpp
#include "../permcomb/concurrent_comb.h"

// callback can return false to stop processing
template<typename int_type, typename container_type>
struct empty_callback_t
{
    // thread_index is zero based and consecutive.
    // Example for 2 threads, thread_index is 0 and 1
    bool operator()(const int thread_index, uint32_t fullset,
        uint32_t subset, const container_type& cont)
    {
        return true;
    }
};

void main()
{
    std::vector<int> fullset_vec(20);
    std::iota(fullset_vec.begin(), fullset_vec.end(), 0);
    uint32_t subset = 10;
    
    typedef int64_t int_type;
    int_type thread_cnt = 1;
    
    typedef empty_callback_t<int_type, decltype(fullset_vec)> callback_t;
    concurrent_comb::compute_all_comb(thread_cnt, subset, fullset_vec, callback_t());
}
```

### Why not pass in begin and end iterators?

Library need to know the container type to instantiate a copy in the worker thread. From the iterator type, we have no way to know the container. iterator type is not compatible: for example string and vector iterator are not interchangeable.

### Cancellation

Cancellation is not directly supported but every callback can return false to stop processing.

### Benchmark results

```

```