/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: Element.cpp $ 
* Revision 1.2  2007/12/07 18:31:07Z  estausb
* Modifications for Prototype 1
* Revision 1.1  2007/11/15 20:42:44Z  estausb
* Initial revision
*
*/

/*! \file Element.cpp
*/

#include "Element.h"
#include "MutexLocker.h"

static const std::string sAttrReplace = "replaceOnVersionUpdate";
static const std::string sAttrValueTrue = "true";
static const std::string sAttrValueFalse = "false";

using namespace BctXml;
Element::Element(void):_dirty(false),_pParent(NULL),_sName("")
{
   _dataType=ELEMENT;
}
Element::Element(const std::string& sName, Element* pParent) : _sName(sName), _pParent(pParent),_dirty(false)
{
   _dataType=ELEMENT;
   if (_pParent)
      _pParent->addChildElement(this);
}

Element::~Element()
{
   deleteChildren();
}

void Element::addAttribute(const std::string& sAttName, const std::string& sAttVal)
{
   bool bExists = hasAttribute(sAttName);

   if (bExists == true)
      updateAttribute(sAttName, sAttVal);
   else
   {
    //Make thread safe
      _mutexAttribute.lock();
      BctXml::Attribute attr(sAttName, sAttVal);
      _attributes.push_back(attr);
    //Make thread safe
      _mutexAttribute.unlock();
   }
}

int Element::setDataString(const std::string& inValue)
{  
  //Make Thread Safe
   _mutexMember.lock();  
   int output =0;
   CharData* newCharData;

   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isData())
      {
         if (output==0)
         {
            (*itMem)->setDataString(inValue);
            output=1;
         }
         else
         {
            (*itMem)->setDataString("");
         }


      }

   }
   if (output==0)
   {
      newCharData = new CharData(inValue);
      _vMembers.push_back(newCharData);
      output=1;
   }
   _mutexMember.unlock();
   return output;
}

void Element::addChildElement(Element* child)
{
   if (child)
   {
    //Make Thread Safe
      _mutexMember.lock();


    //Don't add it twice...this would make deleting a problem.
      if (!isChildElement(child))
      {
         _vMembers.push_back(child);
         child->_pParent = this;
      }
      _mutexMember.unlock();
   }
}

bool Element::isChildElement(const Element* child) const
{
  // Search to see if we already have this child;
   XMLMemberTypeSet::const_iterator it = _vMembers.begin();
   bool alreadyAChild = false;
   for (it; it != _vMembers.end() && !alreadyAChild; it++)
   {
      if (*it == (XMLMemberType*)(child))
         alreadyAChild = true;
   }  
   return alreadyAChild;
}

Element* Element::getParent() const
{
   return _pParent;
}

std::string Element::getName() const
{
   return _sName;
}


std::string Element::getDataString() const
{

   std::string output;
   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isData())
      {
         output+=(*itMem)->getDataString();
      }
   }
   return output;
}

void Element::addData(const std::string& sValue)
{
   CharData* newCharData = new CharData(sValue);
  //Make Thread Safe
   _mutexMember.lock();
   _vMembers.push_back(newCharData);
   _mutexMember.unlock();
}

void Element::deleteChildren()
{
  //Make Thread Safe
   _mutexMember.lock();
   XMLMemberTypeSet::iterator itMem=_vMembers.begin();
   XMLMemberType* child;
   while (itMem != _vMembers.end())
   {
      child = *itMem;
      _vMembers.erase(itMem);
      itMem = _vMembers.begin();
      delete child;
   }

   _mutexMember.unlock();
}


bool Element::hasChildren()const
{
  //Make Thread Safe
   _mutexMember.lock();
   bool output=false;
   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isElement())
      {
         output=true;
         break;
      }
   }

   _mutexMember.unlock();
   return output;
}

ElementSet Element::getAllChildren(){
   ElementSet myElemSet = getImmediateChildren();
   return getAllChildren(&myElemSet);
}

