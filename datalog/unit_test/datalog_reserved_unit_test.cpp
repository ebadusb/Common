/*
 * $Header: Z:/vxWorks/Common/datalog/unit_test/rcs/datalog_reserved_unit_test.cpp 1.1 2006/11/30 17:52:02Z jmedusb Exp jmedusb $ 
 * Secondary Unit Test for passing the __FILE__, and __LINE__ defines
 * To the DataLogReservedStream Calls, fixes a bug where only the file and line
 * of the class where used, instead of the calling file
 *
 * $Log: datalog_reserved_unit_test.cpp $
 */

#include <vxworks.h>
#include "datalog_reserved_stream.h"
#include "alarm_message_res.h"
#include "alarm_header_text_message_res.h"
#include <math.h>
#include <time.h>

using namespace AlarmMessageRes;
using namespace AlarmHeaderTextMessageRes;

typedef enum {
	TAG_CCPTR = 0,	//0
	TAG_CSCPTR,		//1
	TAG_CUCPTR,		//2
	TAG_STRING,		//3
	TAG_USHORT,		//4
	TAG_SHORT,		//5
	TAG_UINT,		//6
	TAG_INT,		//7
	TAG_ULONG,		//8
	TAG_LONG,		//9
	TAG_BOOL,		//10
	TAG_SCHAR,		//11
	TAG_UCHAR,		//12
	TAG_CHAR,		//13
	TAG_FLOAT,		//14
	TAG_NFLOAT,		//15
	TAG_DOUBLE,		//16
	TAG_NDOUBLE,	//17
	TAG_MAX = TAG_NDOUBLE + 1
} TAGS;

const char * STRINGS[] = {
	"TAG_CCPTR",
	"TAG_CSCPTR",
	"TAG_CUCPTR",
	"TAG_STRING",
	"TAG_USHORT",
	"TAG_SHORT",
	"TAG_UINT",
	"TAG_INT",
	"TAG_ULONG",
	"TAG_LONG",
	"TAG_BOOL",
	"TAG_SCHAR",
	"TAG_UCHAR",
	"TAG_CHAR",
	"TAG_FLOAT",
	"TAG_NFLOAT",
	"TAG_DOUBLE",
	"TAG_NDOUBLE",
	"RESERVED"
};

const char * FILE_NAME = "host:c:/temp/1A00001_2006MMDD_000_000001.dlog";
ULONG FILE_SIZE = 4096;
ULONG CRIT_SIZE = (FILE_SIZE / 4);

bool initDataLog(void);
bool initLogLevel(DataLog_Level & logLevel);
bool testReservedMessageName(void);
bool testAllReserveTypes(void);
bool testRandomType(void);
bool testGeneratedHeaderTwo(void);
bool testHex(void);
bool testDec(void);
bool testPrecision(void);

extern "C" void startTests(void)
{
	assert(initDataLog());
    assert(testReservedMessageName());	//Test the constructor
	assert(testAllReserveTypes());		//Test all types
	assert(testRandomType());			//Test random types
	assert(testGeneratedHeaderTwo());	//Test auto-generated header with .cpp file
	assert(testHex());					//Test Hex Manipulator
	assert(testDec());					//Test Dec Manipulator
	assert(testPrecision());			//Test the Precision Manipulator
	//Everything passes if no asserts failed
    printf("\nDataLog Reserved Tests Complete : PASSED.\n");
}

bool initDataLog(void)
{
	DataLog_Result result;
    result = datalog_Init(FILE_SIZE, CRIT_SIZE, FILE_NAME, "platform", "node", "info");
	return (result == DataLog_OK);
}

bool testReservedMessageName(void)
{
	DataLog_Level reservedLevel("reservedLevel");
	DataLogReserved("MSG_TEST_CONSTRUCTOR", reservedLevel) << endmsg;
	printf("testReservedMessageName Complete : PASSED.\n");
    return true;
}

