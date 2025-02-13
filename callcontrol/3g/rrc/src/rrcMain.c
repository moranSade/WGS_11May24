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
 ############################################################################ */

/*############################################################################
 * File Name   : rrcMain.c 
 *
 * Description : This file provides main functionality of Radio Resource Control
 *               (RRC) stack for ImsiCatcher application.
 *
 * History     :
 *
 * Date			Author		            Details
 * ---------------------------------------------------------------------------
 * 26/09/2014	        Ashutosh Samal              File Creation
 ############################################################################ */
 
/* -----------------------------------------------------
 *                 Include Files
 * -----------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "rrc.h"
#include "rrcItf.h"
#include "rrcProto.h"
#include "cmnDs.h"
#include "taskHndlr.h"
#include "msgQue.h"
#include "cmnPf.h"
#include "timerApi.h"
#include "fxLSocketInterfaceApi.h"
#include "cmnDebug.h"
#include "clContext.h"
#include "gsmCommon.h"

static DbgModule_e  DBG_MODULE = rrc;
extern ClCntxt_t                        gClCntxt;
extern unsigned char* getAccessToIdList();
extern releaseAccessToIdList(unsigned char* shMemPtr);
/* -----------------------------------------------------
 *                 Global Variables
 * -----------------------------------------------------
 */

/*
 * UE Control Block
 */
RrcUeControlBlock ueControlBlock;

/*
 * UMTS Config Control Block
 */
extern UmtsConfigControlBlock gUmtsConfig;

/*
 * Client File Descriptor for sending indication to external client
 */
extern int clientFd;

/*
 * Power Offset PPM
 */
unsigned int rrcConSetupPowerOffsetPpm [MAX_CTFC2] = { 4, 5 };

/*
 *############################################################################
 * Method Name : autoConfigureRrcIdentityList
 *
 * Description : This procedure adds black listed IMSI/IMEI in RRC Context
 *
 * Parameters	: None
 *
 * Returns 	: None
 *############################################################################
 */
#if 0
void autoConfigureRrcIdentityList (fxL3gAutoConfigCmd *pIdentityList)
{
    unsigned int idx = 0;
    unsigned int preEntry = 0;


    if (FXL_BLACKUSERSLIST == pIdentityList->listType)
    {
        if (FXL_IDENTITY_IMSI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfBlackListedImsi;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.blackListedImsi[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMSI_LENGTH);
               ueControlBlock.numOfBlackListedImsi++;
            }
        }
        else if (FXL_IDENTITY_IMEI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfBlackListedImei;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.blackListedImei[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMEI_LENGTH);
               ueControlBlock.numOfBlackListedImei++;
            }
        }
        else 
        {
            //Nothing to do
        }
    }
    else if (FXL_WHITEUSERSLIST == pIdentityList->listType)
    {
        if (FXL_IDENTITY_IMSI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfWhiteListedImsi;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.whiteListedImsi[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMSI_LENGTH);
               ueControlBlock.numOfWhiteListedImsi++;
            }
        }
        else if (FXL_IDENTITY_IMEI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfWhiteListedImei;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.whiteListedImei[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMEI_LENGTH);
               ueControlBlock.numOfWhiteListedImei++;
            }
        }
        else 
        {
            //Nothing to do
        }
    }
    else
    {
        //Nothing to do
    }
}
#endif

void AddEntryInRrcIdentityList(fxLConfigureIdentityListCmd *pIdentityList)
{
#if 0
    unsigned int idx = 0;
    unsigned int preEntry = 0;

#ifdef PRINT_IDENTITY_LIST
    DEBUGMSG(( "Identity List After Adding Entries ...\n"));
    DEBUGMSG(( "LIST-TYPE  = %s", pIdentityList->listType == 0 ? "WHITELIST" : "BLACKLIST"));
    DEBUGMSG(( "ID-TYPE    = %s\n", pIdentityList->idType == 0 ? "IMSI" : "IMEI"));
#endif

    if (FXL_BLACKUSERSLIST == pIdentityList->listType)
    {
        if (FXL_IDENTITY_IMSI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfBlackListedImsi;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.blackListedImsi[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMSI_LENGTH);
               ueControlBlock.numOfBlackListedImsi++;
            }
        }
        else if (FXL_IDENTITY_IMEI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfBlackListedImei;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.blackListedImei[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMEI_LENGTH);
               ueControlBlock.numOfBlackListedImei++;
            }
        }
        else 
        {
            //Nothing to do
        }
    }
    else if (FXL_WHITEUSERSLIST == pIdentityList->listType)
    {
        if (FXL_IDENTITY_IMSI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfWhiteListedImsi;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.whiteListedImsi[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMSI_LENGTH);
               ueControlBlock.numOfWhiteListedImsi++;
            }
        }
        else if (FXL_IDENTITY_IMEI == pIdentityList->idType)
        {
            preEntry = ueControlBlock.numOfWhiteListedImei;
            for (idx = 0; idx < pIdentityList->noOfEntries; idx++,preEntry++)
            {
               strncpy(ueControlBlock.whiteListedImei[preEntry], 
                       pIdentityList->identityList[idx], MAX_IMEI_LENGTH);
               ueControlBlock.numOfWhiteListedImei++;
            }
        }
        else 
        {
            //Nothing to do
        }
    }
    else
    {
        //Nothing to do
    }
#ifdef PRINT_IDENTITY_LIST
    for (idx = 0; idx < preEntry; idx++)
    {
       DEBUGMSG(( "IMSI/IMEI : %s\n",updatedListPtr[idx]));
    }
#endif
#endif
}

#if 0
void DeleteEntryFromRrcIdentityList(fxLConfigureIdentityListCmd *pIdentityList)
{
    unsigned int inDelEntryIdx   = 0;
    unsigned int identityListIdx = 0;
    unsigned int i = 0, j = 0;

    switch(pIdentityList->listType)
    {
    case FXL_BLACKUSERSLIST:
        switch(pIdentityList->idType)
        {
            case FXL_IDENTITY_IMSI:
                for (inDelEntryIdx = 0; inDelEntryIdx < pIdentityList->noOfEntries; inDelEntryIdx++)
                {
                   for(identityListIdx = 0; identityListIdx < ueControlBlock.numOfBlackListedImsi; identityListIdx++)
                   {
                      // Check if UE present in Identity List
                      if(0 == strncmp(pIdentityList->identityList[inDelEntryIdx], ueControlBlock.blackListedImsi[identityListIdx], MAX_IMSI_LENGTH) )
                      {
                         if(identityListIdx != ueControlBlock.numOfBlackListedImsi -1)
                         {  // Replace Deleted Entry with Last entry and decrease number of list entries
                            strncpy( ueControlBlock.blackListedImsi[identityListIdx], 
                                     ueControlBlock.blackListedImsi[ueControlBlock.numOfBlackListedImsi - 1], MAX_IMSI_LENGTH);
                         }
                         ueControlBlock.numOfBlackListedImsi--;
                      }
                   }
                }
                break;
            case FXL_IDENTITY_IMEI:
                for (inDelEntryIdx = 0; inDelEntryIdx < pIdentityList->noOfEntries; inDelEntryIdx++)
                {
                   for(identityListIdx = 0; identityListIdx < ueControlBlock.numOfBlackListedImei; identityListIdx++)
                   {
                      // Check if UE present in Identity List
                      if(0 == strncmp(pIdentityList->identityList[inDelEntryIdx], ueControlBlock.blackListedImei[identityListIdx], MAX_IMEI_LENGTH) )
                      {
                         if(identityListIdx != ueControlBlock.numOfBlackListedImei-1)
                         {  // Replace Deleted Entry with Last entry and decrease number of list entries
                            strncpy( ueControlBlock.blackListedImei[identityListIdx], 
                                     ueControlBlock.blackListedImei[ueControlBlock.numOfBlackListedImei - 1], MAX_IMEI_LENGTH);
                         }
                         ueControlBlock.numOfBlackListedImei--;
                      }
                   }
                }
                break;
            default:
                break;
        }
        break;

    case FXL_WHITEUSERSLIST:
        switch(pIdentityList->idType)
        {
            case FXL_IDENTITY_IMSI:
                for (inDelEntryIdx = 0; inDelEntryIdx < pIdentityList->noOfEntries; inDelEntryIdx++)
                {
                   for(identityListIdx = 0; identityListIdx < ueControlBlock.numOfWhiteListedImsi; identityListIdx++)
                   {
                      // Check if UE present in Identity List
                      if(0 == strncmp(pIdentityList->identityList[inDelEntryIdx], ueControlBlock.whiteListedImsi[identityListIdx], MAX_IMSI_LENGTH) )
                      {
                         if(identityListIdx != ueControlBlock.numOfWhiteListedImsi-1)
                         {  // Replace Deleted Entry with Last entry and decrease number of list entries
                            strncpy( ueControlBlock.whiteListedImsi[identityListIdx], 
                                     ueControlBlock.whiteListedImsi[ueControlBlock.numOfWhiteListedImsi- 1], MAX_IMSI_LENGTH);
                         }
                         ueControlBlock.numOfWhiteListedImsi--;
                      }
                   }
                }
                break;
            case FXL_IDENTITY_IMEI:
                for (inDelEntryIdx = 0; inDelEntryIdx < pIdentityList->noOfEntries; inDelEntryIdx++)
                {
                   for(identityListIdx = 0; identityListIdx < ueControlBlock.numOfWhiteListedImei; identityListIdx++)
                   {
                      // Check if UE present in Identity List
                      if(0 == strncmp(pIdentityList->identityList[inDelEntryIdx], ueControlBlock.whiteListedImei[identityListIdx], MAX_IMEI_LENGTH) )
                      {
                         if(identityListIdx != ueControlBlock.numOfWhiteListedImei-1)
                         {  // Replace Deleted Entry with Last entry and decrease number of list entries
                            strncpy( ueControlBlock.whiteListedImei[identityListIdx], 
                                     ueControlBlock.whiteListedImei[ueControlBlock.numOfWhiteListedImei- 1], MAX_IMEI_LENGTH);
                         }
                         ueControlBlock.numOfWhiteListedImsi--;
                      }
                   }
                }
                break;
            default:
                break;
        }
        break;

    default:
        break;
    }
   
}
#endif

/*
 *############################################################################
 * Method Name : configureRrcIdentityList
 *
 * Description : This procedure adds black listed IMSI/IMEI in RRC Context
 *
 * Parameters	: None
 *
 * Returns 	: None
 *############################################################################
 */
void configureRrcIdentityList (fxLConfigureIdentityListCmd *pIdentityList)
{

#if 0
    if(FXL_ADD_ENTRY == pIdentityList->actionOnList)
    {
#endif
        DEBUG1((" Add Entries in Identity List\n"));
        AddEntryInRrcIdentityList(pIdentityList);
#if 0
    }
    else if(FXL_DELETE_ENTRY == pIdentityList->actionOnList)
    {
        DEBUG1((" Delete Entries from Identity List\n"));
        DeleteEntryFromRrcIdentityList(pIdentityList);
    }
    else
    {
        DEBUG1((" configureRrcIdentityList: Wrong value of actionOnList \n"));
        return;
    }
#endif
}

/*
 *############################################################################
 * Method Name : addUeInUeControlBlock
 *
 * Description : This procedure adds the UE in UE Control Block if not present
 *
 * Parameters	: 
 *   pInitialUeIdentity      Pointer to Initial UE Identity
 *
 * Returns 	: 
 *   unsigned int            UE index in UE Control Block
 *############################################################################
 */
unsigned int addUeInUeControlBlock (RrcInitialUEIdentity *pInitialUeIdentity)
{
    unsigned int ueIndex = 0;
    unsigned int idx = 0;
    unsigned int found = 0;
 
    /* Search for unused entry in UE Control Block */
    for (idx = 0; idx < ueControlBlock.numOfUe; idx++)
    {
       if (0 == ueControlBlock.ueSpecificData[idx].ueUsed)
       {
          ueIndex = idx;
          found = 1;
          break;
       }
    }
    
    if (0 == found)
    {
        ueIndex = ueControlBlock.numOfUe;

        /* Increament the number of UE */
        ueControlBlock.numOfUe++;
    }

    if (ueIndex < MAX_UE_SUPPORTED)
    {
       /* Make the entry as Used */
       ueControlBlock.ueSpecificData[idx].ueUsed = 1;

       /* Store Initial UE identity */
       memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
              (RrcInitialUEIdentity *)pInitialUeIdentity,
              sizeof(RrcInitialUEIdentity));

       /* Initialize UE Timer */
       timerInit(&(ueControlBlock.ueSpecificData[ueIndex].ueTimer));
	   timerInit(&(ueControlBlock.ueSpecificData[ueIndex].ueDedMeasTimer));
	   timerInit(&(ueControlBlock.ueSpecificData[ueIndex].ueDedGenericTimer));
    }
    else
    {
        DEBUGMSG(("\n========================================\n"));
        DEBUGMSG(("Maximum Number of UE Supported = %ld\n", MAX_UE_SUPPORTED));
        DEBUGMSG(("========================================\n"));
        ueIndex = 0xFFFF;
    }

    return (ueIndex);
}

/*
 * ############################################################################
 * Method Name : findUeIndexInUeControlBlock
 * 
 * Description : This procedure searches UE Control Block.
 * 
 * Parameters   : 
 *   pInitialUeIdentity      Pointer to Initial UE Identity
 * 
 * Returns      : 
 *   unsigned int            UE index in UE Control Block
 * ############################################################################
 */