ElementSet Element::getAllChildren(ElementSet *input)
{
   ElementSet output;
   std::vector<Element*>::iterator it = input->begin();

   for (it; it != input->end(); it++)
   {

      output.push_back(*it);
      if ((*it)->hasChildren())
      {
         ElementSet subChildren = (*it)->getAllChildren();
         std::vector<Element*>::iterator itsub = subChildren.begin();
         for (itsub; itsub != subChildren.end(); itsub++)
         {
            output.push_back(*itsub);
         }
      }

   }
   return output;
}
ElementSet Element::getImmediateChildren()
{
   ElementSet output;
   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
  //Make Thread Safe
   _mutexMember.lock();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isElement())
      {
         output.push_back(static_cast<Element*>(*itMem));            
      }

   }
  //Make Thread Safe
   _mutexMember.unlock();
   return output;
}

// This method doesn't use the proccess children call in order to avoid
// the need to create vectors of attributes that do not matter.
Element* Element::findFirstChild(const std::string& sName)
{
   Bct::MutexLocker locker(_mutexMember);
   XMLMemberTypeSet::iterator it = _vMembers.begin();
   for (it; it != _vMembers.end(); ++it)
   {
      if ((*it) != NULL && (*it)->isElement() && (*it)->getName() == sName)
         return static_cast<Element*>(*it);
   }
   return NULL;
}

Element* Element::findFirstChild(const std::string& sName, const Attribute &inAttribute)
{
   AttributeSet tempSet;
   tempSet.push_back((inAttribute));
   return(findFirstChild(sName,tempSet));
}
Element* Element::findFirstChild(const std::string& sName, const AttributeSet &inAttributeSet)
{
   Element* output=NULL;

   ElementSet TempSet= processMatchingChildren(sName,inAttributeSet,FindFirst);
   if (TempSet.size()>0)
      output=TempSet.front();
   return output;
}
void Element::deleteFirstChild(const std::string& sName,const AttributeSet &inAttributeSet)
{
   processMatchingChildren(sName,inAttributeSet,DeleteFirst);
}

void Element::deleteFirstChild(const std::string& sName,const Attribute &inAttribute)
{
   AttributeSet tempSet;
   tempSet.push_back((inAttribute));
   deleteFirstChild(sName,tempSet);
}

void Element::deleteFirstChild(const std::string& sName)
{
   AttributeSet tempSet;
   deleteFirstChild(sName,tempSet);
}

ElementSet Element::processMatchingChildren(const std::string& sName,const AttributeSet &inAttributeSet,ProcessingType type)
{
  //Make Thread Safe
   _mutexMember.lock(); 
   ElementSet output;
   bool allAttributesMatch;
   bool restartSearch = false;
   AttributeSet::const_iterator itAtt;
   XMLMemberTypeSet::iterator itMem=_vMembers.begin();

   while ( itMem != _vMembers.end() )
   {

      if ((*itMem) != NULL && (*itMem)->isElement())
      {
         if ((*itMem)->getName() == sName)
         {
            Element* TempElement = static_cast<Element*>(*itMem);
            allAttributesMatch = true;
            itAtt= inAttributeSet.begin();
            for (itAtt; itAtt != inAttributeSet.end() ;itAtt++)
            {
               if (!TempElement->hasAttribute((*itAtt)))
               {
                  allAttributesMatch=false;
                  break;
               }

            }
            if (allAttributesMatch)
            {
               if ((type==DeleteFirst)||(type==DeleteAll))
               {
                  _vMembers.erase(itMem);
                  delete TempElement;
            //Start over
                  itMem=_vMembers.begin();
                  restartSearch=true;
               }
               else
               {
                  output.push_back(TempElement);
               }
               if ((type==DeleteFirst)||(type==FindFirst))break;
            }

         }
      }
      if (restartSearch)
         restartSearch=false;
      else
         itMem++;
   }
  //Make Thread Safe
   _mutexMember.unlock();
   return output;
}

ElementSet Element::findChildren(const std::string& sName,const AttributeSet &inAttributeSet)
{
   return processMatchingChildren(sName,inAttributeSet,FindAll);
}

ElementSet Element::findChildren(const std::string& sName,const Attribute &inAttribute)
{
   AttributeSet tempSet;
   tempSet.push_back((inAttribute));
   return(findChildren(sName,tempSet));
}

ElementSet Element::findChildren(const std::string& sName)
{
   AttributeSet tempSet;
   return(findChildren(sName,tempSet));
}

