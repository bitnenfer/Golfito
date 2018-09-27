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

void mem_initialize(void);
void mem_shutdown(void);
bool32_t mem_page_alloc(size_t size, PageAllocation* pAllocationInfo);
bool32_t mem_page_free(const PageAllocation* pAllocationInfo);
void* mem_flist_alloc(size_t size, uint32_t alignment);
void mem_flist_free(void* p);
void* mem_scratch_alloc(size_t size, uint32_t alignment);
void mem_scratch_reset(void);
size_t mem_flist_used_size(void);
size_t mem_scratch_used_size(void);
size_t mem_system_page_size(void);

#endif
