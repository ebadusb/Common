#ifndef _ALARM_MESSAGE_RES_
#define _ALARM_MESSAGE_RES_

namespace AlarmMessageRes
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

	struct AlarmConstraint
	{
		const char * name;
		const char * constraintName;
		const char * referenceCount;
	};

	extern const AlarmConstraint alarmConstraint;

	struct RaisedAlarm
	{
		const char * name;
		const char * alarmName;
		const char * nodeID;
		const char * moduleLevel;
		const char * alarmID;
		const char * addedConstraint;
	};

	extern const RaisedAlarm raisedAlarm;

	struct OperatorAlarmResponse
	{
		const char * name;
		const char * alarmName;
		const char * buttonID;
		const char * responseState;
		const char * eventID;
		const char * nodeID;
		const char * moduleLevel;
		const char * alarmID;
	};

	extern const OperatorAlarmResponse operatorAlarmResponse;

	struct AlarmRemovedList
	{
		const char * name;
		const char * alarmName;
		const char * eventID;
		const char * nodeID;
		const char * moduleLevel;
		const char * alarmID;
	};

	extern const AlarmRemovedList alarmRemovedList;

	struct AlarmRemovedDisplay
	{
		const char * name;
		const char * alarmName;
		const char * eventID;
		const char * nodeID;
		const char * moduleLevel;
		const char * alarmID;
	};

	extern const AlarmRemovedDisplay alarmRemovedDisplay;

}

#endif _ALARM_MESSAGE_RES_