void Element::deleteChildren(const std::string& sName,const AttributeSet &inAttributeSet)
{
   processMatchingChildren(sName,inAttributeSet,DeleteAll);

}
void Element::deleteChildren(const std::string& sName,const Attribute &inAttribute)
{
   AttributeSet tempSet;
   tempSet.push_back((inAttribute));
   deleteChildren(sName,tempSet);
}

void Element::deleteChildren(const std::string& sName)
{
   AttributeSet tempSet;
   deleteChildren(sName,tempSet);
}

bool Element::hasAttribute(const Attribute &inAttribute)
{
   bool output=false;
  //Make thread safe
   _mutexAttribute.lock();
   AttributeSet::const_iterator it = _attributes.begin();
   for (it; it != _attributes.end(); it++)
   {
      if ((*it)==inAttribute)
      {
         output=true;
         break;
      }
   }
  //Make thread safe
   _mutexAttribute.unlock();
   return output;
}

std::string Element::getAttributeValue(const std::string& sAttName) 
{
   AttributeSet::const_iterator it = _attributes.begin();
   std::string sValue = "";
  //Make thread safe
   _mutexAttribute.lock();
   for (it; it != _attributes.end(); it++)
   {
      if (it->getName() == sAttName)
      {
         sValue = it->getValue();
      }
   }
  //Make thread safe
   _mutexAttribute.unlock();
   return sValue;
}

bool Element::hasAttribute(const std::string& sAttName) 
{
   bool bFound = false;
  //Make thread safe
   _mutexAttribute.lock();
   AttributeSet::const_iterator it = _attributes.begin();
   for (it; it != _attributes.end() && bFound == false; it++)
   {
      if (it->getName() == sAttName)
         bFound = true;
   }
  //Make thread safe
   _mutexAttribute.unlock();
   return bFound;
}

bool Element::updateAttribute(const std::string& sAttName, const std::string& sAttVal)
{
   bool bUpdate = false;
  // We could check for hasAttribute() here, but we are already effectively doing 
  // that in the loop so we have removed it.
  //Make thread safe
   _mutexAttribute.lock();
   AttributeSet::iterator it = _attributes.begin();
   for (it; it != _attributes.end(); it++)
   {
      if (it->getName() == sAttName)
      {
         bUpdate = true;
         it->changeValue(sAttVal);
      }
   }
  //Make thread safe
   _mutexAttribute.unlock();
   return bUpdate;
}
void Element::addComment( Comment* inComment)
{
  //Make Thread Safe
   _mutexMember.lock();
   _vMembers.push_back(inComment);
  //Make Thread Safe
   _mutexMember.unlock();
}
CommentSet Element::getComments() 
{
   CommentSet output;
  //Make Thread Safe
   _mutexMember.lock();
   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isComment())
      {
         output.push_back(static_cast<Comment*>(*itMem));
      }
   }
  //Make Thread Safe
   _mutexMember.unlock();
   return output;
}
AttributeSet Element::getAllAttributes() const{
   return _attributes;
}
void Element::toStream(std::ostream &output) const
{
  //Make Thread Safe
   _mutexMember.lock();
  // starting block
   static int tabCount;
   tabCount++;

   output<<std::endl;
   for (int k=1;k<tabCount;k++)
   {
      output<<TAB;
   }
   output<<"<"+_sName;
   AttributeSet::const_iterator itAtt = _attributes.begin();
   for (itAtt; itAtt != _attributes.end() ;itAtt++)
   {
      output<<" "<<(*itAtt).getName()<<"=\""<<(*itAtt).getValue()<<"\"";
   }
   output<<">";
   bool itMemberIsElement=false;
   XMLMemberTypeSet::const_iterator itMem=_vMembers.begin();
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
    // output<<(*itMem)->print(tabCount);
      (*itMem)->toStream(tabCount,output);
      if ((*itMem)->isElement())itMemberIsElement=true;
   }

   if ((itMemberIsElement)||(_vMembers.size()>1))
   {

      output<<std::endl;
      for (int k=1;k<tabCount;k++)
      {
         output<<TAB;
      }
   }
  //ending block

   output<<"</"+_sName+">";
   tabCount--;
  //Make Thread Safe
   _mutexMember.unlock();
}

