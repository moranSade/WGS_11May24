/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include "fxLSocketInterfaceApi.h"
#include "clientConfig.h"

#define MAX_RECV_DATA_LEN  (1024 * 32)

typedef enum{
	NO_IMAGE = 0,
	L1_SCANNER_IMAGE = 1,
	L1_IDCATCHER_IMAGE = 2
}l1imageType;

unsigned char l1ImageLoaded = NO_IMAGE;
unsigned char server_reply[MAX_RECV_DATA_LEN] = {0};
unsigned char sendMsgBuf[MAX_RECV_DATA_LEN] = {0};


typedef struct ThreadArgs 
{
    char imsi[SIZE_OF_IDENTITY_STR];
    int sock;
}threadArgs;


void sendHandshakeCmd(int sock)
{
    fxLHandShakeCmd *cmd = (fxLHandShakeCmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxLHandShakeCmd);
    cmd->hdr.msgType = FXL_HANDSHAKE_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    cmd->clientSocketApiVersion = FXL_SOCKETINTERFACEAPI_VERSION;

    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxLHandShakeCmd) , 0) < 0)
    {
    	printf ("TcpClient : Error : Send Handshake Message failed\n");
    }
    else
    {
    	printf ("TcpClient : Handshake Message sent....\n");
    }
}

void loadGsmRusDspImage(int sock)
{
    fxLBootL1Cmd *cmd = (fxLBootL1Cmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxLBootL1Cmd);
    cmd->hdr.msgType = FXL_BOOT_L1_CMD;
    cmd->hdr.rat = FXL_RAT_2G;

    if(dspInUse == PRIMARY)
      cmd->dspType = FXL_PRIMARY_DSP;
    else
      cmd->dspType = FXL_SECONDARY_DSP;

    cmd->l1ImageType = FXL_L1_GSM_SLS_IMAGE;

    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxLBootL1Cmd) , 0) < 0)
    {
    	printf("TcpClient : Sending Boot L1 Command for Scanner failed\n");
    }
    else
    {
    	printf("TcpClient : Boot L1 Command for Scanner sent....\n");
    }
}

void loadGsmNativeDspImage(int sock)
{
     fxLBootL1Cmd *cmd = (fxLBootL1Cmd *)sendMsgBuf;
     cmd->hdr.msgLength = sizeof(fxLBootL1Cmd);
     cmd->hdr.msgType = FXL_BOOT_L1_CMD;
     cmd->hdr.rat = FXL_RAT_2G;

     if( dspInUse == PRIMARY )
       cmd->dspType = FXL_PRIMARY_DSP;
     else
       cmd->dspType = FXL_SECONDARY_DSP;

     cmd->l1ImageType = FXL_L1_GSM_NATIVE_IMAGE;

     //Send some data
     if( send(sock , sendMsgBuf , sizeof(fxLBootL1Cmd) , 0) < 0)
     {
     	printf("TcpClient : Sending Boot L1 Command for IdCatcher failed\n");
     }
     else
     {
     	printf("TcpClient : Boot L1 Command for IdCatcher sent....\n");
     }
}

void configBaseStationForRus(int sock)
{
    fxL2gListenModeConfigureBandsCmd *cmd = (fxL2gListenModeConfigureBandsCmd*)sendMsgBuf;
    cmd->noOfBandsToScan = 2;
    cmd->band[0] = FXL_BAND_R_GSM;
    cmd->band[1] = FXL_BAND_DCS_1800;
    cmd->noOfScanIterations = 4;

    cmd->hdr.msgLength = sizeof(fxL2gListenModeConfigureBandsCmd);
    cmd->hdr.msgType = FXL_2G_LISTEN_MODE_CONFIGURE_BANDS_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxL2gListenModeConfigureBandsCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Listen Mode Configure Bands send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Listen Mode Configure Bands sent....\n");
    }

}

void configBaseStationForSelectiveScanRus(int sock)
{
	fxL2gListenModeConfigureSelectiveFreqCmd *cmd = (fxL2gListenModeConfigureSelectiveFreqCmd*)sendMsgBuf;
    cmd->noOfArfcnConfigured = 32;

    cmd->bandArfcn[0].arfcn = 19;
    cmd->bandArfcn[0].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[1].arfcn = 21;
    cmd->bandArfcn[1].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[2].arfcn = 23;
    cmd->bandArfcn[2].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[3].arfcn = 27;
    cmd->bandArfcn[3].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[4].arfcn = 29;
    cmd->bandArfcn[4].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[5].arfcn = 68;
    cmd->bandArfcn[5].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[6].arfcn = 71;
    cmd->bandArfcn[6].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[7].arfcn = 73;
    cmd->bandArfcn[7].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[8].arfcn = 75;
    cmd->bandArfcn[8].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[9].arfcn = 78;
    cmd->bandArfcn[9].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[10].arfcn = 80;
    cmd->bandArfcn[10].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[11].arfcn = 82;
    cmd->bandArfcn[11].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[12].arfcn = 103;
    cmd->bandArfcn[12].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[13].arfcn = 107;
    cmd->bandArfcn[13].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[14].arfcn = 109;
    cmd->bandArfcn[14].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[15].arfcn = 108;
    cmd->bandArfcn[15].band  = FXL_BAND_E_GSM;
    cmd->bandArfcn[16].arfcn = 514;
    cmd->bandArfcn[16].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[17].arfcn = 520;
    cmd->bandArfcn[17].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[18].arfcn = 537;
    cmd->bandArfcn[18].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[19].arfcn = 540;
    cmd->bandArfcn[19].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[20].arfcn = 542;
    cmd->bandArfcn[20].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[21].arfcn = 543;
    cmd->bandArfcn[21].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[22].arfcn = 546;
    cmd->bandArfcn[22].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[23].arfcn = 555;
    cmd->bandArfcn[23].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[24].arfcn = 557;
    cmd->bandArfcn[24].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[25].arfcn = 560;
    cmd->bandArfcn[25].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[26].arfcn = 706;
    cmd->bandArfcn[26].band  = FXL_BAND_DCS_1800;
    cmd->bandArfcn[27].arfcn = 708;
    cmd->bandArfcn[27].band  = FXL_BAND_PCS_1900;
    cmd->bandArfcn[28].arfcn = 712;
    cmd->bandArfcn[28].band  = FXL_BAND_PCS_1900;
    cmd->bandArfcn[29].arfcn = 715;
    cmd->bandArfcn[29].band  = FXL_BAND_PCS_1900;
    cmd->bandArfcn[30].arfcn = 707;
    cmd->bandArfcn[30].band  = FXL_BAND_PCS_1900;
    cmd->bandArfcn[31].arfcn = 709;
    cmd->bandArfcn[31].band  = FXL_BAND_PCS_1900;

    cmd->noOfScanIterations = 1;


    cmd->hdr.msgLength = sizeof(fxL2gListenModeConfigureSelectiveFreqCmd);
    cmd->hdr.msgType = FXL_2G_LISTEN_MODE_CONFIGURE_SELECTIVE_FREQUENCIES_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxL2gListenModeConfigureSelectiveFreqCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Listen Mode Configure Selective Frequencies send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Listen Mode Configure Selective Frequencies sent....\n");
    }

}