bool testAllReserveTypes(void)
{
	DataLog_Level testAllReserveTypes("testAllReserveTypes");
    
    //Simply call the MSG_testAllReserveTypes for each type
    const char * testconstCCPTR 		= "THIS IS A const char *";
    const signed char * testCSCPTR 		= (const signed char *)(&"THIS IS A CSCPTR");
	const unsigned char * testCUCPTR 	= (const unsigned char *)(&"THIS IS A CUCPTR");
	string testSTRING 			= "THIS IS A STRING";
	unsigned short testUSHORT 	= 0xFF;   			//255 hex
	short testSHORT 			= ~0xFF;   			//-255 hex
	unsigned int testUINT 		= 0xFFFF;   		//65535 hex
	int testINT 				= ~0xFFFF;   		//-65535 hex
	unsigned long testULONG 	= 0xFFFF;   		//4294967295 hex
	long testLONG 				= ~0xFFFF;  		//-4294967295 hex
	bool testBOOL				= true;
	signed char testSCHAR 		= rand() % 95 + 33;	//Ascii Character , 33-127
	unsigned char testUCHAR 	= rand() % 95 + 33; //Ascii Character , 33-127
	char testCHAR 				= rand() % 95 + 33; //Ascii Character , 33-127
	float testFLOAT 			= 12345.67890;
	float testNFLOAT 			= -12345.67890;
	double testDOUBLE 			= 1234567890;
	double testNDOUBLE 			= -1234567890;

    DataLogReserved("MSG_testAllReserveTypes", testAllReserveTypes)
		<< taggedItem(STRINGS[TAG_CCPTR], testconstCCPTR)
		<< taggedItem(STRINGS[TAG_CSCPTR], testCSCPTR) 
		<< taggedItem(STRINGS[TAG_CUCPTR], testCUCPTR) 
		<< taggedItem(STRINGS[TAG_STRING], testSTRING)
		<< taggedItem(STRINGS[TAG_USHORT], testUSHORT) 
		<< taggedItem(STRINGS[TAG_SHORT], testSHORT)
		<< taggedItem(STRINGS[TAG_UINT], testUINT) 
		<< taggedItem(STRINGS[TAG_INT], testINT)
		<< taggedItem(STRINGS[TAG_ULONG], testULONG) 
		<< taggedItem(STRINGS[TAG_LONG], testLONG)
		<< taggedItem(STRINGS[TAG_BOOL], testBOOL)
		<< taggedItem(STRINGS[TAG_SCHAR], testSCHAR)
		<< taggedItem(STRINGS[TAG_UCHAR], testUCHAR) 
		<< taggedItem(STRINGS[TAG_CHAR], testCHAR)
		<< taggedItem(STRINGS[TAG_FLOAT], testFLOAT) 
		<< taggedItem(STRINGS[TAG_NFLOAT], testNFLOAT)
		<< taggedItem(STRINGS[TAG_DOUBLE], testDOUBLE) 
		<< taggedItem(STRINGS[TAG_NDOUBLE], testNDOUBLE)
		<< endmsg;

	printf("testAllReserveTypes Complete : PASSED.\n");
	return true;
}

