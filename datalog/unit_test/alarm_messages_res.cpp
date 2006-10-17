#include <vxWorks.h>
#include "alarm_messages_res.h"

const AlarmMessagesReserved::DisplayingAlarm AlarmMessagesReserved::displayingAlarm = { "DISPLAYING_ALARM", "logName", "eventID", "nodeID", "moduleLevel", "alarmID" };
const AlarmMessagesReserved::AlarmHeaderText AlarmMessagesReserved::alarmHeaderText = { "ALARM_HEADER_TEXT", "textID", "textString" };
const AlarmMessagesReserved::AlarmBodyText AlarmMessagesReserved::alarmBodyText = { "ALARM_BODY_TEXT", "textID", "textString" };
const AlarmMessagesReserved::AlarmConstraint AlarmMessagesReserved::alarmConstraint = { "ALARM_CONSTRAINT", "constraintName", "referenceCount" };