unsigned int findUeIndexInUeControlBlock (RrcInitialUEIdentity *pInitialUeIdentity)
{
    unsigned int ueIndex = 0;
    unsigned int idx = 0;
    int found   = 0;
    char imsi[MAX_IMSI_LENGTH];
    char imei[MAX_IMEI_LENGTH];
    char tmsi[MAX_TMSI_LENGTH];

    /* Search in existing UE list */
    if (0 != ueControlBlock.numOfUe)
    {
       if(pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_IMSI)
       {
           for (idx = 0; idx < pInitialUeIdentity->imsiLength; idx++)
           {
               imsi[idx] = pInitialUeIdentity->imsi[idx] + '0';
           }

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
           {
                if (0 == strncmp(imsi,ueControlBlock.ueSpecificData[ueIndex].imsi ,MAX_IMSI_LENGTH))
                {
                     DEBUGMSG(("IMSI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
                     ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
                     found = 1;
                     break;
                }
           }
       }
       else if (pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_IMEI)
       {
           unsigned int lastDigit       = 0;
           unsigned int imeiCount       = 0;
           unsigned int sumOfOddDigits  = 0;
           unsigned int sumOfEvenDigits = 0;

           for (idx = 0; idx < pInitialUeIdentity->imeiLength; idx++)
           {
                imei[idx] = pInitialUeIdentity->imei[idx] + '0';
           }

           /* Calculate Sum of Digits at Odd index */
           for (imeiCount = 1; imeiCount < pInitialUeIdentity->imeiLength; imeiCount = imeiCount + 2)
           {
                if (((pInitialUeIdentity->imei[imeiCount]) * 2) >= 10)
                {
                      sumOfOddDigits = sumOfOddDigits +
                                          ((((pInitialUeIdentity->imei[imeiCount]) * 2) % 10) + 1);
                }
                else
                {
                      sumOfOddDigits = sumOfOddDigits +
                                        ((pInitialUeIdentity->imei[imeiCount]) * 2);
                }
           }

           /* Calculate Sum of Digits at Even index */
           for (imeiCount = 0; imeiCount < pInitialUeIdentity->imeiLength; imeiCount = imeiCount + 2)
           {
                sumOfEvenDigits = sumOfEvenDigits + (pInitialUeIdentity->imei[imeiCount]);
           }

           if (0 == ((sumOfOddDigits + sumOfEvenDigits) % 10))
           {
                lastDigit = 0;
           }
           else
           {
                lastDigit = 10 - ((sumOfOddDigits + sumOfEvenDigits) % 10);
           }

           /* Store the last digit of IMEI */
           imei[pInitialUeIdentity->imeiLength - 1] = lastDigit + '0';

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
           {
                if (0 == strncmp(imei,ueControlBlock.ueSpecificData[ueIndex].imei,MAX_IMEI_LENGTH))
                {
                     DEBUGMSG(("IMEI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
                     ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
                     found = 1;
                     break;
                }
           }
       }
       else if ((pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_TMSI_AND_LAI) ||
                (pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_PTMSI_AND_RAI))
       {
           if(pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_TMSI_AND_LAI)
           {
               /* Fill TMSI */
               for (idx = 0; idx < SIZE_OF_TMSI_IDENTITY; idx++)
               {
                   tmsi[idx] =  pInitialUeIdentity->tmsi[idx];
               }
           }
           else
           {
               /* Fill P-TMSI */
               for (idx = 0; idx < SIZE_OF_TMSI_IDENTITY; idx++)
               {
                   tmsi[idx] =  pInitialUeIdentity->ptmsi[idx];
               }
           }

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
           {
                if (0 == strncmp(tmsi, ueControlBlock.ueSpecificData[ueIndex].tmsi, MAX_TMSI_LENGTH))
                {
                     DEBUGMSG(("TMSI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
                     ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
                     found = 1;
                     break;
                }
           }
       }
    }
   
    if (0 == found)
    {
        ueIndex = 0xFFFF;
    }
 
    return (ueIndex);
}

/*
 *############################################################################
 * Method Name : checkAndUpdateUeControlBlock
 *
 * Description : This procedure searches UE Control Block and adds the UE in
 *               UE Control Block if not present already.
 *
 * Parameters	: 
 *   pInitialUeIdentity      Pointer to Initial UE Identity
 *
 * Returns 	: 
 *   unsigned int            UE index in UE Control Block
 *############################################################################
 */
unsigned int getUeIndexInUeControlBlock (RrcInitialUEIdentity *pInitialUeIdentity)
{
    unsigned int ueIndex = 0;
    unsigned int idx = 0;
    int found   = 0;
    char imsi[MAX_IMSI_LENGTH];
    char imei[MAX_IMEI_LENGTH];
    char tmsi[MAX_TMSI_LENGTH];

    /* Search in existing UE list */
    if (0 != ueControlBlock.numOfUe)
    {
       if(pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_IMSI)
       {
           for (idx = 0; idx < pInitialUeIdentity->imsiLength; idx++)
           {
               imsi[idx] = pInitialUeIdentity->imsi[idx] + '0';
           }

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
       	   {
          	if (0 == strncmp(imsi,ueControlBlock.ueSpecificData[ueIndex].imsi ,MAX_IMSI_LENGTH))
          	{
              	     DEBUGMSG(("IMSI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
	             ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
        	     found = 1;
              	     break;
          	}
       	   }
       }
       else if (pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_IMEI)
       {
           unsigned int lastDigit       = 0;
           unsigned int imeiCount       = 0;
           unsigned int sumOfOddDigits  = 0;
           unsigned int sumOfEvenDigits = 0;

           for (idx = 0; idx < pInitialUeIdentity->imeiLength; idx++)
           {
                imei[idx] = pInitialUeIdentity->imei[idx] + '0';
           }

           /* Calculate Sum of Digits at Odd index */
           for (imeiCount = 1; imeiCount < pInitialUeIdentity->imeiLength; imeiCount = imeiCount + 2)
           {
                if (((pInitialUeIdentity->imei[imeiCount]) * 2) >= 10)
                {
                      sumOfOddDigits = sumOfOddDigits +
                                          ((((pInitialUeIdentity->imei[imeiCount]) * 2) % 10) + 1);
                }
                else
                {
                      sumOfOddDigits = sumOfOddDigits +
                                        ((pInitialUeIdentity->imei[imeiCount]) * 2);
                }
           }

           /* Calculate Sum of Digits at Even index */
           for (imeiCount = 0; imeiCount < pInitialUeIdentity->imeiLength; imeiCount = imeiCount + 2)
           {
                sumOfEvenDigits = sumOfEvenDigits + (pInitialUeIdentity->imei[imeiCount]);
           }

           if (0 == ((sumOfOddDigits + sumOfEvenDigits) % 10))
           {
                lastDigit = 0;
           }
           else
           {
                lastDigit = 10 - ((sumOfOddDigits + sumOfEvenDigits) % 10);
           }

           /* Store the last digit of IMEI */
           imei[pInitialUeIdentity->imeiLength - 1] = lastDigit + '0';

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
           {
                if (0 == strncmp(imei,ueControlBlock.ueSpecificData[ueIndex].imei,MAX_IMEI_LENGTH))
                {
                     DEBUGMSG(("IMEI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
                     ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
                     found = 1;
                     break;
                }
           }
       }
       else if ((pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_TMSI_AND_LAI) ||
                (pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_PTMSI_AND_RAI))
       {
           if(pInitialUeIdentity->initialUEIdentityType == UE_IDENTITY_TMSI_AND_LAI)
           {
               /* Fill TMSI */
               for (idx = 0; idx < SIZE_OF_TMSI_IDENTITY; idx++)
               {
                   tmsi[idx] =  pInitialUeIdentity->tmsi[idx];
               }
           }
           else
           {
               /* Fill P-TMSI */
               for (idx = 0; idx < SIZE_OF_TMSI_IDENTITY; idx++)
               {
                   tmsi[idx] =  pInitialUeIdentity->ptmsi[idx];
               }
           }

           for (ueIndex = 0; ueIndex < ueControlBlock.numOfUe; ueIndex++)
           {
                if (0 == strncmp(tmsi, ueControlBlock.ueSpecificData[ueIndex].tmsi, MAX_TMSI_LENGTH))
                {
                     DEBUGMSG(("TMSI: UE is already present in UE Control Block Index(%ld)\n", ueIndex));
                     ueControlBlock.ueSpecificData[ueIndex].ueUsed = 1;
                     memcpy((RrcInitialUEIdentity *)&ueControlBlock.ueSpecificData[ueIndex].ueInitialIdentity,
                            (RrcInitialUEIdentity *)pInitialUeIdentity,
                             sizeof(RrcInitialUEIdentity));
                     found = 1;
                     break;
                }
           }
       }
    }

    /* Add the new UE to UE Control Block */
    if (0 == found)
    {
        ueIndex = addUeInUeControlBlock (pInitialUeIdentity);
    }

    return (ueIndex);
}

/*
 *############################################################################
 * Method Name : cleanUeContextInUeControlBlock
 *
 * Description : This procedure cleans the UE Context & updates the UE Control
 *               Control Block.
 * Parameters	: 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns 	: None
 *############################################################################
 */
void cleanUeContextInUeControlBlock (unsigned int ueIndex)
{
    unsigned int idx = 0;

    /* Clear the UE Specific Data */ 
    if (1 == ueControlBlock.ueSpecificData[ueIndex].stopMsgHandling)
    {
        ueControlBlock.ueSpecificData[ueIndex].ueState = UE_STOP_MSG_HANDLING;
        ueControlBlock.cleanupCount--;

        if (0 == ueControlBlock.cleanupCount)
        {
            /* Send Delete PCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_CONFIG_DEL_PCH_DATA_REQUEST_E, NULL);


            /* Clean the Memory Allocated */
            delLatchedUeList ();
            memset ((RrcUeControlBlock *)&ueControlBlock, 0, sizeof(RrcUeControlBlock));

            pthread_exit (NULL);
        }
    }
    else
    {
        memset((RrcUeSpecificData *)&ueControlBlock.ueSpecificData[ueIndex], 0, 
               sizeof(RrcUeSpecificData));
    }
}

/*
 *############################################################################
 * Method Name : handleRrcConnectionSetupTimeout
 *
 * Description : This procedure handles RRC Connection Setup Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleRrcConnectionSetupTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUG1(("RRC Connection Setup Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_CONN_SETUP_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : trigPerUeSingleMeas
 *
 * Description : This procedure triggeres Single Measurement RTT/RSCP for each UE
 *
 * Parameters  : 
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void trigPerUeSingleMeas(unsigned int pUeIndex)
{
    MeasInfo_t   *pMsgQMeasInfo = NULL;

    DEBUG1(("RRC: Trigger Single UE measurement Rtt/Rscp for UE Id(%ld)\n", pUeIndex));

    // Trigger On Demand RTT Measurement
    pMsgQMeasInfo            = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
    pMsgQMeasInfo->rlId      = pUeIndex;
    pMsgQMeasInfo->measType  = FXL_RTT_MEAS;
    SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E, CL_NBAP_UE_RTT_ON_DEMAND_MEAS_INIT_REQ_E, pMsgQMeasInfo);

    // Trigger On Demand RSCP Measurement
    pMsgQMeasInfo            = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
    pMsgQMeasInfo->rlId      = pUeIndex;
    pMsgQMeasInfo->measType  = FXL_RSCP_MEAS;
    SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E, CL_NBAP_UE_RSCP_ON_DEMAND_MEAS_INIT_REQ_E, pMsgQMeasInfo);

}

/*
 *############################################################################
 * Method Name : handleRadioLinkSetupRequestTimeout
 *
 * Description : This procedure sends RL Failure if there is no repsonse/ worng
 *               response for Radio Link Setup Request.
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleRadioLinkSetupRequestTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId =  (unsigned int *)pUeIndex;

    DEBUG1(("Radio Link Setup Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_RL_SETUP_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : handleUeEventInIdleState
 *
 * Description : This procedure handles all the events received in IDLE state
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInIdleState (unsigned int ueIndex, RrcUeEvent ueEvent)
{
    unsigned int *pMsgQUeId = NULL;
    unsigned int *pTimerUeId = NULL;

    /* Check for the events received in IDLE state */
    switch (ueEvent)
    {
        case UE_RRC_CON_REQ:
           /* Change the UE State */
           ueControlBlock.ueSpecificData[ueIndex].ueState = 
                                              UE_WAIT_FOR_RL_SETUP_RESP;

           /* Start the Timer for RL Setup Request */
           pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
           *pTimerUeId = ueIndex;

           timerStart(&(ueControlBlock.ueSpecificData[ueIndex].ueTimer),
                      RL_SETUP_REQ_TIMEOUT,
                      0,
                      handleRadioLinkSetupRequestTimeout,
                      pTimerUeId);
 
           /* Post Radio Link Setup Request Event to CL Send Task */
           pMsgQUeId  = (unsigned int *)malloc (sizeof(ueIndex));
           *pMsgQUeId = ueIndex;

           SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E, 
                         CL_NBAP_UE_RADIO_LINK_SETUP_REQ_E, pMsgQUeId);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_IDLE state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : fillRrcConnectionSetup
 *
 * Description : This procedure fills RRC Connection Setup. 
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void fillRrcConnectionSetup (unsigned int ueIndex)
{
   unsigned int idx  = 0;
   unsigned int idx1 = 0;

   RrcConnectionSetupData *pRrcConnSetup = 
                        &ueControlBlock.ueSpecificData[ueIndex].ueRrcConnSetup;

   /* Fill RRC Connection Setup Transaction Id */
   pRrcConnSetup->rrcTransactionId =
                       ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId;

   /* Fill Activation Time */
   pRrcConnSetup->activationTimePrsence = VALUE_NOT_PRESENT;
   pRrcConnSetup->activationTime = VALUE_NOT_PRESENT;

   /* Fill New URNTI */
   pRrcConnSetup->newURNTI.sRNCIdentityLength = SRNC_IDENTITY_LENGTH;
   pRrcConnSetup->newURNTI.sRNCIdentityValue  = gUmtsConfig.rncId;
   pRrcConnSetup->newURNTI.sRNTILength        = SRNTI_LENGTH;
   pRrcConnSetup->newURNTI.sRNTIValue         = SRNTI + ueIndex;

   /* Fill RRC State Indicator */
   pRrcConnSetup->rrcStateIndicator = RRC_STATE_INDICATOR_CELL_DCH;
   
   /* Fill UTRAN DRX Cycle Length Coeff */
   pRrcConnSetup->utranDRXCycleLengthCoeff = UTRAN_DRX_CYCLE_LENGTH_COEFF;

   /* Fill RRC Capability Update Requirement */
   pRrcConnSetup->capabilityUpdateRequirementPresence = VALUE_PRESENT;
   RrcCapabilityUpdateRequirement *pRrcCapUpdReq = 
                                 &pRrcConnSetup->capabilityUpdateRequirement;
   pRrcCapUpdReq->ueRadioCapabilityFDDUpdateRequirement  = VALUE_PRESENT;
   pRrcCapUpdReq->ueRadioCapabilityTDDUpdateRequirement  = VALUE_NOT_PRESENT;
   pRrcCapUpdReq->systemSpecificCapUpdateReqListPresence = VALUE_PRESENT;         
   pRrcCapUpdReq->numOfSystemSpecificCapUpdateReqList    = VALUE_PRESENT;

   /* Fill SRB Info */
   pRrcConnSetup->numOfSrb = MAX_SRB_PER_UE;
   for (idx = 0; idx < MAX_SRB_PER_UE; idx++)
   {
       RrcSrbInfo *pSrbInfo = &pRrcConnSetup->srbInfo[idx];

       pSrbInfo->rbIdPresence = VALUE_PRESENT;
       pSrbInfo->rbId = idx + 1;
       pSrbInfo->rlcInfoType = RLC_INFO_CHOICE_RLC_INFO;
       pSrbInfo->ulRlcModePresence = VALUE_PRESENT;
       pSrbInfo->dlRlcModePresence = VALUE_PRESENT;
       if (0 == idx)
       {
           pSrbInfo->ulRlcModeType = UL_UM_RLC_MODE;
           pSrbInfo->dlRlcModeType = DL_UM_RLC_MODE;
       }
       else
       {
           pSrbInfo->ulRlcModeType = UL_AM_RLC_MODE;
           pSrbInfo->ulAmRlcModeTransmitDiscardType = 
                                          TRANSMISSION_RLC_DISCARD_NO_DISCARD;
           pSrbInfo->ulAmRlcModeTransmitDiscardValue = MAX_DAT_DAT20;
           pSrbInfo->ulAmRlcModeTransmitWindowSize = 
                                          TRANSMISSION_WINDOW_SIZE_TW32;
           pSrbInfo->ulAmRlcModeTimerReset = TIMER_RESET_TR300;
           pSrbInfo->ulAmRlcModeMaxReset = MAX_RESET_4;
           pSrbInfo->ulAmRlcModePollingInfoPresence = VALUE_PRESENT;
           pSrbInfo->ulAmRlcModeTimerPollProhibitPresence = VALUE_NOT_PRESENT;
           pSrbInfo->ulAmRlcModeTimerPollProhibit = VALUE_NOT_PRESENT;
           pSrbInfo->ulAmRlcModeTimerPollPresence = VALUE_PRESENT;
           pSrbInfo->ulAmRlcModeTimerPoll = TIMER_POLL_TP150;
           pSrbInfo->ulAmRlcModePollPDUPresence = VALUE_PRESENT;
           pSrbInfo->ulAmRlcModePollPDU = POLL_PDU_PDU4;
           pSrbInfo->ulAmRlcModePollSDUPresence = VALUE_PRESENT;
           pSrbInfo->ulAmRlcModePollSDU = POLL_SDU_SDU1;
           pSrbInfo->ulAmRlcModeLastTransmissionPDUPoll = VALUE_PRESENT;
           pSrbInfo->ulAmRlcModeLastRetransmissionPDUPoll = VALUE_PRESENT;
           pSrbInfo->ulAmRlcPollWindowPresence = VALUE_PRESENT;
           pSrbInfo->ulAmRlcPollWindow = POLL_WINDOW_PW70;
           pSrbInfo->ulAmRlcTimerPollPeriodicPresence = VALUE_NOT_PRESENT;
           pSrbInfo->ulAmRlcTimerPollPeriodic = VALUE_NOT_PRESENT;

           pSrbInfo->dlRlcModeType = DL_AM_RLC_MODE;
           pSrbInfo->dlAmRlcModeInSequenceDelivery = VALUE_PRESENT;
           pSrbInfo->dlAmRlcModeReceivingWindowSize = RECIEVING_WINDOW_SIZE_RW32;
           pSrbInfo->dlAmRlcModeTimerStatusProhibitPresence = VALUE_NOT_PRESENT; 
           pSrbInfo->dlAmRlcModeTimerStatusProhibit = VALUE_NOT_PRESENT; 
           pSrbInfo->dlAmRlcModeDummyPresence = VALUE_NOT_PRESENT; 
           pSrbInfo->dlAmRlcModeDummy = VALUE_NOT_PRESENT;
           pSrbInfo->dlAmRlcModeMissingPDUIndicator = VALUE_PRESENT;
           pSrbInfo->dlAmRlcModeTimerStatusPeriodicPresence = VALUE_NOT_PRESENT;
           pSrbInfo->dlAmRlcModeTimerStatusPeriodic = VALUE_NOT_PRESENT;
       }

       pSrbInfo->numOfRBMappingInfo = MAX_RB_MAPPING_INFO;
       for (idx1 = 0; idx1 < MAX_RB_MAPPING_INFO; idx1++)
       {
           RrcSrbRbMappingInfo *pRbMappingInfo = 
                                             &pSrbInfo->rbMappingInfo[idx1]; 

           pRbMappingInfo->ulLogicalChannelMappingsPresence = VALUE_PRESENT;
           pRbMappingInfo->ulLogicalChannelMappingType = 
                                             UL_LOGICAL_CHANNEL_MAPPING_ONE;
           if (0 == idx1)
           {
               pRbMappingInfo->ulTransportChannelType =
                                             UL_TRANSPORT_CHANNEL_TYPE_DCH;
               pRbMappingInfo->ulTransportChannelDch = DCH_ID + ueIndex;
               pRbMappingInfo->rlcSizeListType = RLC_SIZE_LIST_CONFIGURED;
               pRbMappingInfo->dlTransportChannelType =
                                             DL_TRANSPORT_CHANNEL_TYPE_DCH;
               pRbMappingInfo->dlTransportChannelDch = DCH_ID + ueIndex;
           }
           else
           {
               pRbMappingInfo->ulTransportChannelType =
                                             UL_TRANSPORT_CHANNEL_TYPE_RACH;
               pRbMappingInfo->rlcSizeListType = RLC_SIZE_LIST_EXPLICIT_LIST;
               pRbMappingInfo->numOfRlcSizeExplicitList = VALUE_PRESENT;
               pRbMappingInfo->rlcSizeIndex = VALUE_PRESENT;
               pRbMappingInfo->dlTransportChannelType =
                                             DL_TRANSPORT_CHANNEL_TYPE_FACH;
           }

           pRbMappingInfo->logicalChannelIdentityPresence = VALUE_PRESENT;
           pRbMappingInfo->logicalChannelIdentity = idx + 1;
           pRbMappingInfo->macLogicalChannelPriority = idx + 1;
           pRbMappingInfo->dlLogicalChannelMappingsPresence = VALUE_PRESENT;
           pRbMappingInfo->numOfDlLogicalCahnnelMappings = VALUE_PRESENT;
           pRbMappingInfo->dlLogicalChannelIdPresence = VALUE_PRESENT;
           pRbMappingInfo->dlLogicalChannelId = idx + 1;
       }
   }

   /* Fill UL Common Transport Channel Info */
   pRrcConnSetup->ulCommonTransportChannelInfoPresence = VALUE_PRESENT;
   RrcUlCommonTransChInfo *pUlCmnTransChInfo = 
                                &pRrcConnSetup->ulCommonTransportChannelInfo;
   pUlCmnTransChInfo->modeSpecificInfoPresence = VALUE_PRESENT;
   pUlCmnTransChInfo->tfcsType = TFCS_NORMAL_TFCI_SIGNALLING;
   pUlCmnTransChInfo->tfcsConfigurationType = TFCS_CONFIGURATION_COMPLETE;
   pUlCmnTransChInfo->ctfcSizeType = CTFC_SIZE_2BIT;
   pUlCmnTransChInfo->numOfCtfc2Bit = MAX_CTFC2;
   for (idx = 0; idx < MAX_CTFC2; idx++)
   {
       pUlCmnTransChInfo->ctfc2Bit[idx].ctfc2 = idx;
       pUlCmnTransChInfo->ctfc2Bit[idx].powerOffsetInformationPresence =
                                                                 VALUE_PRESENT;
       pUlCmnTransChInfo->ctfc2Bit[idx].gainFactorInformationType = 
                                                          SIGNALED_GAIN_FACTOR;
       pUlCmnTransChInfo->ctfc2Bit[idx].gainFactorBetaC = GAIN_FACTOR_BETAC;
       pUlCmnTransChInfo->ctfc2Bit[idx].gainFactorBetaD = GAIN_FACTOR_BETAD;
       pUlCmnTransChInfo->ctfc2Bit[idx].powerOffsetPpmPresence = VALUE_PRESENT;
       pUlCmnTransChInfo->ctfc2Bit[idx].powerOffsetPpm = rrcConSetupPowerOffsetPpm[idx];
   }

   /* Fill UL Add Reconfig Transport Channel List */
   RrcUlAddReconfTransChInfoList *pUlAddReconf = 
                                 &pRrcConnSetup->ulAddReconfTransChInfoList;
   pUlAddReconf->numOfUlAddReconfTransChInfo = VALUE_PRESENT;
   pUlAddReconf->ulTransportChannelType = UL_TRANSPORT_CHANNEL_TYPE_DCH;
   pUlAddReconf->transportChannelIdentity = DCH_ID + ueIndex;
   pUlAddReconf->transportFormatSetType = DEDICATED_TRANSPORT_CHANNEL_TFS;
   pUlAddReconf->dedicatedTransChInfo.ttiType = 
                                      DEDICATED_TRANSPORT_CHANNEL_TFS_TTI40;
   pUlAddReconf->dedicatedTransChInfo.numOfTti40 = VALUE_PRESENT;
   pUlAddReconf->dedicatedTransChInfo.tti40.rlcSizeType = 
                                                 RLC_SIZE_OCTET_MODE_TYPE1;
   pUlAddReconf->dedicatedTransChInfo.tti40.octectModeType =
                                                 OCTET_MODE_RLC_SIZE_TYPE1;
   pUlAddReconf->dedicatedTransChInfo.tti40.sizeType1 = 16;
   pUlAddReconf->dedicatedTransChInfo.tti40.numOfTbSizeList = MAX_TB_SIZE_LIST;
   for (idx = 0; idx < MAX_TB_SIZE_LIST; idx++)
   {
      if (0 == idx)
      {
          pUlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].numberOfTransportBlocksType =
                                                  NUMBER_OF_TRANSPORT_BLOCKS_ZERO;
      }
      else
      {
          pUlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].numberOfTransportBlocksType =
                                                  NUMBER_OF_TRANSPORT_BLOCKS_ONE;
      }
      pUlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].transportBlockvalue = 
                                                          VALUE_NOT_PRESENT;
   }
   pUlAddReconf->dedicatedTransChInfo.tti40.logicalChannelListType = 
                                               LOGICAL_CHANNEL_LIST_ALL_SIZES;
   pUlAddReconf->dedicatedTransChInfo.semistaticTFInfo.channelCodingType =
                                               CHANNEL_CODING_TYPE_CONVOLUTIONAL;
   pUlAddReconf->dedicatedTransChInfo.semistaticTFInfo.codingRateType =
                                                CODING_RATE_THIRD;
   pUlAddReconf->dedicatedTransChInfo.semistaticTFInfo.rateMatchingAttribute =
                                                190;
   pUlAddReconf->dedicatedTransChInfo.semistaticTFInfo.crcSize = 
                                                CRC_SIZE_CRC16;
                           
   /* Fill DL Common Transport Channel Info */
   pRrcConnSetup->dlCommonTransportChannelInfoPresence = VALUE_PRESENT;
   RrcDlCommonTransChInfo *pDlCmnTransChInfo = 
                              &pRrcConnSetup->dlCommonTransportChannelInfo;
   pDlCmnTransChInfo->dlParametersType = DL_PARAMETERS_SAME_AS_UL;

   /* Fill DL Add Reconfig Transport Channel Info */
   RrcDlAddReconfTransChInfoList *pDlAddReconf = 
                              &pRrcConnSetup->dlAddReconfTransChInfoList;
   pDlAddReconf->numOfDlAddReconfTransChInfoList = VALUE_PRESENT;
   pDlAddReconf->dlTransportChannelType = DL_TRANSPORT_CHANNEL_TYPE_DCH;
   pDlAddReconf->dlTransportChannelId = DCH_ID + ueIndex;
   pDlAddReconf->tfsSignallingModeType = TFS_SIGNALLING_EXPLICIT_CONFIG;
   pDlAddReconf->transportFormatSetType = TRANSPORT_FORMAT_SET_DEDICATED;
   pDlAddReconf->dedicatedTransChInfo.ttiType = 
                                   DEDICATED_TRANSPORT_CHANNEL_TFS_TTI40;
   pDlAddReconf->dedicatedTransChInfo.numOfTti40 = VALUE_PRESENT;
   pDlAddReconf->dedicatedTransChInfo.tti40.rlcSizeType = 
                                                 RLC_SIZE_OCTET_MODE_TYPE1;
   pDlAddReconf->dedicatedTransChInfo.tti40.octectModeType =
                                                 OCTET_MODE_RLC_SIZE_TYPE1;
   pDlAddReconf->dedicatedTransChInfo.tti40.sizeType1 = 16;
   pDlAddReconf->dedicatedTransChInfo.tti40.numOfTbSizeList = MAX_TB_SIZE_LIST;
   for (idx = 0; idx < MAX_TB_SIZE_LIST; idx++)
   {
      if (0 == idx)
      {
          pDlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].numberOfTransportBlocksType =
                                                  NUMBER_OF_TRANSPORT_BLOCKS_ZERO;
      }
      else
      {
          pDlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].numberOfTransportBlocksType =
                                                  NUMBER_OF_TRANSPORT_BLOCKS_ONE;
      }
      pDlAddReconf->dedicatedTransChInfo.tti40.tbSizeList[idx].transportBlockvalue = 
                                                          VALUE_NOT_PRESENT;
   }
   pDlAddReconf->dedicatedTransChInfo.tti40.logicalChannelListType = 
                                               LOGICAL_CHANNEL_LIST_ALL_SIZES;
   pDlAddReconf->dedicatedTransChInfo.tti40.logicalChannelListType = 
                                               LOGICAL_CHANNEL_LIST_ALL_SIZES;
   pDlAddReconf->dedicatedTransChInfo.semistaticTFInfo.channelCodingType =
                                               CHANNEL_CODING_TYPE_CONVOLUTIONAL;
   pDlAddReconf->dedicatedTransChInfo.semistaticTFInfo.codingRateType =
                                                CODING_RATE_THIRD;
   pDlAddReconf->dedicatedTransChInfo.semistaticTFInfo.rateMatchingAttribute =
                                                190;
   pDlAddReconf->dedicatedTransChInfo.semistaticTFInfo.crcSize = 
                                                CRC_SIZE_CRC16;
   pDlAddReconf->dchQualityTargetPresence = VALUE_PRESENT;
   pDlAddReconf->blerQualityValue = -20;

   /* Fill Max Allowed UL TX Power */
   pRrcConnSetup->maxAllowedULTXPowerPresence = VALUE_PRESENT;
   pRrcConnSetup->maxAllowedULTXPower = 33;

   /* Fill UL Channel Requirement */
   pRrcConnSetup->ulChannelRequirementPresence = VALUE_PRESENT;
   RrcUlChannelRequirement *pUlChReq = &pRrcConnSetup->ulChannelRequirement;
   pUlChReq->ulChannelRequirementType = UL_CHANNEL_REQUIREMENT_DPCH_INFO;
   pUlChReq->ulDPCHPowerControlPresence = VALUE_PRESENT;
   pUlChReq->dpcchPowerOffset = -65;
   pUlChReq->pcPreamble = VALUE_NOT_PRESENT;
   pUlChReq->srbDelay   = VALUE_NOT_PRESENT;
   pUlChReq->powerControlAlgorithmType = POWER_CONTROL_ALGORITHM1;
   pUlChReq->powerControlAlgorithmValue = VALUE_NOT_PRESENT;
   pUlChReq->scramblingCodeType = SCRAMBLING_CODE_LONG_SC;
   pUlChReq->scramblingCode = UL_SCRAMBLING_CODE + ueIndex;
   pUlChReq->spreadingFactor = SPREADING_FACTOR_SF256;
   pUlChReq->tfciExistence = VALUE_PRESENT;
   pUlChReq->puncturingLimit = SIB_TYPE5_PRACH_RACH_PUNCTURING_LIMIT_PL1;

   /* Fill DL Common Information */
   pRrcConnSetup->dlCommonInformationPresence = VALUE_PRESENT;
   RrcDlCommonInformation *pDlCmnInfo = 
                               &pRrcConnSetup->dlCommonInformation;
   pDlCmnInfo->dlDPCHInfoCommonPresence = VALUE_PRESENT;
   pDlCmnInfo->cfnHandlingType = CFN_HANDLING_MAINTAIN;
   pDlCmnInfo->powerOffsetPilotPDPDCH = VALUE_NOT_PRESENT;
   pDlCmnInfo->spreadingFactorAndPilotType = SPREADING_FACTOR_AND_PILOT_SFD256;
   pDlCmnInfo->pilotBits256Type = PILOT_BITS256_PB4;
   pDlCmnInfo->positionFixedOrFlexible = POSITION_FIXED;
   pDlCmnInfo->tfciExistence = VALUE_PRESENT;
   pDlCmnInfo->defaultDPCHOffsetValuePresence = VALUE_PRESENT;
   pDlCmnInfo->defaultDPCHOffsetValue = VALUE_NOT_PRESENT;

   /* Fill DL Information Per RL List */
   pRrcConnSetup->dlInformationPerRLListPrsence = VALUE_PRESENT;
   RrcDlInformationPerRLList *pDlInfoPerRL = 
                                &pRrcConnSetup->dlInformationPerRLList;
   pDlInfoPerRL->numOfDlInfoPerRLList = VALUE_PRESENT;
   pDlInfoPerRL->primaryScramblingCode = gUmtsConfig.psc;
   pDlInfoPerRL->dlDPCHInfoPerRLPresence = VALUE_PRESENT;
   pDlInfoPerRL->pCPICHUsageForChannelEst = PCPICH_USAGE_MAY_BE_USED;
   pDlInfoPerRL->numOfDlChannelisationCode = VALUE_PRESENT;
   pDlInfoPerRL->dpchFrameOffset = VALUE_NOT_PRESENT;
   pDlInfoPerRL->sfAndCodeNumber = SPREADING_FACTOR_SF256;
   // TBD: Need to use different value for different UEs
   pDlInfoPerRL->sfValue = 32 + ueIndex;
   DEBUGMSG(("==== sfindex(%d) used for ueIdx(%d)\n",pDlInfoPerRL->sfValue, ueIndex));
   pDlInfoPerRL->tpcCombinationIndex = VALUE_NOT_PRESENT;
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcConnectionSetup
 *
 * Description : This procedure fills RRC Connection Setup & send it to RLC 
 *               Layer.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcConnectionSetup (unsigned int ueIndex, unsigned char isL2ConfigRequired)
{
   unsigned char *pRrcConnSetup   = NULL;
   RrcDataReq    *pRlcDataReq     = NULL;
   RrcDataReq    *pRlcDataReq_2   = NULL;
   unsigned int  *pTimerUeId      = NULL;
   unsigned int  *pL2UeId         = NULL;
   unsigned int   rrcConnSetupLen = 0;
   unsigned int   retCode = RRC_SUCCESS;

   RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

   /* Fill RRC Connection Setup Common Data */
   fillRrcConnectionSetup (ueIndex);

   /* Start the Timer for RRC Connection Setup */
   pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
   *pTimerUeId = ueIndex;

   timerStart(&pUeSpecificData->ueTimer,
              RRC_CONNECTION_SETUP_TIMEOUT,
              0,
              handleRrcConnectionSetupTimeout,
              pTimerUeId);

   /* Change the UE State */
   pUeSpecificData->ueState = UE_WAIT_FOR_RRC_CON_SETUP_COMPLETE;

   /* Increament RRC Connection Setup Count */
   pUeSpecificData->ueRrcConnSetupCount++;

#if 0
   /* Initialize ASN Context */
   if (ASN_SUCCESS == initializeASNContext (fileName))
#else
   /* Initialize ASN Context */
   if (ASN_SUCCESS == initializeASNContext ())
#endif
   {
       /* Fill & Encode RRC Connection Setup */
       if (ASN_SUCCESS == fillAndEncodeRrcConnectionSetup (&pUeSpecificData->ueInitialIdentity,
                                                           &pUeSpecificData->ueRrcConnSetup,
                                                           &pRrcConnSetup,
                                                           &rrcConnSetupLen))
       {
           /* Fill RLC Data Indication */
           pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
           pRlcDataReq->cellOrUeId.choice = RRC_CELL_ID;
           pRlcDataReq->cellOrUeId.u.cellId = gUmtsConfig.cellId;
           pRlcDataReq->logicalChType = LOG_CH_CCCH_E;
           pRlcDataReq->logicalChId   = 1;             //Logical Channel Id 1 is used for CCCH message
           pRlcDataReq->rrcPduLen = rrcConnSetupLen;
           pRlcDataReq->rrcPdu = pRrcConnSetup;

           DEBUGMSG(("RRC Connection Setup is sent\n"));
           /* Post DL CCCH Message to L2 DL DATA Task */
           SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                          L2_DL_CCCH_DATA_REQUEST_E, pRlcDataReq);

           pRlcDataReq_2 = (RrcDataReq *)malloc(sizeof(RrcDataReq));
           pRlcDataReq_2->cellOrUeId.choice   = RRC_CELL_ID;
           pRlcDataReq_2->cellOrUeId.u.cellId = 0;
           pRlcDataReq_2->logicalChType       = LOG_CH_CCCH_E;
           pRlcDataReq_2->rrcPduLen           = rrcConnSetupLen;
           pRlcDataReq_2->rrcPdu = (U8 *)malloc(rrcConnSetupLen + 5);
           memcpy(pRlcDataReq_2->rrcPdu, pRlcDataReq->rrcPdu, rrcConnSetupLen);
           SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                          L2_DL_CCCH_DATA_REQUEST_E, pRlcDataReq_2);

           if (1 == isL2ConfigRequired)
           {
               /* Post L2 Config Request to L2 DL DATA Task */
               pL2UeId = (unsigned int *)malloc(sizeof(ueIndex));
               *pL2UeId = ueIndex; 
               SendMsgToTask (APP_RRC_TASK_E, L2_CFG_TASK_E,
                              L2_CONFIG_REQUEST_E, pL2UeId);
               DEBUGMSG(("L2 Config Request is sent\n"));
           }

           /* Increament the RRC Transaction Id */
           ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                    (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4; 
       }
       else
       {
           DEBUG1(("Encoding of RRC Connection Setup failed\n"));
           retCode = RRC_FAILURE;
       }

       /* Clean ASN Context */
       cleanASNContext ();
   }
   else
   {
       DEBUG1(("Initialization of ASN Context failed for RRC Connection Setup\n"));
       retCode = RRC_FAILURE;
   }
}

/*
 * ############################################################################
 * Method Name : buildAndSendRrcConnectionReject
 * 
 * Description : This procedure fills RRC Connection Reject & send it to RLC 
 *               Layer.
 * 
 * Parameters  : RRC Initial UE Identity
 * 
 * Returns     : None
 * ############################################################################
 */
void buildAndSendRrcConnectionReject (RrcConnectionRejectData rrcConnRejData)
{
   unsigned char *pRrcConnRej   = NULL;
   RrcDataReq    *pRlcDataReq     = NULL;
   unsigned int   rrcConnRejLen = 0;
   unsigned int   retCode = RRC_SUCCESS;


   if (ASN_SUCCESS == initializeASNContext ())
   {
       /* Fill & Encode RRC Connection Setup */
       if (ASN_SUCCESS == fillAndEncodeRrcConnectionReject (&rrcConnRejData,
                                                            &pRrcConnRej,
                                                            &rrcConnRejLen))
       {
           /* Fill RLC Data Indication */
           pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
           pRlcDataReq->cellOrUeId.choice = RRC_CELL_ID;
           pRlcDataReq->cellOrUeId.u.cellId = gUmtsConfig.cellId;
           pRlcDataReq->logicalChType = LOG_CH_CCCH_E;
           pRlcDataReq->logicalChId   = 1;             //Logical Channel Id 1 is used for CCCH message
           pRlcDataReq->rrcPduLen = rrcConnRejLen;
           pRlcDataReq->rrcPdu = pRrcConnRej;

           DEBUGMSG(("RRC Connection Reject is sent\n"));
           /* Post DL CCCH Message to L2 DL DATA Task */
           SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                          L2_DL_CCCH_DATA_REQUEST_E, pRlcDataReq);
       }
       else
       {
           DEBUG1(("Encoding of RRC Connection Reject failed\n"));
           retCode = RRC_FAILURE;
       }

       /* Clean ASN Context */
       cleanASNContext ();
   }
   else
   {
       DEBUG1(("Initialization of ASN Context failed for RRC Connection Reject\n"));
       retCode = RRC_FAILURE;
   }
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitRlSetupRespState
 *
 * Description : This procedure handles all the events received in Wait RL
 *               Setup Response state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitRlSetupRespState (unsigned int ueIndex, 
                                          RrcUeEvent ueEvent)
{
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_RL_SETUP_TIMEOUT:
           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break;

        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
           /* Stop the RL Setup Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break; 

        case UE_RL_SETUP_SUCCESS:
           /* Change the UE State */
           pUeSpecificData->ueState = UE_WAIT_FOR_RRC_CON_SETUP_COMPLETE;
                                                 
           /* Stop the RL Setup Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           /* Build And Send RRC Connection Setup */
           buildAndSendRrcConnectionSetup (ueIndex, 1);
           break;

        default:
           DEBUG2((" UE event(%ld) is not handled in UE_WAIT_FOR_RL_SETUP_RESP state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitRrcConnSetupCompState
 *
 * Description : This procedure handles all the events received in Wait RRC
 *               Connection Setup Complete state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitRrcConnSetupCompState (unsigned int ueIndex, 
                                               RrcUeEvent   ueEvent)
{
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_RRC_CON_SETUP_TIMEOUT:
           if (pUeSpecificData->ueRrcConnSetupCount < MAX_RETRANSMIT_RRC_CON_SETUP)
           {
               /* Build And Send RRC Connection Setup */
               buildAndSendRrcConnectionSetup (ueIndex, 0);
           }
           else
           {
               /* Send RL Delete Req to L1 */
               sendRadioLinkpDeleteReqToL1 (ueIndex);
           }
           break;

        case UE_RRC_CON_SETUP_COMP_RCVD:
           /* Change the UE State */
           pUeSpecificData->ueState = UE_WAIT_FOR_IDT;
 
           /* Stop the RRC Connection Setup Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }
#if 1
           trigPerUeSingleMeas(ueIndex);
#endif
           /* Check that IDT is received before RRC Connection Setup Complete */
           if (1 == pUeSpecificData->isIDTSaved)
           {
               /* Copy the UL DCCH Message with Saved IDT */
               memcpy ((RrcUlDCCHData *)&pUeSpecificData->ulDcchData, 
                       (RrcUlDCCHData *)&pUeSpecificData->savedIDT, 
                       sizeof(RrcUlDCCHData));

               /* Handle IDT Message */
               handleUeEvents (ueIndex, UE_IDT_RCVD);
           }

           break;

        case UE_IDT_RCVD:
           /* Save the IDT received before RRC Connection Setup Complete */
           pUeSpecificData->isIDTSaved = 1;

           /* Save the UL DCCH Message */
           memcpy ((RrcUlDCCHData *)&pUeSpecificData->savedIDT, 
                   (RrcUlDCCHData *)&pUeSpecificData->ulDcchData, 
                   sizeof(RrcUlDCCHData));
           break;

        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
           /* Stop the RRC Connection Setup Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_RRC_CON_SETUP_COMPLETE state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : checkImsiReceivedInIdt
 *
 * Description : This procedure decodes the NAS message received in IDT & 
 *               stores IMSI if it is present.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  
 *        0       If IMSI not present in IDT
 *        1       If IMSI present in IDT
 *############################################################################
 */
unsigned char checkImsiReceivedInIdt (unsigned int ueIndex)
{
   unsigned char retCode    = 0;
   unsigned int  imsiIndex  = 0;
   unsigned int  imsiLen    = 0;
   unsigned int  imsiCount  = 0;
   unsigned int  maxImsiLen = 0;

   /* Get UE Specific UL DCCH Data */
   RrcUlDCCHData *pUlDCCH = &ueControlBlock.ueSpecificData[ueIndex].ulDcchData;
   
   /* Decode NAS message received */
   switch (pUlDCCH->pNasMsg[0] & 0x0F)
   {
       /* Mobility Management Messages */
       case 0x05:
       {
          switch (pUlDCCH->pNasMsg[1] & 0x0F)
          {
              case 0x01:
                 DEBUGMSG(("NAS : Received IMSI DETACH from UE Id(%ld)\n", ueIndex));
                 if ((pUlDCCH->pNasMsg[4] & 0x07) == 0x01)
                 {
                     imsiIndex = 4;
                     retCode   = 1;
                 }
                 break;

              case 0x04:
                 DEBUGMSG(("NAS : Received CM Service Request from UE Id(%ld)\n", ueIndex));
                 if ((pUlDCCH->pNasMsg[8] & 0x07) == 0x01)
                 {
                     imsiIndex = 8;
                     retCode   = 1;
                 }
                 pUlDCCH->pNasMsg[2] = pUlDCCH->pNasMsg[2] | 0x70;
                 break;
  
              case 0x08: 
                 DEBUGMSG(("NAS : Received Location Update Request from UE Id(%ld)\n", ueIndex));
				 ueControlBlock.ueSpecificData[ueIndex].locationUpdateReqReceived = 1;
//                 ueControlBlock.ueSpecificData[ueIndex].nasMsgType = NAS_LOCATION_UPDATE_REQ;
                 if ((pUlDCCH->pNasMsg[10] & 0x07) == 0x01)
                 {
                     imsiIndex = 10;
                     retCode   = 1;
                 }
                 pUlDCCH->pNasMsg[2] = pUlDCCH->pNasMsg[2] | 0x70;
                 break;
             
              default:
                 break;
          }
       }
       break;

       case 0x06:
       {
           if ((pUlDCCH->pNasMsg[1] & 0xFF) == 0x27)
           {
               DEBUGMSG(("NAS : Received Paging Response from UE Id(%ld)\n", ueIndex));
			   ueControlBlock.ueSpecificData[ueIndex].locationUpdateReqReceived = 1;
//               ueControlBlock.ueSpecificData[ueIndex].nasMsgType = NAS_LOCATION_UPDATE_REQ;
               if((pUlDCCH->pNasMsg[8] & 0x07) == 0x01)
               {
                   imsiIndex = 8;
                   retCode   = 1;
               }
               pUlDCCH->pNasMsg[2] = pUlDCCH->pNasMsg[2] | 0x07;
           }
       }
       break;

       /* GPRS Mobility Management Messages */
       case 0x08:
       {
          switch (pUlDCCH->pNasMsg[1] & 0x0F)
          {
              case 0x01:
                 DEBUGMSG(("NAS : Received GMM Attach Request from UE Id(%ld)\n", ueIndex));
				 ueControlBlock.ueSpecificData[ueIndex].attachReqReceived = 1;
//                 ueControlBlock.ueSpecificData[ueIndex].nasMsgType = NAS_GMM_ATTACH_REQUEST;
                 unsigned int len = 0;
                 len = pUlDCCH->pNasMsg[2] & 0x0F;
                 if ((pUlDCCH->pNasMsg[2 + len + 5] & 0x07) == 0x01)
                 {
                     imsiIndex = 2 + len + 5;
                     retCode   = 1;
                 }
                 pUlDCCH->pNasMsg[2 + len + 1] = pUlDCCH->pNasMsg[2 + len + 1] & 0x70;
                 break;

              case 0x05:
                 DEBUGMSG(("NAS : Received GMM Detach Request from UE Id(%ld)\n", ueIndex));
                 if ((pUlDCCH->pNasMsg[5] & 0x07) == 0x01)
                 {
                     imsiIndex = 5;
                     retCode   = 1;
                 }
                 break;

              case 0x08:
                 DEBUGMSG(("NAS : Received GMM Routing Area Update Request from UE Id(%ld)\n", ueIndex));
				 ueControlBlock.ueSpecificData[ueIndex].routingAreaUpdateReceived = 1;
//                 ueControlBlock.ueSpecificData[ueIndex].nasMsgType = NAS_GMM_ROUTING_AREA_UPDATE;
                 pUlDCCH->pNasMsg[2] = pUlDCCH->pNasMsg[2] | 0x70;
                 break;

              case 0x0C:
                 DEBUGMSG(("NAS : Received GMM Service Request from UE Id(%ld)\n", ueIndex));
                 if ((pUlDCCH->pNasMsg[4] & 0x07) == 0x01)
                 {
                     imsiIndex = 4;
                     retCode   = 1;
                 }
                 pUlDCCH->pNasMsg[2] = pUlDCCH->pNasMsg[2] | 0x07;
                 break;

              default:
                 break;
          }
       }
       break;

       default:
          DEBUG3(("NAS : Received NAS Message Byte0(%ld) Byte1(%ld)\n", 
                 pUlDCCH->pNasMsg[0], pUlDCCH->pNasMsg[1]));
          break;
   }

   /* Store the IMSI if it is present in IDT */
   if (1 == retCode)
   {
       /* Get the IMSI Length */
       imsiLen = pUlDCCH->pNasMsg[imsiIndex -1];
       maxImsiLen = (((imsiLen * 2) -1) < MAX_IMSI_LENGTH) ? (imsiLen * 2) : MAX_IMSI_LENGTH;

       /* 
        * Store the IMSI received in IDT Message.
        */
       RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];
 
       pUeSpecificData->imsiLen = maxImsiLen;

       for(imsiCount = 0; imsiCount < pUeSpecificData->imsiLen; imsiCount++)
       {
           if (0 == imsiCount % 2)
           {
               /* 
                * Store in ASCII Format. BCD to ASCII Convertion is done 
                * by adding '0'.
                */ 
               pUeSpecificData->imsi[imsiCount] =
                               ((pUlDCCH->pNasMsg[imsiIndex] & 0xF0) >> 4) + '0';
               imsiIndex++;
           }
           else
           {
               /* 
                * Store in ASCII Format. BCD to ASCII Convertion is done 
                * by adding '0'.
                */ 
               pUeSpecificData->imsi[imsiCount] =
                                      (pUlDCCH->pNasMsg[imsiIndex] & 0x0F) + '0'; 
           }
       }
   }

   /* Free the memory allocated for NAS Message */
   if (NULL != pUlDCCH->pNasMsg)
   {
       free (pUlDCCH->pNasMsg);
       pUlDCCH->pNasMsg = NULL;
   }

   return (retCode);
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcPagingType1
 *
 * Description : This procedure builds the Paging Type1 message and posts RRC
 *               PDU to CL Send Task.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcPagingType1 (unsigned int ueIndex) 
{
    unsigned int       rrcPagingType1Len = 0;
    unsigned char     *pRrcPagingType1EncodedData = NULL;
    unsigned int       idx = 0;
    RrcPagingType1Data rrcPagingType1;
    RrcPcchMsg        *pRrcPcchMsg = NULL;
    char imsiValue[MAX_IMSI_LENGTH + 1] = {0};

    memset (&rrcPagingType1, 0 , sizeof (rrcPagingType1));

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Get IMSI Details */
    strncpy (imsiValue, pUeSpecificData->imsi,
             pUeSpecificData->imsiLen);
    imsiValue[pUeSpecificData->imsiLen] = '\0';

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill Paging Type1 Information */
        rrcPagingType1.pagingCause  = PAGING_CAUSE_TERMINATING_HIGH_PRIORITY_SIGNALLING;
        rrcPagingType1.cnDomainType = pUeSpecificData->ulDcchData.cnDomainType;
        rrcPagingType1.imsiLength   = pUeSpecificData->imsiLen;
        for (idx = 0; idx < pUeSpecificData->imsiLen; idx++)
        {
            /* Convert ASCII to Integer for IMSI value */
            rrcPagingType1.imsi[idx] = pUeSpecificData->imsi[idx] - '0';
        }

        if (ASN_SUCCESS == fillAndEncodePagingType1 (&rrcPagingType1,
                                                     &pRrcPagingType1EncodedData,
                                                     &rrcPagingType1Len))
        {
            pRrcPcchMsg = (RrcPcchMsg *)malloc(sizeof(RrcPcchMsg)); 
            pRrcPcchMsg->sfn       = getPagingSfn (imsiValue, pUeSpecificData->imsiLen);
            pRrcPcchMsg->rrcPduLen = rrcPagingType1Len;
            pRrcPcchMsg->rrcPdu    = pRrcPagingType1EncodedData;
            pRrcPcchMsg->count     = 0;
			pRrcPcchMsg->isInitialIMSIPaging = FALSE;
            strncpy (pRrcPcchMsg->imsi, pUeSpecificData->imsi, pUeSpecificData->imsiLen);
            pRrcPcchMsg->imsi[pUeSpecificData->imsiLen] = '\0';

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_PCCH_DATA_REQUEST_E, pRrcPcchMsg);

            DEBUGMSG(("RRC Paging Type1 is sent to L2\n"));
        } 
        else
        {
            DEBUG1((" Encoding RRC Paging Type1 is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Paging Type1 is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcConnectionRelease
 *
 * Description : This procedure builds the RRC Connection Release Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   laterThanR3  RRC Connection Release Type
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcConnectionRelease (unsigned int  ueIndex, 
                                       unsigned char laterThanR3)
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcConnRelLen = 0;
    unsigned char   *pRrcConnRelEncodedData = NULL;
    unsigned int    *pTimerUeId = NULL;
    unsigned int     idx = 0;
    RrcConnectionReleaseData rrcConnRel;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change the UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_RRC_CON_REL_COMPLETE;

    /* Start the Timer for RRC Connection Release */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               RRC_CONNECTION_RELEASE_TIMEOUT,
               0,
               handleRrcConnectionReleaseTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcConnRel.laterThanR3      = laterThanR3;
        rrcConnRel.rrcTransactionId = pUeSpecificData->rrcTransactionId;
        rrcConnRel.n308Presence     = VALUE_PRESENT;
        rrcConnRel.n308             = VALUE_PRESENT;

        if (0 == laterThanR3)
        {
            rrcConnRel.releaseCause = RELEASE_CAUSE_NORMAL_EVENT;
        }
        else
        {
            rrcConnRel.releaseCause = RELEASE_CAUSE_CONGESTION;
       
            if(pUeSpecificData->isBlackListed == 1)
            {
                if (FXL_UMTS_REDIRECTION_TO_2G == gUmtsConfig.redirectionType)
                {
                    rrcConnRel.redirectionType  = REDIRECTION_INFO_INTER_RAT;
                    rrcConnRel.numOfGsmCell     = 1;
                    /* Fill GSM ARFCN */
                    rrcConnRel.gsmInfo[0].arfcn = 
                              gUmtsConfig.redirectionInfo.gsmRedirectinfo.startingarfcn;
                    if (DCS1800 == gUmtsConfig.redirectionInfo.gsmRedirectinfo.bandIndicator)
                    {
                        rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1800;
                    }
                    else
                    {
                        rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1900;
                    }

                    /* Fill GSM BSIC Presence */
                    rrcConnRel.gsmInfo[idx].bsicPresence = 0;

                    /* Fill GSM BSIC NCC */
                    rrcConnRel.gsmInfo[idx].bsicNcc = 0;

                    /* Fill GSM BSIC BCC */
                    rrcConnRel.gsmInfo[idx].bsicBcc = 0;
                }
                else if (FXL_UMTS_REDIRECTION_TO_3G == gUmtsConfig.redirectionType)
                {
                    rrcConnRel.redirectionType = REDIRECTION_INFO_INTER_FREQUENCY;
                    rrcConnRel.dlUarfcn        = gUmtsConfig.redirectionInfo.dlUarfcn;
                }
                else
                {
                    rrcConnRel.laterThanR3  = 0;
                    rrcConnRel.releaseCause = RELEASE_CAUSE_NORMAL_EVENT;
                }
            }
            else
            {
                if (FXL_UMTS_REDIRECTION_TO_2G == gUmtsConfig.wlUserRedirectionType)
                {
                    rrcConnRel.redirectionType  = REDIRECTION_INFO_INTER_RAT;
                    rrcConnRel.numOfGsmCell     = 1;
                    /* Fill GSM ARFCN */
                    rrcConnRel.gsmInfo[0].arfcn = 
                              gUmtsConfig.wlUserRedirectionInfo.gsmRedirectinfo.startingarfcn;
                    if (DCS1800 == gUmtsConfig.wlUserRedirectionInfo.gsmRedirectinfo.bandIndicator)
                    {
                        rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1800;
                    }
                    else
                    {
                        rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1900;
                    }

                    /* Fill GSM BSIC Presence */
                    rrcConnRel.gsmInfo[idx].bsicPresence = 0;

                    /* Fill GSM BSIC NCC */
                    rrcConnRel.gsmInfo[idx].bsicNcc = 0;

                    /* Fill GSM BSIC BCC */
                    rrcConnRel.gsmInfo[idx].bsicBcc = 0;
                }
                else if (FXL_UMTS_REDIRECTION_TO_3G == gUmtsConfig.wlUserRedirectionType)
                {
                    rrcConnRel.redirectionType = REDIRECTION_INFO_INTER_FREQUENCY;
                    rrcConnRel.dlUarfcn        = gUmtsConfig.wlUserRedirectionInfo.dlUarfcn;
                }
                else
                {
                    /* White Listed UE */
                    rrcConnRel.redirectionType = REDIRECTION_INFO_INTER_FREQUENCY;
                    rrcConnRel.dlUarfcn        = gUmtsConfig.redirectDlUarfcn;
                }
            }
        }

        if (ASN_SUCCESS == fillAndEncodeRrcConnectionRelease (&rrcConnRel,
                                                              &pRrcConnRelEncodedData,
                                                              &rrcConnRelLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcConnRelLen;
            pRlcDataReq->rrcPdu            = pRrcConnRelEncodedData;

            DEBUGMSG(("RRC Connection Release is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                          (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        } 
        else
        {
            DEBUG1((" Encoding RRC Connection Release is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Connection Release is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendDirectTransferNasReject
 *
 * Description : This procedure builds the NAS Reject Message, fills DL DCCH 
 *               message and posts RRC PDU to RLC Layer.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendDirectTransferNasReject (unsigned int ueIndex,unsigned int wlRejectCause, 
                                                 NasMsgType nasMsgType)
{
    unsigned char  dlDTMsg[4] = {0};
    unsigned char *pDLDirectTransfer   = NULL;
    RrcDataReq    *pRlcDataReq         = NULL;
    unsigned int   dlDirectTransferLen = 0;
    unsigned int  *pTimerUeId = NULL;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_IDENTITY_RESP;

    if (NAS_LOCATION_UPDATE_REQ == nasMsgType)
    {
        dlDTMsg[0] = 0x05;
        dlDTMsg[1] = 0x04;
        dlDTMsg[2] = 0x0F; //Reject Cause 15 = No Suitable Cell in Location Area
    }
    else if (NAS_GMM_ATTACH_REQUEST == nasMsgType)
    {
        dlDTMsg[0] = 0x08;
        dlDTMsg[1] = 0x04;
        dlDTMsg[2] = 0x0F; //Reject Cause 15 = No Suitable Cell in Location Area
    }
    else if (NAS_GMM_ROUTING_AREA_UPDATE == nasMsgType)
    {
         dlDTMsg[0] = 0x08;
         dlDTMsg[1] = 0x0B;
         dlDTMsg[2] = 0x0F; //Reject Cause 15 = No Suitable Cell in Location Area
    }
    else
    {
         DEBUGMSG(("NAS : IDT Reject dafault case\n"));
         dlDTMsg[0] = 0x05;
         dlDTMsg[1] = 0x04;
         dlDTMsg[2] = 0x0F; //Reject Cause 15 = No Suitable Cell in Location Area
    }

    if(0 != wlRejectCause)
    { //Reject Cause Control Configured. Overwrite reject cause
       dlDTMsg[2] = wlRejectCause;
       DEBUGMSG(("NAS : Reject Cause Control Configured with reject cause(%d) \n", dlDTMsg[2]));
    }


    /* Start the Timer for NAS Reject to be sent */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               100,
               0,
               handleNasRejectTimeout,
               pTimerUeId);


    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        if (NAS_GMM_ROUTING_AREA_UPDATE == nasMsgType)
        {
            if (ASN_SUCCESS == fillAndEncodeDLDirectTransfer (pUeSpecificData->ulDcchData.cnDomainType,
                                                              pUeSpecificData->rrcTransactionId,
                                                              4,
                                                              dlDTMsg,
                                                              &pDLDirectTransfer,
                                                              &dlDirectTransferLen))
            {
                /* Fill RLC Data Indication */
                pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
                pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
                pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
                pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
                pRlcDataReq->logicalChId       = 3;     // NAS message will be sent on logical channel id 3
                pRlcDataReq->rrcPduLen         = dlDirectTransferLen;
                pRlcDataReq->rrcPdu            = pDLDirectTransfer;

                DEBUGMSG((" DL Direct Transfer NAS Reject is sent\n"));

                /* Post DL CCCH Message to L2 DL DATA Task */
                SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                               L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

                /* Increament RRC Transaction Id */
                ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                                     (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
            }
            else
            {
                DEBUG1(("Encoding DL Direct Transfer NAS Reject is failed\n"));
            }
        }
        else
        {
            if (ASN_SUCCESS == fillAndEncodeDLDirectTransfer (pUeSpecificData->ulDcchData.cnDomainType,
                                                              pUeSpecificData->rrcTransactionId,
                                                              3,
                                                              dlDTMsg,
                                                              &pDLDirectTransfer,
                                                              &dlDirectTransferLen))
            {
                /* Fill RLC Data Indication */
                pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
                pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
                pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
                pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
                pRlcDataReq->logicalChId       = 3;     // NAS message will be sent on logical channel id 3
                pRlcDataReq->rrcPduLen         = dlDirectTransferLen;
                pRlcDataReq->rrcPdu            = pDLDirectTransfer;

                DEBUGMSG((" DL Direct Transfer NAS Reject is sent\n"));

                /* Post DL CCCH Message to L2 DL DATA Task */
                SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                               L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

                /* Increament RRC Transaction Id */
                ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                                 (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
            }
            else
            {
                DEBUG1(("Encoding DL Direct Transfer NAS Reject is failed\n"));
            }
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for DL Direct Transfer NAS Reject is failed\n"));
    }
}

/*
 *############################################################################
 * Method Name : handleIdentityRequestTimeout
 *
 * Description : This procedure handles RRC Identity Request Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleIdentityRequestTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("RRC Identity Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_IDENTITY_REQ_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : handleRrcConnectionReleaseTimeout
 *
 * Description : This procedure handles RRC Connection Release Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleRrcConnectionReleaseTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("RRC Connection Release Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_CONNECTION_REL_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : buildAndSendDirectTransferIdentityReqIMEI
 *
 * Description : This procedure builds the NAS Identity Request for IMEI, fills
 *               DL DCCH message and posts RRC PDU to RLC Layer.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendDirectTransferIdentityReqIMEI (unsigned int ueIndex)
{
    unsigned char  dlDTMsg[3] = {0};
    unsigned char *pDLDirectTransfer   = NULL;
    RrcDataReq    *pRlcDataReq         = NULL;
    unsigned int  *pTimerUeId          = NULL;
    unsigned int   dlDirectTransferLen = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (pUeSpecificData->ulDcchData.cnDomainType)
    {
        case CN_DOMAIN_IDENTITY_CS:
           dlDTMsg[0] = 0x05;
           dlDTMsg[1] = 0x18;
           dlDTMsg[2] = 0x02;
           break;

        case CN_DOMAIN_IDENTITY_PS:
           dlDTMsg[0] = 0x08;
           dlDTMsg[1] = 0x15;
           dlDTMsg[2] = 0x02;
           break;

        default:
           break; 
    }

    /* Change UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_IDENTITY_RESP;

    /* Start the Timer for Identity Request */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               IDENTITY_REQUEST_TIMEOUT,
               0,
               handleIdentityRequestTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        if (ASN_SUCCESS == fillAndEncodeDLDirectTransfer (pUeSpecificData->ulDcchData.cnDomainType,
                                                          pUeSpecificData->rrcTransactionId,
                                                          3,
                                                          dlDTMsg,
                                                          &pDLDirectTransfer,
                                                          &dlDirectTransferLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 3;                   // NAS Message will be sent on logical channel id 3
            pRlcDataReq->rrcPduLen         = dlDirectTransferLen;
            pRlcDataReq->rrcPdu            = pDLDirectTransfer;

            DEBUGMSG(("DL Direct Transfer Identity Request for IMEI is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                          (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;

            /* Increament IMEI Request Count */
            pUeSpecificData->ueIMEIReqCount++;
        }
        else
        {
            DEBUG1(("Encoding DL Direct Transfer Identity Request for IMEI is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for DL Direct Transfer Indentity Request for IMEI is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendDirectTransferIdentityReqTMSI
 *
 * Description : This procedure builds the NAS Identity Request for TMSI, fills
 *               DL DCCH message and posts RRC PDU to RLC Layer.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendDirectTransferIdentityReqTMSI (unsigned int ueIndex)
{
    unsigned char  dlDTMsg[3] = {0};
    unsigned char *pDLDirectTransfer   = NULL;
    RrcDataReq    *pRlcDataReq         = NULL;
    unsigned int  *pTimerUeId          = NULL;
    unsigned int   dlDirectTransferLen = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (pUeSpecificData->ulDcchData.cnDomainType)
    {
        case CN_DOMAIN_IDENTITY_CS:
           dlDTMsg[0] = 0x05;
           dlDTMsg[1] = 0x18;
           dlDTMsg[2] = 0x04;
           break;

        case CN_DOMAIN_IDENTITY_PS:
           dlDTMsg[0] = 0x08;
           dlDTMsg[1] = 0x15;
           dlDTMsg[2] = 0x04;
           break;

        default:
           break; 
    }

    /* Change UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_IDENTITY_RESP;

    /* Start the Timer for Identity Request */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               IDENTITY_REQUEST_TIMEOUT,
               0,
               handleIdentityRequestTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        if (ASN_SUCCESS == fillAndEncodeDLDirectTransfer (pUeSpecificData->ulDcchData.cnDomainType,
                                                          pUeSpecificData->rrcTransactionId,
                                                          3,
                                                          dlDTMsg,
                                                          &pDLDirectTransfer,
                                                          &dlDirectTransferLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 3;     // NAS Message will be sent on logical channel id 3
            pRlcDataReq->rrcPduLen         = dlDirectTransferLen;
            pRlcDataReq->rrcPdu            = pDLDirectTransfer;

            DEBUGMSG(("DL Direct Transfer Identity Request for TMSI is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                               (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;

            /* Increament TMSI Request Count */
            pUeSpecificData->ueTMSIReqCount++;
        }
        else
        {
            DEBUG1(("Encoding DL Direct Transfer Identity Request for TMSI is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for DL Direct Transfer Indentity Request for TMSI is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendDirectTransferIdentityReqIMSI
 *
 * Description : This procedure builds the NAS Identity Request for IMSI, fills
 *               DL DCCH message and posts RRC PDU to RLC Layer.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendDirectTransferIdentityReqIMSI (unsigned int ueIndex)
{
    unsigned char  dlDTMsg[3] = {0};
    unsigned char *pDLDirectTransfer   = NULL;
    RrcDataReq    *pRlcDataReq         = NULL;
    unsigned int  *pTimerUeId          = NULL;
    unsigned int   dlDirectTransferLen = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (pUeSpecificData->ulDcchData.cnDomainType)
    {
        case CN_DOMAIN_IDENTITY_CS:
           dlDTMsg[0] = 0x05;
           dlDTMsg[1] = 0x18;
           dlDTMsg[2] = 0x01;
           break;

        case CN_DOMAIN_IDENTITY_PS:
           dlDTMsg[0] = 0x08;
           dlDTMsg[1] = 0x15;
           dlDTMsg[2] = 0x01;
           break;

        default:
           break; 
    }

    /* Change UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_IDENTITY_RESP;

    /* Start the Timer for Identity Request */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               IDENTITY_REQUEST_TIMEOUT,
               0,
               handleIdentityRequestTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        if (ASN_SUCCESS == fillAndEncodeDLDirectTransfer (pUeSpecificData->ulDcchData.cnDomainType,
                                                          pUeSpecificData->rrcTransactionId,
                                                          3,
                                                          dlDTMsg,
                                                          &pDLDirectTransfer,
                                                          &dlDirectTransferLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 3;                   // NAS Message will be sent on logical channel id 3
            pRlcDataReq->rrcPduLen         = dlDirectTransferLen;
            pRlcDataReq->rrcPdu            = pDLDirectTransfer;

            DEBUGMSG(("RRC : DL Direct Transfer Identity Request is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                          (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;

            /* Increament IMEI Request Count */
            pUeSpecificData->ueIMEIReqCount++;
        }
        else
        {
            DEBUG1(("Encoding DL Direct Transfer Identity Request is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for DL Direct Transfer Indentity Request is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : isIMSIWhiteListed
 *
 * Description : This procedure checks if IMSI is White Listed in UE Control
 *               Block or not.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  
 *   0            Black Listed
 *   1            White Listed
 *############################################################################
 */
#if 0
unsigned int isIMSIWhiteListed (unsigned int ueIndex)
{
   unsigned int retCode = 0;
   unsigned int idx = 0;
   
   /* Get IMSI Length */
   unsigned int imsiLen = 
              ueControlBlock.ueSpecificData[ueIndex].imsiLen;

   if (0 != imsiLen)
   {
       for (idx = 0; idx < ueControlBlock.numOfWhiteListedImsi; idx++)
       {
           if (0 == strncmp(ueControlBlock.whiteListedImsi[idx],
                            ueControlBlock.ueSpecificData[ueIndex].imsi, 
                            imsiLen))
           {
               /* Black Listed IMSI */
               retCode = 1;
               break;
           }
       }
   }

   /* Get IMEI Length */
   unsigned int imeiLen = 
              ueControlBlock.ueSpecificData[ueIndex].imeiLen;

   if (0 != imeiLen)
   {
      for (idx = 0; idx < ueControlBlock.numOfWhiteListedImei; idx++)
      {
          if (0 == strncmp(ueControlBlock.whiteListedImei[idx],
                           ueControlBlock.ueSpecificData[ueIndex].imei, 
                           imeiLen))
          {
              /* Black Listed IMEI */
              retCode = 1;
              break;
          }
      }
   }

   return (retCode);
}
#endif

/*
 *############################################################################
 * Method Name : isUeBlackListed
 *
 * Description : This procedure checks if IMSI is Black Listed or WhiteListed
 *               in UE Control Block or not.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  fxLListType_
 *   0            FXL_WHITEUSERSLIST - White Listed
 *   1            FXL_BLACKUSERSLIST - Black Listed
 *############################################################################
 */
unsigned int isUeBlackListed(unsigned int ueIndex, unsigned int *wlRejectCause)
{
   unsigned int retValue = 0;
   unsigned int idx      = 0;
   
   unsigned int imsiLen = ueControlBlock.ueSpecificData[ueIndex].imsiLen;
   unsigned int imeiLen = ueControlBlock.ueSpecificData[ueIndex].imeiLen;
   
   unsigned char *sharedMemPtr   = getAccessToIdList();
   UeIdentityConfig    *configId;
   UmtsConfigList *configUeIdList = (UmtsConfigList *)sharedMemPtr;
   configId = sharedMemPtr + sizeof(char)*4;

   // Default behaviour is whitelisting with no UE specific reject cause
   *wlRejectCause = (0 == gUmtsConfig.wlRejectCauseCfgEnable) ? 0 : gUmtsConfig.wlRejectCause;

   retValue       = configUeIdList->defaultConfig;
   DEBUGMSG(("Identity List default config = (%d)\n", retValue));
  
   for(idx = 0; idx < configUeIdList->numbOfIds; idx++)
   {
      if((configId[idx].idType == FXL_IDENTITY_IMSI) && (0 != imsiLen))
      {
         if (0 == strncmp(configId[idx].identity,
                          ueControlBlock.ueSpecificData[ueIndex].imsi, 
                          imsiLen))
          {
             retValue = configId[idx].listType;
             DEBUG3(("===== IMSI WL/BL retValue(%d), listType(%d)====\n",retValue, configId[idx].listType));
             if(configId[idx].rejectCauseIfWhitelist != 0)
             {
                *wlRejectCause = configId[idx].rejectCauseIfWhitelist;
             }
             break;  //UE found break loop and return.
          }
      }
      else if((configId[idx].idType == FXL_IDENTITY_IMEI) && (0 != imeiLen))
      {
         if (0 == strncmp(configId[idx].identity,
                          ueControlBlock.ueSpecificData[ueIndex].imei, 
                          imeiLen))
          {
             retValue = configId[idx].listType;
             DEBUG3(("===== IMEI WL/BL retValue(%d), listType(%d)====\n",retValue, configId[idx].listType));
             if(configId[idx].rejectCauseIfWhitelist != 0)
             {
                *wlRejectCause = configId[idx].rejectCauseIfWhitelist;
             }
             break;  //UE found break loop and return.
          }
      }
   }

   releaseAccessToIdList(sharedMemPtr);
   return (retValue);
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitIDTState
 *
 * Description : This procedure handles all the events received in Wait IDT
 *               state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitIDTState (unsigned int ueIndex, 
                                  RrcUeEvent   ueEvent)
{
    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_IDT_RCVD:
           /* Check and store if IMSI received in IDT */
           if (0 == checkImsiReceivedInIdt (ueIndex))
           {
               DEBUGMSG(("IMSI is not received in IDT Message\n"));
           }
           else
           {
               DEBUGMSG(("IMSI is received in IDT Message\n"));
           }

           /* Send Identity Request for IMEI to UE */
           buildAndSendDirectTransferIdentityReqIMEI (ueIndex);
           break;
        
        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break; 

        case UE_DED_MEAS_RTT_INIT_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_INIT_REQ_SUCCESS:
           /* For Single Meas on All UEs measurement value is updated in CL.*/
           break; 

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_IDT state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : calculateLastDigitOfImei
 *
 * Description : This procedure calculates the last digit of IMEI as per Luhn
 *               formula & stores it in UE Control Block
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void calculateLastDigitOfImei (unsigned int ueIndex)
{
    unsigned int lastDigit       = 0;
    unsigned int imeiCount       = 0;
    unsigned int sumOfOddDigits  = 0;
    unsigned int sumOfEvenDigits = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Calculate Sum of Digits at Odd index */
    for (imeiCount = 1; imeiCount < pUeSpecificData->imeiLen; 
         imeiCount = imeiCount + 2)
    {
        if (((pUeSpecificData->imei[imeiCount] - '0') * 2) >= 10)
        {
            sumOfOddDigits = sumOfOddDigits + 
                             ((((pUeSpecificData->imei[imeiCount] - '0') * 2) % 10) + 1);
        }
        else
        {
            sumOfOddDigits = sumOfOddDigits + 
                             ((pUeSpecificData->imei[imeiCount] - '0') * 2);
        }
    }

    /* Calculate Sum of Digits at Even index */
    for (imeiCount = 0; imeiCount < pUeSpecificData->imeiLen; 
         imeiCount = imeiCount + 2)
    {
        sumOfEvenDigits = sumOfEvenDigits + (pUeSpecificData->imei[imeiCount] - '0');
    }

    /* Calculate Sum of Total Digits */
    if (0 == ((sumOfOddDigits + sumOfEvenDigits) % 10))
    {
        lastDigit = 0;
    }
    else
    {
        lastDigit = 10 - ((sumOfOddDigits + sumOfEvenDigits) % 10);
    }

    /* Store the last digit of IMEI */
    pUeSpecificData->imei[pUeSpecificData->imeiLen - 1] = lastDigit + '0';

    if (pUeSpecificData->imeiLen < MAX_IMSI_LENGTH)
    {
        pUeSpecificData->imeiLen++;
    }

    char imeiValue[MAX_IMEI_LENGTH + 1];
    strncpy(imeiValue, pUeSpecificData->imei, pUeSpecificData->imeiLen);
    imeiValue[pUeSpecificData->imeiLen] = '\0';

    DEBUG3(("IMEI Length = %d IMEI = %s\n", pUeSpecificData->imeiLen,
                                                 imeiValue));
}

/*
 *############################################################################
 * Method Name : storeImeiInUeControlBlock
 *
 * Description : This procedure stores IMEI in UE Control Block.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void storeImeiInUeControlBlock (unsigned int ueIndex)
{
    unsigned int imeiLen    = 0;
    unsigned int imeiCount  = 0;
    unsigned int maxImeiLen = 0;
    unsigned int imeiIndex  = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    imeiLen = pUeSpecificData->ulDcchData.pNasMsg[2];
    maxImeiLen = (((imeiLen * 2) -1) < MAX_IMEI_LENGTH) ? (imeiLen * 2) : MAX_IMEI_LENGTH;
    imeiIndex = 3;

    /* 
     * Store the IMEI received in Identity Response Message.
     */
    pUeSpecificData->imeiLen = maxImeiLen;

    for (imeiCount = 0; imeiCount < maxImeiLen; imeiCount++)
    {
        if (0 == imeiCount % 2)
        {
            /* 
             * Store in ASCII Format. BCD to ASCII conversion is done
             * by adding '0'.
             */
            pUeSpecificData->imei[imeiCount] = 
                   ((pUeSpecificData->ulDcchData.pNasMsg[imeiIndex] & 0xF0) >> 4) + '0';
            imeiIndex++;
        }
        else
        {
            /* 
             * Store in ASCII Format. BCD to ASCII conversion is done
             * by adding '0'.
             */
            pUeSpecificData->imei[imeiCount] =
                  (pUeSpecificData->ulDcchData.pNasMsg[imeiIndex] & 0x0F) + '0'; 
        }
    }

    /* Calculate the last digit of IMEI */
    calculateLastDigitOfImei (ueIndex);

    /* Free the memory allocated for NAS Message */
    if (NULL != pUeSpecificData->ulDcchData.pNasMsg)
    {
        free (pUeSpecificData->ulDcchData.pNasMsg);
        pUeSpecificData->ulDcchData.pNasMsg = NULL;
    }
}

/*
 *############################################################################
 * Method Name : storeTmsiInUeControlBlock
 *
 * Description : This procedure stores TMSI in UE Control Block.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void storeTmsiInUeControlBlock (unsigned int ueIndex)
{
    unsigned int tmsiCount  = 0;
    unsigned int tmsiIndex  = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    if (pUeSpecificData->ulDcchData.pNasMsg[2] >= MAX_TMSI_LENGTH)
    {
        tmsiIndex = 4;

        /* 
         * Store the TMSI received in Identity Response Message.
         */
        pUeSpecificData->tmsiLen = MAX_TMSI_LENGTH;

        for (tmsiCount = 0; tmsiCount < MAX_TMSI_LENGTH; tmsiCount++)
        {
            pUeSpecificData->tmsi[tmsiCount] = pUeSpecificData->ulDcchData.pNasMsg[tmsiIndex];
            tmsiIndex++;
        }
    }

    /* Free the memory allocated for NAS Message */
    if (NULL != pUeSpecificData->ulDcchData.pNasMsg)
    {
        free(pUeSpecificData->ulDcchData.pNasMsg);
        pUeSpecificData->ulDcchData.pNasMsg = NULL;
    }
}

/*
 *############################################################################
 * Method Name : storeImsiInUeControlBlock
 *
 * Description : This procedure stores IMSI in UE Control Block.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void storeImsiInUeControlBlock (unsigned int ueIndex)
{
    unsigned int imsiLen    = 0;
    unsigned int imsiCount  = 0;
    unsigned int maxImsiLen = 0;
    unsigned int imsiIndex  = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    imsiLen = pUeSpecificData->ulDcchData.pNasMsg[2];
    maxImsiLen = (((imsiLen * 2) -1) < MAX_IMSI_LENGTH) ? (imsiLen * 2) : MAX_IMSI_LENGTH;
    imsiIndex = 3;

    /* 
     * Store the IMSI received in Identity Response Message.
     */
    pUeSpecificData->imsiLen = maxImsiLen;

    for (imsiCount = 0; imsiCount < maxImsiLen; imsiCount++)
    {
        if (0 == imsiCount % 2)
        {
            /* 
             * Store in ASCII Format. BCD to ASCII conversion is done
             * by adding '0'.
             */
            pUeSpecificData->imsi[imsiCount] = 
                   ((pUeSpecificData->ulDcchData.pNasMsg[imsiIndex] & 0xF0) >> 4) + '0';
            imsiIndex++;
        }
        else
        {
            /* 
             * Store in ASCII Format. BCD to ASCII conversion is done
             * by adding '0'.
             */
            pUeSpecificData->imsi[imsiCount] =
                  (pUeSpecificData->ulDcchData.pNasMsg[imsiIndex] & 0x0F) + '0'; 
        }
    }

    /* Free the memory allocated for NAS Message */
    if (NULL != pUeSpecificData->ulDcchData.pNasMsg)
    {
        free(pUeSpecificData->ulDcchData.pNasMsg);
        pUeSpecificData->ulDcchData.pNasMsg = NULL;
    }
}

/*
 *############################################################################
 * Method Name : handleNasRejectTimeout
 *
 * Description : This procedure handles NAS Reject to be sent before RRC
 *               Connection Release. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleNasRejectTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUG1(("NAS Reject Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_NAS_REJECT_TIMEOUT_E, pUeId); 
}


/*
 *############################################################################
 * Method Name : delLatchedUe
 *
 * Description : This procedure deletes single UE from Latched Ue list
 *
 * Parameters  : None
 *
 * Returns     : None
 *############################################################################
 */
void delLatchedUe (char * imsi)
{
    LatchedUeDetails *pCurUe  = NULL;
    LatchedUeDetails *pPrevUe = NULL;
	LatchedUeDetails *pTmpUeContext = NULL;
    fxL3gBlUeOutOfCoverage oocUeDetails;
    memset (&oocUeDetails, 0 , sizeof (fxL3gBlUeOutOfCoverage));

    
    if (0 == strncmp(ueControlBlock.pLatchedUeDetails->imsi, imsi, MAX_IMSI_LENGTH))
    {  // UE to delete is at head node
       pCurUe  = ueControlBlock.pLatchedUeDetails;
       oocUeDetails.hdr.msgLength = sizeof (fxL3gBlUeOutOfCoverage);
       oocUeDetails.hdr.msgType   = FXL_3G_BL_UE_OUT_OF_COVERAGE_IND;
       oocUeDetails.hdr.rat       = FXL_RAT_3G;
       strncpy (oocUeDetails.imsiStr, pCurUe->imsi, MAX_IMSI_LENGTH); /* Fill IMSI */
	   oocUeDetails.imsiStr[MAX_IMSI_LENGTH] = '\0';
       if (-1 != clientFd)
       {
           if (send(clientFd, &oocUeDetails, sizeof(oocUeDetails), 0) < 0)
           {
               DEBUG1(("Unable to send Message to External Client\n"));
           }
           else
           {
               DEBUGMSG(("UE Out Of Coverage Indication is sent to External Client\n"));
           }
       }
       else
       {
            DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
       }

	   pTmpUeContext = ueControlBlock.pLatchedUeDetails->next;

       free(pCurUe);
	   pCurUe = NULL;
	   
       ueControlBlock.pLatchedUeDetails = pTmpUeContext;
       return;
    }
    else
    {
        pPrevUe = pCurUe  = ueControlBlock.pLatchedUeDetails;
        /* Check that UE is already latched or not */
        while (NULL != pCurUe)
        {
            if (0 == strncmp(pCurUe->imsi, imsi, MAX_IMSI_LENGTH))
            {   //OOC  UE Found. Send indication to API
                oocUeDetails.hdr.msgLength = sizeof (fxL3gBlUeOutOfCoverage);
                oocUeDetails.hdr.msgType   = FXL_3G_BL_UE_OUT_OF_COVERAGE_IND;
                oocUeDetails.hdr.rat       = FXL_RAT_3G;
                strncpy (oocUeDetails.imsiStr, pCurUe->imsi, MAX_IMSI_LENGTH); /* Fill IMSI */
				oocUeDetails.imsiStr[MAX_IMSI_LENGTH] = '\0';
                if (-1 != clientFd)
                {
                    if (send(clientFd, &oocUeDetails, sizeof(oocUeDetails), 0) < 0)
                    {
                        DEBUG1(("Unable to send Message to External Client\n"));
                    }
                    else
                    {
                        DEBUGMSG(("UE Out Of Coverage Indication is sent to External Client\n"));
                    }
                }
                else
                {
                     DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
                }

                // Delete Ue context from pLatchedUeDetails
                pPrevUe->next = pCurUe->next;
                free(pCurUe);
                break;
            }

            pPrevUe = pCurUe;
            pCurUe = pCurUe->next;
        }
    }

    free(imsi);
}

/*
 *############################################################################
 * Method Name : delLatchedUeList
 *
 * Description : This procedure deletes all the node of latched UE list.
 *
 * Parameters  : None
 *
 * Returns     : None
 *############################################################################
 */
void delLatchedUeList (void)
{
   LatchedUeDetails *pDelUe = ueControlBlock.pLatchedUeDetails;

   while (NULL != pDelUe)
   {
       ueControlBlock.pLatchedUeDetails = ueControlBlock.pLatchedUeDetails->next;
       free (pDelUe);
       pDelUe = ueControlBlock.pLatchedUeDetails;
   }

   ueControlBlock.pLatchedUeDetails = NULL;
   pDelUe = NULL;
}

/*
 *############################################################################
 * Method Name : getNumOfUeLatched
 *
 * Description : This procedure gives the number of UEs latched to 3G ID catcher
 *
 * Parameters  : None
 *
 * Returns     : Number of UE latched to 3G Id Catcher
 *############################################################################
 */
unsigned int getNumOfUeLatched (void)
{
   unsigned int ueCount = 0;

   LatchedUeDetails *pCurUe = ueControlBlock.pLatchedUeDetails;

   while (NULL != pCurUe)
   {
       ueCount++;
       pCurUe = pCurUe->next;
   }

   return (ueCount);
}

/*
 *############################################################################
 * Method Name : addIMSIInLatchedUeList
 *
 * Description : This procedure adds IMSI in latched UE list if not present.
 *
 * Parameters  : 
 *   pImsi        IMSI Value
 *
 * Returns     :  None
 *############################################################################
 */
unsigned int addIMSIInLatchedUeList (char *pImsi, unsigned int ueIndex)
{
    unsigned int found = 0;
    LatchedUeDetails *pCurUe = NULL;
    LatchedUeDetails *pTmpUe = NULL;
    LatchedUeDetails *pNewUe = NULL;
    RadioLinkReconfigPrepReqInfo *pRLPrepReq = NULL;

    if (NULL == ueControlBlock.pLatchedUeDetails)
    {
        ueControlBlock.pLatchedUeDetails = (LatchedUeDetails *)malloc(sizeof(LatchedUeDetails));
        strncpy (ueControlBlock.pLatchedUeDetails->imsi, pImsi, MAX_IMSI_LENGTH);
        ueControlBlock.pLatchedUeDetails->measType = FXL_NO_MEAS;
        ueControlBlock.pLatchedUeDetails->next = NULL;
    }
    else
    {
        pCurUe = ueControlBlock.pLatchedUeDetails;

        /* Check that UE is already latched or not */
        while (NULL != pCurUe)
        {
            if (0 == strncmp(pCurUe->imsi, pImsi, MAX_IMSI_LENGTH))
            {
               found = 1;
               
               if ((FXL_RTT_MEAS  == pCurUe->measType) || (FXL_RSCP_MEAS == pCurUe->measType) || 
			   	   (FXL_RTT_AND_RSCP_MEAS == pCurUe->measType) || (FXL_RTT_AND_RSCP_AND_GPS_MEAS == pCurUe->measType))
               {
                    /* Post Radio Link Setup Request Event to CL Send Task */
                    pRLPrepReq  = (RadioLinkReconfigPrepReqInfo *)malloc(sizeof(RadioLinkReconfigPrepReqInfo));
                    pRLPrepReq->rlId        = ueIndex;
                    pRLPrepReq->ulSIRtarget = 400;

                    SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                                  CL_NBAP_UE_RADIO_LINK_RECONFIG_PREP_REQ_E, pRLPrepReq);
               }
               break;
            }

            if (NULL == pCurUe->next)
            {
                pTmpUe = pCurUe;
            }

            pCurUe = pCurUe->next;
        }

        /* If not found, add it in last node */
        if (0 == found)
        {
            pNewUe = (LatchedUeDetails *)malloc(sizeof(LatchedUeDetails)); 
            strncpy (pNewUe->imsi, pImsi, MAX_IMSI_LENGTH);
            pNewUe->measType = FXL_NO_MEAS;
            pNewUe->next = NULL;
            pTmpUe->next = pNewUe; 
        }
    }

    return (found);
}

/*############################################################################
 * Method Name : configureSilentCallStart
 *    
 * Description : This procedure start Silent Call for Black listed
 *               Ues configured by external client.
 *       
 * Parameters  : Silent call start command 
 *          
 * Returns     :  None
 * ############################################################################*/
fxLResult configureSilentCallStart (fxL3gStartSilentCallCmd *pStartSilentCallCmd)
{ 
   unsigned int ueIndex                     = 0xFFFFFFFF;
   LatchedUeDetails             *pCurUe     = NULL;
   RadioLinkReconfigPrepReqInfo *pRLPrepReq = NULL;

   /* Black Listed IMSI */
   pCurUe = ueControlBlock.pLatchedUeDetails;

   while (NULL != pCurUe)
   {
       if (0 == strncmp (pCurUe->imsi, 
                         pStartSilentCallCmd->imsiStr,
                         MAX_IMSI_LENGTH))
       {

           /* Get UE Index from IMSI Value */
           ueIndex = getUeIndexFromImsi (pStartSilentCallCmd->imsiStr);

           if((0xFFFFFFFF != ueIndex) && (pCurUe->silentCallEnabFlag == TRUE))
           {
               return FXL_SUCCESS; 
           }

           pCurUe->measType = pStartSilentCallCmd->measType;
           pCurUe->silentCallEnabFlag = TRUE;

           if(0xFFFFFFFF != ueIndex)
           {
                if ((FXL_RTT_MEAS  == pCurUe->measType) || (FXL_RSCP_MEAS == pCurUe->measType) || 
					(FXL_RTT_AND_RSCP_MEAS == pCurUe->measType) || (FXL_RTT_AND_RSCP_AND_GPS_MEAS == pCurUe->measType))
                {
                    /* Post Radio Link Setup Request Event to CL Send Task */
                    pRLPrepReq  = (RadioLinkReconfigPrepReqInfo *)malloc(sizeof(RadioLinkReconfigPrepReqInfo));
                    pRLPrepReq->rlId        = ueIndex;
                    pRLPrepReq->ulSIRtarget = 400;

                    SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                                  CL_NBAP_UE_RADIO_LINK_RECONFIG_PREP_REQ_E, pRLPrepReq);
                }
           }
           else
           {
               /* Send Failure Response to the External Client */
               return (FXL_FAILURE);
           }
           return (FXL_SUCCESS);
       }

       pCurUe = pCurUe->next;
   }             

   return (FXL_FAILURE);
}

/*############################################################################
 * Method Name : configureSilentCallStop
 *    
 * Description : This procedure Stops Silent Call for Black listed
 *               Ues configured by external client.
 *       
 * Parameters  : Silent call start command 
 *          
 * Returns     :  None
 * ############################################################################*/
fxLResult configureSilentCallStop(fxL3gEndSilentCallCmd *pEndSilentCallCmd)
{ 
   unsigned int ueIndex                     = 0xFFFFFFFF;
   LatchedUeDetails             *pCurUe     = NULL;
   RadioLinkReconfigPrepReqInfo *pRLPrepReq = NULL;

   /* Black Listed IMSI */
   pCurUe = ueControlBlock.pLatchedUeDetails;
   while (NULL != pCurUe)
   {
      if (0 == strncmp(pCurUe->imsi, pEndSilentCallCmd->imsiStr, MAX_IMSI_LENGTH))
      {
           /* Get UE Index from IMSI Value */
           ueIndex = getUeIndexFromImsi (pEndSilentCallCmd->imsiStr);

           if(0xFFFFFFFF != ueIndex)
           {
                if ((FXL_RTT_MEAS == pCurUe->measType) || (FXL_RSCP_MEAS == pCurUe->measType) || 
					(FXL_RTT_AND_RSCP_MEAS == pCurUe->measType) || (FXL_RTT_AND_RSCP_AND_GPS_MEAS == pCurUe->measType))
                { 
                    /* Post Radio Link Setup Request Event to CL Send Task */
                    pRLPrepReq  = (RadioLinkReconfigPrepReqInfo *)malloc(sizeof(RadioLinkReconfigPrepReqInfo));
                    pRLPrepReq->rlId        = ueIndex;
                    pRLPrepReq->ulSIRtarget = 60;

                    SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                                  CL_NBAP_UE_RADIO_LINK_RECONFIG_PREP_REQ_E, pRLPrepReq);
                }

                if (FXL_NO_MEAS == pCurUe->measType)
                {
                     sendStopSilentCallRsp(pCurUe->imsi, MAX_IMSI_LENGTH);
                }

                pCurUe->measType           = FXL_NO_MEAS;
                pCurUe->silentCallEnabFlag = FALSE;
           }
           else
           {
               /* Send Failure Response to the External Client */
               return (FXL_FAILURE);
           }

           return (FXL_SUCCESS);
      }

      pCurUe = pCurUe->next;
   }             

   return (FXL_FAILURE);
}

// convert rtt value in time (micro seconds)
float convertRttInTime (unsigned int rttValue)
{
   return (((1024 + (rttValue * 0.0625))* 0.2604)/1000);
}

/* Convert RTT in chips */
float convertRttInChip (unsigned int rttValue)
{
   if(rttValue > 64)
   {
       return (1024.0 + ((rttValue - 64) * 0.0625));
   }
   else
   {
       return 1024.0;
   }
}

// Convert rscp value in dbm
int convertRscpInDbm (unsigned int rscpValue)
{
   return (-115 + (rscpValue - 65421));
}   

/*
 *############################################################################
 * Method Name : checkAndActOnBlackWhiteListIMSI
 *
 * Description : This procedure checks the IMSI is black listed or white listed
 *               and takes proper action.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void checkAndActOnBlackWhiteListIMSI (unsigned int ueIndex)
{
    
    logPrint(LOG_INFO, "checkAndActOnBlackWhiteListIMSI :: lastLac \n");
    unsigned int idx = 0;
    fxL3gRegistrationAttemptedInd ueDetails;
    char *imsiForPcchDel = (char*)malloc((sizeof (char)) * (MAX_IMSI_LENGTH + 1));
    memset (&ueDetails, 0 , sizeof (ueDetails));
    time_t ltime;
    fxL3gBandType bandRet;   
    int activeBand;
    int wlRejectCause = 0;

    char imsiValue[MAX_IMSI_LENGTH + 1];
    char imeiValue[MAX_IMEI_LENGTH + 1];

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Get IMSI Details */
    strncpy (imsiValue, pUeSpecificData->imsi, 
             pUeSpecificData->imsiLen);
    imsiValue[pUeSpecificData->imsiLen] = '\0';

    // Populate Data for Deleting continuous Paging message from L2
    strncpy (imsiForPcchDel , pUeSpecificData->imsi, pUeSpecificData->imsiLen);
    imsiForPcchDel[pUeSpecificData->imsiLen] = '\0';

    /* Post DL CCCH Message to L2 DL DATA Task */
    SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DEL_PCCH_DATA_REQUEST_E, imsiForPcchDel);
    DEBUGMSG(("RRC UE in DCH Stop sending Paging Type1 \n"));

    /* Add the IMSI in latched UE list */
    if(0 == addIMSIInLatchedUeList (imsiValue,ueIndex))
    {
        pUeSpecificData->isFirstTimeLatched = 1;
    }
    else
    {
        pUeSpecificData->isFirstTimeLatched = 0;
    }

    /* Get IMEI Details */
    strncpy(imeiValue, pUeSpecificData->imei, pUeSpecificData->imeiLen);
    imeiValue[pUeSpecificData->imeiLen] = '\0';

    /* Fill Header */
    ueDetails.hdr.msgLength = sizeof (ueDetails);
    ueDetails.hdr.msgType   = FXL_3G_REGISTRATION_ATTEMPTED_IND;
    ueDetails.hdr.rat       = FXL_RAT_3G;
    
    /* Fill IMSI & IMEI */
    strcpy (ueDetails.imsiStr, imsiValue);
    strcpy (ueDetails.imeiStr, imeiValue);

    /* Fill TMSI */
    for (idx = 0; idx < SIZE_OF_TMSI_IDENTITY; idx++)
    {
       ueDetails.tmsi[idx] = pUeSpecificData->tmsi[idx];
    }

    /* Fill Propagation imeiSv */
    ueDetails.imeiSv = ueControlBlock.ueSpecificData[ueIndex].imeiSv;     // Dev team changes
        
    logPrint(LOG_INFO, "Loading ueDetails.imeiSv = %d\n", ueDetails.imeiSv );
    
    /* Fill UL Scrambling Code */
    ueDetails.ulScramblingCode = UL_SCRAMBLING_CODE + ueIndex;

    /* Fill DL Channelization Code */
    ueDetails.dlChannelizationCode = DL_CHANNELISATION_CODE + ueIndex;
//    ueDetails.initialRttVal        = convertRttInTime(pUeSpecificData->rttValue);
    ueDetails.initialRttVal        = convertRttInChip(pUeSpecificData->rttValue);
    ueDetails.initialRscpVal       = convertRscpInDbm(pUeSpecificData->rscpValue);

    /* Fill Propagation Delay */
    ueDetails.propagationDelay = pUeSpecificData->propagationDelay;

    /*Fill GPS Support Type */
    ueDetails.gpsSupportType = pUeSpecificData->networkAssistedGpsSupport;

    /*Fill Access spectrum release support */
    ueDetails.ueAccessSpectrumReleaseSupport = pUeSpecificData->ueAccessSpectrumReleaseIndicator;

    /* copy UMTS Supported Bands */
    ueDetails.numberOfFddBandSupported = ueControlBlock.ueSpecificData[ueIndex].numberOfBandSupported;
    logPrint(LOG_INFO, "Loading LastLac ueDetails.numberOfFddBandSupported = %d\n", ueDetails.numberOfFddBandSupported );
    for(idx = 0; idx < ueDetails.numberOfFddBandSupported; idx++)
    {
        ueDetails.supportedFddBand[idx] = ueControlBlock.ueSpecificData[ueIndex].supportedBand[idx];
    }
    /* Fill Propagation lastLac */
    ueDetails.lastLac = ueControlBlock.ueSpecificData[ueIndex].lastLac;       // Dev team changes
    logPrint(LOG_INFO, "Loading LastLac ueDetails.lastLac = %d\n", ueDetails.lastLac );
    
    
#if 0
    ueDetails.numActiveUsr = ueControlBlock.numOfUe;
    ueDetails.numActiveUsr = 0;
    for(idx = 0; idx <= ueControlBlock.numOfUe; idx++)
    {
        if(0 != ueControlBlock.ueSpecificData[idx].ueUsed )
           ueDetails.numActiveUsr++;
    }

    ueDetails.freeChannel  = 32 - ueDetails.numActiveUsr;
#endif

    DEBUG3(("IMSI Length = %d IMSI = %s\n", pUeSpecificData->imsiLen, imsiValue));
    
    /* Check for WhiteList & BlackList configuration */
//          (1 == isIMSIWhiteListed (ueIndex))) ||
    if(0 == isUeBlackListed(ueIndex, &wlRejectCause))
    {
        DEBUG3(("IMSI(%s) is White Listed, Ue specific Reject cause value(%d)\n", imsiValue, wlRejectCause));

        ueDetails.listType = FXL_WHITEUSERSLIST;

        /* Send NAS Reject Message to whitelisted UE */
		if(1 == pUeSpecificData->routingAreaUpdateReceived)
		{
		    pUeSpecificData->routingAreaUpdateReceived = 0;
            buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_GMM_ROUTING_AREA_UPDATE);
		}
	    else if(1 == pUeSpecificData->attachReqReceived)
		{
		    pUeSpecificData->attachReqReceived = 0;
		    buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_GMM_ATTACH_REQUEST);
		}
        else if(1 == pUeSpecificData->locationUpdateReqReceived)
		{
		    pUeSpecificData->locationUpdateReqReceived = 0;
		    buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_LOCATION_UPDATE_REQ);
		}
		else
		{
		   /* Send RRC Connection Release to whitelisted UE */
		   buildAndSendRrcConnectionRelease (ueIndex, 1);
		}	

   
    }
    else
    {
        DEBUG3(("IMSI(%s) is Black Listed\n", imsiValue));

        ueDetails.listType = FXL_BLACKUSERSLIST;
        pUeSpecificData->isBlackListed = 1;

        if (FXL_UMTS_NO_REDIRECTION == gUmtsConfig.redirectionType)
        {
            /*
             * Send RRC Paging Type2 to Black Listed UE
             */
            buildAndSendRrcPagingType2 (ueIndex);
        }
        else
        {
            /* 
             * Send RRC Connection Release with Redirect Info 
             * to Black Listed UE.
             */ 
            buildAndSendRrcConnectionRelease (ueIndex, 1);
        }

    }

    if (-1 != clientFd)
    {
        if (send(clientFd, &ueDetails, sizeof(ueDetails), 0) < 0)
        {
            DEBUG1(("Unable to send Message to External Client\n"));
        }
        else
        {
            DEBUGMSG(("UE Details Indication is sent to External Client\n"));
        }
    }
    else
    {
         DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
    }
}

int calculateBnadNumber (fxL3gBandType bandRet)
{
    switch (bandRet)
    {
       case FXL_UMTS_BAND_1:
            return 1;
       break;
      
       case FXL_UMTS_BAND_2:
       case FXL_UMTS_ADDITIONAL_BAND_2:
            return 2;
       break;
 
       case FXL_UMTS_BAND_3:
            return 3;
       break;

       case FXL_UMTS_BAND_4:
       case FXL_UMTS_ADDITIONAL_BAND_4:
            return 4;
       break;

       case FXL_UMTS_BAND_5:
       case FXL_UMTS_ADDITIONAL_BAND_5:
            return 5;
       break;

       case FXL_UMTS_BAND_6:
       case FXL_UMTS_ADDITIONAL_BAND_6:
            return 6;
       break;

       case FXL_UMTS_BAND_7:
       case FXL_UMTS_ADDITIONAL_BAND_7:
            return 7;
       break;

       case FXL_UMTS_BAND_8:
            return 8;
       break;

       case FXL_UMTS_BAND_9:
            return 9;
       break;

       case FXL_UMTS_BAND_10:
       case FXL_UMTS_ADDITIONAL_BAND_10:
            return 10;
       break;

       case FXL_UMTS_BAND_11:
            return 11;
       break;

       case FXL_UMTS_BAND_12:
       case FXL_UMTS_ADDITIONAL_BAND_12:
            return 12;
       break;

       case FXL_UMTS_BAND_13:
       case FXL_UMTS_ADDITIONAL_BAND_13:
            return 13;
       break;

       case FXL_UMTS_BAND_14:
       case FXL_UMTS_ADDITIONAL_BAND_14:
            return 14;
       break;

       case FXL_UMTS_BAND_19:
       case FXL_UMTS_ADDITIONAL_BAND_19:
            return 19;
       break;

       case FXL_UMTS_BAND_20:
            return 20;
       break;

       case FXL_UMTS_BAND_21:
            return 21;
       break;

       case FXL_UMTS_BAND_22:
            return 22;
       break;

       case FXL_UMTS_BAND_25:
       case FXL_UMTS_ADDITIONAL_BAND_25:
            return 25;
       break;

       case FXL_UMTS_BAND_26:
       case FXL_UMTS_ADDITIONAL_BAND_26:
            return 26;
       break;

       default:
           return 0;
       break;
    }
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitIdentityRespState
 *
 * Description : This procedure handles all the events received in Wait Identity
 *               Response state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitIdentityRespState (unsigned int ueIndex, 
                                           RrcUeEvent   ueEvent)
{
    logPrint(LOG_INFO, "handleUeEventInWaitIdentityRespState :: lastLac ueEvent = %d\n", ueEvent);   // check if UE_IDENTITY_REQ_TIMEOUT event is triggered
    unsigned int checkForBlackWhiteList = 1;
	int wlRejectCause = 0;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_IDENTITY_REQ_TIMEOUT:
           logPrint(LOG_INFO, "handleUeEventInWaitIdentityRespState :: lastLac UE_IDENTITY_REQ_TIMEOUT ueEvent = %d\n", ueEvent);   // check if UE_IDENTITY_REQ_TIMEOUT event is triggered
           if (0 == pUeSpecificData->imeiLen)
           {
              if (pUeSpecificData->ueIMEIReqCount < MAX_RETRANSMIT_IDENTITY_REQ)
              {
                  /* Send Identity Request for IMEI to UE */
                  buildAndSendDirectTransferIdentityReqIMEI (ueIndex);
              }
              else
              {
                  DEBUG1(("IMEI not received after maximum number retry, filling IMEI as ZERO\n"));

                  /* Fill the IMEI with zero values */
                  pUeSpecificData->imeiLen = MAX_IMEI_LENGTH;
                  memset((char *)pUeSpecificData->imei, 0, MAX_IMEI_LENGTH);

                  /* Send Identity Request for TMSI to UE */
                  buildAndSendDirectTransferIdentityReqTMSI (ueIndex);
              }
           }
           else if (0 == pUeSpecificData->tmsiLen)
           {
              if (pUeSpecificData->ueTMSIReqCount < MAX_RETRANSMIT_IDENTITY_REQ)
              {
                  /* Send Identity Request for TMSI to UE */
                  buildAndSendDirectTransferIdentityReqTMSI (ueIndex);
              }
              else
              {
                  DEBUG1(("TMSI not received after maximum number retry, filling TMSI as ZERO\n"));

                  /* Fill the TMSI with zero values */
                  pUeSpecificData->tmsiLen = MAX_TMSI_LENGTH;
                  memset((char *)pUeSpecificData->tmsi, 0, MAX_TMSI_LENGTH);

                  /* Send Identity Request for IMSI to UE */
                  buildAndSendDirectTransferIdentityReqIMSI (ueIndex);
              }
           }
           else if (0 == pUeSpecificData->imsiLen)
           {
              if (pUeSpecificData->ueIMSIReqCount < MAX_RETRANSMIT_IDENTITY_REQ)
              {
                  /* Send Identity Request for IMSI to UE */
                  buildAndSendDirectTransferIdentityReqIMSI (ueIndex);
              }
              else
              {
                  DEBUG1(("IMSI not received after maximum number retry, filling IMSI as ZERO\n"));

                  /* Fill the IMSI with zero values */
                  pUeSpecificData->imsiLen = MAX_IMSI_LENGTH;
                  memset((char *)pUeSpecificData->imsi, 0, MAX_IMSI_LENGTH);

                  checkAndActOnBlackWhiteListIMSI (ueIndex);
              }
           }
           else
           {
               checkAndActOnBlackWhiteListIMSI (ueIndex);
           }
           break;

        case UE_NAS_REJECT_TIMEOUT:
		   if(0 == isUeBlackListed(ueIndex, &wlRejectCause))
           {
		       if (1 == pUeSpecificData->routingAreaUpdateReceived)
		       {
		           pUeSpecificData->routingAreaUpdateReceived = 0;
			       buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_GMM_ROUTING_AREA_UPDATE);
		       }
		       else if(1 == pUeSpecificData->attachReqReceived)
		       {
		           pUeSpecificData->attachReqReceived = 0;
		           buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_GMM_ATTACH_REQUEST);
		       }
               else if(1 == pUeSpecificData->locationUpdateReqReceived)
		       {
		           pUeSpecificData->locationUpdateReqReceived = 0;
		           buildAndSendDirectTransferNasReject (ueIndex, wlRejectCause, NAS_LOCATION_UPDATE_REQ);
		       }
		       else
		       {
		           /* Send RRC Connection Release to whitelisted UE */
                   buildAndSendRrcConnectionRelease (ueIndex, 1);
		       }
		   }
           break;

        case UE_IDENTITY_RESP_RCVD:
           /* Stop the Identity Request Timer */
           if (TRUE == isTimerRunning (&(pUeSpecificData->ueTimer)))
           {
               timerStop(&(pUeSpecificData->ueTimer));

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           if ((pUeSpecificData->ulDcchData.pNasMsg[3] & 0x07) == 0x02)
           {
               DEBUGMSG(("IMEI received in Indentity Response\n"));
               /*
                * IMEI received in Identity Response.
                * Store the IMEI received in Identity Response.
                */ 
               storeImeiInUeControlBlock (ueIndex);

               /* Send IMSI Identity Request to UE */
               buildAndSendDirectTransferIdentityReqTMSI (ueIndex);
               checkForBlackWhiteList = 0;
           }
           else if ((pUeSpecificData->ulDcchData.pNasMsg[3] & 0x07) == 0x04)
           {
               DEBUGMSG(("RRC : TMSI received in Indentity Response\n"));
               /*
                * TMSI received in Identity Response.
                * Store the TMSI received in Identity Response.
                */ 
               storeTmsiInUeControlBlock (ueIndex);

               /* Check if IMSI is received in RRC Connection Resquest/IDT */
               if ((UE_IDENTITY_IMSI == 
                    pUeSpecificData->ueInitialIdentity.initialUEIdentityType) ||
                   (pUeSpecificData->imsiLen > 0))
               {
                   DEBUGMSG(("IMSI is already received in RRC Connection Request/IDT\n"));
               }
               else
               {
                   /* Send IMSI Identity Request to UE */
                   buildAndSendDirectTransferIdentityReqIMSI (ueIndex);
                   checkForBlackWhiteList = 0;
               }
           }
           else if ((pUeSpecificData->ulDcchData.pNasMsg[3] & 0x07) == 0x01)
           {
               DEBUGMSG(("RRC : IMSI received in Indentity Response\n"));
               /*
                * IMSI received in Identity Response.
                * Store the IMSI received in Identity Response.
                */ 
               storeImsiInUeControlBlock (ueIndex);
           }
           else
           {
               DEBUG3(("Wrong Identity Response received from UE Id(%ld)\n", ueIndex)); 
           }

           if (1 == checkForBlackWhiteList)
           {
               checkAndActOnBlackWhiteListIMSI (ueIndex);
           }
           break;

        case UE_IDT_RCVD:
		   /* Check and store if IMSI received in IDT */
           if (0 == checkImsiReceivedInIdt (ueIndex))
           {
               DEBUGMSG(("IMSI is not received in IDT Message\n"));
           }
           else
           {
               DEBUGMSG(("IMSI is received in IDT Message\n"));
           }
		   break;
		   
        case UE_DED_MEAS_RTT_INIT_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_INIT_REQ_SUCCESS:
           /* For Single Meas on All UEs measurement value is updated in CL.*/
           break; 

        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
           /* Stop the Identity Request Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break; 

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_IDENTITY_RESP state\n", 
                  ueEvent));
           break;
    }
}

/* 
 * ############################################################################
 * Method Name : handleDedMeasInitRequestTimeout
 *        
 * Description : This procedure handle Dedicated Meas Init Request Timeout. 
 *       
 * Parameters  : 
 * pTimer       Pointer to the UE timer
 * pUeIndex     Pointer to the UE Index
 *                   
 * Returns     :  None
 * ############################################################################
 */

void handleDedMeasInitRequestTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("Dedicated Meas Init Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E,
                  RRC_DED_MEAS_INIT_REQ_TIMEOUT_E, pUeId);
}

/*
 * ############################################################################
 * Method Name : sendDedicatedMeasInitReq
 *        
 * Description : This procedure sends Dedicated Measurement Init Request to L1.
 *            
 * Parameters  : 
 * ueIndex      Index of the UE in UE Control Block
 *                
 * Returns     :  None
 * ############################################################################
 */
void sendDedicatedMeasInitReq (MeasInfo_t measInfo)
{
    unsigned int *pTimerUeId    = NULL;
    MeasInfo_t   *pMsgQMeasInfo = NULL;
    MeasInfo_t   *pMsgQRscpMeasInfo = NULL;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[measInfo.rlId];

    /* Start the Timer for Dedicated Meas Init Request */
    pTimerUeId  = (unsigned int *)malloc (sizeof(unsigned int));
    *pTimerUeId = measInfo.rlId;
   
    timerStart(&pUeSpecificData->ueDedMeasTimer,
               DED_MEAS_INIT_REQ_TIMEOUT,
               0,
               handleDedMeasInitRequestTimeout,
               pTimerUeId);

    /* Post Dedicated Meas Init Request Event to CL Send Task */
    pMsgQMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
    pMsgQMeasInfo->rlId = measInfo.rlId;
    pMsgQMeasInfo->measType = measInfo.measType;
    
    /* Change the UE State and send task to CL send */
    if (measInfo.measType == FXL_RTT_MEAS )
    {
       SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                      CL_NBAP_UE_RTT_DEDICATED_MEAS_INIT_REQ_E, pMsgQMeasInfo);

    }
    else if (measInfo.measType == FXL_RSCP_MEAS)
    {
       SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                      CL_NBAP_UE_RSCP_DEDICATED_MEAS_INIT_REQ_E, pMsgQMeasInfo);
    }
    else if ((measInfo.measType == FXL_RTT_AND_RSCP_MEAS) ||
		     (FXL_RTT_AND_RSCP_AND_GPS_MEAS == measInfo.measType))
    {
       pMsgQMeasInfo->measType = FXL_RTT_MEAS;
       SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                      CL_NBAP_UE_RTT_DEDICATED_MEAS_INIT_REQ_E, pMsgQMeasInfo);

       pMsgQRscpMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
       pMsgQRscpMeasInfo->rlId = measInfo.rlId;
       pMsgQRscpMeasInfo->measType = FXL_RSCP_MEAS;
       SendMsgToTask (APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                      CL_NBAP_UE_RSCP_DEDICATED_MEAS_INIT_REQ_E, pMsgQRscpMeasInfo);

    }
    else
    {
       //nothing
    }
}

/* 
 * ############################################################################
 * Method Name : handleDedMeasTermRequestTimeout
 *            
 * Description : This procedure handle Dedicated Meas Term Request Timeout. 
 *            
 * Parameters  : 
 * pTimer       Pointer to the UE timer
 * pUeIndex     Pointer to the UE Index
 *                             
 * Returns     :  None
 * ############################################################################
 */
void handleDedMeasTermRequestTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUG3(("Dedicated Meas Term Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E,
                  RRC_DED_MEAS_TERM_REQ_TIMEOUT_E, pUeId);
}

/*
* ############################################################################
* Method Name : sendDedicatedMeasTermReq
*            
* Description : This procedure sends Dedicated Measurement Term Request to L1.
*                 
* Parameters  : 
* ueIndex      Index of the UE in UE Control Block
*                         
* Returns     :  None
* ############################################################################
*/

void sendDedicatedMeasTermReq (unsigned int ueIndex)
{
   unsigned int *pTimerUeId = NULL;
   MeasInfo_t   *pMsgQMeasInfo = NULL;
   MeasInfo_t   *pMsgQRscpMeasInfo = NULL;

   /* Get UE specific data */
   RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

   /* Start the Timer for RTT Dedicated Meas Term Request */
   pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
   *pTimerUeId = ueIndex;

   timerStart(&pUeSpecificData->ueDedMeasTimer,
              DED_MEAS_TERM_REQ_TIMEOUT,
              0,
              handleDedMeasTermRequestTimeout,
              pTimerUeId);
   
   switch (pUeSpecificData->dedMeasType)
   {
     case FXL_RTT_MEAS:
     {
         /* Post RTT Dedicated Meas Term Request Event to CL Send Task */
         pMsgQMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
         pMsgQMeasInfo->rlId = ueIndex;
         pMsgQMeasInfo->measType = FXL_RTT_MEAS;

         SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                       CL_NBAP_UE_RTT_DEDICATED_MEAS_TERM_REQ_E, pMsgQMeasInfo);
     } 
     break;

     case FXL_RSCP_MEAS:
     {
         /* Post RSCP Dedicated Meas Term Request Event to CL Send Task */
         pMsgQMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
         pMsgQMeasInfo->rlId = ueIndex;
         pMsgQMeasInfo->measType = FXL_RSCP_MEAS;

         SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                       CL_NBAP_UE_RSCP_DEDICATED_MEAS_TERM_REQ_E, pMsgQMeasInfo);
     }
     break;

     case FXL_RTT_AND_RSCP_AND_GPS_MEAS:
     case FXL_RTT_AND_RSCP_MEAS:
     {
         /* Post RSCP Dedicated Meas Term Request Event to CL Send Task */
         pMsgQMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
         pMsgQMeasInfo->rlId = ueIndex;
         pMsgQMeasInfo->measType = FXL_RTT_MEAS;
         SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                       CL_NBAP_UE_RTT_DEDICATED_MEAS_TERM_REQ_E, pMsgQMeasInfo);

         pMsgQRscpMeasInfo  = (MeasInfo_t *)malloc (sizeof(MeasInfo_t));
         pMsgQRscpMeasInfo->rlId = ueIndex;
         pMsgQRscpMeasInfo->measType = FXL_RSCP_MEAS;
         SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E,
                       CL_NBAP_UE_RSCP_DEDICATED_MEAS_TERM_REQ_E, pMsgQRscpMeasInfo);
     }
     break;

     default:
         DEBUG2(("UE event is not handled in DEDICATED MEAS TERM REQ state\n"));
           break;
   }
}

/*
 *############################################################################
 * Method Name : handleRadioLinkDeleteRequestTimeout
 *
 * Description : This procedure handles Radio Link Delete Request Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleRadioLinkDeleteRequestTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUG3((" Radio Link Delete Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_RL_DELETE_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : sendRadioLinkpDeleteReqToL1
 *
 * Description : This procedure sends RL Setup Delete Request to L1.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void sendRadioLinkpDeleteReqToL1 (unsigned int ueIndex)
{
    unsigned int *pTimerUeId = NULL;
    unsigned int *pMsgQUeId  = NULL;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change the UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_RL_DELETE_RESP;

    /* Start the Timer for Radio Link Delete Request */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;

    timerStart(&pUeSpecificData->ueTimer,
               RL_DELETE_REQ_TIMEOUT,
               0,
               handleRadioLinkDeleteRequestTimeout,
               pTimerUeId);
 
    /* Post Radio Link Delete Request Event to CL Send Task */
    pMsgQUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pMsgQUeId = ueIndex;

    SendMsgToTask(APP_RRC_TASK_E, CL_L1C_SND_TASK_E, 
                  CL_NBAP_UE_RADIO_LINK_DELETE_REQ_E, pMsgQUeId);
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitRrcConnRelCompState
 *
 * Description : This procedure handles all the events received in Wait RRC
 *               Connection Release Complete state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitRrcConnRelCompState (unsigned int ueIndex, 
                                             RrcUeEvent   ueEvent)
{
    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
        case UE_RRC_CON_REL_TIMEOUT:
        case UE_RRC_CON_REL_COMP_RCVD:
           /* Stop the RRC Connection Release Timer */
           if (TRUE == isTimerRunning (&(pUeSpecificData->ueTimer)))
           {
               timerStop(&(pUeSpecificData->ueTimer));

               if (NULL != pUeSpecificData->ueTimer.parm)
               {
                   free (pUeSpecificData->ueTimer.parm);
                   pUeSpecificData->ueTimer.parm = NULL;
               }
           }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_RRC_CON_REL_COMPLETE state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitRLDeleteRespState
 *
 * Description : This procedure handles all the events received in Wait Radio
 *               Link Delete Response state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitRLDeleteRespState (unsigned int ueIndex, 
                                           RrcUeEvent   ueEvent)
{
    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];
    unsigned int      *pL2UeId  = NULL;

    switch (ueEvent)
    {
        case UE_RL_DELETE_TIMEOUT:
        case UE_RL_DELETE_SUCCESS:
           /* Stop the Radio Link Delete Timer */
           if (TRUE == isTimerRunning (&(pUeSpecificData->ueTimer)))
           {
               timerStop(&(pUeSpecificData->ueTimer));
           }

           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }

		   /* Stop the Dedicated Generic Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
           {
               timerStop(&pUeSpecificData->ueDedGenericTimer);
           }

           DEBUG3(("RRC: Sending L2 context release \n"));
           /* Post L2 Config Request to L2 DL DATA Task */
           pL2UeId = (unsigned int *)malloc(sizeof(ueIndex));
           *pL2UeId = ueIndex; 
           SendMsgToTask (APP_RRC_TASK_E, L2_CFG_TASK_E,
                          L2_CONFIG_DEL_REQUEST_E, pL2UeId);

           /* 
            * Before Cleaning the UE Context, Send Paging Type1 
            * to Black Listed Users.
            */
           if ((FXL_UMTS_NO_REDIRECTION == gUmtsConfig.redirectionType) &&
               (1 == pUeSpecificData->isBlackListed) && 
               // Adding Paging context in MAC after UE is released, Deleting context after Attach req Ind
               //(1 == pUeSpecificData->isFirstTimeLatched) &&
               (0 == pUeSpecificData->stopMsgHandling) &&
               (0 == pUeSpecificData->emergencyCallAttempted))
           {
                /* 
                 * Send Paging Type1 Message to L2 for the black listed
                 * UE latched first time. Black listed UE is pagged by L2
                 * for each DRX Cycle Length interval.
                 */
                buildAndSendRrcPagingType1 (ueIndex);
           }

           /* Clean the UE Context */
           cleanUeContextInUeControlBlock (ueIndex);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_RL_DELETE_RESP state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : handleUeEvents
 *
 * Description : This procedure handles all the events received from UE
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEvents (unsigned int ueIndex, RrcUeEvent ueEvent)
{
    logPrint(LOG_INFO, "handleUeEvents :: lastLac \n");
    DEBUG3(("Received event(%ld) in UE State (%ld)\n", 
           ueEvent, ueControlBlock.ueSpecificData[ueIndex].ueState));

    /* Check for UE state in particular index */
    switch (ueControlBlock.ueSpecificData[ueIndex].ueState)
    {
        case UE_IDLE:
           handleUeEventInIdleState (ueIndex, ueEvent);
           break;
  
        case UE_WAIT_FOR_RL_SETUP_RESP:
           handleUeEventInWaitRlSetupRespState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_RRC_CON_SETUP_COMPLETE:
           handleUeEventInWaitRrcConnSetupCompState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_IDT:
           handleUeEventInWaitIDTState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_IDENTITY_RESP:
           handleUeEventInWaitIdentityRespState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_RRC_CON_REL_COMPLETE:
           handleUeEventInWaitRrcConnRelCompState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_RL_DELETE_RESP:
           handleUeEventInWaitRLDeleteRespState (ueIndex, ueEvent);
           break;

        case UE_WAIT_FOR_PAGING_TYPE2_TIMEOUT:
           handleUeEventInWaitPagingType2TimeoutState (ueIndex, ueEvent);
           break;

        case UE_STOP_MSG_HANDLING:
           //No need to process message during stop Id Catcher
           break;

        case UE_WAIT_FOR_PHY_CH_RECONF_RESP:
           handleUeEventInWaitPhyChReconfRspState (ueIndex, ueEvent);
           break;

        default:
           DEBUG2(("**** Message received in Wrong State ****\n"));
           break;
    }
}

/*
 *############################################################################
 * Method Name : handleRrcCleanUp
 *
 * Description : This procedure cleans up the UE context during stop ID Catcher
 *
 * Parameters  :  None
 *
 * Returns     :  None
 *############################################################################
 */
void handleRrcCleanUp (void)
{
   unsigned int idx = 0;

   for (idx = 0; idx < MAX_UE_SUPPORTED; idx++)
   {
       if (0 != ueControlBlock.ueSpecificData[idx].ueUsed)
       {
           ueControlBlock.ueSpecificData[idx].stopMsgHandling = 1;
           handleUeEvents (idx, UE_RL_SETUP_FAILURE);
           ueControlBlock.cleanupCount++;
       }
       else
       {
           ueControlBlock.ueSpecificData[idx].ueState = UE_STOP_MSG_HANDLING;
       }
   }

   if (0 == ueControlBlock.cleanupCount)
   {
       /* Clean the Memory Allocated */
       delLatchedUeList ();
       memset ((RrcUeControlBlock *)&ueControlBlock, 0, sizeof(RrcUeControlBlock));

       pthread_exit (NULL);
   }
}

/*
 *############################################################################
 * Method Name : sendStopSilentCallRsp
 *
 * Description : This function used to send stop Silent call Response.
 *
 * Parameters  : 
 *   pImsiStr      IMSI of the UE 
 *   imsiLen       IMSI length
 *
 * Returns     :  None
 *############################################################################
 */

void sendStopSilentCallRsp(char *pImsiStr, unsigned int imsiLen)
{
    fxL3gEndSilentCallRsp endSilentCallRsp;

    /* Send Change UE UL Tx Pwr Response to External Client */
    memset((fxL3gEndSilentCallRsp *)&endSilentCallRsp, 0,
            sizeof(fxL3gEndSilentCallRsp));

    endSilentCallRsp.hdr.msgLength = sizeof(fxL3gEndSilentCallRsp);
    endSilentCallRsp.hdr.msgType   = FXL_3G_END_SILENT_CALL_RSP;
    endSilentCallRsp.hdr.rat = FXL_RAT_3G;
    endSilentCallRsp.result  = FXL_SUCCESS;
    strncpy(endSilentCallRsp.imsiStr, pImsiStr, imsiLen);
    endSilentCallRsp.imsiStr[imsiLen] = '\0';

    if (-1 != clientFd)
    {
        if (send(clientFd, &endSilentCallRsp, sizeof(fxL3gEndSilentCallRsp), 0) < 0)
        {
             DEBUG1(("Send End Silent Call Response is failed\n"));
        }
        else
        {
             DEBUGMSG(("Send End Silent Call Response is sent to External Client\n"));
        }
    }
    else
    {
        DEBUG1(("Send End Silent call Response is failed, clinetFd = -1\n"));
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcPagingType2
 *
 * Description : This procedure builds the RRC Paging Type2 Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcPagingType2 (unsigned int ueIndex) 
{
    RrcDataReq        *pRlcDataReq = NULL;
    unsigned int       rrcPagingType2Len = 0;
    unsigned char     *pRrcPagingType2Data = NULL;
    unsigned int      *pTimerUeId = NULL;
    RrcPagingType2Data rrcPagingType2;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change the UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_PAGING_TYPE2_TIMEOUT;

    /* Start the Timer for RRC Connection Release */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               500,
               0,
               handlePagingType2Timeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcPagingType2.rrcTransactionId   = pUeSpecificData->rrcTransactionId;
        rrcPagingType2.pagingCause        = PAGING_CAUSE_TERMINATING_HIGH_PRIORITY_SIGNALLING;

        if (pUeSpecificData->ulDcchData.cnDomainType == CN_DOMAIN_IDENTITY_CS)
        {
            rrcPagingType2.cnDomainType              = CN_DOMAIN_IDENTITY_PS;
            pUeSpecificData->ulDcchData.cnDomainType = CN_DOMAIN_IDENTITY_PS;
        }
        else
        {
            rrcPagingType2.cnDomainType              = CN_DOMAIN_IDENTITY_CS;
            pUeSpecificData->ulDcchData.cnDomainType = CN_DOMAIN_IDENTITY_CS;
        }

        rrcPagingType2.pagingRecordTypeId = PAGING_RECORD_TYPE_ID_IMSI_GSM_MAP;

        if (ASN_SUCCESS == fillAndEncodePagingType2 (&rrcPagingType2,
                                                     &pRrcPagingType2Data,
                                                     &rrcPagingType2Len))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcPagingType2Len;
            pRlcDataReq->rrcPdu            = pRrcPagingType2Data;

            DEBUGMSG(("RRC Paging Type2 is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                         (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        } 
        else
        {
            DEBUG1((" Encoding RRC Paging Type2 is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Paging Type2 is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : handlePagingType2Timeout
 *
 * Description : This procedure handles Paging Type2 Request Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handlePagingType2Timeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("Paging Type2 Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_PAGING_TYPE2_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitPagingType2TimeoutState
 *
 * Description : This procedure handles all the events received in Wait Paging
 *               Type2 state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitPagingType2TimeoutState (unsigned int ueIndex, 
                                                 RrcUeEvent   ueEvent)
{
    fxL3gDedMeasValueInd measValue;
//    time_t timeSec;
//    timeSec = time (NULL);


    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    switch (ueEvent)
    {
        case UE_RRC_PAGING_TYPE2_TIMEOUT:
        {
           DEBUGMSG(("Paging Type2 Timeout\n"));

           buildAndSendRrcPagingType2 (ueIndex);
           
           /* 
            * Check for Ue Positioning Type selected by user and take
            * appropriate action.
            */
           checkUePositioningMethod (ueIndex); 
        }
        break;
        
        case UE_DED_MEAS_RTT_REPORT_SUCCESS:
        {
           measValue.hdr.msgLength = sizeof(fxL3gDedMeasValueInd);
           measValue.hdr.msgType = FXL_3G_DED_MEAS_VALUE_IND;
           measValue.hdr.rat = FXL_RAT_3G;

           strncpy (measValue.imsiStr, pUeSpecificData->imsi,
                    pUeSpecificData->imsiLen);
           measValue.imsiStr[pUeSpecificData->imsiLen] = '\0';

           measValue.measType = pUeSpecificData->measType;
//           measValue.rttValue = convertRttInTime (pUeSpecificData->rttValue);       
           measValue.rttValue = convertRttInChip (pUeSpecificData->rttValue);       
           measValue.rscpValue = 0;
//           measValue.bfn       = gClCntxt.bfn;
//           measValue.timeSec   = (unsigned int )timeSec;

           if (-1 != clientFd)
           {
               if (send(clientFd, &measValue, sizeof(measValue), 0) < 0)
               {
                  DEBUG1(("Unable to send Message to External Client\n"));
               }
               else
               {
                  DEBUGMSG(("Dedicated Meas value Indication is sent to External Client\n"));
               }
           }
           else
           {
               DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
           }
        } 
        break;

        case UE_DED_MEAS_RSCP_REPORT_SUCCESS:
        {
           measValue.hdr.msgLength = sizeof(fxL3gDedMeasValueInd);
           measValue.hdr.msgType = FXL_3G_DED_MEAS_VALUE_IND;
           measValue.hdr.rat = FXL_RAT_3G;
 
           strncpy (measValue.imsiStr, pUeSpecificData->imsi,
                    pUeSpecificData->imsiLen);
           measValue.imsiStr[pUeSpecificData->imsiLen] = '\0';
 
           measValue.measType  = pUeSpecificData->measType;
           measValue.rttValue  = 0;
           measValue.rscpValue = convertRscpInDbm (pUeSpecificData->rscpValue);
//           measValue.bfn       = gClCntxt.bfn;
//           measValue.timeSec   = (unsigned int )timeSec;
        
           if (-1 != clientFd)
           {
              if (send(clientFd, &measValue, sizeof(measValue), 0) < 0)
              {
                 DEBUG1(("Unable to send Message to External Client\n"));
              }
              else
              {
                 DEBUGMSG(("Dedicated Meas value Indication is sent to External Client\n"));
              }
           }
           else
           {
              DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
           }
        }
        break;

        case UE_RLC_ERROR_CALL_RLS:
        case UE_RL_SETUP_FAILURE:
        {
           /* Stop the UE Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);
           }

           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }

		   /* Stop the Dedicated Generic Meas Timer */
		   if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
           {
               timerStop(&pUeSpecificData->ueDedGenericTimer);
		   }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
        }
        break; 

        case UE_DED_MEAS_RTT_INIT_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_INIT_REQ_SUCCESS:
        {
           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }
        }
        break;

        case UE_DED_MEAS_RTT_TERM_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_TERM_REQ_SUCCESS:
        {
           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }

           if(pUeSpecificData->endSilentCallRspSent == 0)
           {
               pUeSpecificData->endSilentCallRspSent = 1;
               sendStopSilentCallRsp(pUeSpecificData->imsi, pUeSpecificData->imsiLen);
           }
        }
        break;

        case UE_DED_MEAS_INIT_REQ_TIMEOUT:
			//Do Nothing
			break;

        case UE_DED_MEAS_TERM_REQ_TIMEOUT:
		{
			if(pUeSpecificData->endSilentCallRspSent == 0)
			{
				pUeSpecificData->endSilentCallRspSent = 1;
				sendStopSilentCallRsp(pUeSpecificData->imsi, pUeSpecificData->imsiLen);
			}
        }
		break;

        case UE_RRC_START_UE_BASED_GPS_TIMEOUT:
           buildAndSendRrcMeasurementControlSetup (ueIndex);
           break;

        case UE_RRC_GPS_MEAS_CNTRL_REL_TIMEOUT:
           buildAndSendRrcConnectionRelease (ueIndex, 1);
           break;

#if 0
        case UE_RRC_GPS_MEAS_CNTRL_MODIFY_TIMEOUT:
           buildAndSendRrcMeasurementControlModifyWithReport (ueIndex);
           break;
#endif

        case UE_RRC_GPS_MEAS_CNTRL_FAILURE:
           sendGpsMeasurementFailureInd (ueIndex);
           break;

        case UE_RRC_GPS_MEAS_CNTRL_REPORT:
           sendGpsMeasurementSuccessInd (ueIndex);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_PAGING_TYPE2_TIMEOUT state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : checkUePositioningMethod
 *
 * Description : This procedure checks the UE Silent Call is enabled or not. If
 *               Silent Call is enabled then it will start required measurement.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void checkUePositioningMethod (unsigned int ueIndex)
{
  MeasInfo_t     measInfo;

  unsigned int  *pTimerUeId          = NULL;

  /* Get UE specific data */
  RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

  /* Get Latched UE Details */
  LatchedUeDetails *pCurUe = ueControlBlock.pLatchedUeDetails;

  measInfo.rlId = ueIndex; 

  /* Check that UE is already latched or not */
  while (NULL != pCurUe)
  {
      if (0 == strncmp(pCurUe->imsi, pUeSpecificData->imsi, MAX_IMSI_LENGTH))
      {
          ueControlBlock.ueSpecificData[ueIndex].measType           = pCurUe->measType;
          ueControlBlock.ueSpecificData[ueIndex].silentCallEnabFlag = pCurUe->silentCallEnabFlag;

          switch( pCurUe->measType)
          {
              case FXL_RTT_MEAS:
                 if (0 == pUeSpecificData->isDedMeasStarted)
                 { 
                     measInfo.measType = FXL_RTT_MEAS;
                     sendDedicatedMeasInitReq (measInfo);
                     pUeSpecificData->isDedMeasStarted = 1;
                     pUeSpecificData->dedMeasType = FXL_RTT_MEAS;
                     pUeSpecificData->endSilentCallRspSent = 0;
                 }
                 break;

              case FXL_RSCP_MEAS:
                 if (0 == pUeSpecificData->isDedMeasStarted)
                 { 
                     measInfo.measType = FXL_RSCP_MEAS;
                     sendDedicatedMeasInitReq(measInfo);
                     pUeSpecificData->isDedMeasStarted = 1;
                     pUeSpecificData->dedMeasType = FXL_RSCP_MEAS;
                     pUeSpecificData->endSilentCallRspSent = 0;
                 }
                 break;

              case FXL_RTT_AND_RSCP_AND_GPS_MEAS:
			  	 if (0 == pUeSpecificData->isDedMeasStarted)
			  	 {
				 	 measInfo.measType = FXL_RTT_AND_RSCP_AND_GPS_MEAS;
					 sendDedicatedMeasInitReq(measInfo);
					 pUeSpecificData->isDedMeasStarted = 1;
					 pUeSpecificData->dedMeasType = FXL_RTT_AND_RSCP_AND_GPS_MEAS;
					 pUeSpecificData->endSilentCallRspSent = 0;
					 
                     /* Start the Timer for Start Gps */
                     pTimerUeId  = (unsigned int *)malloc (sizeof(unsigned int));
                     *pTimerUeId = ueIndex;
   
                     timerStart(&pUeSpecificData->ueDedGenericTimer,
                                UE_BASED_GPS_MEASUREMENT_TIMEOUT,
                                0,
                                handleStartGpsTimeout,
                                pTimerUeId); 
              	 }
			     break;
				 
              case FXL_RTT_AND_RSCP_MEAS:
                 if (0 == pUeSpecificData->isDedMeasStarted)
                 {
                     measInfo.measType = FXL_RTT_AND_RSCP_MEAS;
                     sendDedicatedMeasInitReq(measInfo);
                     pUeSpecificData->isDedMeasStarted = 1;
                     pUeSpecificData->dedMeasType = FXL_RTT_AND_RSCP_MEAS;
                     pUeSpecificData->endSilentCallRspSent = 0;
                 }
                 break;

              case FXL_GPS_MEAS:
                 if (0 == pUeSpecificData->isDedMeasStarted)
                 { 
                     pUeSpecificData->isDedMeasStarted = 1;
                     pUeSpecificData->dedMeasType = FXL_GPS_MEAS;

                     /* Start the Timer for Start Gps */
                     pTimerUeId  = (unsigned int *)malloc (sizeof(unsigned int));
                     *pTimerUeId = ueIndex;
   
                     timerStart(&pUeSpecificData->ueDedGenericTimer,
                                UE_BASED_GPS_MEASUREMENT_TIMEOUT,
                                0,
                                handleStartGpsTimeout,
                                pTimerUeId);
                 }
                 break;

              case FXL_NO_MEAS:
                 if (1 == pUeSpecificData->isDedMeasStarted)
                 {
                     pUeSpecificData->isDedMeasStarted = 0;

                     if (FXL_GPS_MEAS == pUeSpecificData->dedMeasType)
                     {
                         buildAndSendRrcMeasurementControlRelease (ueIndex);

                         if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
                         {
                             timerStop(&pUeSpecificData->ueDedGenericTimer);
                         }

                         /* Start the Timer for RRC Connection Release */
                         unsigned int *pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
                         *pTimerUeId = ueIndex;
 
                         timerStart(&pUeSpecificData->ueDedGenericTimer,
                                    UE_BASED_GPS_MEASUREMENT_TIMEOUT,
                                    0,
                                    handleGpsMeasCntrlReleaseTimeout,
                                    pTimerUeId);
                         
                         sendStopSilentCallRsp(pUeSpecificData->imsi, pUeSpecificData->imsiLen);

                     }
                     if ((FXL_RTT_MEAS  == pUeSpecificData->dedMeasType) ||
                         (FXL_RSCP_MEAS ==  pUeSpecificData->dedMeasType) ||
                         (FXL_RTT_AND_RSCP_MEAS ==  pUeSpecificData->dedMeasType))
                     {
                         sendDedicatedMeasTermReq (ueIndex);
                     }

					 if(FXL_RTT_AND_RSCP_AND_GPS_MEAS == pUeSpecificData->dedMeasType)
					 {
					     buildAndSendRrcMeasurementControlRelease (ueIndex);

                         if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
                         {
                             timerStop(&pUeSpecificData->ueDedGenericTimer);
                         }

                         /* Start the Timer for RRC Connection Release */
                         unsigned int *pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
                         *pTimerUeId = ueIndex;
 
                         timerStart(&pUeSpecificData->ueDedGenericTimer,
                                    UE_BASED_GPS_MEASUREMENT_TIMEOUT,
                                    0,
                                    handleGpsMeasCntrlReleaseTimeout,
                                    pTimerUeId);

						 sendDedicatedMeasTermReq (ueIndex);
					 }

                     pUeSpecificData->dedMeasType = FXL_NO_MEAS;
                 }
                 break;

              default: 
                 break; 
          }
 
          break;
       }

       pCurUe = pCurUe->next;
   }
}

/*
 *############################################################################
 * Method Name : handleIdleUeEvent
 *
 * Description : This procedure handles all the events received from UE
 *
 * Parameters  : 
 *   oocImsi      IMSI of the UE which went out of coverage
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleIdleUeEvent(char *oocImsi, RrcUeEvent ueEvent)
{
   switch(ueEvent)
   {
      case UE_OUT_OF_COVERAGE_E:
         delLatchedUe(oocImsi);
         break; 

      default: 
         break; 
   }
   return;
}

/*
 *############################################################################
 * Method Name : sendRrcConnectionReleaseWithRedirectInfo
 *
 * Description : This procedure builds the RRC Connection Release Message with Redirect Info, fills DL DCCH message  
 *                    and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void sendRrcConnectionReleaseWithRedirectInfo(unsigned int          ueIndex, 
                                                       UmtsBlUeRedirectInfo *pBlUeRedirect) 
                                       
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcConnRelLen = 0;
    unsigned char   *pRrcConnRelEncodedData = NULL;
    unsigned int    *pTimerUeId = NULL;
    unsigned int     idx = 0;
    RrcConnectionReleaseData rrcConnRel;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change the UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_RRC_CON_REL_COMPLETE;

	/* Stop the UE Timer */
	if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
    {
	    timerStop(&pUeSpecificData->ueTimer);
    }
	
	/* Stop the Dedicated Meas Timer */
	if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
	{
		timerStop(&pUeSpecificData->ueDedMeasTimer);
	}

	/* Stop the Dedicated Generic Meas Timer */
	if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
	{
	    timerStop(&pUeSpecificData->ueDedGenericTimer);
	}

    /* Start the Timer for RRC Connection Release */
    pTimerUeId  = (unsigned int *)malloc(sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueTimer,
               RRC_CONNECTION_RELEASE_TIMEOUT,
               0,
               handleRrcConnectionReleaseTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcConnRel.laterThanR3         = 1;
        rrcConnRel.rrcTransactionId    = pUeSpecificData->rrcTransactionId;
        rrcConnRel.n308Presence        = VALUE_PRESENT;
        rrcConnRel.n308                = VALUE_PRESENT;
        rrcConnRel.releaseCause        = RELEASE_CAUSE_NORMAL_EVENT;//RELEASE_CAUSE_CONGESTION;
        pUeSpecificData->isBlackListed = 0;
		
        if (FXL_UMTS_REDIRECTION_TO_2G == pBlUeRedirect->redirectionType)
        {
            rrcConnRel.redirectionType  = REDIRECTION_INFO_INTER_RAT;
            rrcConnRel.numOfGsmCell     = 1;
			
            /* Fill GSM ARFCN */
            rrcConnRel.gsmInfo[0].arfcn = 
                              pBlUeRedirect->redirectionInfo.gsmRedirectinfo.startingarfcn;
			
            if (DCS1800 == pBlUeRedirect->redirectionInfo.gsmRedirectinfo.bandIndicator)
            {
                rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1800;
            }
            else
            {
                rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1900;
            }

            /* Fill GSM BSIC Presence */
            rrcConnRel.gsmInfo[idx].bsicPresence = 0;

            /* Fill GSM BSIC NCC */
            rrcConnRel.gsmInfo[idx].bsicNcc = 0;

            /* Fill GSM BSIC BCC */
            rrcConnRel.gsmInfo[idx].bsicBcc = 0;
        }
        else if (FXL_UMTS_REDIRECTION_TO_3G == pBlUeRedirect->redirectionType)
        {
            rrcConnRel.redirectionType = REDIRECTION_INFO_INTER_FREQUENCY;
            rrcConnRel.dlUarfcn        = pBlUeRedirect->redirectionInfo.dlUarfcn;
        }
        else
        {
           /* Do Nothing */
        }

        if (ASN_SUCCESS == fillAndEncodeRrcConnectionRelease (&rrcConnRel,
                                                              &pRrcConnRelEncodedData,
                                                              &rrcConnRelLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcConnRelLen;
            pRlcDataReq->rrcPdu            = pRrcConnRelEncodedData;

            DEBUGMSG(("RRC Connection Release is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                              (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        } 
        else
        {
            DEBUG1((" Encoding RRC Connection Release is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Connection Release is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : sendBlUeRedirectRsp
 *
 * Description    : This procedure sends BL UE Redirect Response to external client.         
 *
 * Parameters  : 
 *   success     0 Failure 1 Success
 *
 * Returns       :  None
 *############################################################################
 */
void sendBlUeRedirectRsp (unsigned char success, char *pImsiStr)
{
    fxL3gBLUeRedirectRsp redirectBlUeRsp;

    /* Send Change UE UL Tx Pwr Response to External Client */
    memset((fxL3gBLUeRedirectRsp *)&redirectBlUeRsp, 0,
           sizeof(fxL3gBLUeRedirectRsp));

    redirectBlUeRsp.hdr.msgLength = sizeof(fxL3gBLUeRedirectRsp);
    redirectBlUeRsp.hdr.rat       = FXL_RAT_3G;
    redirectBlUeRsp.hdr.msgType   = FXL_3G_BL_UE_REDIRECT_RSP;
 
    if (1 == success)
    {
        redirectBlUeRsp.result = FXL_SUCCESS;
    }
    else
    {
        redirectBlUeRsp.result = FXL_FAILURE;
    }

    strncpy(redirectBlUeRsp.imsiStr, pImsiStr, SIZE_OF_IDENTITY_STR);

    if (-1 != clientFd)
    {
        if (send(clientFd, &redirectBlUeRsp, sizeof(fxL3gBLUeRedirectRsp), 0) < 0)
        {
            DEBUG1(("Send BL UE Redirect Response is failed\n"));
        }
        else
        {
            DEBUGMSG(("Send BL UE Redirect Response is sent to External Client\n"));
        }
    }
    else
    {
        DEBUG1(("Send BL UE Redirect Response is failed, clinetFd = -1\n"));
    }
}

/*
 *############################################################################
 * Method Name :  handleRrcBlUeRedirect
 *
 * Description     :  This procedure redirects BL UE to a particular channel received from external client.
 *
 * Parameters    : 
 *   UmtsBlUeRedirectInfo      BL UE Redirection Information
 *
 * Returns          :      None
 *############################################################################
 */
void handleRrcBlUeRedirect (UmtsBlUeRedirectInfo *pBlUeRedirect)
{
    unsigned int ueIndex = 0xFFFFFFFF;

    /* Get UE Index from IMSI Value */
    ueIndex = getUeIndexFromImsi (pBlUeRedirect->imsiStr);

    if(0xFFFFFFFF != ueIndex)
    {
        if (FXL_UMTS_NO_REDIRECTION != pBlUeRedirect->redirectionType)
        {
        	sendRrcConnectionReleaseWithRedirectInfo (ueIndex, pBlUeRedirect);
        }

        /* Send SUCESS Response to the External Client */
        sendBlUeRedirectRsp (1, pBlUeRedirect->imsiStr);	
    }
    else
    {
        /* Send Failure Response to the External Client */
        sendBlUeRedirectRsp (0, pBlUeRedirect->imsiStr);
    }

    /* Clear the Memory Allocated */
	if (NULL != pBlUeRedirect)
	{
    	free(pBlUeRedirect);
    	pBlUeRedirect = NULL;
	}
}

/*
 *############################################################################
 * Method Name :  handleRrcChangeUeUlTxPwr
 *
 * Description :     This procedure handles the chamge UE Tx Pwr received from external client.
 *
 * Parameters  : 
 *   UeUlTxPwrData      UE UL Tx Power Data
 *
 * Returns     :      None
 *############################################################################
 */
void handleRrcChangeUeUlTxPwr (UeUlTxPwrData *pUeUlTxPwr)
{
    unsigned int ueIndex = 0xFFFFFFFF;

    /* Get UE Index from IMSI Value */
    ueIndex = getUeIndexFromImsi (pUeUlTxPwr->imsiStr);

    if(0xFFFFFFFF != ueIndex)
    {
        buildAndSendRrcPhyChReconfWithMaxAllowedUeTxPwr (ueIndex, 
                                                         pUeUlTxPwr->maxUlTxPwr);
    }
    else
    {
        /* Send Failure Response to the External Client */
        sendChangeUeUlTxPwrRsp (0, pUeUlTxPwr->imsiStr);
    }

    /* Clear the Memory Allocated */
	if (NULL != pUeUlTxPwr)
	{
    	free(pUeUlTxPwr);
    	pUeUlTxPwr = NULL;
	}
}


/*
 *############################################################################
 * Method Name : buildAndSendRrcPhyChReconfWithMaxAllowedUeTxPwr
 *
 * Description : This procedure builds the RRC Paging Type2 Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcPhyChReconfWithMaxAllowedUeTxPwr (unsigned int ueIndex,
                                                      int          maxUlTxPwr) 
{
    RrcDataReq        *pRlcDataReq = NULL;
    unsigned int       rrcPhyChReconfLen = 0;
    unsigned char     *pRrcPhyChReconfData = NULL;
    unsigned int      *pTimerUeId = NULL;
    RrcPhyChReconfigData rrcPhyChReconf;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
    {
        char imsiValue[MAX_IMSI_LENGTH + 1] = {0};

        /* Get IMSI Details */
        strncpy (imsiValue, pUeSpecificData->imsi,
                 pUeSpecificData->imsiLen);
        imsiValue[pUeSpecificData->imsiLen] = '\0';

        DEBUG1(("Dedicated mode generic timer is already running, sending failure message to external client\n"));

        /* Send Failure Response to the External Client */
        sendChangeUeUlTxPwrRsp (0, imsiValue);
    }
    else
    {
        /* Change the UE State */
        pUeSpecificData->ueState = UE_WAIT_FOR_PHY_CH_RECONF_RESP;

        /* Start the Timer for RRC Physical Channel Reconfig */
        pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
        *pTimerUeId = ueIndex;
 
        timerStart(&pUeSpecificData->ueDedGenericTimer,
                   UE_DEDICATED_MODE_GENERIC_TIMEOUT,
                   0,
                   handlePhyChReconfTimeout,
                   pTimerUeId);

        /* Initialize ASN Context */
        if (ASN_SUCCESS == initializeASNContext ())
        {
            /* Fill RRC Physical Channel Reconfig Information */
            rrcPhyChReconf.rrcTransactionId   = pUeSpecificData->rrcTransactionId;
            rrcPhyChReconf.maxUlTxPwr         = maxUlTxPwr;

            if (ASN_SUCCESS == fillAndEncodePhyChReconfig (&rrcPhyChReconf,
                                                           &pRrcPhyChReconfData,
                                                           &rrcPhyChReconfLen))
            {
                /* Fill RLC Data Indication */
                pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
                pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
                pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
                pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
                pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
                pRlcDataReq->rrcPduLen         = rrcPhyChReconfLen;
                pRlcDataReq->rrcPdu            = pRrcPhyChReconfData;

                DEBUGMSG(("RRC Physical Channel Reconfig for changing UE UL Tx Pwr is sent\n"));

                /* Post DL CCCH Message to L2 DL DATA Task */
                SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                               L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

                /* Increament RRC Transaction Id */
                ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                                 (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
            } 
            else
            {
                DEBUG1((" Encoding RRC Physical Channel Reconfig is for changing UE UL Tx Pwr failed\n"));
            }

            /* Clean ASN Context */
            cleanASNContext ();
        }
        else
        {
            DEBUG1(("Initialization of ASN for RRC Physical Channel Reconfig for changing UE UL Tx Pwr is failed\n")); 
        }
    }
}

/*
 *############################################################################
 * Method Name : sendChangeUeUlTxPwrRsp
 *
 * Description : This procedure sends change UE UL Tx Pwr Response to external
 *               client. 
 *
 * Parameters  : 
 *   success     0 Failure 1 Success
 *
 * Returns     :  None
 *############################################################################
 */
void sendChangeUeUlTxPwrRsp (unsigned char success, char *pImsiStr)
{
    fxL3gChangeUeUlTxPwrRsp changeUeUlTxPwr;

    /* Send Change UE UL Tx Pwr Response to External Client */
    memset((fxL3gChangeUeUlTxPwrRsp *)&changeUeUlTxPwr, 0,
           sizeof(fxL3gChangeUeUlTxPwrRsp));

    changeUeUlTxPwr.hdr.msgLength = sizeof(fxL3gChangeUeUlTxPwrRsp);
    changeUeUlTxPwr.hdr.rat       = FXL_RAT_3G;
    changeUeUlTxPwr.hdr.msgType   = FXL_3G_CHANGE_UE_UL_TX_PWR_RSP;
 
    if (1 == success)
    {
        changeUeUlTxPwr.result = FXL_SUCCESS;
    }
    else
    {
        changeUeUlTxPwr.result = FXL_FAILURE;
    }

    strncpy(changeUeUlTxPwr.imsiStr, pImsiStr, SIZE_OF_IDENTITY_STR);

    if (-1 != clientFd)
    {
        if (send(clientFd, &changeUeUlTxPwr, sizeof(fxL3gChangeUeUlTxPwrRsp), 0) < 0)
        {
            DEBUG1(("Send Change UE UL Tx Pwr Response is failed\n"));
        }
        else
        {
            DEBUGMSG(("Change UE UL Tx Pwr Response is sent to External Client\n"));
        }
    }
    else
    {
        DEBUG1(("Send Change UE UL Tx Pwr Response is failed, clinetFd = -1\n"));
    }
}

/*
 *############################################################################
 * Method Name : handleUeEventInWaitPhyChReconfRspState
 *
 * Description : This procedure handles all the events received in Wait Physical
 *               Channel Reconfig Response state.
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *   ueEvent      Event received from UE
 *
 * Returns     :  None
 *############################################################################
 */
void handleUeEventInWaitPhyChReconfRspState (unsigned int ueIndex, 
                                             RrcUeEvent   ueEvent)
{
    fxL3gDedMeasValueInd measValue;
    unsigned int *pTimerUeId = NULL;
    char imsiValue[MAX_IMSI_LENGTH + 1] = {0};
//    time_t timeSec;
//    timeSec = time (NULL);

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Get IMSI Details */
    strncpy (imsiValue, pUeSpecificData->imsi,
             pUeSpecificData->imsiLen);
    imsiValue[pUeSpecificData->imsiLen] = '\0';

    switch (ueEvent)
    {
        case UE_RRC_PHY_CH_RECONF_COMP_RCVD:
        case UE_RRC_PHY_CH_RECONF_FAIL_RCVD:
        case UE_RRC_PHY_CH_RECONF_TIMEOUT:
        {
            /* Stop Physical Channel Reconfig Timer */
            if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
            {
               timerStop(&pUeSpecificData->ueDedGenericTimer);
            }

            if (UE_RRC_PHY_CH_RECONF_COMP_RCVD == ueEvent)
            {
                /* Send Failure Response to the External Client */
                sendChangeUeUlTxPwrRsp (1, imsiValue);
            }
            else
            {
                /* Send Failure Response to the External Client */
                sendChangeUeUlTxPwrRsp (0, imsiValue);
            }

            /* Change the UE State */
            pUeSpecificData->ueState = UE_WAIT_FOR_PAGING_TYPE2_TIMEOUT;
        }
        break;

        case UE_RRC_PAGING_TYPE2_TIMEOUT:
        {
           DEBUGMSG(("Paging Type2 Timeout..Restart the Paging Type2 Timer\n"));

           /* Stop the Paging Type2 Timer */
           if (TRUE != isTimerRunning (&pUeSpecificData->ueTimer))
           {
                /* Start the Timer for RRC Connection Release */
                pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
                *pTimerUeId = ueIndex;
 
                timerStart(&pUeSpecificData->ueTimer,
                           500,
                           0,
                           handlePagingType2Timeout,
                           pTimerUeId);
           }
        }
        break;
        
        case UE_DED_MEAS_RTT_REPORT_SUCCESS:
        {
           measValue.hdr.msgLength = sizeof(fxL3gDedMeasValueInd);
           measValue.hdr.msgType = FXL_3G_DED_MEAS_VALUE_IND;
           measValue.hdr.rat = FXL_RAT_3G;

           strncpy (measValue.imsiStr, pUeSpecificData->imsi,
                    pUeSpecificData->imsiLen);
           measValue.imsiStr[pUeSpecificData->imsiLen] = '\0';

           measValue.measType = pUeSpecificData->measType;
//           measValue.rttValue = convertRttInTime (pUeSpecificData->rttValue);       
           measValue.rttValue = convertRttInChip (pUeSpecificData->rttValue);       
           measValue.rscpValue = 0;
//           measValue.bfn       = gClCntxt.bfn;
//           measValue.timeSec   = (unsigned int )timeSec;

           if (-1 != clientFd)
           {
               if (send(clientFd, &measValue, sizeof(measValue), 0) < 0)
               {
                  DEBUG1(("Unable to send Message to External Client\n"));
               }
               else
               {
                  DEBUGMSG(("Dedicated Meas value Indication is sent to External Client\n"));
               }
           }
           else
           {
               DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
           }
        } 
        break;

        case UE_DED_MEAS_RSCP_REPORT_SUCCESS:
        {
           measValue.hdr.msgLength = sizeof(fxL3gDedMeasValueInd);
           measValue.hdr.msgType = FXL_3G_DED_MEAS_VALUE_IND;
           measValue.hdr.rat = FXL_RAT_3G;
 
           strncpy (measValue.imsiStr, pUeSpecificData->imsi,
                    pUeSpecificData->imsiLen);
           measValue.imsiStr[pUeSpecificData->imsiLen] = '\0';
 
           measValue.measType = pUeSpecificData->measType;
           measValue.rttValue = 0;
           measValue.rscpValue = convertRscpInDbm (pUeSpecificData->rscpValue);
//           measValue.bfn       = gClCntxt.bfn;
//           measValue.timeSec   = (unsigned int )timeSec;
        
           if (-1 != clientFd)
           {
              if (send(clientFd, &measValue, sizeof(measValue), 0) < 0)
              {
                 DEBUG1(("Unable to send Message to External Client\n"));
              }
              else
              {
                 DEBUGMSG(("Dedicated Meas value Indication is sent to External Client\n"));
              }
           }
           else
           {
              DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
           }
        }
        break;

        case L2_RLC_ERROR_CALL_RLS_E:
        case UE_RL_SETUP_FAILURE:
        {
           /* Stop the UE Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueTimer))
           {
               timerStop(&pUeSpecificData->ueTimer);
           }

           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }

           /* Stop the Dedicated Generic Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedGenericTimer))
           {
               timerStop(&pUeSpecificData->ueDedGenericTimer);
           }

           /* Send RL Delete Req to L1 */
           sendRadioLinkpDeleteReqToL1 (ueIndex);
        }
        break; 

        case UE_DED_MEAS_RTT_INIT_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_INIT_REQ_SUCCESS:
        {
           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }
        }
        break;

        case UE_DED_MEAS_RTT_TERM_REQ_SUCCESS:
        case UE_DED_MEAS_RSCP_TERM_REQ_SUCCESS:
        {
           /* Stop the Dedicated Meas Timer */
           if (TRUE == isTimerRunning (&pUeSpecificData->ueDedMeasTimer))
           {
               timerStop(&pUeSpecificData->ueDedMeasTimer);
           }

           if(pUeSpecificData->endSilentCallRspSent == 0)
           {
               pUeSpecificData->endSilentCallRspSent = 1;
               sendStopSilentCallRsp(pUeSpecificData->imsi, pUeSpecificData->imsiLen);
           }
        }
        break;

        case UE_DED_MEAS_INIT_REQ_TIMEOUT:
			//Do Nothing
			break;

        case UE_DED_MEAS_TERM_REQ_TIMEOUT:
		{
			if(pUeSpecificData->endSilentCallRspSent == 0)
			{
				pUeSpecificData->endSilentCallRspSent = 1;
				sendStopSilentCallRsp(pUeSpecificData->imsi, pUeSpecificData->imsiLen);
			}
        }
        break;

        case UE_RRC_START_UE_BASED_GPS_TIMEOUT:
           buildAndSendRrcMeasurementControlSetup (ueIndex);
           break;

        case UE_RRC_GPS_MEAS_CNTRL_REL_TIMEOUT:
           buildAndSendRrcConnectionRelease (ueIndex, 1);
           break;

#if 0
        case UE_RRC_GPS_MEAS_CNTRL_MODIFY_TIMEOUT:
           buildAndSendRrcMeasurementControlModifyWithReport (ueIndex);
           break;
#endif

        case UE_RRC_GPS_MEAS_CNTRL_FAILURE:
           sendGpsMeasurementFailureInd (ueIndex);
           break;

        case UE_RRC_GPS_MEAS_CNTRL_REPORT:
           sendGpsMeasurementSuccessInd (ueIndex);
           break;

        default:
           DEBUG2(("UE event(%ld) is not handled in UE_WAIT_FOR_PHY_CH_RECONF_RESP state\n", 
                  ueEvent));
           break;
    }
}

/*
 *############################################################################
 * Method Name : handlePhyChReconfTimeout
 *
 * Description : This procedure handles Physical Channel Reconfig Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handlePhyChReconfTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("Physical Channel Reconfig Request Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, RRC_PHY_CH_RECONF_TIMEOUT_E, pUeId); 
}

/*
 *############################################################################
 * Method Name : handleStartGpsTimeout
 *
 * Description : This procedure handles Start GPS Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleStartGpsTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("Start Ue Based Gps Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, 
                  RRC_START_GPS_TIMEOUT_E, pUeId);
}
/*
 *############################################################################
 * Method Name : handleGpsMeasCntrlReleaseTimeout
 *
 * Description : This procedure handles GPS Measurement Control Release Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleGpsMeasCntrlReleaseTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("GPS Measurement Control Release Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, 
                  RRC_GPS_MEAS_CNTRL_REL_TIMEOUT_E, pUeId);
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcMeasurementControlSetup
 *
 * Description : This procedure builds the RRC Measurement Control Setup Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcMeasurementControlSetup (unsigned int ueIndex) 
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcMeasCtrlLen = 0;
    unsigned char   *pRrcMeasCtrlEncodedData = NULL;
    unsigned int    *pTimerUeId = NULL;
    unsigned int     idx = 0;
    RrcMeasurementControlData rrcMeasCtrl;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcMeasCtrl.rrcTransactionId                        = pUeSpecificData->rrcTransactionId;
        rrcMeasCtrl.measurementId                           = UE_MEASUREMENT_ID;
        rrcMeasCtrl.measurementCommand                      = RRC_MEASUREMENT_CONTROL_COMMAND_SETUP;
        rrcMeasCtrl.reportingCriteriaType                   = RRC_MEAS_CNTRL_REPORTING_CRITERIA_PERIODICAL;
        rrcMeasCtrl.reportCriteria.isReportingAmountPresent = 1;
        rrcMeasCtrl.reportCriteria.reportingAmount          = RRC_MEAS_CNTRL_REPORTING_AMOUNT_INFINITY;
        rrcMeasCtrl.reportCriteria.reportingInterval        = RRC_MEAS_CNTRL_REPORTING_INTERVAL_0_25;
		
		if (FXL_UE_BASED_GPS_SUPPORT == pUeSpecificData->networkAssistedGpsSupport)
		{
		    rrcMeasCtrl.isUePositioningGpsAssitancePresent      = 0;
            rrcMeasCtrl.isUePositioningGpsRefTimePresent        = 0;
            rrcMeasCtrl.gpsRefTime.gpsWeek                      = 0;
            rrcMeasCtrl.gpsRefTime.gpsTow                       = 0;
            rrcMeasCtrl.gpsRefTime.isUtranGpsRefTimePresent     = 0;
            rrcMeasCtrl.isUePositioningNavigationModelPresent   = 0;
            rrcMeasCtrl.gpsNavMod.numOfSat                      = MAX_SAT_ID;

            for (idx = 0; idx < MAX_SAT_ID; idx++)
            {
                rrcMeasCtrl.gpsNavMod.satId[idx] = 0;
            }
		}
		else
		{
            rrcMeasCtrl.isUePositioningGpsAssitancePresent      = 1;
            rrcMeasCtrl.isUePositioningGpsRefTimePresent        = 1;
            rrcMeasCtrl.gpsRefTime.gpsWeek                      = gUmtsConfig.gpsWeek;
            rrcMeasCtrl.gpsRefTime.gpsTow                       = gUmtsConfig.gpsTow;
            rrcMeasCtrl.gpsRefTime.isUtranGpsRefTimePresent     = 0;
            rrcMeasCtrl.isUePositioningNavigationModelPresent   = 1;
            rrcMeasCtrl.gpsNavMod.numOfSat                      = MAX_SAT_ID;

            for (idx = 0; idx < rrcMeasCtrl.gpsNavMod.numOfSat; idx++)
            {
                rrcMeasCtrl.gpsNavMod.satId[idx] = idx + 1;
            }
		}

        if (ASN_SUCCESS == fillAndEncodeRrcMeasurementControl (&rrcMeasCtrl,
                                                               &pRrcMeasCtrlEncodedData,
                                                               &rrcMeasCtrlLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcMeasCtrlLen;
            pRlcDataReq->rrcPdu            = pRrcMeasCtrlEncodedData;

            DEBUGMSG(("RRC Measurement Control Setup is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                        (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        }
        else
        {
            DEBUG1((" Encoding RRC Measurement Control Setup is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Measurement Control Setup is failed\n")); 
    }
}

#if 0
/*
 *############################################################################
 * Method Name : handleGpsMeasCntrlModifyTimeout
 *
 * Description : This procedure handles GPS Measurement Control Modify Timeout. 
 *
 * Parameters  : 
 *   pTimer       Pointer to the UE timer
 *   pUeIndex     Pointer to the UE Index
 *
 * Returns     :  None
 *############################################################################
 */
void handleGpsMeasCntrlModifyTimeout (tcbTimer_t *pTimer, void *pUeIndex)
{
    /* Get UE Id */
    unsigned int *pUeId = (unsigned int *)pUeIndex;

    DEBUGMSG(("GPS Measurement Control Modify Timeout for UE Id(%ld)\n", *pUeId));

    /* Post Radio Link Setup Timeout to APP RRC Task */
    SendMsgToTask(CL_L1C_RCV_TASK_E, APP_RRC_TASK_E, 
                  RRC_GPS_MEAS_CNTRL_MODIFY_TIMEOUT_E, pUeId);
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcMeasurementControlModifyNoReport
 *
 * Description : This procedure builds the RRC Measurement Control Modify Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcMeasurementControlModifyNoReport (unsigned int ueIndex) 
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcMeasCtrlLen = 0;
    unsigned char   *pRrcMeasCtrlEncodedData = NULL;
    unsigned int    *pTimerUeId = NULL;
    unsigned int     idx = 0;
    RrcMeasurementControlData rrcMeasCtrl;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Start the Timer for RRC Connection Release */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;
 
    timerStart(&pUeSpecificData->ueDedMeasTimer,
               UE_BASED_GPS_MEASUREMENT_TIMEOUT,
               0,
               handleGpsMeasCntrlModifyTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcMeasCtrl.rrcTransactionId                        = pUeSpecificData->rrcTransactionId;
        rrcMeasCtrl.measurementId                           = UE_MEASUREMENT_ID;
        rrcMeasCtrl.measurementCommand                      = RRC_MEASUREMENT_CONTROL_COMMAND_MODIFY;
        rrcMeasCtrl.reportingCriteriaType                   = RRC_MEAS_CNTRL_REPORTING_CRITERIA_NO_REPORT;
        rrcMeasCtrl.reportCriteria.isReportingAmountPresent = 0;
        rrcMeasCtrl.reportCriteria.reportingAmount          = 0;
        rrcMeasCtrl.reportCriteria.reportingInterval        = 0;
        rrcMeasCtrl.isUePositioningGpsAssitancePresent      = 0;
        rrcMeasCtrl.isUePositioningGpsRefTimePresent        = 0;
        rrcMeasCtrl.gpsRefTime.gpsWeek                      = 0;//gUmtsConfig.gpsWeek;
        rrcMeasCtrl.gpsRefTime.gpsTow                       = 0;//gUmtsConfig.gpsTow;
        rrcMeasCtrl.gpsRefTime.isUtranGpsRefTimePresent     = 0;
        rrcMeasCtrl.isUePositioningNavigationModelPresent   = 0;
        rrcMeasCtrl.gpsNavMod.numOfSat                      = 0;

        for (idx = 0; idx < MAX_SAT_ID; idx++)
        {
            rrcMeasCtrl.gpsNavMod.satId[idx] = 0;
        }

        if (ASN_SUCCESS == fillAndEncodeRrcMeasurementControl (&rrcMeasCtrl,
                                                               &pRrcMeasCtrlEncodedData,
                                                               &rrcMeasCtrlLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcMeasCtrlLen;
            pRlcDataReq->rrcPdu            = pRrcMeasCtrlEncodedData;

            DEBUGMSG(("RRC Measurement Control Modify With No Report is sent\n"));


            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                               (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        }
        else
        {
            DEBUG1((" Encoding RRC Measurement Control Modify With No Report is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Measurement Control Modify With No Report is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcMeasurementControlModifyWithReport
 *
 * Description : This procedure builds the RRC Measurement Control Modify Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcMeasurementControlModifyWithReport (unsigned int ueIndex) 
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcMeasCtrlLen = 0;
    unsigned char   *pRrcMeasCtrlEncodedData = NULL;
    unsigned int     idx = 0;
    RrcMeasurementControlData rrcMeasCtrl;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcMeasCtrl.rrcTransactionId                        = pUeSpecificData->rrcTransactionId;
        rrcMeasCtrl.measurementId                           = UE_MEASUREMENT_ID;
        rrcMeasCtrl.measurementCommand                      = RRC_MEASUREMENT_CONTROL_COMMAND_MODIFY;
        rrcMeasCtrl.reportingCriteriaType                   = RRC_MEAS_CNTRL_REPORTING_CRITERIA_PERIODICAL;
        rrcMeasCtrl.reportCriteria.isReportingAmountPresent = 1;
        rrcMeasCtrl.reportCriteria.reportingAmount          = RRC_MEAS_CNTRL_REPORTING_AMOUNT_INFINITY;
        rrcMeasCtrl.reportCriteria.reportingInterval        = RRC_MEAS_CNTRL_REPORTING_INTERVAL_0_25;
        rrcMeasCtrl.isUePositioningGpsAssitancePresent      = 1;
        rrcMeasCtrl.isUePositioningGpsRefTimePresent        = 1;
        rrcMeasCtrl.gpsRefTime.gpsWeek                      = gUmtsConfig.gpsWeek;
        rrcMeasCtrl.gpsRefTime.gpsTow                       = gUmtsConfig.gpsTow;
        rrcMeasCtrl.gpsRefTime.isUtranGpsRefTimePresent     = 0;
        rrcMeasCtrl.isUePositioningNavigationModelPresent   = 1;
        rrcMeasCtrl.gpsNavMod.numOfSat                      = 1;

        for (idx = 0; idx < MAX_SAT_ID; idx++)
        {
            rrcMeasCtrl.gpsNavMod.satId[idx] = idx + 3;
        }

        if (ASN_SUCCESS == fillAndEncodeRrcMeasurementControl (&rrcMeasCtrl,
                                                               &pRrcMeasCtrlEncodedData,
                                                               &rrcMeasCtrlLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcMeasCtrlLen;
            pRlcDataReq->rrcPdu            = pRrcMeasCtrlEncodedData;

            DEBUGMSG(("RRC Measurement Control Modify With Report is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId = 
                          (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        }
        else
        {
            DEBUG1((" Encoding RRC Measurement Control Modify With Report is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Measurement Control Modify With Report is failed\n")); 
    }
}
#endif

/*
 *############################################################################
 * Method Name : sendGpsMeasurementSuccessInd
 *
 * Description : This procedure sends GPS Measurement Success Indication to
 *               External Client.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void sendGpsMeasurementSuccessInd (unsigned int ueIndex) 
{
    fxl3gUeBasedGpsMeasInd ueBasedGpsInd;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    ueBasedGpsInd.hdr.msgLength = sizeof(fxl3gUeBasedGpsMeasInd);
    ueBasedGpsInd.hdr.msgType   = FXL_3G_UE_BASED_GPS_MEAS_IND;
    ueBasedGpsInd.hdr.rat       = FXL_RAT_3G;
    ueBasedGpsInd.result        = FXL_SUCCESS;
 
    strncpy (ueBasedGpsInd.imsiStr, pUeSpecificData->imsi,
             pUeSpecificData->imsiLen);
    ueBasedGpsInd.imsiStr[pUeSpecificData->imsiLen] = '\0';
        

    if (0 == pUeSpecificData->ulDcchData.measReportErr)
    {
        ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NONE;
   
        ueBasedGpsInd.uePosInfo.gpsLocation.latitude = (float)((float)pUeSpecificData->ulDcchData.uePosInfo.latitude * latconst2 / latconst1);
        if (pUeSpecificData->ulDcchData.uePosInfo.latitudeSign == LATITUDE_SIGN_SOUTH)
        {
           ueBasedGpsInd.uePosInfo.gpsLocation.latitude *= - 1; /* change sign */
        }

        ueBasedGpsInd.uePosInfo.gpsLocation.longitude = (float)((float)pUeSpecificData->ulDcchData.uePosInfo.longitude * lonconst2 / lonconst1);
        /* + 180 to -180 format instead of 0 to 360 format */
        if(ueBasedGpsInd.uePosInfo.gpsLocation.longitude < 0)
        {
            if(ueBasedGpsInd.uePosInfo.gpsLocation.longitude < -180)
            {
                ueBasedGpsInd.uePosInfo.gpsLocation.longitude = (float)(ueBasedGpsInd.uePosInfo.gpsLocation.longitude + 360);
            }
        }
        else
        {
            if(ueBasedGpsInd.uePosInfo.gpsLocation.longitude > 180)
            {
                ueBasedGpsInd.uePosInfo.gpsLocation.longitude = (float)(ueBasedGpsInd.uePosInfo.gpsLocation.longitude - 360);
            }
        }

        if(pUeSpecificData->ulDcchData.uePosInfo.altitudePresence)
        {
            ueBasedGpsInd.uePosInfo.gpsFixType = FXL_GPS_FIX_3D;

            ueBasedGpsInd.uePosInfo.gpsLocation.altitude = pUeSpecificData->ulDcchData.uePosInfo.altitude;
            if(pUeSpecificData->ulDcchData.uePosInfo.altitudeSign == ALTITUDE_SIGN_DEPTH)
            {
                ueBasedGpsInd.uePosInfo.gpsLocation.altitude *= -1; /* change sign */
            }
        }
        else
        {
            ueBasedGpsInd.uePosInfo.gpsFixType           = FXL_GPS_FIX_2D;
            ueBasedGpsInd.uePosInfo.gpsLocation.altitude = 0;
        }
    }
    else
    {
        switch (pUeSpecificData->ulDcchData.measReportErrCause)
        {
            case UE_POS_ERR_NOT_ENOUGH_OTDOA_CELLS:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NOT_ENOUGH_OTDOA_CELLS;
               break;
    
            case UE_POS_ERR_NOT_ENOUGH_GPS_SATELLITE:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NOT_ENOUGH_GPS_SATELLITE;
               break;

            case UE_POS_ERR_ASSISTANCE_DATA_MISSING:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_ASSISTANCE_DATA_MISSING;
               break;

            case UE_POS_ERR_NOT_ACCOMPLISHED_GPS:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NOT_ACCOMPLISHED_GPS;
               break;

            case UE_POS_ERR_UNDEFINED:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_UNDEFINED;
               break;

            case UE_POS_ERR_REQ_DENIED_BY_USER:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_REQ_DENIED_BY_USER;
               break;

            case UE_POS_ERR_NOT_PROCESSED_TIMEOUT:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NOT_PROCESSED_TIMEOUT;
               break;

            case UE_POS_ERR_REF_CELL_NOT_SERVING_CELL:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_REF_CELL_NOT_SERVING_CELL;
               break;

            default:
               ueBasedGpsInd.errorCode = FXL_UE_POS_ERR_NONE;
               break;
        }

        ueBasedGpsInd.uePosInfo.gpsFixType = FXL_GPS_FIX_NONE;
        ueBasedGpsInd.uePosInfo.gpsLocation.latitude  = 0;
        ueBasedGpsInd.uePosInfo.gpsLocation.longitude = 0;
        ueBasedGpsInd.uePosInfo.gpsLocation.altitude  = 0;
    }


    if (-1 != clientFd)
    {
        if (send(clientFd, &ueBasedGpsInd, sizeof(ueBasedGpsInd), 0) < 0)
        {
            DEBUG1(("Unable to send UE Based GPS Ind Message to External Client\n"));
        }
        else
        {
            DEBUGMSG(("UE Based GPS Ind is sent to External Client\n"));
        }
    }
    else
    {
        DEBUG1(("Unable to send UE Based GPS Ind to External Client, clientFd = -1\n"));
    }

    memset ((RrcUlDCCHData *)&pUeSpecificData->ulDcchData, 0, sizeof(RrcUlDCCHData));
}

/*
 *############################################################################
 * Method Name : sendGpsMeasurementFailureInd
 *
 * Description : This procedure sends GPS Measurement Failure Indication to
 *               External Client.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void sendGpsMeasurementFailureInd (unsigned int ueIndex) 
{
    fxl3gUeBasedGpsMeasInd ueBasedGpsInd;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    ueBasedGpsInd.hdr.msgLength = sizeof(fxl3gUeBasedGpsMeasInd);
    ueBasedGpsInd.hdr.msgType   = FXL_3G_UE_BASED_GPS_MEAS_IND;
    ueBasedGpsInd.hdr.rat       = FXL_RAT_3G;
    ueBasedGpsInd.result        = FXL_FAILURE;
 
    strncpy (ueBasedGpsInd.imsiStr, pUeSpecificData->imsi,
             pUeSpecificData->imsiLen);
    ueBasedGpsInd.imsiStr[pUeSpecificData->imsiLen] = '\0';
        
    ueBasedGpsInd.errorCode                        = FXL_UE_POS_ERR_NONE;

    ueBasedGpsInd.uePosInfo.gpsFixType             = FXL_GPS_FIX_NONE;
    ueBasedGpsInd.uePosInfo.gpsLocation.latitude   = 0;
    ueBasedGpsInd.uePosInfo.gpsLocation.longitude  = 0;
    ueBasedGpsInd.uePosInfo.gpsLocation.altitude   = 0;

    if (-1 != clientFd)
    {
        if (send(clientFd, &ueBasedGpsInd, sizeof(ueBasedGpsInd), 0) < 0)
        {
            DEBUG1(("Unable to send UE Based GPS Ind Message to External Client\n"));
        }
        else
        {
            DEBUGMSG(("UE Based GPS Ind is sent to External Client\n"));
        }
    }
    else
    {
        DEBUG1(("Unable to send UE Based GPS Ind to External Client, clientFd = -1\n"));
    }

    memset ((RrcUlDCCHData *)&pUeSpecificData->ulDcchData, 0, sizeof(RrcUlDCCHData));
}

/*
 *############################################################################
 * Method Name : buildAndSendRrcMeasurementControlRelease
 *
 * Description : This procedure builds the RRC Measurement Control Release Message, 
 *               fills DL DCCH message and posts RRC PDU to RLC Layer.
 *               
 *
 * Parameters  : 
 *   ueIndex      Index of the UE in UE Control Block
 *
 * Returns     :  None
 *############################################################################
 */
void buildAndSendRrcMeasurementControlRelease (unsigned int ueIndex) 
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcMeasCtrlLen = 0;
    unsigned char   *pRrcMeasCtrlEncodedData = NULL;
    unsigned int     idx = 0;
    RrcMeasurementControlData rrcMeasCtrl;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcMeasCtrl.rrcTransactionId                        = pUeSpecificData->rrcTransactionId;
        rrcMeasCtrl.measurementId                           = UE_MEASUREMENT_ID;
        rrcMeasCtrl.measurementCommand                      = RRC_MEASUREMENT_CONTROL_COMMAND_RELEASE;
        rrcMeasCtrl.reportingCriteriaType                   = 0;
        rrcMeasCtrl.reportCriteria.isReportingAmountPresent = 0;
        rrcMeasCtrl.reportCriteria.reportingAmount          = 0;
        rrcMeasCtrl.reportCriteria.reportingInterval        = 0;
        rrcMeasCtrl.isUePositioningGpsAssitancePresent      = 0;
        rrcMeasCtrl.isUePositioningGpsRefTimePresent        = 0;
        rrcMeasCtrl.gpsRefTime.gpsWeek                      = 0;
        rrcMeasCtrl.gpsRefTime.gpsTow                       = 0;
        rrcMeasCtrl.gpsRefTime.isUtranGpsRefTimePresent     = 0;
        rrcMeasCtrl.isUePositioningNavigationModelPresent   = 0;
        rrcMeasCtrl.gpsNavMod.numOfSat                      = MAX_SAT_ID;

        for (idx = 0; idx < MAX_SAT_ID; idx++)
        {
            rrcMeasCtrl.gpsNavMod.satId[idx] = 0;
        }

        if (ASN_SUCCESS == fillAndEncodeRrcMeasurementControl (&rrcMeasCtrl,
                                                               &pRrcMeasCtrlEncodedData,
                                                               &rrcMeasCtrlLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcMeasCtrlLen;
            pRlcDataReq->rrcPdu            = pRrcMeasCtrlEncodedData;

            DEBUGMSG(("RRC Measurement Control Release Len (%d) is sent\n", rrcMeasCtrlLen));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId =  
                           (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        }
        else
        {
            DEBUG1((" Encoding RRC Measurement Control Release is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Measurement Control Release is failed\n")); 
    }
}

/*
 *############################################################################
 * Method Name : getUeIndexFromImsi
 *
 * Description : This procedure gets the Ue Index from IMSI value.
 *
 * Parameters  : 
 *   imsiStr      IMSI of the UE
 *
 * Returns     :  
 *   ueIndex      Index of the UE in UE Control Block
 *############################################################################
 */
unsigned int getUeIndexFromImsi (char *pImsiStr)
{
    unsigned int idx     = 0;
    unsigned int ueIndex = 0xFFFFFFFF;
    RrcUeSpecificData *pUeSpecificData = NULL;

    for(idx = 0; idx < MAX_UE_SUPPORTED; idx++)
    {
        /* Get UE specific data */
        pUeSpecificData = &ueControlBlock.ueSpecificData[idx];

        if((1 == pUeSpecificData->isBlackListed) &&
           (0 == strncmp(pImsiStr, pUeSpecificData->imsi, MAX_IMSI_LENGTH)))
        {
            ueIndex = idx;
        }
    }
 
    return (ueIndex);
}

/*
 *############################################################################
 * Method Name : getUeIndexFromImei
 *
 * Description : This procedure gets the Ue Index from IMEI value.
 *
 * Parameters  : 
 *   imeiStr      IMEI of the UE
 *
 * Returns     :  
 *   ueIndex      Index of the UE in UE Control Block
 *############################################################################
 */
unsigned int getUeIndexFromImei (char *pImeiStr)
{
    unsigned int idx     = 0;
    unsigned int ueIndex = 0xFFFFFFFF;
    RrcUeSpecificData *pUeSpecificData = NULL;

    for(idx = 0; idx < MAX_UE_SUPPORTED; idx++)
    {
        /* Get UE specific data */
        pUeSpecificData = &ueControlBlock.ueSpecificData[idx];

        if((1 == pUeSpecificData->isBlackListed) &&
           (0 == strncmp(pImeiStr, pUeSpecificData->imei, MAX_IMEI_LENGTH)))
        {
            ueIndex = idx;
        }
    }
 
    return (ueIndex);
}

/*
 *############################################################################
 * Method Name :  handleDelConfigIdList
 *
 * Description :     This procedure handles the deletion of configured identity list from UE Context.
 *
 * Parameters  : 
 *   delConifgIdList   Configuration ID List to be deleted
 *
 * Returns     :      None
 *############################################################################
 */
void handleDelConfigIdList (DelConfigIdList *pDelConfigId)
{
    unsigned int ueIndex = 0;
    RrcUeSpecificData *pUeSpecificData = NULL;
    char *pImsiForPcchDel = NULL;

    if (1 == pDelConfigId->idType)
    {
        /* Get UE Index from IMEI Value */
	ueIndex = getUeIndexFromImei (pDelConfigId->identity);
    }
    else
    {
        /* Get UE Index from IMSI Value */
        ueIndex = getUeIndexFromImsi (pDelConfigId->identity);
    }

    if(0xFFFFFFFF != ueIndex)
    {
        pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

        pImsiForPcchDel = (char*)malloc(MAX_IMSI_LENGTH + 1);
		memset((char *)pImsiForPcchDel, 0, (MAX_IMSI_LENGTH + 1));
        strncpy (pImsiForPcchDel , pUeSpecificData->imsi, pUeSpecificData->imsiLen);
        pImsiForPcchDel[pUeSpecificData->imsiLen] = '\0';
		
        delLatchedUe(pImsiForPcchDel);
		
        pImsiForPcchDel = (char*)malloc(MAX_IMSI_LENGTH + 1);
		memset((char *)pImsiForPcchDel, 0, (MAX_IMSI_LENGTH + 1));
        strncpy (pImsiForPcchDel , pUeSpecificData->imsi, pUeSpecificData->imsiLen);
        pImsiForPcchDel[pUeSpecificData->imsiLen] = '\0';

        SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                       L2_DL_DEL_PCCH_DATA_REQUEST_E, pImsiForPcchDel);

        pUeSpecificData->isBlackListed = 0;
        sendRadioLinkpDeleteReqToL1 (ueIndex);
    }
    else
    {
        DEBUG1(("Could not find UE Context for Identity Type = %d & Identity = %s\n", 
                 pDelConfigId->idType,pDelConfigId->identity));
    }

    /* Clear the Memory Allocated */
    free(pDelConfigId);
    pDelConfigId = NULL;
}

#ifdef FXL_BL_UE_POWER_OFF
/*
 *############################################################################
 * Method Name : delLatchedUe
 *
 * Description : This procedure deletes single UE from Latched Ue list
 *
 * Parameters  : None
 *
 * Returns     : None
 *############################################################################
 */
void sendBlUePowerOffInd(unsigned int ueIndex)
{
      fxLBlUePowerOffInd powerOffInd;
      memset (&powerOffInd, 0 , sizeof (fxLBlUePowerOffInd));
      powerOffInd.hdr.msgLength = sizeof(fxLBlUePowerOffInd);
      powerOffInd.hdr.rat = FXL_RAT_3G;
      powerOffInd.hdr.msgType = FXL_BL_UE_POWER_OFF_IND;
      powerOffInd.detachCause = FXL_POWER_OFF;
      strncpy (powerOffInd.imsiStr, ueControlBlock.ueSpecificData[ueIndex].imsi, MAX_IMSI_LENGTH);
	  powerOffInd.imsiStr[MAX_IMSI_LENGTH] = '\0';

      if (-1 != clientFd)
      {
         if (send(clientFd, &powerOffInd, sizeof(powerOffInd), 0) < 0)
         {
             DEBUG1(("Unable to send Message to External Client\n"));
         }
         else
         {
             DEBUGMSG(("UE Power Off Indication is sent to External Client\n"));
         }
      }
      else
      {
            DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
      }
      buildAndSendRrcConnectionRelease (ueIndex, 1); 
}
#endif

void sendEmergencyCallInd(unsigned int ueIndex)
{                      
      fxL3gEmergencyCallAttemptedInd emeCallAttemptedInd;
      memset (&emeCallAttemptedInd, 0 , sizeof (fxL3gEmergencyCallAttemptedInd));
      
      emeCallAttemptedInd.hdr.msgLength = sizeof(fxL3gEmergencyCallAttemptedInd);
      emeCallAttemptedInd.hdr.msgType   = FXL_3G_EMERGENCY_CALL_ATTEMPTED_IND;
      emeCallAttemptedInd.hdr.rat       = FXL_RAT_3G;

      emeCallAttemptedInd.initialIdentityType = UE_IDENTITY_IMSI;
      strncpy (emeCallAttemptedInd.u.imsiStr, ueControlBlock.ueSpecificData[ueIndex].imsi, MAX_IMSI_LENGTH);
      emeCallAttemptedInd.u.imsiStr[ueControlBlock.ueSpecificData[ueIndex].imsiLen] = '\0';

      if (-1 != clientFd)
      {
          if (send(clientFd, &emeCallAttemptedInd, sizeof(emeCallAttemptedInd), 0) < 0)
          {
              DEBUG1(("Unable to send Emergency Call Attempted Message to External Client\n"));
          }
          else
          {
              DEBUGMSG(("DCH Emergency Call Attempted Indication is sent to External Client\n"));
          }
      }
      else
      {
         DEBUG1(("Unable to send message to External Client, clientFd = -1\n"));
      }                
}

void buildAndSendRrcConnectionReleaseForEmergencyCall (unsigned int  ueIndex)
{
    RrcDataReq      *pRlcDataReq = NULL;
    unsigned int     rrcConnRelLen = 0;
    unsigned char   *pRrcConnRelEncodedData = NULL;
    unsigned int    *pTimerUeId = NULL;
    unsigned int     idx = 0;
    RrcConnectionReleaseData rrcConnRel;

    /* Get UE specific data */
    RrcUeSpecificData *pUeSpecificData = &ueControlBlock.ueSpecificData[ueIndex];

    /* Change the UE State */
    pUeSpecificData->ueState = UE_WAIT_FOR_RRC_CON_REL_COMPLETE;

    /* Disable the paging type 1 */
    pUeSpecificData->emergencyCallAttempted = 1; 

    /* Start the Timer for RRC Connection Release */
    pTimerUeId  = (unsigned int *)malloc (sizeof(ueIndex));
    *pTimerUeId = ueIndex;

    timerStart(&pUeSpecificData->ueTimer,
               RRC_CONNECTION_RELEASE_TIMEOUT,
               0,
               handleRrcConnectionReleaseTimeout,
               pTimerUeId);

    /* Initialize ASN Context */
    if (ASN_SUCCESS == initializeASNContext ())
    {
        /* Fill RRC Connection Release Information */
        rrcConnRel.laterThanR3      = 1;
        rrcConnRel.rrcTransactionId = pUeSpecificData->rrcTransactionId;
        rrcConnRel.n308Presence     = VALUE_PRESENT;
        rrcConnRel.n308             = VALUE_PRESENT;

        rrcConnRel.releaseCause = RELEASE_CAUSE_CONGESTION;

        if (FXL_UMTS_REDIRECTION_TO_2G == gUmtsConfig.emergencyCallRedirectionType)
        {
             rrcConnRel.redirectionType  = REDIRECTION_INFO_INTER_RAT;
             rrcConnRel.numOfGsmCell     = 1;
             /* Fill GSM ARFCN */
             rrcConnRel.gsmInfo[0].arfcn =
                           gUmtsConfig.emergencyCallRedirectionInfo.gsmRedirectinfo.startingarfcn;
             if (DCS1800 == gUmtsConfig.emergencyCallRedirectionInfo.gsmRedirectinfo.bandIndicator)
             {
                 rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1800;
             }
             else
             {
                 rrcConnRel.gsmInfo[0].frequencyBand = GSM_FREQ_BAND_1900;
             }

             /* Fill GSM BSIC Presence */
             rrcConnRel.gsmInfo[0].bsicPresence = 0;

             /* Fill GSM BSIC NCC */
             rrcConnRel.gsmInfo[0].bsicNcc = 0;

             /* Fill GSM BSIC BCC */
             rrcConnRel.gsmInfo[0].bsicBcc = 0;
        }
        else if (FXL_UMTS_REDIRECTION_TO_3G == gUmtsConfig.emergencyCallRedirectionType)
        {
             rrcConnRel.redirectionType = REDIRECTION_INFO_INTER_FREQUENCY;
             rrcConnRel.dlUarfcn        = gUmtsConfig.emergencyCallRedirectionInfo.dlUarfcn;
        }

        if (ASN_SUCCESS == fillAndEncodeRrcConnectionRelease (&rrcConnRel,
                                                              &pRrcConnRelEncodedData,
                                                              &rrcConnRelLen))
        {
            /* Fill RLC Data Indication */
            pRlcDataReq = (RrcDataReq *)malloc(sizeof(RrcDataReq));
            pRlcDataReq->cellOrUeId.choice = RRC_UE_ID;
            pRlcDataReq->cellOrUeId.u.ueId = ueIndex;
            pRlcDataReq->logicalChType     = LOG_CH_DCCH_E;
            pRlcDataReq->logicalChId       = 2;             //Logical channel id 2 is used for DCCH message
            pRlcDataReq->rrcPduLen         = rrcConnRelLen;
            pRlcDataReq->rrcPdu            = pRrcConnRelEncodedData;

            DEBUGMSG(("RRC Connection Release is sent\n"));

            /* Post DL CCCH Message to L2 DL DATA Task */
            SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
                           L2_DL_DCCH_DATA_REQUEST_E, pRlcDataReq);

            /* Increament RRC Transaction Id */
            ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId =
                          (ueControlBlock.ueSpecificData[ueIndex].rrcTransactionId + 1) % 4;
        }
        else
        {
            DEBUG1((" Encoding RRC Connection Release is failed\n"));
        }

        /* Clean ASN Context */
        cleanASNContext ();
    }
    else
    {
        DEBUG1(("Initialization of ASN for RRC Connection Release is failed\n"));
    }
}

/*
 *############################################################################
 * Method Name : isImsiMatchesToConfiguredPlmn
 *
 * Description    : This procedure matches IMSI to configured PLMN
 *
 * Parameters    : IMSI Value
 *
 * Returns         : TRUE/FALSE
 *############################################################################
 */
unsigned char isImsiMatchesToConfiguredPlmn(char *pImsi)
{
	int plmnNo =0;
	
	for(plmnNo = 0; plmnNo < gUmtsConfig.numOfPlmn; plmnNo++)
	{
		if(((pImsi[0]- '0') == (char)gUmtsConfig.plmn[plmnNo].mcc[0]) &&
		   ((pImsi[1]- '0') == (char)gUmtsConfig.plmn[plmnNo].mcc[1]) &&
		   ((pImsi[2] -'0') == (char)gUmtsConfig.plmn[plmnNo].mcc[2]) &&
		   ((pImsi[3] -'0') == (char)gUmtsConfig.plmn[plmnNo].mnc[0]) &&
		   ((pImsi[4]- '0') == (char)gUmtsConfig.plmn[plmnNo].mnc[1]) )
		{
			if(gUmtsConfig.plmn[plmnNo].mncLength == 3)
			{
				if((pImsi[5]- '0') == (char)gUmtsConfig.plmn[plmnNo].mnc[2])
				{
					return TRUE;
				}
			}				
			else
			{
				return TRUE;
			}
		}
			
	}

	return FALSE;

}

/*
 *############################################################################
 * Method Name : pageAllConfiguredTargetPhone
 *
 * Description    : This procedure send paging with IMSI to all the configured Traget Phone
 *
 * Parameters    : None
 *
 * Returns         : None
 *############################################################################
 */
void pageAllConfiguredTargetPhone(void)
{
   unsigned int idx = 0, idx1 = 0;
   unsigned int       rrcPagingType1Len = 0;
   unsigned char     *pRrcPagingType1EncodedData = NULL;
   RrcPcchMsg        *pRrcPcchMsg = NULL;
   char imsiValue[MAX_IMSI_LENGTH + 1] = {0};
   
   RrcPagingType1Data rrcPagingType1;
   memset (&rrcPagingType1, 0 , sizeof (rrcPagingType1));

   /* Get the Access to Configured Identity List */
   unsigned char *sharedMemPtr   = getAccessToIdList();
   UeIdentityConfig    *configId;
   UmtsConfigList *configUeIdList = (UmtsConfigList *)sharedMemPtr;
   configId = sharedMemPtr + sizeof(char)*4;

   for(idx = 0; idx < configUeIdList->numbOfIds; idx++)
   {
       if((configId[idx].idType == IMSI) &&
          (configId[idx].listType == BLACKLIST))
       {
           if(isImsiMatchesToConfiguredPlmn(configId[idx].identity) == FALSE)
		   {
		       DEBUGMSG(("IMSI(%s) does not match to configured PLMN\n", configId[idx].identity));
			   continue;
		   }
				   
		   /* Get IMSI Details */
		   strncpy (imsiValue, configId[idx].identity,
					MAX_IMSI_LENGTH);
		   imsiValue[MAX_IMSI_LENGTH] = '\0';

		   /* Initialize ASN Context */
		   if (ASN_SUCCESS == initializeASNContext ())
		   {
			   /* Fill Paging Type1 Information */
			   rrcPagingType1.pagingCause  = PAGING_CAUSE_TERMINATING_CONVERSATIONAL_CALL;
			   rrcPagingType1.cnDomainType = CN_DOMAIN_IDENTITY_CS;
			   rrcPagingType1.imsiLength   = MAX_IMSI_LENGTH;
			   for (idx1 = 0; idx1 < MAX_IMSI_LENGTH; idx1++)
			   {
				   /* Convert ASCII to Integer for IMSI value */
				   rrcPagingType1.imsi[idx1] = imsiValue[idx1] - '0';
			   }
		   
			   if (ASN_SUCCESS == fillAndEncodePagingType1 (&rrcPagingType1,
															&pRrcPagingType1EncodedData,
															&rrcPagingType1Len))
			   {
				   pRrcPcchMsg = (RrcPcchMsg *)malloc(sizeof(RrcPcchMsg)); 
				   pRrcPcchMsg->sfn 	  = getPagingSfn (imsiValue, MAX_IMSI_LENGTH);
				   pRrcPcchMsg->rrcPduLen = rrcPagingType1Len;
				   pRrcPcchMsg->rrcPdu	  = pRrcPagingType1EncodedData;
				   pRrcPcchMsg->count	  = 0;
				   pRrcPcchMsg->isInitialIMSIPaging = TRUE;
				   strncpy (pRrcPcchMsg->imsi, imsiValue, (MAX_IMSI_LENGTH + 1));
		   
				   /* Post DL CCCH Message to L2 DL DATA Task */
				   SendMsgToTask (APP_RRC_TASK_E, L2_DL_DATA_TASK_E,
								  L2_DL_PCCH_DATA_REQUEST_E, pRrcPcchMsg);
		   
				   DEBUGMSG(("RRC Paging Type1 is sent to L2 for IMSI = %s\n", 
				   	         configId[idx].identity));
			   } 
			   else
			   {
				   DEBUG1((" Encoding RRC Paging Type1 is failed\n"));
			   }
		   
			   /* Clean ASN Context */
			   cleanASNContext ();
		   }
		   else
		   {
			   DEBUG1(("Initialization of ASN for RRC Paging Type1 is failed\n")); 
		   }   
       }
   }

   return;
}

