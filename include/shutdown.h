/*
 * Copyright (c) 199-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: $
 * $Log: $
 *
 * TITLE:      shutdown.h, machine-specific shutdown
 *
 * ABSTRACT:    Shuts down essential tasks when a severe and unexpected
 *              error occurs.   Essential tasks are those that interface
 *              to the filesystem, operator or hardware.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS8.DOC
 * Test:             I:\ieee1498\STD8.DOC
 */
 
#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <sys/name.h>
#include <sys/psinfo.h>
#include <sys/types.h>

void initializeShutdown();
void applicationShutdown();
void addEssentialTasks(pid_t pid, mqd_t mq, char* name);
void removeEssentialTasks(pid_t pid, char* name);

#endif          // SHUTDOWN_H


