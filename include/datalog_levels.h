/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_levels.h 1.11 2006/10/19 19:40:30Z MS10234 Exp MS10234 $
 * $Log: datalog_levels.h $
 * Revision 1.10  2006/10/07 19:29:42Z  cf10242
 * IT 59: add button press log level
 * Revision 1.9  2005/04/26 23:14:56Z  rm10919
 * Added cgui log levels.
 * Revision 1.8  2003/09/05 17:00:37Z  td07711
 * IT6140 - add datalog levels for hog_info and hog_error
 * Revision 1.7  2003/05/21 20:15:57Z  ms10234
 * Changed debug log level to off by default
 * Revision 1.6  2003/05/15 15:21:59Z  ms10234
 * added log level for state machine
 * Revision 1.5  2003/04/25 17:25:02Z  jl11312
 * - undefined LOG_DEF after use
 * Revision 1.4  2003/04/11 22:40:52Z  td07711
 * add log levels for checkself_info, checkself_error, spoofer_info, spoofer_error.
 * Revision 1.3  2003/04/11 15:27:11Z  jl11312
 * - added support for pre-initialized critical levels and handles
 * Revision 1.2  2003/03/27 16:27:44Z  jl11312
 * - allow disabling datalog levels by default
 * Revision 1.1  2003/03/27 15:48:00Z  jl11312
 * Initial revision
 */

#ifndef _DATALOG_LEVELS_INCLUDE
#define _DATALOG_LEVELS_INCLUDE

#include "datalog.h"

#ifdef DATALOG_LEVELS_INIT_TABLE
#define LOG_DEF
#else /* ifdef DATALOG_LEVELS_INIT_TABLE */
#define LOG_DEF extern
#endif /* ifdef DATALOG_LEVELS_INIT_TABLE */

#ifdef __cplusplus
//
// DataLog_Level class only supported in C++
//
LOG_DEF DataLog_Level	log_level_critical;
LOG_DEF DataLog_Level	log_level_message_system_timer_info;
LOG_DEF DataLog_Level	log_level_message_system_timer_error;
LOG_DEF DataLog_Level	log_level_router_info;
LOG_DEF DataLog_Level	log_level_router_error;
LOG_DEF DataLog_Level	log_level_gateway_info;
LOG_DEF DataLog_Level	log_level_gateway_error;
LOG_DEF DataLog_Level	log_level_message_spoof_info;
LOG_DEF DataLog_Level	log_level_message_spoof_error;
LOG_DEF DataLog_Level	log_level_message_system_info;
LOG_DEF DataLog_Level	log_level_message_system_error;

LOG_DEF DataLog_Level   log_level_config_data_info;
LOG_DEF DataLog_Level   log_level_config_data_error;

LOG_DEF DataLog_Level	log_level_datalog_info;
LOG_DEF DataLog_Level	log_level_datalog_error;

LOG_DEF DataLog_Level	log_level_vxworks_info;
LOG_DEF DataLog_Level	log_level_vxworks_error;

LOG_DEF DataLog_Level	log_level_cds_info;
LOG_DEF DataLog_Level	log_level_cds_error;
LOG_DEF DataLog_Level	log_level_cds_debug;
LOG_DEF DataLog_Level	log_level_cds_spoof_info;
LOG_DEF DataLog_Level	log_level_cds_spoof_error;
LOG_DEF DataLog_Level	log_level_socket_info;
LOG_DEF DataLog_Level	log_level_socket_error;
LOG_DEF DataLog_Level	log_level_checkself_info;
LOG_DEF DataLog_Level	log_level_checkself_error;
LOG_DEF DataLog_Level	log_level_state_machine_info;
LOG_DEF DataLog_Level	log_level_state_machine_debug;
LOG_DEF DataLog_Level	log_level_state_machine_error;
LOG_DEF DataLog_Level	log_level_spoofer_info;
LOG_DEF DataLog_Level	log_level_spoofer_error;
LOG_DEF DataLog_Level	log_level_hog_info;
LOG_DEF DataLog_Level	log_level_hog_error;

LOG_DEF DataLog_Level	log_level_cgui_info;
LOG_DEF DataLog_Level	log_level_cgui_button_press_info;
LOG_DEF DataLog_Level	log_level_cgui_error;


#endif /* ifdef __cplusplus */

LOG_DEF DataLog_Handle  log_handle_critical;
LOG_DEF DataLog_Handle	log_handle_message_system_timer_info;
LOG_DEF DataLog_Handle	log_handle_message_system_timer_error;
LOG_DEF DataLog_Handle	log_handle_router_info;
LOG_DEF DataLog_Handle	log_handle_router_error;
LOG_DEF DataLog_Handle	log_handle_gateway_info;
LOG_DEF DataLog_Handle	log_handle_gateway_error;
LOG_DEF DataLog_Handle	log_handle_message_spoof_info;
LOG_DEF DataLog_Handle	log_handle_message_spoof_error;
LOG_DEF DataLog_Handle	log_handle_message_system_info;
LOG_DEF DataLog_Handle	log_handle_message_system_error;

LOG_DEF DataLog_Handle  log_handle_config_data_info;
LOG_DEF DataLog_Handle  log_handle_config_data_error;

