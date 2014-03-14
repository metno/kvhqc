
#ifndef SET_DIFFERENCES_HH
#define SET_DIFFERENCES_HH 1

#include <algorithm> // std::copy
#include <utility>   // std::pair

/**
 * Produce two set differences: result1 will receive all elements that
 * are in input range 1, but not in input range 2; and result2 will
 * receive all that are in input range 2 but not in input range 1.
 */
template<typename Input1,  typename Input2,
         typename Output1, typename Output2, typename Compare>
std::pair<Output1, Output2>
set_differences(Input1 first1, Input1 last1, Input2 first2, Input2 last2,
    Output1 result1, Output2 result2, Compare comp)
{
  while (first1 != last1 && first2 != last2) {
    if (comp(*first1, *first2)) {
      *result1 = *first1;
      ++first1;
      ++result1;
    } else if (comp(*first2, *first1)) {
      *result2 = *first2;
      ++first2;
      ++result2;
    } else {
      ++first1;
      ++first2;
    }
  }
  result1 = std::copy(first1, last1, result1);
  result2 = std::copy(first2, last2, result2);
  return std::make_pair(result1, result2);
}

/**
 * Produce two set differences: result1 will receive all elements that
 * are in input range 1, but not in input range 2; and result2 will
 * receive all that are in input range 2 but not in input range 1; and
 * result3 those in both input ranges.
 */
template<typename Input1,  typename Input2,
         typename Output1, typename Output2, typename Output3,
         typename Compare>
void set_differences(Input1 first1, Input1 last1, Input2 first2, Input2 last2,
    Output1 result1, Output2 result2, Output3 result3, Compare comp)
{
  while (first1 != last1 && first2 != last2) {
    if (comp(*first1, *first2)) {
      *result1 = *first1;
      ++first1;
      ++result1;
    } else if (comp(*first2, *first1)) {
      *result2 = *first2;
      ++first2;
      ++result2;
    } else {
      *result3 = *first1; // *first1 and *first2 are equal
      ++first1;
      ++first2;
      ++result3;
    }
  }
  result1 = std::copy(first1, last1, result1);
  result2 = std::copy(first2, last2, result2);
}

#endif // SET_DIFFERENCES_HH