void Element::setDirty()
{
   _mutexMember.lock();
   _dirty=true;
   _mutexMember.unlock();
}

Element* Element::getRootElement() 
{
   if (getParent() == NULL)
      return this;
   else
      return getParent()->getRootElement();
}

bool Element::isDirty()
{
  //Make Thread Safe
   _mutexMember.lock();
   bool output =  _dirty;
   if (!_dirty)
   {
      XMLMemberTypeSet::iterator itMem=_vMembers.begin();

      for (itMem; itMem != _vMembers.end() ;itMem++)
      {
         if ((*itMem)->isElement())
         {

            Element* TempElement = static_cast<Element*>(*itMem);
            if (TempElement->isDirty())
            {
               output=true;
               break;
            }
         }
      }
   }
  //Make Thread Safe
   _mutexMember.unlock();
   return output;
}
void Element::setClean()
{
  //Make Thread Safe
   _mutexMember.lock();
   _dirty=false;
   XMLMemberTypeSet::iterator itMem=_vMembers.begin();

   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isElement())
      {
         Element* TempElement = static_cast<Element*>(*itMem);
         TempElement->setClean();
      }
   }
  //Make Thread Safe
   _mutexMember.unlock();
}

void Element::deleteAllAttributes()
{
    //Make it thread safe
   _mutexAttribute.lock();
   _attributes.clear();

    //Make it thread safe
   _mutexAttribute.unlock();

}

void Element::deleteAttribute(const std::string &sAttributeName)
{
    //Make it thread safe
   _mutexAttribute.lock();

   AttributeSet::iterator attrIter = _attributes.begin();
   for (; attrIter != _attributes.end(); ++attrIter)
   {
      if ((*attrIter).getName().compare(sAttributeName) == 0)
      {
         _attributes.erase(attrIter);
         break;
      }
   }

    //Make it thread safe
   _mutexAttribute.unlock();
}

void Element::addChildren(Element* curElement, Element* inElement)
{
  //Make Thread Safe
   _mutexMember.lock();

   if (inElement && curElement)
   {
      if (!inElement->hasChildren())
      {
         XMLMemberTypeSet::iterator itMem = inElement->_vMembers.begin();
         if ((*itMem)->isData())
            curElement->addData((static_cast<CharData*>(*itMem))->getDataString());
      }
      else
      {
         ElementSet inChildren = inElement->getAllChildren();
         ElementSet::iterator itInChildren = inChildren.begin();
         if (itInChildren != inChildren.end())
         {
            Element * childElement = *(itInChildren);
            Element * newChildElement = (childElement)->createCopy(curElement);
            curElement->addChildElement(newChildElement);
         }
      }
   }

   _mutexMember.unlock();
}

bool Element::checkForReplaceAttribute(Element* inElement, bool nameCheckOnly)
{
   // If the input element has the replace=true attribute
   bool retValue = false;

   if (inElement)
   {
      bool bVersionWillBeUpdated = false;
      Element *pRootCurr = this->getRootElement();
      Element *pRootIn = inElement->getRootElement();

      if (!nameCheckOnly)
      {
         if (pRootCurr->hasAttribute("Version") && pRootIn->hasAttribute("Version"))
         {
            if (pRootCurr->getAttributeValue("Version").compare(pRootIn->getAttributeValue("Version")) < 0)
            {
               // Default document being passed in has a higher version number than document on disk
               // This is when we want the replaceOnVersionUpdate flag to take affect...
               bVersionWillBeUpdated = true;
            }
         }
      }

      if ((bVersionWillBeUpdated || nameCheckOnly) &&
          inElement->hasAttribute(sAttrReplace) && 
          inElement->getName().compare(this->getName()) == 0 &&
          inElement->getAttributeValue(sAttrReplace).compare(sAttrValueTrue.c_str()) == 0)
      {
         retValue = true;
      }
   }

   return retValue;
}