bool testRandomType(void)
{
	DataLog_Level testRandomType("testRandomType");
	DataLogRes stream("MSG_testRandomType", testRandomType, __FILE__, __LINE__);

	//Initialize the random generator with the current clock ticks, atleast 1
	srand ( time(NULL) );
	unsigned int random = (rand() % 100 + 1);
	stream << taggedItem("NUM_RANDOM_TAGS", random);
	int i = 0;

    for(i = 0; i < random; i++)
    {
        TAGS selection = (TAGS)(rand() % TAG_MAX);
		const char * TAG = STRINGS[selection];

        switch((int)selection)
        {
			case 0: 
			{
				const char * value = "THIS IS A const char *";
				stream << taggedItem(TAG, value);
				break;
			}
			case 1:
			{
				const signed char * value = (const signed char *)"THIS IS A CSCPTR";
				stream << taggedItem(TAG, value);
				break;
			}
			case 2:
			{
				const unsigned char * value = (const unsigned char *)"THIS IS A CUCPTR";
				stream << taggedItem(TAG, value);
				break;
			}
			case 3:
			{
				string value = "THIS IS A STRING";
				stream << taggedItem(TAG, value);
                break;
			}
			case 4:
			{
				unsigned short value = 0xFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 5:
			{
				short value = ~0xFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 6:
			{
				unsigned int value = 0xFFFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 7:
			{
				int value = 0xFFFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 8:
			{
				unsigned long value = 0xFFFFFFFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 9:
			{
				long value = ~0xFFFFFFFF;
				stream << taggedItem(TAG, value);
                break;
			}
			case 10:	//Changes depending on the time, odd or even
			{
				bool value = true & (bool)(time(NULL) % 2);
				stream << taggedItem(TAG, value);
                break;
			}
			case 11:
			{
				signed char value = rand() % 94 + 33;
				stream << taggedItem(TAG, value);
                break;
			}
			case 12:
			{
				unsigned char value = rand() % 94 + 33;
				stream << taggedItem(TAG, value);
                break;
			}

			case 13:
			{
				char value = rand() % 94 + 33;
				stream << taggedItem(TAG, value);
                break;
			}
			case 14:
			{
				float value = 12345.67890;
				stream << taggedItem(TAG, value);
                break;
			}
			case 15:
			{
				float value = -12345.67890;
				stream << taggedItem(TAG, value);
                break;
			}
			case 16:
			{
				double value = 1234567890.1234567890;
				stream << taggedItem(TAG, value);
                break;
			}
			case 17:
			{
				double value = -1234567890.1234567890;
				stream << taggedItem(TAG, value);
                break;
			}
			default:
				break;
        }
    }

	stream << taggedItem("I (at end of function) = :", i) << endmsg;

	printf("testRandomType Complete : PASSED.\n");
    return true;
}

bool testGeneratedHeaderTwo(void)
{
	DataLog_Level testGeneratedHeaderTwo("testGeneratedHeaderTwo");
	//Test First Message Structure
	DataLogReserved(displayingAlarm.name, testGeneratedHeaderTwo)
		<< taggedItem(displayingAlarm.logName, "AlarmMessagesReserved::displayingAlarm.logName")
		<< taggedItem(displayingAlarm.eventID, "AlarmMessagesReserved::displayingAlarm.eventID")
		<< taggedItem(displayingAlarm.nodeID, "AlarmMessagesReserved::displayingAlarm.nodeID")
		<< taggedItem(displayingAlarm.moduleLevel, "AlarmMessagesReserved::displayingAlarm.moduleLevel")
		<< taggedItem(displayingAlarm.alarmID, "AlarmMessagesReserved::displayingAlarm.alarmID")
		<< endmsg;
	//Test Second Message Structure
	DataLogReserved(alarmHeaderText.name, testGeneratedHeaderTwo)
		<< taggedItem(alarmHeaderText.textID, "AlarmMessagesReserved::alarmHeaderText.textID")
		<< taggedItem(alarmHeaderText.textString, "AlarmMessagesReserved::alarmHeaderText.textString")
		<< endmsg;

	printf("testGeneratedHeaderTwo Complete : PASSED.\n");
	return true;
}

bool testHex(void)
{
	DataLog_Level testHex("HEX_LOG_LEVEL");
	DataLogReserved("HEX_FLAGS_CHECK", testHex)
		<< taggedItem("HEX_TEST_START", "Before Hex Flags.")
		<< taggedItem("TEST HEX VALUE 0", 12345678)
		<< taggedItem("TEST HEX VALUE 1", -12345678)
		<< hex
		<< taggedItem("HEX_TEST_END", "After Hex Flags.")
		<< taggedItem("TEST HEX VALUE 0", 12345678)
		<< taggedItem("TEST HEX VALUE 1", -12345678)
		<< endmsg;

	printf("testHex Complete : PASSED.\n");
	return true;
}

bool testDec(void)
{
	DataLog_Level testDec("DEC_LOG_LEVEL");
	DataLogReserved("DEC_FLAGS_CHECK", testDec)
		<< taggedItem("DEC_TEST_START", "Before Decimal Flags.")
		<< taggedItem("TEST DEC VALUE 0", 12345678)
		<< taggedItem("TEST DEC VALUE 1", -12345678)
		<< dec
		<< taggedItem("DEC_TEST_END", "After Decimal Flags.")
		<< taggedItem("TEST DEC VALUE 0", 12345678)
		<< taggedItem("TEST DEC VALUE 1", -12345678)
		<< endmsg;

	printf("testDec Complete : PASSED.\n");
	return true;
}

bool testPrecision(void)
{
	DataLog_Level testPrecision("PRECISION_LOG_LEVEL");
	DataLogReserved("PRECISION_CHECK", testPrecision)
		<< taggedItem("DEC_TEST_START", "Before Decimal Flags.")
		<< taggedItem("<--------SPACER------->", "<...................>")
		<< taggedItem("BEFORE TEST PRECISION 4", 4.12345)
		<< setPrecision(4)
		<< taggedItem("AFTER  TEST PRECISION 4", 4.12345)
		<< taggedItem("BEFORE TEST PRECISION 2", 2.12345)
		<< setPrecision(2)
		<< taggedItem("AFTER  TEST PRECISION 2", 2.12345)
		<< taggedItem("BEFORE TEST PRECISION 1", 1.12345)
		<< setPrecision(1)
		<< taggedItem("AFTER  TEST PRECISION 1", 1.12345)
		<< taggedItem("BEFORE TEST PRECISION 0", 0.12345)
		<< setPrecision(0)
		<< taggedItem("AFTER TEST PRECISION 0", 0.12345)
		<< endmsg;

	printf("testPrecision Complete : PASSED.\n");
	return true;
}