void configureGsmIdCatcher(int sock)
{
	fxL2gConfigureIdCatcherParamsCmd *cmd = (fxL2gConfigureIdCatcherParamsCmd*)sendMsgBuf;
	cmd->Arfcn = 61;

	if(tcpClientConfig == CONFIG_SCANNER_AND_IDCATCHER_WITH_AUTOCONFIG)
	  cmd->AutoConfigureEnabled = 1;
	else
	  cmd->AutoConfigureEnabled = 0;

    cmd->Band = FXL_BAND_E_GSM;
    cmd->ncc = 3;
    cmd->bcc = 5;
    cmd->Ci  = 765;
    cmd->Lac = 2357;
    cmd->plmn.mccLength = 3;
    cmd->plmn.mncLength = 2;
    cmd->plmn.mcc[0] = 4;
    cmd->plmn.mcc[1] = 0;
    cmd->plmn.mcc[2] = 4;
    cmd->plmn.mnc[0] = 4;
    cmd->plmn.mnc[1] = 5;
    cmd->TxAttenuationInDb = 0;
    cmd->RxGainInDb = 70;
    cmd->changeWLDefaultRejectCause = 1;
    cmd->defaultWLRejectCause = FXL_REJ_CAUSE_NO_SUITABLE_CELLS_IN_LA;
    cmd->timingAdvanceFilter = 3;
    cmd->emergencyredirNcc = 2;
    cmd->emergencyredirBcc = 6;
    cmd->emergengyredirectenable = 1;
    cmd->emergencyRedirectinfo.startingarfcn = 55;
    cmd->emergencyRedirectinfo.bandIndicator = 0;

    cmd->hdr.msgLength = sizeof(fxL2gConfigureIdCatcherParamsCmd);
    cmd->hdr.msgType = FXL_2G_CONFIGURE_IDCATCHER_PARAMS_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxL2gConfigureIdCatcherParamsCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Configure Id Catcher Params Command send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Configure Id Catcher Params Command sent....\n");
    }
}

void configIdentityList(int sock)
{
    int i;
    fxLConfigureIdentityListCmd *cmd = (fxLConfigureIdentityListCmd*)sendMsgBuf;
    cmd->defaultListConfig = FXL_WHITEUSERSLIST;

    cmd->noOfEntries = 10;

    for(i = 0; i < 10; i++)
    {
        cmd->configList[i].idType   = FXL_IDENTITY_IMSI;
        cmd->configList[i].listType = FXL_BLACKUSERSLIST;
        cmd->configList[i].rejectCauseIfWhitelist = 0;
    }
    memcpy(cmd->configList[0].identity, "404450914318552",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[1].identity, "404450914318591",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[2].identity, "404450914318546",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[3].identity, "001012345678904",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[4].identity, "001012345678905",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[5].identity, "001012345678906",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[6].identity, "001012345678907",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[7].identity, "001012345678908",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[8].identity, "001012345678909",SIZE_OF_IDENTITY_STR) ;
    memcpy(cmd->configList[9].identity, "001012345678920",SIZE_OF_IDENTITY_STR) ;

    cmd->hdr.msgLength = sizeof(fxLConfigureIdentityListCmd);
    cmd->hdr.msgType = FXL_CONFIGURE_IDENTITY_LIST_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxLConfigureIdentityListCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Configure Identity List Command send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Configure Identity List Command sent....\n");
    }

}

void changeTxRxAntennaId(int sock)
{
    fxLChangeTxRxAntennaIdCmd *cmd = (fxLChangeTxRxAntennaIdCmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxLChangeTxRxAntennaIdCmd);
    cmd->hdr.msgType   = FXL_CHANGE_TX_RX_ANTENNA_ID_CMD;
    cmd->hdr.rat       = FXL_RAT_2G;
	cmd->dspType       = FXL_PRIMARY_DSP;
    cmd->txAntennaId   = FXL_TX_RX_ANTENNA_ID_0;
    cmd->rxAntennaId   = FXL_TX_RX_ANTENNA_ID_0;

    //Send 
    if( send(sock , sendMsgBuf , sizeof(fxLChangeTxRxAntennaIdCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Change Tx Rx Antenna Id Command send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Change Tx Rx Antenna Id Command sent....\n");
    }
}

void startBaseStationForRus(int sock)
{
    fxL2gListenModeStartScanCmd *cmd = (fxL2gListenModeStartScanCmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxL2gListenModeStartScanCmd);
    cmd->hdr.msgType = FXL_2G_LISTEN_MODE_START_SCAN_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    cmd->scanType = scanCategory;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxL2gListenModeStartScanCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Listen Mode Start Command send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Listen Mode Start Command sent....\n");
    }
}

void stopBaseStationForRus(int sock)
{
    fxL2gListenModeStopScanCmd *cmd = (fxL2gListenModeStopScanCmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxL2gListenModeStopScanCmd);
    cmd->hdr.msgType = FXL_2G_LISTEN_MODE_STOP_SCAN_CMD;
    cmd->hdr.rat = FXL_RAT_2G;
    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxL2gListenModeStopScanCmd) , 0) < 0)
    {
        printf("TcpClient : 2G Listen Mode Stop Command send message failed\n");
    }
    else
    {
        printf("TcpClient : 2G Listen Mode Stop Command sent....\n");
    }
}

void startGsmIdCatcher(int sock)
{
     fxL2gStartIdCatcherCmd *cmd = (fxL2gStartIdCatcherCmd *)sendMsgBuf;
     cmd->hdr.msgLength = sizeof(fxL2gStartIdCatcherCmd);
     cmd->hdr.msgType = FXL_2G_START_IDCATCHER_CMD;
     cmd->hdr.rat = FXL_RAT_2G;
     //Send some data
     if( send(sock , sendMsgBuf , sizeof(fxL2gStartIdCatcherCmd) , 0) < 0)
     {
         printf("TcpClient : 2G Start IdCatcher Command send message failed\n");
     }
     else
     {
         printf("TcpClient : 2G Start IdCatcher Command sent....\n");
     }
}

void enableTxCalibarion(int sock)
{
    fxLEnableTxCalibrationCmd *cmd = (fxLEnableTxCalibrationCmd *)sendMsgBuf;
    cmd->hdr.msgLength = sizeof(fxLEnableTxCalibrationCmd);
    cmd->hdr.msgType = FXL_ENABLE_TX_CALIBRATION_CMD;
    cmd->hdr.rat = FXL_RAT_2G;

    //Send some data
    if( send(sock , sendMsgBuf , sizeof(fxLEnableTxCalibrationCmd) , 0) < 0)
    {
       printf("TcpClient : TX Calibration Command send message failed\n");
    }
    else
    {
       printf("TcpClient : TX Calibration Command sent....\n");
    }
}



