/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

File: octvc1_lte_evt_swap.h	

Copyright (c) 2019 Octasic Inc. All rights reserved.	

Description:	

This source code is Octasic Confidential. Use of and access to this code
is covered by the Octasic Device Enabling Software License Agreement.
Acknowledgement of the Octasic Device Enabling Software License was
required for access to this code. A copy was also provided with the release.	

Release: OCTSDR Software Development Kit OCTSDR_LTE_APP-04.03.00-B3607 (2019/04/16)	

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/	
#ifndef __OCTVC1_LTE_EVT_SWAP_H__
#define __OCTVC1_LTE_EVT_SWAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************  INCLUDE FILES  *******************************/
#include "../octvc1_swap_hdrs.h"
#include "octvc1_lte_evt.h"

/*********************************  MACROS  **********************************/


#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT * pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->Header ); }  \
	((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->hEnodeb = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->hEnodeb); \
	((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->ulUserId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->ulUserId); \
	{ mOCTVC1_LTE_CELL_STATE_ENUM_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->ulOldState ); }  \
	{ mOCTVC1_LTE_CELL_STATE_ENUM_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT *)pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT)->ulNewState ); }  \
}
#else
#define mOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT_SWAP( pOCTVC1_LTE_MSG_ENODEB_CELL_STATE_UPDATE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT_SWAP( _f_pParms ){ 	tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT * pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->Header ); }  \
	((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->hEnodeb = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->hEnodeb); \
	((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->ulUserId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->ulUserId); \
	{ mOCTVC1_LTE_BASS_EVENT_CELL_INFO_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->LastCellInfo ); }  \
	{ mOCTVC1_LTE_BASS_EVENT_CELL_INFO_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT)->CurrentCellInfo ); }  \
}
#else
#define mOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT_SWAP( pOCTVC1_LTE_MSG_ENODEB_BASS_NEW_CELL_FOUND_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT * pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->Header ); }  \
	((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->hEnodeb = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->hEnodeb); \
	((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->ulUserId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->ulUserId); \
	{ mOCTVC1_LTE_BASS_EVENT_CELL_INFO_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->LastCellInfo ); }  \
	{ mOCTVC1_LTE_BASS_EVENT_CELL_INFO_SWAP( &((tOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT *)pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT)->CurrentCellInfo ); }  \
}
#else
#define mOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT_SWAP( pOCTVC1_LTE_MSG_ENODEB_BASS_CELL_SYNC_CHANGE_SOURCE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#ifdef __cplusplus
}

#endif

#endif /* __OCTVC1_LTE_EVT_SWAP_H__ */
