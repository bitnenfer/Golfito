#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#define UT_FALSE 0
#define UT_TRUE 1
#define UT_INVALID_HANDLE NULL
#define UT_SET_TRUE(value, bit) value = ((value) | (1 << (bit)))
#define UT_SET_FALSE(value, bit) value = ((value) & ~(1 << (bit)))
#define UT_IS_TRUE(value, bit) (((value) >> (bit)) & 1)
#define UT_IS_FALSE(value, bit) !UT_IS_TRUE(value, bit)
#define UT_TOGGLE_BIT(value, bit) value = ((value) ^ (1 << (bit)))
#define UT_CLAMP(a, b, c) ((a) < (b) ? (b) : ((a) > (c) ? (c) : (a)))
#define UT_TO_POINTER(value) ((void*)(value))
#define UT_FORWARD_POINTER(pointer, offset) ((void*)((uintptr_t)UT_TO_POINTER(pointer) + offset))
#define UT_POINTER_TO_UINT(pointer) ((uintptr_t)(pointer))
#define UT_ALIGN_POINTER(pointer, base) UT_TO_POINTER(((UT_POINTER_TO_UINT(pointer))+((base)-1L)) & ~((base)-1L))
#define UT_IS_POINTER_ALIGNED(pointer, alignment) (((uintptr_t)pointer & (uintptr_t)(alignment - 1L)) == 0)
#define UT_KB(value) (value * 1024)
#define UT_MB(value) (UT_KB(value) * 1024)
#define UT_GB(value) ((UT_MB((uint64_t)value) * 1024))
#define UT_UNUSED(x) ((void)&x)
#define UT_OFFSETOF(type, member) ((size_t)&(((type*)0)->member))
#define UT_IN_RANGE(value, min_value, max_value) (value >= min_value && value <= max_value)
#define UT_IS_POT(x) (((x != 0) && ((x & (~x + 1)) == x)))
#define UT_ENABLED(x) (bool32_t)((x) ? 1 : 0)
#define UT_ROUND_POT(x) {\
(x)--;\
(x) |= (x) >> 1;\
(x) |= (x) >> 2;\
(x) |= (x) >> 4;\
(x) |= (x) >> 8;\
(x) |= (x) >> 16;\
(x)++;\
}

#endif