void stopGsmIdCatcher(int sock)
{
     fxL2gStopIdCatcherCmd *cmd = (fxL2gStopIdCatcherCmd *)sendMsgBuf;
     cmd->hdr.msgLength = sizeof(fxL2gStopIdCatcherCmd);
     cmd->hdr.msgType = FXL_2G_STOP_IDCATCHER_CMD;
     cmd->hdr.rat = FXL_RAT_2G;
	 cmd->dspResetNeeded = TRUE;
     //Send some data
     if( send(sock , sendMsgBuf , sizeof(fxL2gStopIdCatcherCmd) , 0) < 0)
     {
         printf("TcpClient : 2G Stop IdCatcher Command send message failed\n");
     }
     else
     {
         printf("TcpClient : 2G Stop IdCatcher Command sent....\n");
     }
}

#ifdef FEA_VOICE_CALL_TO_TARGET
void sendSetMoMsIsdnCommand(int sock , unsigned char* imsi)
{
   fxL2gSetMoMsisdnForTargetVoiceCallCmd* cmd = (fxL2gSetMoMsisdnForTargetVoiceCallCmd*)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gSetMoMsisdnForTargetVoiceCallCmd);
   cmd->hdr.msgType = FXL_2G_SET_MO_MSISDN_FOR_TARGET_VOICE_CALL_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   memcpy(cmd->moMsIsdnStr, "+1234567890123", SIZE_OF_IDENTITY_STR);
   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gSetMoMsisdnForTargetVoiceCallCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Set MO MS-ISDN for Target Voice Call Command send message failed for IMSI = %s\n", cmd->imsiStr );
   }
   else
   {
       printf("TcpClient : 2G Set MO MS-ISDN for Target Voice Call Command sent for IMSI = %s....\n", cmd->imsiStr);
   }
}
#endif

void sendGetRssiGpsCommand(int sock , unsigned char* imsi)
{
     fxL2gGetRssiGpsCoordinatesCmd *cmd = (fxL2gGetRssiGpsCoordinatesCmd *)sendMsgBuf;
     cmd->hdr.msgLength = sizeof(fxL2gGetRssiGpsCoordinatesCmd);
     cmd->hdr.msgType = FXL_2G_GET_RSSI_GPS_COORDINATES_CMD;
     cmd->hdr.rat = FXL_RAT_2G;

     cmd->measTriggerType = FXL_MEAS_TRIGGER_CONTINUOUS;
     cmd->startStopTrigger = FXL_START_CONTINUOUS_TRIGGER;
     memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
     //Send some data
     if( send(sock , sendMsgBuf , sizeof(fxL2gGetRssiGpsCoordinatesCmd) , 0) < 0)
     {
         printf("TcpClient : 2G get RSSI, GPS Co-ordinates Command send message failed\n");
     }
     else
     {
         printf("TcpClient : 2G get RSSI, GPS Co-ordinates Command sent....\n");
     }
}

void sendRedirectTo2gCommand(int sock , unsigned char* imsi)
{
     fxL2gRedirectTo2gCellCmd *cmd = (fxL2gRedirectTo2gCellCmd *)sendMsgBuf;
     cmd->hdr.msgLength = sizeof(fxL2gRedirectTo2gCellCmd);
     cmd->hdr.msgType = FXL_2G_REDIRECT_TO_2G_CELL_CMD;
     cmd->hdr.rat = FXL_RAT_2G;

     cmd->gsmRedirectinfo.bandIndicator = FALSE;
     cmd->gsmRedirectinfo.startingarfcn = 520;
     cmd->ncc = 0;
     cmd->bcc = 4;
     memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
     //Send some data
     if( send(sock , sendMsgBuf , sizeof(fxL2gRedirectTo2gCellCmd) , 0) < 0)
     {
         printf("TcpClient : 2G Redirect to 2G Cell(ARFCN = %d, NCC = %d, BCC = %d) failed\n", cmd->gsmRedirectinfo.startingarfcn, cmd->ncc, cmd->bcc);
     }
     else
     {
         printf("TcpClient : 2G Redirect to 2G Cell(ARFCN = %d, BNCC = %d, BCC = %d) sent....\n", cmd->gsmRedirectinfo.startingarfcn, cmd->ncc, cmd->bcc);
     }
}

#ifdef FEA_SILENT_CALL
void sendStartSilentCallCommand(int sock , unsigned char* imsi, fxLSilentCallPurpose purpose)
{
   fxL2gStartSilentCallCmd *cmd = (fxL2gStartSilentCallCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gStartSilentCallCmd);
   cmd->hdr.msgType = FXL_2G_START_SILENT_CALL_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   cmd->purpose = purpose;
   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gStartSilentCallCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Start Silent Call Command send message failed\n");
   }
   else
   {
       printf("TcpClient : 2G Start Silent Call Command sent....\n");
   }

}

void sendModifyUeTxPowerCommand(int sock , unsigned char* imsi)
{
   fxL2gModifyUeTxPowerCmd *cmd = (fxL2gModifyUeTxPowerCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gModifyUeTxPowerCmd);
   cmd->hdr.msgType = FXL_2G_MODIFY_UE_TX_POWER_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   cmd->newPowerLeveldBm = 12;
   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gModifyUeTxPowerCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Modify UE Tx Power Command send message failed\n");
   }
   else
   {
       printf("TcpClient : 2G Modify UE Tx Power Command sent....\n");
   }

}

#ifdef FEA_2G_DYNAMIC_SYS_INFO_CONFIG
void systemInfoModify(int sock)
{
   fxL2gSysInfoModifyCmd *cmd = (fxL2gSysInfoModifyCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gSysInfoModifyCmd);
   cmd->hdr.msgType = FXL_2G_SYS_INFO_MODIFY_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   cmd->cellIdentity = -1;
   cmd->lac = -1;
   cmd->rxlevAccMin = -1;
   cmd->cellReselHyst = -1;
   cmd->msTxpwrMaxCcch = -1;
   cmd->cbq = -1;
   cmd->cro = -1;
   cmd->tempOffset = -1 ;
   cmd->penaltyTime = -1;
   cmd->noOfNeighbors = 5;
   cmd->arfcnNeighbors[0] = 10;
   cmd->arfcnNeighbors[1] = 20;
   cmd->arfcnNeighbors[2] = 30;
   cmd->arfcnNeighbors[3] = 40;
   cmd->arfcnNeighbors[4] = 50;
#if 0
   cmd->arfcnNeighbors[0] = 610;
   cmd->arfcnNeighbors[1] = 620;
   cmd->arfcnNeighbors[2] = 630;
   cmd->arfcnNeighbors[3] = 640;
   cmd->arfcnNeighbors[4] = 650;
#endif
    cmd->t3212 = 50;
    cmd->noOfHoppingNeighbors = 10;
    cmd->hoppingArfcnNeighbors[0] = 5;
    cmd->hoppingArfcnNeighbors[1] = 10;
    cmd->hoppingArfcnNeighbors[2] = 15;
    cmd->hoppingArfcnNeighbors[3] = 20;
    cmd->hoppingArfcnNeighbors[4] = 25;
    cmd->hoppingArfcnNeighbors[5] = 30;
    cmd->hoppingArfcnNeighbors[6] = 35;
    cmd->hoppingArfcnNeighbors[7] = 40;
    cmd->hoppingArfcnNeighbors[8] = 45;
    cmd->hoppingArfcnNeighbors[9] = 50;

    cmd->accessClassList = 0;

   if( send(sock , sendMsgBuf , sizeof(fxL2gSysInfoModifyCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Modify System Info Command send message failed\n");
   }
   else
   {
       printf("TcpClient : 2G Modify System Info Command sent....\n");
   }
}
#endif

void sendEndSilentCallCommand(int sock , unsigned char* imsi)
{
   fxL2gEndSilentCallCmd *cmd = (fxL2gEndSilentCallCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gEndSilentCallCmd);
   cmd->hdr.msgType = FXL_2G_END_SILENT_CALL_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gEndSilentCallCmd) , 0) < 0)
   {
	   printf("TcpClient : 2G End Silent Call Command send message failed\n");
   }
   else
   {
	   printf("TcpClient : 2G End Silent Call Command sent....\n");
   }
}
#endif /* FEA_SILENT_CALL */

