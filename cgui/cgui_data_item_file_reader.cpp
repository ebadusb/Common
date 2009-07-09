/*
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item_file_reader.cpp 1.5 2009/06/24 18:41:45Z wms10235 Exp $
 *
 * $Log: cgui_data_item_file_reader.cpp $
 * Revision 1.5  2009/06/24 18:41:45Z  wms10235
 * IT6958 - Interrmittent page fault occurs at protocol load or during procedure
 * Revision 1.4  2007/11/15 21:02:37Z  rm10919
 * Create new data item class for clock type numeric strings.
 * Revision 1.3  2007/04/30 21:18:27Z  wms10235
 * IT2354 - Updated the getTextItem() method to be a static
 * Revision 1.2  2007/03/01 19:09:55Z  rm10919
 * Add DataItemText to file reader and setText(char *) to DITextItem.
 * Revision 1.1  2005/11/22 00:37:26Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_data_item.h"
#include "cgui_text_item.h"
#include "cgui_variable_database.h"
#include "datalog_levels.h"
#include "link_element.h"
#include "link_group.h"

static bool readDataItemFile (const char * filename, CGUIVariableDatabase * variableDictionary, unsigned int linkLevel = 0)
{
   enum
   {
      LineBufferSize = 65000
   };

   struct DataItemEntry
   {
      char * id;
      char * type;
      char * initialize;
      char * precision;
   };

   bool  readingFileTable = false;
	bool	retVal = true;
	int   line = 0;
	char * p = NULL;

   FILE * dataItemInfo = fopen(filename, "r");

	if( dataItemInfo )
	{
		DataLog( log_level_cgui_info ) << "readDataItemFile opened file " << filename << endmsg;

		char * lineBuffer = new char[LineBufferSize+1];

		while (fgets(lineBuffer, LineBufferSize, dataItemInfo) != NULL)
		{
			line += 1;
			lineBuffer[LineBufferSize] = 0;	// Make sure the buffer is null terminated

			//
			// Get first token. (space, tab, newline, return)
			char * firstToken = strtok_r(lineBuffer, " \t\n\r", &p);

			if (!firstToken || firstToken[0] == '#')
			{
				continue;
			}

			if (strcmp(firstToken, "DATA_ITEM_FILE_TABLE_START") == 0)
			{
				if (readingFileTable)
				{
					DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": unexpected DATA_ITEM_FILE_TABLE_START - " << filename << endmsg;
					retVal = false;
				}
				readingFileTable = true;
				continue;
			}

			if (strcmp(firstToken, "DATA_ITEM_FILE_TABLE_END") == 0)
			{
				if (!readingFileTable)
				{
					DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": unexpected DATA_ITEM_FILE_TABLE_END - " << filename << endmsg;
					retVal = false;
				}
				readingFileTable = false;
				continue;
			}

			if (readingFileTable)
			{
				DataItemEntry entry;

				//
				// Parse entry from file.
				//
				entry.id = firstToken;
				entry.type = strtok_r(NULL, " \t\n\r", &p);
				entry.initialize = strtok_r(NULL," \t\n\r", &p);
				entry.precision = NULL;

				if (entry.id && entry.type)
				{
					if (strcmp(entry.type, "TextItem") == 0)
					{
						// Declare variables for string - text item mapping.
						if( entry.initialize )
						{
							CGUITextItem * pTextItem = CGUITextItem::getTextItem(entry.initialize);

							if( pTextItem )
							{
								CGUIDataItemTextItem * dataItemTextItem = new CGUIDataItemTextItem(pTextItem);
								variableDictionary->addDataItem(entry.id, dataItemTextItem);
							}
							else
							{
								DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": unknown Text Item DATA ITEM - " << filename << endmsg;
								retVal = false;
							}
						}
						else
						{
							DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": cannot initialize Text Item DATA ITEM - " << filename << endmsg;
							retVal = false;
						}
					}
					else if (strcmp(entry.type, "Text") == 0)
					{
						// Declare variables for string - text.
						if( entry.initialize )
						{
							CGUIDataItemText * dataItemText = new CGUIDataItemText(entry.initialize);
							variableDictionary->addDataItem(entry.id, dataItemText);
						}
						else
						{
							DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": bad entry in string info file - " << filename << endmsg;
							retVal = false;
						}
					}
					else if ( strcmp(entry.type, "Double") == 0)
					{
						entry.precision = strtok_r(NULL," \t\n\r", &p);

						if( entry.precision && entry.initialize )
						{
							CGUIDataItemDouble * dataItemDouble = new CGUIDataItemDouble(atof(entry.initialize), atoi(entry.precision));
							variableDictionary->addDataItem(entry.id, dataItemDouble);
						}
						else
						{
							DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": bad entry in string info file - " << filename << endmsg;
							retVal = false;
						}
					}
					else if (strcmp(entry.type, "Integer") == 0)
					{
						if( entry.initialize )
						{
							CGUIDataItemInteger * dataItemInteger = new CGUIDataItemInteger(atoi(entry.initialize));
							variableDictionary->addDataItem(entry.id, dataItemInteger);
						}
						else
						{
							DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": bad entry in string info file - " << filename << endmsg;
							retVal = false;
						}
					}
					else if (strcmp(entry.type, "Clock") == 0)
					{
						if( entry.initialize )
						{
							CGUIDataItemClock * dataItemClock = new CGUIDataItemClock(atoi(entry.initialize));
							variableDictionary->addDataItem(entry.id, dataItemClock);
						}
						else
						{
							DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": unknown Clock DATA ITEM - " << filename << endmsg;
							retVal = false;
						}
					}
					else
					{
						DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": unknown DATA ITEM TYPE - " << filename << endmsg;
						retVal = false;
					}
				}
				else
				{
					DataLog( log_level_cgui_error ) << "Error in file at line " << line << ": bad entry in string info file - " << filename << endmsg;
					retVal = false;
				}
			}

			// taskDelay(0);
		}

		// Close file.
		fclose(dataItemInfo);
		delete [] lineBuffer;
		DataLog( log_level_cgui_info ) << "readDataItemFile closed file " << filename << endmsg;
	}
	else
	{
		DataLog( log_level_cgui_error ) << "Could not open DATA ITEM file: " << filename << endmsg;
		retVal = false;
	}

   return retVal;
}

