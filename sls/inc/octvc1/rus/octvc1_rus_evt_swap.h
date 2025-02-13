/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

File: octvc1_rus_evt_swap.h	

Copyright (c) 2018 Octasic Technologies Private Limited. All rights reserved.	

Description:	

This source code is Octasic Technologies Confidential. Use of and access to this code
is covered by the Octasic Technologies Device Enabling Software License Agreement.
Acknowledgement of the Octasic Technologies Device Enabling Software License was
required for access to this code. A copy was also provided with the release.	

Release: OCTSDR Software Development Kit OCTSDR-SR_RUS-02.07.00-B621 (2018/04/04)	

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/	
#ifndef __OCTVC1_RUS_EVT_SWAP_H__
#define __OCTVC1_RUS_EVT_SWAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************  INCLUDE FILES  *******************************/
#include "../octvc1_swap_hdrs.h"
#include "octvc1_rus_evt.h"

/*********************************  MACROS  **********************************/


#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT * pOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT)->Header ); }  \
	{ mOCTVC1_RUS_WNM_SEARCH_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT * pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT)->Header ); }  \
	((tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT)->ulCellId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT)->ulCellId); \
	{ mOCTVC1_RUS_WNM_GSM_CELL_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT * pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT)->Header ); }  \
	((tOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT)->ulCellId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT)->ulCellId); \
	{ mOCTVC1_RUS_WNM_CDMA2000_CELL_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_CDMA2000_CELL_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT * pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT)->Header ); }  \
	((tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT)->ulCellId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT)->ulCellId); \
	{ mOCTVC1_RUS_WNM_UMTS_CELL_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT * pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT)->Header ); }  \
	((tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT)->ulCellId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT)->ulCellId); \
	{ mOCTVC1_RUS_WNM_LTE_CELL_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT * pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT)->Header ); }  \
	((tOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT)->ulCellId = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT)->ulCellId); \
	{ mOCTVC1_RUS_WNM_NR_CELL_STATS_SWAP( &((tOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT *)pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT)->Stats ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_NR_CELL_STATS_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT_SWAP( _f_pParms ){ 	tOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT * pOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT *)pOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT)->Header ); }  \
}
#else
#define mOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT_SWAP( pOCTVC1_RUS_MSG_WNM_MCC_LIC_ERR_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#ifdef __cplusplus
}

#endif

#endif /* __OCTVC1_RUS_EVT_SWAP_H__ */
