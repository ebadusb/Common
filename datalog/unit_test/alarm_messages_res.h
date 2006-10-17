#ifndef _ALARM_MESSAGES_RES_
#define _ALARM_MESSAGES_RES_

namespace AlarmMessagesReserved
{
	struct DisplayingAlarm
	{
		const char * name;
		const char * logName;
		const char * eventID;
		const char * nodeID;
		const char * moduleLevel;
		const char * alarmID;
	};

	extern const DisplayingAlarm displayingAlarm;

	struct AlarmHeaderText
	{
		const char * name;
		const char * textID;
		const char * textString;
	};

	extern const AlarmHeaderText alarmHeaderText;

	struct AlarmBodyText
	{
		const char * name;
		const char * textID;
		const char * textString;
	};

	extern const AlarmBodyText alarmBodyText;

	struct AlarmConstraint
	{
		const char * name;
		const char * constraintName;
		const char * referenceCount;
	};

	extern const AlarmConstraint alarmConstraint;

}

#endif _ALARM_MESSAGES_RES_