#ifdef FEA_ALERT
void alertBlackUserCmd(int sock , unsigned char* imsi)
{
   fxL2gStartAlertBlacklistCmd* cmd = (fxL2gStartAlertBlacklistCmd*)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gStartAlertBlacklistCmd);
   cmd->hdr.msgType = FXL_2G_ALERT_BLACKLIST_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   memcpy(cmd->callingPartyIsdnStr, "+1234567890", SIZE_OF_IDENTITY_STR);
   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gStartAlertBlacklistCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Start ALERT Command send message failed for IMSI = %s\n", cmd->imsiStr );
   }
   else
   {
       printf("TcpClient : ALERT Command sent for IMSI = %s....\n", cmd->imsiStr);
   }

}

void * sendAlertTriggerInLoop(void *arg)
{
	printf("Inside %s\n",__FUNCTION__);
	threadArgs *tArgs_Alert;
	sleep(10);
	tArgs_Alert = (threadArgs*)(arg);
	while(1)
	{
        alertBlackUserCmd(tArgs_Alert->sock , tArgs_Alert->imsi);
	    sleep(60);
	}
	return;
}
#endif /*FEA_ALERT*/

#ifdef FEA_MT_SMS
void sendIndividualSmsCommand(int sock , unsigned char* imsi)
{
   fxL2gSendIndividualSmsCmd *cmd = (fxL2gSendIndividualSmsCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gSendIndividualSmsCmd);
   cmd->hdr.msgType = FXL_2G_SEND_INDIVIDUAL_SMS_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   memcpy(cmd->imsiStr , imsi , SIZE_OF_IDENTITY_STR);
   memcpy(cmd->smsMessage,"Hi, Your Phone is Blacklisted !!", MAX_SIZE_OF_SMS);
   memcpy(cmd->moMsisdnNoStr, "+1234567890", SIZE_OF_IDENTITY_STR);

   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gSendIndividualSmsCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Send Individual SMS Command send message failed for IMSI = %s\n", cmd->imsiStr);
   }
   else
   {
       printf("TcpClient : 2G Send Individual SMS Command sent for IMSI = %s....\n", cmd->imsiStr);
   }

}

void sendBroadcastSmsCommand(int sock)
{
   fxL2gSendBroadcastSmsCmd *cmd = (fxL2gSendBroadcastSmsCmd *)sendMsgBuf;
   cmd->hdr.msgLength = sizeof(fxL2gSendBroadcastSmsCmd);
   cmd->hdr.msgType = FXL_2G_SEND_BROADCAST_SMS_CMD;
   cmd->hdr.rat = FXL_RAT_2G;

   memcpy(cmd->smsMessage,"Broadcast message - Your Phone is Blacklisted :) !!", MAX_SIZE_OF_SMS);
   memcpy(cmd->moMsisdnNoStr, "+1234567890", SIZE_OF_IDENTITY_STR);

   //Send some data
   if( send(sock , sendMsgBuf , sizeof(fxL2gSendBroadcastSmsCmd) , 0) < 0)
   {
       printf("TcpClient : 2G Send Broadcast SMS Command send message failed\n");
   }
   else
   {
       printf("TcpClient : 2G Send Broadcast SMS Command sent\n");
   }

}


void * sendSMSTriggerInLoop(void *arg)
{
	printf("Inside %s\n",__FUNCTION__);
	threadArgs *tArgs_Sms;
	sleep(10);
	tArgs_Sms = (threadArgs*)(arg);
	while(1)
	{
        sendIndividualSmsCommand(tArgs_Sms->sock , tArgs_Sms->imsi);
	    sleep(60);
	}
	return;
}
#endif

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    fxLHeader *hdr = NULL;
    int enable = 1024*32;
    //pthread_t alrtTid1 , alrtTid2 , alrtTid3 , alrtTid4;
