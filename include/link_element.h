/*
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/link_element.h 1.1 2004/01/26 18:50:31Z jl11312 Exp jl11312 $
 * $Log: link_element.h $
 * Revision 1.1  2004/01/26 18:50:31Z  jl11312
 * Initial revision
 *
 */

#ifndef _LINK_ELEMENT_INCLUDE
#define _LINK_ELEMENT_INCLUDE

#include "link_group.h"

class LinkElement
{
public:
	// LinkType enumerates the ordering options allowed by the link()
	// function.
	//
	enum LinkType
	{
		LT_Invalid,			// enum to denote invalid or un-initialized link type
		LT_Exclusive,		// error if an object is already linked at the same level
		LT_Top,				// object is linked above any existing objects at the same level
		LT_Bottom			// object is linked below any existing objects at the same level
	};

	// CallStatus enumerates the possible results returned from a over-ridden function.
	// The override can either allow the lower level functions to run (and presumably
	// only extend the base behavior) and can prevent the lower level functions from
	// being called (and presumably replace the base behavior).
	//
	enum CallStatus
	{
		CS_Chain,		// call over-ridden function for next (lower) object in chain
		CS_Done			// do not call over-ridden function for any lower level objects
	};

	virtual ~LinkElement();
	virtual const char * name(void);

	LinkElement ** top(void) { return _top; }
	LinkElement * child(void) { return _child; }
	LinkGroup * group(void) { return _group; }
	unsigned int level(void) { return _level; }
	LinkType linkType(void) { return _linkType; }

protected:
	// Constructor is protected, since it only makes sense for LinkElement
	// to be a base for some other class.
	//
	LinkElement(void);

	// Application specific class is expected to define a enum for link levels,
	// and to provide a link() function which calls this protected do_link()
	// function to perform the actual linkage.
	//
	void do_link(
		LinkGroup * group,	// group in which element resides
		LinkElement ** top,	// storage location for current top level element
		unsigned int level,	// level at which object should be linked
		LinkType linkType 	// ordering option for link
		);

private:
	LinkGroup * _group;
	LinkElement ** _top;
	LinkElement * _child;
	unsigned int _level;
	LinkType _linkType;
};

#endif /* ifndef _LINK_ELEMENT_INCLUDE */
