
/************************> TRIMA GUI DOMAIN <****************************

*     Copyright(c) 1998 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   Table_Collection

*     File name:    tablecol.hpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This is a templated class that provides rudimentary ordered table
         creation and retrieval. It does not allow for selective deletion or
         update of table elements -- basically a read-only lookup table.
         All the method definitions are in this module... having found no
         way to include the method definitions in a .cpp file and have them
         compile in the appropritate dependent classes.

*************************< TRIMA GUI DOMAIN >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/tablecol.hpp 1.1 2001/08/16 13:50:09 jl11312 Exp $
$Log: tablecol.hpp $
Revision 1.1  2001/08/16 13:50:09  jl11312
Initial revision
Revision 1.1  1999/12/10 14:36:24  BS04481
Initial revision
Revision 1.1  1998/08/08 00:05:02  tw09453
Initial revision

*/




// Single include define guardian
#ifndef TABLECOL_HPP
#define TABLECOL_HPP

#include <stdio.h>

template <class COL_CLASS>


 class Table_Collection
 {

    private:


      struct links
      {
         links * next_link ;
         COL_CLASS    * col_element;

         //Constructor for initializing "new'ed" instances
         links(links * linkptr, COL_CLASS * col_classptr) :
               next_link(linkptr), col_element(col_classptr) {};
      };


      links      list_anchor ;

      links    * last_table_entry ;

      links    * table_cursor ;



       // The linkage of COL_CLASS elements will look like this:
       //
       //    automatic,
       //    private
       //
       //     list_anchor    "newed" link1    "newed" link2
       //   **************   *------------*   *------------*
       //   |   &link1   |-->|   &link2   |-->|   NULL     |
       //   **************   *------------*   *------------*
       //   | always NULL|   | &COL_CLASS1|   | &COL_CLASS2|
       //   **************   *------------*   *------------*
       //                          |                |
       //                          V                V
       //                    *------------*   *------------*
       //                    | COL_CLASS1 |   | COL_CLASS2 |
       //                    *- - - - - - *   *- - - - - - *
       //
       //
       //



    protected:



    public:


       //CONSTRUCTOR
       Table_Collection() :
                          list_anchor((links*)NULL, (COL_CLASS*)NULL),
                          table_cursor((links*)NULL),
                          last_table_entry(&list_anchor)

       {
       };  // END of Table_Collection CONSTRUCTOR




       //DESTRUCTOR
       virtual ~Table_Collection()
       {
         // This pointer points to the next list element candidate to delete
         links * next_to_delete = list_anchor.next_link ;
         links * point_ahead ;



         while (next_to_delete != NULL)
         {
            //save off any pointer to the next linkage element
            point_ahead = next_to_delete -> next_link ;

            //if we have a valid table element in this linkage, zap it.
            if ((next_to_delete -> col_element) != NULL)
            {
               delete next_to_delete -> col_element ;
            }

            //delete the linkage element itself
            delete next_to_delete ;

            //link ahead to the next (or null) linkage element
            next_to_delete = point_ahead ;

         }

       };  // END of Table_Collection DESTRUCTOR




       //ADD_TABLE_ENTRY
       // This method will take the pointer to a table entry and place it in a linkage
       // element and link onto the table link list.
       void add_table_entry (COL_CLASS * table_element)
       {
         //create a new linkage element
         links * new_linkage = new links((links*)NULL, (COL_CLASS*)NULL) ;

         //have the current "last linkage element" point to the newly created one
         last_table_entry -> next_link = new_linkage ;

         //call the newly created linkage element the last table entry
         last_table_entry = new_linkage ;

         //store the pointer to the collection element passed to us.
         last_table_entry -> col_element = table_element ;


       };  // END of Table_Collection ADD_TABLE_ENTRY




       //SET_CURSOR_TO_TOP
       // This method will reset the table_cursor to the "top" of the table in preparation
       // for a new read or search of the table.
       void set_cursor_to_top ()
       {
          //have the cursor point to the first linkage element off of the anchor.
          table_cursor = list_anchor.next_link ;

       };  // END of Table_Collection SET_CURSOR_TO_TOP




       //GET_NEXT_ENTRY
       // This method will return a pointer to the current table element (not link element)
       // and then bump the cursor to the next link entry. If the cursor is pointing past the end
       // of the table when this is called, it will return a NULL pointer.
       COL_CLASS * get_next_entry ()
       {
          COL_CLASS * return_ptr ;

          if (table_cursor != NULL)
          {
             //get the pointer to the table element managed by this linkage element
             return_ptr = table_cursor -> col_element ;

             //bump the table cursor to the next linkage element (or NULL, if end of chain)
             table_cursor = table_cursor -> next_link ;

             //return a pointer to the table element
             return (return_ptr) ;
          }
          else
          {
             return ((COL_CLASS*)NULL) ;
          }

       };  // END of Table_Collection GET_NEXT_ENTRY




 };  // END of Table_Collection Class Declaration



// End of single include define guardian
#endif
