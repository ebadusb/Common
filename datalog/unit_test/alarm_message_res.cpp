#include <vxWorks.h>
#include "alarm_message_res.h"

const AlarmMessageRes::DisplayingAlarm AlarmMessageRes::displayingAlarm = { "DISPLAYING_ALARM", "LOG_NAME", "EVENT_ID", "NODE_ID", "MODULE_LEVEL", "ALARM_ID" };
const AlarmMessageRes::AlarmConstraint AlarmMessageRes::alarmConstraint = { "ALARM_CONSTRAINT", "CONSTRAINT_NAME", "REFERENCE_COUNT" };
const AlarmMessageRes::RaisedAlarm AlarmMessageRes::raisedAlarm = { "RAISED_ALARM", "ALARM_NAME", "NODE_ID", "MODULE_LEVEL", "ALARM_ID", "ADDED_CONSTRAINT" };
const AlarmMessageRes::OperatorAlarmResponse AlarmMessageRes::operatorAlarmResponse = { "OPERATOR_ALARM_REPSONSE", "ALARM_NAME", "BUTTON_ID", "REPONSE_STATE", "EVENT_ID", "NODE_ID", "MODULE_LEVEL", "ALARM_ID" };
const AlarmMessageRes::AlarmRemovedList AlarmMessageRes::alarmRemovedList = { "ALARM_REMOVED_LIST", "ALARM_NAME", "EVENT_ID", "NODE_ID", "MODULE_LEVEL", "ALARM_ID" };
const AlarmMessageRes::AlarmRemovedDisplay AlarmMessageRes::alarmRemovedDisplay = { "ALARM_REMOVED_DISPLAY", "ALARM_NAME", "EVENT_ID", "NODE_ID", "MODULE_LEVEL", "ALARM_ID" };
