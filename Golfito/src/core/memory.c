#include "memory.h"
#include "utils.h"
#include "assert.h"
#include <string.h>

#define MEM_TO_CHUNK(mem) (FreelistChunkInfo*)((FreelistChunkInfo*)mem - 1)
#define CHUNK_TO_MEM(chunk) (void*)((FreelistChunkInfo*)chunk + 1)

static MemLinearContext gMemLinearContext = { 0 };
static MemFreeListContext gMemFreeListContext = { 0 };
static const size_t kMemLinearContextCapacity = UT_MB(16);
static const size_t kMemFreeListContextCapacity = UT_MB(64);
static MemLinearContext* pCurrentLinearContext = NULL;
static MemFreeListContext* pCurrentFreeListContext = NULL;

void mem_initialize(void) {
    DBG_ASSERT(mem_page_alloc(kMemLinearContextCapacity, &gMemLinearContext.pageAlloc), "Failed to allocate virtual memory for scratch buffer");
    gMemLinearContext.pHead = gMemLinearContext.pageAlloc.pAddress;
    gMemLinearContext.pTail = gMemLinearContext.pHead;
    gMemLinearContext.usedByteSize = 0;
    
    DBG_ASSERT(mem_page_alloc(kMemFreeListContextCapacity, &gMemFreeListContext.pageAlloc), "Failed to allocate virtual memory for free-list buffer");
    gMemFreeListContext.pCurr = gMemFreeListContext.pageAlloc.pAddress;
    gMemFreeListContext.pHead = NULL;
    gMemFreeListContext.pTail = NULL;
    gMemFreeListContext.usedByteSize = 0;
    
    mem_linear_set_default_context();
    mem_freelist_set_default_context();
}

void mem_shutdown(void) {
    mem_page_free(&pCurrentLinearContext->pageAlloc);
    mem_page_free(&pCurrentFreeListContext->pageAlloc);
    memset(&gMemLinearContext, 0, sizeof(gMemLinearContext));
    memset(&gMemFreeListContext, 0, sizeof(gMemFreeListContext));
}

void mem_freelist_set_context(MemFreeListContext* pContext) {
    pCurrentFreeListContext = pContext;
}

void mem_freelist_set_default_context(void) {
    pCurrentFreeListContext = &gMemFreeListContext;
}

void* mem_freelist_alloc(size_t size, uint32_t alignment) {
    DBG_ASSERT(size < kMemFreeListContextCapacity, "Can't allocate a buffer bigger than the free-list capacity.");
    alignment = alignment < MEM_DEFAULT_ALIGNMENT ? MEM_DEFAULT_ALIGNMENT : alignment;
    if (pCurrentFreeListContext->pHead != NULL) {
        FreelistChunkInfo* pInfo = pCurrentFreeListContext->pHead;
        FreelistChunkInfo* pPrev = NULL;
        
        while (pInfo != NULL) {
            if (pInfo->chunkSize >= size && UT_IS_POINTER_ALIGNED(CHUNK_TO_MEM(pInfo), alignment))
            {
                if (pPrev != NULL) {
                    pPrev->pNext = pInfo->pNext;
                    pInfo->pNext = NULL;
                } else if (pInfo->pNext != NULL) {
                    pCurrentFreeListContext->pHead = pInfo->pNext;
                } else {
                    pCurrentFreeListContext->pHead = NULL;
                    pCurrentFreeListContext->pTail = NULL;
                }
                void* p = CHUNK_TO_MEM(pInfo);
#if defined(_DEBUG)
                memset(p, 0xAA, pInfo->chunkSize);
#endif
                pCurrentFreeListContext->usedByteSize += pInfo->chunkSize;
                return p;
            }
            pPrev = pInfo;
            pInfo = pInfo->pNext;
        }
    }
    
    void* p = UT_ALIGN_POINTER(UT_FORWARD_POINTER(pCurrentFreeListContext->pCurr, sizeof(FreelistChunkInfo)), alignment);
    if (UT_IN_RANGE(p, pCurrentFreeListContext->pageAlloc.pAddress, UT_FORWARD_POINTER(pCurrentFreeListContext->pageAlloc.pAddress, kMemFreeListContextCapacity))) {
        FreelistChunkInfo* pInfo = MEM_TO_CHUNK(p);
        pInfo->chunkSize = size;
        pInfo->pNext = NULL;
#if defined(_DEBUG)
        memset(p, 0xAA, size);
#endif
        pCurrentFreeListContext->usedByteSize += size;
        pCurrentFreeListContext->pCurr = UT_FORWARD_POINTER(p, size);
        return p;
    }
    return NULL;
}

void mem_freelist_free(void* p) {
    if (UT_IN_RANGE(p, pCurrentFreeListContext->pageAlloc.pAddress, UT_FORWARD_POINTER(pCurrentFreeListContext->pageAlloc.pAddress, kMemFreeListContextCapacity))) {
        FreelistChunkInfo* pInfo = MEM_TO_CHUNK(p);
        if (pCurrentFreeListContext->pTail != NULL) {
            pInfo->pNext = NULL;
            pCurrentFreeListContext->pTail->pNext = pInfo;
            pCurrentFreeListContext->pTail = pInfo->pNext;
        } else {
            pCurrentFreeListContext->pHead = pInfo;
            pCurrentFreeListContext->pTail = pCurrentFreeListContext->pHead;
        }
        pCurrentFreeListContext->usedByteSize -= pInfo->chunkSize;
#if defined(_DEBUG)
        memset(p, 0xDD, pInfo->chunkSize);
#endif
    }
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
    void* p = UT_ALIGN_POINTER(pCurrentLinearContext->pTail, alignment);
    if (UT_IN_RANGE(UT_FORWARD_POINTER(p, size), pCurrentLinearContext->pHead, UT_FORWARD_POINTER(pCurrentLinearContext->pHead, kMemLinearContextCapacity))) {
        pCurrentLinearContext->pTail = UT_FORWARD_POINTER(p, size);
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
    pCurrentLinearContext->pTail = pCurrentLinearContext->pHead;
}

size_t mem_freelist_used_size(void) {
    return pCurrentFreeListContext->usedByteSize;
}

size_t mem_linear_used_size(void) {
    return pCurrentLinearContext->usedByteSize;
}
