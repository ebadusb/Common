/*
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item_file_reader.cpp 1.5 2009/06/24 18:41:45Z wms10235 Exp wms10235 $
 *
 * $Log: cgui_data_item_file_reader.cpp $
 * Revision 1.3  2007/04/30 21:18:27Z  wms10235
 * IT2354 - Updated the getTextItem() method to be a static
 * Revision 1.2  2007/03/01 19:09:55Z  rm10919
 * Add DataItemText to file reader and setText(char *) to DITextItem.
 * Revision 1.1  2005/11/22 00:37:26Z  rm10919
 * Initial revision
 *
 */

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

   static char lineBuffer[LineBufferSize];
   bool  readingFileTable = false;

   FILE * dataItemInfo = fopen(filename, "r");

   int   line = 0;
   char * p = NULL;

//   taskSuspend(taskIdSelf());

   while (fgets(lineBuffer, LineBufferSize, dataItemInfo) != NULL)
   {
      line += 1;

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
            DataLog( log_level_cgui_error ) << "line " << line << ": unexpected DATA_ITEM_FILE_TABLE_START - " << filename << endmsg;
            printf("line %d: unexpected DATA_ITEM_FILE_TABLE_START\n - %s", line, filename);
            exit(1);
         }
         readingFileTable = true;
         continue;
      }

      if (strcmp(firstToken, "DATA_ITEM_FILE_TABLE_END") == 0)
      {
         if (!readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << line << ": unexpected DATA_ITEM_FILE_TABLE_END - " << filename << endmsg;
            printf("line %d: unexpected DATA_ITEM_FILE_TABLE_END\n - %s", line, filename);
            exit(1);
         }
         readingFileTable = false;
         continue;
      }

      if (readingFileTable)
      {
         DataItemEntry * entry = new DataItemEntry;
         char * hold = NULL;

         //
         // Parse entry from file.
         //
         entry->id = firstToken;
         entry->type = strtok_r(NULL, " \t\n\r", &p);
         entry->initialize = strtok_r(NULL," \t\n\r", &p);

			bool goodDataItem = false;
			
         if (entry->id)
         {
            if (strcmp(entry->type, "TextItem") == 0)
            {
               //
               // Declare variables for string - text item mapping.
               CGUIDataItemTextItem * dataItemTextItem = new CGUIDataItemTextItem(CGUITextItem::getTextItem(entry->id));

               if (dataItemTextItem)
               {
                  variableDictionary->addDataItem(entry->id, dataItemTextItem);
						goodDataItem = true;
               }
               else
               {
                  DataLog( log_level_cgui_error ) << "line " << line << ": unknown Text Item DATA ITEM - " << filename << "\n" << endmsg;
                  printf("line %d: unknown Text Item DATA ITEM\n - %s - %s \n", line, filename, entry->id);
               }
            }
            else if (strcmp(entry->type, "Text") == 0)
            {
               // Declare variables for string - text.
               CGUIDataItemText * dataItemText = new CGUIDataItemText(entry->initialize);

               if (dataItemText)
               {
                  variableDictionary->addDataItem(entry->id, dataItemText);
						goodDataItem = true;
               }
               else
               {
                  DataLog( log_level_cgui_error ) << "line " << line << ": unknown Text Item DATA ITEM - " << filename << endmsg;
                  printf("line %d: unknown Text DATA ITEM\n - %s - %s \n", line, filename, entry->id);
               }
            }
            else if ( strcmp(entry->type, "Double") == 0)
            {
               entry->precision = strtok_r(NULL," \t\n\r", &p);

               CGUIDataItemDouble * dataItemDouble = new CGUIDataItemDouble(atof(entry->initialize), atoi(entry->precision));

               if (dataItemDouble)
               {
                  variableDictionary->addDataItem(entry->id, dataItemDouble);
						goodDataItem = true;
               }
               else
               {
                  DataLog( log_level_cgui_error ) << "line " << line << ": unknown Double DATA ITEM - " << filename << " \n" << endmsg;
                  printf("line %d: unknown Double DATA ITEM\n - %s - %s \n", line, filename, entry->id);
               }
            }
            else if (strcmp(entry->type, "Integer") == 0)
            {
               CGUIDataItemInteger * dataItemInteger = new CGUIDataItemInteger(atoi(entry->initialize));

               if (dataItemInteger)
               {
                  variableDictionary->addDataItem(entry->id, dataItemInteger);
						goodDataItem = true;
               }
               else
               {
                  DataLog( log_level_cgui_error ) << "line " << line << ": unknown Integer DATA ITEM - " << filename << "\n" << endmsg;
                  printf("line %d: unknown Integer DATA ITEM\n - %s - %s \n", line, filename, entry->id);
               }
            }
				else if (strcmp(entry->type, "Clock") == 0)
				{
					CGUIDataItemClock * dataItemClock = new CGUIDataItemClock(atoi(entry->initialize));

					if (dataItemClock)
					{
						variableDictionary->addDataItem(entry->id, dataItemClock);
						goodDataItem = true;
					}
					else
					{
						DataLog( log_level_cgui_error ) << "line " << line << ": unknown Clock DATA ITEM - " << filename << "\n" << endmsg;
						printf("line %d: unknown Clock DATA ITEM\n - %s - %s \n", line, filename, entry->id);
					}			 
				}
            else
            {
               DataLog( log_level_cgui_error ) << "line " << line << ": unknown DATA ITEM TYPE - " << filename << "\n" << endmsg;
               printf("line %d: unknown DATA ITEM TYPE\n - %s - type - %s \n", line, filename, entry->type);
            }
         }
         else
         {
            DataLog( log_level_cgui_error ) << "line " << line << ": bad entry in string info file - " << filename << "\n" << endmsg;
            printf("line %d: bad entry in string info file\n - %s \n", line, filename);
         }
      }  /* readingFileTable */
		
   }   /* While dataItemInfo no NULL */
   // Close file.
   fclose(dataItemInfo);

   return readingFileTable;
}

