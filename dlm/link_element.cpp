/*
 * $Header$
 * $Log$
 *
 */

#include <vxWorks.h>
#include "datalog_levels.h"
#include "error.h"
#include "link_element.h"

LinkElement::LinkElement(void)
	: _top(NULL), _child(NULL), _level(0), _linkType(LT_Invalid)
{
}

LinkElement::~LinkElement(void)
{
	// Fatal errors set a flag which is tested at the end of the routine.  This
	// is intended to insure that the group is correctly unlocked before calling
	// fatal error, so that other tasks using the group (that may be critical to
	// the fatal error handling) are allowed to run.
	//
	bool fatalError = false;

	// Destructor is only needed if this element has already been linked
	if ( _group )
	{
		_group->lock();
		if ( !_top )
		{
			fatalError = true;
			DataLog(log_level_critical) << hex
            << "invalid link (_top==NULL): element=" << name() << "[" << (unsigned int)this << "]"
            << dec << endmsg;
		}
		else if ( *_top == this )
		{
			// this element is at the top of the chain
			*_top = _child;
		}
		else if ( *_top )
		{
			// element is in middle of the chain, need to find parent for unlink
			LinkElement * parent = *_top;
			while ( parent && parent->_child != this )
			{
				parent = parent->_child; 
			}

			if ( !parent )
			{
				// there is a problem if this object isn't in the list
				fatalError = true;
				DataLog(log_level_critical) << hex
               << "missing element in list: element=" << name() << "[" << (unsigned int)this << "] "
               << "top=" << (*_top)->name() << "[" << (unsigned int)*_top << "]"
               << dec << endmsg;
			}
			else
			{
				parent->_child = _child;
			}
		}
		else
		{
			// there is a problem if there is no top level object
			fatalError = true;
			DataLog(log_level_critical) << hex
            << "no top element: element=" << name() << "[" << (unsigned int)this << "]"
            << dec << endmsg;
		}

		_group->unlock();
	}

	if ( fatalError )
	{
		_FATAL_ERROR(__FILE__, __LINE__, "LinkElement");
	}
}

const char * LinkElement::name(void)
{
	return "base link element";
}

void LinkElement::do_link(
	LinkGroup * group,	// group in which element resides
	LinkElement ** top,	// storage location for current top level element
	unsigned int level,	// level at which object should be linked
	LinkType linkType 	// ordering option for link
	)
{
	// Fatal errors set a flag which is tested at the end of the routine.  This
	// is intended to insure that the group is correctly unlocked before calling
	// fatal error, so that other tasks using the group (that may be critical to
	// the fatal error handling) are allowed to run.
	//
	bool fatalError = false;

	_group = group;
	_top = top;
	_level = level;
	_linkType = linkType;
	_child = NULL;

	_group->lock();
	if ( !*_top )
	{
		*_top = this;
	}
	else
	{
		LinkElement ** parent = _top;
		while ( !fatalError )
		{
			if ( (*parent)->_level == _level &&
              (_linkType == LT_Exclusive || (*parent)->_linkType == LT_Exclusive) )
			{
				// more than one object at the same level with LT_Exclusive specified
				fatalError = true;
				DataLog(log_level_critical) << hex
               << "multiple objects at same level: element=" << name() << "[" << (unsigned int)this << "] "
               << "parent=" << (*parent)->name() << "[" << (unsigned int)*parent << "] "
               << "top=" << (*_top)->name() << "[" << (unsigned int)*_top << "]"
               << dec << endmsg;
			}
			else if ( (*parent)->_level < _level ||
			     (_linkType == LT_Top && (*parent)->_level <= _level) )
			{
				_child = *parent;
				*parent = this;
				break;
			}
			else if ( !(*parent)->_child )
			{
				(*parent)->_child = this;
				break;
			}
			else
			{
				parent = &(*parent)->_child;
			}
		}
	}

	_group->unlock();
	if ( fatalError )
	{
		_FATAL_ERROR(__FILE__, __LINE__, "LinkElement");
	}
}

