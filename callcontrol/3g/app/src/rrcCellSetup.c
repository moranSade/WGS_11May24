/*############################################################################
 *
 *                   *** FXLynx Technologies Ltd. ***
 *
 *     The information contained in this file is the property of FXLynx
 *     Technologies Ltd. Except as specifically authorized in writing by
 *     FXLynx Technologies Ltd. The user of this file shall keep information
 *     contained herein confidential and shall protect same in whole or in
 *     part from disclosure and dissemination to third parties.
 *
 *     Without prior written consent of FXLynx Technologies Ltd. you may not
 *     reproduce, represent, or download through any means, the information 
 *     contained herein in any way or in any form.
 *
 *       (c) FXLynx Technologies Ltd. 2014, All Rights Reserved
 *
 *############################################################################*/
/*############################################################################
  * File Name	: clL1RcvMsg.c  
  *
  * Description : This file has interface with L1, all data/control msg are sent
  *               to L1 throught this file
  *
  * History 	:
  *
  * Date		 Author 				 Details
  * ---------------------------------------------------------------------------
  * 12/09/2014	 Kuldeep S. Chauhan 	 File Creation
  *############################################################################*/
 
/* -----------------------------------------------------
 *                 Include Files
 * ----------------------------------------------------- */
#include "stdio.h"
#include "errorCode.h"
#include "l1MsgStatus.h"
#include "msgHeaderDef.h"
#include "l1capi_datastruct.h"
#include "L1C_responseProcDecHdlr.h"
#include "clL1RcvMsg.h"
#include "msgPrimitives.h"
#include "cmnDs.h"
#include "msgHeaderCodec.h"
#include "oct_l1cIoApi.h"
#include "stdlib.h"
#include "msgGetPrimitives.h"
#include "callCSysInfoUpdate.h"
#include "taskHndlr.h"
#include "rrcItf.h"
//#include "logFile.h"
#include "cmnDebug.h"
#include "clContext.h"
#include "timerApi.h"
#include "umtsCfg.h"

static DbgModule_e  DBG_MODULE = rrc;

extern UmtsConfigControlBlock          gUmtsConfig;
extern L1CAPI_Modem_Setup_Request      gModemSetupReq;
extern L1CAPI_Cell_Delete_Request      gCellDeleteReq;
extern L1CAPI_Cell_Setup_Request       gCellSetupReq;
extern L1CAPI_Sccpch_Setup_Request     gSccpchSetupReq;
extern L1CAPI_Prach_Setup_Request      gPrachSetupReq;
extern L1CAPI_Radiolink_Setup_Request  gRadioLinkSetupReq;

/* UE specific messages */
extern L1CAPI_Radiolink_Setup_Request gRadioLinkSetupReq; 
unsigned int modemSetupRequired = 1;
unsigned int isCellSetupRequired = 1;
//CellState_t gCellState;

extern RrcUeControlBlock                ueControlBlock;
extern fxLDspType       dspType;
void sendStartIdCatcherRsp(unsigned int status);
ErrorCode_e RrcAppCellSmStart(void)
{
    gClCntxt.cellState = CL_CELL_IDLE;

    // Uncomment and fill InitStateMachine (Cell State machine), 
    // Uncomment and fill InitAppClCntxt to Fill gModemSetup, gCellSetup ...
    if (modemSetupRequired  == 1)
    {
        FillModemSetupReq();
        if(SUCCESS_E != oct_modem_setup_req(&gModemSetupReq))
        {
                DEBUG1(("Modem setup FAILED\n"))
                return ERROR_GENERIC_FAILURE_E;
        }
        modemSetupRequired = 0;
        isCellSetupRequired = 1;
        return SUCCESS_E;
    }
    else
    {
        FillCellDeleteReq();
        isCellSetupRequired = 1;
        if(SUCCESS_E != oct_cell_delete_req(&gCellDeleteReq))
        {
             DEBUG1(("Cell Delete Request Failed\n"));
             return ERROR_GENERIC_FAILURE_E;
        }
        return SUCCESS_E;
    }  
}/* End of RrcAppCellSmStart*/

