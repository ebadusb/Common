#include <cxxtest/TestSuite.h>

#include "CharData.h"
#include "Comment.h"
#include "XMLParser.h"
#include "Attribute.h"
#include "Document.h"
#include "XMLMemberType.h"
#include "Element.h"
#include "FileParser.h"
#include <string>
#include <algorithm>
#include <iostream>
#include "ProtectedDocument.h"

using namespace BctXml;
using namespace std;

class GFix : public CxxTest::GlobalFixture
{
public:
  bool setUpWorld()
  {
    // create root element
    _refElement = new Element(_parentElementName);
    _refElement->setValue(_parentElementValue);

    // give it some attributes
    _refElement->addAttribute(_parentAttribute1Name,_parentAttribute1Value);
    _refElement->addAttribute(_parentAttribute2Name,_parentAttribute2Value);

    //give it a comment
    _refElement->addComment(new Comment(_parentComment));

    // add children
    Element* Child1 = new Element(_child1ElementName,_refElement);
    Child1->setValue(_child1ElementValue);
    Element* Child2 = new Element(_child2ElementName,_refElement);
    Child2->setValue(_child2ElementValue);

    _refElement->addChildElement(Child1);
    _refElement->addChildElement(Child2);
    _doc = new Document(_fileNameNoDecl);
    _doc->setRootElement(_refElement);

    // make no doc decl xml file
    _doc->write();

    // make doc decl xml file
    std::stringstream output;
    output<<"<?xml version=\""<<_version<<"\" encoding=\""<<_encoding<<"\" ?>";
    _refElement->toStream(output);
    std::ofstream file_op(_fileNameWithDecl.c_str(),std::ios::out);
    file_op<<output.str();
    file_op.close();

    // make no stand alone doc decl xml file
    std::stringstream output2;
    output2<<"<?xml version=\""<<_version<<"\" encoding=\""<<_encoding<<"\" standalone=\"no\" ?>";
    _refElement->toStream(output2);
    std::ofstream file_op2(_fileNameWithDeclNoStandalone.c_str(),std::ios::out);
    file_op2<<output2.str();
    file_op2.close();

    return true;
  }

  bool tearDownWorld()
  {
    delete _doc;
    #ifdef WIN32
      remove(_fileNameNoDecl.c_str());
      remove(_fileNameWithDecl.c_str()); 
      remove(_fileNameWithDeclNoStandalone.c_str());
    #endif
    return true;
  }

  static const std::string _parentElementName;
  static const std::string _parentElementValue;
  static const std::string _parentComment;
  static const std::string _parentAttribute1Name;
  static const std::string _parentAttribute1Value;
  static const std::string _parentAttribute2Name;
  static const std::string _parentAttribute2Value;
  static const std::string _child1ElementName;
  static const std::string _child1ElementValue;
  static const std::string _child2ElementName;
  static const std::string _child2ElementValue;
  static const std::string _fileNameNoDecl;
  static const std::string _fileNameWithDecl;
  static const std::string _fileNameWithDeclNoStandalone;
  static const std::string _version;
  static const std::string _encoding;

  Element* getRefElement()
  {
    return _refElement;
  }
  Document* getDoc()
  {
    return _doc;
  } 

  // Added to get access to the reference document 
  Document * Doc() const
  {
    return _doc;
  }

private:
  Element* _refElement;

  Document* _doc;
};

const std::string GFix::_parentElementName="ParentName";
const std::string GFix::_parentElementValue="Parent Value";
const std::string GFix::_parentComment="Parent Comment";
const std::string GFix::_child1ElementName="Child1Name";
const std::string GFix::_child1ElementValue="1";
const std::string GFix::_child2ElementName="Child2Name";
const std::string GFix::_child2ElementValue="2.1";

const std::string GFix::_parentAttribute1Name="att1Name";
const std::string GFix::_parentAttribute1Value="12";
const std::string GFix::_parentAttribute2Name="att2Name";
const std::string GFix::_parentAttribute2Value="14.3";

const std::string GFix::_version="1.0";
const std::string GFix::_encoding="utf-8";
const std::string GFix::_fileNameNoDecl="NoDocDecl.xml";
const std::string GFix::_fileNameWithDecl="WithDocDecl.xml";
const std::string GFix::_fileNameWithDeclNoStandalone="WithDocDeclNoStandalone.xml";
static GFix _gTheFixture;

class AttributeTest : public CxxTest::TestSuite
{
public:

  void setUp()
  {
    _testAttribute = new Attribute(_testName,_testValue); 
    return ;
  }

  void tearDown()
  {
    delete _testAttribute;
    return ; 
  }

  void xtest_Attribute()
  {
    // Constructor handled in setup & tear down
    //TS_WARN("EMPTY TEST: AttributeTest::test_Attribute()");
  }

  /// UNIT_TEST:  Tests whether XML values can be altered.
  /// ON_FAIL:    Indicates that XML values could not be altered correctly.
  void test_changeValue()
  {
    std::string oldValue = _testAttribute->getValue();
    std::string diffValue = oldValue+"Different";
    // change value and check
    _testAttribute->changeValue(diffValue);
    TS_ASSERT_EQUALS(_testAttribute->getValue(),diffValue);
    TS_ASSERT_DIFFERS(_testAttribute->getValue(),oldValue);
    // change value back and check
    _testAttribute->changeValue(oldValue);
    TS_ASSERT_EQUALS(_testAttribute->getValue(),oldValue);
    TS_ASSERT_DIFFERS(_testAttribute->getValue(),diffValue);
  }

  /// UNIT_TEST:  Tests whether or not the attribute name can be retrieved.
  /// ON_FAIL:    Indicates that the name of the attribute could not be retrieved.
  void test_getName()
  {
    TS_ASSERT_EQUALS(_testAttribute->getName(),_testName);
    TS_ASSERT_DIFFERS(_testAttribute->getName(),_otherString);
  }

  /// UNIT_TEST:  Tests whether or not the attribute value can be retrieved.
  /// ON_FAIL:    Indicates that the value of the attribute could not be retrieved.
  void test_getValue()
  {
    TS_ASSERT_EQUALS(_testAttribute->getValue(),_testValue);
    TS_ASSERT_DIFFERS(_testAttribute->getValue(),_otherString);
  }

  void xtest_operator_NT_EQ()
  {
    // Tested with == operator
    //TS_WARN("EMPTY TEST: AttributeTest::test_operator_NT_EQ()");
  }

  /// UNIT_TEST:  Tests the == operator.
  /// ON_FAIL:    Indicates that the == operator failed.
  void test_operator_EQ_EQ()
  {
    Attribute SameAttribute(_testAttribute->getName(),_testAttribute->getValue());
    Attribute DiffNameAttribute(_testAttribute->getName()+"Diff",_testAttribute->getValue());
    Attribute DiffValueAttribute(_testAttribute->getName(),_testAttribute->getValue()+"Diff");
    Attribute DiffBothAttribute(_testAttribute->getName()+"Diff",_testAttribute->getValue()+"Diff");
    TS_ASSERT(SameAttribute==(*_testAttribute));
    TS_ASSERT(SameAttribute!=(DiffNameAttribute));
    TS_ASSERT(SameAttribute!=(DiffValueAttribute));
    TS_ASSERT(SameAttribute!=(DiffBothAttribute));

    TS_ASSERT(!(SameAttribute!=(*_testAttribute)));
    TS_ASSERT(!(SameAttribute==(DiffNameAttribute)));
    TS_ASSERT(!(SameAttribute==(DiffValueAttribute)));
    TS_ASSERT(!(SameAttribute==(DiffBothAttribute)));
  }

private:
  static const std::string _testName;
  static const std::string _testValue;
  static const std::string _otherString;
  Attribute * _testAttribute;
};

const std::string AttributeTest::_testName="Test Attribute";
const std::string AttributeTest::_testValue="Test Value";
const std::string AttributeTest::_otherString="Other String";

class CharDataTest : public CxxTest::TestSuite
{
public:
  void setUp()
  {
    _testString = new CharData(_string);
    _testNumeric = new CharData(_numeric);
    _testWhitespace = new CharData(_whitespace);
    _testVoid = new CharData();

    return;
  }

  void tearDown()
  {
    delete _testString;
    delete _testNumeric;
    delete _testWhitespace;
    delete _testVoid;
    return;
  }

