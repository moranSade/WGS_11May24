/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

File: wnm.c

Copyright (c) 2016 Octasic Inc. All rights reserved.

Description:

    Wireless Network Monitor sample.

This source code is Octasic Confidential. Use of and access to this code
is covered by the Octasic Device Enabling Software License Agreement.
Acknowledgement of the Octasic Device Enabling Software License was
required for access to this code. A copy was also provided with the release.

Release: OCTSDR Software Development Kit OCTSDR_RUS-02.11.00-B1821 (2016/07/26)

$Revision:  $

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#define OCTVC1_OPT_DECLARE_DEFAULTS

/***************************  INCLUDE FILES  *********************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
/* RUS API header files */
#include "../inc/octvc1/rus/octvc1_rus_api.h"
#include "../inc/octvc1/rus/octvc1_rus_default.h"
#include "../inc/octvc1/rus/octvc1_rus_api_swap.h"
#include "../inc/octvc1/rus/octvc1_rus_evt_swap.h"
/* Hardare API header files */
#include "../inc/octvc1/hw/octvc1_hw_api.h"
#include "../inc/octvc1/hw/octvc1_hw_default.h"

/* Vocallo API header files */
#include "../inc/octvc1/main/octvc1_main_api.h"
#include "../inc/octvc1/main/octvc1_main_default.h"
#include "../inc/octvc1/main/octvc1_main_api_swap.h"
#include "../inc/octvc1/main/octvc1_main_version.h"
#include "../inc/octvc1/pkt_api/octvc1_pkt_api.h"
#include "../inc/octvc1/pkt_api/octvc1_pkt_api_default.h"
#include "../inc/octvc1/octvc1_macro.h"
#include "../inc/octvc1/gsm/octvc1_gsm_api.h"
#include "../inc/octvc1/gsm/octvc1_gsm_default.h"
#include "../inc/octvc1/gsm/octvc1_gsm_api_swap.h"
#include "../inc/octvc1/gsm/octvc1_gsm_evt.h"
#include "../inc/octvc1/gsm/octvc1_gsm_evt_swap.h"
#include "slsDecode2g.h"
#include "feaConfig.h"

/* Octasic applications' common header files */
#include "../inc/oct_common_getopt.h"
#include "../inc/oct_common_macro.h"

/* Samples' common header files */
#include "../inc/octsamples_pkt_api_session.h"
#include "../inc/octsamples_string_utils.h"
#include "../inc/octsamples_main_api_stats.h"

#include "../inc/octvc1/lte/octvc1_lte_api.h"
#include "../inc/octvc1/lte/octvc1_lte_default.h"
#include "../inc/octvc1/lte/octvc1_lte_api_swap.h"
#include "../inc/octvc1/hw/octvc1_hw_api.h"
#include "../inc/octvc1/hw/octvc1_hw_default.h"
#include "../inc/octvc1/hw/octvc1_hw_api_swap.h"
#include "lteL1Defs.h"
#include "slsDecode4g.h"
/* Verbose OCTVC1 return code */
#define OCTVC1_RC2STRING_DECLARE
#include "../inc/octvc1/octvc1_rc2string.h"

/* UMTS Decode */
#include "asnApi.h"
#include "slsDecode3g.h"
#include "fxLSocketInterfaceApi.h"
#include "oct_l1cIoInit.h"
#include "umtsCfg.h"
#include "ratSocketItf.h"
#include "cmnDebug.h"
#include "logManager.h"
#include "linkedlist.h"
#include "rtsTxMsgHandler.h"
#include "dataIndMsgHandler.h"

#define MAX_SIZE_OF_GSM_L1_PKT_BUFFER 1024

static DbgModule_e DBG_MODULE = sls;

// This buffer is used to send message to L1
tOCT_UINT8 l1MsgSendBuffer[GSM_MACBLOCK_LEN]; // 468
// This buffer is used to receive message from L1
static tOCT_UINT8 l1PacketBuffer[MAX_SIZE_OF_GSM_L1_PKT_BUFFER];
tOCTVC1_SUB_OBJECT_ID bcchBufferCellId = 0xFFFF;
extern int si1Cell;
extern int si2Cell;
extern int si2qCell;
extern int si2bisCell;
extern int si2terCell;
extern int si3Cell;
extern int si4Cell;

pthread_t gsmL1Receiver;
extern int exitRatListenMode;
extern int exitRatBassMode;
extern queue dataIndQ;
extern pthread_cond_t conddataInd;
extern pthread_mutex_t mutexdataInd;
#ifdef FEA_GPRS
extern pthread_cond_t condRlcInd;
extern pthread_mutex_t mutexRlcInd;
extern pthread_cond_t condLlcInd;
extern pthread_mutex_t mutexLlcInd;
int exitGprsAppThreads = 0;
int exitLlcAppThreads = 0;
extern stateDataInd rlcIndRcvdstate;
extern stateDataInd llcIndRcvdstate;
extern pthread_t rlcDataIndThread;
extern pthread_t llcDataIndThread;
#endif
extern pthread_mutex_t mutexPagingQ;
extern pthread_mutex_t mutexBlacklistUserQ;
int exitGsmAppThreads = 0;
extern pthread_t gsmDataIndThread;
// extern pthread_t           gsmTimerTickThread;

extern stateDataInd dataIndRcvdstate;
extern fxL4gListenModeSystemInfoInd sysInfoind;
// L1 Messages Receiver thread
void *l1MsgHandler(void *nothing);
extern listNode *headForChanAllocationQ;
extern listNode *tailForChanAllocationQ;
extern listNode *headForPagingReqQ[MAX_PAGING_PERIODICITY][MAX_PAGING_BLOCKS_IN_51_FRAME];
extern listNode *tailForPagingReqQ[MAX_PAGING_PERIODICITY][MAX_PAGING_BLOCKS_IN_51_FRAME];
extern sint8 destroyListenModeThread();
void configureWnmParameters(fxL2gListenModeConfigureBandsCmd *cmd);
tOCT_UINT32 convertToOct2gBandFromFxL(fxL2gBandType band);
fxL2gBandType convertToFxlBandFromOct(tOCT_UINT32 band);
fxLResult stopListenModeApplication();
static int InitApplication();
extern void initializeSystemInformationInd(void);
#ifdef FXL_4G_TX_JAMMER
extern int exitLteTxJammer;
extern void buildAndSend4gTxJammerStatusInd(fxL4gTxJammerStatus status);
extern void lteConfigureJammerParamsHandler(void);
#endif
int getEarfcnFromFreqBandIndicator(int freqBandIndicator);
extern unsigned int bootL1InProgress;
int gsmGate = FALSE;
extern int gsmRxGainInDb;
extern int gsmTxAttenuationInDb;
extern tOCT_UINT8 checkForInactive2gBLUsers;
extern tOCT_UINT8 cellBarDuringTchSilentCall;
extern gsm_trx *trx_g;
extern gsm_time *gsm_time_g;
extern rts_time *rts_time_g;
extern ScanResult gScanResult;
extern int scanNotRequired;
extern tOCT_UINT8 sndMsgBuf[1024 * 6];
extern eLteBw lteSelectedBandwidth;
static tOCT_UINT8 msgBufSls[EXTERNAL_MESSAGE_SIZE];
extern tOCT_UINT8 timerTickReceived;
extern pthread_cond_t condTimerTickInd;
extern pthread_mutex_t mutexTimerTickInd;
extern oneBinState currentOneBinState;
extern fxLTxRxAntennaId txAntennaIdPri;
extern fxLTxRxAntennaId rxAntennaIdPri;
extern fxLTxRxAntennaId txAntennaIdSec;
extern fxLTxRxAntennaId rxAntennaIdSec;
extern void checkAndSetOnBinState(void);

const tOCT_UINT8 const dummyPagingReq[GSM_MACBLOCK_LEN] =
    {
        0x15, 0x06, 0x21, 0x00, 0x01, 0x00, 0x2B, 0x2B, 0x2B, 0x2B,
        0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
        0x2B, 0x2B, 0x2B};

const tOCT_UINT8 const fillFrameDrop[3] =
    {
        0x01, 0x03, 0x01};

const tOCT_UINT8 const fillFrame[GSM_MACBLOCK_LEN] =
    {
        0x03, 0x03, 0x01, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
        0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
        0x2B, 0x2B, 0x2B};
extern void readyToSendIndHandler(tOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MDA *rtsInd);
extern void get_sysinfo(gsm_bts *bts, rts_time *g_time, tOCT_UINT8 *si);

#define mSWAP32(f_ulValue)                          \
    (((((unsigned)f_ulValue) & 0xff000000) >> 24) | \
     ((((unsigned)f_ulValue) & 0x00ff0000) >> 8) |  \
     ((((unsigned)f_ulValue) & 0x0000ff00) << 8) |  \
     ((((unsigned)f_ulValue) & 0x000000ff) << 24))

/***************************  TYPE DEFINITIONS  ******************************/

/*
 * This enumeration defines the modes of operation.
 */
#define tOCTSAMPLES_OPERATION_MODE_ENUM tOCT_UINT32
#define cOCTSAMPLES_OPERATION_MODE_ENUM_READ 0
#define cOCTSAMPLES_OPERATION_MODE_ENUM_WRITE 1

/*
 * This structure contains application configuration data.
 */
typedef struct tOCTSAMPLES_APP_CFG_TAG
{
    /* Packet API's physical network interfaces for commands and responses. */
    tOCT_UINT8 abyProcessorCtrlMacAddr[6]; /* Processor control port's MAC address. */
    tOCT_UINT8 abyTargetCtrlMacAddr[6];    /* Target control port's MAC address (port 0 or 1). */
} tOCTSAMPLES_APP_CFG, *tPOCTSAMPLES_APP_CFG, **tPPOCTSAMPLES_APP_CFG;

/*
 * This structure contains application context data.
 */
typedef struct tOCTSAMPLES_APP_CTX_TAG
{
    tOCTSAMPLES_PKT_API_INFO PktApiInfo; /* Packet API information. */
    char *pszLocalFileName;              /* Local file name. */
    char *pszRemoteFileName;             /* Remote file name. */

} tOCTSAMPLES_APP_CTX, *tPOCTSAMPLES_APP_CTX, **tPPOCTSAMPLES_APP_CTX;

/***************************  CONSTANTS  *************************************/

/*
 * Activity indicators.
 */
const tOCT_UINT8 g_abyActivityIndicators[] = {'-', '\\', '|', '/'};

/***************************  GLOBAL VARIABLES  ******************************/

/*
 * Application configuration data.
 *
 * Note: The values used in this sample are provided as a guide for supplying
 *       the real values in the actual system.
 */
tOCTSAMPLES_APP_CFG g_AppCfg =
    {
        /* Packet API's physical network interfaces for commands and responses. */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Processor control port's MAC address. */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  /* Target control port's MAC address (port 0 or 1). */
};

#define QUARTER_DB_TO_Q7(x) ((x) * 128)
#define Q7_TO_QUARTER_DB(x) ((x) / 128)

#define QUARTER_DB_TO_Q9(x) ((x) * 512)
#define Q9_TO_QUARTER_DB(x) ((x) / 512)
/*
 * Application context data.
 */
tOCTSAMPLES_APP_CTX g_AppCtxRus;
tOCTSAMPLES_APP_CTX_GSM g_AppCtxGsm;

#define mMAX(a, b) ((a > b) ? a : b)

#define cRUS_WNM_MAX_NUM_BTS_BY_DB mMAX(mMAX(cOCTVC1_RUS_WNM_MAX_NUM_2GBTS_BY_DB, cOCTVC1_RUS_WNM_MAX_NUM_3GBTS_BY_DB), cOCTVC1_RUS_WNM_MAX_NUM_4GBTS_BY_DB)

typedef struct
{
    tOCT_UINT32 ulNumCell;
    tOCTVC1_SUB_OBJECT_ID aCellId[cRUS_WNM_MAX_NUM_BTS_BY_DB];
} tBTSLIST;

typedef struct
{
    tOCTVC1_RADIO_STANDARD_ENUM ulWaveForm;
    tOCTVC1_RADIO_STANDARD_ENUM ulBand;
    tOCT_UINT32 ulNumRssiChannel;
    tOCTVC1_RUS_WNM_RSSI_CHANNEL aChannelEntry[cOCTVC1_RUS_WNM_RSSI_MAX_NUM_RSSI_CHANNEL_ENTRY];
} tWNM_RSSI_BAND_CHANNEL;

typedef struct
{
    tOCTVC1_HANDLE_OBJECT hWnm;
    tOCTVC1_RUS_WNM_SEARCH_PARMS PrmSetup; // Setup parameters of the monitor
    tOCTVC1_RUS_WNM_SEARCH_STATS tStatus;  // Status of the monitor
    tBTSLIST UmtsBtsList;
    tBTSLIST GsmBtsList;
    tBTSLIST LteBtsList;
    tBTSLIST CdmaBtsList;
    tWNM_RSSI_BAND_CHANNEL RssiBandChannel;
} tWNM_CTX;

/* UMTS Decode */
unsigned int gUarfcn;
unsigned int gOperatorCount;
unsigned int gCellCount;
tWNM_CTX tWnm = {0};
// Setup
//unsigned char bContinuousMonitoringEnable = 0;
short bContinuousMonitoringEnable = 0;
fxL3gListenModeSystemInfoInd g3gListenModeSysInfo;
fxL3gBandType convertToFxLFromOctBand(tOCT_UINT32 band);
extern L1C_CONFIG_DB g_L1CConfigDB;
extern AutoConfigCfg gAutoConfigCfg;
extern UmtsConfigControlBlock gUmtsConfig;
tOCT_UINT32 gPrevCellCount = 0;
AutoCfgScanResult_t *pAutoCfgScanResult = NULL;

/* Bass Feature */
#define cMAX_NUM_SB_BTS (256)
typedef struct
{
    tOCTVC1_RADIO_STANDARD_ENUM ulWaveForm;
    tOCT_UINT32 ulBand;
    tOCT_UINT32 ulRfcn;
    tOCT_INT32 lSNR;
    tOCT_UINT16 usPCI;
} tBTS;

typedef struct
{
    tOCT_UINT32 ulNumBts;
    tBTS atList[cMAX_NUM_SB_BTS];
} tSB_BTSLIST;

typedef struct
{
    tOCTVC1_HANDLE_OBJECT hBass;
    tOCTVC1_RUS_BASS_ATTACH_CONFIG tBassSetup;
    tOCTVC1_RUS_BASS_ATTACH_STATS tBassStatus;
    tSB_BTSLIST tBlindBassGsmBtsList;
} tBASS_CTX;

tBASS_CTX tBass = {0};

/*
 * This structure contains application context data.
 */
typedef struct tOCTSAMPLES_APP_CTX_TAG_RF
{
    tOCTSAMPLES_PKT_API_INFO PktApiInfo; /* Packet API information. */
    tOCTVC1_HANDLE_OBJECT hEnodeb;
    tOCTVC1_LTE_CELL_STATE_ENUM ulCellState; // Cell status

    tOCT_UINT32 ulDuplexMode;
    tOCT_UINT32 ulCpType;
    tOCT_UINT32 ulBwId;
    tOCT_UINT32 ulDlEarfcn;
    tOCT_UINT32 ulRfRxGaindB; // in dB
    tOCT_INT32 ulRfTxAttndB;  // in 1/4 dB
    tOCT_UINT32 ulTddConfig;
    tOCT_UINT32 ulTddSpecialSfConfig;
    tOCT_UINT32 relativeD;
} tOCTSAMPLES_APP_CTX_RF, *tPOCTSAMPLES_APP_CTX_RF, **tPPOCTSAMPLES_APP_CTX_RF;

/*
 * Application context data.l
 */
tOCTSAMPLES_APP_CTX_RF g_AppCtxRf;

/***************************  PRIVATE FUNCTIONS  *****************************/

extern void rr_si_servcell_bcch_si_handle(const char *si_ptr, FILE *pFile, cellParams *params);
void configure4gWnmParameters(fxL4gListenModeConfigureBandsCmd *cmd);
extern void sendListenModeSysInfoInd(void);
extern void sendRssiScanResultInd(tWNM_RSSI_BAND_CHANNEL rssiDetails);
extern void send2gListenModeScanCompInd(void);
extern void send3gListenModeScanCompInd(unsigned int result);
extern void send4gListenModeScanCompInd(void);
int checkPlmnFound(void);
extern sint8 createAndStartListenModeThread();
extern void autoConfigureUmtsIdCatcherParameters(void);
extern void autoBootL1Image(void);
extern fxLResult createAndStart3gIdCatcherThread(void);
#ifdef CELL_ANTENNA_CHANGE
static int currentBand = 0xFF;
extern void configureCellAntennaPA(char dspType, int band, char mode);
tOCTVC1_RUS_WNM_SEARCH_PARMS prmSetup = {0};
int bandIndex = -1;
#endif

extern fxLScanType scanMode;

#if 0
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ResetDevice

Description:    Resets the target device

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int ResetDevice(void)
{
    tOCTVC1_MAIN_MSG_TARGET_RESET_CMD  DeviceResetCmd;
    tOCTVC1_MAIN_MSG_TARGET_RESET_RSP  DeviceResetRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS  CmdExecuteParms;
    tOCT_UINT32                        ulResult;

    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_TARGET_RESET_CMD_DEF(&DeviceResetCmd);
    mOCTVC1_MAIN_MSG_TARGET_RESET_CMD_SWAP(&DeviceResetCmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd           = &DeviceResetCmd;
    CmdExecuteParms.pRsp           = &DeviceResetRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(DeviceResetRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_DEVICE_RESET_CID() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult) );
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    return 0;

ErrorHandling:
    return -1;
}

#endif

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       StartBassModule
 *
 * Description:    Start the Bass module cores
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int StartBassModule(void)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD StatsCmd;
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP StatsRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;

    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD Cmd;
    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP Rsp;
    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_DEF(&StatsCmd);
    StatsCmd.ulModuleId = cOCTVC1_RUS_APPLICATION_ID_SUB_MODULE_BASS;
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_SWAP(&StatsCmd);
    mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_DEF(&Cmd);
    Cmd.ulModuleId = cOCTVC1_RUS_APPLICATION_ID_SUB_MODULE_BASS;
    mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_SWAP(&Cmd);

    /*
     * Execute the stats command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &StatsCmd;
    CmdExecuteParms.pRsp = &StatsRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP_SWAP(&StatsRsp);
    if (StatsRsp.Stats.ulState == cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STARTED)
    {
        logPrint(LOG_DEBUG, "\nModule BASS already started\n");
        return 0;
    }
    else if (StatsRsp.Stats.ulState != cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STOPPED)
    {
        goto ErrorHandling;
    }
    else
    {
        /*
         * Execute the start command.
         */
        logPrint(LOG_DEBUG, "\nStarting BASS module\n");
        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &Cmd;
        CmdExecuteParms.pRsp = &Rsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            goto ErrorHandling;
        }
    }
    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       CheckBassObject
 *
 * Description:    Check and close all Bass components.
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int CheckBassObject(tOCTVC1_HANDLE_OBJECT *f_hBass)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_LIST_CMD ListCmd;
    tOCTVC1_RUS_MSG_BASS_LIST_RSP ListRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    *f_hBass = 0;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_LIST_CMD_DEF(&ListCmd);
    mOCTVC1_RUS_MSG_BASS_LIST_CMD_SWAP(&ListCmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &ListCmd;
    CmdExecuteParms.pRsp = &ListRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_LIST_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_GENERIC_RC_NO_MORE_OBJECT == ulResult)
        return cOCTVC1_RC_OK;

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_LIST_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        return ulResult;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_LIST_RSP_SWAP(&ListRsp);

    *f_hBass = ListRsp.ObjectList.aHandleObject[0];

    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       CheckBass
 *
 * Description:    Check Bass object on the target
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int CheckBass(void)
{
    tOCTVC1_HANDLE_OBJECT f_hBass;
    /* Verify if Bass is already open */
    if (CheckBassObject(&f_hBass) != 0)
        goto CriticalAbort;

    if (f_hBass != 0)
    {
        /* Close Bass and attached application */
        if (BassClose(f_hBass) != 0)
            goto CriticalAbort;
    }

    return (0);

CriticalAbort:
    return (-1);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassOpen
 *
 * Description:    Open Bass object on the target
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassOpen(tOCTVC1_HANDLE_OBJECT *f_hBass)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_OPEN_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_OPEN_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_OPEN_CMD_DEF(&Cmd);
    Cmd.ulPortIndex = 0;
    mOCTVC1_RUS_MSG_BASS_OPEN_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_OPEN_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_OPEN_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_OPEN_RSP_SWAP(&Rsp);

    *f_hBass = Rsp.hBass;

    OctOsalSleepMs(1000);
    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassSetup
 *
 * Description:    Setup the Bass operating mode.
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassSetup(tOCTVC1_HANDLE_OBJECT f_hBass, tOCTVC1_RUS_BASS_ATTACH_CONFIG *f_AttachConfig)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_CMD_DEF(&Cmd);
    Cmd.hBass = f_hBass;
    Cmd.AttachConfig = *f_AttachConfig;
    mOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_MODIFY_ATTACH_RSP_SWAP(&Rsp);
    OctOsalSleepMs(100);

    return ulResult;

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassStart
 *
 * Description:    Start the monitoring
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassStart(tOCTVC1_HANDLE_OBJECT f_hBass)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_START_ATTACH_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_START_ATTACH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_START_ATTACH_CMD_DEF(&Cmd);
    Cmd.hBass = f_hBass;
    mOCTVC1_RUS_MSG_BASS_START_ATTACH_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_START_ATTACH_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_MODIFY_START_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_START_ATTACH_RSP_SWAP(&Rsp);
    OctOsalSleepMs(100);

    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassGetStatus
 *
 * Description:    Start the monitoring
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassGetStatus(tOCTVC1_HANDLE_OBJECT f_hBass, tOCTVC1_RUS_BASS_ATTACH_STATS *f_pStatus)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_STATS_ATTACH_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_STATS_ATTACH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_STATS_ATTACH_CMD_DEF(&Cmd);
    Cmd.hBass = f_hBass;
    mOCTVC1_RUS_MSG_BASS_STATS_ATTACH_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_STATS_ATTACH_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_MODIFY_STATS_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_STATS_ATTACH_RSP_SWAP(&Rsp);
    *f_pStatus = Rsp.Stats;

    return ulResult;
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassStop
 *
 * Description:    Start the monitoring
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassStop(tOCTVC1_HANDLE_OBJECT f_hBass)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_STOP_ATTACH_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_STOP_ATTACH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_STOP_ATTACH_CMD_DEF(&Cmd);
    Cmd.hBass = f_hBass;
    mOCTVC1_RUS_MSG_BASS_STOP_ATTACH_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_STOP_ATTACH_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_MODIFY_STOP_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_STOP_ATTACH_RSP_SWAP(&Rsp);

    return ulResult;
    OctOsalSleepMs(100);

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BassClose
 *
 * Description:    Check and close the Bass components.
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int BassClose(tOCTVC1_HANDLE_OBJECT f_hBass)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_BASS_CLOSE_CMD Cmd;
    tOCTVC1_RUS_MSG_BASS_CLOSE_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_BASS_CLOSE_CMD_DEF(&Cmd);
    Cmd.hBass = f_hBass;
    mOCTVC1_RUS_MSG_BASS_CLOSE_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_BASS_CLOSE_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_BASS_CLOSE_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }
    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_BASS_CLOSE_RSP_SWAP(&Rsp);

    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ReadFileFromTarget

Description:    Reads a file from the target's file system.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int ReadFileFromTarget(char *f_pszLocalFileName, char *f_pszRemoteFileName, unsigned f_bVerbose)
{
    FILE *pLocalFileStream = NULL;
    tOCTVC1_MAIN_MSG_FILE_OPEN_CMD FileOpenCmd;
    tOCTVC1_MAIN_MSG_FILE_OPEN_RSP FileOpenRsp;
    tOCTVC1_MAIN_MSG_FILE_READ_CMD FileReadCmd;
    tOCTVC1_MAIN_MSG_FILE_READ_RSP FileReadRsp;
    tOCTVC1_MAIN_MSG_FILE_CLOSE_CMD FileCloseCmd;
    tOCTVC1_MAIN_MSG_FILE_CLOSE_RSP FileCloseRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCT_UINT32 i = 0;

    FileOpenRsp.hFile = 0x0;

    /*
     * Open the local file.
     */
    pLocalFileStream = fopen(f_pszLocalFileName, "wb");
    if (NULL == pLocalFileStream)
    {
        fprintf(stderr, "Error: can't open %s, errno = %s\n", f_pszLocalFileName, strerror(errno));
        goto ErrorHandling;
    }

    /*
     * Open the remote file.
     */
    mOCTVC1_MAIN_MSG_FILE_OPEN_CMD_DEF(&FileOpenCmd);
    strncpy((char *)FileOpenCmd.szFileName, f_pszRemoteFileName, cOCTVC1_MAIN_FILE_NAME_MAX_LENGTH);
    FileOpenCmd.ulAccessMode = cOCTVC1_MAIN_FILE_OPEN_MODE_MASK_READ;
    mOCTVC1_MAIN_MSG_FILE_OPEN_CMD_SWAP(&FileOpenCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &FileOpenCmd;
    CmdExecuteParms.pRsp = &FileOpenRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(FileOpenRsp);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_FILE_OPEN_CID failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the file open command response.
     */
    mOCTVC1_MAIN_MSG_FILE_OPEN_RSP_SWAP(&FileOpenRsp);

    if (f_bVerbose)
        DEBUG4(("\tStarting read of remote file %s to local file %s\n", f_pszRemoteFileName, f_pszLocalFileName));

    /*
     * Initialize the read command.
     */
    mOCTVC1_MAIN_MSG_FILE_READ_CMD_DEF(&FileReadCmd);
    FileReadCmd.hFile = FileOpenRsp.hFile;
    FileReadCmd.ulMaxNumByteToRead = sizeof(FileReadRsp.abyData);
    mOCTVC1_MAIN_MSG_FILE_READ_CMD_SWAP(&FileReadCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &FileReadCmd;
    CmdExecuteParms.pRsp = &FileReadRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(FileReadRsp);

    /*
     * Perform the read operation.
     */
    do
    {
        /*
         * Display and update the activity indicator.
         */
        if (f_bVerbose)
        {
            DEBUG3(("\tReading %c\r", g_abyActivityIndicators[i++]));
            fflush(stdout);
            i %= sizeof(g_abyActivityIndicators);
        }

        /*
         * Read data from the remote file.
         */
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_FILE_READ_CID failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
            goto ErrorHandling;
        }

        /*
         * Swap the file read response.
         */
        mOCTVC1_MAIN_MSG_FILE_READ_RSP_SWAP(&FileReadRsp);

        /*
         * Write data that have been read to the local file.
         */
        if (0 < FileReadRsp.ulNumByteRead)
        {
            tOCT_UINT32 ulNumByteWritten = fwrite(FileReadRsp.abyData,
                                                  1,
                                                  FileReadRsp.ulNumByteRead,
                                                  pLocalFileStream);
            if (ulNumByteWritten != FileReadRsp.ulNumByteRead)
            {
                fprintf(stderr, "\n\nError: failed to write to the local file\n");
                goto ErrorHandling;
            }
        }
    } while (0 < FileReadRsp.ulNumByteRead);

    if (f_bVerbose)
    {
        DEBUG3(("\n\tRead operation completed successfully\n"));
    }

    /*
     * Close the remote file.
     */
    mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_DEF(&FileCloseCmd);
    FileCloseCmd.hFile = FileOpenRsp.hFile;
    mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_SWAP(&FileCloseCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &FileCloseCmd;
    CmdExecuteParms.pRsp = &FileCloseRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(FileCloseRsp);
    OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_FILE_CLOSE_CID failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Close the local file.
     */
    fclose(pLocalFileStream);
    pLocalFileStream = NULL;

    return 0;

ErrorHandling:
    /*
     * Close the remote file.
     */
    if (FileOpenRsp.hFile)
    {
        mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_DEF(&FileCloseCmd);
        FileCloseCmd.hFile = FileOpenRsp.hFile;
        mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_SWAP(&FileCloseCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &FileCloseCmd;
        CmdExecuteParms.pRsp = &FileCloseRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(FileCloseRsp);
        OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    }

    /*
     * Close the local file.
     */
    if (pLocalFileStream)
    {
        fclose(pLocalFileStream);
        pLocalFileStream = NULL;
    }

    return -1;
}

#if 0
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WriteFileToTarget

Description:    Writes a file to the target's file system.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int WriteFileToTarget(char *f_pszRemoteFileName,char *f_pszLocalFileName, unsigned f_bVerbose)
{
    FILE                              *pLocalFileStream = NULL;
    tOCTVC1_MAIN_MSG_FILE_OPEN_CMD    FileOpenCmd;
    tOCTVC1_MAIN_MSG_FILE_OPEN_RSP    FileOpenRsp;
    tOCTVC1_MAIN_MSG_FILE_WRITE_CMD   FileWriteCmd;
    tOCTVC1_MAIN_MSG_FILE_WRITE_RSP   FileWriteRsp;
    tOCTVC1_MAIN_MSG_FILE_CLOSE_CMD   FileCloseCmd;
    tOCTVC1_MAIN_MSG_FILE_CLOSE_RSP   FileCloseRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32                       ulResult;
    tOCT_UINT32                       i = 0;

    FileOpenRsp.hFile = 0x0;

    /*
     * Open the local file.
     */
    pLocalFileStream = fopen(f_pszLocalFileName, "rb");
    if (NULL == pLocalFileStream)
    {
        fprintf(stderr, "Error: can't open %s\n", f_pszLocalFileName);
        goto ErrorHandling;
    }

    /*
     * Open the remote file.
     */
    mOCTVC1_MAIN_MSG_FILE_OPEN_CMD_DEF(&FileOpenCmd);
    strncpy((char *)FileOpenCmd.szFileName, f_pszRemoteFileName, cOCTVC1_MAIN_FILE_NAME_MAX_LENGTH - 1);
    FileOpenCmd.ulAccessMode = cOCTVC1_MAIN_FILE_OPEN_MODE_MASK_WRITE;
    mOCTVC1_MAIN_MSG_FILE_OPEN_CMD_SWAP(&FileOpenCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd           = &FileOpenCmd;
    CmdExecuteParms.pRsp           = &FileOpenRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(FileOpenRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_FILE_OPEN_CID failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult) );
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the file open command response.
     */
    mOCTVC1_MAIN_MSG_FILE_OPEN_RSP_SWAP(&FileOpenRsp);

    if(f_bVerbose)
    {
        DEBUG3(("\tStarting write of remote file %s from local file %s\n", f_pszRemoteFileName, f_pszLocalFileName));
    }

    /*
     * Initialize the write command.
     */
    mOCTVC1_MAIN_MSG_FILE_WRITE_CMD_DEF(&FileWriteCmd);
    FileWriteCmd.hFile = FileOpenRsp.hFile;
    mOCTVC1_MAIN_MSG_FILE_WRITE_CMD_SWAP(&FileWriteCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd           = &FileWriteCmd;
    CmdExecuteParms.pRsp           = &FileWriteRsp;
    CmdExecuteParms.ulCmdTimeoutMs = 2000000; /* Write commands can take a while; make sure we don't timeout. */
    CmdExecuteParms.ulMaxRspLength = sizeof(FileWriteRsp);

    /*
     * Perform the write operation.
     */
    while (!feof(pLocalFileStream))
    {
        /*
         * Display and update the activity indicator.
         */
        if(f_bVerbose)
        {
            DEBUG3(("\tWriting %c\r", g_abyActivityIndicators[i++]));
            fflush(stdout);
            i %= sizeof(g_abyActivityIndicators);
        }

        /*
         * Read data from the local file.
         */
        FileWriteCmd.ulNumByteToWrite = fread(FileWriteCmd.abyData,
                                               1,
                                               sizeof(FileWriteCmd.abyData),
                                               pLocalFileStream);
        if (0 != ferror(pLocalFileStream))
        {
            fprintf(stderr, "\n\nError: failed to read from the local file\n");
            goto ErrorHandling;
        }
        if (0 == FileWriteCmd.ulNumByteToWrite)
        {
            continue;
        }

        /*
         * Write data to the remote file.
         */
        FileWriteCmd.ulNumByteToWrite = mOCTVC1_SWAP32_IF_LE(FileWriteCmd.ulNumByteToWrite);

        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            goto ErrorHandling;
        }

        if (FileWriteRsp.ulNumByteWritten != FileWriteCmd.ulNumByteToWrite)
        {
             fprintf(stderr, "\n\nError: failed to write to the remote file\n");
             goto ErrorHandling;
        }
    }

    if(f_bVerbose)
    {
        DEBUG3(("\n\tWrite operation completed successfully\n"));
    }

    /*
     * Close the remote file.
     */
    mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_DEF(&FileCloseCmd);
    FileCloseCmd.hFile = FileOpenRsp.hFile;
    mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_SWAP(&FileCloseCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd           = &FileCloseCmd;
    CmdExecuteParms.pRsp           = &FileCloseRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(FileCloseRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_FILE_CLOSE_CID failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult) );
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Close the local file.
     */
    fclose(pLocalFileStream);
    pLocalFileStream = NULL;

    return 0;

ErrorHandling:
    /*
     * Close the remote file.
     */
    if (FileOpenRsp.hFile)
    {
        mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_DEF(&FileCloseCmd);
        FileCloseCmd.hFile = FileOpenRsp.hFile;
        mOCTVC1_MAIN_MSG_FILE_CLOSE_CMD_SWAP(&FileCloseCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd           = &FileCloseCmd;
        CmdExecuteParms.pRsp           = &FileCloseRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(FileCloseRsp);
        OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    }

    /*
     * Close the local file.
     */
    if (pLocalFileStream)
    {
        fclose(pLocalFileStream);
        pLocalFileStream = NULL;
    }

    return -1;
}
#endif

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       BoardDisplayInfo

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int BoardDisplayInfo(unsigned f_bDumpUserInfo)
{
    tOCT_UINT32 ulResult;
    {
        tOCTVC1_HW_MSG_PCB_INFO_CMD Cmd;
        tOCTVC1_HW_MSG_PCB_INFO_RSP Rsp;
        tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
        /*
         * Prepare command data.
         */
        mOCTVC1_HW_MSG_PCB_INFO_CMD_DEF(&Cmd);
        mOCTVC1_HW_MSG_PCB_INFO_CMD_SWAP(&Cmd);
        /*
         * Execute the command.
         */
        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &Cmd;
        CmdExecuteParms.pRsp = &Rsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_PCB_INFO_RSP);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_HW_MSG_PCB_INFO_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            goto ErrorHandling;
        }

        /*
         * Swap the command response.
         */
        mOCTVC1_HW_MSG_PCB_INFO_RSP_SWAP(&Rsp);

        // logPrint(LOG_DEBUG,"===========================================\n");
        // logPrint(LOG_DEBUG," Board Type  : %s \n",Rsp.szName);
        //  logPrint(LOG_DEBUG," SN          : %s \n",Rsp.szSerial);
        //  logPrint(LOG_DEBUG," ulDeviceId  : %d \n",Rsp.ulDeviceId);
    }
    if (f_bDumpUserInfo != 0)
    {
        tOCTVC1_MAIN_MSG_TARGET_INFO_CMD Cmd;
        tOCTVC1_MAIN_MSG_TARGET_INFO_RSP Rsp;
        tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
        /*
         * Prepare command data.
         */
        mOCTVC1_MAIN_MSG_TARGET_INFO_CMD_DEF(&Cmd);
        mOCTVC1_MAIN_MSG_TARGET_INFO_CMD_SWAP(&Cmd);
        /*
         * Execute the command.
         */
        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &Cmd;
        CmdExecuteParms.pRsp = &Rsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_TARGET_INFO_RSP);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_TARGET_INFO_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            goto ErrorHandling;
        }

        /*
         * Swap the command response.
         */
        mOCTVC1_MAIN_MSG_TARGET_INFO_RSP_SWAP(&Rsp);
        printf("==Exhaustive info\n");
        printf("abyTargetInfo\n%s\n", Rsp.abyTargetInfo);
    }
    // logPrint(LOG_DEBUG,"===========================================\n\n\n");

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       CheckClkMgrStatus

Description:    Retreive TX/RX samples from thr RFFI

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int CheckClkMgrStatus(tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP *f_pClkSyncStatus, tPOCTVC1_PKT_API_SESS pSess)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_CMD Cmd;
    tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_CMD_DEF(&Cmd);
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(pSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP_SWAP(&Rsp);
    *f_pClkSyncStatus = Rsp;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       StartWnmModule
 *
 * Description:    Start the Wnm module cores
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int StartWnmModule(void)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD StatsCmd;
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP StatsRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;

    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD Cmd;
    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP Rsp;
    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_DEF(&StatsCmd);
    StatsCmd.ulModuleId = cOCTVC1_RUS_APPLICATION_ID_SUB_MODULE_WNM;
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_SWAP(&StatsCmd);
    mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_DEF(&Cmd);
    Cmd.ulModuleId = cOCTVC1_RUS_APPLICATION_ID_SUB_MODULE_WNM;
    mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_SWAP(&Cmd);

    /*
     * Execute the stats command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &StatsCmd;
    CmdExecuteParms.pRsp = &StatsRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP_SWAP(&StatsRsp);
    if (StatsRsp.Stats.ulState == cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STARTED)
    {
        return 0;
    }
    else if (StatsRsp.Stats.ulState != cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STOPPED)
    {
        goto ErrorHandling;
    }
    else
    {
        /*
         * Execute the start command.
         */
        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &Cmd;
        CmdExecuteParms.pRsp = &Rsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            goto ErrorHandling;
        }
    }
    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       InitApplication

Description:    Performs initialization tasks required by the application.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int InitApplication(void)
{
    tOCT_UINT32 ulResult;

    /*
     * Open a transport packet API session.
     */
    memcpy(g_AppCfg.abyProcessorCtrlMacAddr, g_L1CConfigDB.l23SrcMacAddr, 6);
    memcpy(g_AppCfg.abyTargetCtrlMacAddr, g_L1CConfigDB.l1DstMacAddr, 6);
    g_AppCtxRus.PktApiInfo.pPktApiCnct = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiSess = NULL;

    ulResult = OctSamplesOpenPktApiSession(&g_AppCtxRus.PktApiInfo, // &gPktApiInfoForBass,
                                           g_AppCfg.abyProcessorCtrlMacAddr,
                                           g_AppCfg.abyTargetCtrlMacAddr);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: OctSamplesOpenPktApiSession() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        return -1;
    }

    /*
     * Start the wnm module
     */
    ulResult = StartWnmModule();
    if (cOCTVC1_RC_OK != ulResult)
    {
        return -1;
    }

    /* Initialize Uarfcn & Operator Count */
    gUarfcn = 0;
    gOperatorCount = 0;
    gCellCount = 0;
    gPrevCellCount = 0;
    initializeSystemInformationInd();

    return 0;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       GetClockSyncManagerInfo

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int GetClockSyncManagerInfo(tOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM *f_ulClkSourceRef)
{

    tOCT_UINT32 ulResult;
    tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_CMD Cmd;
    tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_CMD_DEF(&Cmd);
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_GENERIC_RC_NO_MORE_OBJECT == ulResult)
        return cOCTVC1_RC_OK;

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        return ulResult;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_INFO_RSP_SWAP(&Rsp);

    *f_ulClkSourceRef = Rsp.ulClkSourceRef;

    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnableEvents/DisableEvents

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int SessionEventModify(tOCT_BOOL32 f_bEnable)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD Cmd;
    tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_RSP Rsp;

    mOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD_DEF(&Cmd)
        Cmd.ulEvtActiveFlag = f_bEnable; // Enable/Disable Event on a current session
    mOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD_SWAP(&Cmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD() failed, rc = 0x%08x\n", ulResult);
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_RSP_SWAP(&Rsp);

    return 0;

ErrorHandling:
    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EventHandler

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int EventHandler(void *f_Payload, tOCT_UINT32 *f_PayloadSize, tOCT_UINT32 f_ulMaxPayloadLength)
{

    tOCTVC1_PKT_API_INST_RECV_PARMS PktApiInstRecvParms;
    tOCT_UINT32 ulResult = 0;

    mOCTVC1_PKT_API_INST_RECV_PARMS_DEF(&PktApiInstRecvParms);
    PktApiInstRecvParms.PktRcvMask = cOCTVC1_PKT_API_RCV_MASK_EVENT;
    PktApiInstRecvParms.pPayload = f_Payload;
    PktApiInstRecvParms.ulMaxPayloadLength = f_ulMaxPayloadLength;
    PktApiInstRecvParms.ulTimeoutMs = 2000;

    ulResult = OctVc1PktApiInstRecv(g_AppCtxRus.PktApiInfo.pPktApiInst, &PktApiInstRecvParms);
    if (cOCTVC1_PKT_API_RC_OK == ulResult)
    {
        if (PktApiInstRecvParms.ulRcvPayloadLength < f_ulMaxPayloadLength)
        {
            memcpy(f_Payload, PktApiInstRecvParms.pPayload, PktApiInstRecvParms.ulRcvPayloadLength);
            *f_PayloadSize = PktApiInstRecvParms.ulRcvPayloadLength;
            return (0);
        }
        else
        {
            return (0);
        }
    }
    else
    {
        return (ulResult);
    }

    return 0;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ExitApplication

Description:    Frees any resources used by the application.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static void ExitApplication(void)
{
    tOCT_UINT32 ulResult;

    /*
     * Close the packet API session.
     */
    ulResult = OctSamplesClosePktApiSession(&g_AppCtxRus.PktApiInfo);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: OctSamplesClosePktApiSession() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
    }

    g_AppCtxRus.PktApiInfo.pPktApiCnct = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiSess = NULL;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ApplicationDisplayInfo

Description:    Display information

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int ApplicationDisplayInfo(void)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_MAIN_MSG_APPLICATION_INFO_CMD Cmd;
    tOCTVC1_MAIN_MSG_APPLICATION_INFO_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;

    tOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_CMD Cmd2;
    tOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_RSP Rsp2;

    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_CMD_DEF(&Cmd);
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_CMD_SWAP(&Cmd);

    /*
     * Execute the Application info command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_INFO_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_INFO_CMD_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Execute the Application system info command.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_CMD_DEF(&Cmd2);
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_CMD_SWAP(&Cmd2);

    /*
     * Execute the Application info command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd2;
    CmdExecuteParms.pRsp = &Rsp2;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_CMD() failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));

        goto ErrorHandling;
    }

    /*
     * Swap the commands responses.
     */
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_RSP_SWAP(&Rsp);
    mOCTVC1_MAIN_MSG_APPLICATION_INFO_SYSTEM_RSP_SWAP(&Rsp2);

    printf("===========================================\n");
    printf(" Application : %s\n", "2G/3G/4G SLS");
    printf(" Description : %s\n", "2G/3G/4G Scanner Software");
    printf(" Version     : V_%.3f\n", FXL_SOCKETINTERFACEAPI_VERSION);
    // printf(" System Plateform        : %s\n",Rsp2.szPlatform);
    // printf(" System Version          : %s\n",Rsp2.szVersion);
    printf("===========================================\n\n\n");
    return ulResult;

ErrorHandling:
    return ulResult;
}

/*
 * Statitics display utilities
 */
char *WnmGetStateString(tOCTVC1_RUS_WNM_SEARCH_STATE_ENUM f_ulState)
{
    static char szState[6][32] = {{"UNINITIALZED"}, {"INITIALZED"}, {"SEARCHING"}, {"DONE"}, {"PAUSE"}, {"ABORT_ERROR"}};
    return (&szState[f_ulState][0]);
}
char *WnmGetSearchDiagCodeString(tOCTVC1_RUS_WNM_SEARCH_DIAG_ENUM f_ulDiagCode)
{
    static char szSearchDiag[8][32] = {{"Success"}, {"CELL DB FULL"}, {"DECODER TIMEOUT"}, {"RFFI TIMEOUT"}, {"DECODER ERROR"}, {"CELL DB ACCESS ERROR"}, {"RSSI CHX DB FULL"}, {"RSSI CHX DB ACCESS ERROR"}};
    return (&szSearchDiag[f_ulDiagCode][0]);
}
char *WnmGetWaveFormString(tOCTVC1_RADIO_STANDARD_ENUM f_ulWaveform)
{
    static char szWaveForm[5][32] = {{"GSM"}, {"UMTS"}, {"LTE"}, {"CDMA2000"}, {"INVALID"}};
    static char szInvalid[32] = {"INVALID"};
    if (f_ulWaveform == cOCTVC1_RADIO_STANDARD_ENUM_INVALID)
        return (&szInvalid[0]);

    return (&szWaveForm[f_ulWaveform][0]);
}

char *WnmGetBandString(tOCTVC1_RADIO_STANDARD_ENUM f_ulWaveform, tOCT_UINT32 f_Band)
{
    static char szGsmBand[cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_LAST][32] = {
        "Gsm 380",
        "Gsm 410",
        "Gsm 450",
        "Gsm 480",
        "Gsm 710",
        "Gsm 750",
        "Gsm 810",
        "Gsm 850",
        "Gsm E 900",
        "Gsm P 900",
        "Gsm R 900",
        "Gsm DCS 1800",
        "Gsm PCS 1900",
    };

    static char szUmtsBand[cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_LAST][38] = {
        "Band1",
        "Band2",
        "Band3",
        "Band4",
        "Band5",
        "Band6",
        "Band7",
        "Band8",
        "Band9",
        "Band10",
        "Band11",
        "Band12",
        "Band13",
        "Band14",
        "Band15",
        "Band16",
        "Band17",
        "Band18",
        "Band19",
        "Band20",
        "Band21",
        "Band22",
        "Band23",
        "Band24",
        "Band25",
        "Band26",
        "Additional band 2",
        "Additional band 4",
        "Additional band 5",
        "Additional band 6",
        "Additional band 7",
        "Additional band 10",
        "Additional band 12",
        "Additional band 13",
        "Additional band 14",
        "Additional band 19",
        "Additional band 25",
        "Additional band 26",
    };

    static char szLteBand[cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_LAST][63] = {
        "Band1",
        "Band2",
        "Band3",
        "Band4",
        "Band5",
        "Band6",
        "Band7",
        "Band8",
        "Band9",
        "Band10",
        "Band11",
        "Band12",
        "Band13",
        "Band14",
        "Band15",
        "Band16",
        "Band17",
        "Band18",
        "Band19",
        "Band20",
        "Band21",
        "Band22",
        "Band23",
        "Band24",
        "Band25",
        "Band26",
        "Band27",
        "Band28",
        "Band29",
        "Band30",
        "Band31",
        "Band32",
        "Band33",
        "Band34",
        "Band35",
        "Band36",
        "Band37",
        "Band38",
        "Band39",
        "Band40",
        "Band41",
        "Band42",
        "Band43",
        "Band44",
        "Band45",
        "Band46",
        "Band47",
        "Band48",
        "Band50",
        "Band51",
        "Band65",
        "Band66",
        "Band67",
        "Band68",
        "Band69",
        "Band70",
        "Band71",
        "Band72",
        "Band74",
        "Band75",
        "Band76",
        "Band252",
        "Band255",
    };

    static char szInvalid[32] = {"INVALID"};

    if (f_ulWaveform == cOCTVC1_RADIO_STANDARD_ENUM_INVALID)
        return (&szInvalid[0]);

    if (f_Band == cOCTVC1_RADIO_STANDARD_ENUM_INVALID)
        return (&szInvalid[0]);

    if (f_ulWaveform == cOCTVC1_RADIO_STANDARD_ENUM_GSM)
    {
        return (&szGsmBand[f_Band][0]);
    }
    else if (f_ulWaveform == cOCTVC1_RADIO_STANDARD_ENUM_UMTS)
    {
        return (&szUmtsBand[f_Band][0]);
    }
    else if (f_ulWaveform == cOCTVC1_RADIO_STANDARD_ENUM_LTE)
    {
        return (&szLteBand[f_Band][0]);
    }
    else
    {
        return (&szGsmBand[0][0]);
    }
}

char *WnmGetGsmCodestring(tOCTVC1_RUS_WNM_GSM_DECODER_DIAG_ENUM f_ulDecoderDiagnosticCode)
{
    static char szGsmCode[5][32] = {
        "Success",
        "Inner modem out of sync",
        "Outer modem Timeout",
        "Outer modem out of sync",
        "Filtered"};
    return (&szGsmCode[f_ulDecoderDiagnosticCode][0]);
}

char *WnmGetUmtsCodestring(tOCTVC1_RUS_WNM_UMTS_DECODER_DIAG_ENUM f_ulDecoderDiagnosticCode)
{
    static char szUmtsCode[6][32] = {
        "Success",
        "Inner modem Timeout",
        "Inner modem out of sync",
        "Outer modem Timeout",
        "Outer modem out of sync",
        "Filtered"};
    return (&szUmtsCode[f_ulDecoderDiagnosticCode][0]);
}

char *WnmGetLteCodestring(tOCTVC1_RUS_WNM_LTE_DECODER_DIAG_ENUM f_ulDecoderDiagnosticCode)
{
    static char szLteCode[13][32] = {
        "Success",
        "Inner modem Timeout",
        "Inner modem out of sync",
        "Outer modem Timeout",
        "Outer modem out of sync",
        "Decoder format not supported",
        "Realtime Error OMS0",
        "Realtime Error OMS1",
        "Realtime Error IMS0",
        "Realtime Error IMS1",
        "Realtime Error IMS2",
        "Realtime Error IMS3",
        "Filtered"};
    return (&szLteCode[f_ulDecoderDiagnosticCode][0]);
}

char *WnmGetLteBandwidthString(tOCTVC1_RADIO_STANDARD_BANDWIDTH_ENUM f_ulBandwidth)
{
    static char szBandwidth[cOCTVC1_RADIO_STANDARD_BANDWIDTH_ENUM_LAST][32] = {{"1.4MHz"}, {"3Mhz"}, {"5Mhz"}, {"10Mhz"}, {"15Mhz"}, {"20Mhz"}};

    return (&szBandwidth[f_ulBandwidth][0]);
}

char *WnmGetLteWaveModeString(tOCTVC1_RUS_WNM_LTE_MODE_ENUM f_ulWaveMode)
{
    static char szWaveMode[2][32] = {{"FDD"}, {"TDD"}};
    return (&szWaveMode[f_ulWaveMode][0]);
}

char *WnmGetLtePhichDurString(tOCTVC1_RUS_WNM_LTE_PHICH_DURATION_ENUM f_ulPhichDur)
{
    static char szPhichDur[2][32] = {{"NORMAL"}, {"EXTENDED"}};
    return (&szPhichDur[f_ulPhichDur][0]);
}

char *WnmGetLtePhichGroupString(tOCTVC1_RUS_WNM_LTE_PHICH_GROUP_ENUM f_ulPhichGroup)
{
    static char szPhichGroup[4][32] = {{"1//6"}, {"1//2"}, {"1"}, {"2"}};
    return (&szPhichGroup[f_ulPhichGroup][0]);
}

char *WnmGetLteNumAntennaPortString(tOCTVC1_RUS_WNM_LTE_NUM_ANTENNA_ENUM f_ulNumAntenna)
{
    static char szNumAntenna[4][32] = {{"invalid"}, {"1"}, {"2"}, {"4"}};
    return (&szNumAntenna[f_ulNumAntenna][0]);
}

char *WnmGetLteCPModeString(tOCTVC1_RUS_WNM_LTE_CP_ENUM f_ulCPMode)
{
    static char szCPMode[3][32] = {{"invalid"}, {"NORMAL"}, {"EXTENDED"}};
    return (&szCPMode[f_ulCPMode][0]);
}

char *WnmGetRxGainModeString(tOCTVC1_RUS_WNM_RXMODE_ENUM f_ulRxGainMode)
{
    static char szRxGainMode[2][32] = {{"Manual"}, {"Automatic"}};
    return (&szRxGainMode[f_ulRxGainMode][0]);
}
#define ARRAY_LENGTH 2048
char szSibName[ARRAY_LENGTH];
char *WnmGetLteSibsPresentInMask(tOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK f_ulSibMask)
{
    memset(szSibName, '\0', sizeof(char) * ARRAY_LENGTH);

    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB2)
        strcat(szSibName, "2, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB3)
        strcat(szSibName, "3, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB4)
        strcat(szSibName, "4, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB5)
        strcat(szSibName, "5, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB6)
        strcat(szSibName, "6, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB7)
        strcat(szSibName, "7, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB8)
        strcat(szSibName, "8, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB10)
        strcat(szSibName, "10, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB11)
        strcat(szSibName, "11, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB12)
        strcat(szSibName, "12, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB13)
        strcat(szSibName, "13 ");

    return (&szSibName[0]);
}

char *WnmGetUmtsSibsPresentInMask(tOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK f_ulSibMask)
{
    memset(szSibName, '\0', sizeof(char) * ARRAY_LENGTH);

    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB1)
        strcat(szSibName, "1, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB2)
        strcat(szSibName, "2, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB3)
        strcat(szSibName, "3, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB4)
        strcat(szSibName, "4, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB5)
        strcat(szSibName, "5, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB5_BIS)
        strcat(szSibName, "5_BIS, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB6)
        strcat(szSibName, "6, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB7)
        strcat(szSibName, "7, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB11)
        strcat(szSibName, "11, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB12)
        strcat(szSibName, "12, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB13)
        strcat(szSibName, "13, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB13_1)
        strcat(szSibName, "13_1, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB13_2)
        strcat(szSibName, "13_2, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB13_3)
        strcat(szSibName, "13_3, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB13_4)
        strcat(szSibName, "13_4, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB14)
        strcat(szSibName, "14, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15)
        strcat(szSibName, "15, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15_1)
        strcat(szSibName, "15_1, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15_2)
        strcat(szSibName, "15_2, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15_3)
        strcat(szSibName, "15_3, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15_4)
        strcat(szSibName, "15_4, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB15_5)
        strcat(szSibName, "15_5, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB16)
        strcat(szSibName, "16, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB17)
        strcat(szSibName, "17, ");
    if (f_ulSibMask & cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB18)
        strcat(szSibName, "18, ");

    return (&szSibName[0]);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmOpen

Description:    Open Wnm object on the target

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int WnmOpen(tOCTVC1_HANDLE_OBJECT *f_hWnm)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_OPEN_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_OPEN_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_OPEN_CMD_DEF(&Cmd);
    Cmd.ulPortIndex = 0;
    mOCTVC1_RUS_MSG_WNM_OPEN_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_OPEN_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_OPEN_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_OPEN_RSP_SWAP(&Rsp);

    *f_hWnm = Rsp.hWnm;

    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmResume

Description:    resume the monitoring task

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmResume(tOCTVC1_HANDLE_OBJECT f_hWnm)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_MODIFY_RESUME_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_RESUME_SEARCH_RSP_SWAP(&Rsp);

    return ulResult;

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmSetup

Description:    Setup the Wnm operating mode.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmSetup(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_RUS_WNM_SEARCH_PARMS *f_SearchParms)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    Cmd.SearchParms = *f_SearchParms;
    mOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_MODIFY_SEARCH_RSP_SWAP(&Rsp);
    OctOsalSleepMs(100);

    return ulResult;
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmStart

Description:    Start the monitoring

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmStart(tOCTVC1_HANDLE_OBJECT f_hWnm)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_START_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_START_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_START_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_START_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_START_SEARCH_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_MODIFY_START_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_START_SEARCH_RSP_SWAP(&Rsp);
    OctOsalSleepMs(100);

    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmSetupInfo

Description:    Start the monitoring

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmSetupInfo(tOCTVC1_HANDLE_OBJECT f_hWnm)
{
    tOCTVC1_RUS_WNM_SEARCH_PARMS Info;
    tOCT_UINT32 ulResult;
    tOCTVC1_RUS_MSG_WNM_INFO_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_INFO_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_INFO_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_INFO_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_INFO_SEARCH_RSP);

    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_INFO_SEARCH_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_INFO_SEARCH_RSP_SWAP(&Rsp);
    Info = Rsp.SearchParms;
    OctOsalSleepMs(100);

    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmStop

Description:    Start the monitoring

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmStop(tOCTVC1_HANDLE_OBJECT f_hWnm)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_STOP_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STOP_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STOP_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_STOP_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STOP_SEARCH_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_MODIFY_STOP_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STOP_SEARCH_RSP_SWAP(&Rsp);

    return ulResult;
    OctOsalSleepMs(100);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmGetStatus

Description:    Start the monitoring

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmGetStatus(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_RUS_WNM_SEARCH_STATS *f_pStatus)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_STATS_SEARCH_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_SEARCH_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_SEARCH_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_STATS_SEARCH_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_SEARCH_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_SEARCH_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_SEARCH_RSP_SWAP(&Rsp);
    *f_pStatus = Rsp.Stats;

    return ulResult;
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmClose

Description:    Check and close the Wnm components.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmClose(tOCTVC1_HANDLE_OBJECT f_hWnm)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_CLOSE_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_CLOSE_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_CLOSE_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    mOCTVC1_RUS_MSG_WNM_CLOSE_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_CLOSE_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_CLOSE_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_CLOSE_RSP_SWAP(&Rsp);
    f_hWnm = 0;
    return ulResult;

ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       CheckWnmObject

Description:    Check and close all Wnm components.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int CheckWnmObject(tOCTVC1_HANDLE_OBJECT *f_hWnm)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_LIST_CMD ListCmd;
    tOCTVC1_RUS_MSG_WNM_LIST_RSP ListRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    *f_hWnm = 0;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_CMD_DEF(&ListCmd);
    mOCTVC1_RUS_MSG_WNM_LIST_CMD_SWAP(&ListCmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &ListCmd;
    CmdExecuteParms.pRsp = &ListRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_LIST_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_GENERIC_RC_NO_MORE_OBJECT == ulResult)
        return cOCTVC1_RC_OK;

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_LIST_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        return ulResult;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_RSP_SWAP(&ListRsp);

    *f_hWnm = ListRsp.ObjectList.aHandleObject[0];

    return ulResult;
}

int CheckWnm(void)
{
    tOCTVC1_HANDLE_OBJECT f_hWnm;
    // Verify if Wnm is already open
    if (CheckWnmObject(&f_hWnm) != 0)
        goto CriticalAbort;
    if (f_hWnm != 0)
    {
        // Close Wnm and attached application
        if (WnmClose(f_hWnm) != 0)
            goto CriticalAbort;
    }

    return (0);

CriticalAbort:
    return (-1);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmDisplayStatus

Description:    Start the monitoring

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmDisplayStatus(tOCTVC1_RUS_WNM_SEARCH_STATS *f_pStatus)
{
    static int prevRsltSuccessCnt = 0;
    printf("- 2G/3G/4G SCANNER STATUS -----------------------------------\n");
    printf("  Monitor status              : %s\n", WnmGetStateString(f_pStatus->ulState));
    printf("  Number of 2G cell found     : %d\n", f_pStatus->ul2GBaseStationCnt);
    printf("  Number of 3G cell found     : %d\n", f_pStatus->ul3GBaseStationCnt);
    printf("  Number of 4G cell found     : %d\n", f_pStatus->ul4GBaseStationCnt);
    printf("  Number of Rssi Band scanned : %d\n", f_pStatus->ulRssiBandCnt);
    printf("  Process current entry index : %d\n", f_pStatus->ulCurrentEntryIndex);
    printf("  Number of band to scan      : %d / %d\n", f_pStatus->ulNumBandScanned, f_pStatus->ulNumBandToScan);
    printf("   + Waveform                 : %s\n", WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm));
    printf("   + Band                     : %s\n", WnmGetBandString(f_pStatus->CurrentEntry.ulWaveForm, f_pStatus->CurrentEntry.ulBand));
    printf("   + Uarfcn                   : %u\n", f_pStatus->ulCurrentUarfcn);
    printf("   + Rssi threshold           : %6.3f dBm \n", f_pStatus->CurrentEntry.lRssiThreshold / 512.0);
    printf("   + Rsp threshold            : %6.3f dBm \n", f_pStatus->CurrentEntry.lRSPThresholddBm / 512.0);
    printf("   + SQ threshold             : %6.3f dBm \n", f_pStatus->CurrentEntry.lSQThresholddB / 512.0);
    // printf( "   + Receiver Gain mode       : %s \n", WnmGetRxGainModeString(f_pStatus->CurrentEntry.ulRxGainControlMode));
    // if(f_pStatus->CurrentEntry.ulRxGainControlMode == cOCTVC1_RUS_WNM_RXMODE_ENUM_MGC)
    //     printf( "     + Manual gain            : %6.3f dB \n", f_pStatus->CurrentEntry.lManualRxGaindB/512.0);
    printf("   + DiagCode Error           : %s\n", WnmGetSearchDiagCodeString(f_pStatus->ulSearcherDiagnosticCode));
    printf("-----------------------------------------------------------\n");

    if (((f_pStatus->ulNumBandScanned % 2 == 0) || (f_pStatus->ulNumBandScanned == f_pStatus->ulNumBandToScan)) && (prevRsltSuccessCnt != f_pStatus->ulNumBandScanned))
    {
        prevRsltSuccessCnt = f_pStatus->ulNumBandScanned;

        fxLlistenModeStatusInd *ind = (fxLlistenModeStatusInd *)msgBufSls;
        memset(msgBufSls, 0, sizeof(msgBufSls));

        if (!strcmp(WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm), "UMTS"))
        {
            printf("enter in to 3G");
            ind->hdr.rat = FXL_RAT_3G;
        }
        else if (!strcmp(WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm), "LTE"))
        {
            printf("enter in to 4G");
            ind->hdr.rat = FXL_RAT_4G;
        }
        else if (!strcmp(WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm), "GSM"))
        {
            printf("enter in to 2G");
            ind->hdr.rat = FXL_RAT_2G;
        }
        else
        {
            printf("enter in to invalid");
            return (0);
        }

        ind->hdr.msgLength = sizeof(fxLlistenModeStatusInd);
        ind->hdr.msgType = FXL_LISTEN_MODE_STATUS_IND;
        ind->numFreqScanned = f_pStatus->ulNumBandScanned;
        ind->totalFreqtoScan = f_pStatus->ulNumBandToScan;
        strcpy(ind->bandString, WnmGetBandString(f_pStatus->CurrentEntry.ulWaveForm, f_pStatus->CurrentEntry.ulBand));
        writeToExternalSocketInterface(msgBufSls, sizeof(fxLlistenModeStatusInd));
    }

    return (0);
}

#ifdef CELL_ANTENNA_CHANGE
void setCellAntennaPAForScanner()
{
    int bandType = 0;
    if (prmSetup.aEntryList[bandIndex].ulWaveForm == cOCTVC1_RADIO_STANDARD_ENUM_UMTS)
    {
        switch (prmSetup.aEntryList[bandIndex].ulBand)
        {
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_1:
            bandType = 1;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_2:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_2:
            bandType = 2;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_3:
            bandType = 3;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_4:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_4:
            bandType = 4;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_5:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_5:
            bandType = 5;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_6:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_6:
            bandType = 6;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_7:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_7:
            bandType = 7;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_8:
            bandType = 8;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_9:
            bandType = 9;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_10:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_10:
            bandType = 10;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_11:
            bandType = 11;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_12:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_12:
            bandType = 12;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_13:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_13:
            bandType = 13;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_14:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_14:
            bandType = 14;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_19:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_19:
            bandType = 19;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_20:
            bandType = 20;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_21:
            bandType = 21;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_22:
            bandType = 22;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_25:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_25:
            bandType = 25;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_26:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_26:
            bandType = 26;
            break;

        default:
            break;
        }
    }
    else if (prmSetup.aEntryList[bandIndex].ulWaveForm == cOCTVC1_RADIO_STANDARD_ENUM_LTE)
    {
        switch (prmSetup.aEntryList[bandIndex].ulBand)
        {
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_1:
            bandType = 1;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_2:
            bandType = 2;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_3:
            bandType = 3;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_4:
            bandType = 4;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_5:
            bandType = 5;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_6:
            bandType = 6;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_7:
            bandType = 7;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_8:
            bandType = 8;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_9:
            bandType = 9;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_10:
            bandType = 10;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_11:
            bandType = 11;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_12:
            bandType = 12;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_13:
            bandType = 13;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_14:
            bandType = 14;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_15:
            bandType = 15;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_16:
            bandType = 16;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_17:
            bandType = 17;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_18:
            bandType = 18;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_19:
            bandType = 19;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_20:
            bandType = 20;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_21:
            bandType = 21;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_22:
            bandType = 22;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_23:
            bandType = 23;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_24:
            bandType = 24;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_25:
            bandType = 25;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_26:
            bandType = 26;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_27:
            bandType = 27;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_28:
            bandType = 28;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_29:
            bandType = 29;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_30:
            bandType = 30;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_31:
            bandType = 31;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_32:
            bandType = 32;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_33:
            bandType = 33;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_34:
            bandType = 34;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_35:
            bandType = 35;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_36:
            bandType = 36;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_37:
            bandType = 37;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_38:
            bandType = 38;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_39:
            bandType = 39;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_40:
            bandType = 40;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_41:
            bandType = 41;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_42:
            bandType = 42;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_43:
            bandType = 43;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_44:
            bandType = 44;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_45:
            bandType = 45;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_46:
            bandType = 46;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_47:
            bandType = 47;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_48:
            bandType = 48;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_50:
            bandType = 50;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_51:
            bandType = 51;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_65:
            bandType = 65;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_66:
            bandType = 66;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_67:
            bandType = 67;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_68:
            bandType = 68;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_69:
            bandType = 69;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_70:
            bandType = 70;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_71:
            bandType = 71;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_72:
            bandType = 72;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_74:
            bandType = 74;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_75:
            bandType = 75;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_76:
            bandType = 76;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_252:
            bandType = 252;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_255:
            bandType = 255;
            break;

        default:
            break;
        }
    }
    else if (prmSetup.aEntryList[bandIndex].ulWaveForm == cOCTVC1_RADIO_STANDARD_ENUM_GSM)
    {
        switch (prmSetup.aEntryList[bandIndex].ulBand)
        {
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_850:
            bandType = 5;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_P_900:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_R_900:
        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_E_900:
            bandType = 8;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_DCS_1800:
            bandType = 3;
            break;

        case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_PCS_1900:
            bandType = 2;
            break;

        default:
            break;
        }
    }

    if (0 != bandType)
    {
        char dspType;
        if (TRUE == isItPrimaryDsp())
        {
            dspType = '1';
        }
        else
        {
            dspType = '2';
        }

        configureCellAntennaPA(dspType, bandType, 't');
    }
}
#endif

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmUmtsCellStatsGet

Description:    Check and close the Wnm components.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmUmtsCellStatsGet(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_UMTS_CELL_STATS *f_pUmtsCellStats)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    Cmd.ulCellId = f_ulCellId;
    mOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_UMTS_CELL_RSP_SWAP(&Rsp);

    *f_pUmtsCellStats = Rsp.Stats;

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmUmtsCellStatsDisplay

Description:    Check and close the Wnm components.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void WnmUmtsCellStatsDisplay(tOCTVC1_SUB_OBJECT_ID f_ulCellId,
                             tOCTVC1_RUS_WNM_UMTS_CELL_STATS *f_pUmtsCellStats)
{
    FILE *pFile = NULL;
    char fileName[50];

    DEBUGMSG(("----- UMTS (%08x)\n", f_ulCellId));
    DEBUGMSG(("   + RF   : %s \t uarfcn=%d (%6.3f Mhz)\n",
              WnmGetBandString(f_pUmtsCellStats->ulWaveForm, f_pUmtsCellStats->ulBand),
              f_pUmtsCellStats->ulUarfcn,
              ((double)f_pUmtsCellStats->ulFrequency) / 1000000.0));
    DEBUGMSG(("   + RSSI          : %6.3f dBm\n", ((double)f_pUmtsCellStats->lRssidBm) / 512.0));
    DEBUGMSG(("   + Rscp          : %6.3f dBm (CPICH)\n", ((double)f_pUmtsCellStats->lRscpdBm) / 512.0));
    DEBUGMSG(("   + Ec/I0         : %6.3f dB  \n", ((double)f_pUmtsCellStats->lEcI0dB) / 512.0));
    DEBUGMSG(("   + Delay         : %d chip\n", f_pUmtsCellStats->lRelativeDelayChip));
    DEBUGMSG(("   + PSC           : %d\n", f_pUmtsCellStats->usPsc));
    DEBUGMSG(("   + Diag Code     : %s \n", WnmGetUmtsCodestring(f_pUmtsCellStats->ulDecoderDiagnosticCode)));
    DEBUGMSG(("   + BCH payload file\n"));
    DEBUGMSG(("    + BlkCnt       : %d \n", f_pUmtsCellStats->usBchPayloadBlockCnt));
    DEBUGMSG(("    + BlkPassCnt   : %d \n", f_pUmtsCellStats->usBchPayloadValidBlockCnt));
    DEBUGMSG(("    + Payload file : %s \n", f_pUmtsCellStats->szBchPayloadFileName));
    //    WnmCellNetworkInfoDisplay(&f_pUmtsCellStats->NetworkInfo);

    /* Calculate Different Operators */
    if (0 == gUarfcn)
    {
        gUarfcn = f_pUmtsCellStats->ulUarfcn;
    }
    else
    {
        if (gUarfcn != f_pUmtsCellStats->ulUarfcn)
        {
            gCellCount = 0;
            gOperatorCount++;
            gUarfcn = f_pUmtsCellStats->ulUarfcn;
        }
        else
        {
            gCellCount++;
        }
    }

    sprintf(fileName, "/tmp/3GListenMode_Output_Operator%d_CellId%d.txt", gOperatorCount, gCellCount);

    pFile = fopen(fileName, "wb");
    if (NULL == pFile)
    {
        DEBUG1(("Failed to open file %s\n", fileName));
        return;
    }

    /* Write the BAND */
    fprintf(pFile, "%s %s\n", "BAND   : ",
            WnmGetBandString(f_pUmtsCellStats->ulWaveForm, f_pUmtsCellStats->ulBand));
    g3gListenModeSysInfo.band = convertToFxLFromOctBand(f_pUmtsCellStats->ulBand);

    /* Write the UARFCN */
    fprintf(pFile, "%s %d\n", "UARFCN : ",
            f_pUmtsCellStats->ulUarfcn);
    g3gListenModeSysInfo.uarfcn = f_pUmtsCellStats->ulUarfcn;

    /* Set the Cell Count */
    g3gListenModeSysInfo.numOfCell = 1;

    /* Write the RSSI */
    fprintf(pFile, "%s %f\n", "RSSI   : ",
            ((double)f_pUmtsCellStats->lRssidBm) / 512.0);
    g3gListenModeSysInfo.cellInfo[0].rssi = ((double)f_pUmtsCellStats->lRssidBm) / 512.0;

    /* Write the RSCP */
    fprintf(pFile, "%s %f\n", "RSCP   : ",
            ((double)f_pUmtsCellStats->lRscpdBm) / 512.0);
    g3gListenModeSysInfo.cellInfo[0].rscp = ((double)f_pUmtsCellStats->lRscpdBm) / 512.0;

    /* Write the EC/I0 */
    fprintf(pFile, "%s %f\n", "EC/I0  : ",
            ((double)f_pUmtsCellStats->lEcI0dB) / 512.0);
    g3gListenModeSysInfo.cellInfo[0].ecio = ((double)f_pUmtsCellStats->lEcI0dB) / 512.0;

    /* Write the PSC */
    fprintf(pFile, "%s %d\n", "PSC    : ",
            f_pUmtsCellStats->usPsc);
    g3gListenModeSysInfo.cellInfo[0].psc = f_pUmtsCellStats->usPsc;

    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    return;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmGsmCellStatsGet

Description:    Return cell stats

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmGsmCellStatsGet(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_GSM_CELL_STATS *f_pGsmCellStats)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    Cmd.ulCellId = f_ulCellId;
    mOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_GSM_CELL_RSP_SWAP(&Rsp);

    *f_pGsmCellStats = Rsp.Stats;

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmGsmCellStatsDisplay

Description:    Display stats

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void WnmGsmCellStatsDisplay(tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_GSM_CELL_STATS *f_pGsmCellStats)
{
    printf("----- GSM (%08x)\n", f_ulCellId);
    printf("   + RF   : %s \t Arfcn=%d (%6.3f Mhz)\n",
           WnmGetBandString(f_pGsmCellStats->ulWaveForm, f_pGsmCellStats->ulBand),
           f_pGsmCellStats->ulArfcn,
           ((double)f_pGsmCellStats->ulFrequency) / 1000000.0);
    printf("   + BSIC: NCC  : %d\n", f_pGsmCellStats->NCC);
    printf("   + BSIC: BCC  : %d\n", f_pGsmCellStats->BCC);
    printf("   + RSSI       : %6.3f dBm\n", ((double)f_pGsmCellStats->lRssidBm) / 512.0);
    printf("   + SNR        : %6.3f dB\n", ((double)f_pGsmCellStats->lSNRdB) / 512.0);
    printf("   + Time Adv.  : %d \n", f_pGsmCellStats->lTimeAdvance);
    printf("   + Diag Code  : %s \n", WnmGetGsmCodestring(f_pGsmCellStats->ulDecoderDiagnosticCode));
    printf("   + BCH payload file\n");
    printf("    + BlkCnt       : %d \n", f_pGsmCellStats->usBcchPayloadBlockCnt);
    printf("    + BlkPassCnt   : %d \n", f_pGsmCellStats->usBcchPayloadValidBlockCnt);
    printf("    + Payload file : %s \n", f_pGsmCellStats->szBcchPayloadFileName);
    printf("    + Found TC mask: %08x \n", f_pGsmCellStats->ulFoundSIsMask);
    return;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmLteCellStatsGet

Description:    Return cell stats

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmLteCellStatsGet(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_LTE_CELL_STATS *f_pLteCellStats)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_CMD_DEF(&Cmd);
    Cmd.hWnm = f_hWnm;
    Cmd.ulCellId = f_ulCellId;
    mOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_LTE_CELL_RSP_SWAP(&Rsp);

    *f_pLteCellStats = Rsp.Stats;

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmLteCellStatsDisplay

Description:    Display stats

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void WnmLteCellStatsDisplay(tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_LTE_CELL_STATS *f_pLteCellStats)
{
    printf("----- LTE (%08x)\n", f_ulCellId);
    printf("   + RF         : %s \t Erfcn=%d (%6.3f Mhz)\n",
           WnmGetBandString(f_pLteCellStats->ulWaveForm, f_pLteCellStats->ulBand),
           f_pLteCellStats->ulEarfcn,
           ((double)f_pLteCellStats->ulFrequency) / 1000000.0);
    printf("   + CellId     : %d\n", f_pLteCellStats->usCellIdentity);
    printf("   + Mode       : %s\n", WnmGetLteWaveModeString(f_pLteCellStats->byWaveFormMode));
    printf("   + Bandwidth  : %s\n", WnmGetLteBandwidthString(f_pLteCellStats->ulBandWidth));
    printf("   + Num Tx Ant.: %s\n", WnmGetLteNumAntennaPortString(f_pLteCellStats->byNumTxAntennaPorts));
    printf("   + CP mode    : %s\n", WnmGetLteCPModeString(f_pLteCellStats->byCyclicPrefix));
    printf("   + Pich Group : %s\n", WnmGetLtePhichGroupString(f_pLteCellStats->byPhichGroup));
    printf("   + Pich Dur   : %s\n", WnmGetLtePhichDurString(f_pLteCellStats->byPhichDuration));
    printf("   + lRelativeD : %d\n", f_pLteCellStats->lRelativeD);
    if (f_pLteCellStats->byWaveFormMode == cOCTVC1_RUS_WNM_LTE_MODE_ENUM_TDD)
    {
        printf("   + Tdd UlDlCfg: %d\n", f_pLteCellStats->byTddUlDlConfig);
        printf("   + Tdd SSFCfg : %d\n", f_pLteCellStats->byTddSpecialSF);
    }
    printf("   + RSSI       : %6.3f dBm\n", ((double)f_pLteCellStats->lRssidBm) / 512.0);
    printf("   + RSRP       : %6.3f dBm\n", ((double)f_pLteCellStats->lRsrpdBm) / 512.0);
    printf("   + RSRQ       : %6.3f dB\n", ((double)f_pLteCellStats->lRsrqdB) / 512.0);
    printf("   + Diag Code     : %s \n", WnmGetLteCodestring(f_pLteCellStats->ulDecoderDiagnosticCode));
    printf("   + BCH payload file\n");
    printf("    + BlkCnt       : %d \n", f_pLteCellStats->usBchPayloadBlockCnt);
    printf("    + BlkPassCnt   : %d \n", f_pLteCellStats->usBchPayloadValidBlockCnt);
    printf("    + Payload file : %s \n", f_pLteCellStats->szBchPayloadFileName);
    printf("    + Sibs Found   : 0x%08x ==> SIBs %s \n", f_pLteCellStats->ulFoundSibsMask, WnmGetLteSibsPresentInMask(f_pLteCellStats->ulFoundSibsMask));
    return;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmRssiBandStatsDisplay

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmRssiBandStatsDisplay(tOCT_UINT32 f_ulIndx, tOCTVC1_RUS_WNM_RSSI_BAND_STATS *f_ptRssiBandStats)
{
    printf("-Rssi % 2d: Waveform=%s \t\t band=%s \t Num carrier=%d\n",
           f_ulIndx,
           WnmGetWaveFormString(f_ptRssiBandStats->BandId.ulStandard),
           WnmGetBandString(f_ptRssiBandStats->BandId.ulStandard, f_ptRssiBandStats->BandId.ulBandNumber),
           f_ptRssiBandStats->ulNumRssiChannel);
    return (0);
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmRssiBandChannelGetData

Description:    Retreive Rssi mesurements on a specified ulWaveForm and ulBand.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int WnmRssiBandChannelGetData(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_RADIO_STANDARD_FREQ_BAND_ID *f_pBandId, tWNM_RSSI_BAND_CHANNEL *f_ptRssiBandChannel)
{
    tOCT_UINT32 EntryIndx, ulResult, i;
    tOCTVC1_OBJECT_CURSOR_ENUM ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_FIRST;
    tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;

    EntryIndx = 0;
    do
    {
        /*
         * Prepare command data.
         */
        mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_CMD_DEF(&Cmd);
        Cmd.hWnm = f_hWnm;
        Cmd.BandId = *f_pBandId;
        Cmd.ObjectGet.ulGetMode = ulGetMode;
        mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_CMD_SWAP(&Cmd);

        /*
         * Execute the command.
         */
        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &Cmd;
        CmdExecuteParms.pRsp = &Rsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_RSP);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            goto ErrorHandling;
        }

        /*
         * Swap the command response.
         */
        mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_RSP_SWAP(&Rsp);
        ulGetMode = Rsp.ObjectGet.ulGetMode;
        if (Rsp.ulNumEntry > cOCTVC1_RUS_WNM_RSSI_BAND_CHANNEL_MAX_ENTRY)
        {
            fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CHANNELS_CMD(). ulNumEntry out of range \n");
            goto ErrorHandling;
        }

        for (i = 0; i < Rsp.ulNumEntry; i++)
            f_ptRssiBandChannel->aChannelEntry[EntryIndx + i] = Rsp.aChannelEntry[i];

        EntryIndx += Rsp.ulNumEntry;

    } while (Rsp.ObjectGet.ulGetMode != cOCTVC1_OBJECT_CURSOR_ENUM_DONE);

    f_ptRssiBandChannel->ulWaveForm = f_pBandId->ulStandard;
    f_ptRssiBandChannel->ulBand = f_pBandId->ulBandNumber;
    f_ptRssiBandChannel->ulNumRssiChannel = EntryIndx;
    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmRssiBandChannelDataAnalysis

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmRssiBandChannelDataAnalysis(tWNM_RSSI_BAND_CHANNEL *f_ptRssiBandChannel, char *f_szRssiBandChannelFileName, tOCT_UINT32 f_bGenerateReportFile, tOCT_UINT32 f_bVerbose)
{
    FILE *pFileCsv = NULL;
    tOCT_UINT32 i;

    if (f_bGenerateReportFile)
    {
        pFileCsv = fopen(f_szRssiBandChannelFileName, "wt");
        if (pFileCsv == NULL)
        {
            return (-1);
        }
    }

    // Diplay Header File
    if (f_bVerbose)
    {
        printf("------------------------------------------------\n");
        printf("+ RSSI channel file: (%s) \t %s \t%s sz=%d \n",
               f_szRssiBandChannelFileName,
               WnmGetWaveFormString(f_ptRssiBandChannel->ulWaveForm),
               WnmGetBandString(f_ptRssiBandChannel->ulWaveForm, f_ptRssiBandChannel->ulBand),
               f_ptRssiBandChannel->ulNumRssiChannel);
    }
    if (f_bGenerateReportFile)
    {
        fprintf(pFileCsv, "+ RSSI channel file:  (%s) \t %s \t%s sz=%d \n",
                WnmGetWaveFormString(f_ptRssiBandChannel->ulWaveForm),
                f_szRssiBandChannelFileName,
                WnmGetBandString(f_ptRssiBandChannel->ulWaveForm, f_ptRssiBandChannel->ulBand),
                f_ptRssiBandChannel->ulNumRssiChannel);
    }

    if (f_ptRssiBandChannel->ulNumRssiChannel > cOCTVC1_RUS_WNM_RSSI_MAX_NUM_RSSI_CHANNEL_ENTRY)
    {
        if (f_bGenerateReportFile)
        {
            if (pFileCsv)
            {
                fclose(pFileCsv);
                pFileCsv = NULL;
            }
        }
        return (-1);
    }

    for (i = 0; i < f_ptRssiBandChannel->ulNumRssiChannel; i++)
    {
        if (f_bGenerateReportFile)
        {
            fprintf(pFileCsv, "%d,%6.2f\n", f_ptRssiBandChannel->aChannelEntry[i].ulRfcn, f_ptRssiBandChannel->aChannelEntry[i].lRssidBm / 512.0);
        }
        if (f_bVerbose)
        {
            printf("%d,%6.2f\n", f_ptRssiBandChannel->aChannelEntry[i].ulRfcn, f_ptRssiBandChannel->aChannelEntry[i].lRssidBm / 512.0);
        }
    }

    if (f_bGenerateReportFile)
    {
        if (pFileCsv)
        {
            fclose(pFileCsv);
            pFileCsv = NULL;
        }
    }

    return (0);
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmRssiBandStatsGet

Description:    Retreive Rssi statistic on the band.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

int WnmRssiBandStatsGet(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_OBJECT_CURSOR_ENUM *f_ulGetMode, tOCTVC1_RUS_WNM_RSSI_BAND_STATS *f_ptRssiBandStats)
{
    tOCT_UINT32 ulResult;
    tOCTVC1_OBJECT_CURSOR_ENUM ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_FIRST;
    tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;

    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CMD_DEF(&Cmd);
    Cmd.ObjectGet.hWnm = f_hWnm;
    Cmd.ObjectGet.ulGetMode = *f_ulGetMode;
    Cmd.ObjectGet.BandId.ulBandNumber = 0;
    Cmd.ObjectGet.BandId.ulStandard = 0;
    mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_RSP);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_STATS_RSSI_BAND_RSP_SWAP(&Rsp);

    *f_ptRssiBandStats = Rsp.Stats;
    *f_ulGetMode = Rsp.ObjectGet.ulGetMode;
    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmGsmCellGetList/WnmUmtsCellGetList/WnmLteCellGetList/WnmCdma2000CellGetList

Description:    Retreive all the list of GSM/UMTS/LTE cell identifier.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmGsmCellGetList(tOCTVC1_HANDLE_OBJECT f_hWnm, tBTSLIST *f_CellIdList)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF, i;
    tOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulCellIdx = 0;
    f_CellIdList->ulNumCell = 0;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_CMD_DEF(&Cmd);
    Cmd.SubObjectIdGet.hObject = f_hWnm;
    Cmd.SubObjectIdGet.ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_FIRST;
    Cmd.SubObjectIdGet.ulSubObjectId = 0;
    mOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_CMD_SWAP(&Cmd);
GetNextChunk:
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_LIST_GSM_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_RSP_SWAP(&Rsp);

    f_CellIdList->ulNumCell += Rsp.SubObjectIdList.ulNumSubObjectId;
    for (i = 0; i < Rsp.SubObjectIdList.ulNumSubObjectId; i++)
    {
        f_CellIdList->aCellId[ulCellIdx] = Rsp.SubObjectIdList.aSubObjectId[i];
        ulCellIdx++;
    }

    if (Rsp.SubObjectIdGet.ulGetMode == cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_NEXT)
    {

        mOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_CMD_DEF(&Cmd);
        Cmd.SubObjectIdGet.hObject = f_hWnm;
        Cmd.SubObjectIdGet.ulGetMode = Rsp.SubObjectIdGet.ulGetMode;
        Cmd.SubObjectIdGet.ulSubObjectId = Rsp.SubObjectIdGet.ulSubObjectId;
        mOCTVC1_RUS_MSG_WNM_LIST_GSM_CELL_CMD_SWAP(&Cmd);
        goto GetNextChunk;
    }

    return (0);
ErrorHandling:
    return ulResult;
}

int WnmUmtsCellGetList(tOCTVC1_HANDLE_OBJECT f_hWnm, tBTSLIST *f_CellIdList)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF, i;
    tOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulCellIdx = 0;
    f_CellIdList->ulNumCell = 0;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_CMD_DEF(&Cmd);
    Cmd.SubObjectIdGet.hObject = f_hWnm;
    Cmd.SubObjectIdGet.ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_FIRST;
    Cmd.SubObjectIdGet.ulSubObjectId = 0;
    mOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
GetNextChunk:
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_LIST_UMTS_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_RSP_SWAP(&Rsp);

    f_CellIdList->ulNumCell += Rsp.SubObjectIdList.ulNumSubObjectId;
    for (i = 0; i < Rsp.SubObjectIdList.ulNumSubObjectId; i++)
    {
        f_CellIdList->aCellId[ulCellIdx] = Rsp.SubObjectIdList.aSubObjectId[i];
        ulCellIdx++;
    }

    if (Rsp.SubObjectIdGet.ulGetMode == cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_NEXT)
    {

        mOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_CMD_DEF(&Cmd);
        Cmd.SubObjectIdGet.hObject = f_hWnm;
        Cmd.SubObjectIdGet.ulGetMode = Rsp.SubObjectIdGet.ulGetMode;
        Cmd.SubObjectIdGet.ulSubObjectId = Rsp.SubObjectIdGet.ulSubObjectId;
        mOCTVC1_RUS_MSG_WNM_LIST_UMTS_CELL_CMD_SWAP(&Cmd);
        goto GetNextChunk;
    }

    return (0);
ErrorHandling:
    return ulResult;
}

int WnmLteCellGetList(tOCTVC1_HANDLE_OBJECT f_hWnm, tBTSLIST *f_CellIdList)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF, i;
    tOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_CMD Cmd;
    tOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulCellIdx = 0;
    f_CellIdList->ulNumCell = 0;
    /*
     * Prepare command data.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_CMD_DEF(&Cmd);
    Cmd.SubObjectIdGet.hObject = f_hWnm;
    Cmd.SubObjectIdGet.ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_FIRST;
    Cmd.SubObjectIdGet.ulSubObjectId = 0;
    mOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
GetNextChunk:
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_RSP);
    if (g_AppCtxRus.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRus.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_RUS_MSG_LIST_LTE_CELL_CMD() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_RSP_SWAP(&Rsp);

    f_CellIdList->ulNumCell += Rsp.SubObjectIdList.ulNumSubObjectId;
    for (i = 0; i < Rsp.SubObjectIdList.ulNumSubObjectId; i++)
    {
        f_CellIdList->aCellId[ulCellIdx] = Rsp.SubObjectIdList.aSubObjectId[i];
        ulCellIdx++;
    }

    if (Rsp.SubObjectIdGet.ulGetMode == cOCTVC1_OBJECT_CURSOR_ENUM_SUB_OBJECT_NEXT)
    {

        mOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_CMD_DEF(&Cmd);
        Cmd.SubObjectIdGet.hObject = f_hWnm;
        Cmd.SubObjectIdGet.ulGetMode = Rsp.SubObjectIdGet.ulGetMode;
        Cmd.SubObjectIdGet.ulSubObjectId = Rsp.SubObjectIdGet.ulSubObjectId;
        mOCTVC1_RUS_MSG_WNM_LIST_LTE_CELL_CMD_SWAP(&Cmd);
        goto GetNextChunk;
    }

    return (0);
ErrorHandling:
    return ulResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmUmtsBCHBufferAnalysis/WnmGsmBCHBufferAnalysis/WnmLteBCHBufferAnalysis/WnmCdma2000BCHBufferAnalysis

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmUmtsBCHBufferAnalysis(char *f_szFileName, tOCT_UINT16 f_usBchPayloadBlockCnt, tOCT_UINT32 f_bVerbose)
{
    FILE *pFile;
    tOCT_INT32 i, j, CrcErr, TrChSzErr, SfnJumpErr, SfnNextExpected;
    tOCTVC1_RUS_WNM_UMTS_BCH_TB tTrBch;
#if 0
    char fileName[50];

    sprintf (fileName, "3GListenMode_BCCH_Data_Operator%d_CellId%d.txt", gOperatorCount, gCellCount);

    /* Initialize ASN Context */
    initializeASNContextWithInputFile (fileName);
#else
    /* Initialize ASN Context */
    initializeASNContext();
#endif

    /* BCCH SIB Info */
    BcchSibInfo_t bcchSibInfo;
    memset((BcchSibInfo_t *)&bcchSibInfo, 0, sizeof(BcchSibInfo_t));

    pFile = fopen(f_szFileName, "rb");
    if (pFile == NULL)
    {
        return (-1);
    }

    if (f_usBchPayloadBlockCnt > cOCTVC1_RUS_WNM_UMTS_BCH_MAX_NUM_TB_IN_FILE)
    {
        if (NULL != pFile)
        {
            fclose(pFile);
            pFile = NULL;
        }

        return (-1);
    }

    CrcErr = TrChSzErr = SfnJumpErr = 0;
    SfnNextExpected = -1;
    for (i = 0; i < f_usBchPayloadBlockCnt; i++)
    {
        // Read back a transport block
        fread(&tTrBch, sizeof(tOCTVC1_RUS_WNM_UMTS_BCH_TB), 1, pFile);
        // Manage the endianness
        mOCTVC1_RUS_WNM_UMTS_BCH_TB_SWAP(&tTrBch);
        // Count Crc;
        CrcErr += (tTrBch.usCrc != 1);

        if (tTrBch.usCrc)
        {
            if (SfnNextExpected != -1)
            {
                SfnJumpErr += (SfnNextExpected != tTrBch.usSFN);
            }
            SfnNextExpected = (tTrBch.usSFN + 2) % 4096;
        }
        else
        {
            SfnNextExpected = -1;
        }

        // Check payload size;
        TrChSzErr += (tTrBch.usNumBits != 246);

        // UMTS Buffer Decode
        if ((CrcErr == 0) && (TrChSzErr == 0) && (SfnJumpErr == 0))
        {
            WnmUmtsBCCHBufferDecode(&tTrBch.abPayload[0], &bcchSibInfo);
        }
    }

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if ((CrcErr == 0) && (TrChSzErr == 0) && (SfnJumpErr == 0))
    {
        printf("    Umts Bch Sanity check ok\n");
    }
    else
    {
        printf("    Umts Bch Sanity check error:\n");
        printf("    CrcErr=%d\n", CrcErr);
        printf("    TrChSzErr=%d\n", TrChSzErr);
        printf("    SfnJumpErr=%d\n", SfnJumpErr);
    }

    /* Clean ASN Context */
    cleanASNContext();
    // cleanASNContextWithInputFile ();

    return (0);
}

int WnmGsmBCCHBufferAnalysis(char *f_szFileName, tOCTVC1_SUB_OBJECT_ID f_ulCellId, tOCTVC1_RUS_WNM_GSM_CELL_STATS *f_pGsmCellStats)
{
    FILE *pFile, *siInfoFile;
    cellParams params;
    tOCT_UINT32 i, CrcErr, SfnErr, TrChSzErr;
    char *FileName = "/home/wgs/GsmSysInfo.txt";
    tOCTVC1_RUS_WNM_GSM_BCCH_TB tTrBch;
    pFile = fopen(f_szFileName, "rb");
    combinedSIs *sysInfo = (combinedSIs *)&sndMsgBuf[sizeof(fxL2gListenModeSystemInfoInd)];
    fxL2gListenModeSystemInfoInd *sendInd = (fxL2gListenModeSystemInfoInd *)sndMsgBuf;

    memset(sysInfo, 0, sizeof(combinedSIs));
    memset(sendInd, 0, sizeof(fxL2gListenModeSystemInfoInd));

    if (pFile == NULL)
    {
        return (-1);
    }
    siInfoFile = fopen(FileName, "at");
    if (siInfoFile == NULL)
        fprintf(stderr, "Error: can't open %s\n", FileName);

    if (f_pGsmCellStats->usBcchPayloadBlockCnt > cOCTVC1_RUS_WNM_GSM_BCCH_MAX_NUM_TB_IN_FILE)
    {
        if (NULL != pFile)
        {
            fclose(pFile);
            pFile = NULL;
        }

        if (NULL != siInfoFile)
        {
            fclose(siInfoFile);
            siInfoFile = NULL;
        }

        return (-1);
    }

    CrcErr = SfnErr = TrChSzErr = 0;
    for (i = 0; i < f_pGsmCellStats->usBcchPayloadBlockCnt; i++)
    {
        // Read back a transport block
        fread(&tTrBch, sizeof(tOCTVC1_RUS_WNM_GSM_BCCH_TB), 1, pFile);
        // Manage the endianness
        mOCTVC1_RUS_WNM_GSM_BCCH_TB_SWAP(&tTrBch);
        // Count Crc;
        CrcErr += (tTrBch.usCrc != 0);
        if (siInfoFile != NULL)
        {
            if (f_ulCellId != bcchBufferCellId)
            {
                fprintf(siInfoFile, "\n##################################################################################################\n");
                fprintf(siInfoFile, "GSM-ARFCN=%d\n", f_pGsmCellStats->ulArfcn);
                fprintf(siInfoFile, "GSM-BAND=%d\n", f_pGsmCellStats->ulBand);
                fprintf(siInfoFile, "GSM-BSIC-NCC=%d\n", f_pGsmCellStats->NCC);
                fprintf(siInfoFile, "GSM-BSIC-BCC=%d\n", f_pGsmCellStats->BCC);
                fprintf(siInfoFile, "GSM-RSSI=%f dBm\n", ((double)f_pGsmCellStats->lRssidBm) / 512.0);
                fprintf(siInfoFile, "GSM-SNR=%f dB\n", ((double)f_pGsmCellStats->lSNRdB) / 512.0);
                fprintf(siInfoFile, "GSM cell found in band  %s \n", WnmGetBandString(f_pGsmCellStats->ulWaveForm, f_pGsmCellStats->ulBand));
                fprintf(siInfoFile, "\n##################################################################################################\n");
                bcchBufferCellId = f_ulCellId;
                params.arfcn = f_pGsmCellStats->ulArfcn;
                params.band = convertToFxlBandFromOct(f_pGsmCellStats->ulBand);
                params.ncc = f_pGsmCellStats->NCC;
                params.bcc = f_pGsmCellStats->BCC;
                params.rssi = (float)(((double)f_pGsmCellStats->lRssidBm) / 512.0);
                params.snr = (float)(((double)f_pGsmCellStats->lSNRdB) / 512.0);
                params.cellId = f_ulCellId;
            }
            if ((CrcErr == 0))
            {
                // We are currently interested in SI-1, SI-2, SI-2bis, SI-2ter, SI-2quater, SI-3 and SI-4.
                if (((sysInfo->siRcvd & RR_SI1_MASK) == RR_SI1_MASK) &&
                    ((sysInfo->siRcvd & RR_SI2_MASK) == RR_SI2_MASK) &&
                    ((sysInfo->siRcvd & RR_SI3_MASK) == RR_SI3_MASK) &&
                    ((sysInfo->siRcvd & RR_SI4_MASK) == RR_SI4_MASK) &&
                    ((sysInfo->siRcvd & RR_SI2BIS_MASK) == RR_SI2BIS_MASK) &&
                    ((sysInfo->siRcvd & RR_SI2TER_MASK) == RR_SI2TER_MASK) &&
                    ((sysInfo->siRcvd & RR_SI2QUATER_MASK) == RR_SI2QUATER_MASK))
                {
                    break;
                }
                else
                {
                    rr_si_servcell_bcch_si_handle(&(tTrBch.abPayload[0]), siInfoFile, &params);
                }
            }
            fflush(siInfoFile);
        }
    }

    // BCCH Binary file parsing is done, send the populated SYS-INFO-IND to client here, if SI-3 atleast is decoded.
    if (((sysInfo->siRcvd & RR_SI3_MASK) == RR_SI3_MASK) && (sysInfo->servingCellId == params.cellId))
    {
        sendInd->hdr.msgLength = sizeof(fxL2gListenModeSystemInfoInd);
        sendInd->hdr.msgType = FXL_2G_LISTEN_MODE_SYSTEM_INFO_IND;
        sendInd->hdr.rat = FXL_RAT_2G;
        writeToExternalSocketInterface((tOCT_UINT8 *)sendInd, sizeof(fxL2gListenModeSystemInfoInd));

        // clear the global variale context of earlier run
        bcchBufferCellId = 0xFFFF;
        si1Cell = 0xFFFFFFF0;
        si2Cell = 0xFFFFFFF1;
        si2qCell = 0xFFFFFFF2;
        si2bisCell = 0xFFFFFFF3;
        si2terCell = 0xFFFFFFF4;
        si3Cell = 0xFFFFFFF5;
        si4Cell = 0xFFFFFFF6;
    }

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if ((CrcErr == 0))
    {
        printf("    Gsm Bcch Sanity check ok\n");
    }
    else
    {
        printf("    Gsm Bcch Sanity check error:\n");
        printf("    CrcErr=%d\n", CrcErr);
    }

    if (siInfoFile != NULL)
    {
        fclose(siInfoFile);
        siInfoFile = NULL;
    }

    return (0);
}

tOCTVC1_RUS_WNM_LTE_BCH_TB tTrBch;
// int WnmLteBCHBufferAnalysis( char * f_szFileName , tOCT_UINT16 f_usBchPayloadBlockCnt, tOCT_UINT32 f_bVerbose )
int WnmLteBCHBufferAnalysis(char *f_szFileName, tOCT_UINT16 f_usBchPayloadBlockCnt, lteServingCellStruct *servingCell)
{
    FILE *pFile;
    tOCT_UINT32 i, CrcErr, SfnErr, TrChSzErr;
    pFile = fopen(f_szFileName, "rb");
    if (pFile == NULL)
    {
        return (-1);
    }
    memset(&tTrBch, 0, sizeof(tOCTVC1_RUS_WNM_LTE_BCH_TB));

    if (f_usBchPayloadBlockCnt > cOCTVC1_RUS_WNM_LTE_BCH_MAX_NUM_TB_IN_FILE)
    {
        if (NULL != pFile)
        {
            fclose(pFile);
            pFile = NULL;
        }

        return (-1);
    }

    /* Enum 3 for anttena port means 4 */
    if (servingCell->noOfTxAntennaPorts == 3)
    {
        servingCell->noOfTxAntennaPorts = 4;
    }

    logPrint(LOG_DEBUG, "PhyCellId = %d EARFCN = %d  noOfTxAntennaPorts = %d\n", servingCell->sfNsfcellId, servingCell->earfcn, servingCell->noOfTxAntennaPorts);
    logPrint(LOG_DEBUG, "Serving Cell RSRP = %f RSSI = %d RELATIVED = %u\n", servingCell->rsrp, servingCell->rssi, servingCell->relativeD);
    logPrint(LOG_DEBUG, "Mib-Bandwidth = %d Mib-PhichDuration = %d  Mib-PhichResource = %d\n", servingCell->mib.dlBw, servingCell->mib.phichDuration, servingCell->mib.phichResource);
    sysInfoind.dlBw = servingCell->mib.dlBw;
    sysInfoind.phichDuration = servingCell->mib.phichDuration;
    sysInfoind.phichResource = servingCell->mib.phichResource;
    sysInfoind.phyCellId = servingCell->sfNsfcellId;
    sysInfoind.rsrp = servingCell->rsrp;
    sysInfoind.rssi = servingCell->rssi;
    sysInfoind.rsrq = servingCell->rsrq;
    sysInfoind.earfcn = servingCell->earfcn;
    sysInfoind.noOfTxAntPorts = servingCell->noOfTxAntennaPorts;
    sysInfoind.sibMask = 0;
    sysInfoind.relativeD = servingCell->relativeD;

    CrcErr = SfnErr = TrChSzErr = 0;
    for (i = 0; i < f_usBchPayloadBlockCnt; i++)
    {
        // Read back a transport block
        fread(&tTrBch, sizeof(tOCTVC1_RUS_WNM_LTE_BCH_TB), 1, pFile);
        // Manage the endianness
        mOCTVC1_RUS_WNM_LTE_BCH_TB_SWAP(&tTrBch);
        // Count Crc;
        CrcErr += (tTrBch.usCrc != 0);
        if ((tTrBch.usCrc == 0) && (tTrBch.usNumBits > 8))
        {
            decodeBcchData((tOCT_UINT8 *)tTrBch.abPayload, tTrBch.usNumBits / 8, servingCell);
        }
    }

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if ((CrcErr == 0))
    {
        printf("    LTE Bch Sanity check ok\n");
    }
    else
    {
        printf("    LTE Bch Sanity check error:\n");
        printf("    CrcErr=%d\n", CrcErr);
    }
    sendLteSysInfoInd();
    return (0);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmEventHandler

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmEventHandler(tWNM_CTX *f_pWnm)
{
    tOCT_UINT32 PayloadSize;
    tOCT_UINT32 lRc;
    tOCT_UINT8 abyPayload[1500]; /* 1500 is the MTU for Ethernet. */
    tOCT_INT8 szLocalFileName[1024];
    lRc = EventHandler(abyPayload, &PayloadSize, 1500);
    if (lRc == 0)
    {
        tOCTVC1_EVENT_HEADER *pHeader = (tOCTVC1_EVENT_HEADER *)abyPayload;
        mOCTVC1_EVENT_HEADER_SWAP(pHeader);
        switch (pHeader->ulEventId)
        {
        case cOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EID:
        {
            tOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT *pEvt = (tOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT *)abyPayload;
            mOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EVT_SWAP(pEvt);
            f_pWnm->tStatus = pEvt->Stats;
            WnmDisplayStatus(&f_pWnm->tStatus);
        }
        break;
        case cOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EID:
        {
            tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *pEvt = (tOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT *)abyPayload;
            mOCTVC1_RUS_MSG_WNM_GSM_CELL_STATS_EVT_SWAP(pEvt);
            WnmGsmCellStatsDisplay(pEvt->ulCellId, &pEvt->Stats);
            if (f_pWnm->PrmSetup.ulSearchMode == cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE)
            {
                sprintf(szLocalFileName, "%s%02d.bin", "/tmp/gsm_customer_bcch", pEvt->ulCellId);
                if (ReadFileFromTarget(szLocalFileName, pEvt->Stats.szBcchPayloadFileName, 0) != 0)
                    goto CriticalAbort;
            }
        }
        break;
        case cOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EID:
        {
            tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *pEvt = (tOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT *)abyPayload;
            mOCTVC1_RUS_MSG_WNM_UMTS_CELL_STATS_EVT_SWAP(pEvt);
            WnmUmtsCellStatsDisplay(pEvt->ulCellId, &pEvt->Stats);
            if (f_pWnm->PrmSetup.ulSearchMode == cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE)
            {
                sprintf(szLocalFileName, "%s%02d.bin", "/tmp/umts_customer_bch", pEvt->ulCellId);
                if (ReadFileFromTarget(szLocalFileName, pEvt->Stats.szBchPayloadFileName, 0) != 0)
                    goto CriticalAbort;
            }
        }
        break;
        case cOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EID:
        {
            tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *pEvt = (tOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT *)abyPayload;
            mOCTVC1_RUS_MSG_WNM_LTE_CELL_STATS_EVT_SWAP(pEvt);
            WnmLteCellStatsDisplay(pEvt->ulCellId, &pEvt->Stats);
            if (f_pWnm->PrmSetup.ulSearchMode == cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE)
            {
                sprintf(szLocalFileName, "%s%02d.bin", "/tmp/lte_customer_bch", pEvt->ulCellId);
                if (ReadFileFromTarget(szLocalFileName, pEvt->Stats.szBchPayloadFileName, 0) != 0)
                    goto CriticalAbort;
            }
        }
        break;
        }
    }
    return (0);
CriticalAbort:
    return (-1);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmDiagnose

Description:    Retreives some diagnostic capture

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmDiagnose(char *f_szBaseFileName, tOCTVC1_RADIO_STANDARD_ENUM f_eWaveform)
{
    tOCT_INT8 szLocalFileName[1024];

    switch (f_eWaveform)
    {
    case cOCTVC1_RADIO_STANDARD_ENUM_GSM:
        // Retreive capture of the last Gsm decoder
        sprintf(szLocalFileName, "%sGsmDiagnostic.bin", f_szBaseFileName);
        if (ReadFileFromTarget(szLocalFileName, "WNM/gsm/diag/GsmDiagnostic.bin", 0) != 0)
            goto CriticalAbort;

        break;
    case cOCTVC1_RADIO_STANDARD_ENUM_UMTS:
        // Retreive capture of the last UMTS decoder
        sprintf(szLocalFileName, "%sUmtsDiagnostic.bin", f_szBaseFileName);
        if (ReadFileFromTarget(szLocalFileName, "WNM/umts/diag/UmtsDiagnostic.bin", 0) != 0)
            goto CriticalAbort;

        break;

    case cOCTVC1_RADIO_STANDARD_ENUM_LTE:
        // Retreive capture of the last LTE decoder
        sprintf(szLocalFileName, "%sLteDiagnostic.bin", f_szBaseFileName);
        if (ReadFileFromTarget(szLocalFileName, "WNM/lte/diag/LteDiagnostic.bin", 0) != 0)
            goto CriticalAbort;

        break;
    }

    // Retreive Received samples
    sprintf(szLocalFileName, "%sRxSamples.bin", f_szBaseFileName);
    if (ReadFileFromTarget(szLocalFileName, "WNM/rffi/cap_rx_samples.bin", 0) != 0)
        goto CriticalAbort;

    // Retreive SA module
    sprintf(szLocalFileName, "%sSaDiagnostic.bin", f_szBaseFileName);
    if (ReadFileFromTarget(szLocalFileName, "WNM/Sa/diag/SaDiagnostic.bin", 0) != 0)
        goto CriticalAbort;

    return (0);
CriticalAbort:
    return (-1);
}

int BtsCandidateListAddTo(tSB_BTSLIST *f_pList, tBTS *f_pBts)
{
    if (f_pList->ulNumBts >= (cMAX_NUM_SB_BTS))
        return (-1);
    f_pList->atList[f_pList->ulNumBts] = *f_pBts;
    f_pList->ulNumBts++;
    return (0);
}

int BtsCandidateListGet(tOCTVC1_HANDLE_OBJECT f_hWnm, tOCTVC1_RADIO_STANDARD_ENUM f_ulWaveForm, tSB_BTSLIST *f_pList)
{
    tOCT_UINT32 i, j;
    tBTSLIST BtsListId;
    tBTS BtsInfo;

    switch (f_ulWaveForm)
    {
    case cOCTVC1_RADIO_STANDARD_ENUM_GSM:
    {
        tOCTVC1_RUS_WNM_GSM_CELL_STATS CellStats;
        /* Retreive the list of the BTS object index */

        if (WnmGsmCellGetList(f_hWnm, &BtsListId) != 0)
            goto CriticalAbort;

        for (i = 0; i < BtsListId.ulNumCell; i++)
        {
            tOCTVC1_SUB_OBJECT_ID CellId = BtsListId.aCellId[i];
            /* Retreive the stats */
            if (WnmGsmCellStatsGet(f_hWnm, CellId, &CellStats) != 0)
                goto CriticalAbort;

            BtsInfo.ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
            BtsInfo.ulBand = CellStats.ulBand;
            BtsInfo.ulRfcn = CellStats.ulArfcn;
            BtsInfo.usPCI = (CellStats.BCC << 4) | (CellStats.NCC);
            BtsInfo.lSNR = CellStats.lSNRdB;
            if (BtsCandidateListAddTo(f_pList, &BtsInfo) != 0)
            {
                goto CriticalAbort;
            }
        }
    }
    break;

    case cOCTVC1_RADIO_STANDARD_ENUM_UMTS:
    {
        tOCTVC1_RUS_WNM_UMTS_CELL_STATS CellStats;

        /* Retreive the list of the BTS object index */
        if (WnmUmtsCellGetList(f_hWnm, &BtsListId) != 0)
            goto CriticalAbort;

        for (i = 0; i < BtsListId.ulNumCell; i++)
        {
            tOCTVC1_SUB_OBJECT_ID CellId = BtsListId.aCellId[i];
            /* Retreive the stats */
            if (WnmUmtsCellStatsGet(f_hWnm, CellId, &CellStats) != 0)
                goto CriticalAbort;

            BtsInfo.ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
            BtsInfo.ulBand = CellStats.ulBand;
            BtsInfo.ulRfcn = CellStats.ulUarfcn;
            BtsInfo.usPCI = CellStats.usPsc;
            BtsInfo.lSNR = CellStats.lEcI0dB;
            if (BtsCandidateListAddTo(f_pList, &BtsInfo) != 0)
            {
                goto CriticalAbort;
            }
        }
    }
    break;
    case cOCTVC1_RADIO_STANDARD_ENUM_LTE:
    {
        tOCTVC1_RUS_WNM_LTE_CELL_STATS CellStats;
        /* Retreive the list of the BTS object index */
        if (WnmLteCellGetList(f_hWnm, &BtsListId) != 0)
            goto CriticalAbort;

        for (i = 0; i < BtsListId.ulNumCell; i++)
        {
            tOCTVC1_SUB_OBJECT_ID CellId = BtsListId.aCellId[i];
            /* Retreive the stats */
            if (WnmLteCellStatsGet(f_hWnm, CellId, &CellStats) != 0)
                goto CriticalAbort;

            BtsInfo.ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_LTE;
            BtsInfo.ulBand = CellStats.ulBand;
            BtsInfo.ulRfcn = CellStats.ulEarfcn;
            BtsInfo.usPCI = CellStats.usCellIdentity;
            BtsInfo.lSNR = CellStats.lRsrpdBm;
            if (BtsCandidateListAddTo(f_pList, &BtsInfo) != 0)
            {
                goto CriticalAbort;
            }
        }
    }
    break;

    default:
        return (-1);
        break;
    }

    /* Sorting assending SNR */
    for (i = 0; i < BtsListId.ulNumCell; i++)
    {
        tBTS TpmBtsInfo;
        tOCT_INT32 lSNRMax = -100 * 512;
        tOCT_UINT32 lSNRMaxIndx = 0;

        for (j = i; j < f_pList->ulNumBts; j++)
        {
            if (f_pList->atList[j].lSNR >= lSNRMax)
            {
                lSNRMax = f_pList->atList[j].lSNR;
                lSNRMaxIndx = j;
            }
        }
        /* swap */
        TpmBtsInfo = f_pList->atList[i];
        f_pList->atList[i] = f_pList->atList[lSNRMaxIndx];
        f_pList->atList[lSNRMaxIndx] = TpmBtsInfo;
    }

    return (0);

CriticalAbort:
    return (-1);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       BlindBassSample
 *
 * Description:
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int BlindBassSample(void)
{
    tOCT_UINT32 BtsIdx;
    tOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM ulClkSourceRef = cOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM_NONE;
    tOCTVC1_RADIO_STANDARD_ENUM ulSelectedWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;

#ifdef CELL_ANTENNA_CHANGE
    if (bandIndex == 0)
    {
        tWnm.PrmSetup.ulEntryCount = 1;
        tWnm.PrmSetup.aEntryList[0].ulWaveForm = prmSetup.aEntryList[0].ulWaveForm;
        tWnm.PrmSetup.aEntryList[0].ulBand = prmSetup.aEntryList[0].ulBand;
        tWnm.PrmSetup.aEntryList[0].ulUarfcnStart = prmSetup.aEntryList[0].ulUarfcnStart;
        tWnm.PrmSetup.aEntryList[0].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[0].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[0].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[0].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
        // Set Cell Antenna PA
        setCellAntennaPAForScanner();
    }
#endif

    /* Display Application informations */
    if (ApplicationDisplayInfo() != 0)
        goto BassCriticalAbort;

    logPrint(LOG_DEBUG, "\nVerify if the clock manager is properly set to ClockReferenceSource=SOFT_APP\n\n");
    if (GetClockSyncManagerInfo(&ulClkSourceRef) != 0)
        goto BassCriticalAbort;

    if (ulClkSourceRef != cOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM_SOFT_APP)
    {
        logPrint(LOG_DEBUG, "\nError: Using BASS required to use an image with the setting 'ClockReferenceSource=SOFT_APP' in the configuration file octbtsxxxx.ini\n");
        goto SampleEnd;
    }

    // Verify a Wnm application is alreasy openned
    if (CheckWnm() != 0)
    {
        logPrint(LOG_DEBUG, "CheckWnm Failed \n");
        goto BassCriticalAbort;
    }

cellAntennaMonitoringForBass:
    // Open Wireless Networl Monitor
    if (WnmOpen(&tWnm.hWnm) != 0)
    {
        logPrint(LOG_DEBUG, "WnmOpen Failed \n");
        goto BassCriticalAbort;
    }
    OctOsalSleepMs(100);

    // Setup the monitor
    tWnm.PrmSetup.ulSearchMode = cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_DETECTION_ONLY;
    tWnm.PrmSetup.ulRxGainControlMode = cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC;
    tWnm.PrmSetup.lManualRxGaindB = cOCTVC1_RUS_WNM_RXMODE_MGC_GAIN_USE_DEFAULT;
    tWnm.PrmSetup.ulReserved0 = 0;
    // Events setup
    tWnm.PrmSetup.ubyEventCellStatsEnableFlag = 0;
    tWnm.PrmSetup.ubyEventSearchStatsRate = 0;
    tWnm.PrmSetup.ulUserEventId = 0;
    tWnm.PrmSetup.SelectiveSibDecoding.ulEnableFlag = cOCT_FALSE;
    tWnm.PrmSetup.SelectiveSibDecoding.ulGsmSIMask = cOCTVC1_RUS_WNM_SYSINFO_GSM_TYPE_MASK_NONE;
    tWnm.PrmSetup.SelectiveSibDecoding.ulUmtsSibMask = cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_NONE;
    tWnm.PrmSetup.SelectiveSibDecoding.ulLteSibMask = cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_NONE;

    if (WnmSetup(tWnm.hWnm, &tWnm.PrmSetup) != 0)
    {
        logPrint(LOG_DEBUG, "WnmSetup Failed \n");
        goto BassCriticalAbort;
    }

    /* Launch the monitoring */
    if (WnmStart(tWnm.hWnm) != 0)
    {
        logPrint(LOG_DEBUG, "WnmStart Failed \n");
        goto BassCriticalAbort;
    }

    /* Monitoring by polling */
    do
    {
        if (exitRatBassMode)
        {
            exitRatBassMode = 0;
            WnmClose(tWnm.hWnm);
            memset(&tWnm, 0, sizeof(tWnm));
            memset(&tBass, 0, sizeof(tBass));
            ExitApplication();
            pthread_exit(NULL);
        }

        if (WnmGetStatus(tWnm.hWnm, &tWnm.tStatus) != 0)
            goto BassCriticalAbort;
        WnmDisplayStatus(&tWnm.tStatus);

        if (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_ABORT_ERROR)
            goto BassCriticalAbort;
        OctOsalSleepMs(1000);
    } while (tWnm.tStatus.ulState != cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_DONE);

    /* Retreive all base station candidate and build a list */
    if (BtsCandidateListGet(tWnm.hWnm, ulSelectedWaveForm, &tBass.tBlindBassGsmBtsList) != 0)
    {
        logPrint(LOG_DEBUG, "Unable to get the base station list\n");
        goto BassCriticalAbort;
    }

    /* Stop monitoring */
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_DEBUG, "WnmStop Failed \n");
        goto BassCriticalAbort;
    }

    /* Close the application */
    if (WnmClose(tWnm.hWnm) != 0)
    {
        logPrint(LOG_DEBUG, "WnmClose Failed \n");
        goto BassCriticalAbort;
    }

    /* Bass procedure required to have at least one base station detected */
    if (tBass.tBlindBassGsmBtsList.ulNumBts == 0)
    {
#ifdef CELL_ANTENNA_CHANGE
        if (bandIndex >= 0)
        {
            /* increment the band index */
            bandIndex += 1;

            if (prmSetup.ulEntryCount != bandIndex)
            {
                memset(&tWnm, 0, sizeof(tWnm));

                tWnm.PrmSetup.ulEntryCount = 1;
                tWnm.PrmSetup.aEntryList[0].ulWaveForm = prmSetup.aEntryList[bandIndex].ulWaveForm;
                tWnm.PrmSetup.aEntryList[0].ulBand = prmSetup.aEntryList[bandIndex].ulBand;
                tWnm.PrmSetup.aEntryList[0].ulUarfcnStart = prmSetup.aEntryList[bandIndex].ulUarfcnStart;
                tWnm.PrmSetup.aEntryList[0].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
                tWnm.PrmSetup.aEntryList[0].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
                tWnm.PrmSetup.aEntryList[0].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
                tWnm.PrmSetup.aEntryList[0].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;

                /* Set Cell Antenna PA */
                setCellAntennaPAForScanner();
                goto cellAntennaMonitoringForBass;
            }
            else
            {
                logPrint(LOG_DEBUG, "Bass procedure can't be done; samples end\n");
                send2gBassModeScanCompInd();
                goto SampleEnd;
            }
        }
#endif
    }

    logPrint(LOG_DEBUG, "Start Bass procedure on %d base stations\n", tBass.tBlindBassGsmBtsList.ulNumBts);

    /* Open Bass */
    if (StartBassModule() != 0)
    {
        logPrint(LOG_DEBUG, "StartBassModule Failed \n");
        goto BassCriticalAbort;
    }

    /* Verify a Bass application is alreasy openned */
    if (CheckBass() != 0)
    {
        logPrint(LOG_DEBUG, "CheckBass Failed \n");
        goto BassCriticalAbort;
    }

    if (BassOpen(&tBass.hBass) != 0)
    {
        logPrint(LOG_DEBUG, "BassOpen Failed \n");
        goto BassCriticalAbort;
    }

    /* Try to synchronize on base station */
    for (BtsIdx = 0; BtsIdx < tBass.tBlindBassGsmBtsList.ulNumBts; BtsIdx++)
    {
        tBTS BtsInfo = tBass.tBlindBassGsmBtsList.atList[BtsIdx];

        /* Select the BASS operating mode */
        tBass.tBassSetup.ulSyncMode = cOCTVC1_RUS_BASS_ATTACH_MODE_ENUM_SINGLE_ONLY;
        tBass.tBassSetup.BaseStationCfg.ulRxGainControlMode = cOCTVC1_RUS_BASS_RXMODE_ENUM_AGC;
        tBass.tBassSetup.BaseStationCfg.lManualRxGaindB = cOCTVC1_RUS_BASS_RXMODE_MGC_GAIN_USE_DEFAULT;
        tBass.tBassSetup.BaseStationCfg.ulWaveForm = BtsInfo.ulWaveForm;
        tBass.tBassSetup.BaseStationCfg.ulBand = BtsInfo.ulBand;
        tBass.tBassSetup.BaseStationCfg.ulRfcn = BtsInfo.ulRfcn;
        tBass.tBassSetup.BaseStationCfg.usPCI = 0;

        if (BassSetup(tBass.hBass, &tBass.tBassSetup) != 0)
        {
            logPrint(LOG_DEBUG, "BassSetup Failed \n");
            goto BassCriticalAbort;
        }

        /* Start synchronization procedure */
        if (BassStart(tBass.hBass) != 0)
        {
            logPrint(LOG_DEBUG, "BassStart Failed \n");
            goto BassCriticalAbort;
        }

        do
        {
            if (exitRatBassMode)
            {
                exitRatBassMode = 0;
                BassClose(tBass.hBass);
                memset(&tWnm, 0, sizeof(tWnm));
                memset(&tBass, 0, sizeof(tBass));
                ExitApplication();
                pthread_exit(NULL);
            }

            if (BassGetStatus(tBass.hBass, &tBass.tBassStatus) != 0)
            {
                logPrint(LOG_DEBUG, "BassGetStatus Failed \n");
                goto BassCriticalAbort;
            }
            OctOsalSleepMs(1000);
        } while (tBass.tBassStatus.ulState != cOCTVC1_RUS_BASS_ATTACH_STATE_ENUM_DONE);

        /* Stop synchronization procedure */
        if (BassStop(tBass.hBass) != 0)
        {
            logPrint(LOG_DEBUG, "BassStop Failed \n");
            goto BassCriticalAbort;
        }

        if (tBass.tBassStatus.BaseStationStats.ulCellSyncDiagnosticCode == cOCTVC1_RUS_BASS_SYNC_DIAG_ENUM_CELL_SYNC)
        {
            /* At this point the receiver's clock is synchroized with the base station */
            logPrint(LOG_DEBUG, "Successfully synchronized with frequency error of %4.2f Hz\n", tBass.tBassStatus.BaseStationStats.lFreqErrorHzQ2 / 4.0);
            sendBassSyncCellInfoInd(BtsInfo);
            goto BassProcedureIsDone;
        }
    } /* end bass trial */

    /* Bass procedure has been done on all BTS but synchronization is not done */
    logPrint(LOG_DEBUG, "\nUnable to synchronize with base stations\n");

BassProcedureIsDone:
    logPrint(LOG_DEBUG, "\nBass processing done\n");
    send2gBassModeScanCompInd();
    /* Close the application */
    if (BassClose(tBass.hBass) != 0)
    {
        logPrint(LOG_DEBUG, "BassClose Failed \n");
        goto BassCriticalAbort;
    }

    memset(&tWnm, 0, sizeof(tWnm));
    memset(&tBass, 0, sizeof(tBass));
    return (0);

SampleEnd:
    memset(&tWnm, 0, sizeof(tWnm));
    memset(&tBass, 0, sizeof(tBass));
    return (0);

BassCriticalAbort:
    WnmClose(tWnm.hWnm);
    BassClose(tBass.hBass);
    fprintf(stderr, "Error: aborting Bass Sample\n");

    memset(&tWnm, 0, sizeof(tWnm));
    memset(&tBass, 0, sizeof(tBass));

    return (-1);
}

void *ratBassMode(void *nothing)
{
    /*
     * Display application version information.
     */
    mOCT_PRINT_APP_VERSION_INFO("2G/3G/4G BASS",
                                ((cOCTVC1_MAIN_VERSION_ID >> 25) & 0x7F),
                                ((cOCTVC1_MAIN_VERSION_ID >> 18) & 0x7F),
                                ((cOCTVC1_MAIN_VERSION_ID >> 11) & 0x7F));

    /*
     * Perform initialization tasks required by the application.
     */
    if (0 != InitApplication())
    {
        logPrint(LOG_DEBUG, ("2G BassMode : Initialization of Application is failed\n"));
        currentOneBinState = ONEBIN_STATE_IDLE;
        return;
    }

    if (0 != BlindBassSample())
    {
        logPrint(LOG_DEBUG, "2G BassMode : BassSample Failed\n");
        currentOneBinState = ONEBIN_STATE_IDLE;
        goto ErrorHandling;
    }

    struct timespec tim, tim2;
    tim.tv_sec = 1;
    tim.tv_nsec = 500;

    while (1)
    {
        if (exitRatBassMode)
        {
            exitRatBassMode = 0;
            ExitApplication();
            pthread_exit(NULL);
        }

        nanosleep(&tim, &tim2);
    }

    return NULL;

ErrorHandling:
    ExitApplication();
    pthread_exit(NULL);
    return NULL;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmSample

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int WnmSample(void)
{
    logPrint(LOG_ERR, "wnmsample function call \n");
    tOCT_UINT32 i;
    tOCTVC1_RUS_WNM_GSM_CELL_STATS GsmCellStats;
    tOCTVC1_RUS_WNM_UMTS_CELL_STATS UmtsCellStats;
    tOCTVC1_RUS_WNM_LTE_CELL_STATS LteCellStats;
    tOCTVC1_OBJECT_CURSOR_ENUM ulGetMode;
    tOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM ulClkSourceRef = cOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM_NONE;
    unsigned bEnableExaustiveBoardInfo = 0;
    unsigned SystemClockWaitTimeMs = 5 * (4 * 250); // MSec

    char szLocalFileName[128] = {0};
    char szLocalFileName2[128] = {0};

    tOCT_UINT32 gPrevCellCount3g = 0, gPrevCellCount2g = 0, gPrevCellCount4g = 0;
    lteServingCellStruct servingCell = {0};
    unsigned bGenerateRssiBandCannelReportFile = 0;
    unsigned EventMonitoringRate = 0; // by increment of 100ms

    tOCTVC1_RUS_WNM_RSSI_BAND_STATS tRssiBandStats;

#ifdef CELL_ANTENNA_CHANGE
    if (bandIndex == 0)
    {
        tWnm.PrmSetup.ulEntryCount = 1;
        tWnm.PrmSetup.aEntryList[0].ulWaveForm = prmSetup.aEntryList[0].ulWaveForm;
        tWnm.PrmSetup.aEntryList[0].ulBand = prmSetup.aEntryList[0].ulBand;
        tWnm.PrmSetup.aEntryList[0].ulUarfcnStart = prmSetup.aEntryList[0].ulUarfcnStart;
        tWnm.PrmSetup.aEntryList[0].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[0].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[0].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[0].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
        // Set Cell Antenna PA
        setCellAntennaPAForScanner();
    }
    else
    {
        char dspType;
        if (TRUE == isItPrimaryDsp())
        {
            dspType = '1';
        }
        else
        {
            dspType = '2';
        }

        configureCellAntennaPA(dspType, 1, 't');
    }
#endif

    // Display application informations
    if (ApplicationDisplayInfo() != 0)
    {
        goto CriticalAbort;
    }

    // Verify that the clock manager's clock source
    //--------------------------------------------
    if (GetClockSyncManagerInfo(&ulClkSourceRef) != 0)
        goto CriticalAbort;

    if (ulClkSourceRef != cOCTVC1_HW_CLOCK_SYNC_MGR_SOURCE_ENUM_NONE)
    {
        tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP clkStatus;
        // Wait for the system clock is locked
        while ((SystemClockWaitTimeMs -= 250) != 0)
        {
            if (CheckClkMgrStatus(&clkStatus, g_AppCtxRus.PktApiInfo.pPktApiSess) != cOCTVC1_RC_OK)
                goto CriticalAbort;

            // Make sure the the Clk Status is locked
            if (clkStatus.ulState == cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_LOCKED)
                break;

            OctOsalSleepMs(250);
        };
        printf(" Status of the system clock is: %s\n", (SystemClockWaitTimeMs == 0) ? "NOT LOCKED (WARNING)!" : "LOCKED");
    }

    // Enable all targets events of the current session
    if (EventMonitoringRate != 0)
    {
        if (SessionEventModify(1) != 0)
        {
            logPrint(LOG_DEBUG, " Unable to enable event on the current session\n");
            goto CriticalAbort;
        }
    }

    // Verify a Wnm application is alreasy openned
    if (CheckWnm() != 0)
    {
        logPrint(LOG_DEBUG, "CheckWnm Failed \n");
        goto CriticalAbort;
    }

cellAntennaMonitoring:

    // Open Wireless Networl Monitor
    if (WnmOpen(&tWnm.hWnm) != 0)
    {
        logPrint(LOG_DEBUG, "WnmOpen Failed \n");
        goto CriticalAbort;
    }
    OctOsalSleepMs(100);

    if (FXL_SCAN_TYPE_RSSI == scanMode)
    {
        // Setup the monitor
        tWnm.PrmSetup.ulSearchMode = cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_RSSI_ONLY; // Performs BCH information decoding
        bGenerateRssiBandCannelReportFile = 1;
    }
    else
    {
        // Setup the monitor
        tWnm.PrmSetup.ulSearchMode = cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE; // Performs BCH information decoding
    }

    // Events setup
    tWnm.PrmSetup.ubyEventCellStatsEnableFlag = (EventMonitoringRate != 0); // Performs notification when a base station is detected (see cOCTVC1_RUS_MSG_WNM_*_CELL_STATS_EID)
    tWnm.PrmSetup.ubyEventSearchStatsRate = EventMonitoringRate;            // Performs periodic notification of search statistic (see cOCTVC1_RUS_MSG_WNM_SEARCH_STATS_EID)
    tWnm.PrmSetup.ulUserEventId = 777;                                      // This optional user identifier will be forwarded by each events
    tWnm.PrmSetup.SelectiveSibDecoding.ulEnableFlag = cOCT_TRUE;
    tWnm.PrmSetup.ulReserved0 = 0;
    tWnm.PrmSetup.ulRxGainControlMode = cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC;
    tWnm.PrmSetup.lManualRxGaindB = cOCTVC1_RUS_WNM_RXMODE_MGC_GAIN_USE_DEFAULT;

    tWnm.PrmSetup.SelectiveSibDecoding.ulGsmSIMask = cOCTVC1_RUS_WNM_SYSINFO_GSM_TYPE_MASK_SI_1 | cOCTVC1_RUS_WNM_SYSINFO_GSM_TYPE_MASK_SI_2 |
                                                     cOCTVC1_RUS_WNM_SYSINFO_GSM_TYPE_MASK_SI_3 | cOCTVC1_RUS_WNM_SYSINFO_GSM_TYPE_MASK_SI_4;

    tWnm.PrmSetup.SelectiveSibDecoding.ulUmtsSibMask = cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB1 | cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB3 |
                                                       cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB4 | cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB5 |
                                                       cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB11 | cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB12 |
                                                       cOCTVC1_RUS_WNM_SYSINFO_UMTS_TYPE_MASK_SIB19;

    tWnm.PrmSetup.SelectiveSibDecoding.ulLteSibMask = cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB2 | cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB3 |
                                                      cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB4 | cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB5 |
                                                      cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB6 | cOCTVC1_RUS_WNM_SYSINFO_LTE_TYPE_MASK_SIB7;

ContinuousMonitoring: // continuous scan

    //logPrint(LOG_ERR, "Inside ContinuousMonitoring section \n");
    if (WnmSetup(tWnm.hWnm, &tWnm.PrmSetup) != 0)
    {
        logPrint(LOG_ERR, "WnmSetup Failed \n");
        goto CriticalAbort;
    }
    // Launch the monitoring
    if (WnmStart(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "WnmStart Failed \n");
        goto CriticalAbort;
    }
    // Monitoring
    do
    {
        // logPrint(LOG_ERR, "Inside ContinuousMonitoring section :: Inside do while loop  exitRatListenMode = %d \n", exitRatListenMode);
        if (exitRatListenMode)
        {
#ifdef CELL_ANTENNA_CHANGE
            bandIndex = -1;
#endif
            exitRatListenMode = 0;
            WnmClose(tWnm.hWnm);
            memset(&tWnm, 0, sizeof(tWnm));
            ExitApplication();
            pthread_exit(NULL);
        }

        if (WnmGetStatus(tWnm.hWnm, &tWnm.tStatus) != 0)
            goto CriticalAbort;
        WnmDisplayStatus(&tWnm.tStatus);

        if (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_ABORT_ERROR)
        {
            logPrint(LOG_ERR, "MonitorAbortError line = %d\n", __LINE__);
            goto MonitorAbortError;
        }
        if (cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_RSSI_ONLY != tWnm.PrmSetup.ulSearchMode)
        {
            /* Cell Found */
            if ((tWnm.tStatus.ul3GBaseStationCnt - gPrevCellCount3g) > 0)
            {
                if (WnmUmtsCellGetList(tWnm.hWnm, &tWnm.UmtsBtsList) != 0)
                    goto CriticalAbort;

                for (i = 0; i < (tWnm.tStatus.ul3GBaseStationCnt - gPrevCellCount3g); i++)
                {
                    tOCTVC1_SUB_OBJECT_ID CellId = tWnm.UmtsBtsList.aCellId[gPrevCellCount3g + i];

                    // Retreive the stats
                    if (WnmUmtsCellStatsGet(tWnm.hWnm, CellId, &UmtsCellStats) != 0)
                        goto CriticalAbort;

                    WnmUmtsCellStatsDisplay(CellId, &UmtsCellStats);

                    // Read the BCH from the file
                    sprintf(szLocalFileName, "%s%02d.bin", "/tmp/umts_customer_bch", CellId);
                    if (ReadFileFromTarget(szLocalFileName, UmtsCellStats.szBchPayloadFileName, 0) != 0)
                        goto CriticalAbort;

                    // BCH file sanity check
                    if (UmtsCellStats.usBchPayloadBlockCnt != 0)
                        WnmUmtsBCHBufferAnalysis(szLocalFileName, UmtsCellStats.usBchPayloadBlockCnt, 0);

                    // WnmUmtsBCHBufferAnalysis(CellId, szLocalFileName);
                    // if(UmtsCellStats.usBchPayloadBlockCnt!= 0)
                    //     WnmUmtsBCHBufferAnalysis(szLocalFileName,UmtsCellStats.usBchPayloadBlockCnt,0);

                    // Fill Physical Cell Id
                    g3gListenModeSysInfo.pci = CellId;

                    // updated the scan result
                    store3gScanResult();

                    // Send Indication to GUI
                    sendListenModeSysInfoInd();

                    if (1 == gAutoConfigCfg.autoConfig3g)
                    {
                        bContinuousMonitoringEnable = 1;

                        if (1 == checkPlmnFound())
                        {
                            addAutoCfgScanResult();
                            //                               autoConfigureUmtsIdCatcherParameters ();
                            //                               break;
                        }
                    }

                    initializeSystemInformationInd();
                }

                gPrevCellCount3g = tWnm.tStatus.ul3GBaseStationCnt;
            }
            else if ((tWnm.tStatus.ul2GBaseStationCnt - gPrevCellCount2g) > 0)
            {
                // Retrieve the list of the BTS object index
                if (WnmGsmCellGetList(tWnm.hWnm, &tWnm.GsmBtsList) != 0)
                    goto CriticalAbort;

                for (i = 0; i < (tWnm.tStatus.ul2GBaseStationCnt - gPrevCellCount2g); i++)
                {
                    tOCTVC1_SUB_OBJECT_ID CellId = tWnm.GsmBtsList.aCellId[gPrevCellCount2g + i];

                    // Retreive the stats
                    if (WnmGsmCellStatsGet(tWnm.hWnm, CellId, &GsmCellStats) != 0)
                        goto CriticalAbort;

                    WnmGsmCellStatsDisplay(CellId, &GsmCellStats);

                    // Read the BCCH from the file
                    sprintf(szLocalFileName, "%s%02d.bin", "/tmp/gsm_customer_bcch", CellId);
                    if (ReadFileFromTarget(szLocalFileName, (char *)GsmCellStats.szBcchPayloadFileName, 0) != 0)
                        goto CriticalAbort;
                    // BCCH file sanity check
                    if (GsmCellStats.usBcchPayloadBlockCnt != 0)
                        WnmGsmBCCHBufferAnalysis(szLocalFileName, CellId, &GsmCellStats);
                }

                gPrevCellCount2g = tWnm.tStatus.ul2GBaseStationCnt;
            }
            else if ((tWnm.tStatus.ul4GBaseStationCnt - gPrevCellCount4g) > 0)
            {
                // Retrieve the list of the BTS object index
                if (WnmLteCellGetList(tWnm.hWnm, &tWnm.LteBtsList) != 0)
                    goto CriticalAbort;

                for (i = 0; i < (tWnm.tStatus.ul4GBaseStationCnt - gPrevCellCount4g); i++)
                {
                    tOCTVC1_SUB_OBJECT_ID CellId = tWnm.LteBtsList.aCellId[gPrevCellCount4g + i];

                    // Retreive the stats
                    if (WnmLteCellStatsGet(tWnm.hWnm, CellId, &LteCellStats) != 0)
                        goto CriticalAbort;

                    WnmLteCellStatsDisplay(CellId, &LteCellStats);

                    // Read the BCCH from the file
                    sprintf(szLocalFileName, "%s%02d.bin", "/tmp/Lte_customer_bch", CellId);
                    if (ReadFileFromTarget(szLocalFileName, LteCellStats.szBchPayloadFileName, 0) != 0)
                        goto CriticalAbort;

                    // BCCH file sanity check
                    if (LteCellStats.usBchPayloadBlockCnt != 0)
                    {
                        if (((LteCellStats.byWaveFormMode == 0) && (LteCellStats.ulBand < cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_33)) ||
                            ((LteCellStats.byWaveFormMode == 1) && ((LteCellStats.ulBand >= cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_33) || (LteCellStats.ulBand <= cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_51))) ||
                            ((LteCellStats.byWaveFormMode == 0) && ((LteCellStats.ulBand >= cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_65) || (LteCellStats.ulBand <= cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_255))))
                        {
                            servingCell.sfNsfcellId = LteCellStats.usCellIdentity;
                            servingCell.rssi = (double)(LteCellStats.lRssidBm / 512.0);
                            servingCell.rsrp = (double)(LteCellStats.lRsrpdBm / 512.0);
                            servingCell.rsrq = (double)(LteCellStats.lRsrqdB / 512.0);
                            servingCell.mib.dlBw = LteCellStats.ulBandWidth;
                            servingCell.mib.phichDuration = LteCellStats.byPhichDuration;
                            servingCell.mib.phichResource = LteCellStats.byPhichGroup;
                            servingCell.earfcn = LteCellStats.ulEarfcn;
                            servingCell.noOfTxAntennaPorts = LteCellStats.byNumTxAntennaPorts;
                            servingCell.relativeD = LteCellStats.lRelativeD;
                            WnmLteBCHBufferAnalysis(szLocalFileName, LteCellStats.usBchPayloadBlockCnt, &servingCell);
                        }
                        else
                        {
                            logPrint(LOG_ERR, "Found cell in %s Mode for Wrong Band = %s\n", (LteCellStats.byWaveFormMode == 0) ? "FDD" : "TDD",
                                     WnmGetBandString(LteCellStats.ulWaveForm, LteCellStats.ulBand));
                        }
                    }
                }

                gPrevCellCount4g = tWnm.tStatus.ul4GBaseStationCnt;
            }
        }

        OctOsalSleepMs(250);
        // logPrint(LOG_ERR, "(tWnm.tStatus.ulState) = %d, cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_DONE = %d \n", (tWnm.tStatus.ulState), cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_DONE);
    } while (tWnm.tStatus.ulState != cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_DONE);

    if (cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_RSSI_ONLY == tWnm.PrmSetup.ulSearchMode)
    {
        ulGetMode = cOCTVC1_OBJECT_CURSOR_ENUM_FIRST;

        for (i = 0; i < cOCTVC1_RUS_WNM_MAX_NUM_RSSI_CHX_BY_DB; i++)
        {
            // Retreive and disply Rssi's band statistic
            if (0 != WnmRssiBandStatsGet(tWnm.hWnm, &ulGetMode, &tRssiBandStats))
                goto CriticalAbort;

            WnmRssiBandStatsDisplay(i, &tRssiBandStats);

            // Retreive Rssi band channel data of the given ulWaveForm and ulBand identifiers
            if (WnmRssiBandChannelGetData(tWnm.hWnm, &tRssiBandStats.BandId, &tWnm.RssiBandChannel) != 0)
                goto CriticalAbort;

            // Send RSSI Scan Result to GUI
            sendRssiScanResultInd(tWnm.RssiBandChannel);

            if (ulGetMode == cOCTVC1_OBJECT_CURSOR_ENUM_DONE)
                break;
        }
    }

#ifdef CELL_ANTENNA_CHANGE
    if (bandIndex >= 0)
    {
        // increment the band index
        bandIndex += 1;

        if (prmSetup.ulEntryCount != bandIndex)
        {
            // Stop the application
            if (WnmStop(tWnm.hWnm) != 0)
                goto CriticalAbort;

            // Close the application
            if (WnmClose(tWnm.hWnm) != 0)
                goto CriticalAbort;

            memset(&tWnm, 0, sizeof(tWnm));
            memset(&GsmCellStats, 0, sizeof(GsmCellStats));
            memset(&UmtsCellStats, 0, sizeof(UmtsCellStats));
            memset(&LteCellStats, 0, sizeof(LteCellStats));
            gPrevCellCount3g = 0;
            gPrevCellCount2g = 0;
            gPrevCellCount4g = 0;

            tWnm.PrmSetup.ulEntryCount = 1;
            tWnm.PrmSetup.aEntryList[0].ulWaveForm = prmSetup.aEntryList[bandIndex].ulWaveForm;
            tWnm.PrmSetup.aEntryList[0].ulBand = prmSetup.aEntryList[bandIndex].ulBand;
            tWnm.PrmSetup.aEntryList[0].ulUarfcnStart = prmSetup.aEntryList[bandIndex].ulUarfcnStart;
            tWnm.PrmSetup.aEntryList[0].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
            tWnm.PrmSetup.aEntryList[0].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
            tWnm.PrmSetup.aEntryList[0].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
            tWnm.PrmSetup.aEntryList[0].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;

            // Set Cell Antenna PA
            setCellAntennaPAForScanner();
            goto cellAntennaMonitoring;
        }
    }
#endif

    // Retreive the list of the BTS object index
    // Echo if nothing is found
    if ((tWnm.GsmBtsList.ulNumCell == 0) && (tWnm.UmtsBtsList.ulNumCell == 0) && (tWnm.LteBtsList.ulNumCell == 0))
    {
        logPrint(LOG_DEBUG, "No cells found\n");
    }

    // Stop monitoring
MonitorAbortError:

    // Stop monitoring
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "Dev team support Calling critical abort = %d \n", __LINE__);
        goto CriticalAbort;
    }

    if (--bContinuousMonitoringEnable)
    {
        logPrint(LOG_ERR, "Dev team support find3gCell UMTS scan bContinuousMonitoringEnable = %d , lineno = %d\n", bContinuousMonitoringEnable, __LINE__);
        goto ContinuousMonitoring;
    }

#ifdef CELL_ANTENNA_CHANGE
    currentBand = 0xFF;
    bandIndex = -1;
#endif

    /* Close the application */
    if (WnmClose(tWnm.hWnm) != 0)
        goto CriticalAbort;

    /* CLose the Packet API */
    ExitApplication();

#ifdef CELL_ANTENNA_CHANGE
    currentBand = 0xFF;
    bandIndex = -1;
#endif

    // logPrint(LOG_DEBUG, "\n Sending the Scan Complete Indication to the external Client\n");
    if (cOCTVC1_RADIO_STANDARD_ENUM_UMTS == tWnm.PrmSetup.aEntryList[0].ulWaveForm)
    {
        if ((1 == gAutoConfigCfg.autoConfig3g) && (NULL == pAutoCfgScanResult))
        {
            memset(&tWnm, 0, sizeof(tWnm));
            currentOneBinState = ONEBIN_STATE_IDLE;
            send3gListenModeScanCompInd(1);
        }
        else
        {
            memset(&tWnm, 0, sizeof(tWnm));
            send3gListenModeScanCompInd(0);
        }
    }
    else if (cOCTVC1_RADIO_STANDARD_ENUM_GSM == tWnm.PrmSetup.aEntryList[0].ulWaveForm)
    {
        memset(&tWnm, 0, sizeof(tWnm));
        send2gListenModeScanCompInd();
    }
    else if (cOCTVC1_RADIO_STANDARD_ENUM_LTE == tWnm.PrmSetup.aEntryList[0].ulWaveForm)
    {
        memset(&tWnm, 0, sizeof(tWnm));
        send4gListenModeScanCompInd();
    }

    return (0);

CriticalAbort:
    WnmStop(tWnm.hWnm); // Pic-six dev changes
    WnmClose(tWnm.hWnm);
    fprintf(stderr, "Error: aborting Rus Sample\n");

#ifdef CELL_ANTENNA_CHANGE
    currentBand = 0xFF;
#endif

    memset(&tWnm, 0, sizeof(tWnm));
    ExitApplication();

    return (-1);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ratListenMode

Description:    Main program.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

void *ratListenMode(void *nothing)
{
    /*
     * Display application version information.
     */
    mOCT_PRINT_APP_VERSION_INFO("2G/3G/4G SLS",
                                ((cOCTVC1_MAIN_VERSION_ID >> 25) & 0x7F),
                                ((cOCTVC1_MAIN_VERSION_ID >> 18) & 0x7F),
                                ((cOCTVC1_MAIN_VERSION_ID >> 11) & 0x7F));

    /*
     * Perform initialization tasks required by the application.
     */
    if (scanNotRequired == 0)
    {
        if (0 != InitApplication())
        {
            logPrint(LOG_DEBUG, "2G/3G/4G ListenMode : Initialization of Application is failed\n");
            currentOneBinState = ONEBIN_STATE_IDLE;
            return NULL;
        }

        if (0 != WnmSample())
        {
            logPrint(LOG_DEBUG, "2G/3G/4G ListenMode : WnmSample Failed\n");
            currentOneBinState = ONEBIN_STATE_IDLE;
            goto ErrorHandling;
        }
    }

    if ((1 == gAutoConfigCfg.autoConfig3g) && (NULL != pAutoCfgScanResult))
    {
        if (gAutoConfigCfg.dspResetNeeded == 1)
        {
            /* Start ID Catcher */
            bootL1InProgress = 1;
            autoConfigureUmtsIdCatcherParameters();
            gAutoConfigCfg.l1Image = FXL_L1_UMTS_NATIVE_IMAGE;
            autoBootL1Image();
            createAndStart3gIdCatcherThread();
            initializeSystemInformationInd();
            bootL1InProgress = 0;
        }
        else
        {
            /* Start ID Catcher */
            bootL1InProgress = 1;
            autoConfigureUmtsIdCatcherParameters();
            gAutoConfigCfg.l1Image = FXL_L1_UMTS_NATIVE_IMAGE;
            createAndStart3gIdCatcherThread();
            initializeSystemInformationInd();
            bootL1InProgress = 0;
        }

        /* Destroy the Listen Mode Thread */
        if (scanNotRequired == 0)
        {
            memset(&g_AppCfg, 0, sizeof(g_AppCfg));
            memset(&g_AppCtxRus, 0, sizeof(g_AppCtxRus));
        }
        delAllAutoCfgScanResult();
        scanNotRequired = 0;
        pthread_exit(NULL);
    }
    else
    {
        struct timespec tim, tim2;
        tim.tv_sec = 1;
        tim.tv_nsec = 500;

        while (1)
        {
            if (exitRatListenMode)
            {
                exitRatListenMode = 0;
                pthread_exit(NULL);
            }

            nanosleep(&tim, &tim2);
        }
    }

    return NULL;

ErrorHandling:

    checkAndSetOnBinState();
    return NULL;
}

/*
 * Convert the bands.
 */
tOCT_UINT32 convertToOct3gBandFromFxL(fxL3gBandType band)
{
    tOCT_UINT32 ret = 0xFF;

    switch (band)
    {
    case FXL_UMTS_BAND_1:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_1;
        break;

    case FXL_UMTS_BAND_2:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_2;
        break;

    case FXL_UMTS_BAND_3:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_3;
        break;

    case FXL_UMTS_BAND_4:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_4;
        break;

    case FXL_UMTS_BAND_5:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_5;
        break;

    case FXL_UMTS_BAND_6:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_6;
        break;

    case FXL_UMTS_BAND_7:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_7;
        break;

    case FXL_UMTS_BAND_8:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_8;
        break;

    case FXL_UMTS_BAND_9:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_9;
        break;

    case FXL_UMTS_BAND_10:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_10;
        break;

    case FXL_UMTS_BAND_11:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_11;
        break;

    case FXL_UMTS_BAND_12:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_12;
        break;

    case FXL_UMTS_BAND_13:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_13;
        break;

    case FXL_UMTS_BAND_14:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_14;
        break;

    case FXL_UMTS_BAND_19:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_19;
        break;

    case FXL_UMTS_BAND_20:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_20;
        break;

    case FXL_UMTS_BAND_21:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_21;
        break;

    case FXL_UMTS_BAND_22:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_22;
        break;

    case FXL_UMTS_BAND_25:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_25;
        break;

    case FXL_UMTS_BAND_26:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_26;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_2:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_2;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_4:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_4;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_5:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_5;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_6:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_6;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_7:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_7;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_10:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_10;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_12:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_12;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_13:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_13;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_14:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_14;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_19:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_19;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_25:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_25;
        break;

    case FXL_UMTS_ADDITIONAL_BAND_26:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_26;
        break;

    default:
        logPrint(LOG_DEBUG, "3GListenMode : Invalid Band configured\n");
        break;
    }

    return ret;
}

/*
 * Configure Listen Mode parameters
 */
void configureListenMode(fxL3gListenModeConfigureBandsCmd *cmd)
{
    int i = 0;
    // Select the operating mode of the receiver gain

#ifdef CELL_ANTENNA_CHANGE
    prmSetup.ulEntryCount = cmd->noOfBandsToScan;
    bandIndex = 0;
#else
    tWnm.PrmSetup.ulEntryCount = cmd->noOfBandsToScan;
#endif

    bContinuousMonitoringEnable = cmd->noOfInterationsToScan;

    
#ifdef CELL_ANTENNA_CHANGE
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        prmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
        prmSetup.aEntryList[i].ulBand = convertToOct3gBandFromFxL(cmd->band[i]);
        prmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        prmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        prmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        prmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        prmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#else
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
        tWnm.PrmSetup.aEntryList[i].ulBand = convertToOct3gBandFromFxL(cmd->band[i]);
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#endif
}

/*
 *  * find band using UARFCN
 *   */

fxL3gBandType CalculateBandUsingUarfcn(unsigned int dlUarfcn)
{
    /* Band1 */
    if ((dlUarfcn >= 10562) && (dlUarfcn <= 10838))
    {
        return FXL_UMTS_BAND_1;
    }
    /* Band2 */
    else if ((dlUarfcn >= 9662) && (dlUarfcn <= 9938))
    {
        return FXL_UMTS_BAND_2;
    }
    /* Band3 */
    else if ((dlUarfcn >= 1162) && (dlUarfcn <= 1513))
    {
        return FXL_UMTS_BAND_3;
    }
    /* Band4 */
    else if ((dlUarfcn >= 1537) && (dlUarfcn <= 1738))
    {
        return FXL_UMTS_BAND_4;
    }
    /* Band5 */
    else if ((dlUarfcn >= 4357) && (dlUarfcn <= 4458))
    {
        return FXL_UMTS_BAND_5;
    }
    /* Band7 */
    else if ((dlUarfcn >= 2237) && (dlUarfcn <= 2463))
    {
        return FXL_UMTS_BAND_7;
    }
    /* Band8 */
    else if ((dlUarfcn >= 2937) && (dlUarfcn <= 3088))
    {
        return FXL_UMTS_BAND_8;
    }
    /* Band9 */
    else if ((dlUarfcn >= 9237) && (dlUarfcn <= 9387))
    {
        return FXL_UMTS_BAND_9;
    }
    /* Band10 */
    else if ((dlUarfcn >= 3112) && (dlUarfcn <= 3388))
    {
        return FXL_UMTS_BAND_10;
    }
    /* Band11 */
    else if ((dlUarfcn >= 3712) && (dlUarfcn <= 3787))
    {
        return FXL_UMTS_BAND_11;
    }
    /* Band12 */
    else if ((dlUarfcn >= 3842) && (dlUarfcn <= 3903))
    {
        return FXL_UMTS_BAND_12;
    }
    /* Band13 */
    else if ((dlUarfcn >= 4017) && (dlUarfcn <= 4043))
    {
        return FXL_UMTS_BAND_13;
    }
    /* Band14 */
    else if ((dlUarfcn >= 4117) && (dlUarfcn <= 4143))
    {
        return FXL_UMTS_BAND_14;
    }
    /* Band19 */
    else if ((dlUarfcn >= 712) && (dlUarfcn <= 763))
    {
        return FXL_UMTS_BAND_19;
    }
    /* Band20 */
    else if ((dlUarfcn >= 4512) && (dlUarfcn <= 4638))
    {
        return FXL_UMTS_BAND_20;
    }
    /* Band21 */
    else if ((dlUarfcn >= 862) && (dlUarfcn <= 912))
    {
        return FXL_UMTS_BAND_21;
    }
    /* Band22 */
    else if ((dlUarfcn >= 4662) && (dlUarfcn <= 5038))
    {
        return FXL_UMTS_BAND_22;
    }
    /* Band25 */
    else if ((dlUarfcn >= 5112) && (dlUarfcn <= 5413))
    {
        return FXL_UMTS_BAND_25;
    }
    /* Band26 */
    else if ((dlUarfcn >= 5762) && (dlUarfcn <= 5913))
    {
        return FXL_UMTS_BAND_26;
    }
    /* Band2 Additional */
    else if ((dlUarfcn == 412) || (dlUarfcn == 437) ||
             (dlUarfcn == 462) || (dlUarfcn == 487) ||
             (dlUarfcn == 512) || (dlUarfcn == 537) ||
             (dlUarfcn == 562) || (dlUarfcn == 587) ||
             (dlUarfcn == 612) || (dlUarfcn == 637) ||
             (dlUarfcn == 662) || (dlUarfcn == 687))
    {
        return FXL_UMTS_ADDITIONAL_BAND_2;
    }
    /* Band5 Additional */
    else if ((dlUarfcn == 1007) || (dlUarfcn == 1012) ||
             (dlUarfcn == 1032) || (dlUarfcn == 1037) ||
             (dlUarfcn == 1062) || (dlUarfcn == 1087))
    {
        return FXL_UMTS_ADDITIONAL_BAND_5;
    }
    /* Band4 Additional */
    else if ((dlUarfcn == 1887) || (dlUarfcn == 1912) ||
             (dlUarfcn == 1937) || (dlUarfcn == 1962) ||
             (dlUarfcn == 1987) || (dlUarfcn == 2012) ||
             (dlUarfcn == 2037) || (dlUarfcn == 2062) ||
             (dlUarfcn == 2087))
    {
        return FXL_UMTS_ADDITIONAL_BAND_4;
    }
    /* Band7 Additional */
    else if ((dlUarfcn == 2587) || (dlUarfcn == 2612) ||
             (dlUarfcn == 2637) || (dlUarfcn == 2662) ||
             (dlUarfcn == 2687) || (dlUarfcn == 2712) ||
             (dlUarfcn == 2737) || (dlUarfcn == 2762) ||
             (dlUarfcn == 2787) || (dlUarfcn == 2812) ||
             (dlUarfcn == 2837) || (dlUarfcn == 2862) ||
             (dlUarfcn == 2887) || (dlUarfcn == 2912))
    {
        return FXL_UMTS_ADDITIONAL_BAND_7;
    }
    /* Band10 Additional */
    else if ((dlUarfcn == 3412) || (dlUarfcn == 3437) ||
             (dlUarfcn == 3462) || (dlUarfcn == 3487) ||
             (dlUarfcn == 3512) || (dlUarfcn == 3537) ||
             (dlUarfcn == 3562) || (dlUarfcn == 3587) ||
             (dlUarfcn == 3612) || (dlUarfcn == 3637) ||
             (dlUarfcn == 3662) || (dlUarfcn == 3687))
    {
        return FXL_UMTS_ADDITIONAL_BAND_10;
    }
    /* Band12 Additional */
    else if ((dlUarfcn == 3932) || (dlUarfcn == 3957) ||
             (dlUarfcn == 3962) || (dlUarfcn == 3987) ||
             (dlUarfcn == 3992))
    {
        return FXL_UMTS_ADDITIONAL_BAND_12;
    }
    /* Band13 Additional */
    else if ((dlUarfcn == 4067) || (dlUarfcn == 4092))
    {
        return FXL_UMTS_ADDITIONAL_BAND_13;
    }
    /* Band14 Additional */
    else if ((dlUarfcn == 4167) || (dlUarfcn == 4192))
    {
        return FXL_UMTS_ADDITIONAL_BAND_14;
    }
    /* Band19 Additional */
    else if ((dlUarfcn == 787) || (dlUarfcn == 812) ||
             (dlUarfcn == 837))
    {
        return FXL_UMTS_ADDITIONAL_BAND_19;
    }
    /* Band25 Additional */
    else if ((dlUarfcn == 6292) || (dlUarfcn == 6317) ||
             (dlUarfcn == 6342) || (dlUarfcn == 6367) ||
             (dlUarfcn == 6392) || (dlUarfcn == 6417) ||
             (dlUarfcn == 6442) || (dlUarfcn == 6467) ||
             (dlUarfcn == 6492) || (dlUarfcn == 6517) ||
             (dlUarfcn == 6542) || (dlUarfcn == 6567) ||
             (dlUarfcn == 6592))
    {
        return FXL_UMTS_ADDITIONAL_BAND_25;
    }
    /* Band26 Additional */
    else if ((dlUarfcn == 5937) || (dlUarfcn == 5962) ||
             (dlUarfcn == 5987) || (dlUarfcn == 5992) ||
             (dlUarfcn == 6012) || (dlUarfcn == 6017) ||
             (dlUarfcn == 6037) || (dlUarfcn == 6042) ||
             (dlUarfcn == 6062) || (dlUarfcn == 6067) ||
             (dlUarfcn == 6087))
    {
        return FXL_UMTS_ADDITIONAL_BAND_26;
    }
    /* Unsupported Band */
    else
    {
        return FXL_UMTS_MAX_BAND_SUPPORTED;
    }
}

/*
 * Priority scan Listen mode parametes
 */
fxLResult priorityScanListenMode(fxL3gListenModeConfigureUarfcnCmd *cmd)
{
    int i = 0;

    tOCTVC1_RUS_WNM_RXMODE_ENUM ulRxGainControlMode = cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC; // cOCTVC1_RUS_WNM_RXMODE_ENUM_MGC; //cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC

#ifdef CELL_ANTENNA_CHANGE
    bandIndex = -1;
#endif

    tWnm.PrmSetup.ulEntryCount = cmd->noOfUarfcn;
    bContinuousMonitoringEnable = cmd->noOfIteration;

    for (i = 0; i < cmd->noOfUarfcn; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
        tWnm.PrmSetup.aEntryList[i].ulBand = convertToOct3gBandFromFxL(cmd->bandUarfcn[i].band);
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cmd->bandUarfcn[i].uarfcn;
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = cmd->bandUarfcn[i].uarfcn;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
    return FXL_SUCCESS;
}

/*
 * Configure Listen Mode
 */
void autoConfigureListenMode(fxL3gAutoConfigCmd *cmd)
{
    int i = 0;
    tOCTVC1_RUS_WNM_RXMODE_ENUM ulRxGainControlMode = cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC; // cOCTVC1_RUS_WNM_RXMODE_ENUM_MGC; //cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC

#ifdef CELL_ANTENNA_CHANGE
    prmSetup.ulEntryCount = cmd->noOfBandsToScan;
    bandIndex = 0;
#else
    tWnm.PrmSetup.ulEntryCount = cmd->noOfBandsToScan;
#endif

    bContinuousMonitoringEnable = 1;

#ifdef CELL_ANTENNA_CHANGE
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        prmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
        prmSetup.aEntryList[i].ulBand = convertToOct3gBandFromFxL(cmd->band[i]);
        prmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        prmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        prmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        prmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        prmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#else
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
        tWnm.PrmSetup.aEntryList[i].ulBand = convertToOct3gBandFromFxL(cmd->band[i]);
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#endif
}

/*
 * Stop Bass Mode Application
 */
fxLResult stopBassModeApplication(void)
{
    if (0 != destroyBassModeThread())
    {
        return (FXL_FAILURE);
    }

    memset(&g_AppCfg, 0, sizeof(g_AppCfg));
    memset(&g_AppCtxRus, 0, sizeof(g_AppCtxRus));

    g_AppCtxRus.PktApiInfo.pPktApiSess = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiCnct = NULL;
    return (FXL_SUCCESS);
}

/*
 * Stop Listen Mode Application
 */
fxLResult stopListenModeApplication(void)
{
#if 0
   if (0 != CheckWnm ())
   {
      return (FXL_FAILURE);
   }
   ExitApplication();
#endif

    if (0 != destroyListenModeThread())
    {
        return (FXL_FAILURE);
    }

    memset(&g_AppCfg, 0, sizeof(g_AppCfg));
    memset(&g_AppCtxRus, 0, sizeof(g_AppCtxRus));

    g_AppCtxRus.PktApiInfo.pPktApiSess = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxRus.PktApiInfo.pPktApiCnct = NULL;
    return (FXL_SUCCESS);
}

/*
 * Convert the bands.
 */
fxL3gBandType convertToFxLFromOctBand(tOCT_UINT32 band)
{
    fxL3gBandType ret = FXL_UMTS_MAX_BAND_SUPPORTED;

    switch (band)
    {
    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_1:
        ret = FXL_UMTS_BAND_1;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_2:
        ret = FXL_UMTS_BAND_2;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_3:
        ret = FXL_UMTS_BAND_3;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_4:
        ret = FXL_UMTS_BAND_4;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_5:
        ret = FXL_UMTS_BAND_5;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_6:
        ret = FXL_UMTS_BAND_6;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_7:
        ret = FXL_UMTS_BAND_7;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_8:
        ret = FXL_UMTS_BAND_8;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_9:
        ret = FXL_UMTS_BAND_9;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_10:
        ret = FXL_UMTS_BAND_10;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_11:
        ret = FXL_UMTS_BAND_11;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_12:
        ret = FXL_UMTS_BAND_12;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_13:
        ret = FXL_UMTS_BAND_13;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_14:
        ret = FXL_UMTS_BAND_14;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_19:
        ret = FXL_UMTS_BAND_19;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_20:
        ret = FXL_UMTS_BAND_20;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_21:
        ret = FXL_UMTS_BAND_21;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_22:
        ret = FXL_UMTS_BAND_22;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_25:
        ret = FXL_UMTS_BAND_25;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_26:
        ret = FXL_UMTS_BAND_26;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_2:
        ret = FXL_UMTS_ADDITIONAL_BAND_2;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_4:
        ret = FXL_UMTS_ADDITIONAL_BAND_4;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_5:
        ret = FXL_UMTS_ADDITIONAL_BAND_5;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_6:
        ret = FXL_UMTS_ADDITIONAL_BAND_6;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_7:
        ret = FXL_UMTS_ADDITIONAL_BAND_7;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_10:
        ret = FXL_UMTS_ADDITIONAL_BAND_10;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_12:
        ret = FXL_UMTS_ADDITIONAL_BAND_12;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_13:
        ret = FXL_UMTS_ADDITIONAL_BAND_13;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_14:
        ret = FXL_UMTS_ADDITIONAL_BAND_14;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_19:
        ret = FXL_UMTS_ADDITIONAL_BAND_19;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_25:
        ret = FXL_UMTS_ADDITIONAL_BAND_25;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_UMTS_ENUM_ADD_26:
        ret = FXL_UMTS_ADDITIONAL_BAND_26;
        break;

    default:
        logPrint(LOG_DEBUG, "3GListenMode : Invalid Band received from L1\n");
        break;
    }

    return ret;
}

void update3gScanResult(void)
{
    unsigned int idx1, idx2;
    unsigned int numOfValidOperator = gScanResult.numOfValidOperator;

    gScanResult.op[numOfValidOperator] = (OperatorScanResult *)malloc(sizeof(OperatorScanResult));
    memset(gScanResult.op[numOfValidOperator], 0, (sizeof(OperatorScanResult)));

    gScanResult.op[numOfValidOperator]->numOfPlmn = g3gListenModeSysInfo.numOfPlmn;
    gScanResult.op[numOfValidOperator]->mibPlmnIdentity = g3gListenModeSysInfo.mibPlmnIdentity;

    for (idx1 = 0; idx1 < 6; idx1++)
    {
        for (idx2 = 0; idx2 < FXL_UMTS_MAX_MCC_LEN; idx2++)
        {
            gScanResult.op[numOfValidOperator]->plmn[idx1].mcc[idx2] = 0xFFFFFFFF;
            gScanResult.op[numOfValidOperator]->plmn[idx1].mnc[idx2] = 0xFFFFFFFF;
        }
    }

    for (idx1 = 0; idx1 < gScanResult.op[numOfValidOperator]->numOfPlmn; idx1++)
    {
        gScanResult.op[numOfValidOperator]->plmn[idx1].mccLength = g3gListenModeSysInfo.plmn[idx1].mccLength;
        gScanResult.op[numOfValidOperator]->plmn[idx1].mncLength = g3gListenModeSysInfo.plmn[idx1].mncLength;

        if (g3gListenModeSysInfo.plmn[idx1].mccLength <= FXL_UMTS_MAX_MCC_LEN)
        {
            for (idx2 = 0; idx2 < gScanResult.op[numOfValidOperator]->plmn[idx1].mccLength; idx2++)
            {
                gScanResult.op[numOfValidOperator]->plmn[idx1].mcc[idx2] = g3gListenModeSysInfo.plmn[idx1].mcc[idx2];
            }
        }

        if (g3gListenModeSysInfo.plmn[idx1].mncLength <= FXL_UMTS_MAX_MNC_LEN)
        {
            for (idx2 = 0; idx2 < gScanResult.op[numOfValidOperator]->plmn[idx1].mncLength; idx2++)
            {
                gScanResult.op[numOfValidOperator]->plmn[idx1].mnc[idx2] = g3gListenModeSysInfo.plmn[idx1].mnc[idx2];
            }
        }
    }

    gScanResult.op[numOfValidOperator]->numberOfCell = 0;

    gScanResult.op[numOfValidOperator]->scanCellRslt[0] = (ScanCellInfo *)malloc(sizeof(ScanCellInfo));
    memset(gScanResult.op[numOfValidOperator]->scanCellRslt[0], 0, (sizeof(ScanCellInfo)));

    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->t3212 = g3gListenModeSysInfo.t3212;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->nmo = g3gListenModeSysInfo.nmo;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->uarfcn = g3gListenModeSysInfo.uarfcn;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->band = g3gListenModeSysInfo.band;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->rscp = g3gListenModeSysInfo.cellInfo[0].rscp;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->psc = g3gListenModeSysInfo.cellInfo[0].psc;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qQualMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qQualMin;

    if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin)
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qRxLevMin = ((g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin - 1) / 2);
    }
    else
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qRxLevMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin;
    }

    if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS)
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qHystLS = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS / 2);
    }
    else
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qHystLS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS;
    }

    if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S)
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qHyst2S = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S / 2);
    }
    else
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->qHyst2S = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S;
    }

    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->tReselS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.tReselS;
    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->maxAllowedULTxPower =
        g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.maxAllowedULTxPower;

    for (idx1 = 0; idx1 < FXL_UMTS_MAX_INTRA_FREQ_CELL_LIST; idx1++)
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->neighbourIntraPsc[idx1] = g3gListenModeSysInfo.cellInfo[0].intraFreqCellList[idx1].psc;
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->neighbourInterPsc[idx1] = g3gListenModeSysInfo.cellInfo[0].interFreqCellList[idx1].psc;
    }

    gScanResult.op[numOfValidOperator]->scanCellRslt[0]->servingCellPriority = g3gListenModeSysInfo.priorityCellInfo.servingCellPriority;

    for (idx1 = 0; idx1 < 8; idx1++)
    {
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->fddPriorityCellInfo[idx1].uarfcn = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx1].uarfcn;
        gScanResult.op[numOfValidOperator]->scanCellRslt[0]->fddPriorityCellInfo[idx1].priority = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx1].priority;
    }

    gScanResult.op[numOfValidOperator]->numberOfCell++;
    gScanResult.numOfValidOperator++;
}

int findPlmnIn3gScanResult()
{
    int found = 0;
    unsigned int idx1, idx2;

    for (idx1 = 0; idx1 < 6; idx1++)
    {
        for (idx2 = 0; idx2 < FXL_UMTS_MAX_MCC_LEN; idx2++)
        {
            if (g3gListenModeSysInfo.plmn[idx1].mcc[idx2] == FXL_UMTS_VALUE_NOT_APPLICABLE)
            {
                g3gListenModeSysInfo.plmn[idx1].mcc[idx2] = 0xFFFFFFFF;
            }

            if (g3gListenModeSysInfo.plmn[idx1].mnc[idx2] == FXL_UMTS_VALUE_NOT_APPLICABLE)
            {
                g3gListenModeSysInfo.plmn[idx1].mnc[idx2] = 0xFFFFFFFF;
            }
        }
    }

    for (idx1 = 0; idx1 < gScanResult.numOfValidOperator; idx1++)
    {
        found = 0;

        if ((gScanResult.op[idx1]->numOfPlmn == g3gListenModeSysInfo.numOfPlmn) &&
            (gScanResult.op[idx1]->scanCellRslt[0]->band == g3gListenModeSysInfo.band))
        {
            for (idx2 = 0; idx2 < gScanResult.op[idx1]->numOfPlmn; idx2++)
            {
                if ((gScanResult.op[idx1]->plmn[idx2].mccLength == g3gListenModeSysInfo.plmn[idx2].mccLength) &&
                    (gScanResult.op[idx1]->plmn[idx2].mncLength == g3gListenModeSysInfo.plmn[idx2].mncLength) &&
                    (gScanResult.op[idx1]->plmn[idx2].mcc[0] == g3gListenModeSysInfo.plmn[idx2].mcc[0]) &&
                    (gScanResult.op[idx1]->plmn[idx2].mcc[1] == g3gListenModeSysInfo.plmn[idx2].mcc[1]) &&
                    (gScanResult.op[idx1]->plmn[idx2].mcc[2] == g3gListenModeSysInfo.plmn[idx2].mcc[2]) &&
                    (gScanResult.op[idx1]->plmn[idx2].mnc[0] == g3gListenModeSysInfo.plmn[idx2].mnc[0]) &&
                    (gScanResult.op[idx1]->plmn[idx2].mnc[1] == g3gListenModeSysInfo.plmn[idx2].mnc[1]) &&
                    (gScanResult.op[idx1]->plmn[idx2].mnc[2] == g3gListenModeSysInfo.plmn[idx2].mnc[2]))
                {
                    found++;
                }
            }
        }

        if (found == gScanResult.op[idx1]->numOfPlmn)
        {
            break;
        }

        found = 0;
    }

    if (found != 0)
    {
        return idx1;
    }
    else
    {
        return -1;
    }
}

void store3gScanResult(void)
{
    int currentOpertaor, numberOfCell;

    if (gScanResult.numOfValidOperator == 0)
    {
        update3gScanResult();
    }
    else
    {
        currentOpertaor = findPlmnIn3gScanResult();

        logPrint(LOG_DEBUG, "Number of valid Opeartor %d \n", gScanResult.numOfValidOperator);
        logPrint(LOG_DEBUG, "Current Opeartor %d \n", currentOpertaor);
        logPrint(LOG_DEBUG, "Current Opeartor MCC %d %d %d \n", g3gListenModeSysInfo.plmn[0].mcc[0], g3gListenModeSysInfo.plmn[0].mcc[1], g3gListenModeSysInfo.plmn[0].mcc[2]);

        if (g3gListenModeSysInfo.plmn[0].mncLength == 2)
            logPrint(LOG_DEBUG, "Current Opeartor MNC %d %d \n", g3gListenModeSysInfo.plmn[0].mnc[0], g3gListenModeSysInfo.plmn[0].mnc[1]);
        else
            logPrint(LOG_DEBUG, "Current Opeartor MNC %d %d %d \n", g3gListenModeSysInfo.plmn[0].mnc[0], g3gListenModeSysInfo.plmn[0].mnc[1], g3gListenModeSysInfo.plmn[0].mnc[2]);

        if (currentOpertaor != -1)
        {
            logPrint(LOG_DEBUG, "Number of Cell %d \n", gScanResult.op[currentOpertaor]->numberOfCell);

            numberOfCell = gScanResult.op[currentOpertaor]->numberOfCell;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell] = (ScanCellInfo *)malloc(sizeof(ScanCellInfo));
            memset(gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell], 0, (sizeof(ScanCellInfo)));

            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->t3212 = g3gListenModeSysInfo.t3212;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->nmo = g3gListenModeSysInfo.nmo;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->uarfcn = g3gListenModeSysInfo.uarfcn;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->band = g3gListenModeSysInfo.band;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->rscp = g3gListenModeSysInfo.cellInfo[0].rscp;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->psc = g3gListenModeSysInfo.cellInfo[0].psc;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qQualMin =
                g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qQualMin;

            if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin)
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qRxLevMin =
                    ((g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin - 1) / 2);
            }
            else
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qRxLevMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin;
            }

            if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S)
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qHyst2S =
                    (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S / 2);
            }
            else
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qHyst2S = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S;
            }

            if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS)
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qHystLS =
                    (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS / 2);
            }
            else
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->qHystLS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS;
            }

            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->tReselS =
                g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.tReselS;
            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->maxAllowedULTxPower =
                g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.maxAllowedULTxPower;

            for (unsigned idx = 0; idx < FXL_UMTS_MAX_INTRA_FREQ_CELL_LIST; idx++)
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->neighbourIntraPsc[idx] = g3gListenModeSysInfo.cellInfo[0].intraFreqCellList[idx].psc;
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->neighbourInterPsc[idx] = g3gListenModeSysInfo.cellInfo[0].interFreqCellList[idx].psc;
            }

            gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->servingCellPriority = g3gListenModeSysInfo.priorityCellInfo.servingCellPriority;

            for (unsigned idx = 0; idx < 8; idx++)
            {
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->fddPriorityCellInfo[idx].uarfcn = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].uarfcn;
                gScanResult.op[currentOpertaor]->scanCellRslt[numberOfCell]->fddPriorityCellInfo[idx].priority = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].priority;
            }

            gScanResult.op[currentOpertaor]->numberOfCell++;
        }
        else
        {
            update3gScanResult();
        }
    }
}

void delstore3gScanResult()
{
    unsigned int idx, idx1;
    for (idx = 0; idx < gScanResult.numOfValidOperator; idx++)
    {
        for (idx1 = 0; idx1 < gScanResult.op[idx]->numberOfCell; idx1++)
        {
            free(gScanResult.op[idx]->scanCellRslt[idx1]);
            gScanResult.op[idx]->scanCellRslt[idx1] = NULL;
        }
        free(gScanResult.op[idx]);
        gScanResult.op[idx] = NULL;
    }

    gScanResult.numOfValidOperator = 0;
}

unsigned int findAutoConfigPlmnInScanResult(fxL3gAutoConfigCmd *cmd)
{
    unsigned int idx, idx1, idx2, idx3;
    unsigned int found = 0;
    int lastIndexSent = -1;

    for (idx = 0; idx < gScanResult.numOfValidOperator; idx++)
    {
        for (idx3 = 0; idx3 < gScanResult.op[idx]->numOfPlmn; idx3++)
        {
            if ((gScanResult.op[idx]->plmn[idx3].mccLength == gUmtsConfig.plmn[0].mccLength) &&
                (gScanResult.op[idx]->plmn[idx3].mncLength == gUmtsConfig.plmn[0].mncLength) &&
                (gScanResult.op[idx]->plmn[idx3].mcc[0] == gUmtsConfig.plmn[0].mcc[0]) &&
                (gScanResult.op[idx]->plmn[idx3].mcc[1] == gUmtsConfig.plmn[0].mcc[1]) &&
                (gScanResult.op[idx]->plmn[idx3].mcc[2] == gUmtsConfig.plmn[0].mcc[2]) &&
                (gScanResult.op[idx]->plmn[idx3].mnc[0] == gUmtsConfig.plmn[0].mnc[0]) &&
                (gScanResult.op[idx]->plmn[idx3].mnc[1] == gUmtsConfig.plmn[0].mnc[1]) &&
                (gScanResult.op[idx]->plmn[idx3].mnc[2] == gUmtsConfig.plmn[0].mnc[2]))
            {
                for (idx1 = 0; idx1 < cmd->noOfBandsToScan; idx1++)
                {
                    for (idx2 = 0; idx2 < gScanResult.op[idx]->numberOfCell; idx2++)
                    {
                        if (gScanResult.op[idx]->scanCellRslt[idx2]->band == cmd->band[idx1])
                        {
                            if (lastIndexSent != idx)
                            {
                                addCellDetailsWithPreviousScanResult(idx);
                                lastIndexSent = idx;
                                found = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return found;
}

unsigned int fillCellSelReSelInfoFromScanResult()
{
    unsigned int idx, idx1, idx2;

    // Initialized in fillSibCfgParams. removed from here so that change of this parameter if done is not reset.
#if 0
	/* Fill the Default Value for qQualMin */
	gUmtsConfig.qQualMin   = -24;

	/* Fill the Default Value for qRxLevMin */
	gUmtsConfig.qRxLevMin  = -58;

	/* Fill the Default Value for tReselec */
	gUmtsConfig.tReselec   = 31;

	/* Fill the Default Value for qHystLS */
	gUmtsConfig.qHystLS    = 4;

	/* Fill the Default Value for qHyst2S */
	gUmtsConfig.qHyst2S    = 4;

	/* Fill the Default Value for maxUlTxPwr */
	gUmtsConfig.maxUlTxPwr = 24;
#endif

    /* Find and overwrite the Default Value with Scan Result */
    for (idx = 0; idx < gScanResult.numOfValidOperator; idx++)
    {
        for (idx2 = 0; idx2 < gScanResult.op[idx]->numOfPlmn; idx2++)
        {
            if ((gScanResult.op[idx]->plmn[idx2].mccLength == gUmtsConfig.plmn[0].mccLength) &&
                (gScanResult.op[idx]->plmn[idx2].mncLength == gUmtsConfig.plmn[0].mncLength) &&
                (gScanResult.op[idx]->plmn[idx2].mcc[0] == gUmtsConfig.plmn[0].mcc[0]) &&
                (gScanResult.op[idx]->plmn[idx2].mcc[1] == gUmtsConfig.plmn[0].mcc[1]) &&
                (gScanResult.op[idx]->plmn[idx2].mcc[2] == gUmtsConfig.plmn[0].mcc[2]) &&
                (gScanResult.op[idx]->plmn[idx2].mnc[0] == gUmtsConfig.plmn[0].mnc[0]) &&
                (gScanResult.op[idx]->plmn[idx2].mnc[1] == gUmtsConfig.plmn[0].mnc[1]) &&
                (gScanResult.op[idx]->plmn[idx2].mnc[2] == gUmtsConfig.plmn[0].mnc[2]))
            {
                for (idx1 = 0; idx1 < gScanResult.op[idx]->numberOfCell; idx1++)
                {
                    if (gUmtsConfig.dlUarfcn == gScanResult.op[idx]->scanCellRslt[idx1]->uarfcn)
                    {
                        /* Fill qQualMin */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->qQualMin)
                        {
                            gUmtsConfig.qQualMin = gScanResult.op[idx]->scanCellRslt[idx1]->qQualMin;
                        }

                        /* Fill qRxLevMin */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->qRxLevMin)
                        {
                            gUmtsConfig.qRxLevMin = gScanResult.op[idx]->scanCellRslt[idx1]->qRxLevMin;
                        }

                        /* Fill tReselS */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->tReselS)
                        {
                            gUmtsConfig.tReselec = gScanResult.op[idx]->scanCellRslt[idx1]->tReselS;
                        }

                        /* Fill qHystLS */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->qHystLS)
                        {
                            gUmtsConfig.qHystLS = gScanResult.op[idx]->scanCellRslt[idx1]->qHystLS;
                        }

                        /* Fill qHyst2S */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->qHyst2S)
                        {
                            gUmtsConfig.qHyst2S = gScanResult.op[idx]->scanCellRslt[idx1]->qHyst2S;
                        }

                        /* Fill maxAllowedULTxPower */
                        if (-1 != gScanResult.op[idx]->scanCellRslt[idx1]->maxAllowedULTxPower)
                        {
                            gUmtsConfig.maxUlTxPwr = gScanResult.op[idx]->scanCellRslt[idx1]->maxAllowedULTxPower;
                        }

                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

unsigned int fillSibCfgParams()
{
    unsigned int idx = 0;
    gUmtsConfig.mibValueTag = 1;

    gUmtsConfig.att = 0x01;
    {
        gUmtsConfig.sib1Info.cnDomainList[0] = CN_DOMAIN_IDENTITY_CS;
        gUmtsConfig.sib1Info.cnDomainList[1] = CN_DOMAIN_IDENTITY_PS;
        gUmtsConfig.sib1Info.T300 = SIB_TYPE1_UE_IDLE_TIMER_T300_4000_MS;
        gUmtsConfig.sib1Info.N300 = 7;
        gUmtsConfig.sib1Info.T301 = SIB_TYPE1_UE_CONN_TIMER_T301_2000_MS;
        gUmtsConfig.sib1Info.T302 = SIB_TYPE1_UE_CONN_TIMER_T302_100_MS;
        gUmtsConfig.sib1Info.T304 = SIB_TYPE1_UE_CONN_TIMER_T304_2000_MS;
        gUmtsConfig.sib1Info.T305 = SIB_TYPE1_UE_CONN_TIMER_T305_NO_UPDATE;
        gUmtsConfig.sib1Info.T307 = SIB_TYPE1_UE_CONN_TIMER_T307_S30;
        gUmtsConfig.sib1Info.T308 = SIB_TYPE1_UE_CONN_TIMER_T308_160_MS;
        gUmtsConfig.sib1Info.T310 = SIB_TYPE1_UE_CONN_TIMER_T310_160_MS;
        gUmtsConfig.sib1Info.T311 = SIB_TYPE1_UE_CONN_TIMER_T311_2000_MS;
        gUmtsConfig.sib1Info.T312 = 15;
        gUmtsConfig.sib1Info.N312 = SIB_TYPE1_UE_CONN_TIMER_N312_S50;
        gUmtsConfig.sib1Info.N313 = SIB_TYPE1_UE_CONN_TIMER_N313_S10;
        gUmtsConfig.sib1Info.T314 = SIB_TYPE1_UE_CONN_TIMER_T314_S4;
        gUmtsConfig.sib1Info.T315 = SIB_TYPE1_UE_CONN_TIMER_T315_S10;
        gUmtsConfig.sib1Info.N315 = SIB_TYPE1_UE_CONN_TIMER_N315_S1;
        gUmtsConfig.sib1Info.T316 = SIB_TYPE1_UE_CONN_TIMER_T316_S30;
        gUmtsConfig.sib1Info.T317 = SIB_TYPE1_UE_CONN_TIMER_T317_INFINITY4;
    }
    {
        gUmtsConfig.sib2Info.numUraIdentity = 1;
        gUmtsConfig.sib2Info.uraIdentityLst[0] = 0xd071;
    }
    {
        gUmtsConfig.sib3Info.sib4Indicator = 0;
        gUmtsConfig.sib3Info.cellId = gUmtsConfig.cellId = 0;
        gUmtsConfig.cellSelQualMeas = gUmtsConfig.sib3Info.cellSelQualMeas = 0;
        gUmtsConfig.qHyst2S = gUmtsConfig.sib3Info.qHyst2S = 4;
        gUmtsConfig.sib3Info.sIntraSearch = 5;
        gUmtsConfig.sib3Info.sInterSearch = 4;
        gUmtsConfig.sib3Info.sSearchHCS = 0;
        gUmtsConfig.sib3Info.ratSearch = 2;
        gUmtsConfig.sib3Info.sLimitSearchRAT = 0;
        gUmtsConfig.sib3Info.qQualMin = gUmtsConfig.qQualMin = -24;
        gUmtsConfig.sib3Info.qRxLevMin = gUmtsConfig.qRxLevMin = -58;
        gUmtsConfig.sib3Info.qHystLS = gUmtsConfig.qHystLS = 4;
        gUmtsConfig.tReselec = gUmtsConfig.sib3Info.tReselect = 31;
        gUmtsConfig.maxUlTxPwr = gUmtsConfig.sib3Info.maxULTxPower = 24;
        gUmtsConfig.sib3Info.cellBarredInfo.status = 1;
        gUmtsConfig.sib3Info.cellReservedForOperatiorUse = 0;
        gUmtsConfig.sib3Info.cellReservationExtention = 0;
        gUmtsConfig.sib3Info.accessClassBarredLstPres = 1;

        if (gUmtsConfig.sib3Info.accessClassBarredLstPres)
        {
            for (idx = 0; idx < 16; idx++)
            {
                gUmtsConfig.sib3Info.accessClassBarredLst[idx] = ACCESS_CLASS_NOT_BARRED;
            }
        }
    }
    { // SIB5
        gUmtsConfig.sib5Info.sib6indicator = 0;

        if (true == gUmtsConfig.pilot_boost_enable)
        {
            gUmtsConfig.sib5Info.pichPwr = -8; // DEF_PICH_Power;
            gUmtsConfig.sib5Info.aichPwr = -8; // DEF_AICH_Power;
        }
        else
        {
            gUmtsConfig.sib5Info.pichPwr = -5; // DEF_PICH_Power;
            gUmtsConfig.sib5Info.aichPwr = -5; // DEF_AICH_Power;
        }
        gUmtsConfig.sib5Info.prachPowerRampStep = 1;
        gUmtsConfig.sib5Info.prachpreambleRetransMax = 30;
    }
    { // SIB7
        gUmtsConfig.sib7Info.ulInterference = -102;
    }
    { // SIB11
        gUmtsConfig.sib11Info.sib12indicator = 0;
        gUmtsConfig.sib11Info.hscUsed = 0;
        gUmtsConfig.sib11Info.numInterFreqNeighbr = 0;
        gUmtsConfig.sib11Info.numIntraFreqNeighbr = 0;
    }
}

void addCellDetailsWithPreviousScanResult(unsigned int index)
{
    unsigned int idx, idx1, idx2 = 0;
    AutoCfgScanResult_t *pNewNode = NULL;
    AutoCfgScanResult_t *pCurNode = NULL;
    AutoCfgScanResult_t *pPrevNode = NULL;
    unsigned int addAtLastNode = 1;

    /* First Node */
    if (NULL == pAutoCfgScanResult)
    {
        pAutoCfgScanResult = (AutoCfgScanResult_t *)malloc(sizeof(AutoCfgScanResult_t));

        /* Limited to 4 because of MIB segmentaion */
        if (gScanResult.op[index]->numOfPlmn > 4)
        {
            pAutoCfgScanResult->numOfPlmn = 4;
        }
        else
        {
            pAutoCfgScanResult->numOfPlmn = gScanResult.op[index]->numOfPlmn;
        }

        pAutoCfgScanResult->mibPlmnIdentity = gScanResult.op[index]->mibPlmnIdentity;

        for (idx = 0; idx < pAutoCfgScanResult->numOfPlmn; idx++)
        {
            pAutoCfgScanResult->plmn[idx].mccLength = gScanResult.op[index]->plmn[idx].mccLength;

            for (idx2 = 0; idx2 < pAutoCfgScanResult->plmn[idx].mccLength; idx2++)
            {
                pAutoCfgScanResult->plmn[idx].mcc[idx2] = gScanResult.op[index]->plmn[idx].mcc[idx2];
            }

            pAutoCfgScanResult->plmn[idx].mncLength = gScanResult.op[index]->plmn[idx].mncLength;

            for (idx2 = 0; idx2 < pAutoCfgScanResult->plmn[idx].mncLength; idx2++)
            {
                pAutoCfgScanResult->plmn[idx].mnc[idx2] = gScanResult.op[index]->plmn[idx].mnc[idx2];
            }
        }

        pAutoCfgScanResult->t3212 = gScanResult.op[index]->scanCellRslt[0]->t3212;
        pAutoCfgScanResult->nmo = gScanResult.op[index]->scanCellRslt[0]->nmo;

        pAutoCfgScanResult->psc = gScanResult.op[index]->scanCellRslt[0]->psc;
        pAutoCfgScanResult->rscp = gScanResult.op[index]->scanCellRslt[0]->rscp;
        pAutoCfgScanResult->dlUarfcn = gScanResult.op[index]->scanCellRslt[0]->uarfcn;
        pAutoCfgScanResult->band = gScanResult.op[index]->scanCellRslt[0]->band;
        pAutoCfgScanResult->qQualMin = gScanResult.op[index]->scanCellRslt[0]->qQualMin;
        pAutoCfgScanResult->qRxLevMin = gScanResult.op[index]->scanCellRslt[0]->qRxLevMin;
        pAutoCfgScanResult->qHystLS = gScanResult.op[index]->scanCellRslt[0]->qHystLS;
        pAutoCfgScanResult->qHyst2S = gScanResult.op[index]->scanCellRslt[0]->qHyst2S;
        pAutoCfgScanResult->tReselS = gScanResult.op[index]->scanCellRslt[0]->tReselS;
        pAutoCfgScanResult->maxAllowedULTxPower = gScanResult.op[index]->scanCellRslt[0]->maxAllowedULTxPower;

        for (idx = 0; idx < 32; idx++)
        {
            pAutoCfgScanResult->intraFreqPsc[idx] = gScanResult.op[index]->scanCellRslt[0]->neighbourIntraPsc[idx];
            pAutoCfgScanResult->interFreqPsc[idx] = gScanResult.op[index]->scanCellRslt[0]->neighbourInterPsc[idx];
        }

        pAutoCfgScanResult->servingCellPriority = gScanResult.op[index]->scanCellRslt[0]->servingCellPriority;

        for (idx = 0; idx < 8; idx++)
        {
            pAutoCfgScanResult->fddPriorityCellInfo[idx].uarfcn = gScanResult.op[index]->scanCellRslt[0]->fddPriorityCellInfo[idx].uarfcn;
            pAutoCfgScanResult->fddPriorityCellInfo[idx].priority = gScanResult.op[index]->scanCellRslt[0]->fddPriorityCellInfo[idx].priority;
        }

        pAutoCfgScanResult->next = NULL;
    }

    if (pAutoCfgScanResult->dlUarfcn == gScanResult.op[index]->scanCellRslt[0]->uarfcn)
    {
        idx1 = 1;
    }
    else
    {
        idx1 = 0;
    }

    for (idx1; idx1 < gScanResult.op[index]->numberOfCell; idx1++)
    {
        pCurNode = pAutoCfgScanResult;

        /* Allocate & Fill new node */
        pNewNode = (AutoCfgScanResult_t *)malloc(sizeof(AutoCfgScanResult_t));

        /* Limited to 4 because of MIB segmentaion */
        if (gScanResult.op[index]->numOfPlmn > 4)
        {
            pNewNode->numOfPlmn = 4;
        }
        else
        {
            pNewNode->numOfPlmn = gScanResult.op[index]->numOfPlmn;
        }

        pNewNode->mibPlmnIdentity = gScanResult.op[index]->mibPlmnIdentity;

        for (idx = 0; idx < pNewNode->numOfPlmn; idx++)
        {
            pNewNode->plmn[idx].mccLength = gScanResult.op[index]->plmn[idx].mccLength;

            for (idx2 = 0; idx2 < pNewNode->plmn[idx].mccLength; idx2++)
            {
                pNewNode->plmn[idx].mcc[idx2] = gScanResult.op[index]->plmn[idx].mcc[idx2];
            }

            pNewNode->plmn[idx].mncLength = gScanResult.op[index]->plmn[idx].mncLength;

            for (idx2 = 0; idx2 < pNewNode->plmn[idx].mncLength; idx2++)
            {
                pNewNode->plmn[idx].mnc[idx2] = gScanResult.op[index]->plmn[idx].mnc[idx2];
            }
        }

        pNewNode->t3212 = gScanResult.op[index]->scanCellRslt[idx1]->t3212;
        pNewNode->nmo = gScanResult.op[index]->scanCellRslt[idx1]->nmo;

        pNewNode->psc = gScanResult.op[index]->scanCellRslt[idx1]->psc;
        pNewNode->rscp = gScanResult.op[index]->scanCellRslt[idx1]->rscp;
        pNewNode->dlUarfcn = gScanResult.op[index]->scanCellRslt[idx1]->uarfcn;
        pNewNode->band = gScanResult.op[index]->scanCellRslt[idx1]->band;
        pNewNode->qQualMin = gScanResult.op[index]->scanCellRslt[idx1]->qQualMin;
        pNewNode->qRxLevMin = gScanResult.op[index]->scanCellRslt[idx1]->qRxLevMin;
        pNewNode->qHystLS = gScanResult.op[index]->scanCellRslt[idx1]->qHystLS;
        pNewNode->qHyst2S = gScanResult.op[index]->scanCellRslt[idx1]->qHyst2S;
        pNewNode->tReselS = gScanResult.op[index]->scanCellRslt[idx1]->tReselS;
        pNewNode->maxAllowedULTxPower = gScanResult.op[index]->scanCellRslt[idx1]->maxAllowedULTxPower;

        for (idx = 0; idx < 32; idx++)
        {
            pNewNode->intraFreqPsc[idx] = gScanResult.op[index]->scanCellRslt[idx1]->neighbourIntraPsc[idx];
            pNewNode->interFreqPsc[idx] = gScanResult.op[index]->scanCellRslt[idx1]->neighbourInterPsc[idx];
        }

        pNewNode->servingCellPriority = gScanResult.op[index]->scanCellRslt[idx1]->servingCellPriority;

        for (idx = 0; idx < 8; idx++)
        {
            pNewNode->fddPriorityCellInfo[idx].uarfcn = gScanResult.op[index]->scanCellRslt[idx1]->fddPriorityCellInfo[idx].uarfcn;
            pNewNode->fddPriorityCellInfo[idx].priority = gScanResult.op[index]->scanCellRslt[idx1]->fddPriorityCellInfo[idx].priority;
        }

        /* Add at First Node */
        if ((pNewNode->servingCellPriority > pCurNode->servingCellPriority) || ((pNewNode->servingCellPriority == pCurNode->servingCellPriority) && (pNewNode->rscp >= pCurNode->rscp)))
        {
            pNewNode->next = pCurNode;
            pAutoCfgScanResult = pNewNode;
        }
        else
        {
            pPrevNode = pCurNode;
            while (NULL != pCurNode)
            {
                if ((pNewNode->servingCellPriority > pCurNode->servingCellPriority) || ((pNewNode->servingCellPriority == pCurNode->servingCellPriority) && (pNewNode->rscp >= pCurNode->rscp)))
                {
                    pNewNode->next = pCurNode;
                    pPrevNode->next = pNewNode;
                    addAtLastNode = 0;
                    break;
                }
                pPrevNode = pCurNode;
                pCurNode = pCurNode->next;
            }

            if ((1 == addAtLastNode) && (NULL == pCurNode) && (NULL != pPrevNode))
            {
                pNewNode->next = NULL;
                pPrevNode->next = pNewNode;
            }
        }
    }
}

int checkPlmnFound(void)
{
    int retCode = 0;
    unsigned int idx1, idx2 = 0;

    for (idx1 = 0; idx1 < 6; idx1++)
    {
        for (idx2 = 0; idx2 < FXL_UMTS_MAX_MCC_LEN; idx2++)
        {
            if (g3gListenModeSysInfo.plmn[idx1].mcc[idx2] == FXL_UMTS_VALUE_NOT_APPLICABLE)
            {
                g3gListenModeSysInfo.plmn[idx1].mcc[idx2] = 0xFFFFFFFF;
            }

            if (g3gListenModeSysInfo.plmn[idx1].mnc[idx2] == FXL_UMTS_VALUE_NOT_APPLICABLE)
            {
                g3gListenModeSysInfo.plmn[idx1].mnc[idx2] = 0xFFFFFFFF;
            }
        }
    }
    for (idx1 = 0; idx1 < g3gListenModeSysInfo.numOfPlmn; idx1++)
    {
        if ((g3gListenModeSysInfo.plmn[idx1].mccLength == gUmtsConfig.plmn[0].mccLength) &&
            (g3gListenModeSysInfo.plmn[idx1].mncLength == gUmtsConfig.plmn[0].mncLength) &&
            (g3gListenModeSysInfo.plmn[idx1].mcc[0] == gUmtsConfig.plmn[0].mcc[0]) &&
            (g3gListenModeSysInfo.plmn[idx1].mcc[1] == gUmtsConfig.plmn[0].mcc[1]) &&
            (g3gListenModeSysInfo.plmn[idx1].mcc[2] == gUmtsConfig.plmn[0].mcc[2]) &&
            (g3gListenModeSysInfo.plmn[idx1].mnc[0] == gUmtsConfig.plmn[0].mnc[0]) &&
            (g3gListenModeSysInfo.plmn[idx1].mnc[1] == gUmtsConfig.plmn[0].mnc[1]) &&
            (g3gListenModeSysInfo.plmn[idx1].mnc[2] == gUmtsConfig.plmn[0].mnc[2]))
        {
            retCode = 1;
        }
    }

    return (retCode);
}

void addAutoCfgScanResult(void)
{
    unsigned int idx = 0;
    unsigned int idx1 = 0;
    AutoCfgScanResult_t *pNewNode = NULL;
    AutoCfgScanResult_t *pCurNode = NULL;
    AutoCfgScanResult_t *pPrevNode = NULL;
    unsigned int addAtLastNode = 1;

    /* First Node */
    if (NULL == pAutoCfgScanResult)
    {
        pAutoCfgScanResult = (AutoCfgScanResult_t *)malloc(sizeof(AutoCfgScanResult_t));

        /* Limited to 4 because of MIB segmentaion */
        if (g3gListenModeSysInfo.numOfPlmn > 4)
        {
            pAutoCfgScanResult->numOfPlmn = 4;
        }
        else
        {
            pAutoCfgScanResult->numOfPlmn = g3gListenModeSysInfo.numOfPlmn;
        }

        pAutoCfgScanResult->mibPlmnIdentity = g3gListenModeSysInfo.mibPlmnIdentity;

        for (idx = 0; idx < pAutoCfgScanResult->numOfPlmn; idx++)
        {
            pAutoCfgScanResult->plmn[idx].mccLength = g3gListenModeSysInfo.plmn[idx].mccLength;

            for (idx1 = 0; idx1 < g3gListenModeSysInfo.plmn[idx].mccLength; idx1++)
            {
                pAutoCfgScanResult->plmn[idx].mcc[idx1] = g3gListenModeSysInfo.plmn[idx].mcc[idx1];
            }

            pAutoCfgScanResult->plmn[idx].mncLength = g3gListenModeSysInfo.plmn[idx].mncLength;

            for (idx1 = 0; idx1 < g3gListenModeSysInfo.plmn[idx].mncLength; idx1++)
            {
                pAutoCfgScanResult->plmn[idx].mnc[idx1] = g3gListenModeSysInfo.plmn[idx].mnc[idx1];
            }
        }

        pAutoCfgScanResult->t3212 = g3gListenModeSysInfo.t3212;
        pAutoCfgScanResult->nmo = g3gListenModeSysInfo.nmo;

        pAutoCfgScanResult->psc = g3gListenModeSysInfo.cellInfo[0].psc;
        pAutoCfgScanResult->rscp = g3gListenModeSysInfo.cellInfo[0].rscp;
        pAutoCfgScanResult->qQualMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qQualMin;

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin)
        {
            pAutoCfgScanResult->qRxLevMin = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin - 1) / 2;
        }
        else
        {
            pAutoCfgScanResult->qRxLevMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin;
        }

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S)
        {
            pAutoCfgScanResult->qHyst2S = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S / 2);
        }
        else
        {
            pAutoCfgScanResult->qHyst2S = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S;
        }

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS)
        {
            pAutoCfgScanResult->qHystLS = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS / 2);
        }
        else
        {
            pAutoCfgScanResult->qHystLS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS;
        }

        pAutoCfgScanResult->tReselS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.tReselS;
        pAutoCfgScanResult->maxAllowedULTxPower = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.maxAllowedULTxPower;
        pAutoCfgScanResult->dlUarfcn = g3gListenModeSysInfo.uarfcn;
        pAutoCfgScanResult->band = g3gListenModeSysInfo.band;
        for (idx = 0; idx < 32; idx++)
        {
            pAutoCfgScanResult->intraFreqPsc[idx] = g3gListenModeSysInfo.cellInfo[0].intraFreqCellList[idx].psc;
            pAutoCfgScanResult->interFreqPsc[idx] = g3gListenModeSysInfo.cellInfo[0].interFreqCellList[idx].psc;
        }

        pAutoCfgScanResult->servingCellPriority = g3gListenModeSysInfo.priorityCellInfo.servingCellPriority;

        for (idx = 0; idx < 8; idx++)
        {
            pAutoCfgScanResult->fddPriorityCellInfo[idx].uarfcn = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].uarfcn;
            pAutoCfgScanResult->fddPriorityCellInfo[idx].priority = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].priority;
        }

        pAutoCfgScanResult->next = NULL;
    }
    else
    {
        pCurNode = pAutoCfgScanResult;

        /* Allocate & Fill new node */
        pNewNode = (AutoCfgScanResult_t *)malloc(sizeof(AutoCfgScanResult_t));

        /* Limited to 4 because of MIB segmentaion */
        if (g3gListenModeSysInfo.numOfPlmn > 4)
        {
            pNewNode->numOfPlmn = 4;
        }
        else
        {
            pNewNode->numOfPlmn = g3gListenModeSysInfo.numOfPlmn;
        }

        pNewNode->mibPlmnIdentity = g3gListenModeSysInfo.mibPlmnIdentity;

        for (idx = 0; idx < pNewNode->numOfPlmn; idx++)
        {
            pNewNode->plmn[idx].mccLength = g3gListenModeSysInfo.plmn[idx].mccLength;

            for (idx1 = 0; idx1 < g3gListenModeSysInfo.plmn[idx].mccLength; idx1++)
            {
                pNewNode->plmn[idx].mcc[idx1] = g3gListenModeSysInfo.plmn[idx].mcc[idx1];
            }

            pNewNode->plmn[idx].mncLength = g3gListenModeSysInfo.plmn[idx].mncLength;

            for (idx1 = 0; idx1 < g3gListenModeSysInfo.plmn[idx].mncLength; idx1++)
            {
                pNewNode->plmn[idx].mnc[idx1] = g3gListenModeSysInfo.plmn[idx].mnc[idx1];
            }
        }

        pNewNode->t3212 = g3gListenModeSysInfo.t3212;
        pNewNode->nmo = g3gListenModeSysInfo.nmo;

        pNewNode->psc = g3gListenModeSysInfo.cellInfo[0].psc;
        pNewNode->rscp = g3gListenModeSysInfo.cellInfo[0].rscp;
        pNewNode->qQualMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qQualMin;

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin)
        {
            pNewNode->qRxLevMin = ((g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin - 1) / 2);
        }
        else
        {
            pNewNode->qRxLevMin = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qRxLevMin;
        }

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S)
        {
            pNewNode->qHyst2S = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S / 2);
        }
        else
        {
            pNewNode->qHyst2S = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHyst2S;
        }

        if (-1 != g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS)
        {
            pNewNode->qHystLS = (g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS / 2);
        }
        else
        {
            pNewNode->qHystLS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.qHystLS;
        }

        pNewNode->tReselS = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.tReselS;
        pNewNode->maxAllowedULTxPower = g3gListenModeSysInfo.cellInfo[0].cellSelReselInfo.maxAllowedULTxPower;
        pNewNode->dlUarfcn = g3gListenModeSysInfo.uarfcn;
        pNewNode->band = g3gListenModeSysInfo.band;
        for (idx = 0; idx < 32; idx++)
        {
            pNewNode->intraFreqPsc[idx] = g3gListenModeSysInfo.cellInfo[0].intraFreqCellList[idx].psc;
            pNewNode->interFreqPsc[idx] = g3gListenModeSysInfo.cellInfo[0].interFreqCellList[idx].psc;
        }

        pNewNode->servingCellPriority = g3gListenModeSysInfo.priorityCellInfo.servingCellPriority;

        for (idx = 0; idx < 8; idx++)
        {
            pNewNode->fddPriorityCellInfo[idx].uarfcn = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].uarfcn;
            pNewNode->fddPriorityCellInfo[idx].priority = g3gListenModeSysInfo.priorityCellInfo.fddPriorityCellInfo[idx].priority;
        }

        /* Add at First Node */
        if ((pNewNode->servingCellPriority > pCurNode->servingCellPriority) || ((pNewNode->servingCellPriority == pCurNode->servingCellPriority) && (pNewNode->rscp >= pCurNode->rscp)))
        {
            pNewNode->next = pCurNode;
            pAutoCfgScanResult = pNewNode;
        }
        else
        {
            pPrevNode = pCurNode;
            while (NULL != pCurNode)
            {
                if ((pNewNode->servingCellPriority > pCurNode->servingCellPriority) || ((pNewNode->servingCellPriority == pCurNode->servingCellPriority) && (pNewNode->rscp >= pCurNode->rscp)))
                {
                    pNewNode->next = pCurNode;
                    pPrevNode->next = pNewNode;
                    addAtLastNode = 0;
                    break;
                }
                pPrevNode = pCurNode;
                pCurNode = pCurNode->next;
            }

            if ((1 == addAtLastNode) && (NULL == pCurNode) && (NULL != pPrevNode))
            {
                pNewNode->next = NULL;
                pPrevNode->next = pNewNode;
            }
        }
    }
}

void delAllAutoCfgScanResult(void)
{
    AutoCfgScanResult_t *pCurNode = pAutoCfgScanResult;

    while (NULL != pCurNode)
    {
        pAutoCfgScanResult = pAutoCfgScanResult->next;
        free(pCurNode);
        pCurNode = pAutoCfgScanResult;
    }

    pAutoCfgScanResult = NULL;
    pCurNode = NULL;
}

void configureGsmBassParameters(fxL2gBassModeConfigureBandsCmd *cmd)
{
    unsigned int i = 0;

    if (cmd->noOfBandsToScan == 0)
    {
        logPrint(LOG_DEBUG, "Num of Bands to Scan is 0, Please configure atleast 1 band to scan\n");
        return;
    }

#ifdef CELL_ANTENNA_CHANGE
    prmSetup.ulEntryCount = cmd->noOfBandsToScan;
    bandIndex = 0;
#else
    tWnm.PrmSetup.ulEntryCount = cmd->noOfBandsToScan;
#endif

    bContinuousMonitoringEnable = 1;

#ifdef CELL_ANTENNA_CHANGE
    for (i = 0; i < prmSetup.ulEntryCount; i++)
    {
        prmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
        prmSetup.aEntryList[i].ulBand = convertToOct2gBandFromFxL(cmd->band[i]);
        prmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN; // for full band scanning
        prmSetup.aEntryList[i].ulUarfcnStop = 123;
        prmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        prmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        prmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        prmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#else
    for (i = 0; i < tWnm.PrmSetup.ulEntryCount; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
        tWnm.PrmSetup.aEntryList[i].ulBand = convertToOct2gBandFromFxL(cmd->band[i]);
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN; // for full band scanning
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = 123;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#endif

    return;
}

void configureWnmParameters(fxL2gListenModeConfigureBandsCmd *cmd)
{
    int i = 0;

    if (cmd->noOfBandsToScan == 0)
    {
        logPrint(LOG_DEBUG, "Num of Bands to Scan is 0, Please configure atleast 1 band to scan\n");
        return;
    }

#ifdef CELL_ANTENNA_CHANGE
    prmSetup.ulEntryCount = cmd->noOfBandsToScan;
    bandIndex = 0;
#else
    tWnm.PrmSetup.ulEntryCount = cmd->noOfBandsToScan;
#endif

    bContinuousMonitoringEnable = cmd->noOfInterationsToScan;


#ifdef CELL_ANTENNA_CHANGE
    for (i = 0; i < prmSetup.ulEntryCount; i++)
    {
        //logPrint(LOG_DEBUG, "GSM scan for loop = %d  \n", i);
        prmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
        prmSetup.aEntryList[i].ulBand = convertToOct2gBandFromFxL(cmd->band[i]);
        prmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN; // for full band scanning
        prmSetup.aEntryList[i].ulUarfcnStop = 123;
        prmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        prmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        prmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        prmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#else
    for (i = 0; i < tWnm.PrmSetup.ulEntryCount; i++)
    {
        //logPrint(LOG_DEBUG, "GSM scan for loop else part = %d  \n", i);
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
        tWnm.PrmSetup.aEntryList[i].ulBand = convertToOct2gBandFromFxL(cmd->band[i]);
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN; // for full band scanning
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = 123;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#endif
    return;
}

unsigned int getGsmBandFromARFCN(unsigned int gsmArfcn)
{
    unsigned int gsmBand = 0xFFFFFF;
    if ((gsmArfcn >= 1) && (gsmArfcn <= 124))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_P_900;
    }
    else if (((gsmArfcn >= 0) && (gsmArfcn <= 124)) || ((gsmArfcn >= 975) && (gsmArfcn <= 1023)))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_E_900;
    }
    else if (((gsmArfcn >= 0) && (gsmArfcn <= 124)) || ((gsmArfcn >= 955) && (gsmArfcn <= 1023)))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_R_900;
    }
    else if ((gsmArfcn >= 128) && (gsmArfcn <= 251))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_850;
    }
    else if ((gsmArfcn >= 259) && (gsmArfcn <= 293))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_450;
    }
    else if ((gsmArfcn >= 306) && (gsmArfcn <= 340))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_480;
    }
    else if ((gsmArfcn >= 512) && (gsmArfcn <= 810) && (trx_g->bts.band == PCS_1900))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_PCS_1900;
    }
    else if ((gsmArfcn >= 512) && (gsmArfcn <= 885))
    {
        gsmBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_DCS_1800;
    }
    else
    {
        logPrint(LOG_DEBUG, "INVALID ARFCN PASSED ARFCN = %d\n", gsmArfcn);
    }

    return gsmBand;
}

fxLResult configureGsmFreqForSelectiveScan(fxL2gListenModeConfigureSelectiveFreqCmd *cmd)
{
    int i = 0;
    fxLResult result = FXL_SUCCESS;
    tOCT_UINT32 band = 0xFFFFFFFF;

    tWnm.PrmSetup.ulEntryCount = cmd->noOfArfcnConfigured;

#ifdef CELL_ANTENNA_CHANGE
    bandIndex = -1;
#endif

    bContinuousMonitoringEnable = cmd->noOfScanIterations;

    //logPrint(LOG_DEBUG, "GSM selective scan cmd->noOfScanIterations = %d\n", cmd->noOfScanIterations);
    //logPrint(LOG_DEBUG, "GSM selective scan bContinuousMonitoringEnable = %d \n", bContinuousMonitoringEnable);

    if (tWnm.PrmSetup.ulEntryCount == 0)
    {
        logPrint(LOG_DEBUG, "Num of GSM-ARFCNS to Scan is 0, Please configure atleast 1 ARFCN to scan\n");
        return;
    }

    for (i = 0; i < tWnm.PrmSetup.ulEntryCount; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_GSM;
        band = convertToOct2gBandFromFxL(cmd->bandArfcn[i].band);
        if (0xFFFFFFFF != band)
        {
            tWnm.PrmSetup.aEntryList[i].ulBand = band;
        }
        else
        {
            tWnm.PrmSetup.aEntryList[i].ulBand = 0;
            result = FXL_FAILURE;
        }
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cmd->bandArfcn[i].arfcn;
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = cmd->bandArfcn[i].arfcn;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }

    return result;
}

fxL2gBandType convertToFxlBandFromOct(tOCT_UINT32 band)
{
    fxL2gBandType ret = 0xFF;
    switch (band)
    {
    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_850:
        ret = FXL_BAND_GSM_850;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_P_900:
        ret = FXL_BAND_GSM_900;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_DCS_1800:
        ret = FXL_BAND_DCS_1800;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_PCS_1900:
        ret = FXL_BAND_PCS_1900;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_R_900:
        ret = FXL_BAND_R_GSM;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_E_900:
        ret = FXL_BAND_E_GSM;
        break;

    case cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_450:
        ret = FXL_BAND_GSM_450;
        break;

    default:
        logPrint(LOG_DEBUG, "convertToFxlBandFromOct: Invalid Band(%d)\n", band);
        break;
    }

    return ret;
}
tOCT_UINT32 convertToOct2gBandFromFxL(fxL2gBandType band)
{
    tOCT_UINT32 ret = 0xFFFFFF;

    switch (band)
    {
    case FXL_BAND_GSM_850:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_850;
        break;

    case FXL_BAND_GSM_900:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_P_900;
        break;

    case FXL_BAND_DCS_1800:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_DCS_1800;
        break;

    case FXL_BAND_PCS_1900:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_PCS_1900;
        break;

    case FXL_BAND_R_GSM:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_R_900;
        break;

    case FXL_BAND_E_GSM:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_E_900;
        break;

    case FXL_BAND_GSM_450:
        ret = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_450;
        break;

    default:
        logPrint(LOG_DEBUG, "convertToOct2gBandFromFxL: Invalid Band\n");
        break;
    }

    return ret;
}

int getEarfcnFromFreqBandIndicator(int freqBandIndicator)
{
    switch (freqBandIndicator)
    {
    case 1:
        return 300;
    case 3:
        return 1650;
    case 5:
        return 2525;
    case 7:
        return 3100;
    case 8:
        return 3625;
    case 20:
        return 6300;
    case 28:
        return 9435;
    default:
    {
        logPrint(LOG_DEBUG, "Unsupported Band exiting now\n");
        exit(0);
    }
    }
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       GetModuleStateById

Description:    Get the state of the module specified by its ID.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int GetModuleStateById(tOCT_UINT32 ulModuleId, tOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM *peModuleState)
{
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD ModuleStatsCmd;
    tOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP ModuleStatsRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CID ----\n");

    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_DEF(&ModuleStatsCmd);
    ModuleStatsCmd.ulModuleId = ulModuleId;
    ModuleStatsCmd.ulResetStatsFlag = cOCT_FALSE;

    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CMD_SWAP(&ModuleStatsCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &ModuleStatsCmd;
    CmdExecuteParms.pRsp = &ModuleStatsRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(ModuleStatsRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    mOCTVC1_MAIN_MSG_APPLICATION_STATS_MODULE_RSP_SWAP(&ModuleStatsRsp);

    logPrint(LOG_DEBUG, "\tDone\n");

    if (peModuleState != NULL)
    {
        *peModuleState = ModuleStatsRsp.Stats.ulState;
    }

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       ModuleStartById

Description:    Start the module specified by its ID.
                Nothing will be done if the module is already started.

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int ModuleStartById(tOCT_UINT32 ulModuleId)
{
    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD ModuleStartCmd;
    tOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP ModuleStartRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;
    tOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM eModuleState;

    if (0 != GetModuleStateById(ulModuleId, &eModuleState))
    {
        goto ErrorHandling;
    }

    if (eModuleState == cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STARTED)
    {
        // Module already started
    }
    else if (eModuleState == cOCTVC1_MAIN_APPLICATION_MODULE_STATE_ENUM_STOPPED)
    {
        // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CID ----\n");

        mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_DEF(&ModuleStartCmd);
        ModuleStartCmd.ulModuleId = ulModuleId;

        mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CMD_SWAP(&ModuleStartCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &ModuleStartCmd;
        CmdExecuteParms.pRsp = &ModuleStartRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(ModuleStartRsp);
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_CID failed, rc = 0x%08x\n", ulResult);
            fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
            goto ErrorHandling;
        }

        mOCTVC1_MAIN_MSG_APPLICATION_START_MODULE_RSP_SWAP(&ModuleStartRsp);

        // logPrint(LOG_DEBUG,"\tDone\n");
    }
    else
    {
        fprintf(stderr, "Error: Module 0x%08x is in a bad state (%d)\n", ulModuleId, eModuleState);
        goto ErrorHandling;
    }

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       LteEnodebGetHandle

Description:    Modify LTE Cell Rf properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int LteEnodebGetHandle(void)
{
    tOCTVC1_LTE_MSG_ENODEB_LIST_CMD LteCellListCmd;
    tOCTVC1_LTE_MSG_ENODEB_LIST_RSP LteCellListRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    mOCTVC1_LTE_MSG_ENODEB_LIST_CMD_DEF(&LteCellListCmd);

    mOCTVC1_LTE_MSG_ENODEB_LIST_CMD_SWAP(&LteCellListCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &LteCellListCmd;
    CmdExecuteParms.pRsp = &LteCellListRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(LteCellListRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_GENERIC_RC_NO_MORE_OBJECT == ulResult)
    {
        ulResult = cOCTVC1_RC_OK;
    }
    else if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_LIST_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the file open command response.
     */
    mOCTVC1_LTE_MSG_ENODEB_LIST_RSP_SWAP(&LteCellListRsp);

    // logPrint(LOG_DEBUG,"EnodeB List:(%d)\n", LteCellListRsp.ObjectList.ulNumHandleObject );
    if (LteCellListRsp.ObjectList.ulNumHandleObject)
    {
        g_AppCtxRf.hEnodeb = LteCellListRsp.ObjectList.aHandleObject[0];
    }
    else
    {
        g_AppCtxRf.hEnodeb = 0;
    }

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       HwRfPortModifyAntennaTxConfig

Description:    Modify HW RF Port Antenna TX properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int HwRfPortModifyAntennaTxConfig(tOCT_UINT32 f_ulRfPortIdx, tOCT_UINT32 f_ulAntennaIdx)
{
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_CMD RfPortModifyAntennaTxConfigCmd;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_RSP RfPortModifyAntennaTxConfigRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_CMD_CID ----\n");

    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_CMD_DEF(&RfPortModifyAntennaTxConfigCmd);
    RfPortModifyAntennaTxConfigCmd.lTxGaindB = QUARTER_DB_TO_Q7(g_AppCtxRf.ulRfTxAttndB);
    RfPortModifyAntennaTxConfigCmd.ulAntennaIndex = f_ulAntennaIdx;
    RfPortModifyAntennaTxConfigCmd.ulPortIndex = f_ulRfPortIdx;
    RfPortModifyAntennaTxConfigCmd.ulRelativeGainFlag = cOCT_FALSE;

    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_CMD_SWAP(&RfPortModifyAntennaTxConfigCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &RfPortModifyAntennaTxConfigCmd;
    CmdExecuteParms.pRsp = &RfPortModifyAntennaTxConfigRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(RfPortModifyAntennaTxConfigRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CONFIG_RSP_SWAP(&RfPortModifyAntennaTxConfigRsp);

    printf("\tHW RF Port{%d} Antenna{%d}: [TX Gain:%d dB]\n",
           RfPortModifyAntennaTxConfigRsp.ulPortIndex,
           RfPortModifyAntennaTxConfigRsp.ulAntennaIndex,
           Q7_TO_QUARTER_DB(RfPortModifyAntennaTxConfigRsp.lTxGaindB));

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       HwRfPortModifyAntennaRxConfig

Description:    Modify HW RF Port Antenna RX properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int HwRfPortModifyAntennaRxConfig(tOCT_UINT32 f_ulRfPortIdx, tOCT_UINT32 f_ulAntennaIdx)
{
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_CMD RfPortModifyAntennaRxConfigCmd;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_RSP RfPortModifyAntennaRxConfigRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_CMD_CID ----\n");

    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_CMD_DEF(&RfPortModifyAntennaRxConfigCmd);
    RfPortModifyAntennaRxConfigCmd.lRxGaindB = QUARTER_DB_TO_Q9(g_AppCtxRf.ulRfRxGaindB);
    RfPortModifyAntennaRxConfigCmd.ulAntennaIndex = f_ulAntennaIdx;
    RfPortModifyAntennaRxConfigCmd.ulPortIndex = f_ulRfPortIdx;
    RfPortModifyAntennaRxConfigCmd.ulRelativeGainFlag = cOCT_FALSE;

    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_CMD_SWAP(&RfPortModifyAntennaRxConfigCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &RfPortModifyAntennaRxConfigCmd;
    CmdExecuteParms.pRsp = &RfPortModifyAntennaRxConfigRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(RfPortModifyAntennaRxConfigRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_RX_CONFIG_RSP_SWAP(&RfPortModifyAntennaRxConfigRsp);

    printf("\tHW RF Port{%d} Antenna{%d}: [RX Gain:%d dB]\n",
           RfPortModifyAntennaRxConfigRsp.ulPortIndex,
           RfPortModifyAntennaRxConfigRsp.ulAntennaIndex,
           Q9_TO_QUARTER_DB(RfPortModifyAntennaRxConfigRsp.lRxGaindB));

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       HwRfPortInfoAntennaTxConfig

Description:    Query HW RF Port Antenna TX Config

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int HwRfPortInfoAntennaTxConfig(tOCT_UINT32 f_ulRfPortIdx,
                                       tOCT_UINT32 f_ulAntennaIdx,
                                       tOCT_BOOL32 f_ulValidateBandFlag)
{
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_CMD RfPortInfoAntennaTxConfigCmd;
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_RSP RfPortInfoAntennaTxConfigRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_CMD_CID ----\n");

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_CMD_DEF(&RfPortInfoAntennaTxConfigCmd);
    RfPortInfoAntennaTxConfigCmd.ulAntennaIndex = f_ulAntennaIdx;
    RfPortInfoAntennaTxConfigCmd.ulPortIndex = f_ulRfPortIdx;

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_CMD_SWAP(&RfPortInfoAntennaTxConfigCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &RfPortInfoAntennaTxConfigCmd;
    CmdExecuteParms.pRsp = &RfPortInfoAntennaTxConfigRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(RfPortInfoAntennaTxConfigRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CONFIG_RSP_SWAP(&RfPortInfoAntennaTxConfigRsp);

    if (cOCT_TRUE == f_ulValidateBandFlag)
    {
        if (Q7_TO_QUARTER_DB(RfPortInfoAntennaTxConfigRsp.TxConfig.lTxGaindB) != g_AppCtxRf.ulRfTxAttndB)
        {
            fprintf(stderr, "Error: lTxGaindB does not match, \
							lTxGaindB [expecting:%d dB, get:%d dB]\n",
                    g_AppCtxRf.ulRfTxAttndB,
                    Q7_TO_QUARTER_DB(RfPortInfoAntennaTxConfigRsp.TxConfig.lTxGaindB));

            goto ErrorHandling;
        }
    }
    logPrint(LOG_DEBUG, "InfoHW RF Port{%d} Antenna{%d}: [TX Gain:%d dB]\n",
             RfPortInfoAntennaTxConfigRsp.ulPortIndex,
             RfPortInfoAntennaTxConfigRsp.ulAntennaIndex,
             Q7_TO_QUARTER_DB(RfPortInfoAntennaTxConfigRsp.TxConfig.lTxGaindB));

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       HwRfPortInfoAntennaRxConfig

Description:    Query HW RF Port Antenna RX Config

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static int HwRfPortInfoAntennaRxConfig(tOCT_UINT32 f_ulRfPortIdx,
                                       tOCT_UINT32 f_ulAntennaIdx,
                                       tOCT_BOOL32 f_ulValidateBandFlag)
{
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_CMD RfPortInfoAntennaRxConfigCmd;
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_RSP RfPortInfoAntennaRxConfigRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_CMD_CID ----\n");

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_CMD_DEF(&RfPortInfoAntennaRxConfigCmd);
    RfPortInfoAntennaRxConfigCmd.ulAntennaIndex = f_ulAntennaIdx;
    RfPortInfoAntennaRxConfigCmd.ulPortIndex = f_ulRfPortIdx;

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_CMD_SWAP(&RfPortInfoAntennaRxConfigCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &RfPortInfoAntennaRxConfigCmd;
    CmdExecuteParms.pRsp = &RfPortInfoAntennaRxConfigRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(RfPortInfoAntennaRxConfigRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_RX_CONFIG_RSP_SWAP(&RfPortInfoAntennaRxConfigRsp);

    if (cOCT_TRUE == f_ulValidateBandFlag)
    {
        if (Q9_TO_QUARTER_DB(RfPortInfoAntennaRxConfigRsp.RxConfig.lRxGaindB) != g_AppCtxRf.ulRfRxGaindB)
        {
            fprintf(stderr, "Error: lRxGaindB does not match, \
							lRxGaindB [expecting:%d dB, get:%d dB]\n",
                    g_AppCtxRf.ulRfRxGaindB,
                    Q9_TO_QUARTER_DB(RfPortInfoAntennaRxConfigRsp.RxConfig.lRxGaindB));

            goto ErrorHandling;
        }
    }
    logPrint(LOG_DEBUG, "Info-HW RF Port{%d} Antenna{%d}: [RX Gain:%d dB]\n",
             RfPortInfoAntennaRxConfigRsp.ulPortIndex,
             RfPortInfoAntennaRxConfigRsp.ulAntennaIndex,
             Q9_TO_QUARTER_DB(RfPortInfoAntennaRxConfigRsp.RxConfig.lRxGaindB));

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebInfoCell

Description:    Query LTE Enodeb CELL properties info

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebInfoCell(tOCT_BOOL32 f_ulValidateBandFlag)
{
    tOCTVC1_LTE_MSG_ENODEB_INFO_CELL_CMD EnodebInfoCellCmd;
    tOCTVC1_LTE_MSG_ENODEB_INFO_CELL_RSP EnodebInfoCellRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_INFO_CELL_CMD_CID ----\n");

    mOCTVC1_LTE_MSG_ENODEB_INFO_CELL_CMD_DEF(&EnodebInfoCellCmd);
    EnodebInfoCellCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_INFO_CELL_CMD_SWAP(&EnodebInfoCellCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebInfoCellCmd;
    CmdExecuteParms.pRsp = &EnodebInfoCellRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebInfoCellRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_INFO_CELL_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_LTE_MSG_ENODEB_INFO_CELL_RSP_SWAP(&EnodebInfoCellRsp);

    if (cOCT_TRUE == f_ulValidateBandFlag)
    {
        if (EnodebInfoCellRsp.hEnodeb != g_AppCtxRf.hEnodeb ||
            EnodebInfoCellRsp.CellConfig.ulDuplexMode != g_AppCtxRf.ulDuplexMode ||
            EnodebInfoCellRsp.CellConfig.ulCpType != g_AppCtxRf.ulCpType ||
            EnodebInfoCellRsp.CellConfig.ulBwId != g_AppCtxRf.ulBwId)
        {
            fprintf(stderr, "Error: ulEarfcn or ulBwId does not match Enodeb{0x%08x}, \
							ulDuplexMode [expecting:%d, get:%d], \
							ulCpType [expecting:%d, get:%d], \
							ulBwId [expecting:%d, get:%d] \n",
                    EnodebInfoCellRsp.hEnodeb,
                    g_AppCtxRf.ulDuplexMode, EnodebInfoCellRsp.CellConfig.ulDuplexMode,
                    g_AppCtxRf.ulCpType, EnodebInfoCellRsp.CellConfig.ulCpType,
                    g_AppCtxRf.ulBwId, EnodebInfoCellRsp.CellConfig.ulBwId);
            goto ErrorHandling;
        }
        else if ((g_AppCtxRf.ulDuplexMode == cOCTVC1_LTE_DUPLEX_MODE_ENUM_TDD) &&
                 (EnodebInfoCellRsp.CellTddConfig.ulUplinkDownlinkConfigIdx != g_AppCtxRf.ulTddConfig ||
                  EnodebInfoCellRsp.CellTddConfig.ulSpecialSubframeConfigIdx != g_AppCtxRf.ulTddSpecialSfConfig))
        {
            fprintf(stderr, "Error: TDD config does not match Enodeb{0x%08x}, \
							ulTddConfig [expecting:%d, get:%d], \
							ulTddSpecialSfConfig [expecting:%d, get:%d]\n",
                    EnodebInfoCellRsp.hEnodeb,
                    g_AppCtxRf.ulTddConfig, EnodebInfoCellRsp.CellTddConfig.ulUplinkDownlinkConfigIdx,
                    g_AppCtxRf.ulTddSpecialSfConfig, EnodebInfoCellRsp.CellTddConfig.ulSpecialSubframeConfigIdx);
            goto ErrorHandling;
        }
    }

    if (g_AppCtxRf.ulDuplexMode == cOCTVC1_LTE_DUPLEX_MODE_ENUM_TDD)
    {
        logPrint(LOG_DEBUG, "Info CELL:[BwId:%d , ULDL : %d , SSA : %d]\n",
                 EnodebInfoCellRsp.CellConfig.ulBwId,
                 EnodebInfoCellRsp.CellTddConfig.ulUplinkDownlinkConfigIdx,
                 EnodebInfoCellRsp.CellTddConfig.ulSpecialSubframeConfigIdx);
    }
    else
    {
        logPrint(LOG_DEBUG, "Info CELL:[BwId:%d]\n",
                 EnodebInfoCellRsp.CellConfig.ulBwId);
    }
    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebInfoRf

Description:    Query LTE Enodeb CELL properties info

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebInfoRf(tOCT_BOOL32 f_ulValidateBandFlag)
{
    tOCTVC1_LTE_MSG_ENODEB_INFO_RF_CMD EnodebInfoRfCmd;
    tOCTVC1_LTE_MSG_ENODEB_INFO_RF_RSP EnodebInfoRfRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_INFO_RF_CMD_CID ----\n");

    mOCTVC1_LTE_MSG_ENODEB_INFO_RF_CMD_DEF(&EnodebInfoRfCmd);
    EnodebInfoRfCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_INFO_RF_CMD_SWAP(&EnodebInfoRfCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebInfoRfCmd;
    CmdExecuteParms.pRsp = &EnodebInfoRfRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebInfoRfRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_INFO_RF_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_LTE_MSG_ENODEB_INFO_RF_RSP_SWAP(&EnodebInfoRfRsp);
#if 0
	if( cOCT_TRUE == f_ulValidateBandFlag )
	{
		if( EnodebInfoRfRsp.hEnodeb != g_AppCtxRf.hEnodeb ||
			EnodebInfoRfRsp.RfConfig.ulRxGainDb != g_AppCtxRf.ulRfRxGaindB ||
			EnodebInfoRfRsp.RfConfig.ulTxAttnDb != g_AppCtxRf.ulRfTxAttndB )
		{
            fprintf(stderr, "Error: ulRfRxGaindB or ulRfTxAttndB does not match Enodeb{0x%08x}, \
							ulRfRxGaindB [expecting:%d, get:%d], ulTxAttndB [expecting:%d, get:%d]\n",
			EnodebInfoRfRsp.hEnodeb,
			g_AppCtxRf.ulRfRxGaindB, EnodebInfoRfRsp.RfConfig.ulRxGainDb,
			g_AppCtxRf.ulRfTxAttndB, EnodebInfoRfRsp.RfConfig.ulTxAttnDb);
			goto ErrorHandling;
		}
	}
    logPrint(LOG_DEBUG,"Info RF:[ulRfRxGaindB:%d, ulTxAttndB:%d]\n",
            EnodebInfoRfRsp.RfConfig.ulRxGainDb,
			EnodebInfoRfRsp.RfConfig.ulTxAttnDb);

#endif
    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebInfoProperties

Description:    Query LTE Enodeb RF and CELL properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebInfoProperties(tOCT_BOOL32 f_ulValidateBandFlag)
{
    if (EnodebInfoCell(f_ulValidateBandFlag))
        return -1;
    if (EnodebInfoRf(f_ulValidateBandFlag))
        return -1;

    return 0;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebCellStatus

Description:    Get LTE Enodeb Cell status

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebCellStatus(void)
{
    tOCTVC1_LTE_MSG_ENODEB_STATS_CELL_CMD EnodebStatsCellCmd;
    tOCTVC1_LTE_MSG_ENODEB_STATS_CELL_RSP EnodebStatsCellRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_STATS_CELL_CMD_CID ----\n");

    mOCTVC1_LTE_MSG_ENODEB_STATS_CELL_CMD_DEF(&EnodebStatsCellCmd);
    EnodebStatsCellCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_STATS_CELL_CMD_SWAP(&EnodebStatsCellCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebStatsCellCmd;
    CmdExecuteParms.pRsp = &EnodebStatsCellRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebStatsCellRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_STATS_CELL_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_LTE_MSG_ENODEB_STATS_CELL_RSP_SWAP(&EnodebStatsCellRsp);
    g_AppCtxRf.ulCellState = EnodebStatsCellRsp.ulState;

    logPrint(LOG_DEBUG, "Status :[%d,%s]\n",
             g_AppCtxRf.ulCellState,
             (g_AppCtxRf.ulCellState == cOCTVC1_LTE_CELL_STATE_ENUM_INVALID) ? "INVALID" : (g_AppCtxRf.ulCellState == cOCTVC1_LTE_CELL_STATE_ENUM_IDLE)  ? "IDLE"
                                                                                       : (g_AppCtxRf.ulCellState == cOCTVC1_LTE_CELL_STATE_ENUM_READY)   ? "READY"
                                                                                       : (g_AppCtxRf.ulCellState == cOCTVC1_LTE_CELL_STATE_ENUM_RUNNING) ? "RUNNING"
                                                                                                                                                         : "UNKNOWN");

    return 0;

ErrorHandling:

    return -1;
}

void enableTxCalibrationFor2G()
{
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD TxCalInfoCmd;
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP TxCalInfoRsp;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD TxCalModifyCmd;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP TxCalModifyRsp;

    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
    TxCalInfoCmd.ulPortIndex = 0;

    if (TRUE == isItPrimaryDsp())
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
    }
    else
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
    }

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &TxCalInfoCmd;
    CmdExecuteParms.pRsp = &TxCalInfoRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_2G);
        return;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

    if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_UNAVAILABLE)
    {
        sendTxCalibrationRsp(FXL_FAILURE, 2, FXL_RAT_2G);
        return;
    }
    else if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_INACTIVE)
    {
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_DEF(&TxCalModifyCmd);
        if (TRUE == isItPrimaryDsp())
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdSec;
        }
        TxCalModifyCmd.ulPortIndex = 0;
        TxCalModifyCmd.ulActivateFlag = cOCT_TRUE;
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalModifyCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalModifyCmd;
        CmdExecuteParms.pRsp = &TxCalModifyRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP);

        if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
            ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_2G);
            return;
        }

        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
        TxCalInfoCmd.ulPortIndex = 0;

        if (TRUE == isItPrimaryDsp())
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
        }

        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalInfoCmd;
        CmdExecuteParms.pRsp = &TxCalInfoRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

        if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
            ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_2G);
            return;
        }

        /*
         * Swap the message response.
         */
        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

        if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_ACTIVE)
        {
            sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_2G);
        }
        else
        {
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_2G);
        }
    }
    else
    {
        sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_2G);
    }
}

void enableTxCalibrationFor4G()
{
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD TxCalInfoCmd;
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP TxCalInfoRsp;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD TxCalModifyCmd;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP TxCalModifyRsp;

    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
    TxCalInfoCmd.ulPortIndex = 0;

    if (TRUE == isItPrimaryDsp())
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
    }
    else
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
    }

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &TxCalInfoCmd;
    CmdExecuteParms.pRsp = &TxCalInfoRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

    if (g_AppCtxRf.PktApiInfo.pPktApiSess != NULL)
    {
        logPrint(LOG_DEBUG, "execute the PKT API");
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    }

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_4G);
        return;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

    if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_UNAVAILABLE)
    {
        sendTxCalibrationRsp(FXL_FAILURE, 2, FXL_RAT_4G);
        return;
    }
    else if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_INACTIVE)
    {
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_DEF(&TxCalModifyCmd);
        if (TRUE == isItPrimaryDsp())
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdSec;
        }
        TxCalModifyCmd.ulPortIndex = 0;
        TxCalModifyCmd.ulActivateFlag = cOCT_TRUE;
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalModifyCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalModifyCmd;
        CmdExecuteParms.pRsp = &TxCalModifyRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP);

        if (g_AppCtxRf.PktApiInfo.pPktApiSess != NULL)
            ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_4G);
            return;
        }

        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
        TxCalInfoCmd.ulPortIndex = 0;
        if (TRUE == isItPrimaryDsp())
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
        }
        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalInfoCmd;
        CmdExecuteParms.pRsp = &TxCalInfoRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

        if (g_AppCtxRf.PktApiInfo.pPktApiSess != NULL)
            ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_4G);
            return;
        }

        /*
         * Swap the message response.
         */
        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

        if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_ACTIVE)
        {
            sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_4G);
        }
        else
        {
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_4G);
        }
    }
    else
    {
        sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_4G);
    }
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebOpen

Description:    Open LTE Enodeb

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebOpen(void)
{
    tOCTVC1_LTE_MSG_ENODEB_OPEN_CMD EnodebOpenCmd;
    tOCTVC1_LTE_MSG_ENODEB_OPEN_RSP EnodebOpenRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_OPEN_CMD_CID ----\n");
    mOCTVC1_LTE_MSG_ENODEB_OPEN_CMD_DEF(&EnodebOpenCmd);

    EnodebOpenCmd.ulRfPortIndex = 0; // Default is 0 .. may already be used by another app.
                                     // If so use tOCTVC1_HW_MSG_RF_PORT_INFO_CMD to find unused port

    // cell config
    EnodebOpenCmd.CellConfig.ulDuplexMode = g_AppCtxRf.ulDuplexMode;
    EnodebOpenCmd.CellConfig.ulCpType = g_AppCtxRf.ulCpType;
    EnodebOpenCmd.CellConfig.ulBwId = g_AppCtxRf.ulBwId;

    // RF config
    EnodebOpenCmd.RfConfig.ulDownlinkEarfcn = g_AppCtxRf.ulDlEarfcn;
    EnodebOpenCmd.RfConfig.ulConfigSource = cOCTVC1_LTE_RF_CONFIG_SOURCE_ENUM_EARFCN;

    if (EnodebOpenCmd.CellConfig.ulDuplexMode)
    {
        EnodebOpenCmd.CellTddConfig.ulSpecialSubframeConfigIdx = g_AppCtxRf.ulTddSpecialSfConfig; // octasic recommends 7 only
        EnodebOpenCmd.CellTddConfig.ulUplinkDownlinkConfigIdx = g_AppCtxRf.ulTddConfig;           // octasic supports UL/DL config 1
        EnodebOpenCmd.ulFrameBorderAlignEnableFlag = cOCT_TRUE;

        if (g_AppCtxRf.ulBwId == cOCTVC1_LTE_BANDWIDTH_ENUM_1p4MHZ)
            EnodebOpenCmd.ulFrameBorderDelaySamples = (((192 * g_AppCtxRf.relativeD) + 1536) / 3072);
        else if (g_AppCtxRf.ulBwId == cOCTVC1_LTE_BANDWIDTH_ENUM_3MHZ)
            EnodebOpenCmd.ulFrameBorderDelaySamples = (((384 * g_AppCtxRf.relativeD) + 1536) / 3072);
        else if (g_AppCtxRf.ulBwId == cOCTVC1_LTE_BANDWIDTH_ENUM_5MHZ)
            EnodebOpenCmd.ulFrameBorderDelaySamples = (((768 * g_AppCtxRf.relativeD) + 1536) / 3072);
        else if (g_AppCtxRf.ulBwId == cOCTVC1_LTE_BANDWIDTH_ENUM_10MHZ)
            EnodebOpenCmd.ulFrameBorderDelaySamples = (((1536 * g_AppCtxRf.relativeD) + 1536) / 3072);
        else if (g_AppCtxRf.ulBwId == cOCTVC1_LTE_BANDWIDTH_ENUM_20MHZ)
            EnodebOpenCmd.ulFrameBorderDelaySamples = (((3072 * g_AppCtxRf.relativeD) + 1536) / 3072);
    }
    mOCTVC1_LTE_MSG_ENODEB_OPEN_CMD_SWAP(&EnodebOpenCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebOpenCmd;
    CmdExecuteParms.pRsp = &EnodebOpenRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebOpenRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_GENERIC_RC_NO_MORE_OBJECT == ulResult)
        ulResult = cOCTVC1_RC_OK;

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_OPEN_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_LTE_MSG_ENODEB_OPEN_RSP_SWAP(&EnodebOpenRsp);

    // Keep new Enodeb
    g_AppCtxRf.hEnodeb = EnodebOpenRsp.hEnodeb;
    // logPrint(LOG_DEBUG,"\tDone ... ({0x%08x})\n",g_AppCtxRf.hEnodeb);

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebCellStart

Description:    Start LTE Enodeb Cell

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebCellStart(void)
{
    tOCTVC1_LTE_MSG_ENODEB_START_CELL_CMD EnodebStartCellCmd;
    tOCTVC1_LTE_MSG_ENODEB_START_CELL_RSP EnodebStartCellRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_START_CELL_CMD_CID ----\n");

    mOCTVC1_LTE_MSG_ENODEB_START_CELL_CMD_DEF(&EnodebStartCellCmd);
    EnodebStartCellCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_START_CELL_CMD_SWAP(&EnodebStartCellCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebStartCellCmd;
    CmdExecuteParms.pRsp = &EnodebStartCellRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebStartCellRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_START_CELL_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }
    // logPrint(LOG_DEBUG,"\tDone\n");

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebClose

Description:    Close LTE Enodeb Cell

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebClose(void)
{
    tOCTVC1_LTE_MSG_ENODEB_CLOSE_CMD EnodebCloseCmd;
    tOCTVC1_LTE_MSG_ENODEB_CLOSE_RSP EnodebCloseRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    mOCTVC1_LTE_MSG_ENODEB_CLOSE_CMD_DEF(&EnodebCloseCmd);
    EnodebCloseCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_CLOSE_CMD_SWAP(&EnodebCloseCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebCloseCmd;
    CmdExecuteParms.pRsp = &EnodebCloseRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebCloseRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: tOCTVC1_LTE_MSG_ENODEB_CLOSE_CMD failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    g_AppCtxRf.hEnodeb = 0;

    // logPrint(LOG_DEBUG,"\tDone\n");
    return 0;

ErrorHandling:
    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebCellStop

Description:    Stop LTE Enodeb Cell

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebCellStop(void)
{
    tOCTVC1_LTE_MSG_ENODEB_STOP_CELL_CMD EnodebStopCellCmd;
    tOCTVC1_LTE_MSG_ENODEB_STOP_CELL_RSP EnodebStopCellRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_STOP_CELL_CMD_CID ----\n");
    mOCTVC1_LTE_MSG_ENODEB_STOP_CELL_CMD_DEF(&EnodebStopCellCmd);
    EnodebStopCellCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    mOCTVC1_LTE_MSG_ENODEB_STOP_CELL_CMD_SWAP(&EnodebStopCellCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebStopCellCmd;
    CmdExecuteParms.pRsp = &EnodebStopCellRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebStopCellRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_STOP_CELL_CMD_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }
    // logPrint(LOG_DEBUG,"\tDone\n");
    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebModifyCell

Description:    Modify LTE Enodeb Cell properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebModifyCell(void)
{
    tOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_CMD EnodebModifyCellCmd;
    tOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_RSP EnodebModifyCellRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    // logPrint(LOG_DEBUG,"\n+-- cOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_CID ----\n");
    mOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_CMD_DEF(&EnodebModifyCellCmd);
    EnodebModifyCellCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    // cell config
    EnodebModifyCellCmd.CellConfig.ulDuplexMode = g_AppCtxRf.ulDuplexMode;
    EnodebModifyCellCmd.CellConfig.ulCpType = g_AppCtxRf.ulCpType;
    EnodebModifyCellCmd.CellConfig.ulBwId = g_AppCtxRf.ulBwId;

    // TDD config
    EnodebModifyCellCmd.CellTddConfig.ulUplinkDownlinkConfigIdx = g_AppCtxRf.ulTddConfig;
    EnodebModifyCellCmd.CellTddConfig.ulSpecialSubframeConfigIdx = g_AppCtxRf.ulTddSpecialSfConfig;

    mOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_CMD_SWAP(&EnodebModifyCellCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebModifyCellCmd;
    CmdExecuteParms.pRsp = &EnodebModifyCellRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebModifyCellRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_LTE_MSG_ENODEB_MODIFY_CELL_RSP_SWAP(&EnodebModifyCellRsp);

    logPrint(LOG_DEBUG, "LTE Modify Cell :[EARFCN:%d, BwId:%d]\n",
             g_AppCtxRf.ulDlEarfcn,
             g_AppCtxRf.ulBwId);

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       EnodebModifyRfParams

Description:    Modify LTE Enodeb Rf Parameters

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static int EnodebModifyRfParams(void)
{
    if (TRUE == isItPrimaryDsp())
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdPri)
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }

        if (FXL_TX_RX_ANTENNA_ID_1 == rxAntennaIdPri)
        {
            if (0 != HwRfPortModifyAntennaRxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaRxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaRxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaRxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
    }
    else
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdSec)
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }

        if (FXL_TX_RX_ANTENNA_ID_1 == rxAntennaIdSec)
        {
            if (0 != HwRfPortModifyAntennaRxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaRxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaRxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaRxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
    }

    return 0;

ErrorHandling:

    return -1;
}

static int EnodebModifyRf(void)
{
    tOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD EnodebModifyRfCmd;
    tOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_RSP EnodebModifyRfRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    logPrint(LOG_DEBUG, "\n+-- cOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CID ----\n");

    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD_DEF(&EnodebModifyRfCmd);

    EnodebModifyRfCmd.hEnodeb = g_AppCtxRf.hEnodeb;

    // RF config
    EnodebModifyRfCmd.RfConfig.ulConfigSource = cOCTVC1_LTE_RF_CONFIG_SOURCE_ENUM_EARFCN;
    EnodebModifyRfCmd.RfConfig.ulDownlinkEarfcn = g_AppCtxRf.ulDlEarfcn;

    if (g_AppCtxRf.ulDlEarfcn != cOCTVC1_DO_NOT_MODIFY)
    {
        EnodebModifyRfCmd.RfConfig.ulUplinkEarfcn = cOCTVC1_LTE_AUTO;
    }

    if (TRUE == isItPrimaryDsp())
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdPri)
        {
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[0].ulPhysicalAntennaId = 1;
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[1].ulPhysicalAntennaId = 0;
        }
        else
        {
            /* Default aTxLogicalAntennaConfig[0] = 0 & aTxLogicalAntennaConfig[1] = 1.
             * By default second antenna is disabled.
             */
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[0].ulPhysicalAntennaId = 0;
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[1].ulPhysicalAntennaId = 1;
        }

        if (FXL_TX_RX_ANTENNA_ID_1 == rxAntennaIdPri)
        {
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[0].ulPhysicalAntennaId = 1;
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[1].ulPhysicalAntennaId = 0;
        }
        else
        {
            /* Default aRxLogicalAntennaConfig[0] = 0 & aRxLogicalAntennaConfig[1] = 1.
             * By default second antenna is disabled.
             */
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[0].ulPhysicalAntennaId = 0;
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[1].ulPhysicalAntennaId = 1;
        }
    }
    else
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdSec)
        {
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[0].ulPhysicalAntennaId = 1;
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[1].ulPhysicalAntennaId = 0;
        }
        else
        {
            /* Default aTxLogicalAntennaConfig[0] = 0 & aTxLogicalAntennaConfig[1] = 1.
             * By default second antenna is disabled.
             */
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[0].ulPhysicalAntennaId = 0;
            EnodebModifyRfCmd.aTxLogicalAntennaConfig[1].ulPhysicalAntennaId = 1;
        }

        if (FXL_TX_RX_ANTENNA_ID_1 == rxAntennaIdSec)
        {
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[0].ulPhysicalAntennaId = 1;
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[1].ulPhysicalAntennaId = 0;
        }
        else
        {
            /* Default aRxLogicalAntennaConfig[0] = 0 & aRxLogicalAntennaConfig[1] = 1.
             * By default second antenna is disabled.
             */
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[0].ulPhysicalAntennaId = 0;
            EnodebModifyRfCmd.aRxLogicalAntennaConfig[1].ulPhysicalAntennaId = 1;
        }
    }

    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD_SWAP(&EnodebModifyRfCmd);

    // configure packet API
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &EnodebModifyRfCmd;
    CmdExecuteParms.pRsp = &EnodebModifyRfRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(EnodebModifyRfRsp);

    // execute
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    // check result
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    // swap response
    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_RSP_SWAP(&EnodebModifyRfRsp);

    // Tx Attenuation and Rx Gain parameters
    //	EnodebModifyRfParams();

    return 0;

ErrorHandling:

    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       lteEnodebConfig

Description:    Configure LTE Enodeb prameters

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*static*/ int lteEnodebConfig(lteRfConfig *rfConfig)
{
    tOCT_UINT32 ulResult;

    if (rfConfig->f_enodebStartOrStop == TRUE) //
    {
        memcpy(g_AppCfg.abyProcessorCtrlMacAddr, g_L1CConfigDB.l23SrcMacAddr, 6);
        memcpy(g_AppCfg.abyTargetCtrlMacAddr, g_L1CConfigDB.l1DstMacAddr, 6);

        // init Application : Open pktApi
        ulResult = OctSamplesOpenPktApiSession(&g_AppCtxRf.PktApiInfo,
                                               g_AppCfg.abyProcessorCtrlMacAddr,
                                               g_AppCfg.abyTargetCtrlMacAddr);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: OctSamplesOpenPktApiSession() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
            return -1;
        }

        /*
         * Start LTE Module (if not already started).
         */
        if (0 != ModuleStartById(cOCTVC1_LTE_APPLICATION_ID_MODULE_PHY_ENODEB))
        {
            goto ErrorHandling;
        }

        // Get Handle Object
        if (0 != LteEnodebGetHandle())
        {
            logPrint(LOG_DEBUG, "LteEnodebGetHandle FAILED... EXITING \n");
            exit(0);
        }

        g_AppCtxRf.ulDlEarfcn = rfConfig->earfcn;
        g_AppCtxRf.ulBwId = rfConfig->cellBandWidth;       // cOCTVC1_LTE_BANDWIDTH_ENUM_10MHZ; // 10Mhz
        g_AppCtxRf.ulRfTxAttndB = -1 * rfConfig->txAttnDb; // 0 or negative values only
        g_AppCtxRf.ulRfRxGaindB = rfConfig->ulRxGainDb;    // 56;//70; // 140
        g_AppCtxRf.relativeD = rfConfig->relativeD;
        if (rfConfig->islteTDD)
        {
            g_AppCtxRf.ulDuplexMode = cOCTVC1_LTE_DUPLEX_MODE_ENUM_TDD; // Default
            g_AppCtxRf.ulTddConfig = LTE_TDD_UD_CONFIG;                 // TDD as per octasic configuration
            g_AppCtxRf.ulTddSpecialSfConfig = LTE_TDD_SPECIALSUBF_ASSG; // TDD as per octasic configuration
        }
        else
        {
            g_AppCtxRf.ulDuplexMode = cOCTVC1_LTE_DUPLEX_MODE_ENUM_FDD;                       // Default
            g_AppCtxRf.ulTddConfig = cOCTVC1_LTE_TDD_UL_DL_CONFIG_ENUM_INVALID;               // default invalid for FDD
            g_AppCtxRf.ulTddSpecialSfConfig = cOCTVC1_LTE_TDD_SPECIAL_SF_CONFIG_ENUM_INVALID; // default invalid for FDD
        }
        g_AppCtxRf.ulCpType = cOCTVC1_LTE_CP_TYPE_ENUM_NORMAL; // default

        // Test if Enodeb is open
        if (g_AppCtxRf.hEnodeb == 0)
        {

            // if(rfConfig->islteTDD == TRUE)
            {
                tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATS_RSP clkStatus;
                unsigned int SystemClockWaitTimeMs = 25 * (4 * 250); // MSec

                // Wait for the system clock is locked
                while ((SystemClockWaitTimeMs -= 250) != 0)
                {
                    if (CheckClkMgrStatus(&clkStatus, g_AppCtxRf.PktApiInfo.pPktApiSess) != 0)
                        goto ErrorHandling;

                    // Make sure the the Clk Status is locked
                    if (clkStatus.ulState == cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_LOCKED)
                    {
                        if (isItPrimaryDsp() == TRUE)
                        {
                            OctOsalSleepMs(300); // Add extra 250ms for primary
                        }

                        break;
                    }

                    OctOsalSleepMs(250);
                };
                logPrint(LOG_DEBUG, "Before calling Enodebopen Status of the system clock is: %s\n", (SystemClockWaitTimeMs == 0) ? "NOT LOCKED (WARNING)!" : "LOCKED");
                if ((rfConfig->islteTDD == TRUE) && (clkStatus.ulState != cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_LOCKED))
                {
                    goto ErrorHandling;
                }
            }
            // Enodeb is not open.
            if (0 != EnodebOpen())
            {
                goto ErrorHandling;
            }
            /* Modify TxAttn  */
            if (0 != EnodebModifyRf())
            {
                goto ErrorHandling;
            }
        }
        else
        {

            // Get Cell status
            if (0 != EnodebCellStatus())
            {
                goto ErrorHandling;
            }

            // Must stop Enodeb.Cell if started
            if (g_AppCtxRf.ulCellState != cOCTVC1_LTE_CELL_STATE_ENUM_IDLE)
            {
                if (0 != EnodebCellStop())
                {
                    goto ErrorHandling;
                }
            }
            // Modify Earfcn and BWId
            if (0 != EnodebModifyCell())
            {
                goto ErrorHandling;
            }
            // Modify TxAttn
            if (0 != EnodebModifyRf())
            {
                goto ErrorHandling;
            }
        }

        if (0 != EnodebModifyRfParams())
        {
            goto ErrorHandling;
        }

        if (0 != EnodebCellStart())
        {
            goto ErrorHandling;
        }

        if (0 != EnodebInfoProperties(cOCT_TRUE))
        {
            goto ErrorHandling;
        }

        // Get Cell status
        EnodebCellStatus();
    }
    else
    {
        int counter = 0;
        while (g_AppCtxRf.ulCellState != cOCTVC1_LTE_CELL_STATE_ENUM_IDLE)
        {
            if (0 != EnodebCellStop())
            {
                goto ErrorHandling;
            }

            sleep(1);

            if (0 != EnodebCellStatus())
            {
                goto ErrorHandling;
            }
            if (++counter == 3)
            {
                break;
            }

            logPrint(LOG_DEBUG, "Cell-Stop Counter = %d\n", counter);
        }

        if (rfConfig->earfcn == 1) // Dsp Reset is Needed
        {
            EnodebClose();
        }

        ulResult = OctSamplesClosePktApiSession(&g_AppCtxRf.PktApiInfo);
        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "Error: OctSamplesClosePktApiSession() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
            fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        }
    }
    return 0;

ErrorHandling:
    ulResult = OctSamplesClosePktApiSession(&g_AppCtxRf.PktApiInfo);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: OctSamplesClosePktApiSession() failed,  rc = 0x%08x (%s)\n", ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
    }
    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       LteCellModifyRf

Description:    Modify LTE Cell Rf properties

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*static*/ int LteCellModifyRf(int txAttnDb)
{
    txAttnDb = -1 * txAttnDb;
    g_AppCtxRf.ulRfTxAttndB = txAttnDb;

    if (TRUE == isItPrimaryDsp())
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdPri)
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
    }
    else
    {
        if (FXL_TX_RX_ANTENNA_ID_1 == txAntennaIdSec)
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 1))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 1, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
        else
        {
            if (0 != HwRfPortModifyAntennaTxConfig(0, 0))
            {
                goto ErrorHandling;
            }

            if (0 != HwRfPortInfoAntennaTxConfig(0, 0, cOCT_TRUE))
            {
                goto ErrorHandling;
            }
        }
    }

    return 0;

ErrorHandling:
    return -1;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 *
 * Function:       LteCellModifyRfEARFCN
 *
 * Description:    Modify LTE Cell Rf properties
 *
 * \*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*static*/ int LteCellModifyRfEARFCN(int earfcn)
{

    tOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD LteCellModifyRfCmd;
    tOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_RSP LteCellModifyRfRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;
    logPrint(LOG_ERR, "Inside LteCellModifyRfEARFCN  value of EARFCN recieved is %d", earfcn);

    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD_DEF(&LteCellModifyRfCmd);
    LteCellModifyRfCmd.hEnodeb = g_AppCtxRf.hEnodeb;
    LteCellModifyRfCmd.RfConfig.ulConfigSource = cOCTVC1_LTE_RF_CONFIG_SOURCE_ENUM_EARFCN;
    LteCellModifyRfCmd.RfConfig.ulDownlinkEarfcn = g_AppCtxRf.ulDlEarfcn = earfcn;
    LteCellModifyRfCmd.RfConfig.ulUplinkEarfcn = cOCTVC1_LTE_AUTO;
    LteCellModifyRfCmd.RfConfig.ulDownlinkFrequencyKhz = cOCTVC1_DO_NOT_MODIFY;
    LteCellModifyRfCmd.RfConfig.ulUplinkFrequencyKhz = cOCTVC1_DO_NOT_MODIFY;

    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CMD_SWAP(&LteCellModifyRfCmd);
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &LteCellModifyRfCmd;
    CmdExecuteParms.pRsp = &LteCellModifyRfRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(LteCellModifyRfRsp);
    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
     * Swap the file open command response.
     */
    mOCTVC1_LTE_MSG_ENODEB_MODIFY_RF_RSP_SWAP(&LteCellModifyRfRsp);

    if (cOCTVC1_RC_OK == ulResult)
    {
        logPrint(LOG_ERR, "OctVc1PktApiSessCmdExecute ok  with updated EARFCN %d\n", LteCellModifyRfCmd.RfConfig.ulDownlinkEarfcn);
    }

    if (0 != EnodebInfoRf(cOCT_TRUE))
    {
        goto ErrorHandling;
    }

    return 0;

ErrorHandling:
    return -1;
}

unsigned int getLteBandFromEARFCN(unsigned int lteEarfcn)
{
    unsigned int lteBand = 0xFFFFFFFF;
    if ((lteEarfcn >= 0) && (lteEarfcn <= 599))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_1;
    }
    else if ((lteEarfcn >= 600) && (lteEarfcn <= 1199))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_2;
    }
    else if ((lteEarfcn >= 1200) && (lteEarfcn <= 1949))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_3;
    }
    else if ((lteEarfcn >= 1950) && (lteEarfcn <= 2399))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_4;
    }
    else if ((lteEarfcn >= 2400) && (lteEarfcn <= 2649))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_5;
    }
    else if ((lteEarfcn >= 2650) && (lteEarfcn <= 2749))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_6;
    }
    else if ((lteEarfcn >= 2750) && (lteEarfcn <= 3449))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_7;
    }
    else if ((lteEarfcn >= 3450) && (lteEarfcn <= 3799))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_8;
    }
    else if ((lteEarfcn >= 3800) && (lteEarfcn <= 4149))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_9;
    }
    else if ((lteEarfcn >= 4150) && (lteEarfcn <= 4749))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_10;
    }
    else if ((lteEarfcn >= 4750) && (lteEarfcn <= 4949))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_11;
    }
    else if ((lteEarfcn >= 5010) && (lteEarfcn <= 5179))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_12;
    }
    else if ((lteEarfcn >= 5180) && (lteEarfcn <= 5279))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_13;
    }
    else if ((lteEarfcn >= 5280) && (lteEarfcn <= 5379))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_14;
    }
    else if ((lteEarfcn >= 5730) && (lteEarfcn <= 5849))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_17;
    }
    else if ((lteEarfcn >= 5850) && (lteEarfcn <= 5999))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_18;
    }
    else if ((lteEarfcn >= 6000) && (lteEarfcn <= 6149))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_19;
    }
    else if ((lteEarfcn >= 6150) && (lteEarfcn <= 6449))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_20;
    }
    else if ((lteEarfcn >= 6450) && (lteEarfcn <= 6599))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_21;
    }
    else if ((lteEarfcn >= 6600) && (lteEarfcn <= 7399))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_22;
    }
    else if ((lteEarfcn >= 7500) && (lteEarfcn <= 7699))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_23;
    }
    else if ((lteEarfcn >= 7700) && (lteEarfcn <= 8039))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_24;
    }
    else if ((lteEarfcn >= 8040) && (lteEarfcn <= 8689))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_25;
    }
    else if ((lteEarfcn >= 8690) && (lteEarfcn <= 9039))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_26;
    }
    else if ((lteEarfcn >= 9040) && (lteEarfcn <= 9209))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_27;
    }
    else if ((lteEarfcn >= 9210) && (lteEarfcn <= 9659))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_28;
    }
    else if ((lteEarfcn >= 9660) && (lteEarfcn <= 9769))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_29;
    }
    else if ((lteEarfcn >= 9770) && (lteEarfcn <= 9869))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_30;
    }
    else if ((lteEarfcn >= 9870) && (lteEarfcn <= 9919))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_31;
    }
    else if ((lteEarfcn >= 9920) && (lteEarfcn <= 10359))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_32;
    }
    else if ((lteEarfcn >= 36000) && (lteEarfcn <= 36199))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_33;
    }
    else if ((lteEarfcn >= 36200) && (lteEarfcn <= 36349))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_34;
    }
    else if ((lteEarfcn >= 36350) && (lteEarfcn <= 36949))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_35;
    }
    else if ((lteEarfcn >= 36950) && (lteEarfcn <= 37549))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_36;
    }
    else if ((lteEarfcn >= 37550) && (lteEarfcn <= 37749))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_37;
    }
    else if ((lteEarfcn >= 37750) && (lteEarfcn <= 38249))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_38;
    }
    else if ((lteEarfcn >= 38250) && (lteEarfcn <= 38649))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_39;
    }
    else if ((lteEarfcn >= 38650) && (lteEarfcn <= 39649))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_40;
    }
    else if ((lteEarfcn >= 39650) && (lteEarfcn <= 41589))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_41;
    }
    else if ((lteEarfcn >= 41590) && (lteEarfcn <= 43589))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_42;
    }
    else if ((lteEarfcn >= 43590) && (lteEarfcn <= 45589))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_43;
    }
    else if ((lteEarfcn >= 45590) && (lteEarfcn <= 46589))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_44;
    }
    else if ((lteEarfcn >= 46590) && (lteEarfcn <= 46789))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_45;
    }
    else if ((lteEarfcn >= 46790) && (lteEarfcn <= 54539))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_46;
    }
    else if ((lteEarfcn >= 54540) && (lteEarfcn <= 55239))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_47;
    }
    else if ((lteEarfcn >= 55240) && (lteEarfcn <= 56739))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_48;
    }
    else if ((lteEarfcn >= 58240) && (lteEarfcn <= 59089))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_50;
    }
    else if ((lteEarfcn >= 59090) && (lteEarfcn <= 59139))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_51;
    }
    else if ((lteEarfcn >= 65536) && (lteEarfcn <= 66435))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_65;
    }
    else if ((lteEarfcn >= 66436) && (lteEarfcn <= 67335))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_66;
    }
    else if ((lteEarfcn >= 67336) && (lteEarfcn <= 67535))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_67;
    }
    else if ((lteEarfcn >= 67536) && (lteEarfcn <= 67835))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_68;
    }
    else if ((lteEarfcn >= 67836) && (lteEarfcn <= 68335))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_69;
    }
    else if ((lteEarfcn >= 68336) && (lteEarfcn <= 68585))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_70;
    }
    else if ((lteEarfcn >= 68586) && (lteEarfcn <= 68935))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_71;
    }
    else if ((lteEarfcn >= 68936) && (lteEarfcn <= 68985))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_72;
    }
    else if ((lteEarfcn >= 69036) && (lteEarfcn <= 69465))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_74;
    }
    else if ((lteEarfcn >= 69466) && (lteEarfcn <= 70315))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_75;
    }
    else if ((lteEarfcn >= 70316) && (lteEarfcn <= 70365))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_76;
    }
    else if ((lteEarfcn >= 255144) && (lteEarfcn <= 256143))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_252;
    }
    else if ((lteEarfcn >= 260894) && (lteEarfcn <= 262143))
    {
        lteBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_LTE_ENUM_255;
    }
    else
    {
        logPrint(LOG_DEBUG, "INVALID ARFCN PASSED ARFCN = %d\n", lteEarfcn);
        assert(0);
    }

    return lteBand;
}

void configure4gWnmParameters(fxL4gListenModeConfigureBandsCmd *cmd)
{
    int i = 0;

#ifdef CELL_ANTENNA_CHANGE
    prmSetup.ulEntryCount = cmd->noOfBandsToScan;
    bandIndex = 0;
#else
    tWnm.PrmSetup.ulEntryCount = cmd->noOfBandsToScan;
#endif

    bContinuousMonitoringEnable = cmd->noOfInterationsToScan;

  
#ifdef CELL_ANTENNA_CHANGE
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        prmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_LTE;
        prmSetup.aEntryList[i].ulBand = cmd->band[i];
        prmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        prmSetup.aEntryList[i].ulUarfcnStop = 123;
        prmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        prmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        prmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        prmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#else
    for (i = 0; i < cmd->noOfBandsToScan; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_LTE;
        tWnm.PrmSetup.aEntryList[i].ulBand = cmd->band[i];
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cOCTVC1_RUS_WNM_SEARCH_ENTRY_ALL_RFCN;
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = 123;
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }
#endif
    //  }
    return;
}

fxLResult configureLteFreqForSelectiveScan(fxL4gListenModeConfigureSelectiveFreqCmd *cmd)
{
    fxLResult result = FXL_SUCCESS;
    unsigned int band = 0xFFFFFFFF;

    int i = 0;
    tWnm.PrmSetup.ulEntryCount = cmd->noOfEarfcnConfigured;
    assert(cmd->noOfEarfcnConfigured < MAX_NO_EARFCN_FOR_SELECTIVE_SCAN);

#ifdef CELL_ANTENNA_CHANGE
    bandIndex = -1;
#endif

    bContinuousMonitoringEnable = cmd->noOfScanIterations;   

   // logPrint(LOG_DEBUG, "LTE selective scan cmd->noOfInterationsToScan = %d\n", cmd->noOfInterationsToScan);
   // logPrint(LOG_DEBUG, "LTE selective scan bContinuousMonitoringEnable = %d \n", bContinuousMonitoringEnable);
    if (tWnm.PrmSetup.ulEntryCount == 0)
    {
        logPrint(LOG_DEBUG, "Num of LTE-ARFCNS to Scan is 0, Please configure atleast 1 EARFCN to scan\n");
        return;
    }

    for (i = 0; i < tWnm.PrmSetup.ulEntryCount; i++)
    {
        tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_LTE;
        band = getLteBandFromEARFCN(cmd->earfcn[i]);
        if (0xFFFFFFFF != band)
        {
            tWnm.PrmSetup.aEntryList[i].ulBand = band;
        }
        else
        {
            tWnm.PrmSetup.aEntryList[i].ulBand = 0;
            result = FXL_FAILURE;
        }

        tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = cmd->earfcn[i];
        tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = cmd->earfcn[i];
        tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
        tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
        tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;
    }

    return result;
}

tOCT_UINT32 InitGsmIdCatcherApplication(void)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    /*
     * Open a transport packet API session.
     */
    memcpy(g_AppCfg.abyProcessorCtrlMacAddr, g_L1CConfigDB.l23SrcMacAddr, 6);
    memcpy(g_AppCfg.abyTargetCtrlMacAddr, g_L1CConfigDB.l1DstMacAddr, 6);

    ulResult = OctSamplesOpenPktApiSession(&g_AppCtxGsm.PktApiInfo,
                                           g_AppCfg.abyProcessorCtrlMacAddr,
                                           g_AppCfg.abyTargetCtrlMacAddr);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: OctSamplesOpenPktApiSession() failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        return ulResult;
    }

    return 0;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       PrintDeviceInfo

Description:    Prints the current hardware configuration parameters.
                Uses syncronous messaging , meaning we use OctVc1PktApiSessCmdExecute
                which is a blocking function

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

tOCT_UINT32 PrintDeviceInfo(void)
{
    tOCTVC1_MAIN_MSG_TARGET_INFO_CMD DeviceInfoCmd;
    tOCTVC1_MAIN_MSG_TARGET_INFO_RSP DeviceInfoRsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    char szDeviceType[20];
    char *szResult;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_TARGET_INFO_CMD_DEF(&DeviceInfoCmd);
    mOCTVC1_MAIN_MSG_TARGET_INFO_CMD_SWAP(&DeviceInfoCmd);

    /*
     * Execute the command. This is the main distinguishing feature of syncronous messaging,
     * we "Execute" the command, meaning we do not continue until we receive some response
     * or a timeout
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &DeviceInfoCmd;
    CmdExecuteParms.pRsp = &DeviceInfoRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(DeviceInfoRsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        logPrint(LOG_INFO, "PrintDeviceInfo COmmand FAILED\n");
        goto ErrorHandling;
    }

    /*
     * Swap the command response.
     */
    mOCTVC1_MAIN_MSG_TARGET_INFO_RSP_SWAP(&DeviceInfoRsp);

    switch (DeviceInfoRsp.ulTargetType)
    {
    case cOCTDEV_DEVICES_TYPE_ENUM_INVALID:
        sprintf(szDeviceType, "INVALID");
        break;
    case cOCTDEV_DEVICES_TYPE_ENUM_OCT1010:
        sprintf(szDeviceType, "OCT1010");
        break;
    case cOCTDEV_DEVICES_TYPE_ENUM_OCT2200:
        sprintf(szDeviceType, "OCT2200");
        break;
    }

    /*
     * Print the information.
     */
    logPrint(LOG_INFO, "+-- DEVICE INFORMATION ------------------------------------------------------\n");
    // logPrint(LOG_INFO, "| DeviceType           : %s\n", szDeviceType);
    logPrint(LOG_INFO, "| DeviceInfo :\n");
    szResult = strtok((char *)DeviceInfoRsp.abyTargetInfo, ";");
    while (NULL != szResult)
    {
        logPrint(LOG_INFO, "| %s\n", szResult);
        szResult = strtok(NULL, ";");
    }

    logPrint(LOG_INFO, "| \n");
    szResult = strtok((char *)DeviceInfoRsp.abyUserInfo, ";\n");

    logPrint(LOG_INFO, "| UserInfo :\n");
    while (NULL != szResult)
    {
        logPrint(LOG_INFO, "| %s\n", szResult);
        szResult = strtok(NULL, ";\n");
    }
    logPrint(LOG_INFO, "|\n\n");

    return cOCTVC1_RC_OK;

ErrorHandling:
    return ulResult;
}

void ExitGsmIdCatcherApplication(void)
{
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    /*
     * Close the packet API session.
     */
    ulResult = OctSamplesClosePktApiSession(&g_AppCtxGsm.PktApiInfo);
    if (cOCTVC1_RC_OK != ulResult)
    {
        logPrint(LOG_ERR, "ExitGsmIdCatcherApplication(%s) failed \n", octvc1_rc2string(ulResult));
    }

    g_AppCtxGsm.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiSess = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiCnct = NULL;
}

static char *GetClockSyncMgrStatusStateStr(tOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM f_ulState)
{
    switch (f_ulState)
    {
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_UNINITIALIZE:
        return "UNINITIALIZE";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_UNUSED:
        return "UNUSED";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_NO_EXT_CLOCK:
        return "NO_EXT_CLOCK";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_LOCKED:
        return "LOCKED";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_UNLOCKED:
        return "UNLOCKED";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_ERROR:
        return "ERROR";
    case cOCTVC1_HW_CLOCK_SYNC_MGR_STATE_ENUM_DISABLE:
        return "DISABLE";
    default:
        return "UNKNOWNED";
    }
}

int EnableEventReception(tOCT_BOOL32 f_fEnableFlag)
{
    tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD Cmd;
    tOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    if (f_fEnableFlag)
        logPrint(LOG_INFO, "Enable event reception ...");
    else
        logPrint(LOG_INFO, "Disable event reception ...");
    /*
     * Prepare command data.
     */
    mOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD_DEF(&Cmd);

    /*
    Cmd.ulSessionIndex default to cOCTVC1_MAIN_API_SYSTEM_SESSION_INDEX_CURRENT_TRANSPORT */

    /* Enable to receive all event */
    Cmd.ulEvtActiveFlag = f_fEnableFlag;

    /* SWAP cmd ... if have to */
    mOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_EVT_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "Error: cOCTVC1_MAIN_MSG_API_SYSTEM_MODIFY_SESSION_CID failed, rc = 0x%08x\n", ulResult);
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    // logPrint(LOG_INFO, "EnableEventReception Done\n");
    return 0;

ErrorHandling:
    logPrint(LOG_ERR, "EnableEventReception failed \n");
    return -1;
}

int TrxOpen(void)
{
    tOCTVC1_GSM_MSG_TRX_OPEN_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_OPEN_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCTVC1_GSM_TRX_CONFIG Config;

    logPrint(LOG_INFO, "Open TRX with Gain = %d , Attenuation = %d...\n", gsmRxGainInDb, gsmTxAttenuationInDb);

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_OPEN_CMD_DEF(&Cmd);

    /* Enable to receive all event */
    Cmd.ulRfPortIndex = g_AppCtxGsm.ulRfPortIndex = trx_g->trx_id;
    Cmd.TrxId.byTrxId = g_AppCtxGsm.ulTrxId = 0;

    // Cmd.Config.ulHoppingFlag = cOCT_FALSE

    Cmd.Config.ulBand = convertToOct2gBandFromFxL(trx_g->bts.band);
    Cmd.Config.usArfcn = trx_g->arfcn;
    Cmd.Config.usBcchArfcn = trx_g->arfcn;
    Cmd.Config.usCentreArfcn = trx_g->arfcn;

    // This is a tweek to support both E_GSM and R_GSM in 900 band for octasic
    if ((Cmd.Config.ulBand == cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_P_900) && Cmd.Config.usArfcn > 124)
    {
        if (Cmd.Config.usArfcn >= 955 && Cmd.Config.usArfcn <= 974)
        {
            Cmd.Config.ulBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_R_900;
        }
        if (Cmd.Config.usArfcn >= 975 && Cmd.Config.usArfcn <= 1023)
        {
            Cmd.Config.ulBand = cOCTVC1_RADIO_STANDARD_FREQ_BAND_GSM_ENUM_E_900;
        }
    }

    // We calculate TSC from BSIC
    Cmd.Config.usTsc = (tOCT_UINT16)trx_g->bts.bsic & 7;

    // Set Gain
    Cmd.RfConfig.ulRxGainDb = gsmRxGainInDb;
    Cmd.RfConfig.ulTxAttndB = gsmTxAttenuationInDb;

    if (TRUE == isItPrimaryDsp())
    {
        Cmd.RfConfig.ulTxAntennaId = txAntennaIdPri;
        Cmd.RfConfig.ulRxAntennaId = rxAntennaIdPri;
    }
    else
    {
        Cmd.RfConfig.ulTxAntennaId = txAntennaIdSec;
        Cmd.RfConfig.ulRxAntennaId = rxAntennaIdSec;
    }

    /* Keep config to compare if already open */
    memcpy(&Config, &Cmd.Config, sizeof(Config));

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_OPEN_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK != ulResult)
    {
        if (ulResult == cOCTVC1_GSM_RC_TRX_ALREADY_OPENED)
        {
            tOCTVC1_GSM_MSG_TRX_INFO_CMD InfoCmd;
            tOCTVC1_GSM_MSG_TRX_INFO_RSP InfoRsp;

            mOCTVC1_GSM_MSG_TRX_OPEN_RSP_SWAP(&Rsp);

            logPrint(LOG_INFO, "Warning: GSM_RC_TRX_ALREADY_OPENED");

            mOCTVC1_GSM_MSG_TRX_INFO_CMD_DEF(&InfoCmd);
            InfoCmd.TrxId = Rsp.TrxId;
            mOCTVC1_GSM_MSG_TRX_INFO_CMD_SWAP(&InfoCmd);

            mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
            CmdExecuteParms.pCmd = &InfoCmd;
            CmdExecuteParms.pRsp = &InfoRsp;
            CmdExecuteParms.ulMaxRspLength = sizeof(InfoRsp);
            if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
                ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
            if (cOCTVC1_RC_OK != ulResult)
            {
                fprintf(stderr, "\nError: cOCTVC1_GSM_MSG_TRX_INFO_CID failed, rc = 0x%08x\n", ulResult);
                fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
                goto ErrorHandling;
            }

            mOCTVC1_GSM_MSG_TRX_INFO_RSP_SWAP(&InfoRsp);

            /* Test if was open with same param */
            if ((g_AppCtxGsm.ulRfPortIndex != InfoRsp.ulRfPortIndex) ||
                (memcmp(&Config, &InfoRsp.Config, sizeof(Config)) != 0))
            {
                ulResult = cOCTVC1_GSM_RC_TRX_ALREADY_OPENED;
                fprintf(stderr, "\nError: GSM_MSG_TRX_OPEN_CID with differant param.\n", ulResult);
                fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
                goto ErrorHandling;
            }
            g_AppCtxGsm.TrxId = Rsp.TrxId;
        }
        else
        {
            fprintf(stderr, "Error: GSM_MSG_TRX_OPEN_CID failed, rc = 0x%08x\n", ulResult);
            fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
            goto ErrorHandling;
        }
    }
    else
    {
        mOCTVC1_GSM_MSG_TRX_OPEN_RSP_SWAP(&Rsp);
        g_AppCtxGsm.TrxId = Rsp.TrxId;
        g_AppCtxGsm.hGsm = Rsp.hGsm;

        // Temporary fix for enabling events after TRX Close and Reopen
        ulResult = EnableEventReception(cOCT_TRUE);
        if (cOCTVC1_RC_OK != ulResult)
        {
            logPrint(LOG_ERR, "Could not enable Events in L1\n\n");
            logPrint(LOG_DEBUG, "Exiting... \n\n");
            exit(-1);
        }
    }
    logPrint(LOG_INFO, "Received GSM_MSG_TRX_OPEN_RSP for TRX %d\n", Rsp.TrxId.byTrxId);
    return 0;

ErrorHandling:
    return ulResult;
}

int GsmTrxModifyRf(tOCT_UINT32 TxAttenuationInDb)
{
    tOCTVC1_GSM_MSG_TRX_MODIFY_RF_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_MODIFY_RF_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    logPrint(LOG_INFO, "GSM TRX Modify RF for TX Attenuation = %d...", TxAttenuationInDb);

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_MODIFY_RF_CMD_DEF(&Cmd);

    Cmd.RfConfig.ulTxAttndB = TxAttenuationInDb;

    if (TRUE == isItPrimaryDsp())
    {
        Cmd.RfConfig.ulTxAntennaId = txAntennaIdPri;
        Cmd.RfConfig.ulRxAntennaId = rxAntennaIdPri;
    }
    else
    {
        Cmd.RfConfig.ulTxAntennaId = txAntennaIdSec;
        Cmd.RfConfig.ulRxAntennaId = rxAntennaIdSec;
    }

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_MODIFY_RF_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_MODIFY_RF_RSP_SWAP(&Rsp);
        // logPrint( LOG_INFO, "Done, Modified TRX RF\n" );
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int GsmTrxModifyTsDigitalScaling(tOCT_UINT8 nonApplicableTs)
{
    tOCTVC1_GSM_MSG_TRX_MODIFY_TS_DIGITAL_SCALING_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_MODIFY_TS_DIGITAL_SCALING_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;
    tOCT_UINT8 i = 0;

    logPrint(LOG_INFO, "GSM TRX Modify TS Digital Scaling(Except for TS %d)...", nonApplicableTs);

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_MODIFY_TS_DIGITAL_SCALING_CMD_DEF(&Cmd);

    Cmd.TSDigitalScaling[0] = 0; /* No change for TS - 0 */
    for (i = 1; i < MAX_TS - 1; i++)
    {
        Cmd.TSDigitalScaling[i] = 0xFF;
    }
    if (nonApplicableTs > 0 && nonApplicableTs < MAX_TS - 1)
        Cmd.TSDigitalScaling[nonApplicableTs] = 0; /* No Change for SilentCall TS */

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_MODIFY_TS_DIGITAL_SCALING_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_MODIFY_TS_DIGITAL_SCALING_RSP_SWAP(&Rsp);
        // logPrint( LOG_INFO, "Done, Modified TRX\n" );
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int GsmTrxResetDigitalScaling(void)
{
    tOCTVC1_GSM_MSG_TRX_RESET_TS_DIGITAL_SCALING_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_RESET_TS_DIGITAL_SCALING_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    logPrint(LOG_INFO, "GSM TRX Reset TS Digital Scaling...");

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_RESET_TS_DIGITAL_SCALING_CMD_DEF(&Cmd);

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_RESET_TS_DIGITAL_SCALING_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_RESET_TS_DIGITAL_SCALING_RSP_SWAP(&Rsp);
        // logPrint( LOG_INFO, "Done\n" );
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int TrxCloseAll(void)
{
    tOCTVC1_GSM_MSG_TRX_CLOSE_ALL_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_CLOSE_ALL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    logPrint(LOG_INFO, "Close all TRX ...");

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_CLOSE_ALL_CMD_DEF(&Cmd);

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_CLOSE_ALL_CMD_SWAP(&Cmd);
    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_CLOSE_ALL_RSP_SWAP(&Rsp);
        logPrint(LOG_INFO, "Done, closed (%d) TRX\n", Rsp.ulCloseCount);
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int PhysicalChanActivate(
    tOCTVC1_GSM_PHYSICAL_CHANNEL_ID f_PchId,
    tOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM f_ulChannelType,
    tOCTVC1_GSM_PAYLOAD_TYPE_ENUM f_ulPayloadType)
{
    tOCTVC1_GSM_MSG_TRX_ACTIVATE_PHYSICAL_CHANNEL_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_ACTIVATE_PHYSICAL_CHANNEL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    logPrint(LOG_INFO, "Activate physical channel Tslot(%d)...",
             f_PchId.byTimeslotNb);

    mOCTVC1_GSM_MSG_TRX_ACTIVATE_PHYSICAL_CHANNEL_CMD_DEF(&Cmd);

    Cmd.TrxId = g_AppCtxGsm.TrxId;
    Cmd.PchId = f_PchId;

    Cmd.ulChannelType = f_ulChannelType;
    Cmd.ulPayloadType = f_ulPayloadType;

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_ACTIVATE_PHYSICAL_CHANNEL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        // logPrint( LOG_INFO, "Done\n" );
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return -1;
}

int LogicalChanActivate(tOCTVC1_GSM_LOGICAL_CHANNEL_ID f_LchId)
{
    tOCTVC1_GSM_MSG_TRX_ACTIVATE_LOGICAL_CHANNEL_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_ACTIVATE_LOGICAL_CHANNEL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_ACTIVATE_LOGICAL_CHANNEL_CMD_DEF(&Cmd);

    Cmd.TrxId = g_AppCtxGsm.TrxId;
    Cmd.LchId = f_LchId;
    Cmd.Config.byTimingAdvance = 0;
    Cmd.Config.byBSIC = trx_g->bts.bsic;
    if (isTimeSlotTypeSupportsTCHF(f_LchId.byTimeslotNb) == TRUE)
        Cmd.LchId.bySubChannelNb = SUBCHF;

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_ACTIVATE_LOGICAL_CHANNEL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_ACTIVATE_LOGICAL_CHANNEL_RSP_SWAP(&Rsp);
#if 0
		logPrint(LOG_DEBUG,  "Activated logical channel Tslot(%d)-sub(%d)-SAPI(%d)-Dir(%d)...\n",
			Rsp.LchId.byTimeslotNb,
			Rsp.LchId.bySubChannelNb,
			Rsp.LchId.bySAPI,
			Rsp.LchId.byDirection);
#endif

        /* Enable cell bar if TCH Silent Call has started and default ID config is not Blacklist*/
#ifdef CELL_BAR_IN_GSM_TCH_SC
        if ((f_LchId.bySAPI == cOCTVC1_GSM_SAPI_ENUM_TCHF) && (f_LchId.byDirection == cOCTVC1_GSM_DIRECTION_ENUM_TX_BTS_MS) && (configUeIdList.defaultConfig != BLACKLIST) && (cellBarDuringTchSilentCall == TRUE))
        {
            trx_g->bts.rach_ctrl_par.cell_bar = 1;
            generateSysInfoMsgs();
        }
#endif
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int LogicalChanDeactivate(tOCTVC1_GSM_LOGICAL_CHANNEL_ID f_LchId)
{
    tOCTVC1_GSM_MSG_TRX_DEACTIVATE_LOGICAL_CHANNEL_CMD Cmd;
    tOCTVC1_GSM_MSG_TRX_DEACTIVATE_LOGICAL_CHANNEL_RSP Rsp;
    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    /*
     * Prepare command data.
     */
    mOCTVC1_GSM_MSG_TRX_DEACTIVATE_LOGICAL_CHANNEL_CMD_DEF(&Cmd);

    Cmd.TrxId = g_AppCtxGsm.TrxId;
    Cmd.LchId = f_LchId;

    if (isTimeSlotTypeSupportsTCHF(f_LchId.byTimeslotNb) == TRUE)
        Cmd.LchId.bySubChannelNb = SUBCHF;

    /* SWAP cmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_DEACTIVATE_LOGICAL_CHANNEL_CMD_SWAP(&Cmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &Cmd;
    CmdExecuteParms.pRsp = &Rsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(Rsp);
#ifdef CELL_BAR_IN_GSM_TCH_SC
    if ((f_LchId.bySAPI == cOCTVC1_GSM_SAPI_ENUM_TCHF) && (f_LchId.byDirection == cOCTVC1_GSM_DIRECTION_ENUM_TX_BTS_MS))
    {
        trx_g->bts.rach_ctrl_par.cell_bar = 0;
        generateSysInfoMsgs();
    }
#endif
    if (g_AppCtxGsm.PktApiInfo.pPktApiSess != NULL)
        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxGsm.PktApiInfo.pPktApiSess, &CmdExecuteParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        mOCTVC1_GSM_MSG_TRX_DEACTIVATE_LOGICAL_CHANNEL_RSP_SWAP(&Rsp);
#if 0
		logPrint(LOG_DEBUG, "De-Activated logical channel Tslot(%d)-sub(%d)-SAPI(%d)-Dir(%d)...\n",
			Rsp.LchId.byTimeslotNb,
			Rsp.LchId.bySubChannelNb,
			Rsp.LchId.bySAPI,
			Rsp.LchId.byDirection);
#endif
        return 0;
    }

    fprintf(stderr, "ERROR: (%s)\n\n", octvc1_rc2string(ulResult));
    return ulResult;
}

int octasicBtsInit()
{
    // struct sched_param params;
    //  Create receiver thread for octasic L1 messages
    if (pthread_create(&gsmL1Receiver, NULL, l1MsgHandler, NULL))
    {
        logPrint(LOG_ERR, "GSM L1 Event Handler Thread creation failed");
        return -1;
    }
    /*
        //Get Maximum Scheduling Priority
        params.sched_priority = sched_get_priority_max(SCHED_FIFO);

        //Set the Receiver thread priority to Maximum
        if(pthread_setschedparam(gsmL1Receiver, SCHED_FIFO, &params))
        {
            return -1;
        }
    */
    return 0;
}

int gsmBtsInit()
{
    tOCT_UINT32 ulResult;

    /* Init app global */
    memset(&g_AppCtxGsm, 0, sizeof(tOCTSAMPLES_APP_CTX_GSM));
    g_AppCtxGsm.ulTimeIndEventCount = 100;
    g_AppCtxGsm.TrxId.byTrxId = 0xFF;
    g_AppCtxGsm.PktApiInfo.pPktApiSess = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiCnct = NULL;
    /*
     * Perform initialization tasks required by the application.
     */
    ulResult = InitGsmIdCatcherApplication();
    if (cOCTVC1_RC_OK != ulResult)
    {
        logPrint(LOG_ERR, "Error: InitGsmIdCatcherApplication() failed, %s\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /*
        ulResult = PrintDeviceInfo();
        if (cOCTVC1_RC_OK != ulResult)
        {
            logPrint(LOG_ERR, "Error: PrintDeviceInfo() failed, %s\n\n", octvc1_rc2string(ulResult));
            goto ErrorHandling;
        }
    */

    // Close all TRX if open
    ulResult = TrxCloseAll();
    if (cOCTVC1_RC_OK != ulResult)
    {
        logPrint(LOG_ERR, "Error: TrxCloseAll() failed, %s\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    // Open TRX
    ulResult = TrxOpen();
    if (cOCTVC1_RC_OK != ulResult)
    {
        logPrint(LOG_ERR, "Error: TrxOpen() failed, %s\n\n", octvc1_rc2string(ulResult));
        goto ErrorHandling;
    }

    /* Physical channel connect */
    {
        tOCT_UINT32 i, ulTimeSlotCount = 8;
        tOCTVC1_GSM_PHYSICAL_CHANNEL_ID PchId;
        tOCTVC1_GSM_PAYLOAD_TYPE_ENUM ulPayloadType;
        tOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM ulChannelType[8];

        for (i = 0; i < 8; i++)
        {
            if (trx_g->ts[i].lchan == SDCCH8_SACCHC8)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_SDCCH8_SACCHC8;
            else if (trx_g->ts[i].lchan == TCHF_FACCHF_SACCHTF)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_TCHF_FACCHF_SACCHTF;
            else if (trx_g->ts[i].lchan == TCHH_FACCHH_SACCHTH)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_TCHH_FACCHH_SACCHTH;
            else if (trx_g->ts[i].lchan == FCCH_SCH_BCCH_CCCH)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_FCCH_SCH_BCCH_CCCH;
            else if (trx_g->ts[i].lchan == FCCH_SCH_BCCH_CCCH_SDCCH4_SACCHC4)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_FCCH_SCH_BCCH_CCCH_SDCCH4_SACCHC4;
#ifdef FEA_GPRS
            else if (trx_g->ts[i].lchan == PDTCHF_PACCHF_PTCCHF)
                ulChannelType[i] = cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_PDTCHF_PACCHF_PTCCHF;
#endif
        }

        for (i = 0; i < ulTimeSlotCount; i++)
        {
            PchId.byTimeslotNb = (tOCTVC1_GSM_ID_TIMESLOT_NB_ENUM)i;
            if ((ulChannelType[i] == cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_TCHF_FACCHF_SACCHTF) ||
                (ulChannelType[i] == cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_PDTCHF_PACCHF_PTCCHF))
                ulPayloadType = cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_FULL_RATE;
            else if (ulChannelType[i] == cOCTVC1_GSM_LOGICAL_CHANNEL_COMBINATION_ENUM_TCHH_FACCHH_SACCHTH)
                ulPayloadType = cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_HALF_RATE;
            else
                ulPayloadType = cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE;

            ulResult = PhysicalChanActivate(PchId, ulChannelType[i], ulPayloadType);
            if (cOCTVC1_RC_OK != ulResult)
            {
                logPrint(LOG_ERR, "Error: PhysicalChanActivate() failed, %s\n\n", octvc1_rc2string(ulResult));
                goto ErrorHandling;
            }
        }

        tOCTVC1_GSM_LOGICAL_CHANNEL_ID LchId;
        tOCTVC1_GSM_SAPI_ENUM aTxSapi[] = {
            cOCTVC1_GSM_SAPI_ENUM_FCCH,
            cOCTVC1_GSM_SAPI_ENUM_SCH,
            cOCTVC1_GSM_SAPI_ENUM_BCCH,
            cOCTVC1_GSM_SAPI_ENUM_PCH_AGCH};

        tOCTVC1_GSM_SAPI_ENUM aRxSapi[] = {
            cOCTVC1_GSM_SAPI_ENUM_RACH};

        /* Activate CCCH logical channel */
        for (i = 0; i < ulTimeSlotCount; i++)
        {
            if (trx_g->ts[i].lchan == FCCH_SCH_BCCH_CCCH ||
                trx_g->ts[i].lchan == FCCH_SCH_BCCH_CCCH_SDCCH4_SACCHC4)
            {
                PchId.byTimeslotNb = (tOCTVC1_GSM_ID_TIMESLOT_NB_ENUM)i;

                tOCT_UINT32 j;

                /* Activate Tx logical channel */
                for (j = 0; j < sizeof(aTxSapi); j++)
                {
                    LchId.byTimeslotNb = PchId.byTimeslotNb;
                    LchId.bySubChannelNb = cOCTVC1_GSM_ID_SUB_CHANNEL_NB_ENUM_ALL;
                    LchId.byDirection = cOCTVC1_GSM_DIRECTION_ENUM_TX_BTS_MS;
                    LchId.bySAPI = aTxSapi[j];

                    if (cOCTVC1_RC_OK != LogicalChanActivate(LchId))
                    {
                        logPrint(LOG_ERR, "Error: Tx Logical Channel Activation Failed for CCCH!\n");
                        goto ErrorHandling;
                    }
                }

                /* Activate Rx logical channel */
                for (j = 0; j < sizeof(aRxSapi); j++)
                {
                    LchId.byTimeslotNb = PchId.byTimeslotNb;
                    LchId.bySubChannelNb = cOCTVC1_GSM_ID_SUB_CHANNEL_NB_ENUM_ALL;
                    LchId.byDirection = cOCTVC1_GSM_DIRECTION_ENUM_RX_BTS_MS;
                    LchId.bySAPI = aRxSapi[j];

                    if (cOCTVC1_RC_OK != LogicalChanActivate(LchId))
                    {
                        logPrint(LOG_ERR, "Error: Rx Logical Channel Activation Failed for CCCH!\n");
                        goto ErrorHandling;
                    }
                }
            }
        }
        logPrint(LOG_DEBUG, "All Logical Channels are Activated for the CCCH  \n");
    }

    return 0;

ErrorHandling:
    /* Close all if did open */
    if (g_AppCtxGsm.TrxId.byTrxId != 0xFF)
        TrxCloseAll();
    logPrint(LOG_ERR, "octasicBtsInit failed closing everything \n");
    ExitGsmIdCatcherApplication();

    return 1;
}

int octasicBtsStop()
{
    while (1)
    {
        if (pthread_tryjoin_np(gsmL1Receiver, NULL))
        {
            // logPrint(LOG_ERR , "GSM L1 Receiver Thread join failed");
        }
        else
            break;
    }

    gsmGate = FALSE;
    if (0 != TrxCloseAll())
    {
        return (FXL_FAILURE);
    }

    if (0 != EnableEventReception(cOCT_FALSE))
    {
        return (FXL_FAILURE);
    }

    /*
     * Free any resources used by the application.
     */
    ExitGsmIdCatcherApplication();

#if 0
    if(pthread_cancel(gsmL1Receiver))
    {
        logPrint(LOG_ERR , "L1 Event Handler Thread cancellation failed");
    }
    else
    {
    	pthread_detach(gsmL1Receiver);
    }
#endif

    memset(&gsmL1Receiver, 0, sizeof(gsmL1Receiver));
    memset(&g_AppCfg, 0, sizeof(g_AppCfg));
    memset(&g_AppCtxGsm, 0, sizeof(g_AppCtxGsm));
    g_AppCtxGsm.PktApiInfo.pPktApiSess = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiInst = NULL;
    g_AppCtxGsm.PktApiInfo.pPktApiCnct = NULL;
    return (FXL_SUCCESS);
}
unsigned char gsmDataBuffer[MAX_SIZE_OF_GSM_L1_PKT_BUFFER];
void buildSendLogicalChannelDataCmd(tOCT_UINT8 *msg, tOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MDA *rtsInd, tOCTVC1_GSM_PAYLOAD_TYPE_ENUM payloadType)
{
    tOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA *logicalChnCmd;
    tOCTVC1_PKT_API_DATA_SEND_PARMS DataSendParms;
    tOCT_UINT32 ulResult = 0xFFFFFFFF;

    logicalChnCmd = (tOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA *)&gsmDataBuffer[0];
    memset(&gsmDataBuffer[0], 0, MAX_SIZE_OF_GSM_L1_PKT_BUFFER);

    mOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA_DEF(logicalChnCmd);

    logicalChnCmd->ModuleData.hLogicalObj = g_AppCtxGsm.hGsm;

    logicalChnCmd->TrxId = rtsInd->TrxId; // g_AppCtxGsm.TrxId;
    logicalChnCmd->LchId = rtsInd->LchId;
    logicalChnCmd->Data.ulFrameNumber = rtsInd->ulFrameNumber;

    if (logicalChnCmd->LchId.bySAPI == cOCTVC1_GSM_SAPI_ENUM_SCH)
    {
        logicalChnCmd->Data.ulDataLength = 4;
    }
    else if (payloadType == cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_FULL_RATE)
    {
        logicalChnCmd->Data.ulDataLength = 33;
    }
    else if (payloadType == cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_HALF_RATE)
    {
        logicalChnCmd->Data.ulDataLength = 14;
    }
    else
    {
        logicalChnCmd->Data.ulDataLength = GSM_MACBLOCK_LEN;
    }
    logicalChnCmd->Data.ulPayloadType = payloadType;

    logicalChnCmd->ModuleData.ulModuleDataSize = sizeof(tOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA) + logicalChnCmd->Data.ulDataLength;
    // Copy Data Request Payload
    memcpy((&gsmDataBuffer[0] + sizeof(tOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA)), msg, logicalChnCmd->Data.ulDataLength);

    /* SWAP logicalChnCmd ... if have to */
    mOCTVC1_GSM_MSG_TRX_DATA_REQUEST_LOGICAL_CHANNEL_MDA_SWAP(logicalChnCmd);

    /*
     * Execute the command.
     */
    mOCTVC1_PKT_API_DATA_SEND_PARMS_DEF(&DataSendParms);
    DataSendParms.ulFormat = cOCTVOCNET_PKT_FORMAT_MODULE_DATA;
    DataSendParms.ulLength = mOCTVC1_SWAP32_IF_LE(logicalChnCmd->ModuleData.ulModuleDataSize);
    DataSendParms.pData = logicalChnCmd;

    ulResult = OctVc1PktApiCnctDataSend(g_AppCtxGsm.PktApiInfo.pPktApiCnct, &DataSendParms);
    if (cOCTVC1_RC_OK == ulResult)
    {
        return;
    }
    else
    {
        logPrint(LOG_ERR, "buildSendLogicalChannelDataCmd failed (%s) for SAPI = %d\n", octvc1_rc2string(ulResult), rtsInd->LchId.bySAPI);
        return;
    }
}

// L1 Messages Receiver thread
void *l1MsgHandler(void *nothing)
{
    int ret = 0;
    tOCTVC1_PKT_API_INST_RECV_PARMS RecvParms;
    tOCT_UINT32 ulResult = 0xFFFF;
    if (gsmBtsInit() == -1)
    {
        exit(0);
    }
    gsmGate = TRUE;
    while (1)
    {
        if (exitGsmAppThreads)
        {
            /* Exit Data Indication Thread */
            pthread_mutex_lock(&mutexdataInd);
            dataIndRcvdstate = STATE_DATAIND_RECEIVED;
            pthread_mutex_unlock(&mutexdataInd);
            pthread_cond_signal(&conddataInd);

            while (1)
            {
                if (pthread_tryjoin_np(gsmDataIndThread, NULL))
                {
                    logPrint(LOG_ERR, "ERROR : GSM Data Indication Thread Join Failed\n");
                }
                else
                    break;
            }

#if 0
            /* Exit Timer Tick Thread */
            pthread_mutex_lock(&mutexTimerTickInd);
            timerTickReceived = TRUE;
            pthread_mutex_unlock(&mutexTimerTickInd);
            pthread_cond_signal(&condTimerTickInd);
            if(pthread_tryjoin_np(gsmTimerTickThread, NULL))
            {
              logPrint(LOG_ERR, "ERROR : GSM Timer Tick Thread Join Failed");
            }
              logPrint(LOG_ERR, "GSM TimeTicker Data Indication Thread killed");
#endif

#ifdef FEA_GPRS
            /* Exit Llc Indication Thread */
            pthread_mutex_lock(&mutexLlcInd);
            llcIndRcvdstate = STATE_DATAIND_RECEIVED;
            pthread_mutex_unlock(&mutexLlcInd);
            pthread_cond_signal(&condLlcInd);
            if (pthread_join(llcDataIndThread, NULL))
            {
                logPrint(LOG_ERR, "ERROR : LLC Data Indication Thread Join Failed");
            }
            logPrint(LOG_ERR, "LLC Data Indication Thread killed");

            /* Exit Rlc Indication Thread */
            pthread_mutex_lock(&mutexRlcInd);
            rlcIndRcvdstate = STATE_DATAIND_RECEIVED;
            pthread_mutex_unlock(&mutexRlcInd);
            pthread_cond_signal(&condRlcInd);
            if (pthread_join(rlcDataIndThread, NULL))
            {
                logPrint(LOG_ERR, "ERROR : RLC Data Indication Thread Join Failed\n");
            }
            logPrint(LOG_ERR, "RLC Data Indication Thread killed");

#endif
            /* Exit GSM L1 Reciever Thread */
            pthread_exit(NULL);
        }

        mOCTVC1_PKT_API_INST_RECV_PARMS_DEF(&RecvParms);
        RecvParms.ulTimeoutMs = cOCTVC1_PKT_API_FOREVER;
        RecvParms.pPayload = l1PacketBuffer;
        RecvParms.ulMaxPayloadLength = MAX_SIZE_OF_GSM_L1_PKT_BUFFER;

        if ((g_AppCtxGsm.PktApiInfo.pPktApiInst != NULL) && (gsmGate == TRUE) && (0 == exitGsmAppThreads))
        {
            ulResult = OctVc1PktApiInstRecv(g_AppCtxGsm.PktApiInfo.pPktApiInst, &RecvParms);
        }
        else
        {
            continue;
        }

        if (ulResult == cOCTVC1_RC_OK)
        {

            if (RecvParms.RcvPktType == cOCTVC1_PKT_API_PKT_TYPE_ENUM_DATA)
            {
                // RcvParms.Info.Data.ulPktFormat
                if (RecvParms.Info.Data.ulPktFormat == cOCTVOCNET_PKT_FORMAT_MODULE_DATA)
                {
                    tOCT_UINT32 ulModuleDataId = 0;
                    tOCTVC1_MODULE_DATA *pModuleData = (tOCTVC1_MODULE_DATA *)RecvParms.pPayload;
                    ulModuleDataId = mOCTVC1_SWAP32_IF_LE(pModuleData->ulModuleDataId);
                    // logPrint( LOG_INFO, "Module Id = %d RcvPktType = %d , ulPktFormat = %d\n", ulModuleDataId , RecvParms.RcvPktType , RecvParms.Info.Data.ulPktFormat);

                    switch (ulModuleDataId)
                    {
                    case cOCTVC1_GSM_MSG_TRX_STATUS_CHANGE_MID:
                    {
                        tOCTVC1_GSM_MSG_TRX_STATUS_CHANGE_MDA *pEvt = (tOCTVC1_GSM_MSG_TRX_STATUS_CHANGE_MDA *)l1PacketBuffer;

                        mOCTVC1_GSM_MSG_TRX_STATUS_CHANGE_MDA_SWAP(pEvt);
                        if (pEvt->ulStatus == cOCTVC1_GSM_TRX_STATUS_ENUM_RADIO_READY)
                        {
                            logPrint(LOG_INFO, "GSM_TRX_STATUS_ENUM_RADIO_READY");
                            logPrint(LOG_DEBUG, "\n\n============================================================================\n");
                            logPrint(LOG_DEBUG, "                       * GSM IDCATCHER IS UP AND RUNNING *\n");
                            logPrint(LOG_DEBUG, "============================================================================\n\n");
#ifdef FEA_GPRS
                            for (i = 0; i < 8; i++)
                            {
                                if (trx_g->ts[i].lchan == PDTCHF_PACCHF_PTCCHF)
                                {
                                    logPrint(LOG_DEBUG, "Calling activateLogicalChannel from phRachIndHandler TxSlotNum = %d", i);
                                    activateLogicalChannel(i, 0xf1);
                                }
                            }
#endif

#ifdef FEA_PICSIX_CHANGE
                            /* Send system status indication */
                            sendSystemStatusIndication(FXL_2G_IDC, FXL_STARTED);
#endif
                        }
                        else
                        {
                            /* Error on TRX */
                            logPrint(LOG_ERR, "Radio Error!!!");
                        }
                    }
                    break;

                    case cOCTVC1_GSM_MSG_TRX_TIME_INDICATION_MID:
                    {
                        tOCTVC1_GSM_MSG_TRX_TIME_INDICATION_MDA *timeInd = (tOCTVC1_GSM_MSG_TRX_TIME_INDICATION_MDA *)l1PacketBuffer;

                        mOCTVC1_GSM_MSG_TRX_TIME_INDICATION_MDA_SWAP(timeInd);

                        timeIndHandler(timeInd);

                        if (checkForInactive2gBLUsers == TRUE)
                        {
                            deleteInactiveBlacklistUsers();

                            pthread_mutex_lock(&mutexBlacklistUserQ);
                            checkForInactive2gBLUsers = FALSE;
                            pthread_mutex_unlock(&mutexBlacklistUserQ);
                        }
                    }
                    break;

                    case cOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MID:
                    {
                        tOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MDA *rtsInd = (tOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MDA *)l1PacketBuffer;

                        mOCTVC1_GSM_MSG_TRX_READY_TO_SEND_INDICATION_LOGICAL_CHANNEL_MDA_SWAP(rtsInd);

                        listNode *qNode = NULL;
                        tOCTVC1_GSM_SAPI_ENUM sapi = (tOCTVC1_GSM_SAPI_ENUM)rtsInd->LchId.bySAPI;
                        tOCT_UINT32 fn = rtsInd->ulFrameNumber;
                        updateRtstime(rts_time_g, fn);

                        if (sapi == cOCTVC1_GSM_SAPI_ENUM_PCH_AGCH)
                        {
                            if ((headForChanAllocationQ != NULL) && ((qNode = popMsgFromList(&headForChanAllocationQ)) != NULL))
                            {
                                memcpy((tOCT_UINT8 *)l1MsgSendBuffer, (tOCT_UINT8 *)qNode->msg, GSM_MACBLOCK_LEN);

                                buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);

                                headForChanAllocationQ = qNode->next; // Update head pointer to next
                                if (headForChanAllocationQ == NULL)
                                {
                                    tailForChanAllocationQ = NULL;
                                }
                                free(qNode);
                                qNode = NULL;
                            }
                            else if (FN2CCCHBLOCK(fn) >= trx_g->bts.blockReservedForAgch)
                            {
                                pthread_mutex_lock(&mutexPagingQ);

                                tOCT_UINT8 pagingMultiFrame = ((fn / 51) % trx_g->bts.pagingPeriodicity);
                                tOCT_UINT8 pagingBlockIndex = FN2CCCHBLOCK(fn) - trx_g->bts.blockReservedForAgch;
                                if ((headForPagingReqQ[pagingMultiFrame][pagingBlockIndex] != NULL) && ((qNode = popMsgFromList(&headForPagingReqQ[pagingMultiFrame][pagingBlockIndex])) != NULL))
                                {
                                    /* Real paging with IMSI */

                                    headForPagingReqQ[pagingMultiFrame][pagingBlockIndex] = qNode->next; // Update head pointer to next
                                    if (headForPagingReqQ[pagingMultiFrame][pagingBlockIndex] == NULL)
                                    {
                                        tailForPagingReqQ[pagingMultiFrame][pagingBlockIndex] = NULL;
                                    }
                                    memcpy((tOCT_UINT8 *)l1MsgSendBuffer, (tOCT_UINT8 *)qNode->msg, GSM_MACBLOCK_LEN);
                                    buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);

                                    /* Update the Paging attempted counter */
                                    blackListedUserInfo *blackUserInfo = NULL;

                                    tOCT_UINT8 *identity = (tOCT_UINT8 *)(&qNode->msg[4]);

                                    blackUserInfo = blacklistUserFindUsingIdentity(identity, TRUE, FALSE);

                                    if (blackUserInfo != NULL)
                                    {

                                        logPrint(LOG_DEBUG, "Paging is sent on P-MF = %d, PB = %d for Blacklist User = %p, IMSI = %s due to Trigger(0x%x)",
                                                 blackUserInfo->pagingMultiFrame, blackUserInfo->pagingBlockIndex,
                                                 blackUserInfo, blackUserInfo->userImsi_str, blackUserInfo->bitMaskForTriggers);
#ifdef FEA_BL_VAL_CHK_BY_PERIODIC_PAGING
                                        blackUserInfo->pagingInactivityCounter++;
#endif
                                        /* Starting periodic timer for paging retransmission here */
                                        tOCT_UINT8 retVal = timerStart(&blackUserInfo->tmr, TIMER_FOR_PAGE_RETRANSMIT, TIMER_FOR_PAGE_RETRANSMIT, &blacklistUserInfoTimerCbFunction, blackUserInfo);
                                        assert(retVal == RC_TCB_OK);
                                    }

                                    free(qNode);
                                    qNode = NULL;
                                }
                                else
                                {
                                    /* Dummy Paging to keep UE connected with our BTS */
                                    memcpy((tOCT_UINT8 *)l1MsgSendBuffer, (tOCT_UINT8 *)dummyPagingReq, GSM_MACBLOCK_LEN);
                                    buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);
                                }
                                pthread_mutex_unlock(&mutexPagingQ);
                            }
                            else
                            {
                                // Nothing to be sent, send fill frame
                                memcpy((tOCT_UINT8 *)l1MsgSendBuffer, (tOCT_UINT8 *)fillFrame, GSM_MACBLOCK_LEN);
                                buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);
                            }
                        }
                        else if (sapi == cOCTVC1_GSM_SAPI_ENUM_SCH)
                        {
                            tOCT_UINT32 t3prime = 0;
                            gsm_bts *bts = &trx_g->bts;

                            // compute T3prime
                            t3prime = (rts_time_g->t3 - 1) / 10;

                            // fill SCH burst with data
                            l1MsgSendBuffer[0] = (bts->bsic << 2) | (rts_time_g->t1 >> 9);
                            l1MsgSendBuffer[1] = (rts_time_g->t1 >> 1);
                            l1MsgSendBuffer[2] = (rts_time_g->t1 << 7) | (rts_time_g->t2 << 2) | (t3prime >> 1);
                            l1MsgSendBuffer[3] = (t3prime & 1);

                            buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);
                            // logPrint(LOG_DEBUG , "\n Synchronization Info send\n");
                        }
                        else if (sapi == cOCTVC1_GSM_SAPI_ENUM_BCCH)
                        {
                            gsm_bts *bts = &trx_g->bts;
                            // get SYS_INFO messages from bts->si_buf[]
                            get_sysinfo(bts, rts_time_g, l1MsgSendBuffer);
                            buildSendLogicalChannelDataCmd(l1MsgSendBuffer, rtsInd, cOCTVC1_GSM_PAYLOAD_TYPE_ENUM_NONE);
                        }
                        else // SDCCH , SACCH , TCH , or any other
                        {
                            readyToSendIndHandler(rtsInd);
                        }
                    }
                    break;

                    case cOCTVC1_GSM_MSG_TRX_RACH_INDICATION_LOGICAL_CHANNEL_MID:
                    {
                        tOCTVC1_GSM_MSG_TRX_RACH_INDICATION_LOGICAL_CHANNEL_MDA *rachInd = (tOCTVC1_GSM_MSG_TRX_RACH_INDICATION_LOGICAL_CHANNEL_MDA *)l1PacketBuffer;

                        mOCTVC1_GSM_MSG_TRX_RACH_INDICATION_LOGICAL_CHANNEL_MDA_SWAP(rachInd);

                        phRachIndHandler(rachInd);
                    }
                    break;

                    case cOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MID:
                    {
                        tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA *dataIndEvt = (tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA *)l1PacketBuffer;

                        mOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA_SWAP(dataIndEvt);

                        tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA *incomingMsg = getDataIndBuffer();
                        tOCT_UINT8 sapi = dataIndEvt->LchId.bySAPI;
                        tOCT_UINT8 *evtDataContent = (tOCT_UINT8 *)((tOCT_UINT8 *)dataIndEvt + sizeof(tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA));

                        // Dropping fill frame here
#if !defined(FEA_SILENT_CALL) && !defined(FEA_VOICE_CALL_TO_TARGET)
                        if ((sapi == cOCTVC1_GSM_SAPI_ENUM_SDCCH) && (memcmp(fillFrameDrop, evtDataContent, 3) != 0))
#else
                        if (((sapi == cOCTVC1_GSM_SAPI_ENUM_SACCH) || (sapi == cOCTVC1_GSM_SAPI_ENUM_SDCCH) || (sapi == cOCTVC1_GSM_SAPI_ENUM_FACCHF) || (sapi == cOCTVC1_GSM_SAPI_ENUM_FACCHH) || (sapi == cOCTVC1_GSM_SAPI_ENUM_TCHH) || (sapi == cOCTVC1_GSM_SAPI_ENUM_TCHF)) && (memcmp(fillFrameDrop, evtDataContent, 3) != 0))
#endif
                        {
                            memcpy(incomingMsg, dataIndEvt, sizeof(tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA) + dataIndEvt->Data.ulDataLength);

                            pthread_mutex_lock(&mutexdataInd);
                            {
                                enQueue(&dataIndQ, (void *)incomingMsg);
                                dataIndRcvdstate = STATE_DATAIND_RECEIVED;
                            }
                            pthread_mutex_unlock(&mutexdataInd);
                            pthread_cond_signal(&conddataInd);
                        }
#ifdef FEA_GPRS
                        else if ((sapi == cOCTVC1_GSM_SAPI_ENUM_PDTCH) && (memcmp(fillFrameDrop, dataIndEvt->Data.abyDataContent, 3) != 0) &&
                                 (isTimeSlotTypeSupportsPDTCH(dataIndEvt->LchId.byTimeslotNb) == TRUE))
                        {
                            // RLC/MAC processing
                            memcpy(incomingMsg, dataIndEvt, sizeof(tOCTVC1_GSM_MSG_TRX_DATA_INDICATION_LOGICAL_CHANNEL_MDA) + dataIndEvt->Data.ulDataLength);

                            pthread_mutex_lock(&mutexRlcInd);
                            {
                                enQueue(&dataIndQ, (void *)incomingMsg);
                                rlcIndRcvdstate = STATE_DATAIND_RECEIVED;
                            }
                            pthread_mutex_unlock(&mutexRlcInd);
                            pthread_cond_signal(&condRlcInd);
                        }
#endif
                    }
                    break;

                    case cOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATUS_CHANGE_EID:
                    {
                        tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATUS_CHANGE_EVT *pEvt = (tOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATUS_CHANGE_EVT *)l1PacketBuffer;

                        mOCTVC1_HW_MSG_CLOCK_SYNC_MGR_STATUS_CHANGE_EVT_SWAP(pEvt);

                        logPrint(LOG_INFO, "Received HW_MSG_CLOCK_SYNC_MGR_STATUS_CHANGE_EID event:NewState(%s) - PreviousState(%s)\n",
                                 GetClockSyncMgrStatusStateStr(pEvt->ulState),
                                 GetClockSyncMgrStatusStateStr(pEvt->ulPreviousState));
                    }
                    break;

                    default:
                    {
                        logPrint(LOG_WARNING, "Warning: Ignoring Unknown event id 0x%x from L1\n",
                                 RecvParms.Info.Evt.ulEvtId);
                    }
                    break;
                    }
                }
            }
        }
    }
    return (void *)ret;
}

#ifdef FXL_3G_TX_JAMMER
/*
 * Global Variable for Jammer
 */
jammerPscInfo jammer3gPsc;

/*
 * Extern
 */
extern int exitUmtsTxJammer;
extern void buildAndSend3gTxJammerStatusInd(fxL3gTxJammerStatus status);
extern fxLResult stop3gIdCatcherThread(void);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmDisplayStatus3g

Description:    Display the 3G Cells

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmDisplayStatus3g(tOCTVC1_RUS_WNM_SEARCH_STATS *f_pStatus)
{
    printf("- FINDING 3G CELLS ----------------------------------------\n");
    printf("  Monitor status              : %s\n", WnmGetStateString(f_pStatus->ulState));
    printf("  Number of 3G cell found     : %d\n", f_pStatus->ul3GBaseStationCnt);
    printf("  Number of Rssi Band scanned : %d\n", f_pStatus->ulRssiBandCnt);
    printf("  Process current entry index : %d\n", f_pStatus->ulCurrentEntryIndex);
    printf("  Number of band to scan      : %d / %d\n", f_pStatus->ulNumBandScanned, f_pStatus->ulNumBandToScan);
    printf("   + Waveform                 : %s\n", WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm));
    printf("   + Band                     : %s\n", WnmGetBandString(f_pStatus->CurrentEntry.ulWaveForm, f_pStatus->CurrentEntry.ulBand));
    printf("   + Uarfcn                   : %u\n", f_pStatus->ulCurrentUarfcn);
    printf("   + Rssi threshold           : %6.3f dBm \n", f_pStatus->CurrentEntry.lRssiThreshold / 512.0);
    printf("   + Rsp threshold            : %6.3f dBm \n", f_pStatus->CurrentEntry.lRSPThresholddBm / 512.0);
    printf("   + SQ threshold             : %6.3f dBm \n", f_pStatus->CurrentEntry.lSQThresholddB / 512.0);
    printf("   + Receiver Gain mode       : %s \n", WnmGetRxGainModeString(f_pStatus->ulRxGainControlMode));
    printf("     + Manual gain            : %6.3f dB \n", f_pStatus->ulRxGainControlMode);
    printf("-----------------------------------------------------------\n");
    return (0);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       find3gCell

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int find3gCell(void)
{
    unsigned i;
    tOCTVC1_RUS_WNM_UMTS_CELL_STATS UmtsCellStats;
    tOCTVC1_OBJECT_CURSOR_ENUM ulGetMode;
    unsigned bEnableExaustiveBoardInfo = 0;
    unsigned SystemClockWaitTimeMs = 5 * (4 * 250); // MSec
    tOCT_UINT32 gPrevCellCount3g = 0;

    // Wait for the system clock is locked
    logPrint(LOG_INFO, "3G Tx Jammer : Wait for stable system clock: ...\n");
    while ((SystemClockWaitTimeMs -= 250) != 0)
    {
        if (CheckClkMgrStatus() == 0)
            break;
        OctOsalSleepMs(250);
    }
    logPrint(LOG_INFO, "3G Tx Jammer : Status of the system clock is: %s\n", (SystemClockWaitTimeMs == 0) ? "NOT LOCKED (WARNING)!" : "LOCKED");

    // Verify a Wnm application is already openned
    if (CheckWnm() != 0)
    {
        logPrint(LOG_ERR, "3G Tx Jammer : CheckWnm Failed\n");
        goto CriticalAbort;
    }

    // Open Wireless Networl Monitor
    if (WnmOpen(&tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "3G Tx Jammer : WnmOpen Failed\n");
        goto CriticalAbort;
    }
    OctOsalSleepMs(100);

    // Setup the monitor
    tWnm.PrmSetup.ulSearchMode = cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE; // Performs BCH information decoding

ContinuousMonitoring: // continuous scan
    logPrint(LOG_ERR, "ContinuousMonitoring level\n");
    if (WnmSetup(tWnm.hWnm, &tWnm.PrmSetup) != 0)
    {
        logPrint(LOG_ERR, "3G Tx Jammer : WnmSetup Failed\n");
        goto CriticalAbort;
    }

    // Launch the monitoring
    if (WnmStart(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "3G Tx Jammer : WnmStart Failed\n");
        goto CriticalAbort;
    }

    // Send 3G Tx Jammer Status as finding cell
    buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_SCANNING_CELL);

    // Monitoring
    do
    {
        if (exitUmtsTxJammer)
        {
            logPrint(LOG_ERR, "MonitorAbortError line = %d\n", __LINE__);
            goto MonitorAbortError;
        }

        if (WnmGetStatus(tWnm.hWnm, &tWnm.tStatus) != 0)
            goto CriticalAbort;

        WnmDisplayStatus3g(&tWnm.tStatus);

        if (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_ABORT_ERROR)
        {
            logPrint(LOG_ERR, "MonitorAbortError line = %d\n", __LINE__);
            goto MonitorAbortError;
        }

        /* Cell Found */
        if ((tWnm.tStatus.ul3GBaseStationCnt - gPrevCellCount3g) > 0)
        {
            /* Assign the number of PSC found */
            jammer3gPsc.numOfPsc = tWnm.tStatus.ul3GBaseStationCnt;

            if (WnmUmtsCellGetList(tWnm.hWnm, &tWnm.UmtsBtsList) != 0)
                goto CriticalAbort;

            for (i = 0; i < (tWnm.tStatus.ul3GBaseStationCnt - gPrevCellCount3g); i++)
            {
                tOCTVC1_SUB_OBJECT_ID CellId = tWnm.UmtsBtsList.aCellId[gPrevCellCount3g + i];

                // Retreive the stats
                if (WnmUmtsCellStatsGet(tWnm.hWnm, CellId, &UmtsCellStats) != 0)
                    goto CriticalAbort;

                /* Fill the PSC info */
                if (i < MAX_NUM_OF_PSC_PER_UARFCN)
                {
                    jammer3gPsc.psc[i] = UmtsCellStats.usPsc;
                    jammer3gPsc.rscp[i] = ((double)UmtsCellStats.lRscpdBm) / 512.0;
                }
            }
        }
        OctOsalSleepMs(250);
    } while (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_SEARCHING);

    // Echo if nothing is found
    if ((tWnm.GsmBtsList.ulNumCell == 0) &&
        (tWnm.UmtsBtsList.ulNumCell == 0) &&
        (tWnm.LteBtsList.ulNumCell == 0))
    {
        logPrint(LOG_INFO, "3G Tx Jammer : No cells found\n");
    }

    // Stop monitoring
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "Dev team support Calling critical abort = %d \n", __LINE__);
        goto CriticalAbort;
    }

    if (--bContinuousMonitoringEnable)
    {
        logPrint(LOG_ERR, "Dev team support find3gCell UMTS scan bContinuousMonitoringEnable = %d , lineno = %d\n", bContinuousMonitoringEnable, __LINE__);
        goto ContinuousMonitoring;
    }

    // Close the application
    if (WnmClose(tWnm.hWnm) != 0)
        goto CriticalAbort;

    memset(&tWnm, 0, sizeof(tWnm));

    return (0);

MonitorAbortError:
    // Stop monitoring
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "Dev team support Calling critical abort = %d \n", __LINE__);
        goto CriticalAbort;
    }

    if (--bContinuousMonitoringEnable)
    {
        logPrint(LOG_ERR, "Dev team support find3gCell UMTS scan bContinuousMonitoringEnable = %d , lineno = %d\n", bContinuousMonitoringEnable, __LINE__);
        goto ContinuousMonitoring;
    }

    // Close the application
    if (WnmClose(tWnm.hWnm) != 0)
        goto CriticalAbort;

    memset(&tWnm, 0, sizeof(tWnm));

    return (-1);

CriticalAbort:
    WnmClose(tWnm.hWnm);

    memset(&tWnm, 0, sizeof(tWnm));

    return (-1);
}

/*
 * Get PSC with highest RSCP
 */
unsigned int getPscWithHighestRscp(void)
{
    unsigned int pscCount = 0;
    unsigned int pscIdx = 0;
    unsigned int result = 0;
    float rscp = jammer3gPsc.rscp[0];

    for (pscCount = 0; pscCount < jammer3gPsc.numOfPsc; pscCount++)
    {
        if (rscp < jammer3gPsc.rscp[pscCount])
        {
            rscp = jammer3gPsc.rscp[pscCount];
            pscIdx = pscCount;
        }
    }

    result = jammer3gPsc.psc[pscIdx];

    return (result);
}

/*
 * Main method for 3G Tx Jammer
 */
void *umtsTxJammer(void *pJammerParams)
{
    int i;
    unsigned int umtsThreadCreated = 0;

    /* Initialize PSC info */
    memset((jammerPscInfo *)&jammer3gPsc, 0, sizeof(jammerPscInfo));

    umtsJammerParams jammerParams = *((umtsJammerParams *)pJammerParams);

    unsigned int uarfcn = jammerParams.uarfcn;
    fxL3gBandType band = jammerParams.band;

    tWnm.PrmSetup.ulEntryCount = 1;
    bContinuousMonitoringEnable = 1;
    tWnm.PrmSetup.aEntryList[0].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_UMTS;
    tWnm.PrmSetup.aEntryList[0].ulBand = convertToOct3gBandFromFxL(band);
    tWnm.PrmSetup.aEntryList[0].ulUarfcnStart = uarfcn;
    tWnm.PrmSetup.aEntryList[0].ulUarfcnStop = uarfcn;
    tWnm.PrmSetup.aEntryList[0].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold above a cell serach is performed
    tWnm.PrmSetup.aEntryList[0].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
    tWnm.PrmSetup.aEntryList[0].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
    tWnm.PrmSetup.aEntryList[0].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;

    logPrint(LOG_DEBUG, "===========================================\n");
    logPrint(LOG_DEBUG, " Application : %s\n", "3G JAMMER");
    logPrint(LOG_DEBUG, " Description : %s\n", "3G Jammer Software");
    logPrint(LOG_DEBUG, " Version     : V_%.1f\n", FXL_SOCKETINTERFACEAPI_VERSION);
    logPrint(LOG_DEBUG, "===========================================\n\n\n");

    /*
     * Perform initialization tasks required by the application.
     */
    if (0 != InitApplication())
    {
        logPrint(LOG_ERR, "3G Tx Jammer : Initialization of Application is failed\n");
        buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_INITIALIZATION_FAILED);
        goto CriticalAbort;
    }

    /*
     * Find the 3G Cells to block
     */
    if (0 != find3gCell())
    {
        if (0 == exitUmtsTxJammer)
        {
            logPrint(LOG_ERR, "3G Tx Jammer : Finding 3G Cell failed\n");
            buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_SCANNING_CELL_FAILED);
        }
        goto CriticalAbort;
    }

    /*
     * Check the cells found or not
     */
    if (0 == jammer3gPsc.numOfPsc)
    {
        logPrint(LOG_ERR, "3G Tx Jammer : No Cell Found\n");
        buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_NO_CELL_FOUND);
        goto CriticalAbort;
    }
    else
    {
        logPrint(LOG_INFO, "3G Tx Jammer : Number of PSC found in UARFCN[%d] = %d\n", uarfcn, jammer3gPsc.numOfPsc);
        for (i = 0; i < jammer3gPsc.numOfPsc; i++)
        {
            logPrint(LOG_INFO, "3G Tx Jammer : PSC[%d] = %d\n", i, jammer3gPsc.psc[i]);
        }
        buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_CELL_FOUND);
    }

    memset((UmtsConfigControlBlock *)&gUmtsConfig, 0, sizeof(UmtsConfigControlBlock));

    gUmtsConfig.dlUarfcn = uarfcn;
    gUmtsConfig.psc = getPscWithHighestRscp();
    gUmtsConfig.txJammerEnabled = TRUE;

    /* Start ID Catcher */
    bootL1InProgress = 1;
    gAutoConfigCfg.l1Image = FXL_L1_UMTS_NATIVE_IMAGE;
    gAutoConfigCfg.dspType = jammer3gPsc.dspType;
    autoBootL1Image();
    createAndStart3gIdCatcherThread();
    bootL1InProgress = 0;

    umtsThreadCreated = 1;
    buildAndSend3gTxJammerStatusInd(FXL_3G_TX_JAMMER_RUNNING);

    goto CriticalAbort;

CriticalAbort:
    while (1)
    {
        /*
         * Wait for stop command from External Client
         */
        if (exitUmtsTxJammer)
        {
            exitUmtsTxJammer = 0;

            /* Free the memory allocated */
            if (NULL != pJammerParams)
            {
                free(pJammerParams);
                pJammerParams = NULL;
            }

            /* Initialize PSC info */
            memset((jammerPscInfo *)&jammer3gPsc, 0, sizeof(jammerPscInfo));

            ExitApplication();

            if (1 == umtsThreadCreated)
            {
                stop3gIdCatcherThread();
            }

            pthread_exit(NULL);
        }

        // Sleep 2 sec
        OctOsalSleepMs(2000);
    }

    return NULL;
}
#endif

#ifdef FXL_4G_TX_JAMMER
int lteJamCellFound = FALSE;
int lteJamThreadsCreated = FALSE;

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       WnmDisplayStatus4g

Description:    Display the 4G Cells

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int WnmDisplayStatus4g(tOCTVC1_RUS_WNM_SEARCH_STATS *f_pStatus)
{
    printf("- FINDING 4G CELLS ----------------------------------------\n");
    printf("  Monitor status              : %s\n", WnmGetStateString(f_pStatus->ulState));
    printf("  Number of 4G cell found     : %d\n", f_pStatus->ul4GBaseStationCnt);
    printf("  Number of Rssi Band scanned : %d\n", f_pStatus->ulRssiBandCnt);
    printf("  Process current entry index : %d\n", f_pStatus->ulCurrentEntryIndex);
    printf("  Number of band to scan      : %d / %d\n", f_pStatus->ulNumBandScanned, f_pStatus->ulNumBandToScan);
    printf("   + Waveform                 : %s\n", WnmGetWaveFormString(f_pStatus->CurrentEntry.ulWaveForm));
    printf("   + Band                     : %s\n", WnmGetBandString(f_pStatus->CurrentEntry.ulWaveForm, f_pStatus->CurrentEntry.ulBand));
    printf("   + Uarfcn                   : %u\n", f_pStatus->ulCurrentUarfcn);
    printf("   + Rssi threshold           : %6.3f dBm \n", f_pStatus->CurrentEntry.lRssiThreshold / 512.0);
    printf("   + Rsp threshold            : %6.3f dBm \n", f_pStatus->CurrentEntry.lRSPThresholddBm / 512.0);
    printf("   + SQ threshold             : %6.3f dBm \n", f_pStatus->CurrentEntry.lSQThresholddB / 512.0);
    printf("   + Receiver Gain mode       : %s \n", WnmGetRxGainModeString(f_pStatus->ulRxGainControlMode));
    printf("     + Manual gain            : %6.3f dB \n", f_pStatus->ulRxGainControlMode);
    printf("-----------------------------------------------------------\n");

    return (0);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\

Function:       find4gCell

Description:

\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int find4gCell(void)
{
    unsigned int i;
    tOCTVC1_RUS_WNM_LTE_CELL_STATS LteCellStats;
    tOCTVC1_OBJECT_CURSOR_ENUM ulGetMode;
    unsigned bEnableExaustiveBoardInfo = 0;
    unsigned SystemClockWaitTimeMs = 5 * (4 * 250); // MSec
    tOCT_UINT32 gPrevCellCount4g = 0;
    char szLocalFileName[128] = {0};
    lteServingCellStruct servingCell = {0};

    // Wait for the system clock is locked
    logPrint(LOG_INFO, "4G Tx Jammer : Wait for stable system clock: ...\n");
    while ((SystemClockWaitTimeMs -= 250) != 0)
    {
        if (CheckClkMgrStatus() == 0)
            break;
        OctOsalSleepMs(250);
    }
    logPrint(LOG_INFO, "4G Tx Jammer : Status of the system clock is: %s\n", (SystemClockWaitTimeMs == 0) ? "NOT LOCKED (WARNING)!" : "LOCKED");

    // Verify a Wnm application is already openned
    if (CheckWnm() != 0)
    {
        logPrint(LOG_ERR, "4G Tx Jammer : CheckWnm Failed\n");
        goto CriticalAbort;
    }

    // Open Wireless Networl Monitor
    if (WnmOpen(&tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "4G Tx Jammer : WnmOpen Failed\n");
        goto CriticalAbort;
    }
    OctOsalSleepMs(100);

    // Setup the monitor
    tWnm.PrmSetup.ulSearchMode = cOCTVC1_RUS_WNM_SEARCH_MODE_ENUM_EXHAUSTIVE; // Performs BCH information decoding

ContinuousMonitoring: // continuous scan
    if (WnmSetup(tWnm.hWnm, &tWnm.PrmSetup) != 0)
    {
        logPrint(LOG_ERR, "4G Tx Jammer : WnmSetup Failed\n");
        goto CriticalAbort;
    }

    // Launch the monitoring
    if (WnmStart(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "4G Tx Jammer : WnmStart Failed\n");
        goto CriticalAbort;
    }

    // Send 4G Tx Jammer Status as finding cell
    buildAndSend4gTxJammerStatusInd(FXL_4G_TX_JAMMER_SCANNING_CELL);

    // Monitoring
    do
    {
        if (exitLteTxJammer)
        {
            logPrint(LOG_ERR, "MonitorAbortError line = %d\n", __LINE__);
            goto MonitorAbortError;
        }

        if (WnmGetStatus(tWnm.hWnm, &tWnm.tStatus) != 0)
            goto CriticalAbort;

        WnmDisplayStatus4g(&tWnm.tStatus);

        if (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_ABORT_ERROR)
        {
            logPrint(LOG_ERR, "MonitorAbortError line = %d\n", __LINE__);
            goto MonitorAbortError;
        }

        if ((tWnm.tStatus.ul4GBaseStationCnt - gPrevCellCount4g) > 0)
        {
            // Retrieve the list of the BTS object index
            if (WnmLteCellGetList(tWnm.hWnm, &tWnm.LteBtsList) != 0)
                goto CriticalAbort;

            /* Cell Found */
            for (i = 0; i < (tWnm.tStatus.ul4GBaseStationCnt - gPrevCellCount4g); i++)
            {
                tOCTVC1_SUB_OBJECT_ID CellId = tWnm.LteBtsList.aCellId[gPrevCellCount4g + i];

                // Retreive the stats
                if (WnmLteCellStatsGet(tWnm.hWnm, CellId, &LteCellStats) != 0)
                    goto CriticalAbort;

                WnmLteCellStatsDisplay(CellId, &LteCellStats);

                // Read the BCCH from the file
                sprintf(szLocalFileName, "%s%02d.bin", "/tmp/Lte_customer_bch", CellId);
                if (ReadFileFromTarget(szLocalFileName, LteCellStats.szBchPayloadFileName, 0) != 0)
                    goto CriticalAbort;

                // BCCH file sanity check
                if (LteCellStats.usBchPayloadBlockCnt != 0)
                {
                    servingCell.sfNsfcellId = LteCellStats.usCellIdentity;
                    servingCell.rssi = (double)(LteCellStats.lRssidBm / 512.0);
                    servingCell.rsrp = (double)(LteCellStats.lRsrpdBm / 512.0);
                    servingCell.rsrq = (double)(LteCellStats.lRsrqdB / 512.0);
                    servingCell.mib.dlBw = LteCellStats.ulBandWidth;
                    servingCell.mib.phichDuration = LteCellStats.byPhichDuration;
                    servingCell.mib.phichResource = LteCellStats.byPhichGroup;
                    servingCell.earfcn = LteCellStats.ulEarfcn;
                    servingCell.noOfTxAntennaPorts = LteCellStats.byNumTxAntennaPorts;
                    WnmLteBCHBufferAnalysis(szLocalFileName, LteCellStats.usBchPayloadBlockCnt, &servingCell);
                }
            }
            gPrevCellCount4g = tWnm.tStatus.ul4GBaseStationCnt;
        }
        WnmDisplayStatus4g(&tWnm.tStatus);
        OctOsalSleepMs(250);
    } while (tWnm.tStatus.ulState == cOCTVC1_RUS_WNM_SEARCH_STATE_ENUM_SEARCHING);

    // Echo if nothing is found
    //  logPrint(LOG_DEBUG,"tWnm.LteBtsList.ulNumCell : %d \n", tWnm.LteBtsList.ulNumCell);
    if ((tWnm.GsmBtsList.ulNumCell == 0) && (tWnm.UmtsBtsList.ulNumCell == 0) && (tWnm.LteBtsList.ulNumCell == 0))
    {
        logPrint(LOG_DEBUG, "4G Tx Jammer : No cells found\n");
    }

    if (tWnm.LteBtsList.ulNumCell != 0)
    {
        lteJamCellFound = 1; // update jammer Cell Foudn info
        logPrint(LOG_DEBUG, "4G Tx Jammer : Debug -2 : %d \n", lteJamCellFound);
    }

    // Stop monitoring
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "Dev team support Calling critical abort = %d \n", __LINE__);
        goto CriticalAbort;
    }

    if (--bContinuousMonitoringEnable)
    {
        logPrint(LOG_ERR, "Dev team support find3gCell UMTS scan bContinuousMonitoringEnable = %d, lineno = %d\n", bContinuousMonitoringEnable, __LINE__);
        goto ContinuousMonitoring;
    }

    // Close the application
    if (WnmClose(tWnm.hWnm) != 0)
        goto CriticalAbort;

    memset(&tWnm, 0, sizeof(tWnm));

    return (0);

MonitorAbortError:
    // Stop monitoring
    if (WnmStop(tWnm.hWnm) != 0)
    {
        logPrint(LOG_ERR, "Dev team support Calling critical abort = %d \n", __LINE__);
        goto CriticalAbort;
    }

    if (--bContinuousMonitoringEnable)
    {
        logPrint(LOG_ERR, "Dev team support find3gCell UMTS scan bContinuousMonitoringEnable = %d linenumber = %d\n", bContinuousMonitoringEnable, __LINE__);
        goto ContinuousMonitoring;
    }

    // Close the application
    if (WnmClose(tWnm.hWnm) != 0)
        goto CriticalAbort;

    memset(&tWnm, 0, sizeof(tWnm));

    return (-1);

CriticalAbort:
    WnmClose(tWnm.hWnm);

    memset(&tWnm, 0, sizeof(tWnm));

    return (-1);
}

void *lteTxJammer(void *pEarfcn)
{

    int i;
    unsigned int earfcn = *((unsigned int *)pEarfcn);
    lteJamCellFound = FALSE;

    bContinuousMonitoringEnable = 1;
    tWnm.PrmSetup.ulEntryCount = 1;
    tWnm.PrmSetup.aEntryList[i].ulWaveForm = cOCTVC1_RADIO_STANDARD_ENUM_LTE;
    tWnm.PrmSetup.aEntryList[i].ulBand = getLteBandFromEARFCN(earfcn);
    tWnm.PrmSetup.aEntryList[i].ulUarfcnStart = earfcn;
    tWnm.PrmSetup.aEntryList[i].ulUarfcnStop = earfcn;
    tWnm.PrmSetup.aEntryList[i].lRssiThreshold = (int)(-160.0 * 512.0);   // Rssi threshold
    tWnm.PrmSetup.aEntryList[i].lRSPThresholddBm = (int)(-160.0 * 512.0); // RSP threshold above a cell decoding is performed
    tWnm.PrmSetup.aEntryList[i].lSQThresholddB = (int)(-160.0 * 512.0);   // SQ threshold above a cell decoding is performed
    tWnm.PrmSetup.ulRxGainControlMode = cOCTVC1_RUS_WNM_RXMODE_ENUM_AGC;
    tWnm.PrmSetup.lManualRxGaindB = cOCTVC1_RUS_WNM_RXMODE_MGC_GAIN_USE_DEFAULT;
    tWnm.PrmSetup.aEntryList[i].usMaxBchPayloadBlockCnt = cOCTVC1_RUS_WNM_DEFAULT_BCH_MAX_NUM_TB_IN_FILE;

    logPrint(LOG_DEBUG, "===========================================\n");
    logPrint(LOG_DEBUG, " Application : %s\n", "4G JAMMER");
    logPrint(LOG_DEBUG, " Description : %s\n", "4G Jammer Software");
    logPrint(LOG_DEBUG, " Version     : V_%.1f\n", FXL_SOCKETINTERFACEAPI_VERSION);
    logPrint(LOG_DEBUG, "===========================================\n\n\n");

    /*
     * Perform initialization tasks required by the application.
     */
    if (0 != InitApplication())
    {
        logPrint(LOG_ERR, "4G Tx Jammer : Initialization of Application is failed\n");
        buildAndSend4gTxJammerStatusInd(FXL_4G_TX_JAMMER_INITIALIZATION_FAILED);
        goto CriticalAbort;
    }

    /*
     * Find the 4G Cells to block
     */
    if (0 != find4gCell())
    {
        if (0 == exitLteTxJammer)
        {
            logPrint(LOG_ERR, "4G Tx Jammer : Finding 4G Cell failed\n");
            buildAndSend4gTxJammerStatusInd(FXL_4G_TX_JAMMER_SCANNING_CELL_FAILED);
        }
        goto CriticalAbort;
    }

    if (lteJamCellFound == TRUE)
    {
        logPrint(LOG_DEBUG, "4G Tx Jammer : 4G Cell found ...ConfiguringParams\n");
        buildAndSend4gTxJammerStatusInd(FXL_4G_TX_JAMMER_CELL_FOUND);
        lteConfigureJammerParamsHandler();
    }
    else if (lteJamCellFound == FALSE)
    {
        logPrint(LOG_DEBUG, "4G Tx Jammer : No Cell found ..Cannot GoAhead\n");
        buildAndSend4gTxJammerStatusInd(FXL_4G_TX_JAMMER_NO_CELL_FOUND);
        goto CriticalAbort;
    }

    /*
     * Check the cells found or not

    if (0 == ??)
    {
        logPrint(LOG_ERR, "4G Tx Jammer : No Cell Found\n");
        buildAndSend4gTxJammerStatusInd (FXL_4G_TX_JAMMER_NO_CELL_FOUND);
        goto CriticalAbort;
    }
    else
    {
        logPrint(LOG_INFO,"4G Tx Jammer : Number of Cells  found in EARFCN = %d\n", earfcn, ??);
        buildAndSend4gTxJammerStatusInd (FXL_4G_TX_JAMMER_CELL_FOUND);
    }
      */

CriticalAbort:
    while (1)
    {
        /*
         * Wait for stop command from External Client
         */
        if (exitLteTxJammer)
        {
            exitLteTxJammer = 0;
            lteJamCellFound = FALSE;

            /* Free the memory allocated */
            if (NULL != pEarfcn)
            {
                free(pEarfcn);
                pEarfcn = NULL;
            }

            ExitApplication();

            pthread_exit(NULL);
        }

        // Sleep 2 sec
        OctOsalSleepMs(2000);
    }

    return NULL;
}

#endif

void sendBassSyncCellInfoInd(tBTS btsInfo)
{
    memset(msgBufSls, 0, EXTERNAL_MESSAGE_SIZE);

    fxL2gBassModeSyncCellInfoInd *ind = (fxL2gBassModeSyncCellInfoInd *)msgBufSls;

    ind->hdr.msgLength = sizeof(fxL2gBassModeSyncCellInfoInd);
    ind->hdr.msgType = FXL_2G_BASS_MODE_SYNC_CELL_INFO_IND;

    if (cOCTVC1_RADIO_STANDARD_ENUM_GSM == btsInfo.ulWaveForm)
    {
        ind->hdr.rat = FXL_RAT_2G;
        ind->arfcn = btsInfo.ulRfcn;
        ind->band = convertToFxlBandFromOct(btsInfo.ulBand);
        ind->snr = ((double)btsInfo.lSNR) / 512.0;
    }
    else
    {
        ind->hdr.rat = FXL_RAT_NONE;
    }

    writeToExternalSocketInterface(msgBufSls, sizeof(fxL2gBassModeSyncCellInfoInd));
    return;
}

void sendRssiScanResultInd(tWNM_RSSI_BAND_CHANNEL rssiDetails)
{
    int idx = 0, idx1 = 0;

    memset(msgBufSls, 0, EXTERNAL_MESSAGE_SIZE);

    fxLListenModeRssiScanResultInd *ind = (fxLListenModeRssiScanResultInd *)msgBufSls;

    ind->hdr.msgLength = sizeof(fxLListenModeRssiScanResultInd);
    ind->hdr.msgType = FXL_LISTEN_MODE_RSSI_SCAN_RESULT_IND;

    if (cOCTVC1_RADIO_STANDARD_ENUM_GSM == rssiDetails.ulWaveForm)
    {
        ind->hdr.rat = FXL_RAT_2G;
        ind->band2g = convertToFxlBandFromOct(rssiDetails.ulBand);
    }
    else if (cOCTVC1_RADIO_STANDARD_ENUM_UMTS == rssiDetails.ulWaveForm)
    {
        ind->hdr.rat = FXL_RAT_3G;
        ind->band3g = convertToFxLFromOctBand(rssiDetails.ulBand);
    }
    else if (cOCTVC1_RADIO_STANDARD_ENUM_LTE == rssiDetails.ulWaveForm)
    {
        ind->hdr.rat = FXL_RAT_4G;
        ind->band4g = rssiDetails.ulBand;
    }
    else
    {
        ind->hdr.rat = FXL_RAT_NONE;
    }

    ind->numOfChannels = rssiDetails.ulNumRssiChannel;
    for (idx = 0; idx < rssiDetails.ulNumRssiChannel; idx++)
    {
        ind->channelInfo[idx].channelNum = rssiDetails.aChannelEntry[idx].ulRfcn;
        ind->channelInfo[idx].rssi = (rssiDetails.aChannelEntry[idx].lRssidBm / 512.0);
    }

    writeToExternalSocketInterface(msgBufSls, sizeof(fxLListenModeRssiScanResultInd));

    return;
}

int initUmtsApplication(void)
{
    int retCode = TRUE;
    tOCT_UINT32 ulResult;

    memcpy(g_AppCfg.abyProcessorCtrlMacAddr, g_L1CConfigDB.l23SrcMacAddr, 6);
    memcpy(g_AppCfg.abyTargetCtrlMacAddr, g_L1CConfigDB.l1DstMacAddr, 6);

    // init Application : Open pktApi
    ulResult = OctSamplesOpenPktApiSession(&g_AppCtxRf.PktApiInfo,
                                           g_AppCfg.abyProcessorCtrlMacAddr,
                                           g_AppCfg.abyTargetCtrlMacAddr);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "initUmtsApplication Error: OctSamplesOpenPktApiSession() failed,  rc = 0x%08x (%s)\n",
                ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        retCode = FALSE;
    }

    return retCode;
}

int exitUmtsApplication(void)
{
    int retCode = TRUE;
    tOCT_UINT32 ulResult;

    ulResult = OctSamplesClosePktApiSession(&g_AppCtxRf.PktApiInfo);
    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "exitUmtsApplication Error: OctSamplesClosePktApiSession() failed,  rc = 0x%08x (%s)\n",
                ulResult, mOCTVC1_RC2STRING(ulResult));
        fprintf(stderr, "(%s)\n\n", octvc1_rc2string(ulResult));
        retCode = FALSE;
    }

    return retCode;
}

void enableTxCalibrationFor3G()
{
    // Initialize UMTS Application
    if ((FALSE == initUmtsApplication()) ||
        (NULL == g_AppCtxRf.PktApiInfo.pPktApiSess))
    {
        sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_3G);
        return;
    }

    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD TxCalInfoCmd;
    tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP TxCalInfoRsp;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD TxCalModifyCmd;
    tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP TxCalModifyRsp;

    tOCTVC1_PKT_API_CMD_EXECUTE_PARMS CmdExecuteParms;
    tOCT_UINT32 ulResult;

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
    TxCalInfoCmd.ulPortIndex = 0;

    if (TRUE == isItPrimaryDsp())
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
    }
    else
    {
        TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
    }

    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

    mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
    CmdExecuteParms.pCmd = &TxCalInfoCmd;
    CmdExecuteParms.pRsp = &TxCalInfoRsp;
    CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

    ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

    if (cOCTVC1_RC_OK != ulResult)
    {
        fprintf(stderr, "enableTxCalibrationFor3G Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n",
                ulResult, mOCTVC1_RC2STRING(ulResult));
        sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_3G);
        return;
    }

    /*
     * Swap the message response.
     */
    mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

    if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_UNAVAILABLE)
    {
        sendTxCalibrationRsp(FXL_FAILURE, 2, FXL_RAT_3G);
        return;
    }
    else if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_INACTIVE)
    {
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_DEF(&TxCalModifyCmd);
        if (TRUE == isItPrimaryDsp())
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalModifyCmd.ulAntennaIndex = txAntennaIdSec;
        }

        TxCalModifyCmd.ulPortIndex = 0;
        TxCalModifyCmd.ulActivateFlag = cOCT_TRUE;
        mOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalModifyCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalModifyCmd;
        CmdExecuteParms.pRsp = &TxCalModifyRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_RSP);

        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "enableTxCalibrationFor3G Error: tOCTVC1_HW_MSG_RF_PORT_MODIFY_ANTENNA_TX_CALIB_CMD, rc = 0x%08x (%s)\n",
                    ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_3G);
            return;
        }

        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_DEF(&TxCalInfoCmd);
        TxCalInfoCmd.ulPortIndex = 0;
        if (TRUE == isItPrimaryDsp())
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdPri;
        }
        else
        {
            TxCalInfoCmd.ulAntennaIndex = txAntennaIdSec;
        }
        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD_SWAP(&TxCalInfoCmd);

        mOCTVC1_PKT_API_CMD_EXECUTE_PARMS_DEF(&CmdExecuteParms);
        CmdExecuteParms.pCmd = &TxCalInfoCmd;
        CmdExecuteParms.pRsp = &TxCalInfoRsp;
        CmdExecuteParms.ulMaxRspLength = sizeof(tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP);

        ulResult = OctVc1PktApiSessCmdExecute(g_AppCtxRf.PktApiInfo.pPktApiSess, &CmdExecuteParms);

        if (cOCTVC1_RC_OK != ulResult)
        {
            fprintf(stderr, "enableTxCalibrationFor3G Error: tOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_CMD failed, rc = 0x%08x (%s)\n",
                    ulResult, mOCTVC1_RC2STRING(ulResult));
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_3G);
            return;
        }

        /*
         * Swap the message response.
         */
        mOCTVC1_HW_MSG_RF_PORT_INFO_ANTENNA_TX_CALIB_RSP_SWAP(&TxCalInfoRsp);

        if (TxCalInfoRsp.ulCalibState == cOCTVC1_HW_RF_PORT_ANTENNA_CALIB_STATE_ENUM_ACTIVE)
        {
            sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_3G);
        }
        else
        {
            sendTxCalibrationRsp(FXL_FAILURE, 1, FXL_RAT_3G);
        }
    }
    else
    {
        sendTxCalibrationRsp(FXL_SUCCESS, 0, FXL_RAT_3G);
    }

    // Exit UMTS Application
    exitUmtsApplication();
}
