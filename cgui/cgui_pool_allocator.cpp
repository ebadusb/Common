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

#include <vxWorks.h>
#include <string.h>

#include "cgui_pool_allocator.h"
#include "error.h"
#include "ugl/uglmem.h"

class CGUIMemPool
{
public:
	CGUIMemPool(unsigned short blockSize, unsigned short blockCount)
	{
		_blockSize = blockSize;
		_blockCount = blockCount;
		_start = malloc((size_t)blockSize * (size_t)blockCount);

		_freeArray = (unsigned short *)malloc(((size_t)blockCount+1) * sizeof(unsigned short));
		for ( int i=0; i<blockCount; i++ ) _freeArray[i] = i;

		_freeStart = 0;
		_freeEnd = blockCount;

		_sem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
	}

	virtual ~CGUIMemPool(void)
	{
		_FATAL_ERROR(__FILE__, __LINE__, "~CGUIMemPool not supported");
	}

	int blockSize(void * blockAddr)
	{
		bool	result = -1;
		if ( blockAddr >= _start )
		{
			size_t	offset = (char *)blockAddr - (char *)_start;
			int	index = offset/_blockSize;
			if ( index < _blockCount )
			{
				if ( (offset % _blockSize) == 0 )
				{
					result = _blockSize;
				}
				else
				{
					_FATAL_ERROR(__FILE__, __LINE__, "bad pointer in pool space");
				}
			}
		}

		return result;
	}

	inline void * alloc(size_t size)
	{
		void * result = NULL;

		if ( size <= _blockSize )
		{
			semTake(_sem, WAIT_FOREVER);
			if ( _freeStart != _freeEnd )
			{
				result = (void *)(((char *)_start) + (size_t)_blockSize*(size_t)_freeArray[_freeStart]);

				_freeStart += 1;
				if ( _freeStart > _blockCount ) _freeStart = 0;
			}

			semGive(_sem);
		}

		return result;
	}

	inline bool free(void * blockAddr)
	{
		bool	result = false;

		if ( blockAddr >= _start )
		{
			size_t	offset = (char *)blockAddr - (char *)_start;
			int	index = offset/_blockSize;
			if ( index < _blockCount )
			{
				if ( (offset % _blockSize) == 0 )
				{
					semTake(_sem, WAIT_FOREVER);
					_freeArray[_freeEnd] = index;

					_freeEnd += 1;
					if ( _freeEnd > _blockCount ) _freeEnd = 0;
					semGive(_sem);

					result = true;
				}
				else
				{
					_FATAL_ERROR(__FILE__, __LINE__, "bad pointer in pool space");
				}
			}
		}

		return result;
	}

private:
	SEM_ID	_sem;

	unsigned short _blockSize;
	unsigned short	_blockCount;

	unsigned short	* _freeArray;
	unsigned short	_freeStart;
	unsigned short	_freeEnd;

	void * _start;
};

static CGUIMemPool	* memPool = NULL;

static UGL_MEM_POOL_ID cguiMemPoolCreate(void * poolAddress, UGL_SIZE poolSize)
{
	// Dummy routine, since pools aren't used.  Just return a unique ID number for each
	// pool created.
	//
   static int id = 0;
	id += 1;
	return (UGL_MEM_POOL_ID)id;
}

static UGL_STATUS cguiMemPoolDestroy(UGL_MEM_POOL_ID poolId)
{
	// No need to destroy the pool - see cguiMemPoolCreate
	//
   return UGL_STATUS_OK;
}

static void * cguiMemAlloc(UGL_MEM_POOL_ID poolId, UGL_SIZE size)
{
	void * result;

	result = memPool->alloc(size);
	if ( !result )
	{
		result = malloc(size);
	}

   return result;
}

static void * cguiMemCalloc
    (
    UGL_MEM_POOL_ID poolId, 
    UGL_ORD numItems,
    UGL_SIZE itemSize
    )
{
   UGL_SIZE size = numItems * itemSize;

   void * pMem = cguiMemAlloc(poolId, size);
   memset(pMem, 0, size);

   return pMem;
}

static UGL_STATUS cguiMemFree 
    (
    void * pMem
    )
{
	if ( !memPool->free(pMem) )
	{
		free(pMem);
	}

   return UGL_STATUS_OK;
}

static void * cguiMemRealloc
    (
    UGL_MEM_POOL_ID poolId,
    void * pMem,
    UGL_SIZE size
    )
{
	void * result = NULL;

	if ( pMem == NULL )
	{
		result = cguiMemAlloc(poolId, size);
	}
	else
	{
		size_t	blockSize = memPool->blockSize(pMem);
		if ( blockSize < 0 )
		{
			result = realloc(pMem, size);
		}
		else if ( size > blockSize )
		{
			result = malloc(size);
			memmove(result, pMem, blockSize);
			cguiMemFree(pMem);
		}
	}

	return result;
}

void cguiPoolAllocatorInitialize(size_t blockCount)
{
	memPool = new CGUIMemPool(512, blockCount);
	uglMemInterfaceSet((UGL_FPTR)&cguiMemPoolCreate, (UGL_FPTR)&cguiMemPoolDestroy, (UGL_VOIDFPTR)&cguiMemAlloc, (UGL_VOIDFPTR)&cguiMemCalloc, (UGL_VOIDFPTR)&cguiMemRealloc, (UGL_FPTR)&cguiMemFree);
}