  void xtest_CharData()
  {
    //Constructor tested on setup and tear down
    //TS_WARN("EMPTY TEST: CharDataTest::test_CharData()");
  }

  /// UNIT_TEST:  Tests that a string value can be retrieved.
  /// ON_FAIL:    Indicates that a string value could not be retrieved.
  void test_getDataString()
  {
    // string
    TS_ASSERT_EQUALS(_testString->getDataString(),_string);
    TS_ASSERT_DIFFERS(_testString->getDataString(),_string+"DIFF");

    // numeric
    TS_ASSERT_EQUALS(_testNumeric->getDataString(),_numeric);
    TS_ASSERT_DIFFERS(_testNumeric->getDataString(),_numeric+"DIFF");

    // whitespace
    TS_ASSERT_EQUALS(_testWhitespace->getDataString(),_whitespace);
    TS_ASSERT_DIFFERS(_testWhitespace->getDataString(),_whitespace+"DIFF");

    // void
    TS_ASSERT_EQUALS(_testVoid->getDataString(),"");
    TS_ASSERT_DIFFERS(_testVoid->getDataString(),"DIFF");
  }

  /// UNIT_TEST:  Tests that a string value can be retrieved.
  /// ON_FAIL:    Indicates that a string value could not be retrieved.
  void test_getName()
  {
    TS_ASSERT_EQUALS(_testString->getName(),"");
    TS_ASSERT_DIFFERS(_testString->getName(),"DIFF");
  }

  /// UNIT_TEST:  Tests that a string value can be set.
  /// ON_FAIL:    Indicates that string values cannot be set.
  void test_setDataString()
  {
    // new string...
    std::string oldValue = _testString->getDataString();
    std::string newValue = oldValue +"New";
    _testString->setDataString(newValue);
    TS_ASSERT_EQUALS(_testString->getDataString(),newValue);
    TS_ASSERT_DIFFERS(_testString->getDataString(),oldValue);
    // back to original value...
    _testString->setDataString(oldValue);
    TS_ASSERT_EQUALS(_testString->getDataString(),oldValue);
    TS_ASSERT_DIFFERS(_testString->getDataString(),newValue);
  }

  /// UNIT_TEST:  Tests xml to stream conversion.
  /// ON_FAIL:    Indicates that the xml could not be converted to a stream.
  void test_toStream()
  {
    std::stringstream ans;
    _testString->toStream(ans);
    TS_ASSERT((ans.str().length())>0);
  }

private:
  static const std::string _string;
  static const std::string _numeric;
  static const std::string _whitespace;
  CharData * _testString;
  CharData * _testNumeric;
  CharData * _testWhitespace;
  CharData * _testVoid;
};

const std::string CharDataTest::_string="String Value";
const std::string CharDataTest::_numeric="756";
const std::string CharDataTest::_whitespace="   ";

class CommentTest : public CxxTest::TestSuite
{
public:
  void setUp()
  {
    _testString = new Comment(_string);
    return ;
  }

  void tearDown()
  {
    delete _testString;
    return ;
  }

  void xtest_Comment()
  {
    // Tested in setup and tear down
    //TS_WARN("EMPTY TEST: CommentTest::test_Comment()");
  }

  /// UNIT_TEST:  Tests that a string value can be retrieved.
  /// ON_FAIL:    Indicates that a string value could not be retrieved.
  void test_getDataString()
  {
    TS_ASSERT_EQUALS(_testString->getDataString(),_string);
    TS_ASSERT_DIFFERS(_testString->getDataString(),_string+"DIFF");
  }

  /// UNIT_TEST:  Tests that a string value can be retrieved.
  /// ON_FAIL:    Indicates that a string value could not be retrieved.
  void test_getName()
  {
    TS_ASSERT_EQUALS(_testString->getName(),"");
    TS_ASSERT_DIFFERS(_testString->getName(),"DIFF");
  }

  /// UNIT_TEST:  Tests that a string value can be set.
  /// ON_FAIL:    Indicates that a string value could not be set.
  void test_setDataString()
  {
    // new string...
    std::string oldValue = _testString->getDataString();
    std::string newValue = oldValue +"New";
    _testString->setDataString(newValue);
    TS_ASSERT_EQUALS(_testString->getDataString(),newValue);
    TS_ASSERT_DIFFERS(_testString->getDataString(),oldValue);

    // original string...
    _testString->setDataString(oldValue);
    TS_ASSERT_EQUALS(_testString->getDataString(),oldValue);
    TS_ASSERT_DIFFERS(_testString->getDataString(),newValue);
  }

  /// UNIT_TEST:  Tests xml to stream conversion.
  /// ON_FAIL:    Indicates that the xml could not be converted to a stream.
  void test_toStream()
  {
    std::stringstream ans;
    _testString->toStream(ans);
    TS_ASSERT((ans.str().length())>0);

    std::stringstream ans2;
    _testString->toStream(2,ans2);
    TS_ASSERT((ans.str().length())>0);
  }

private:
  static const std::string _string;
  Comment * _testString;
};

const std::string CommentTest::_string="Comment Data";

class DocumentTest : public CxxTest::TestSuite
{
public:
  /// UNIT_TEST:  Tests getting the root element of an XML document.
  /// ON_FAIL:    Root element was not retrieved successfully.
  void test_getRootElement()
  {
    TS_ASSERT((*(_gTheFixture.getDoc()->getRootElement()))==(*(_gTheFixture.getRefElement())));
  }

  void xtest_Document()
  {
    // Constructor tested in global setupworld
    //TS_WARN("EMPTY TEST: DocumentTest::test_Document()");
  }

  void xtest_isDirty()
  {
    // tested with setDirty
    //TS_WARN("EMPTY TEST: DocumentTest::test_isDirty()");
  }

  /// UNIT_TEST:  Tests file handling.
  /// ON_FAIL:    File handling is not working correctly.
  void test_isFileMissing()
  {
    // file missing
    Document theDoc(_gTheFixture._fileNameWithDecl);
    TS_ASSERT(theDoc.read());

    TS_ASSERT(!(theDoc.isFileMissing()));
    TS_ASSERT(!(theDoc.isFileUnreadable()));
#ifdef WIN32
    remove("SomeRandomFileThatDoesNotExist");
#endif
    Document theDoc2("SomeRandomFileThatDoesNotExist");
    TS_ASSERT(!theDoc2.read());

    // file does not exist
    TS_ASSERT((theDoc2.isFileMissing()));
    TS_ASSERT((theDoc2.isFileUnreadable()));

    std::fstream file_op("/garbage.xml",std::ios::out);
    file_op<<"Your standard non- XML garbage expected to cause an error\n";
    file_op.close();
    Document theDoc3("/garbage.xml");
    TS_TRACE("The following line should produce error messages indicating that the file couldn't be parsed");
    TS_ASSERT(!theDoc3.read());
    TS_TRACE("End of excpecting an error");

    // file exists
    TS_ASSERT(!(theDoc3.isFileMissing()));
    TS_ASSERT((theDoc3.isFileUnreadable()));  
#ifdef WIN32
    remove("garbage.xml");
#endif
  }

  void xtest_isFileUnreadable()
  {
    // Tested with isFileMissing
    //TS_WARN("EMPTY TEST: DocumentTest::test_isFileUnreadable()");
  }

  /// UNIT_TEST:  Tests the << operator.
  /// ON_FAIL:    Indicates that the << operator failed.
  void test_operator_LT_LT()
  {
    std::stringstream ans;
    ans<<(*(_gTheFixture.getDoc()));
    TS_ASSERT((ans.str().length())>0);
    std::cout<<std::endl<<std::endl<<"Document Using <<"<<std::endl;
    std::cout<<(*(_gTheFixture.getDoc()));
    std::cout<<std::endl;
  }

  /// UNIT_TEST:  Tests XML document printing.
  /// ON_FAIL:    Indicates that the print() failed to print the XML document.
  void test_print()
  {
    std::string printResults = _gTheFixture.getDoc()->print();
    TS_ASSERT((printResults.length())>0);
  }

  /// UNIT_TEST:  Tests XML document reading.
  /// ON_FAIL:    Indicates that the read() failed to read in the XML document.
  void test_read()
  {
    Document theDoc(_gTheFixture._fileNameWithDecl);
    TS_ASSERT(theDoc.read());
    TS_ASSERT((*(theDoc.getRootElement()))==(*(_gTheFixture.getRefElement())));
  }

