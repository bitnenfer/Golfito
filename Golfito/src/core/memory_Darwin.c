#include "memory.h"
#include "utils.h"
#include <mach/mach.h>

size_t mem_system_page_size(void) {
    vm_size_t size = 0;
    host_page_size(mach_task_self(), &size);
    return (size_t)size;
}

bool32_t mem_page_alloc(size_t size, PageAllocation* pAllocationInfo) {
    vm_address_t address = 0;
    vm_size_t pageSize = round_page(size);
    kern_return_t result = KERN_SUCCESS;
    result = vm_allocate(mach_task_self(), &address, pageSize, VM_FLAGS_ANYWHERE);
    if (result != KERN_SUCCESS) return UT_FALSE;
    pAllocationInfo->pAddress = (void*)address;
    pAllocationInfo->size = pageSize;
    return UT_TRUE;
}

bool32_t mem_page_free(const PageAllocation* pAllocationInfo) {
    kern_return_t result = KERN_SUCCESS;
    result = vm_deallocate(mach_task_self(), (vm_address_t)pAllocationInfo->pAddress, (vm_size_t)pAllocationInfo->size);
    if (result != KERN_SUCCESS) return UT_FALSE;
    return UT_TRUE;
}
