#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "types.h"

#if (defined(UINTPTR_MAX) && UINTPTR_MAX == UINT32_MAX)
#define MEM_DEFAULT_ALIGNMENT 4
#elif defined(UINTPTR_MAX) && UINTPTR_MAX == UINT64_MAX
#define MEM_DEFAULT_ALIGNMENT 8
#else
#error "Invalid Platform Architecture"
#endif

typedef struct {
    void* pAddress;
    size_t size;
} PageAllocation;

typedef struct _LinearContext {
    PageAllocation pageAlloc;
    void* pHead;
    void* pCurr;
    size_t usedByteSize;
} MemLinearContext;

void mem_initialize(void);
void mem_shutdown(void);
bool32_t mem_page_alloc(size_t size, PageAllocation* pAllocationInfo);
bool32_t mem_page_free(const PageAllocation* pAllocationInfo);
void mem_linear_set_context(MemLinearContext* pContext);
void* mem_linear_alloc(size_t size, uint32_t alignment);
void mem_linear_reset(void);
void mem_linear_set_default_context(void);
void* mem_pool_alloc(size_t size);
void mem_pool_free(void* p);
size_t mem_pool_used_size(void);
size_t mem_linear_used_size(void);
size_t mem_system_page_size(void);

#endif