  /*
    A bug was reported that when an xml file is read and then saved it changes
    in size.  This appears to be due to the prior version of the xml code making
    tokens out of white space and line feeds.  White space and line feeds are
    introduced by the formatting done by the xml output processing.  Therefore
    xml files tended to grow with extra white space and blank lines.  This test
    detected that problem and passed once that problem was addressed.
    */
  void test_FileMatch()
  {
    static const std::string gen1File( "BCFile");
    static const std::string gen2File( "BCFile" );
    Document doc1(_gTheFixture._fileNameWithDecl);
    doc1.read();
    doc1.write( gen1File );
    Document doc2( gen2File );
    doc2.read();
    doc2.write( "BCFile2" );

    // Compare the documents using the class's definition of equal.
    TS_ASSERT( IsEqual( _gTheFixture.Doc(), & doc1 ));
    TS_ASSERT( IsEqual( _gTheFixture.Doc(), & doc2 ));

    std::ifstream inStreamRef( _gTheFixture._fileNameWithDecl,ios::binary | ios::ate );
    std::ifstream inStreamGen1( gen1File,ios::binary | ios::ate );
    std::ifstream inStreamGen2( gen2File,ios::binary | ios::ate );

    _ULonglong nRefLen = inStreamRef.tellg();
    _ULonglong nGen1Len = inStreamGen1.tellg();
    _ULonglong nGen2Len = inStreamGen2.tellg();

    TS_ASSERT( nRefLen > 0 );
    TS_ASSERT_EQUALS( nRefLen, nGen1Len );
    TS_ASSERT_EQUALS( nRefLen, nGen2Len )
      
    inStreamRef.close();
    inStreamGen1.close();
    inStreamGen2.close();

    {
      // Since I can't seem to get the streams repositioned to their beginning,
      // we open some extras here for character-by-character compare.
      std::ifstream inStreamRef( _gTheFixture._fileNameWithDecl,ios::binary | ios::beg );
      std::ifstream inStreamRefx( _gTheFixture._fileNameWithDecl,ios::binary | ios::beg );
      std::ifstream inStreamGen1( gen1File,ios::binary | ios::beg );
      std::ifstream inStreamGen2( gen2File,ios::binary | ios::beg );

      TS_ASSERT( IsEqual( & inStreamRef, & inStreamGen1 ));
      TS_ASSERT( IsEqual( & inStreamRefx, & inStreamGen2 ));

      inStreamRef.close();
      inStreamRefx.close();
      inStreamGen1.close();
      inStreamGen2.close();
    }
  }

  // Helper routine to compare that the contents of two input file streams are equal.
  // (Note: if both file streams are length of zero, they are still equal.)
  bool IsEqual( std::ifstream * pStream1, std::ifstream * pStream2 )
  {
    bool bResult = true;

    while( ! pStream1->eof() && ! pStream2->eof())
    {
      char c1, c2;
      (* pStream1)>>c1;
      (* pStream2)>>c2;
      if( c1 != c2 )
      {
        bResult = false;
        break;
      }
    }
    return bResult && pStream1->eof() == pStream2->eof();
  }

  // Helper routine to compare two documents based on the class's
  // definition of equal content.
  bool IsEqual( Document * pD1, Document * pD2 )
  {
    Element * pRoot1 = pD1->getRootElement();
    Element * pRoot2 = pD2->getRootElement();

    return * pRoot1 == * pRoot2;
  }

  void test_Protected()
  {
    static const std::string sProtFileName = "TestProtectedFile.xml";
    static const std::string DumbCRCValue = "What the heck is this?";
    static const std::string CRCAttributeName = "CRC";

    bool bOK;
    std::string CRCValue, CRCValue2, CRCValueRegDoc;

    // read an unprotected file using the protected file class
    ProtectedDocument protDoc( _gTheFixture._fileNameNoDecl );
    bOK = protDoc.read();
    // Should have returned true since the document should not have been protected.
    TS_ASSERT( bOK );

    // show that element was added.  It should be empty.
    TS_ASSERT( NULL != protDoc.getRootElement());
    TS_ASSERT( NULL != protDoc.getRootElement()->hasAttribute( CRCAttributeName ));
    CRCValue = protDoc.getRootElement()->getAttributeValue( CRCAttributeName );
    TS_ASSERT( 0 == CRCValue.length() );

    // The base class has two write functions.
    // The protected document class should cover them both.
    protDoc.write( sProtFileName );

    // The CRC should now have an actual value
    CRCValue = protDoc.getRootElement()->getAttributeValue( CRCAttributeName );
    TS_ASSERT( 0 != CRCValue.length() );

    // See if all the characters are of the expected values

    for( std::string::iterator i = CRCValue.begin()+2; i != CRCValue.cend(); ++ i )
    {
      TS_ASSERT( *i >= '0' && *i <= '9' );
    }

    // set the CRC to a dumb thing
    protDoc.getRootElement()->updateAttribute( CRCAttributeName, DumbCRCValue );
    // Assure that the value is changed
    TS_ASSERT( protDoc.getRootElement()->getAttributeValue( CRCAttributeName ) == DumbCRCValue );

    // Write it not using an explicit file name and see that the CRC gets updated to the
    // same value as the previous write (the contents have not changed excelt for the CRC).
    protDoc.write( sProtFileName );
    TS_ASSERT( NULL != protDoc.getRootElement());
    TS_ASSERT( NULL != protDoc.getRootElement()->hasAttribute( CRCAttributeName ));
    CRCValue2 = protDoc.getRootElement()->getAttributeValue( CRCAttributeName );
    TS_ASSERT( CRCValue == CRCValue2 );

    // Show that our protected document is just another document and that we can read it as such
    ProtectedDocument pd1( sProtFileName );
    Document & regularDoc = pd1;
    regularDoc.read();

    TS_ASSERT( NULL != regularDoc.getRootElement());
    TS_ASSERT( NULL != regularDoc.getRootElement()->hasAttribute( CRCAttributeName ));
    CRCValueRegDoc = regularDoc.getRootElement()->getAttributeValue( CRCAttributeName );
    TS_ASSERT( CRCValue == CRCValueRegDoc );

    // Change the CRC value as read by the regular document and rewrite
    //regularDoc.getRootElement()->updateAttribute( CRCAttributeName, std::to_string( foo ));
    regularDoc.getRootElement()->updateAttribute( CRCAttributeName, std::to_string( (_ULonglong)( std::stoul( CRCValueRegDoc ) + 1 )));
    // Arguably, we should not have to do this but we do.
    regularDoc.getRootElement()->setDirty();
    regularDoc.write();
    // CRC should be messed up.

    // Document should be read but an error should have been noted.
    ProtectedDocument protDoc2( sProtFileName );
    bOK = protDoc2.read();
    // Not an OK return.
    TS_ASSERT( ! bOK );
    // but we have content.
    TS_ASSERT( NULL != protDoc2.getRootElement() );

    // Document is freshly read and should be clean
    TS_ASSERT( ! protDoc2.isDirty());
    protDoc2.getRootElement()->setDirty();
    // Document should be marked as dirty
    TS_ASSERT( protDoc2.isDirty());
    // Since it is dirty, write should work.
    protDoc2.write();
    // CRC should be all OK and therefore read should return true.
    bOK = protDoc.read();
    TS_ASSERT( bOK );
  }

  /// UNIT_TEST:  Tests XML document dirty bit.
  /// ON_FAIL:    Indicates that the dirty bit fn() manipulators failed.
  void test_setDirty()
  {
    Document theDoc(_gTheFixture._fileNameWithDecl);
    theDoc.read();
    TS_ASSERT(!theDoc.isDirty());

    // manually set dirty
    theDoc.setDirty();
    TS_ASSERT(theDoc.isDirty());

    // clear by writing doc
    theDoc.write("/TestDoc.xml");
    TS_ASSERT(!theDoc.isDirty());

    // make dirty by changing an element
    int data = 777;
    theDoc.getRootElement()->setValue(data);
    TS_ASSERT(theDoc.isDirty());

#ifdef WIN32
    remove("TestDoc.xml");
#endif
  }

  void xtest_setRootElement()
  {
    // tested in global fixture
    //TS_WARN("EMPTY TEST: DocumentTest::test_setRootElement()");
  }

