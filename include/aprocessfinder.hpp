/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      aprocessfinder.hpp
 *
 * AUTHOR:     Tate Moore
 *
 * ABSTRACT:   This file implements the header for the reply object. 
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 */

#ifndef APROCESSFINDER_HPP  //  Prevents multiple compilations.
#define APROCESSFINDER_HPP

class aProcessfinder
{
   private:
      pid_t _currentProcess;

   public:
      // Constructors.
      aProcessfinder();

      // Destructor.
      ~aProcessfinder();

      // Methods.
      int find_another_process(char *name, pid_t id = 1, pid_t node = 0);
      int find_process(char *name, pid_t id = 1, pid_t node = 0);
};

#endif
