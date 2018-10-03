#include "memory.h"
#include "utils.h"
#include "assert.h"
#include <string.h>

#define MEM_TO_CHUNK(mem) (FreelistChunkInfo*)((FreelistChunkInfo*)mem - 1)
#define CHUNK_TO_MEM(chunk) (void*)((FreelistChunkInfo*)chunk + 1)

#if MEM_DEFAULT_ALIGNMENT == 4
#define POOL_COUNT 9
static size_t kPoolSizes[POOL_COUNT] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
static uint32_t kPoolCapacities[POOL_COUNT] = { UT_MB(1), UT_MB(1), UT_MB(1), UT_MB(1), UT_MB(1), UT_MB(2), UT_MB(4), UT_MB(8), UT_MB(16) };
#else
#define POOL_COUNT 8
static size_t kPoolSizes[POOL_COUNT] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
static uint32_t kPoolCapacities[POOL_COUNT] = { UT_MB(1), UT_MB(1), UT_MB(1), UT_MB(1), UT_MB(2), UT_MB(4), UT_MB(8), UT_MB(16) };
#endif

static const size_t kMemLinearContextCapacity = UT_MB(16);

typedef struct {
    PageAllocation pageAlloc;
    void** pAddresses;
    size_t capacity;
    size_t count;
} MemAddrStack;

typedef struct {
    MemAddrStack freeStack;
    PageAllocation pageAlloc;
    void* pHead;
    void* pCurr;
    size_t elementSize;
    size_t usedByteSize;
} MemPool;

typedef struct {
    MemPool pools[POOL_COUNT];
} MemPoolContext;

static MemLinearContext gMemLinearContext = { 0 };
static MemPoolContext gMemPoolContext = { 0 };
static MemLinearContext* pCurrentLinearContext = NULL;

void mem_initialize(void) {
    DBG_ASSERT(mem_page_alloc(kMemLinearContextCapacity, &gMemLinearContext.pageAlloc), "Failed to allocate virtual memory for scratch buffer");
    gMemLinearContext.pHead = gMemLinearContext.pageAlloc.pAddress;
    gMemLinearContext.pCurr = gMemLinearContext.pHead;
    gMemLinearContext.usedByteSize = 0;
    
    for (uint32_t index = 0; index < POOL_COUNT; ++index) {
        MemPool* pPool = &gMemPoolContext.pools[index];
        size_t elementSize = kPoolSizes[index];
        size_t poolCapacity = kPoolCapacities[index];
        size_t stackCapacity = poolCapacity / elementSize;
        size_t stackByteSize = stackCapacity * sizeof(void*);
        pPool->elementSize = elementSize;
        pPool->usedByteSize = 0;
        pPool->freeStack.count = 0;
        pPool->freeStack.capacity = stackCapacity;
        DBG_ASSERT(mem_page_alloc(poolCapacity, &pPool->pageAlloc), "Failed to allocate virtual memory for pool of size %zu", elementSize);
        DBG_ASSERT(mem_page_alloc(stackByteSize, &pPool->freeStack.pageAlloc), "Failed to allocate virtual memory for pool's free stack of size %zu", elementSize);
        pPool->freeStack.pAddresses = (void**)pPool->freeStack.pageAlloc.pAddress;
        pPool->pHead = pPool->pageAlloc.pAddress;
        pPool->pCurr = pPool->pHead;
    }
    
    mem_linear_set_default_context();
}

void mem_shutdown(void) {
    mem_page_free(&pCurrentLinearContext->pageAlloc);
    memset(&gMemLinearContext, 0, sizeof(gMemLinearContext));

    for (uint32_t index = 0; index < POOL_COUNT; ++index) {
        MemPool* pPool = &gMemPoolContext.pools[index];
        mem_page_free(&pPool->pageAlloc);
    }
    memset(&gMemPoolContext, 0, sizeof(MemPoolContext));
}

void mem_linear_set_context(MemLinearContext* pContext) {
    pCurrentLinearContext = pContext;
}