  /// UNIT_TEST:  Tests xml to stream conversion.
  /// ON_FAIL:    Indicates that the xml could not be converted to a stream.
  void test_toStream()
  {
    std::stringstream ans;
    _gTheFixture.getDoc()->toStream(ans);
    TS_ASSERT((ans.str().length())>0);

    std::cout<<std::endl<<std::endl<<"Document using toStream"<<std::endl;
    _gTheFixture.getDoc()->toStream(std::cout);
    std::cout<<std::endl;
  }

  void xtest_write()
  {
    // write tested in global setupworld and confirmed in read
    //TS_WARN("EMPTY TEST: DocumentTest::test_write()");
  }
};

class FileParserTest : public CxxTest::TestSuite
{
public:
  /// UNIT_TEST:  Tests parsing the XML file.
  /// ON_FAIL:    Indicates that file parsing failed.
  void test_parseFile()
  {
    FileParser parser;
    Element* theRoot;
    // parse file constructed by the  with no doc declaration and see if it returns 
    // the original element
    theRoot =parser.parseFile(_gTheFixture._fileNameNoDecl);
    TS_ASSERT((*theRoot)==(*(_gTheFixture.getRefElement())));
    TS_ASSERT(!parser.documentHasDocDecl());
    TS_ASSERT(parser.documentIsStandAlone());

    // parse file constructed by the  with doc declaration and see if it returns 
    // the original element
    theRoot =parser.parseFile(_gTheFixture._fileNameWithDecl);
    TS_ASSERT((*(_gTheFixture.getRefElement()))==(*theRoot));

    // doc decl functions tested here 
    TS_ASSERT(parser.documentHasDocDecl());
    // stand alone tested here
    TS_ASSERT(parser.documentIsStandAlone());
    // version tested here
    TS_ASSERT_EQUALS(parser.documentVersion(),_gTheFixture._version);
    // encoding tested here
    TS_ASSERT_EQUALS(parser.documentEncoding(),_gTheFixture._encoding);

    delete theRoot;

    // parse file constructed by the  with doc declaration  and standalone = no and see if it returns 
    // the original element
    theRoot =parser.parseFile(_gTheFixture._fileNameWithDeclNoStandalone);
    TS_ASSERT((*theRoot)==(*(_gTheFixture.getRefElement())));
    TS_ASSERT(parser.documentHasDocDecl());
    // stand alone tested here
    TS_ASSERT(!parser.documentIsStandAlone());

    delete theRoot;
  }

  void xtest_FileParser()
  {
    // constructor tested in parse file
    //TS_WARN("EMPTY TEST: FileParserTest::test_FileParser()");
  }


  void xtest_documentEncoding()
  {
    // tested with parseFile
    //TS_WARN("EMPTY TEST: FileParserTest::test_documentEncoding()");
  }


  void xtest_documentHasDocDecl()
  {
    // tested with parseFile
    //TS_WARN("EMPTY TEST: FileParserTest::test_documentHasDocDecl()");
  }


  void xtest_documentIsStandAlone()
  {
    // tested with parseFile
    //TS_WARN("EMPTY TEST: FileParserTest::test_documentIsStandAlone()");
  }


  void xtest_documentVersion()
  {
    // tested with parseFile
    //TS_WARN("EMPTY TEST: FileParserTest::test_documentVersion()");
  }
};

class ElementTest : public CxxTest::TestSuite
{
public:
  void xtest_operator_NT_EQ()
  {
    //Tested with ==
    //TS_WARN("EMPTY TEST: ElementTest::test_operator_NT_EQ()");
  }

  /// UNIT_TEST:  Tests the == operator.
  /// ON_FAIL:    Indicates that the == operator failed.
  void test_operator_EQ_EQ()
  {
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);

    // add attributes and extra children
    setup_SearchChildrenTest(att1,att2);

    //add a comment to parent
    _testParent->addComment(new Comment("Blank"));

    Element * copyElement = _testParent->createCopy();

    // see if equivalent
    TS_ASSERT((*_testParent)==(*copyElement));

    // modify copy element and see if different
    std::string newCopyValue;
    copyElement->getValue(newCopyValue);
    newCopyValue=newCopyValue+"diff";
    copyElement->setValue(newCopyValue);
    TS_ASSERT((*_testParent)!=(*copyElement));

    delete copyElement;
    copyElement = _testParent->createCopy();

    // see if equivalent
    TS_ASSERT((*_testParent)==(*copyElement));

    // modify child element of original and see if different
    _testChild->getValue(newCopyValue);
    newCopyValue=newCopyValue+"diff";
    _testChild->setValue(newCopyValue);
    bool output = (*_testParent)!=(*copyElement);
    TS_ASSERT(output);

