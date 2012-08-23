/*****************************************************************************\
**
** LogExtractor.h
**
** Interface for processing datalog files.
**
\*****************************************************************************/

#ifndef LOGPROCESS_H
#define LOGPROCESS_H

/* Includes ******************************************************************/

#include "LogReader.h"

namespace DECODER {

/* Classes *******************************************************************/

class LogExtractor {
public:

	/**
	 *
	 * Register
	 *
	 * Registers the necessary handlers with the reader.
	 *
	 * @param reader The file reader.
	 *
	 */
	virtual void Register(LogReader &reader) = 0;

	/**
	 *
	 * Unregister
	 *
	 * Unregisters the handlers from the reader.
	 *
	 * @param reader The file reader.
	 *
	 */
	virtual void Unregister(LogReader &reader) = 0;

	/**
	 *
	 * PreProcess
	 *
	 * Perform the procedures before the import process.
	 *
	 */
	virtual void PreProcess(void) = 0;

	/**
	 *
	 * PostProcess
	 *
	 * Perform the procedures after the import process.
	 *
	 */
	virtual void PostProcess(void) = 0;
};

} // namespace DECODER

#endif // LOGPROCESS_H