//	pthread_t smsTid1 , smsTid2 , smsTid3;
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
    	printf("TcpClient : Error : Socket creation failed\n");
    	exit (0);
    }
    printf("TcpClient : Socket created....\n");

    /* Reuse the address */
	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
	   printf("TcpClient : Error : failed to set socket option for reuse\n");
	   return 1;
	}

	/* Set Send Buffer Size */
	enable = 1024*32;
	if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF, &enable, sizeof(int)) < 0)
	{
	   printf("TcpClient : Error : failed to set socket option for send buffer size\n");
	   return 1;
	}

	/* Set Recv Buffer Size */
	enable = 1024*32;
	if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF, &enable, sizeof(int)) < 0)
	{
	   printf("TcpClient : Error : failed to set socket option for recv buffer size\n");
	   return 1;
	}

    server.sin_addr.s_addr = inet_addr(ipaddress);
    server.sin_family = AF_INET;

    if(dspInUse == PRIMARY)
      server.sin_port = htons( SERVER_PORT_FOR_PRI );
    else
      server.sin_port = htons( SERVER_PORT_FOR_SEC );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
    	printf("TcpClient : Error : Connection failed to PORT = %d \n" , server.sin_port);
    	exit (0);
    }

    printf("TcpClient : Connected.....\n");
    sendHandshakeCmd(sock);
     
    //keep communicating with server
    while(1)
    {       
        //Receive a reply from the server
        if( recv(sock , server_reply , MAX_RECV_DATA_LEN , 0) < 0)
        {
        	puts("TcpClient : Error : Receive failed");
        }
        hdr = (fxLHeader*)server_reply;

        switch(hdr->msgType)
        {
            case FXL_HANDSHAKE_RSP:
            {
                fxLHandShakeRsp *rsp = (fxLHandShakeRsp*)server_reply;
                printf("TcpClient : Handshake Response received with result(%ld)\n", rsp->result);
                if( tcpClientConfig == CONFIG_IDCATCHER_ONLY )
                {
                    configIdentityList(sock);
                }
                else
                {
                	l1ImageLoaded = L1_SCANNER_IMAGE;
                    loadGsmRusDspImage(sock);
                }
            }
            break;

            case FXL_BOOT_L1_RSP:
            {
                fxLBootL1Rsp *rsp = (fxLBootL1Rsp*)server_reply;
                printf("TcpClient : Boot L1 Response received with result(%ld)\n",rsp->result);

                if( l1ImageLoaded == L1_SCANNER_IMAGE )
                {
                	if(scanInUse == SELECTIVE_SCAN)
                	configBaseStationForSelectiveScanRus(sock) ;
                	else
                	configBaseStationForRus(sock);
                }
                else
                  configureGsmIdCatcher(sock);
            }
            break;

            case FXL_2G_LISTEN_MODE_CONFIGURE_BANDS_RSP:
            {
                fxL2gListenModeConfigureBandsRsp *rsp = (fxL2gListenModeConfigureBandsRsp*)server_reply;
                //printf("TcpClient : Listen Mode Configure Bands Response received with result(%ld)\n",rsp->result);
                startBaseStationForRus(sock);
            }
            break;

            case FXL_2G_LISTEN_MODE_CONFIGURE_SELECTIVE_FREQUENCIES_RSP:
            {
            	fxL2gListenModeConfigureSelectiveFreqRsp *rsp = (fxL2gListenModeConfigureSelectiveFreqRsp*)server_reply;
                //printf("TcpClient : Listen Mode Configure GSM Selective Frequencies Response received with result(%ld)\n",rsp->result);
                startBaseStationForRus(sock);
            }
            break;

            case FXL_2G_LISTEN_MODE_START_SCAN_RSP:
            {
                fxL2gListenModeStartScanRsp *rsp= (fxL2gListenModeStartScanRsp*)server_reply;
                printf("TcpClient : Start Scan Command Response received with result(%ld)\n",rsp->result);
            }
            break;

            case FXL_2G_LISTEN_MODE_SYSTEM_INFO_IND:
            {
                fxL2gListenModeSystemInfoInd *ind = (fxL2gListenModeSystemInfoInd*)server_reply;
                int i,j = 0;

                printf("============================================================\n");
		        printf("      TcpClient : 2G Listen Mode Sys Info Ind received\n");
		        printf("============================================================\n");
		        printf("MCC\t\t\t\t\t\t:\tlen(%d), %x%x%x\n", ind->plmn.mccLength, ind->plmn.mcc[0],ind->plmn.mcc[1],ind->plmn.mcc[2]);
		        printf("MNC\t\t\t\t\t\t:\tlen(%d), %x%x%x\n", ind->plmn.mncLength, ind->plmn.mnc[0],ind->plmn.mnc[1],ind->plmn.mnc[2]);
		        printf("ARFCN\t\t\t\t\t\t:\t%ld\n", ind->arfcn);
		        printf("Band\t\t\t\t\t\t:\t%ld\n", ind->band);
		        printf("LAC\t\t\t\t\t\t:\t%ld\n", ind->lac);
		        printf("CELL-ID\t\t\t\t\t\t:\t%ld\n", ind->cellId);
		        printf("NCC\t\t\t\t\t\t:\t%ld\n", ind->ncc);
                printf("BCC\t\t\t\t\t\t:\t%ld\n", ind->bcc);
		        printf("RSSI\t\t\t\t\t\t:\t%f\n", ind->rssi);
		        printf("SNR\t\t\t\t\t\t:\t%f\n", ind->snr);
                printf("C1 \t\t\t\t\t\t:\t%ld\n", ind->c1);
                printf("C2 \t\t\t\t\t\t:\t%ld\n", ind->c2);

                printf("RXLEV_ACCESS_MIN\t\t\t\t:\t%ld\n", ind->rxlevAccMin);
                printf("MS_TXPWR_MAX_CCH\t\t\t\t:\t%ld\n", ind->msTxpwrMaxCcch);
                printf("CELL_RESELECT_OFFSET\t\t\t\t:\t%ld\n", ind->cro);
                printf("TEMPORARY_OFFSET\t\t\t\t:\t%ld\n", ind->tempOffset);
                printf("PENALTY_TIME \t\t\t\t:\t%ld\n", ind->penaltyTime);

                for(i=0; i < ind->noOfHoppingArfcns; i++)
                  printf("Hopping ARFCN[%ld]\t\t\t\t:\t%ld\n", i, ind->hoppingList[i]);

                printf("No-Of-Neighbors\t\t\t\t\t:\t%ld\n",ind->noOfNeighbors);
                for(i=0; i < ind->noOfNeighbors; i++)
                    printf("Neighbor ARFCN[%ld]\t\t\t\t:\t%ld\n", i, ind->arfcnNeighbors[i]);

                printf("Serving Cell Prio Params Present\t\t:\t%ld\n",ind->si2qInfo.servCellPrioParams.scellPrioParamsPresent);
                if(ind->si2qInfo.servCellPrioParams.scellPrioParamsPresent)
                {
                    printf("Serving Cell Prio Params -> Geran-Priority:\t%ld\n",ind->si2qInfo.servCellPrioParams.geranPriority);
                    printf("Serving Cell Prio Params -> Thresh-Prio-Search:\t%ld\n",ind->si2qInfo.servCellPrioParams.threshPrioSearch);
                    printf("Serving Cell Prio Params -> Thresh-Gsm-Low:\t%ld\n",ind->si2qInfo.servCellPrioParams.threshGsmLow);
                    printf("Serving Cell Prio Params -> H-Priority:\t\t%ld\n",ind->si2qInfo.servCellPrioParams.hPrio);
                    printf("Serving Cell Prio Params -> T-Reselection:\t\t%ld\n",ind->si2qInfo.servCellPrioParams.tReselection);
                }

                printf("3G Meas -> QSearch-I\t\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.qSearchI);
                printf("3G Meas -> QSearch-C-Initial\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.qSearchCInit);
                printf("3G Meas -> FDD-Params-Present\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.fddParamsPresent);
                if(ind->si2qInfo.umtsMeasParams.fddParamsPresent)
                {
                    printf("3G Meas -> FDD-Q-Min\t\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.fddParams.fddQmin);
                    printf("3G Meas -> FDD-Q-Offset\t\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.fddParams.fddQoffset);
                    printf("3G Meas -> FDD-Rep-Quantity\t\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.fddParams.fddRepQuant);
                    printf("3G Meas -> FDD-MultiRAT-Reporting\t\t:\t%ld\n",ind->si2qInfo.umtsMeasParams.fddParams.fddMultiratReporting);
                }

                printf("3G Neighbors -> Number-Of-3G-Neighbors\t\t:\t%ld\n",ind->si2qInfo.umtsNeighbors.noNeighUmtsCells);
                printf("3G Neighbors -> Priority-Info-Present\t\t:\t%ld\n",ind->si2qInfo.umtsNeighbors.priorityParamsPresent);
                for(i = 0; i < ind->si2qInfo.umtsNeighbors.noNeighUmtsCells; i++)
                {
                    printf("3G Neighbors[%ld] -> UARFCN\t\t\t:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].uarfcn);
                    printf("3G Neighbors[%ld] -> PSC\t\t\t\t:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].psc);
                    printf("3G Neighbors[%ld] -> Diversity-Applied\t\t:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].diversityApplied);
                    if(ind->si2qInfo.umtsNeighbors.priorityParamsPresent)
                    {
                        printf("3G Neighbors[%ld] -> Utran-Priority\t:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].utranPriority);
                        printf("3G Neighbors[%ld] -> Thresh-Utran-High-Db\t:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].threshUtranHighDb);
                        printf("3G Neighbors[%ld] -> Thresh-Utran-Low-Db:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].threshUtranLowDb);
                        printf("3G Neighbors[%ld] -> Utran-QrxLevMin-Dbm:\t%ld\n", i, ind->si2qInfo.umtsNeighbors.neighFddCell[i].utranQrxlevminDbm);
                    }
                }
                
                printf("4G Neighbors -> Number-Of-4G-Neighbor-Freq\t:\t%ld\n",ind->si2qInfo.lteNeighbors.noEutranFreqs);
                for(i = 0; i < ind->si2qInfo.lteNeighbors.noEutranFreqs; i++)
                {
                    printf("4G Neighbors[%ld] -> EARFCN\t\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].earfcn);
                    for(j = 0; j < E_MAX_PCID_BITMAP; j++)
                    {
                        printf("4G Neighbors[%ld] -> PCID-BitMap[%ld]\t\t\t:\t%ld\n", i, j, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranPcidBitmap[j]);
                    }
                    printf("4G Neighbors[%ld] -> Meas-BW-Nrb\t\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranMeasBwNrb);
                    printf("4G Neighbors[%ld] -> Eutran-Priority\t\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranPriorityInfo.eutranPriority);
                    printf("4G Neighbors[%ld] -> Eutran-QrxLevMin-Dbm\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranPriorityInfo.eutranQrxlevminDbm);
                    printf("4G Neighbors[%ld] -> Thresh-Eutran-High-Db\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranPriorityInfo.threshEutranHighDb);
                    printf("4G Neighbors[%ld] -> Thresh-Eutran-Low-Db\t\t:\t%ld\n", i, ind->si2qInfo.lteNeighbors.eutranFreq[i].eutranPriorityInfo.threshEutranLowDb);
                }
                printf("T3212 %ld \n",ind->t3212);
                printf("\n");
            }
            break;

            case FXL_2G_LISTEN_MODE_SCAN_COMPLETE_IND:
            {
            	fxL2gListenModeScanCompleteInd *rsp= (fxL2gListenModeScanCompleteInd*)server_reply;
            	printf("TcpClient : Listen Mode scan Complete Indication received\n");
                printf("#########################################################\n");
                printf("\n");
                
                stopBaseStationForRus(sock);
            }
            break;

            case FXL_2G_LISTEN_MODE_STOP_SCAN_RSP:
            {
                fxL2gListenModeStopScanRsp *rsp= (fxL2gListenModeStopScanRsp*)server_reply;
                printf("TcpClient : 2G Listen Mode Stop Scan Response received with result(%ld)\n",rsp->result);

                if( tcpClientConfig == CONFIG_SCANNER_ONLY )
                {
                	exit(1);
                }
                else
                {
                	configIdentityList(sock);
                }
            }
            break;

            case FXL_CONFIGURE_IDENTITY_LIST_RSP:
    	    {
        	    fxLConfigureIdentityListRsp *rsp = (fxLConfigureIdentityListRsp*)server_reply;
                printf("TcpClient : 2G Identity List Configure Response received with result(%ld)\n", rsp->result);
                changeTxRxAntennaId(sock);
    	    }
    	    break;

            case FXL_CHANGE_TX_RX_ANTENNA_ID_RSP:
    	    {
        	    fxLConfigureIdentityListRsp *rsp = (fxLConfigureIdentityListRsp*)server_reply;
        		printf("TcpClient : 2G Change Tx Rx Antenna Id Response received with result(%ld)\n", rsp->result);
        		l1ImageLoaded = L1_IDCATCHER_IMAGE;
        		loadGsmNativeDspImage(sock);
    	    }
    	    break;

            case FXL_2G_CONFIGURE_IDCATCHER_PARAMS_RSP:
    	    {
        		fxL2gConfigureIdCatcherParamsRsp *rsp = (fxL2gConfigureIdCatcherParamsRsp*)server_reply;
        		printf("TcpClient : 2G Configure Id Catcher Params Response received with result(%ld)\n", rsp->result);
        		startGsmIdCatcher(sock);
    	    }
    	    break;

            case FXL_2G_START_IDCATCHER_RSP:
    	    {
    		    fxL2gStartIdCatcherRsp *rsp= (fxL2gStartIdCatcherRsp*)server_reply;
                printf("TcpClient : 2G Id Catcher Start Response received with result(%ld)\n", rsp->result);
                 sleep(5);
                enableTxCalibarion(sock);

#ifdef FEA_2G_DYNAMIC_SYS_INFO_CONFIG
                //systemInfoModify(sock); 
#endif
            }
    	    break;

            case FXL_ENABLE_TX_CALIBRATION_RSP:
            {
                fxLEnableTxCalibrationRsp *rsp= (fxLEnableTxCalibrationRsp*)server_reply;
                printf("TcpClient : TX Calibration Response received with result(%d) and cause (%d)\n",rsp->result,rsp->failureCause);
            }
    	    break;

    	    case FXL_2G_REGISTRATION_ATTEMPTED_IND:
    	    {
    		fxL2gRegistrationAttemptedInd *ind = (fxL2gRegistrationAttemptedInd*)server_reply;
    		printf("==============================================================\n");

    		if (ind->listType == FXL_WHITEUSERSLIST)
    		{
        	    printf("\t\tWhite Listed UE Details\n");
                printf("IMSI = %s, IMEI = %s\n", ind->imsiStr, ind->imeiStr);
    		    printf("TMSI = %#X%X%X%X\n",
    		    ind->tmsi[0] & 0xff, ind->tmsi[1] & 0xff,
    		    ind->tmsi[2] & 0xff, ind->tmsi[3] & 0xff);
                printf("RSSI(dBm) = %lf, SNR(dB) = %lf\n", ind->rssidBm, ind->snrdB);
           	    printf("TA = %d\n", ind->timingAdvance);
                printf("CLASSMARK = %02X %02X %02X %02X\n", ind->msClassmark.encodedCM2[0], ind->msClassmark.encodedCM2[1], ind->msClassmark.encodedCM2[2], ind->msClassmark.encodedCM2[3]);
                printf("imeiSv = %d\n", ind->imeiSv);
                printf("last_lac = %d\n", ind->last_lac);
            
            }
    		else if (ind->listType == FXL_BLACKUSERSLIST)
    		{
    		    printf("\t\tBlack Listed UE Details\n");
    		    printf("IMSI = %s, IMEI = %s\n", ind->imsiStr, ind->imeiStr);
    		    printf("TMSI = %#X%X%X%X\n",
    		    ind->tmsi[0] & 0xff, ind->tmsi[1] & 0xff,
    		    ind->tmsi[2] & 0xff, ind->tmsi[3] & 0xff);
               	printf("RSSI(dBm) = %lf, SNR(dB) = %lf\n", ind->rssidBm, ind->snrdB);
           	    printf("TA = %d\n", ind->timingAdvance);
                printf("CLASSMARK = %02X %02X %02X %02X\n", ind->msClassmark.encodedCM2[0], ind->msClassmark.encodedCM2[1], ind->msClassmark.encodedCM2[2], ind->msClassmark.encodedCM2[3]);
                printf("imeiSv = %d\n", ind->imeiSv);
                printf("last_lac = %d\n", ind->last_lac);
            }
    		else
    		{}
    		printf("==============================================================\n");
    		if(ind->listType == FXL_BLACKUSERSLIST)
    		{
                  
                if( (strcmp(ind->imsiStr, "001012345678920") == 0) )      
                {
                    sleep(2);
    			    			// sendStartSilentCallCommand(sock , ind->imsiStr, FXL_TCH_SILENT_CALL);
                    //sendSetMoMsIsdnCommand(sock, ind->imsiStr);
                }

#if 0
    		    //sendBroadcastSmsCommand(sock);
                if( (strcmp(ind->imsiStr, "001012345678901") == 0) )      
    			{
                    sleep(2);
                    sendGetRssiGpsCommand(sock , ind->imsiStr);
                }

                else if( (strcmp(ind->imsiStr, "001012345678904") == 0) )      
    			{
                    sleep(2);
                    sendRedirectTo2gCommand(sock , ind->imsiStr);
                }

#ifdef FEA_SILENT_CALL
    		    else if( (strcmp(ind->imsiStr, "001012345678902") == 0) || \
                         (strcmp(ind->imsiStr, "001012345678903") == 0) )
                {
    		        sleep(2);
    			    printf("Starting SDCCH Silent Call for IMSI %s\n\n",ind->imsiStr);
    			    sendStartSilentCallCommand(sock , ind->imsiStr, FXL_SDCCH_SILENT_CALL);
    		    }
#endif /* FEA_SILENT_CALL */

#ifdef FEA_MT_SMS
                else if(strcmp(ind->imsiStr, "001012345678905") == 0)
    		    {
    		        threadArgs tArgs_Sms1;
    			    tArgs_Sms1.sock = sock;
    			    memcpy(tArgs_Sms1.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );
    			    printf("Creating Thread to send SMS for IMSI = %s\n ",ind->imsiStr );
    						
    			    if(pthread_create(&smsTid1, NULL, sendSMSTriggerInLoop, &tArgs_Sms1))
    			      printf("Thread Creation failed ");
    	        }
    			else if(strcmp(ind->imsiStr, "001012345678906") == 0)
    		    {
    		        threadArgs tArgs_Sms2;
    			    tArgs_Sms2.sock = sock;
    			    memcpy(tArgs_Sms2.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );				
    			    printf("Creating Thread to send SMS for IMSI = %s\n ",ind->imsiStr );
    						
    			    if(pthread_create(&smsTid2, NULL, sendSMSTriggerInLoop, &tArgs_Sms2))
    			      printf("Thread Creation failed ");
    	        }
    			else if(strcmp(ind->imsiStr, "001012345678907") == 0)
    		    {
    		        threadArgs tArgs_Sms3;
    			    tArgs_Sms3.sock = sock;
    			    memcpy(tArgs_Sms3.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );				
    			    printf("Creating Thread to send SMS for IMSI = %s\n ",ind->imsiStr );
    						
    			    if(pthread_create(&smsTid3, NULL, sendSMSTriggerInLoop, &tArgs_Sms3))
    			      printf("Thread Creation failed ");
    	        }
#endif

#ifdef FEA_ALERT
    		    else if(strcmp(ind->imsiStr, "001012345678904") == 0)
    		    {
    		        threadArgs tArgs_Alert1;
    			    tArgs_Alert1.sock = sock;
    			    memcpy(tArgs_Alert1.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );
    			    printf("Creating Thread to send ALERT for IMSI = %s\n ",ind->imsiStr);
    						
    			    if(pthread_create(&alrtTid1, NULL, sendAlertTriggerInLoop, &tArgs_Alert1))
    			      printf("Thread Creation failed ");
    	        }

    		    else if(strcmp(ind->imsiStr, "001012345678908") == 0)
    		    {
    			    threadArgs tArgs_Alert2;
    			    tArgs_Alert2.sock = sock;
    			    memcpy(tArgs_Alert2.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR);
    			    printf("Creating Thread to send ALERT for IMSI = %s\n ",ind->imsiStr );
    							
    			    if(pthread_create(&alrtTid2, NULL, sendAlertTriggerInLoop, &tArgs_Alert2))
    			      printf("Thread Creation failed ");
    	        }

    		    else if(strcmp(ind->imsiStr, "001012345678909") == 0)
    		    {
    			    threadArgs tArgs_Alert3;
    			    tArgs_Alert3.sock = sock;
    			    memcpy(tArgs_Alert3.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );
    			    printf("Creating Thread to send ALERT for IMSI = %s\n ",ind->imsiStr );
    							
    			    if(pthread_create(&alrtTid3, NULL, sendAlertTriggerInLoop, &tArgs_Alert3))
    			      printf("Thread Creation failed "); 
                }

                else if(strcmp(ind->imsiStr, "001012345678910") == 0)
    		    {
    			    threadArgs tArgs_Alert4;
    			    tArgs_Alert4.sock = sock;
    			    memcpy(tArgs_Alert4.imsi, ind->imsiStr, SIZE_OF_IDENTITY_STR );			
    			    printf("Creating Thread to send ALERT for IMSI = %s\n ",ind->imsiStr );
    							
    			    if(pthread_create(&alrtTid4, NULL, sendAlertTriggerInLoop, &tArgs_Alert4))
    			      printf("Thread Creation failed "); 
    		    }
#endif

                else
    		    {
                    // Do Nothing
    		    }
#endif
    					
    		}
    		//stopGsmIdCatcher(sock);
    	    }
    	    break;
#ifdef FEA_SILENT_CALL
    	    case FXL_2G_START_SILENT_CALL_RSP:
            {
                fxL2gStartSilentCallRsp *rsp = (fxL2gStartSilentCallRsp*)server_reply;
                printf("TcpClient : 2G Start Silent Call Response received for IMSI = %s with result(%ld)\n", rsp->imsiStr, rsp->result);
            }
            break;

    	    case FXL_2G_SILENT_CALL_MEAS_IND:
    	    {
        		fxL2gSilentCallMeasInd *ind = (fxL2gSilentCallMeasInd*)server_reply;
        		printf("================== Silent Call Location Info =================\n");
        		printf("Blacklisted IMSI\t:\t%s\n",ind->imsiStr);
        		printf("RSSI\t\t\t:\t%f\n",ind->rssidBm);
        		printf("SNR\t\t\t:\t%f\n",ind->snrdB);
        		printf("==============================================================\n");
    	    }
    	    break;

            case FXL_2G_SILENT_CALL_RESOURCES_IND:
            {
                fxL2gSilentCallResourcesInd *ind = (fxL2gSilentCallResourcesInd*)server_reply;
                printf("TcpClient : 2G Start Silent Call Resources Indication received\n");
                printf("IMSI = %s", ind->imsiStr);
                printf("TSC = %d, Timeslot = %d, SubChannel = %d\n", ind->tsc, ind->timeSlot, ind->subChannel);
                sendModifyUeTxPowerCommand(sock , ind->imsiStr);
            }
            break;

    	    case FXL_2G_MODIFY_UE_TX_POWER_RSP:
    	    {
    	    	fxL2gModifyUeTxPowerRsp *rsp = (fxL2gModifyUeTxPowerRsp*)server_reply;
    	    	printf("TcpClient : 2G Modify UE Tx Power Response received for IMSI = %s with result(%ld)\n", rsp->imsiStr, rsp->result);

    	    	if(rsp->result == FXL_FAILURE)
    	    	{
        		    sleep(100);
        		    sendEndSilentCallCommand(sock , rsp->imsiStr);
    		    }
    	    }
    	    break;

            case FXL_2G_END_SILENT_CALL_RSP:
    	    {
        		fxL2gStopIdCatcherRsp *rsp= (fxL2gStopIdCatcherRsp*)server_reply;
        		printf("TcpClient : 2G End Silent Call Response received with result(%ld)\n", rsp->result);
    	    }
    	    break;
#endif /* FEA_SILENT_CALL */

#ifdef FEA_MT_SMS
            case FXL_2G_SEND_INDIVIDUAL_SMS_RSP:
            {
                fxL2gSendIndividualSmsRsp *rsp= (fxL2gSendIndividualSmsRsp*)server_reply;
                printf("TcpClient : Send Individual SMS Command Response received with result(%ld)\n",rsp->result);
            }
            break;
#endif

#ifdef FEA_DECODE_MO_SMS
            case FXL_2G_MO_SMS_ATTEMPTED_IND:
            {
                fxL2gMoSmsAttemptedInd *ind= (fxL2gMoSmsAttemptedInd*)server_reply;
                printf("========================== MO-SMS-IND ========================\n");
                printf("Blacklisted IMSI\t:\t%s\n",ind->imsiStr);
                printf("Recipient MS-ISDN\t:\t%s\n",ind->msMsisdnOfRecipientNoStr);
                printf("Concatenated Msg\t:\t%s\n",(ind->moSms.isConcatenatedMsg == 1)?"TRUE":"FALSE");
                if(ind->moSms.isConcatenatedMsg == 1)
                {
                    printf("Message Ref Num \t:\t%d\n",ind->moSms.messageRefNo);
                    printf("Total Message Parts\t:\t%d\n",ind->moSms.messageTotalNoOfParts);
                    printf("Message Part Num\t:\t%d\n",ind->moSms.messagePartNumber);
                }
                printf("MO-SMS\t\t\t:\n\n%s\n",ind->moSms.smsMessage);
                printf("==============================================================\n");
            }
            break;
#endif

    	    case FXL_2G_GET_RSSI_GPS_COORDINATES_RSP:
    	    {
        		fxL2gGetRssiGpsCoordinatesRsp *rsp = (fxL2gGetRssiGpsCoordinatesRsp*)server_reply;
        		printf("TcpClient : 2G get RSSI, GPS Co-ordinates Response received with result(%ld)\n", rsp->result);
    	    }
    	    break;

    	    case FXL_2G_GET_RSSI_GPS_COORDINATES_IND:
    	    {
        		fxL2gGetRssiGpsCoordinatesInd *ind = (fxL2gGetRssiGpsCoordinatesInd*)server_reply;
        		printf("==================== GPS-IND Location Info ===================\n");
        		printf("Blacklisted IMSI\t:\t%s\n",ind->imsiStr);
        		printf("RSSI\t\t\t:\t%f\n",ind->rssi);
        		//printf("GPS-INFO\t\t:\t%s\n",ind->gpsInfo);
        		printf("==============================================================\n");
    	    }
    	    break;

    	    case FXL_2G_MO_CALL_ATTEMPTED_IND:
    	    {
        		fxL2gCallAttemptedInd *ind = (fxL2gCallAttemptedInd*)server_reply;
        		printf("======================== Blocked Calls =======================\n");
        		printf("Blacklisted IMSI\t:\t%s\n",ind->imsiStr);
        		printf("DAILED-MS_ISDN\t\t:\t%s\n",ind->msisdnOfDialedNoStr);
        		printf("BLOCKED_CALL_COUNTER\t:\t%d\n",ind->blockedCallCounter);
        		printf("==============================================================\n");
    	    }
    	    break;

            
    	    case FXL_2G_BL_USER_DETACHED_IND:
    	    {
        		fxL2gBlUserDetachedInd *ind = (fxL2gBlUserDetachedInd*)server_reply;
        		printf("======================== BL DETACH IND =======================\n");
        		printf("Blacklisted IMSI\t:\t%s\n",ind->imsiStr);
        		printf("==============================================================\n");
    	    }
    	    break;

    	    case FXL_2G_STOP_IDCATCHER_RSP:
    	    {
    		fxL2gStopIdCatcherRsp *rsp= (fxL2gStopIdCatcherRsp*)server_reply;
    		printf("TcpClient : 2G Id Catcher Stop Response received with result(%ld)\n", rsp->result);
    		configIdentityList(sock);
    	    }
    	    break;

#ifdef FEA_2G_DYNAMIC_SYS_INFO_CONFIG
            case FXL_2G_SYS_INFO_MODIFY_RSP:
            {
                fxL2gSysInfoModifyRsp *rsp= (fxL2gSysInfoModifyRsp*)server_reply;
                printf("TcpClient : 2G Modify System Info Response received with result(%ld)\n", rsp->result);
            }
            break;
#endif

#ifdef FEA_ALERT

            case FXL_2G_ALERT_BLACKLIST_RSP:
            {
                fxL2gStartAlertBlacklistRsp *rsp= (fxL2gStartAlertBlacklistRsp*)server_reply;
        	    printf("TcpClient : ALERT Response received for IMSI = %s....\n", rsp->imsiStr );
                printf("\n");
            }
            break;
#endif

#ifdef FEA_VOICE_CALL_TO_TARGET
            case FXL_2G_SET_MO_MSISDN_FOR_TARGET_VOICE_CALL_RSP:
            {
                fxL2gSetMoMsisdnForTargetVoiceCallRsp *rsp= (fxL2gSetMoMsisdnForTargetVoiceCallRsp*)server_reply;
        	    printf("TcpClient : fxL2gSetMoMsisdnForTargetVoiceCallRsp received for IMSI = %s....\n", rsp->imsiStr );
                printf("\n");
            }
            break;
#endif

            case FXL_LISTEN_MODE_RSSI_SCAN_RESULT_IND:
			{
                                int i;
				fxLListenModeRssiScanResultInd *ind = (fxLListenModeRssiScanResultInd *)server_reply;
				printf("TcpClient : fxLListenModeRssiScanResultInd received\n");
				printf("TcpClinet : Band = %d, Number of Channels = %d\n", ind->band2g, ind->numOfChannels);
				for (i = 0; i < ind->numOfChannels; i++)
				{
				   printf("ARFCN = %d & RSSI = %6.3f\n", 
				   	      ind->channelInfo[i].channelNum, ind->channelInfo[i].rssi);
				}
				printf("\n");
            }
			break;

            default:
            {
                printf("TcpClient : Unknown message received msgType(%ld)\n", hdr->msgType);
            }
            break;
        }
    }
     
    return 0;
}