    delete copyElement;
  }

  /// UNIT_TEST:  Tests the createCopy().
  /// ON_FAIL:    Indicates that the createCopy() failed.
  void test_createCopy()
  {
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);

    // add attributes and extra children
    setup_SearchChildrenTest(att1,att2);

    //add a comment to parent
    _testParent->addComment(new Comment("Blank"));

    Element * copyElement = _testParent->createCopy();
    TS_ASSERT_EQUALS(_testParent->print(),copyElement->print());

    //Test merge functionality
    std::string testMergeString = "Some Other Value";
    std::string testMergeElementName = "SomeChild";
    Element * newEl = new Element(testMergeElementName,copyElement);
    newEl->setValue(testMergeString);
    copyElement->addChildElement(newEl);

    // Test and add only of the merge this should make the merged element look just like the 
    //  element it merged with
    _testParent->merge(copyElement);
    TS_ASSERT_EQUALS(_testParent->print(),copyElement->print());

    // Test the merge where one of the non new values changed
    std::string newTestMergeString = testMergeString+" diff";
    newEl->setValue(newTestMergeString);
    _testParent->merge(copyElement);
    TS_ASSERT_DIFFERS(_testParent->print(),copyElement->print());

    Element * TestElement = _testParent->findFirstChild(testMergeElementName);
    std::string testValue;
    testValue=TestElement->getDataString();
    TS_ASSERT_EQUALS(testValue,testMergeString);

    delete copyElement;
  }

  /// UNIT_TEST:  Tests the getParent().
  /// ON_FAIL:    Indicates that the getParent() failed.
  void test_getParent()
  {
    Element* Parent = _testChild->getParent();
    TS_ASSERT_EQUALS(Parent->getName(),_testParent->getName());
  }

  void setUp()
  {
    _testParent = new Element(_parentName);

    _testChild = new Element(_childName,_testParent);
    _testSecondChild = new Element(_secondChildName,_testParent);
    _testParent->addChildElement(_testChild);
    _testParent->addChildElement(_testSecondChild);

    _testParent->setValue(_parentInfo);
    _testChild->setValue(_childInfo);
    _testSecondChild->setValue(_secondChildInfo);

    return ;
  }

  void tearDown()
  {
    delete _testParent;
    return ;
  }

  /// UNIT_TEST:  Tests various child/parent functions on an element that has parent element.
  /// ON_FAIL:    Handling children elements is not working correctly.
  void test_ElementWithParent()
  {
    // This test comes for IT 755

    // Element with parent
    BctXml::Element *xstatus = new BctXml::Element("status");
    BctXml::Element *xcode = new BctXml::Element("code",xstatus);
    xcode->setValue((404));

    TS_ASSERT(xstatus != NULL);
    TS_ASSERT(xcode != NULL);
    Element* el = xstatus->findFirstChild("code");
    TS_ASSERT(el == xcode);
    unsigned int value;
    el->getValue(value);
    TS_ASSERT(value == 404);

    TS_ASSERT(xcode->getParent() == xstatus);
  }

  /// UNIT_TEST:  Tests a single element.
  /// ON_FAIL:    Handling of elements is not working correctly.
  void test_Element()
  {
    Element* myElement = new Element("Junk");
    int val = 72;
    myElement->setValue(val);
    int valRead;
    myElement->getValue(valRead);
    TS_ASSERT(valRead == val);
    delete myElement;
  }

  /// UNIT_TEST:  Tests adding an attribute to an xml element.
  /// ON_FAIL:    Indicates that an attribute could not be added to an xml element.
  void test_addAttribute()
  {
    _testParent->addAttribute(_parentAttributeName,_parentAttributeValue);
    std::string result = _testParent->getAttributeValue(_parentAttributeName);
    TS_ASSERT_EQUALS(result,_parentAttributeValue);

    // Test get all attributes 
    _testParent->addAttribute(_childAttributeName,_parentAttributeValue);
    AttributeSet resultSet = _testParent->getAllAttributes();
    TS_ASSERT(resultSet.size()>1);
  }

  /// UNIT_TEST:  Tests adding a child element to an xml element.
  /// ON_FAIL:    Indicates that a child element could not be added to an xml element.
  void test_addChildElement()
  {
    std::string thirdChildData ="Third Child";
    Element * ThirdChild = new Element(thirdChildData);
    _testParent->addChildElement(ThirdChild);
    Element * result = _testParent->findFirstChild(thirdChildData);
    TS_ASSERT_EQUALS(result->getName(),ThirdChild->getName());
  }

  /// UNIT_TEST:  Tests adding a comment to an xml element.
  /// ON_FAIL:    Indicates that a comment could not be added to an xml element.
  void test_addComment()
  {
    _testParent->addComment(new Comment(_parentComment));
    _testChild->addComment(new Comment(_childComment));

    CommentSet result = _testParent->getComments();
    TS_ASSERT(result.size()==1)
    Comment* output = (Comment *)(*(result.begin()));
    TS_ASSERT_EQUALS(output->getDataString(),_parentComment);
  }

  /// UNIT_TEST:  Tests adding data to an xml element.
  /// ON_FAIL:    Indicates that data could not be added to an xml element.
  void test_addData()
  {
    Element * localElement = new Element();

    localElement->addData(_parentName);
    std::string result = localElement->getDataString();
    TS_ASSERT_EQUALS(result,_parentName);
    TS_ASSERT_DIFFERS(result,_parentName+"DIFF");

    delete localElement;
  }

  void setup_SearchChildrenTest(Attribute at1, Attribute at2)
  {
    Element * localElement1;
    Element * localElement2;
    Element * localElement3;

    // local element 1 has both atts, local 2 only has 1 att
    localElement1 = new Element(_childName);
    localElement2 = new Element(_childName);
    localElement3 = new Element(_childName+"diff");
    _testChild->addChildElement(localElement1);
    _testChild->addChildElement(localElement2);
    _testChild->addChildElement(localElement3);
    localElement1->addAttribute(at1.getName(),at1.getValue());
    localElement1->addAttribute(at2.getName(),at2.getValue());
    localElement2->addAttribute(at1.getName(),at1.getValue());
  }

  /// UNIT_TEST:  Tests deleting a child element from an xml element.
  /// ON_FAIL:    Indicates that a child element could not be deleted from an xml element.
  void test_deleteChildren()
  {
    AttributeSet TheASet;
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    TheASet.push_back(att1);
    TheASet.push_back(att2);

    ElementSet Check;
    int BeforeNumElements=0;
    int AfterNumElements=0;

    // Test delete with att set
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteChildren(_childName,TheASet);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete where only 1 has the att
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteChildren(_childName,att2);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete where both have the att
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteChildren(_childName,att1);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+2),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete with name
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteChildren(_childName);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+2),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete 
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteChildren();
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT(AfterNumElements==0);
  }

  /// UNIT_TEST:  Tests deleting the first child from an xml element.
  /// ON_FAIL:    Indicates that the first child could not be deleted correctly.
  void test_deleteFirstChild()
  {
    AttributeSet TheASet;
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    TheASet.push_back(att1);
    TheASet.push_back(att2);

    ElementSet Check;
    int BeforeNumElements=0;
    int AfterNumElements=0;

    // Test delete with att set
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteFirstChild(_childName,TheASet);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete where only 1 has the att
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteFirstChild(_childName,att2);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete where both have the att
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteFirstChild(_childName,att1);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();

    // test delete with name
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->getImmediateChildren();
    BeforeNumElements=(int)Check.size();
    _testChild->deleteFirstChild(_childName);
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT_EQUALS((AfterNumElements+1),BeforeNumElements);
    _testChild->deleteChildren();
  }

  /// UNIT_TEST:  Tests finding children.
  /// ON_FAIL:    XML Children could not be found.
  void test_findChildren()
  {
    AttributeSet TheASet;
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    TheASet.push_back(att1);
    TheASet.push_back(att2);

    ElementSet Check;
    Element * FoundFirstElement;
    int BeforeNumElements=0;
    int AfterNumElements=0;

    // Test search with att set
    setup_SearchChildrenTest(att1,att2);

    Check = _testChild->findChildren(_childName,TheASet);
    AfterNumElements=(int)Check.size();
    TS_ASSERT((AfterNumElements==1))
    FoundFirstElement=(Element *)(*(Check.begin()));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att1.getName()),att1.getValue());
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att2.getName()),att2.getValue());

    // test search where only 1 has the att
    Check = _testChild->findChildren(_childName,att2);
    AfterNumElements=(int)Check.size();
    TS_ASSERT((AfterNumElements==1))
    FoundFirstElement=(Element *)(*(Check.begin()));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att2.getName()),att2.getValue());

    // test search where both have the att
    Check = _testChild->findChildren(_childName,att1);
    AfterNumElements=(int)Check.size();
    TS_ASSERT((AfterNumElements==2))
    FoundFirstElement=(Element *)(*(Check.begin()));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att1.getName()),att1.getValue());
    FoundFirstElement=(Element *)(&(*(Check.back())));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att1.getName()),att1.getValue());

    // test Find with name
    Check = _testChild->findChildren(_childName);
    AfterNumElements=(int)Check.size();
    TS_ASSERT((AfterNumElements==2));
    FoundFirstElement=(Element *)(*(Check.begin()));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    FoundFirstElement=(Element *)(&(*(Check.back())));
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);

    // test getImmediateChildren which is quite similar
    Check = _testChild->getImmediateChildren();
    AfterNumElements=(int)Check.size();
    TS_ASSERT((AfterNumElements==3));
  }

  /// UNIT_TEST:  Tests finding the first child of an xml element.
  /// ON_FAIL:    Indicates that the first child could not be found.
  void test_findFirstChild()
  {
    AttributeSet TheASet;
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    TheASet.push_back(att1);
    TheASet.push_back(att2);

    Element * FoundFirstElement;
    int BeforeNumElements=0;
    int AfterNumElements=0;

    // Test search with att set
    setup_SearchChildrenTest(att1,att2);

    FoundFirstElement = _testChild->findFirstChild(_childName,TheASet);
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att1.getName()),att1.getValue());
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att2.getName()),att2.getValue());

    // test search where only 1 has the att
    FoundFirstElement = _testChild->findFirstChild(_childName,att2);
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att2.getName()),att2.getValue());

    // test search where both have the att
    FoundFirstElement = _testChild->findFirstChild(_childName,att1);
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getAttributeValue(att1.getName()),att1.getValue());

    // test Find with name
    FoundFirstElement = _testChild->findFirstChild(_childName);
    TS_ASSERT_EQUALS(FoundFirstElement->getName(),_childName);
  }

  void xtest_getAllAttributes()
  {
    //tested in addAttribute
  }

  /// UNIT_TEST:  Tests that all children from an xml element can be retrieved.
  /// ON_FAIL:    Indicates that not all of the children were able to be retrieved.
  void test_getAllChildren()
  {
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    setup_SearchChildrenTest(att1,att2);
    ElementSet resultSet = _testParent->getAllChildren();

    TS_ASSERT(resultSet.size()>3);
  }

  void xtest_getAttributeValue()
  {
    //Tested in addAttribute
    //TS_WARN("EMPTY TEST: ElementTest::test_getAttributeValue()");
  }

  void xtest_getImmediateChildren()
  {
    // Tested at end of FindChildren
  }

  void xtest_getComments()
  {
    //Tested with add comment
    //TS_WARN("EMPTY TEST: ElementTest::test_getComments()");
  }

  void xtest_getDataString()
  {
    //Tested with addData()
    //TS_WARN("EMPTY TEST: ElementTest::test_getDataString()");
  }

  void xtest_getName()
  {
    // Tested in find Tests
    //TS_WARN("EMPTY TEST: ElementTest::test_getName()");
  }

  /// UNIT_TEST:  Tests getting a value from the xml element
  /// ON_FAIL:    Indicates that the value was not able to be retrieved.
  void test_getValue()
  {
    // int
    Element myElement("TestSetGetValue");
    int iValIn = 3;
    myElement.setValue(iValIn);
    int iValOut;
    myElement.getValue(iValOut);
    TS_ASSERT_EQUALS(iValIn,iValOut);

    // float
    float fValIn = 3.5;
    myElement.setValue(fValIn);
    float fValOut;
    myElement.getValue(fValOut);
    TS_ASSERT_EQUALS(fValIn,fValOut);

    // bool
    bool bValIn = true;
    myElement.setValue(bValIn);
    bool bValOut;
    myElement.getValue(bValOut);
    TS_ASSERT_EQUALS(bValIn,bValOut);

    // string
    std::string sValIn = "Junk";
    myElement.setValue(sValIn);
    std::string sValOut;
    myElement.getValue(sValOut);
    TS_ASSERT_EQUALS(sValIn,sValOut);

    // string with whitespace
    std::string sValIn2 = "Junk with whitespace";
    myElement.setValue(sValIn2);
    myElement.addComment(new Comment("Test Comment"));
    std::string sValOut2;
    myElement.getValue(sValOut2);
    TS_ASSERT_EQUALS(sValIn,sValOut2);
  }

  /// UNIT_TEST:  Tests the hasAttribute().
  /// ON_FAIL:    Indicates that the hasAttribute() has failed.
  void test_hasAttribute()
  {
    Element theElement("TestHasAttribute");
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);
    theElement.addAttribute(att1.getName(),att1.getValue());

    TS_ASSERT(theElement.hasAttribute(att1.getName()));
    TS_ASSERT(!theElement.hasAttribute(att2.getName()));
  }

  /// UNIT_TEST:  Tests the hasChildren().
  /// ON_FAIL:    Indicates that the hasChildren() has failed.
  void test_hasChildren()
  {
    TS_ASSERT(_testParent->hasChildren());
    TS_ASSERT(!_testChild->hasChildren());
  }

  /// UNIT_TEST:  Tests the isDirty().
  /// ON_FAIL:    Indicates that the isDirty() has failed.
  void test_isDirty()
  {
    // dirty
    Element theElement("TestHasAttribute");
    TS_ASSERT(!theElement.isDirty());
    theElement.setDirty();
    TS_ASSERT(theElement.isDirty());
    // clean
    theElement.setClean();
    TS_ASSERT(!theElement.isDirty());
  }

  // UNIT_TEST: Tests that the merge function works as expected.
  // ON_FAIL:   Merge did not work as expected.
  void test_merge()
  {
    // Note:  Some Merge testing is done in the test_createCopy() 

    // Setup
    Element *childElement = new Element("Child");
    childElement->addAttribute("Type", "SomeType");

    // Different Version numbers...
    Element *origRoot = new Element("AnElement");
    Element *newRoot = new Element("AnElement");
    Attribute att1("Attribute1", "Attribute1Value");
    Attribute att2("Attribute2", "Attribute2Value");
    Attribute ver1("Version", "1.0");
    Attribute ver2("Version", "1.5");
    Attribute newAtt("NewAttribute", "NewAttrValue");

    origRoot->addAttribute(att1.getName(), att1.getValue());
    origRoot->addAttribute(att2.getName(), att2.getValue());
    origRoot->addAttribute(ver1.getName(), ver1.getValue());

    newRoot->addAttribute(att1.getName(), att1.getValue());
    newRoot->addAttribute(att2.getName(), att2.getValue());
    newRoot->addAttribute(ver2.getName(), ver2.getValue());

    newRoot->addChildElement(childElement);

    // We expect NO merging to take place...
    // -------------------------------------
    // This tests the true/false parameter of merge()
    origRoot->merge(newRoot, false);
    TS_ASSERT_DIFFERS(origRoot->print(), newRoot->print());
    ElementSet origSet = origRoot->getAllChildren();
    ElementSet newSet = newRoot->getAllChildren();
    TS_ASSERT(origSet.size() == 0);
    TS_ASSERT(newSet.size() == 1);

    // We expect a merge to take place...
    // -------------------------------------
    // This tests the true/false parameter of merge()
    origRoot->merge(newRoot, true);
    TS_ASSERT_DIFFERS(origRoot->print(), newRoot->print()); // versions should still be different...
    origSet = origRoot->getAllChildren(); // ...but the children should have merged.
    newSet = newRoot->getAllChildren();
    TS_ASSERT(origSet.size() == 1);
    TS_ASSERT(newSet.size() == 1);
    ElementSet::iterator origIter = origSet.begin();  // only 1 child at this point, using begin() is all we need.
    ElementSet::iterator newIter = newSet.begin();
    TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());

    TS_ASSERT(origRoot->getAttributeValue("Version").compare("1.0") == 0);  // just extra checking
    TS_ASSERT(newRoot->getAttributeValue("Version").compare("1.5") == 0);   // just extra checking
    origRoot->updateAttribute(ver1.getName(), "1.5");
    TS_ASSERT_EQUALS(origRoot->print(), newRoot->print());

    // We expect a merge to take place...
    // -----------------------------------
    // Make sure that "true" for ignoreAttrInNameCompare doesn't affect 
    // new attributes that need to be merged.
    newRoot->addAttribute(newAtt.getName(), newAtt.getValue());
    origRoot->merge(newRoot, true);
    TS_ASSERT_EQUALS(origRoot->print(), newRoot->print());

    // We expect a merge to take place...
    // ----------------------------------
    Element *newChildElement = new Element("Child");
    newChildElement->addAttribute("Type", "SomeNewType");
    newRoot->addChildElement(newChildElement);
    origRoot->merge(newRoot, true);
    TS_ASSERT_EQUALS(origRoot->print(), newRoot->print());
    origSet = origRoot->getAllChildren();
    newSet = newRoot->getAllChildren();
    origIter = origSet.begin();
    newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    // We expect a merge to take place...
    newChildElement->addValueChildElement<double>("Option", 1.0);
    newChildElement->addValueChildElement<double>("Option2", 2.0);
    origRoot->merge(newRoot, true);
    TS_ASSERT_EQUALS(origRoot->print(), newRoot->print());
    origSet = origRoot->getAllChildren();
    newSet = newRoot->getAllChildren();
    origIter = origSet.begin();
    newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    delete origRoot;
    delete newRoot;
  }

    void test_replaceAttribute()
  {
    Element *origRoot = new Element("AnElement");
    Element *newRoot = new Element("AnElement");

    // We DO NOT expect a merge to take place...
    origRoot->addAttribute("Version", "1.0");
    newRoot->addAttribute("Version", "1.0");

    // this should be the new data in the merged file...(once we update the version number below)
    newRoot->addValueChildElement<int>("SomeValueToReplace", 55);
    newRoot->setOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace"));
    newRoot->addValueChildElement<int>("SomeValueToReplace2", 56);
    newRoot->setOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace2"));
    newRoot->addValueChildElement<int>("SomeValueToReplace3", 57);
    newRoot->setOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace3"));

    // this should be the old value that gets blown away...(once we update the version number below)
    origRoot->addValueChildElement<int>("SomeValueToReplace", 99);
    origRoot->addValueChildElement<int>("SomeValueToReplace2", 100);
    origRoot->addValueChildElement<int>("SomeValueToReplace3", 101);

    std::cout << std::endl << std::endl << "........................" << std::endl;
    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (NO MERGE EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;
    
    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();

    // No merge should have taken place...
    ElementSet origSet = origRoot->getAllChildren();
    ElementSet newSet = newRoot->getAllChildren();
    ElementSet::iterator origIter = origSet.begin();
    ElementSet::iterator newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_DIFFERS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    // We EXPECT a merge / replace to take place...
    newRoot->updateAttribute("Version", "1.1");

    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (MERGE EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;

    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();
    std::cout << std::endl << std::endl << "........................" << std::endl;

    // all children should be equal now; note that element with "replace = true" should not be compared 
    // because that is not part of the merge process...so we need to remove it for the next part...
    newRoot->deleteOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace"));
    newRoot->deleteOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace2"));
    newRoot->deleteOverrideAttributeFlag(newRoot->findFirstChild("SomeValueToReplace3"));

    origSet = origRoot->getAllChildren();
    newSet = newRoot->getAllChildren();
    origIter = origSet.begin();
    newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    delete origRoot;
    delete newRoot;
  }

  void test_replaceAttributeComplex()
  {
    Element *origRoot = new Element("AnElement");
    Element *newRoot = new Element("AnElement");

    // We DO NOT expect a merge to take place...
    origRoot->addAttribute("Version", "1.0");
    newRoot->addAttribute("Version", "1.0");

    Element *someChild = new Element("SomeChild");
    someChild->addAttribute("childAttr", "blah");
    someChild->addAttribute("anotherAttr", "blah2");
    
    Element *deepChild = new Element("deepChild");
    deepChild->addAttribute("deepAttr", "blah3");

    Element *deeperChild = new Element("deeperChild");
    deeperChild->addValueChildElement<int>("myData", 5);

    deepChild->addChildElement(deeperChild);
    someChild->addChildElement(deepChild);
    origRoot->addChildElement(someChild);
    
    // now what we plan on replacing...everything from "deepChild" on down...that is
    // once the version number is updated below...
    Element *newSomeChild = new Element("SomeChild");
    newSomeChild->addAttribute("childAttr", "blah");
    newSomeChild->addAttribute("anotherAttr", "blah2");

    Element *newDeepChild = new Element("deepChild");
    newDeepChild->addAttribute("deepAttr", "blah3");
    
    Element *newDeeperChild = new Element("deeperChild");
    newDeeperChild->addValueChildElement<int>("myData", 30);

    newDeepChild->addChildElement(newDeeperChild);
    newSomeChild->addChildElement(newDeepChild);

    // let the system know that everyong here on down should be replaced...again, once the
    // version number is updated below...
    newSomeChild->setOverrideAttributeFlag(newSomeChild->findFirstChild("deepChild"));

    newRoot->addChildElement(newSomeChild);

    std::cout << std::endl << std::endl << "........................" << std::endl;
    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (MERGE NOT EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;

    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();

    ElementSet origSet = origRoot->getAllChildren();
    ElementSet newSet = newRoot->getAllChildren();
    ElementSet::iterator origIter = origSet.begin();
    ElementSet::iterator newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_DIFFERS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }
  
    // Now we EXPECT a merge to take place...
    newRoot->updateAttribute("Version", "1.1");

    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (MERGE EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;

    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();
    std::cout << std::endl << std::endl << "........................" << std::endl;

    // all children should be equal now; note that element with "replace = true" should not be compared 
    // because that is not part of the merge process...so we need to remove it for the next part...
    newSomeChild->deleteOverrideAttributeFlag(newSomeChild->findFirstChild("deepChild"));

    origSet = origRoot->getAllChildren();
    newSet = newRoot->getAllChildren();
    origIter = origSet.begin();
    newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    delete origRoot;
    delete newRoot;
  }
  
  void test_replaceUserSettings()
  {
    // We do not expect a merge to take place...
    Element *origRoot = new Element("UserSettings");
    Element *newRoot = new Element("UserSettings");

    origRoot->addAttribute("Version", "1.1");
    origRoot->addAttribute("FileName", "/config");
    origRoot->addAttribute("ReadOnly", "false");
    origRoot->addAttribute("ServiceChangeable", "false");

    Element *alarmSoundTimeout = new Element("AlarmSoundTimeout");
    alarmSoundTimeout->addData("60000");
    origRoot->addChildElement(alarmSoundTimeout);
    Element *muteAllAlarms = new Element("MuteAllAlarms");
    muteAllAlarms->addData("false");
    origRoot->addChildElement(muteAllAlarms);
    Element *decimalSeparatorIsPeriod = new Element("DecimalSeparatorIsPeriod");
    decimalSeparatorIsPeriod->addData("true");
    origRoot->addChildElement(decimalSeparatorIsPeriod);
    
    newRoot->addAttribute("Version", "1.1");
    newRoot->addAttribute("FileName", "/config");
    newRoot->addAttribute("ReadOnly", "false");
    newRoot->addAttribute("ServiceChangeable", "false");

    Element *newAlarmSoundTimeout = new Element("AlarmSoundTimeout");
    newAlarmSoundTimeout->addData("1000");
    newRoot->setOverrideAttributeFlag(newAlarmSoundTimeout);
    newRoot->addChildElement(newAlarmSoundTimeout);
    Element *newMuteAllAlarms = new Element("MuteAllAlarms");
    newMuteAllAlarms->addData("false");
    newRoot->addChildElement(newMuteAllAlarms);
    Element *newDecimalSeparatorIsPeriod = new Element("DecimalSeparatorIsPeriod");
    newDecimalSeparatorIsPeriod->addData("true");
    newRoot->addChildElement(newDecimalSeparatorIsPeriod);

    std::cout << std::endl << std::endl << "........................" << std::endl;
    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (NO MERGE EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;

    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();

    // At this point, only the first (merged and replaced) element should differ, 
    // the rest of the tree should match.
    ElementSet origSet = origRoot->getAllChildren();
    ElementSet newSet = newRoot->getAllChildren();
    ElementSet::iterator origIter = origSet.begin();
    ElementSet::iterator newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
       if (origIter == origSet.begin())
       {
         TS_ASSERT_DIFFERS((*origIter)->print(), (*newIter)->print());
       }
       else
       {
         TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
       }
       ++newIter;
    }

    // Now we EXPECT a merge to take place...
    newRoot->updateAttribute("Version", "2.0");

    std::cout << std::endl << std::endl << "test_replaceAttribute: Test Pre-Merge (MERGE EXPECTED)";
    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print() << std::endl;

    origRoot->merge(newRoot, true);

    std::cout << std::endl << std::endl << "test_replaceAttribute: OrigRoot: " << origRoot->print();
    std::cout << std::endl << std::endl << "test_replaceAttribute: NewRoot: " << newRoot->print();
    std::cout << std::endl << std::endl << "........................" << std::endl;

    // all children should be equal now; note that element with "replace = true" should not be compared 
    // because that is not part of the merge process...so we need to remove it for the next part...
    newRoot->deleteOverrideAttributeFlag(newRoot->findFirstChild("AlarmSoundTimeout"));

    origSet = origRoot->getAllChildren();
    newSet = newRoot->getAllChildren();
    origIter = origSet.begin();
    newIter = newSet.begin();
    for (; origIter != origSet.end(); ++origIter)
    {
      TS_ASSERT_EQUALS((*origIter)->print(), (*newIter)->print());
      ++newIter;
    }

    delete origRoot;
    delete newRoot;
  }
  
  void xtest_replaceReadInDocument()
  {
      // Test merging with REAL task settings...must create these files first
      // Files are easily created by adding code to TaskManagementTest and then running
      // that Test suite.
      //
      // TaskSettings* harvestSettings = TaskSettingsManager::instance().createSettings(UserTasks::HARVEST_CELLS); 
      // BctXml::Document *harvestDocument = new BctXml::Document("C:\\c_NewHarvestOptions.xml");
      // harvestDocument->setRootElement(harvestSettings->optionsToXmlElement()->getRootElement());
      // harvestDocument->write();
      // delete harvestDocument;
      // delete harvestSettings;
      //
      Document *harvestDoc = new Document("C:\\c_HarvestOptions.xml");
      Document *newHarvestDoc = new Document("C:\\c_NewHarvestOptions.xml");

      harvestDoc->read();
      newHarvestDoc->read();

      harvestDoc->getRootElement()->merge(newHarvestDoc->getRootElement(), true);

      harvestDoc->write("C:\\c_MergedHarvestOptions.xml");

      delete harvestDoc;
      delete newHarvestDoc;
  }

  void xtest_setClean()
  {
    //Tested in isDirty
    //TS_WARN("EMPTY TEST: ElementTest::test_setClean()");
  }


  void xtest_setDirty()
  {
    //Tested in isDirty
    //TS_WARN("EMPTY TEST: ElementTest::test_setDirty()");
  }


  void xtest_setValue()
  {
    //Tested with get value
    //TS_WARN("EMPTY TEST: ElementTest::test_setValue()");
  }


  /// UNIT_TEST:  Tests the toStream function.
  /// ON_FAIL:    Indicates that the toStream function has failed.
  void test_toStream()
  {
    std::stringstream ans;
    _testParent->toStream(ans);
    TS_ASSERT((ans.str().length())>0);
    std::cout<<std::endl<<std::endl<<"Stream of Parent Element Data"<<std::endl;
    _gTheFixture.getDoc()->toStream(std::cout);
    std::cout<<std::endl;
  }

  /// UNIT_TEST:  Tests the updateAttribute() function.
  /// ON_FAIL:    Indicates that the updateAttribute() function has failed.
  void test_updateAttribute()
  {
    AttributeSet TheASet;
    std::string Att1Name = "Att1";
    std::string Att1Value = "Att1 Value";
    std::string Att2Name = "Att2";
    std::string Att2Value = "Att2 Value";
    std::string newValue = "newValue";
    Attribute att1(Att1Name,Att1Value);
    Attribute att2(Att2Name,Att2Value);

    // add attribute
    _testChild->addAttribute(Att1Name,Att1Value);
    _testChild->addAttribute(Att2Name,Att2Value);

    TS_ASSERT_EQUALS( _testChild->getAttributeValue(Att1Name),Att1Value);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att1Name),newValue);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att1Name),Att2Value);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att2Name),Att1Value);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att2Name),newValue);
    TS_ASSERT_EQUALS( _testChild->getAttributeValue(Att2Name),Att2Value);

    // update attribute
    _testChild->updateAttribute(Att1Name,newValue);

    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att1Name),Att1Value);
    TS_ASSERT_EQUALS( _testChild->getAttributeValue(Att1Name),newValue);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att1Name),Att2Value);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att2Name),Att1Value);
    TS_ASSERT_DIFFERS( _testChild->getAttributeValue(Att2Name),newValue);
    TS_ASSERT_EQUALS( _testChild->getAttributeValue(Att2Name),Att2Value);
  }

