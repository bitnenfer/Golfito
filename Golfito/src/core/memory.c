#include "memory.h"
#include "utils.h"
#include "assert.h"
#include <string.h>

typedef struct {
    PageAllocation pageAlloc;
    void* pHead;
    void* pTail;
    size_t usedByteSize;
} ScratchMemory;

typedef struct _FreelistChunkInfo {
    struct _FreelistChunkInfo* pNext;
    size_t chunkSize;
} FreelistChunkInfo;

typedef struct {
    PageAllocation pageAlloc;
    FreelistChunkInfo* pHead;
    FreelistChunkInfo* pTail;
    FreelistChunkInfo* pCurr;
    size_t usedByteSize;
} FreelistMemory;

#define MEM_TO_CHUNK(mem) (FreelistChunkInfo*)((FreelistChunkInfo*)mem - 1)
#define CHUNK_TO_MEM(chunk) (void*)((FreelistChunkInfo*)chunk + 1)

static ScratchMemory gScratchMemory = { 0 };
static FreelistMemory gFreelistMemory = { 0 };
static const size_t kScratchMemoryCapacity = UT_MB(16);
static const size_t kFreelistMemoryCapacity = UT_MB(64);

void mem_initialize(void) {
    DBG_ASSERT(mem_page_alloc(kScratchMemoryCapacity, &gScratchMemory.pageAlloc), "Failed to allocate virtual memory for scratch buffer");
    gScratchMemory.pHead = gScratchMemory.pageAlloc.pAddress;
    gScratchMemory.pTail = gScratchMemory.pHead;
    gScratchMemory.usedByteSize = 0;
    
    DBG_ASSERT(mem_page_alloc(kFreelistMemoryCapacity, &gFreelistMemory.pageAlloc), "Failed to allocate virtual memory for free-list buffer");
    gFreelistMemory.pCurr = gFreelistMemory.pageAlloc.pAddress;
    gFreelistMemory.pHead = NULL;
    gFreelistMemory.pTail = NULL;
    gFreelistMemory.usedByteSize = 0;
}

void mem_shutdown(void) {
    mem_page_free(&gScratchMemory.pageAlloc);
    mem_page_free(&gFreelistMemory.pageAlloc);
    memset(&gScratchMemory, 0, sizeof(gScratchMemory));
    memset(&gFreelistMemory, 0, sizeof(gFreelistMemory));
}

void* mem_flist_alloc(size_t size, uint32_t alignment) {
    DBG_ASSERT(size < kFreelistMemoryCapacity, "Can't allocate a buffer bigger than the free-list capacity.");
    alignment = alignment < MEM_DEFAULT_ALIGNMENT ? MEM_DEFAULT_ALIGNMENT : alignment;
    if (gFreelistMemory.pHead != NULL) {
        FreelistChunkInfo* pInfo = gFreelistMemory.pHead;
        FreelistChunkInfo* pPrev = NULL;
        
        while (pInfo != NULL) {
            if (pInfo->chunkSize >= size && UT_IS_POINTER_ALIGNED(CHUNK_TO_MEM(pInfo), alignment))
            {
                if (pPrev != NULL) {
                    pPrev->pNext = pInfo->pNext;
                    pInfo->pNext = NULL;
                } else if (pInfo->pNext != NULL) {
                    gFreelistMemory.pHead = pInfo->pNext;
                } else {
                    gFreelistMemory.pHead = NULL;
                    gFreelistMemory.pTail = NULL;
                }
                void* p = CHUNK_TO_MEM(pInfo);
#if defined(_DEBUG)
                memset(p, 0xAA, pInfo->chunkSize);
#endif
                gFreelistMemory.usedByteSize += pInfo->chunkSize;
                return p;
            }
            pPrev = pInfo;
            pInfo = pInfo->pNext;
        }
    }
    
    void* p = UT_ALIGN_POINTER(UT_FORWARD_POINTER(gFreelistMemory.pCurr, sizeof(FreelistChunkInfo)), alignment);
    if (UT_IN_RANGE(p, gFreelistMemory.pageAlloc.pAddress, UT_FORWARD_POINTER(gFreelistMemory.pageAlloc.pAddress, kFreelistMemoryCapacity))) {
        FreelistChunkInfo* pInfo = MEM_TO_CHUNK(p);
        pInfo->chunkSize = size;
        pInfo->pNext = NULL;
#if defined(_DEBUG)
        memset(p, 0xAA, size);
#endif
        gFreelistMemory.usedByteSize += size;
        gFreelistMemory.pCurr = UT_FORWARD_POINTER(p, size);
        return p;
    }
    return NULL;
}

void mem_flist_free(void* p) {
    if (UT_IN_RANGE(p, gFreelistMemory.pageAlloc.pAddress, UT_FORWARD_POINTER(gFreelistMemory.pageAlloc.pAddress, kFreelistMemoryCapacity))) {
        FreelistChunkInfo* pInfo = MEM_TO_CHUNK(p);
        if (gFreelistMemory.pTail != NULL) {
            pInfo->pNext = NULL;
            gFreelistMemory.pTail->pNext = pInfo;
            gFreelistMemory.pTail = pInfo->pNext;
        } else {
            gFreelistMemory.pHead = pInfo;
            gFreelistMemory.pTail = gFreelistMemory.pHead;
        }
        gFreelistMemory.usedByteSize -= pInfo->chunkSize;
#if defined(_DEBUG)
        memset(p, 0xDD, pInfo->chunkSize);
#endif
    }
}

void* mem_scratch_alloc(size_t size, uint32_t alignment) {
    DBG_ASSERT(size < kScratchMemoryCapacity, "Can't allocate a buffer bigger than the scratch capacity.");
    alignment = alignment < MEM_DEFAULT_ALIGNMENT ? MEM_DEFAULT_ALIGNMENT : alignment;
    void* p = UT_ALIGN_POINTER(gScratchMemory.pTail, alignment);
    if (UT_IN_RANGE(UT_FORWARD_POINTER(p, size), gScratchMemory.pHead, UT_FORWARD_POINTER(gScratchMemory.pHead, kScratchMemoryCapacity))) {
        gScratchMemory.pTail = UT_FORWARD_POINTER(p, size);
        gScratchMemory.usedByteSize += size;
#if defined(_DEBUG)
        memset(p, 0xAA, size);
#endif
        return p;
    }
    return NULL;
}

void mem_scratch_reset() {
#if defined(_DEBUG)
    if (gScratchMemory.usedByteSize > 0)
        memset(gScratchMemory.pHead, 0xDD, gScratchMemory.usedByteSize);
#endif
    gScratchMemory.usedByteSize = 0;
    gScratchMemory.pTail = gScratchMemory.pHead;
}

size_t mem_flist_used_size(void) {
    return gFreelistMemory.usedByteSize;
}

size_t mem_scratch_used_size(void) {
    return gScratchMemory.usedByteSize;
}
