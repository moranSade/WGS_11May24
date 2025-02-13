/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

File: octvc1_obm_evt_swap.h	

Copyright (c) 2018 Octasic Inc. All rights reserved.	

Description:	

This source code is Octasic Confidential. Use of and access to this code
is covered by the Octasic Device Enabling Software License Agreement.
Acknowledgement of the Octasic Device Enabling Software License was
required for access to this code. A copy was also provided with the release.	

Release: Octasic Board management software (OBM) 2.5.0-B393 (07/02/2018)	

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/	
#ifndef __OCTVC1_OBM_EVT_SWAP_H__
#define __OCTVC1_OBM_EVT_SWAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************  INCLUDE FILES  *******************************/
#include "../octvc1_swap_hdrs.h"
#include "octvc1_obm_evt.h"

/*********************************  MACROS  **********************************/


#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT * pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulHighTempWarningFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulHighTempWarningFlag); \
	((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulLowTempWarningFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->ulLowTempWarningFlag); \
	((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->lDspSensorTemp = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT)->lDspSensorTemp); \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_TEMPERATURE_ALARM_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT * pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT *)pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT *)pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT *)pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)->ulBoardIndex); \
	{ mOCTVC1_OBM_INPUT_REFERENCE_CLOCK_ENUM_SWAP( &((tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT *)pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)->ulInputReferenceClock ); }  \
	{ mOCTVC1_OBM_INPUT_REFERENCE_CLOCK_ENUM_SWAP( &((tOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT *)pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT)->ulInputReferenceClockRequested ); }  \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_REF_CLOCK_OVERRIDDEN_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT * pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)->ulBoardIndex); \
	{ mOCTVC1_OBM_RF_ANTENNA_CONNECTOR_ID_SWAP( &((tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)->AntennaConnectorId ); }  \
	{ mOCTVC1_OBM_RF_ALARM_INFO_SWAP( &((tOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT)->AlarmInfo ); }  \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_RF_ALARM_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT * pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->ulDspId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->ulDspId); \
	{ mOCTVC1_OBM_DSP_BOOT_PROGRESS_ENUM_SWAP( &((tOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT *)pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT)->ulBootProgress ); }  \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_DSP_BOOT_PROGRESS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT * pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulGpsId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulGpsId); \
	{ mOCTVC1_OBM_GPS_FIX_ENUM_SWAP( &((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulPreviousFix ); }  \
	{ mOCTVC1_OBM_GPS_FIX_ENUM_SWAP( &((tOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT)->ulNewFix ); }  \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_GPS_STATE_CHANGE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT * pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulPowerSwitchId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulPowerSwitchId); \
	((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulOffMs = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulOffMs); \
	((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulOnMs = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT)->ulOnMs); \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_POWER_SWITCH_CHANGE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT * pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryId); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryChargingFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryChargingFlag); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryFaultFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryFaultFlag); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryTempFaultFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryTempFaultFlag); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryLowLevelWarnFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryLowLevelWarnFlag); \
	((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryPowerLevel = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT *)pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT)->ulBatteryPowerLevel); \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_BATTERY_CHANGE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT_SWAP( _f_pParms ){ 	tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT * pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->Header ); }  \
	((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulBoardIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulBoardIndex); \
	((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulFanIndex = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulFanIndex); \
	((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulFanErrorFlag = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT *)pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT)->ulFanErrorFlag); \
}
#else
#define mOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT_SWAP( pOCTVC1_OBM_MSG_BOARD_FAN_ALARM_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#ifdef __cplusplus
}

#endif

#endif /* __OCTVC1_OBM_EVT_SWAP_H__ */