private:
  static const std::string _parentInfo;
  static const std::string _childInfo;
  static const std::string _secondChildInfo;
  static const std::string _parentName;
  static const std::string _childName;
  static const std::string _secondChildName;
  static const std::string _parentAttributeName;
  static const std::string _childAttributeName;
  static const std::string _parentAttributeValue;
  static const std::string _childAttributeValue;
  static const std::string _parentComment;
  static const std::string _childComment;

  Element * _testParent;
  Element * _testChild;
  Element * _testSecondChild;
};

const std::string ElementTest::_parentInfo="Parent Element Info";
const std::string ElementTest::_childInfo="Child Element Info";
const std::string ElementTest::_secondChildInfo="Second Child Element Info";
const std::string ElementTest::_parentName="Parent Element Name";
const std::string ElementTest::_childName="Child Element Name";
const std::string ElementTest::_secondChildName="Second Child Element Name";
const std::string ElementTest::_parentAttributeName="Parent Attribute Name";
const std::string ElementTest::_childAttributeName="Child Attribute Name";

const std::string ElementTest::_parentAttributeValue="Parent Attribute Value";
const std::string ElementTest::_childAttributeValue="Child Attribute Value";

const std::string ElementTest::_parentComment="Parent Comment";
const std::string ElementTest::_childComment="Child Comment";

