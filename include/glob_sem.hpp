/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      glob_sem.hpp
 *
 * AUTHOR:     Tate Moore
 *
 * ABSTRACT:   This header file defines the global semaphore classes.
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 */

#ifndef GLOB_SEM_HPP  //  Prevents multiple compilations.
#define GLOB_SEM_HPP


// Include Files
#include <sys/trace.h>
#include <sys/tracecod.h>
#include <sys/types.h>
#include <unistd.h>
#include <conio.h>
#include <stdio.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stddef.h>
#include <fcntl.h>
#include "commondefs.h"
#include "error.h"

// Defines
#define  SEM_SHARED     1       // semaphore is in shared memory

// NOTE:
//			Definition of the structure for use in the GlobalSemaphore must 
//          start with a sem_t as the first variable in the list. See example
//			below.
//
// typedef struct
//{
// Shared Data Memory semaphore
// - MUST BE FIRST VARIABLE IN STRUCT LIST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/******************************************************************************/
//    sem_t data_semaphore;
/******************************************************************************/
//  something1 name1
//  something2 name2....
//} A_SAMPLE_SEMAPHORE_STRUCT;

// Create the template using
//			DATA_MEMBERS is the structure to base the semaphore.
// 
template <class DATA_MEMBERS>
class GlobalSemaphore
{
    protected:

        int memory_handle;      /* memory handler */

        char *memname;          /* name of the memory object  */

        char *data_ptr;         /* pointer to mapped input data */

		int memory_locked;

    protected:

        // Method to copy the entire common data store to another location.
        // *********************************************************************
        // THIS METHOD IS PROTECTED so that general Procedure Common Data
        // stores can not overwrite all data...
        virtual void setAllData( DATA_MEMBERS * source )
		{
			// Check out memory resource
			LockMemory();

			// Copy the common data store info to the destination structure.
			memcpy((void *)data_ptr, (const void *)source, (sizeof(DATA_MEMBERS)));

			// Release memory resource
			FreeMemory();
		};
        // *********************************************************************
		void setDataDirect(unsigned int offset, unsigned char value)
		{
			char *workptr;

		    workptr = data_ptr + offset;
			*( (unsigned char *) workptr) = value;

		};

        // Create shared memory structure
        virtual void CreateMemory(void)
		{
			char *workptr;
			unsigned int i;

			// Create a memory object with name memname, read/write, mode 777
			memory_handle = shm_open(memname, O_CREAT | O_RDWR, 0777);
			if (memory_handle == -1)
			{
				_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Shared memory error" );
			}

			// Now set size to sizeof(DATA_MEMBERS)
			if ( ltrunc( memory_handle, sizeof(DATA_MEMBERS), SEEK_SET)
				  == -1)
			{
				_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Shared memory trucation error" );
			}

			// Now map the data memory
			if ( (data_ptr = (char *) mmap((void *) 0,
										   sizeof(DATA_MEMBERS),
										   PROT_READ | PROT_WRITE,
										   MAP_SHARED,
										   memory_handle,
										   0))
				  == (char *) -1)
			{
				_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Memory mapping error" );
			}

			// Initialize the data to 0's
			for (i = 0; i < sizeof(DATA_MEMBERS); i++)
			{
				setDataDirect(i, (unsigned char) '\0');
			}

			// Now initialize the shared data memory semaphore
			workptr = data_ptr + offsetof(DATA_MEMBERS, data_semaphore);
			if ( sem_init( (sem_t *) workptr, SEM_SHARED, 1) == -1)
			{
				_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Semaphore initialization" );
			}

		};

	private:
		// Hide the default constructor
		GlobalSemaphore();

    public:

        //  Constructor.
        //
        // NOTE:  ONLY SET semaphore_already_locked IF YOU ALREADY HAVE THE SEMAPHORE LOCKED
        //        ON CREATION OF THIS OBJECT.  THIS WILL ALLOW FOR USE OF THE OBJECT WITHOUT 
        //        ATTEMPT TO ACTUALLY GET A SEMAPHORE YOU ALREADY HAVE.
        //
        GlobalSemaphore(const char * name, int semaphore_already_locked = 0) : memory_handle(-1),
			                                                                      data_ptr(0),
											                                              memory_locked(0)
		{
			if ( name ) 
				memname = strdup(name);
			else
				memname = strdup("GLOBAL");
         // This allows people to set up the semaphore as if it we already locked.
         memory_locked = semaphore_already_locked;
		};

        //  Destructor.
        virtual ~GlobalSemaphore(void) 
		{
			delete memname;
			memname = NULL;
		};
        
        // Delete shared memory structure
        virtual void DeleteMemory(void)
		{
			// Clean up internal pointers
			munmap(data_ptr, sizeof(DATA_MEMBERS));
			close(memory_handle);

			// Get rid of memory object
			(void) shm_unlink(memname);
		};

        // Access previously created memory structure
        virtual void AccessMemory(void)
		{
			// Access memory object with name memname, read/write, mode 777
			memory_handle = shm_open(memname, O_RDWR, 0777);
			if (memory_handle == -1)
			{
				// If someone calls this before it exists then create the 
				// semaphore memory space.
				CreateMemory();
				memory_handle = shm_open(memname, O_RDWR, 0777);
				if ( memory_handle == -1 )
					_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Shared memory open" );
			}

			// Now map the input data memory
			if ( (data_ptr = (char *) mmap((void *) 0,
										   sizeof(DATA_MEMBERS),
										   PROT_READ | PROT_WRITE,
										   MAP_SHARED,
										   memory_handle,
										   0))
					== (char *) -1)
			{
				_FATAL_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, "Memory map error" );
			}

		};

        // Check to see if shared memory has been allocated
        virtual iBool CheckAccessMemory(void)
		{
			// Access memory object with name memname, read/write, mode 777
			int temp_memory_handle = shm_open(memname, O_RDWR, 0777);
			if (temp_memory_handle == -1) return (unsigned char)0;
			else return (unsigned char)!0;
		};

        inline int LockMemory()
        {
            sem_t *workptr;
            workptr = 0;

			// NOTE:  THIS IS NOT THREAD SAFE...
			// If the memory locked is set then this object already
			// has the semaphore.
			if ( memory_locked )
			{
				memory_locked++;
				return 0;
			}
			// NOTE:  THIS IS NOT THREAD SAFE...
            else 
            {
				memory_locked++;
                return( sem_wait( (sem_t *) data_ptr) );

            }
        };

        inline int FreeMemory(void)
        {
			// Reduce the amount of locks on the semaphore internal use count.
			memory_locked--;
			if ( memory_locked <= 0 )
            {
				// This should already be zero but let's just force it.
				memory_locked = 0;
				return( sem_post( (sem_t *) data_ptr) );
			}
			else return 0;
        };

        //
        //
        //  Get access to my data pointer.  This method can be used when you
        //  wish to set multiple members without semaphoring each time.  THE
        //  DATA RESOURCE WILL BE LOCKED BEFORE THE POINTER IS RETURNED AND
        //  YOU _MUST_ CALL FreeMemory() TO UNLOCK THE RESOURCE!
        //
        virtual const char *getDataPtr()
		{
		   // Check out memory resource
		   LockMemory();
		   return data_ptr;
		};


}; // End GlobalSemaphore class


#endif
