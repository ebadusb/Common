/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#include <vxworks.h>
#include <ctype.h>
#include <envLib.h>
#include <stdio.h>
#include <string.h>
#include "envfile.h"

/*
 *	Read the specified file, which should consists of settings for environment
 * variables.  The format is identical to that for a Unix shell script
 * consisting of variable settings.  Blank lines (no non-whitspace characters)
 * and lines starting with '#' are treated as comments.  All other lines must
 * be of the form variable_name=setting.
 */
STATUS readEnvironmentFile(const char * fileName)
{
	STATUS	retVal = ERROR;
	FILE	*  fp = fopen(fileName, "r");

	if ( !fp )
	{
		/*
		 *	This function is used by the vxWorks image to load globvars, and can
		 * not contain calls to the data logging system.
		 */
		perror(fileName);
	}
	else
	{
		typedef enum { WaitStartOfVar, ReadingVar, WaitEndOfLine } State;
		State	state = WaitStartOfVar;

		enum { BufferSize = 128 };
		char	buffer[BufferSize];
		int	bufferPos = 0;
		int	ch;

		retVal = OK;
		while ( (ch = fgetc(fp)) != EOF )
		{
			switch ( state )
			{
			case WaitStartOfVar:
				if ( isalnum(ch) )
				{
					/*
					 *	Found start of a valid variable name
					 */
					buffer[0] = ch;
					bufferPos = 1;
					state = ReadingVar;
				}
				else if ( ch == '#' )
				{
					/*
					 *	Found comment, just skip to end of line
					 */
					state = WaitEndOfLine;
				}

				break;

			case ReadingVar:
				if ( ch != '\n' && ch != '\r' )
				{
					/*
					 *	Keep buffering data until end of line reached
					 */
					if ( bufferPos < BufferSize )
					{
						buffer[bufferPos] = ch;
						bufferPos += 1;
					}
				}
				else
				{
					/*
					 *	Found end of line.  If we have a valid environment setting, add it
					 * to the environment.  Note that globvars may also contain export commands
					 * used in the original Unix implementation.  Since these lines do not
					 * contain an '=' character, they will simply be ignored.
					 */
					state = WaitStartOfVar;
					if ( bufferPos < BufferSize )
					{
						if ( strchr(buffer, '=') )
						{
							buffer[bufferPos] = '\0';
							putenv(buffer);
						}
					}
					else
					{
						buffer[BufferSize-1] = '\0';
						fprintf(stderr, "%s: variable setting longer than buffer size of %d bytes\n", fileName, BufferSize);
						fprintf(stderr, "Partial setting: \"%s\"\n", buffer);

						retVal = ERROR;
					}
				}
				break;

			case WaitEndOfLine:
				if ( ch == '\n' || ch == '\r' )
				{
					state = WaitStartOfVar;
				}
				break;
			}
		}
	}

	return retVal;
}