class GlobalFunctions_Document_h_Test : public CxxTest::TestSuite
{
public:
  void xtest_operator_LT_LT()
  {
    //Tested with Document class
    //TS_WARN("EMPTY TEST: GlobalFunctions_Document_h_Test::test_operator_LT_LT()");
  }
};


class GlobalFunctions_XMLMemberType_h_Test : public CxxTest::TestSuite
{
public:
  void xtest_operator_LT_LT()
  {
    //Tested with class
    //TS_WARN("EMPTY TEST: GlobalFunctions_XMLMemberType_h_Test::test_operator_LT_LT()");
  }
};

class XMLMemberTypeTest : public CxxTest::TestSuite
{
public:
  void xtest_operator_NT_EQ()
  {
    //Tested with ==
    //TS_WARN("EMPTY TEST: XMLMemberTypeTest::test_operator_NT_EQ()");
  }

  /// UNIT_TEST:  Tests the == operator.
  /// ON_FAIL:    Indicates the == operator has failed.
  void test_operator_EQ_EQ()
  {
    // comment == test
    Comment C1(" Comment ");
    Comment matchC1("Comment");
    Comment noMatch("noMatchComment");
    TS_ASSERT((C1)==(matchC1));
    TS_ASSERT((C1)!=(noMatch));

    // char data equal test
    CharData Ch1(" Char Data ");
    CharData matchCh1("Char Data");
    CharData noMatchCh("noMatch Char Data");
    TS_ASSERT(Ch1==matchCh1);
    TS_ASSERT(Ch1!=noMatchCh);
  }