void Element::merge(Element* inElement, bool ignoreAttrForNameCompare /*=false*/)
{
   if (inElement)
   {
      // If the input element has the replace=true attribute
      if (checkForReplaceAttribute(inElement, false))
      {
         // First we remove all attributes from the current element
         deleteAllAttributes();

         // we add all the attributes on the current element with those from inElement 
         AttributeSet inAttributes = inElement->getAllAttributes();
         for (AttributeSet::iterator inAttribIter = inAttributes.begin(); inAttribIter != inAttributes.end(); ++inAttribIter)
         {
            // do not add the "replace" attribute
            if (inAttribIter->getName().compare(sAttrReplace) != 0 )
            {
               addAttribute(inAttribIter->getName(), inAttribIter->getValue());
            }
         }
         setDirty();

         // and remove all children from current element 
         deleteChildren();

         // add children from inElement to the current element
         addChildren(this, inElement);
      }
      else
      {
         while (true)
         {
            if (isNameMatch(inElement, ignoreAttrForNameCompare))
            {
               // get all children for the default inElement
               ElementSet inChildren=inElement->getImmediateChildren();
               ElementSet::iterator itInChildren=inChildren.begin();

               ElementSet myChildren=getImmediateChildren();
               ElementSet::iterator itMyChildren;

               // for each one check all of this elements children to see if there
               // is a match...make sure to look for "replace = true" attributes (so we can call merge)
               for (itInChildren; itInChildren != inChildren.end(); ++itInChildren)
               {
                  bool ChildMatches=false;
                  itMyChildren=myChildren.begin();

                  for (itMyChildren; itMyChildren != myChildren.end(); ++itMyChildren)
                  {
                     Element *pCurrent = *(itMyChildren);
                     Element *pIn = *(itInChildren);
                     if (pCurrent->isNameMatch(pIn, false) || pCurrent->checkForReplaceAttribute(pIn, false))
                     {
                        // if so recursively merge that child with the matching child
                        ChildMatches=true;
                        (*(itMyChildren))->merge(*(itInChildren));
                        break;
                     }
                     else if (pCurrent->checkForReplaceAttribute(pIn, true))
                     {
                        // we don't merge "replace" children, those are handled above in the "if" statement
                        // and only when the version of the documents are being updated
                        ChildMatches=true;
                        break;
                     }
                  }// for all my children

                  // if at the end none matched create a copy of the inElements data 
                  // and add it to this element
                  if (!ChildMatches)
                  {
                     addChildElement((*(itInChildren))->createCopy(this));
                     setDirty();
                  }// if no matching child
               }// for all incoming children
            }

            // if the name didn't match, was it because it has new attributes to merge?
            if (_attributeMisMatch)
            {
               // merge the attributes
               bool doneMerging = true;
               AttributeSet inAttributes = inElement->getAllAttributes();
               AttributeSet::iterator inAttribIter;
               for (inAttribIter = inAttributes.begin(); inAttribIter != inAttributes.end(); ++inAttribIter)
               {
                  // new attribute?
                  std::string attribName = inAttribIter->getName();
                  // Don't merge replace attributes or Version attributes (Version gets added after the document has been merged)
                  if ((attribName.compare(sAttrReplace) != 0) && !hasAttribute(attribName))
                  {
                     if (attribName.compare("Version") == 0)
                        addAttribute(inAttribIter->getName(), "0.0");
                     else
                        addAttribute(inAttribIter->getName(), inAttribIter->getValue());
                     setDirty();
                     doneMerging = false;
                  }
               }

               if (doneMerging)
               {
                  break;
               }
            }
            else
            {
               // IT 2122
               // Name matches and attributes match, we're good to go...
               // and if nothing matches we still end up here, but at least we don't leave
               // with just a partial merge.
               break;
            }
         }
      }
   }
}

void Element::mergeAttributes(Element *inElement)
{
   // merge the attributes
   AttributeSet inAttributes = inElement->getAllAttributes();
   AttributeSet::iterator inAttribIter;
   for (inAttribIter = inAttributes.begin(); inAttribIter != inAttributes.end(); ++inAttribIter)
   {
      // new attribute?
      if (!hasAttribute(inAttribIter->getName()))
      {
         addAttribute(inAttribIter->getName(), inAttribIter->getValue());
         setDirty();
      }
   }
}