LOG_DEF DataLog_Handle	log_handle_datalog_info;
LOG_DEF DataLog_Handle	log_handle_datalog_error;

LOG_DEF DataLog_Handle	log_handle_vxworks_info;
LOG_DEF DataLog_Handle	log_handle_vxworks_error;

LOG_DEF DataLog_Handle	log_handle_cds_info;
LOG_DEF DataLog_Handle	log_handle_cds_error;
LOG_DEF DataLog_Handle	log_handle_cds_debug;
LOG_DEF DataLog_Handle	log_handle_cds_spoof_info;
LOG_DEF DataLog_Handle	log_handle_cds_spoof_error;
LOG_DEF DataLog_Handle	log_handle_socket_info;
LOG_DEF DataLog_Handle	log_handle_socket_error;
LOG_DEF DataLog_Handle	log_handle_checkself_info;
LOG_DEF DataLog_Handle	log_handle_checkself_error;
LOG_DEF DataLog_Handle	log_handle_state_machine_info;
LOG_DEF DataLog_Handle	log_handle_state_machine_debug;
LOG_DEF DataLog_Handle	log_handle_state_machine_error;
LOG_DEF DataLog_Handle	log_handle_spoofer_info;
LOG_DEF DataLog_Handle	log_handle_spoofer_error;
LOG_DEF DataLog_Handle	log_handle_hog_info;
LOG_DEF DataLog_Handle	log_handle_hog_error;

LOG_DEF DataLog_Handle	log_handle_cgui_info;
LOG_DEF DataLog_Handle	log_handle_cgui_button_press_info;
LOG_DEF DataLog_Handle	log_handle_cgui_error;


#ifdef DATALOG_LEVELS_INIT_TABLE

struct DataLog_InitTable
{
	DataLog_Level * level;
	DataLog_Handle * handle;
	const char * name;
	bool enabled;
};

static DataLog_InitTable	initTable[] =
{
	{ &log_level_message_system_timer_info, &log_handle_message_system_timer_info, "message_system_timer_info", true },
	{ &log_level_message_system_timer_error, &log_handle_message_system_timer_error, "message_system_timer_error", true },
	{ &log_level_router_info, &log_handle_router_info, "router_info", true },
	{ &log_level_router_error, &log_handle_router_error, "router_error", true },
	{ &log_level_gateway_info, &log_handle_gateway_info, "gateway_info", true },
	{ &log_level_gateway_error, &log_handle_gateway_error, "gateway_error", true },
	{ &log_level_message_spoof_info, &log_handle_message_spoof_info, "message_spoof_info", true },
	{ &log_level_message_spoof_error, &log_handle_message_spoof_error, "message_spoof_error", true },
	{ &log_level_message_system_info, &log_handle_message_system_info, "message_system_info", true },
	{ &log_level_message_system_error, &log_handle_message_system_error, "message_system_error", true },

   { &log_level_config_data_info, &log_handle_config_data_info, "config_data_info", true },
   { &log_level_config_data_error, &log_handle_config_data_error, "config_data_error", true },

	{ &log_level_datalog_info, &log_handle_datalog_info, "datalog_info", true },
	{ &log_level_datalog_error, &log_handle_datalog_error, "datalog_error", true },

	{ &log_level_vxworks_info, &log_handle_vxworks_info, "vxworks_info", true },
	{ &log_level_vxworks_error, &log_handle_vxworks_error, "vxworks_error", true },

	{ &log_level_cds_info, &log_handle_cds_info, "cds_info", true },
	{ &log_level_cds_error, &log_handle_cds_error, "cds_error", true },
	{ &log_level_cds_debug, &log_handle_cds_debug, "cds_debug", false },
	{ &log_level_cds_spoof_info, &log_handle_cds_spoof_info, "cds_spoof_info", true },
	{ &log_level_cds_spoof_error, &log_handle_cds_spoof_error, "cds_spoof_error", true },
	{ &log_level_socket_info, &log_handle_socket_info, "socket_info", true },
	{ &log_level_socket_error, &log_handle_socket_error, "socket_error", true },
	{ &log_level_checkself_error, &log_handle_checkself_error, "checkself_error", true },
	{ &log_level_checkself_info, &log_handle_checkself_info, "checkself_info", true },
	{ &log_level_state_machine_error, &log_handle_state_machine_error, "state_machine_error", true },
	{ &log_level_state_machine_debug, &log_handle_state_machine_debug, "state_machine_debug", false },
	{ &log_level_state_machine_info, &log_handle_state_machine_info, "state_machine_info", true },
	{ &log_level_spoofer_error, &log_handle_spoofer_error, "spoofer_error", true },
    { &log_level_spoofer_info, &log_handle_spoofer_info, "spoofer_info", true },
    
    { &log_level_cgui_error, &log_handle_cgui_error, "cgui_error", true },
	{ &log_level_cgui_info, &log_handle_cgui_info, "cgui_info", true },
	{ &log_level_cgui_button_press_info, &log_handle_cgui_button_press_info, "cgui_button_press_info", true },
};

#endif /* ifdef DATALOG_LEVELS_INIT_TABLE */

#undef LOG_DEF
#endif /* ifndef _DATALOG_LEVELS_INCLUDE */