ErrorCode_e CellSmEvntHndlr(Msg_t msgBuf)
{
   DEBUG3(("CellSmEvntHndlr: \n"));
   switch(msgBuf.msgType)
   {
      case CL_NBAP_MODEM_SETUP_RSP_MSG_E:
          if (CL_WAIT_FOR_MODEM_SETUP_RESP == gClCntxt.cellState)
          {
              DEBUGMSG((" MODEM SEUP SUCCESSCULL. Sending CELL DELETE_REQUEST to layer1 \n"));
              FillCellDeleteReq();
              if(SUCCESS_E != oct_cell_delete_req(&gCellDeleteReq))
              {                         
                 DEBUG1(("Cell Delete Request Failed\n"));
                 sendStartIdCatcherRsp(1);
                 return ERROR_GENERIC_FAILURE_E;
              }
          }
          else
          {
              DEBUGMSG(("MODEM SEUP Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("MODEM SETUP Response is not handled and dropped\n"));
          }
          break;

      case CL_NBAP_CELL_DELETE_RSP_E:
          if (CL_WAIT_FOR_CELL_DELETE_RESP == gClCntxt.cellState)
          {
              if (isCellSetupRequired == 0)
                   break;
              DEBUGMSG((" CELL DELETE SUCCESSCULL. Sending CELL SETEP REQUEST to layer1 \n"));
              FillCellSetupReq();
              if(SUCCESS_E != oct_cell_setup_req(&gCellSetupReq))
              {                         
                 DEBUG1(("Cell Setup Request Failed\n"));
                 sendStartIdCatcherRsp(1);
                 return ERROR_GENERIC_FAILURE_E;
              }
          }
          else
          {
              DEBUGMSG(("CELL DELETE Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("CELL DELETE Response is not handled and dropped\n"));
          }
          break;

      case CL_NBAP_CELL_SETUP_RSP_E:
          if (CL_WAIT_FOR_CELL_SETUP_RESP == gClCntxt.cellState)
          {
              DEBUGMSG((" CELL SETUP SUCCESSCULL. Sending SIB PARAM(SYS_INFO_UPDATE) to layer1 \n"));
              if (RFAILED == buildAndSendL1SibMessage())
              {
                  DEBUG1(("Sending SYSTEM_INFO_UPDATE_REQUEST to L1 failed \n"));
                  sendStartIdCatcherRsp(1);
                  return ERROR_GENERIC_FAILURE_E;
              }
          }
          else
          {
              DEBUGMSG(("CELL SETUP Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("CELL SETUP Response is not handled and dropped\n"));
          }
          break;

      case CL_NBAP_SYSTEM_INFO_UPDATE_RSP_E:
          if (CL_WAIT_FOR_BRING_UP_SYSTEM_INFO_UPDATE_RESP == gClCntxt.cellState)
          {
              DEBUGMSG(("SYSTEM_INFO_UPDATE_REQUEST SUCCESSFULL. Sending SCCPCH Setup Req to Layer1\n"));
              fillSccpchSetupRequest();
              if(SUCCESS_E != oct_sccpch_setup_req(&gSccpchSetupReq))
              {                         
                 DEBUG1(("Sccpch Setup Request Request failure \n"));
                 sendStartIdCatcherRsp(1);
                 return ERROR_GENERIC_FAILURE_E;
              }
          }
          else if (CL_WAIT_FOR_ONLINE_CPICH_SYSTEM_INFO_UPDATE_RESP == gClCntxt.cellState)
          {
               gClCntxt.cellState = CL_CELL_ACTIVE;
              
               /* Stop the Cell Timer */
               if (TRUE == isTimerRunning (&gClCntxt.cellTimer))
               {
                   timerStop(&gClCntxt.cellTimer);
               }

               /* Send success message to external client */
               sendChgTxAttnCpichTxPwrRsp (1);
          }
          else if (CL_WAIT_FOR_ONLINE_SYSTEM_INFO_UPDATE_RESP == gClCntxt.cellState)
          {
               gClCntxt.cellState = CL_CELL_ACTIVE;
               /* Stop the Cell Timer */
               if (TRUE == isTimerRunning (&gClCntxt.cellTimer))
               {
                   timerStop(&gClCntxt.cellTimer);
               }

               /* Send success message to external client */
               sendSystemInfoUpdateRsp (1);
          }
          else
          {
              DEBUGMSG(("SYSTEM INFORMATION UPDATE Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("SYSTEM INFORMATION UPDATE Response is not handled and dropped\n"));
          }
          break;

      case CL_NBAP_SCCPCH_SETUP_RSP_E:
          if (CL_WAIT_FOR_SCCPCH_SETUP_RESP == gClCntxt.cellState)
          {
              DEBUGMSG((" SCCPCH SETUP SUCCESSCULL. Sending PRACH Req to layer1 \n"));
              fillPrachSetupRequest();
              DEBUGMSG((" Sending PRACH SETUP REQUEST message to layer1 \n"));
              if(SUCCESS_E != oct_prach_setup_req(&gPrachSetupReq))
              {                         
                  DEBUG1(("Prach Setup Request failed\n"));
                  return ERROR_GENERIC_FAILURE_E;
              }
          }
          else
          {
              DEBUGMSG(("SCCPCH SETUP Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("SCCPCH SETUP Response is not handled and dropped\n"));
          }
          break;

      case CL_NBAP_PRACH_SETUP_RSP_E:
          if (CL_WAIT_FOR_PRACH_SETUP_RESP == gClCntxt.cellState)
          {
              DEBUGMSG((" PRACH SETUP SUCCESSCULL.\n"));

#ifdef FXL_3G_TX_JAMMER
              if(TRUE == gUmtsConfig.txJammerEnabled)
              {
                  printf("  =====================================================\n"); 
              	  printf("     JAMMER IS RUNNING WITH UARFCN = %d, PSC = %d\n", gUmtsConfig.dlUarfcn, gUmtsConfig.psc); 
              	  printf("  =====================================================\n");
              }
			  else
			  {
#endif
              	  printf("  =======================================\n"); 
              	  printf("  ==== CELL BRING UP SUCCESSFULL!! ======\n"); 
              	  printf("  =======================================\n"); 
                  sendStartIdCatcherRsp(0);
#ifdef FXL_3G_TX_JAMMER
			  }
#endif

              gClCntxt.cellState = CL_CELL_ACTIVE;

              /* Initialize Latched UE Details to NULL */
              ueControlBlock.pLatchedUeDetails = NULL;

              /* Initialize Cell Timer */
              timerInit(&gClCntxt.cellTimer);

#ifdef FEA_PICSIX_CHANGE
              /* Send system status indication */
              sendSystemStatusIndication(FXL_3G_IDC, FXL_STARTED);  
#endif			              
			  pageAllConfiguredTargetPhone();
              if (dspType == FXL_PRIMARY_DSP)
              {
                  if (-1 == system("/home/wgs/scripts/setGPSPri.sh"))
                  {
                        printf(": GPS setting failed\n");
                  }
              }
              else
              {
                  if (-1 == system("/home/wgs/scripts/setGPSSec.sh"))
                  {
                        printf(": GPS setting failed\n");
                  }
              } 
          }
          else
          {
              DEBUGMSG(("PRACH SETUP Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("PRACH SETUP Response is not handled and dropped\n"));
          }
          break;

       case CL_NBAP_CELL_RECONFIG_RSP_E:
          if (CL_WAIT_FOR_CELL_RECONFIG_RESP == gClCntxt.cellState)
          {
             DEBUGMSG((" CELL RECONFIG SUCCESSCULL.\n"));

             /* Stop the Cell Reconfig Timer */
             if (TRUE == isTimerRunning (&gClCntxt.cellTimer))
             {
                 timerStop(&gClCntxt.cellTimer);
             }

             /* Send SIB Update for change of CPICH Power */
             if (RFAILED == buildAndSendL1SibMessage())
             {
                 DEBUG1(("Sending SYSTEM_INFO_UPDATE_REQUEST to L1 failed \n"));

                 /* Send Failure message to external client */
                 sendChgTxAttnCpichTxPwrRsp (0);
             }
          }
          else
          {
              DEBUGMSG(("CELL RECONFIG Response Received in Cell State = %d\n", gClCntxt.cellState));
              DEBUG1(("CELL RECONFIG Response is not handled and dropped\n"));
          }
          break;
          
       default:
          DEBUG1(("UNKNOWN Message Received in Cell State = %d\n", gClCntxt.cellState));
          break;
   }
} /*End of CellSmEvntHndlr*/

ErrorCode_e UeSmL1EvtHndlr(Msg_t  msgBuf)
{
    logPrint(LOG_INFO, "UeSmL1EvtHndlr :: lastLac \n");
   ErrorCode_e retCode = SUCCESS_E;
   unsigned int *pUeRLId = NULL;
   UeDedicatedMeasReport *pDedMeasRep = NULL;
   RrcUeSpecificData *pUeSpecificData = NULL;
   unsigned int *pUeId = NULL;

   switch(msgBuf.msgType)
   {
       case CL_NBAP_UE_RADIO_LINK_SETUP_RSP_E:
	   	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RADIO LINK SETUP SUCCESSCULL for UE Id(%ld)\n",
                 *pUeId));
          handleUeEvents (*pUeId, UE_RL_SETUP_SUCCESS);

		  /* Free the memory */
		  free (pUeId);
		  pUeId = NULL;

          break;
       
       case CL_NBAP_UE_RADIO_LINK_FAILURE_E:
	   	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RADIO LINK FAILURE INDICATION for UE Id(%ld)\n", 
                 *pUeId));
          handleUeEvents (*pUeId, UE_RL_SETUP_FAILURE);
		  /* Free the memory */
		  free (pUeId);
		  pUeId = NULL;

          break;
       
       case CL_NBAP_UE_RADIO_LINK_DELETE_RSP_E:
	   	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG((" RADIO LINK DELETE SUCCESS for UE Id(%ld)\n", 
                 *pUeId));
          handleUeEvents (*pUeId, UE_RL_DELETE_SUCCESS);
		  /* Free the memory */
		  free (pUeId);
		  pUeId = NULL;
          break;

      case CL_NBAP_UE_RTT_DEDICATED_MEAS_INIT_RSP_E:
	  	 pDedMeasRep = (UeDedicatedMeasReport *)msgBuf.msgBuf;
         DEBUGMSG(("RTT DEDICATED MEAS INIT RESPONSE SUCCESSCULL for UE Id(%ld)\n",
                   pDedMeasRep->rlId));
		 pUeSpecificData = &ueControlBlock.ueSpecificData[pDedMeasRep->rlId];
		 pUeSpecificData->rttValue = pDedMeasRep->rttValue;
         handleUeEvents (pDedMeasRep->rlId, UE_DED_MEAS_RTT_INIT_REQ_SUCCESS);
		 /* Free the memory */
		 free (pDedMeasRep);
		 pDedMeasRep = NULL;
         break;
       
      case CL_NBAP_UE_RSCP_DEDICATED_MEAS_INIT_RSP_E:
	  	 pDedMeasRep = (UeDedicatedMeasReport *)msgBuf.msgBuf;
         DEBUGMSG(("RSCP DEDICATED MEAS INIT RESPONSE SUCCESSCULL for UE Id(%ld)\n",
                 pDedMeasRep->rlId));
		 pUeSpecificData = &ueControlBlock.ueSpecificData[pDedMeasRep->rlId];
		 pUeSpecificData->rscpValue = pDedMeasRep->rscpValue;
         handleUeEvents (pDedMeasRep->rlId, UE_DED_MEAS_RSCP_INIT_REQ_SUCCESS);
		 /* Free the memory */
		 free (pDedMeasRep);
		 pDedMeasRep = NULL;
         break;

      case CL_NBAP_UE_RTT_DEDICATED_MEAS_REPORT_E:
	  	  pDedMeasRep = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RTT DEDICATED MEAS REPORT  SUCCESSCULL for UE Id(%ld)\n",
                    pDedMeasRep->rlId));
		  pUeSpecificData = &ueControlBlock.ueSpecificData[pDedMeasRep->rlId];
		  pUeSpecificData->rttValue = pDedMeasRep->rttValue;
          handleUeEvents (pDedMeasRep->rlId, UE_DED_MEAS_RTT_REPORT_SUCCESS);
		  /* Free the memory */
		  free (pDedMeasRep);
		  pDedMeasRep = NULL;
          break;
     
      case CL_NBAP_UE_RSCP_DEDICATED_MEAS_REPORT_E:
	  	  pDedMeasRep = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RSCP DEDICATED MEAS REPORT SUCCESSCULL for UE Id(%ld)\n",
                    pDedMeasRep->rlId));
		  pUeSpecificData = &ueControlBlock.ueSpecificData[pDedMeasRep->rlId];
		  pUeSpecificData->rscpValue = pDedMeasRep->rscpValue;

          handleUeEvents (pDedMeasRep->rlId, UE_DED_MEAS_RSCP_REPORT_SUCCESS);
		  /* Free the memory */
		  free (pDedMeasRep);
		  pDedMeasRep = NULL;
          break;

      case CL_NBAP_UE_RTT_DEDICATED_MEAS_TERM_RSP_E:
	  	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RTT DEDICATED MEAS TERM RESPONSE SUCCESSCULL for UE Id(%ld)\n",
                   *pUeId));
          handleUeEvents (*pUeId, UE_DED_MEAS_RTT_TERM_REQ_SUCCESS);
		  /* Free the memory */
		  free (pUeId);
		  pUeId = NULL;
          break;

      case CL_NBAP_UE_RSCP_DEDICATED_MEAS_TERM_RSP_E:
	  	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RSCP DEDICATED MEAS TERM RESPONSE SUCCESSCULL for UE Id(%ld)\n",
                   *pUeId));
          handleUeEvents (*pUeId, UE_DED_MEAS_RSCP_TERM_REQ_SUCCESS);
		  /* Free the memory */
		  free (pUeId);
		  pUeId = NULL;
          break;

      case CL_NBAP_UE_RADIO_LINK_RECONFIG_READY_E:
	  	  pUeId = (unsigned int *)msgBuf.msgBuf;
          DEBUGMSG(("RADIO LINK RECONFIG READY SUCCESSCULL for UE Id(%ld)\n",
                    *pUeId));
          DEBUGMSG(("Send RADIO LINK RECONFIG COMMIT for UE Id(%ld)\n",
                    *pUeId));
          pUeRLId = (unsigned int *)malloc(sizeof(unsigned int));
          *pUeRLId = *pUeId;

          SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                        CL_NBAP_UE_RADIO_LINK_RECONFIG_COMMIT_REQ_E, pUeRLId);
		  free (pUeId);
		  pUeId = NULL;
          break;

      default:
	  	  DEBUG2(("UNKNOWN MESSAGE TYPE received from CL_L1C_RCV_TASK_E\n"));
	  	  pUeId = (unsigned int *)msgBuf.msgBuf;
		  free (pUeId);
		  pUeId = NULL;
          break;
   }

   return (retCode);
}

