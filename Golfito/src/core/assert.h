#ifndef _CORE_ASSERT_H_
#define _CORE_ASSERT_H_


#if defined(_DEBUG)
#include <stdio.h>
#include <stdlib.h>

#if defined(TARGET_MACOS)
#define _BREAKPOINT() asm("ud2a\n")
#elif defined(TARGET_IOS) || defined(TARGET_TVOS)
#include <signal.h>
#define _BREAKPOINT() raise(SIGTRAP)
#else
#define _BREAKPOINT __debugbreak
#endif

#define DBG_ASSERT(x, msg, ...) if (!(x)) { fprintf(stderr, "!!! Failed Assert: " msg " !!!\n", ##__VA_ARGS__); _BREAKPOINT(); exit(-1); }

#else
#define _BREAKPOINT()
#define DBG_ASSERT(x, msg)
#endif

#endif