Element* Element::createCopy(Element* inParent)
{
   Element *output= new Element(getName(),inParent);
   XMLMemberTypeSet::iterator itMem=_vMembers.begin();
   AttributeSet::const_iterator itAtt = _attributes.begin();
   for (itAtt; itAtt != _attributes.end() ;itAtt++)
   {
      std::string attName = (*itAtt).getName();
      if (attName.compare(sAttrReplace) != 0)   // don't copy replace attributes
         output->addAttribute(attName, (*itAtt).getValue());
   }
   for (itMem; itMem != _vMembers.end() ;itMem++)
   {
      if ((*itMem)->isElement())
      {
         Element* TempElement = (static_cast<Element*>(*itMem));
         output->addChildElement(TempElement->createCopy(output));
      }
      if ((*itMem)->isData())
      {
         output->addData((static_cast<CharData*>(*itMem))->getDataString());
      }
      if ((*itMem)->isComment())
      {
         output->addComment(new Comment((static_cast<Comment*>(*itMem))->getDataString()));
      }
   }
   return output;
}

void Element::setOverrideAttributeFlag(Element *inElement)
{
   if (inElement && inElement->hasAttribute(sAttrReplace))
   {
      inElement->updateAttribute(sAttrReplace, sAttrValueTrue);
   }
   else if (inElement)
   {
      inElement->addAttribute(sAttrReplace, sAttrValueTrue);
   }
}

void Element::clearOverrideAttributeFlag(Element *inElement)
{
   if (inElement && inElement->hasAttribute(sAttrReplace))
   {
      inElement->updateAttribute(sAttrReplace, sAttrValueFalse);
   }
}

void Element::deleteOverrideAttributeFlag(Element *inElement)
{
   if (inElement && inElement->hasAttribute(sAttrReplace))
   {
      inElement->deleteAttribute(sAttrReplace);
   }
}

bool Element::isNameMatch(const Element* inElement, bool ignoreAttrInNameCompare)
{
   // see if names match
   _attributeMisMatch = false;
   bool output = (getName()==inElement->getName());
   if (output)
   {
      // are the attributes the same?
      AttributeSet incomingAtts = inElement->getAllAttributes();
      if (incomingAtts.size()>0)
      {
         AttributeSet::const_iterator itAtt = incomingAtts.begin();
         for (itAtt; itAtt != incomingAtts.end() ;itAtt++)
         {
            bool bHasAttr = false;
            if (ignoreAttrInNameCompare)
               bHasAttr = hasAttribute((*itAtt).getName());
            else
               bHasAttr = hasAttribute(*itAtt);

            if (!bHasAttr)
            {
               output=false;
               _attributeMisMatch = true;
               break;
            } // if has attribute
         } // for every attribute
      } // if  # of attributes>0
   } // if output
   return output;
}

bool Element::operator ==(Element &other)
{
   bool output;
   // see if name and string data are the same
   try
   {
      std::string theLocalValue = trim(this->getDataString());
      std::string theOtherValue = trim(other.getDataString());
      output=(theLocalValue == theOtherValue);// = XMLMemberType::operator ==(other);
      if (output)
      {
         // see if all attributes match
         output = isNameMatch(&other, false);
         if (output)
         {
            ElementSet thisChildren = getImmediateChildren();
            ElementSet otherChildren = other.getImmediateChildren();
            ElementSet::iterator itThisChildren=thisChildren.begin();
            ElementSet::iterator itOtherChildren=otherChildren.begin();

            //See if it has the same number of children
            output = (thisChildren.size()==otherChildren.size());
            if (output)
            {
               // for each one check all of this elements children to see if there
               // is a match with any of the others children
               for (itOtherChildren; itOtherChildren != otherChildren.end() ;itOtherChildren++)
               {
                  bool ChildMatches=false;
                  itThisChildren=thisChildren.begin();
                  for (itThisChildren; itThisChildren != thisChildren.end() ;itThisChildren++)
                  {
                     if (*(*(itThisChildren))==*(*(itOtherChildren)))
                     {
                        ChildMatches=true;

                        break;
                     }// if name match
                  }// for all my children

                  // if at the end none matched create a copy of the inElements data 
                  // and add it to this element
                  if (!ChildMatches)
                  {
                     output =false;
                     break;

                  }// if no matching child
               }// for all incoming children
            }
         }
      }
   }
   catch (std::string err)
   {
      output =false;
   }
   return output;
}
