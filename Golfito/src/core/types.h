#ifndef _GOLFITO_TYPES_H_
#define _GOLFITO_TYPES_H_

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>

typedef float float32_t;
typedef double float64_t;
typedef int32_t bool32_t;
typedef int16_t bool16_t;
typedef int8_t bool8_t;
typedef void* opaque_handle_t;
typedef uint8_t byte_t;

#define GF_FALSE 0
#define GF_TRUE 1
#define GF_INVALID_HANDLE NULL
#define GF_SET_TRUE(value, bit) value = ((value) | (1 << (bit)))
#define GF_SET_FALSE(value, bit) value = ((value) & ~(1 << (bit)))
#define GF_IS_TRUE(value, bit) (((value) >> (bit)) & 1)
#define GF_IS_FALSE(value, bit) !GF_IS_TRUE(value, bit)
#define GF_TOGGLE_BIT(value, bit) value = ((value) ^ (1 << (bit)))

#endif
