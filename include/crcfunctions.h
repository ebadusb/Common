//## begin module%35EC152600BC.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%35EC152600BC.cm

//## begin module%35EC152600BC.cp preserve=no
//## end module%35EC152600BC.cp

//## Module: CRCFunctions%35EC152600BC; Pseudo Package specification
//## Subsystem: <Top Level>
//## Source file: D:\Program Files\Rational\Rational Rose 98 Enterprise Edition\c++\source\CRCFunctions.h

#ifndef CRCFunctions_h
#define CRCFunctions_h 1

//## begin module%35EC152600BC.additionalIncludes preserve=no
//## end module%35EC152600BC.additionalIncludes

//## begin module%35EC152600BC.includes preserve=yes
//## end module%35EC152600BC.includes

//## begin module%35EC152600BC.additionalDeclarations preserve=yes
//## end module%35EC152600BC.additionalDeclarations


//## Class: CRCFunctions%35EC152600BC
//## Category: <Top Level>
//## Subsystem: <Top Level>
//## Persistence: Transient
//## Cardinality/Multiplicity: n

class CRCFunctions 
{
  //## begin CRCFunctions%35EC152600BC.initialDeclarations preserve=yes
  //## end CRCFunctions%35EC152600BC.initialDeclarations

  public:
    //## Constructors (generated)
      CRCFunctions();

    //## Destructor (generated)
      ~CRCFunctions();

    //## Other Operations (specified)
      //## Operation: generateCRC%904748907
      // generateCRC: method takes three inputs crc, data_buffer, buffer_length.  The crc is used
      // as both an input and an output.  It will be used as the starting crc on entry and will have
      // the new crc on exit of the method if it does not fail.
      int generateCRC (char* crc, char *data_buffer, long buffer_length);

      //## Operation: generateFileCRC%904748908
      int generateFileCRC (char* crc, const char* filename);

      //## Operation: generateCRC%904748909
      int generateCRC (char* crc, iostream* the_stream);

      //## Operation: validateFileCRC%904748910
      int validateFileCRC (const char *crc, const char* filename);

      //## Operation: validateCRC%904748911
      int validateCRC (const char* crc, iostream* the_stream);

      //## Operation: moveValidFile%904748912
      int moveValidFile (const char* crc, const char* source, const char* dest);

    // Additional Public Declarations
      //## begin CRCFunctions%35EC152600BC.public preserve=yes
      //## end CRCFunctions%35EC152600BC.public

  protected:
    // Additional Protected Declarations
      //## begin CRCFunctions%35EC152600BC.protected preserve=yes
      //## end CRCFunctions%35EC152600BC.protected

  private:
    // Additional Private Declarations
      //## begin CRCFunctions%35EC152600BC.private preserve=yes
      //## end CRCFunctions%35EC152600BC.private

  private:  //## implementation
    // Additional Implementation Declarations
      //## begin CRCFunctions%35EC152600BC.implementation preserve=yes
      //## end CRCFunctions%35EC152600BC.implementation

};

//## begin CRCFunctions%35EC152600BC.postscript preserve=yes
//## end CRCFunctions%35EC152600BC.postscript

// Class CRCFunctions 

//## begin module%35EC152600BC.epilog preserve=yes
//## end module%35EC152600BC.epilog


#endif
