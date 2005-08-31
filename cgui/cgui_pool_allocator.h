/*
 *	$Header$
 *
 *	Pool allocator for small memory blocks.  UGL tends to allocate many small chunks of memory.
 * Using a pool allocator for this significantly improves performance and reduces memory
 * fragmentation associated with long run times.
 *
 * $Log$
 *
 */

#ifndef _CGUI_POOL_ALLOCATOR_INCLUDE
#define _CGUI_POOL_ALLOCATOR_INCLUDE

// Set up pool allocator for UGL.  Must be called before initializing UGL.
//
void cguiPoolAllocatorInitialize(size_t blockCount);

#endif /* ifndef _CGUI_POOL_ALLOCATOR_INCLUDE */