  /// UNIT_TEST:  Tests the trim() function.
  /// ON_FAIL:    Indicates the trim() function has failed.
  void test_trim()
  {
    std::string DesiredResult("Test");

    // Space in front
    std::string TestString(" Test");
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    //Space and tab
    TestString="\t Test";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    // some returns in front 
    TestString="\n\rTest";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    // Space in back
    TestString=("Test ");
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    //tab in back;
    TestString="Test\t ";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    // some returns in back 
    TestString="Test\n\r";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    // Space both
    TestString=(" Test ");
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    //tab both
    TestString=" \tTest\t ";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);

    // some returns both 
    TestString="\r\nTest\n\r";
    TS_ASSERT_DIFFERS(TestString,DesiredResult);
    TestString=XMLMemberType::trim(TestString);
    TS_ASSERT_EQUALS(TestString,DesiredResult);
  }

  void setUp()
  {
    _testElement = new Element(_stringElementName);
    _testElement->setValue(_stringElement);
    _testComment = new Comment(_stringComment);
    _testCharData= new CharData(_stringCharData);
    return ;
  }

  void tearDown()
  {
    delete _testElement;
    delete _testComment;
    delete _testCharData;
    return ;
  }

  void xtest_XMLMemberType()
  {
    // virtual class is never built
    //TS_WARN("EMPTY TEST: XMLMemberTypeTest::test_XMLMemberType()");
  }

  /// UNIT_TEST:  Tests the isComment() function.
  /// ON_FAIL:    Indicates the isComment() function has failed.
  void test_isComment()
  {
    TS_ASSERT(!_testElement->isComment());
    TS_ASSERT(_testComment->isComment());
    TS_ASSERT(!_testCharData->isComment());
  }

  /// UNIT_TEST:  Tests the isData() function.
  /// ON_FAIL:    Indicates the isData() function has failed.
  void test_isData()
  {
    TS_ASSERT(!_testElement->isData());
    TS_ASSERT(!_testComment->isData());
    TS_ASSERT(_testCharData->isData());
  }

  /// UNIT_TEST:  Tests the isElement() function.
  /// ON_FAIL:    Indicates the isElement() function has failed.
  void test_isElement()
  {
    TS_ASSERT(_testElement->isElement());
    TS_ASSERT(!_testComment->isElement());
    TS_ASSERT(!_testCharData->isElement());
  }

  /// UNIT_TEST:  Tests the << operator.
  /// ON_FAIL:    Indicates the << operator has failed.
  void test_operator_LT_LT()
  {
    std::stringstream ans;
    ans<<(*_testComment);
    TS_ASSERT((ans.str().length())>0);

    std::stringstream ans1;
    ans1<<(*_testCharData);
    TS_ASSERT((ans1.str().length())>0);

    std::stringstream ans2;
    ans2<<(*_testElement);
    TS_ASSERT((ans2.str().length())>0);
  }

  void xtest_print()
  {
    //tested in derived classes
    //TS_WARN("EMPTY TEST: XMLMemberTypeTest::test_print()");
  }

  void xtest_toStream()
  {
    //tested in derived classes
    //TS_WARN("EMPTY TEST: XMLMemberTypeTest::test_toStream()");
  }
private:
  static const std::string _stringElement;
  static const std::string _stringCharData;
  static const std::string _stringComment;
  static const std::string _stringElementName;
  Comment * _testComment;
  CharData * _testCharData;
  Element * _testElement;
};

const std::string XMLMemberTypeTest::_stringElementName="Element Name";
const std::string XMLMemberTypeTest::_stringElement="Element Data";
const std::string XMLMemberTypeTest::_stringCharData="Char Data";
const std::string XMLMemberTypeTest::_stringComment="Comment Data";

class XmlParserTest : public CxxTest::TestSuite
{
public:

  void xtest_XmlParser()
  {
    // constructor not needed as this class is never built
    //TS_WARN("EMPTY TEST: XmlParserTest::test_XmlParser()");
  }


  void xtest_getErrorCode()
  {
    // Not tested; this method just acts as a third party pass through from expat  
    // TS_WARN("EMPTY TEST: XmlParserTest::test_getErrorCode()");
  }

};

