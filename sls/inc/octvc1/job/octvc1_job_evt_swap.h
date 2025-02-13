/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

File: octvc1_job_evt_swap.h	

Copyright (c) 2019 Octasic Technologies Private Limited. All rights reserved.	

Description:	

This source code is Octasic Technologies Confidential. Use of and access to this code
is covered by the Octasic Technologies Device Enabling Software License Agreement.
Acknowledgement of the Octasic Technologies Device Enabling Software License was
required for access to this code. A copy was also provided with the release.	

Release: OCTSDR Software Development Kit OCTSDR_GSM-04.02.00-B3135 (2019/03/05)	

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/	
#ifndef __OCTVC1_JOB_EVT_SWAP_H__
#define __OCTVC1_JOB_EVT_SWAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************  INCLUDE FILES  *******************************/
#include "../octvc1_swap_hdrs.h"
#include "octvc1_job_evt.h"

/*********************************  MACROS  **********************************/


#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT_SWAP( _f_pParms ){ 	tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT * pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->Header ); }  \
	((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->hExecutor = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->hExecutor); \
	((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->hActiveCase = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->hActiveCase); \
	{ mOCTVC1_JOB_RUNNER_CASE_STATE_ENUM_SWAP( &((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->ulOldState ); }  \
	{ mOCTVC1_JOB_RUNNER_CASE_STATE_ENUM_SWAP( &((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT)->ulNewState ); }  \
}
#else
#define mOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT_SWAP( pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_STATE_CHANGE_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#if defined( _OCT_ENDIAN_TYPE_LE_ )
#define mOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT_SWAP( _f_pParms ){ 	tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT * pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT = (_f_pParms); SWAP_UNUSED(pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)\
	{ mOCTVC1_EVENT_HEADER_SWAP( &((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)->Header ); }  \
	((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)->hExecutor = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)->hExecutor); \
	((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)->hRunnerCase = \
		mOCT_SWAP32_IF_LE(((tOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT *)pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT)->hRunnerCase); \
}
#else
#define mOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT_SWAP( pOCTVC1_JOB_MSG_RUNNER_EXECUTOR_ITERATION_EVT )
#endif /* _OCT_ENDIAN_TYPE_LE_ */

#ifdef __cplusplus
}

#endif

#endif /* __OCTVC1_JOB_EVT_SWAP_H__ */
