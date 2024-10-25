#ifndef _COMMON_MISC_H_
#define _COMMON_MISC_H_

template<bool x> struct STATIC_ASSERTION_FAILURE;
template<> struct STATIC_ASSERTION_FAILURE<true>{};
template<int x> struct static_assert_test{};
#define STATIC_ASSERT(B) typedef static_assert_test<sizeof(STATIC_ASSERTION_FAILURE<(bool)(B)>)> static_assert_typedef_ ## __LINE__ __attribute__((unused))

#endif