void mem_linear_set_default_context(void) {
    pCurrentLinearContext = &gMemLinearContext;
}

void* mem_linear_alloc(size_t size, uint32_t alignment) {
    DBG_ASSERT(size < kMemLinearContextCapacity, "Can't allocate a buffer bigger than the scratch capacity.");
    alignment = alignment < MEM_DEFAULT_ALIGNMENT ? MEM_DEFAULT_ALIGNMENT : alignment;
    void* p = UT_ALIGN_POINTER(pCurrentLinearContext->pCurr, alignment);
    if (UT_IN_RANGE(UT_FORWARD_POINTER(p, size), pCurrentLinearContext->pHead, UT_FORWARD_POINTER(pCurrentLinearContext->pHead, kMemLinearContextCapacity))) {
        pCurrentLinearContext->pCurr = UT_FORWARD_POINTER(p, size);
        pCurrentLinearContext->usedByteSize += size;
#if defined(_DEBUG)
        memset(p, 0xAA, size);
#endif
        return p;
    }
    return NULL;
}

void mem_linear_reset() {
#if defined(_DEBUG)
    if (pCurrentLinearContext->usedByteSize > 0)
        memset(pCurrentLinearContext->pHead, 0xDD, pCurrentLinearContext->usedByteSize);
#endif
    pCurrentLinearContext->usedByteSize = 0;
    pCurrentLinearContext->pCurr = pCurrentLinearContext->pHead;
}

size_t mem_linear_used_size(void) {
    return pCurrentLinearContext->usedByteSize;
}

void* mem_pool_alloc(size_t size) {
    if (size < kPoolSizes[0]) size = kPoolSizes[0];
    UT_ROUND_POT(size);
    if (size <= kPoolSizes[POOL_COUNT - 1]) {
        uint32_t poolIndex = 0;
        if (size < kPoolSizes[POOL_COUNT / 2]) {
            for (poolIndex = 0; poolIndex < POOL_COUNT; ++poolIndex) {
                if (size == kPoolSizes[poolIndex]) break;
            }
        } else {
            for (poolIndex = POOL_COUNT - 1; poolIndex >= 0; --poolIndex) {
                if (size == kPoolSizes[poolIndex]) break;
            }
        }
        MemPool* pPool = &gMemPoolContext.pools[poolIndex];
        if (pPool->freeStack.count > 0) {
            void* pFreeAddress = pPool->freeStack.pAddresses[--pPool->freeStack.count];
            return pFreeAddress;
        }
        void* pNewAddress = pPool->pCurr;
        void* pNextAddress = UT_FORWARD_POINTER(pNewAddress, size);
        if (UT_IN_RANGE(pNextAddress, pPool->pHead, UT_FORWARD_POINTER(pPool->pHead, pPool->pageAlloc.size))) {
            pPool->pCurr = pNextAddress;
            pPool->usedByteSize += size;
#if defined(_DEBUG)
            memset(pNewAddress, 0xAA, size);
#endif
            return pNewAddress;
        }
    }
    return NULL;
}
void mem_pool_free(void* p) {
    for (uint32_t index = 0; index < POOL_COUNT; ++index) {
        MemPool* pPool = &gMemPoolContext.pools[index];
        void* pHead = pPool->pHead;
        if (UT_IN_RANGE(p, pHead, UT_FORWARD_POINTER(pHead, pPool->pageAlloc.size))) {
            pPool->freeStack.pAddresses[pPool->freeStack.count++] = p;
            pPool->usedByteSize -= pPool->elementSize;
#if defined(_DEBUG)
            memset(p, 0xDD, pPool->elementSize);
#endif
            return;
        }
    }
    DBG_ASSERT(0, "Trying de free memory that doesn't belong to any pool on the current context");
}

size_t mem_pool_used_size(void) {
    size_t size = 0;
    for (uint32_t index = 0; index < POOL_COUNT; ++index) {
        size += gMemPoolContext.pools[index].usedByteSize;
    }
    return size;
}