ErrorCode_e AppSocketItfEvtHndlr (Msg_t  msgBuf)
{
   UeUlTxPwrData *pUeUlTxPwr = NULL;

   UmtsBlUeRedirectInfo *p3gBlUeRedirectInfo = NULL;

   ErrorCode_e retCode = SUCCESS_E;
   DelConfigIdList *pDelConfigId = NULL;

   switch(msgBuf.msgType)
   {
       case APP_STOP_ID_CATCHER_E:
          handleRrcCleanUp ();
          break;

       case RRC_CHANGE_UE_UL_TX_PWR_E:
          pUeUlTxPwr = (UeUlTxPwrData *)msgBuf.msgBuf;
          handleRrcChangeUeUlTxPwr (pUeUlTxPwr);
          break;

       case RRC_REDIRECT_BL_UE_E:
	   	  p3gBlUeRedirectInfo = (UmtsBlUeRedirectInfo *)msgBuf.msgBuf;
		  handleRrcBlUeRedirect (p3gBlUeRedirectInfo);
	   	  break;

       case RRC_DEL_ID_LIST_E:
          pDelConfigId = (DelConfigIdList *)msgBuf.msgBuf;
          handleDelConfigIdList (pDelConfigId);
          break;

       default:
          break;
   }

   return (retCode);
}
