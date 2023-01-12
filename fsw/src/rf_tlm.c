/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the RF Telemetry Output App.
 */

/*
** Include Files:
*/
#include "rf_tlm_events.h"
#include "rf_tlm_version.h"
#include "rf_tlm.h"

#include <string.h>

/*
** global data
*/
RF_TLM_Data_t RF_TLM_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void RF_TLM_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(RF_TLM_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = RF_TLM_Init();
    if (status != CFE_SUCCESS)
    {
        RF_TLM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }else{
      RF_TLM_openTLM();
    }

    /*
    ** RF TLM Runloop
    */
    while (CFE_ES_RunLoop(&RF_TLM_Data.RunStatus) == true)
    {

        CFE_ES_PerfLogExit(RF_TLM_PERF_ID);

        CFE_ES_PerfLogEntry(RF_TLM_PERF_ID);

        RF_TLM_forward_telemetry();

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, RF_TLM_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        if (status == CFE_SUCCESS)
        {
            RF_TLM_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(RF_TLM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RF TLM APP: SB Pipe Read Error, App Will Exit\n");

            RF_TLM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }

    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(RF_TLM_PERF_ID);

    CFE_ES_ExitApp(RF_TLM_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RF_TLM_Init(void)
{
    int32 status;

    RF_TLM_Data.downlink_on = false;
    RF_TLM_Data.tlm_debug = false;

    RF_TLM_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    RF_TLM_Data.CmdCounter = 0;
    RF_TLM_Data.ErrCounter = 0;

    RF_TLM_Data.AppReporting1 = false;
    RF_TLM_Data.AppReporting2 = false;
    RF_TLM_Data.AppReporting3 = false;
    RF_TLM_Data.AppReporting4 = false;

    /*
    ** Initialize app configuration data
    */
    RF_TLM_Data.PipeDepth = RF_TLM_PIPE_DEPTH;

    strncpy(RF_TLM_Data.PipeName, "RF_TLM_CMD_PIPE", sizeof(RF_TLM_Data.PipeName));
    RF_TLM_Data.PipeName[sizeof(RF_TLM_Data.PipeName) - 1] = 0;

    /*
    ** Initialize event filter table...
    */
    RF_TLM_Data.EventFilters[0].EventID = RF_TLM_STARTUP_INF_EID;
    RF_TLM_Data.EventFilters[0].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[1].EventID = RF_TLM_COMMAND_ERR_EID;
    RF_TLM_Data.EventFilters[1].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[2].EventID = RF_TLM_COMMANDNOP_INF_EID;
    RF_TLM_Data.EventFilters[2].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[3].EventID = RF_TLM_COMMANDRST_INF_EID;
    RF_TLM_Data.EventFilters[3].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[4].EventID = RF_TLM_INVALID_MSGID_ERR_EID;
    RF_TLM_Data.EventFilters[4].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[5].EventID = RF_TLM_LEN_ERR_EID;
    RF_TLM_Data.EventFilters[5].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[6].EventID = RF_TLM_PIPE_ERR_EID;
    RF_TLM_Data.EventFilters[6].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[7].EventID = RF_TLM_SUBSCRIBE_ERR_EID;
    RF_TLM_Data.EventFilters[7].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[8].EventID = RF_TLM_TLMOUTSTOP_ERR_EID;
    RF_TLM_Data.EventFilters[8].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[9].EventID = RF_TLM_GENUC_ERR_EID;
    RF_TLM_Data.EventFilters[9].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[10].EventID = RF_TLM_TLMOUTENA_INF_EID;
    RF_TLM_Data.EventFilters[10].Mask    = 0x0000;
    RF_TLM_Data.EventFilters[11].EventID = RF_TLM_DEV_INF_EID;
    RF_TLM_Data.EventFilters[11].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(RF_TLM_Data.EventFilters, RF_TLM_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Initialize private data
    */
    RF_TLM_Data_Init();

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(RF_TLM_HK_TLM_MID),
                 sizeof(RF_TLM_Data.HkTlm));

    /*
    ** Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&RF_TLM_Data.CommandPipe, RF_TLM_Data.PipeDepth, RF_TLM_Data.PipeName);
    if (status == CFE_SUCCESS){
        /* Subscribe to Housekeeping request commands */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RF_TLM_SEND_HK_MID), RF_TLM_Data.CommandPipe);
        if (status != CFE_SUCCESS){
           CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
           return status;
        }

        /* Subscribe to ground command packets */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RF_TLM_CMD_MID), RF_TLM_Data.CommandPipe);
        if (status != CFE_SUCCESS){
           CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);
           return status;
        }
    }else{
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Software Bus telemetry pipe.
    */
    char   RFTlmPipeName[16];
    uint16 RFTlmPipeDepth;

    RFTlmPipeDepth = RF_TLM_TO_PIPE_DEPTH;
    strcpy(RFTlmPipeName, "RF_TLM_TO_PIPE");

    status = CFE_SB_CreatePipe(&RF_TLM_Data.TlmPipe, RFTlmPipeDepth, RFTlmPipeName);
    if (status != CFE_SUCCESS){
        CFE_EVS_SendEvent(RF_TLM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "RF Can't create Tlm pipe status %i\n",(int)status);
        return status;
    }

    /*
    ** Subscribe to IMU App RF packets
    */
    // status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(IMU_APP_RF_DATA_MID), RF_TLM_Data.TlmPipe);
    status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(IMU_APP_RF_DATA_MID),  /* Msg Id to Receive */
                                RF_TLM_Data.TlmPipe,                            /* Pipe Msg is to be Rcvd on */
                                CFE_SB_DEFAULT_QOS,                             /* Quality of Service */
                                10);                                            /* Max Number to Queue */
    if (status != CFE_SUCCESS){
       CFE_EVS_SendEvent(RF_TLM_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
         "RF Telemetry Output App: Error Subscribing to IMU App, RC = 0x%08lX\n",
         (unsigned long)status);
       return status;
    }

    /*
    ** Subscribe to Blinky App RF packets
    */
    // status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BLINKY_RF_DATA_MID), RF_TLM_Data.TlmPipe);
    status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(BLINKY_RF_DATA_MID),  /* Msg Id to Receive */
                                RF_TLM_Data.TlmPipe,                            /* Pipe Msg is to be Rcvd on */
                                CFE_SB_DEFAULT_QOS,                             /* Quality of Service */
                                10);                                            /* Max Number to Queue */
    if (status != CFE_SUCCESS){
       CFE_EVS_SendEvent(RF_TLM_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
         "RF Telemetry Output App: Error Subscribing to Blinky App, RC = 0x%08lX\n",
         (unsigned long)status);
       return status;
    }

    CFE_EVS_SendEvent(RF_TLM_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "RF Tlm App Initialized.%s",
                     RF_TLM_VERSION_STRING);


    return CFE_SUCCESS;
}

/*
** Initialize private data
*/
void RF_TLM_Data_Init(void){
  RF_TLM_Data.AppID_H = 0x00;
  RF_TLM_Data.AppID_L = 0x00;
  RF_TLM_Data.Ext_CmdCounter = 0;
  RF_TLM_Data.Ext_ErrCounter = 0;
  for(int i=0;i<4;i++){
    RF_TLM_Data.byte_group_1[i] = 0;
    RF_TLM_Data.byte_group_2[i] = 0;
    RF_TLM_Data.byte_group_3[i] = 0;
    RF_TLM_Data.byte_group_4[i] = 0;
    RF_TLM_Data.byte_group_5[i] = 0;
    RF_TLM_Data.byte_group_6[i] = 0;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* RF_TLM_EnableOutput() -- TLM output enabled                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 RF_TLM_EnableOutput(const RF_TLM_EnableOutputCmd_t *data){
    RF_TLM_Data.suppress_sendto = false;
    CFE_EVS_SendEvent(RF_TLM_TLMOUTENA_INF_EID, CFE_EVS_EventType_INFORMATION, "RF telemetry output enabled\n");

    if (!RF_TLM_Data.downlink_on){
        RF_TLM_Data.downlink_on = true;
    }

    ++RF_TLM_Data.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the RF Telemetry    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void RF_TLM_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case RF_TLM_CMD_MID:
            RF_TLM_ProcessGroundCommand(SBBufPtr);
            break;

        case RF_TLM_SEND_HK_MID:
            RF_TLM_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(RF_TLM_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RF TLM: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RF TLM ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void RF_TLM_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr){
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" RF TLM app ground commands
    */
    switch (CommandCode)
    {
        case RF_TLM_NOOP_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_NoopCmd_t)))
            {
                RF_TLM_Noop((RF_TLM_NoopCmd_t *)SBBufPtr);
            }

            break;

        case RF_TLM_RESET_COUNTERS_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_ResetCountersCmd_t)))
            {
                RF_TLM_ResetCounters((RF_TLM_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case RF_TLM_OUTPUT_ENABLE_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_EnableOutputCmd_t)))
            {
                RF_TLM_EnableOutput((const RF_TLM_EnableOutputCmd_t *)SBBufPtr);
            }

            break;

        case RF_TLM_DEBUG_ENABLE_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_EnableDebugCmd_t)))
            {
                RF_TLM_Enable_Debug((const RF_TLM_EnableDebugCmd_t *)SBBufPtr);
            }

            break;

        case RF_TLM_DEBUG_DISABLE_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_DisableDebugCmd_t)))
            {
                RF_TLM_Disable_Debug((const RF_TLM_DisableDebugCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(RF_TLM_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 RF_TLM_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{

    /*
    ** Get command execution counters...
    */
    RF_TLM_Data.HkTlm.Payload.CommandErrorCounter = RF_TLM_Data.ErrCounter;
    RF_TLM_Data.HkTlm.Payload.CommandCounter      = RF_TLM_Data.CmdCounter;

    if(RF_TLM_Data.AppReporting1){
      RF_TLM_Data.HkTlm.Payload.AppReportingID1[1] = 0x08;
      RF_TLM_Data.HkTlm.Payload.AppReportingID1[0] = 0xE0;
    }else{
      RF_TLM_Data.HkTlm.Payload.AppReportingID1[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID1[0] = 0x00;
    }

    if(RF_TLM_Data.AppReporting2){
      RF_TLM_Data.HkTlm.Payload.AppReportingID2[1] = 0x08;
      RF_TLM_Data.HkTlm.Payload.AppReportingID2[0] = 0xB3;
    }else{
      RF_TLM_Data.HkTlm.Payload.AppReportingID2[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID2[0] = 0x00;
    }

    if(RF_TLM_Data.AppReporting3){
      RF_TLM_Data.HkTlm.Payload.AppReportingID3[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID3[0] = 0x00;
    }else{
      RF_TLM_Data.HkTlm.Payload.AppReportingID3[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID3[0] = 0x00;
    }

    if(RF_TLM_Data.AppReporting4){
      RF_TLM_Data.HkTlm.Payload.AppReportingID4[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID4[0] = 0x00;
    }else{
      RF_TLM_Data.HkTlm.Payload.AppReportingID4[1] = 0x00;
      RF_TLM_Data.HkTlm.Payload.AppReportingID4[0] = 0x00;
    }

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RF TLM Enable Debug command                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RF_TLM_Enable_Debug(const RF_TLM_EnableDebugCmd_t *Msg)
{
    RF_TLM_Data.CmdCounter++;
    RF_TLM_Data.tlm_debug = true;

    CFE_EVS_SendEvent(RF_TLM_COMMANDDEBUG_INF_EID, CFE_EVS_EventType_INFORMATION, "RF TLM: Debug enabled");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RF TLM Disable Debug command                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RF_TLM_Disable_Debug(const RF_TLM_DisableDebugCmd_t *Msg)
{
    RF_TLM_Data.CmdCounter++;
    RF_TLM_Data.tlm_debug = false;

    CFE_EVS_SendEvent(RF_TLM_COMMANDDEBUG_INF_EID, CFE_EVS_EventType_INFORMATION, "RF TLM: Debug disabled");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RF TLM NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RF_TLM_Noop(const RF_TLM_NoopCmd_t *Msg)
{
    RF_TLM_Data.CmdCounter++;

    CFE_EVS_SendEvent(RF_TLM_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "RF TLM: NOOP command %s",
                      RF_TLM_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 RF_TLM_ResetCounters(const RF_TLM_ResetCountersCmd_t *Msg)
{
    RF_TLM_Data.CmdCounter = 0;
    RF_TLM_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(RF_TLM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "RF TLM: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool RF_TLM_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(RF_TLM_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        RF_TLM_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* RF_TLM_openTLM() -- Open TLM                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RF_TLM_openTLM(void){
    int32 status;

    status = genuC_driver_open();
    if (status != CFE_SUCCESS){
      CFE_EVS_SendEvent(RF_TLM_GENUC_ERR_EID, CFE_EVS_EventType_ERROR, "RF TLM: Error opening the genuC driver");
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* RF_TLM_forward_telemetry() -- Forward telemetry                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RF_TLM_forward_telemetry(void){
    int32            status = 0;
    int32            CFE_SB_status = CFE_SUCCESS;
    CFE_SB_Buffer_t* TlmMsgPtr = NULL;
    CFE_SB_MsgId_t   TlmMsgId;


    SUBS_APP_OutData_t* dataPtr = NULL;

    do{
        CFE_SB_status = CFE_SB_ReceiveBuffer(&TlmMsgPtr, RF_TLM_Data.TlmPipe, CFE_SB_POLL);
        dataPtr = NULL;

        if (CFE_SB_status == CFE_SUCCESS){
            if((RF_TLM_Data.suppress_sendto == false) && (RF_TLM_Data.downlink_on == true)){
              CFE_MSG_GetMsgId(&TlmMsgPtr->Msg, &TlmMsgId);

              /* Update the private data */
              dataPtr = (SUBS_APP_OutData_t *)TlmMsgPtr;
              RF_TLM_Data.AppID_H = dataPtr->AppID_H;
              RF_TLM_Data.AppID_L = dataPtr->AppID_L;
              RF_TLM_Data.Ext_CmdCounter = dataPtr->CommandCounter;
              RF_TLM_Data.Ext_ErrCounter = dataPtr->CommandErrorCounter;
              for(int i=0;i<4;i++){
                RF_TLM_Data.byte_group_1[i] = dataPtr->byte_group_1[i];
                RF_TLM_Data.byte_group_2[i] = dataPtr->byte_group_2[i];
                RF_TLM_Data.byte_group_3[i] = dataPtr->byte_group_3[i];
                RF_TLM_Data.byte_group_4[i] = dataPtr->byte_group_4[i];
                RF_TLM_Data.byte_group_5[i] = dataPtr->byte_group_5[i];
                RF_TLM_Data.byte_group_6[i] = dataPtr->byte_group_6[i];
              }

              CFE_ES_PerfLogEntry(RF_TLM_I2C_SEND_PERF_ID);

              status = send_tlm_data();

              CFE_ES_PerfLogExit(RF_TLM_I2C_SEND_PERF_ID);

              if(RF_TLM_Data.tlm_debug){
                switch (CFE_SB_MsgIdToValue(TlmMsgId)){

                  case IMU_APP_RF_DATA_MID:
                    CFE_EVS_SendEvent(RF_TLM_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_INFORMATION,
                                      "RF TLM - Enviando Altitud app con status %d",status);
                    break;

                  case BLINKY_RF_DATA_MID:
                    CFE_EVS_SendEvent(RF_TLM_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_INFORMATION,
                                      "RF TLM - Enviando Blinky app con status %d",status);
                    break;

                  default:
                    CFE_EVS_SendEvent(RF_TLM_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "RF TLM - Recvd invalid TLM msgId (0x%08X)", CFE_SB_MsgIdToValue(TlmMsgId));
                    break;
                }
              }
            }else{
                status = 0;
            }

            if (status < 0){
                CFE_EVS_SendEvent(RF_TLM_TLMOUTSTOP_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "RF TLM: RF send tlm error. Tlm output suppressed\n");
                RF_TLM_Data.suppress_sendto = true;
            }
        }else if(CFE_SB_status == CFE_SB_NO_MESSAGE){
          // The pipe is empty
          break;
        }else if(CFE_SB_status == CFE_SB_TIME_OUT){
          CFE_EVS_SendEvent(RF_TLM_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "RF TLM: Message fails to arrive within the specified timeout period\n");
          break;
        }
        // RF_TLM_Data_Init();
        OS_TaskDelay(RF_TLM_TASK_MSEC); /*2 Hz*/
    }while(CFE_SB_status == CFE_SUCCESS);
}

static int uC_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg);

int32 send_tlm_data(){
  int rv;

  uint8_t *val;
  val = NULL;
  val = malloc(RF_PAYLOAD_BYTES * sizeof(uint8_t));

  if(RF_TLM_Data.tlm_debug){
    CFE_EVS_SendEvent(RF_TLM_TLMOUTSTOP_ERR_EID, CFE_EVS_EventType_ERROR,
                      "RF TLM: Sending packet from [AppID]: 0x%x%x",RF_TLM_Data.AppID_H, RF_TLM_Data.AppID_L);
  }

  val[0] = RF_TLM_Data.AppID_H;
  val[1] = RF_TLM_Data.AppID_L;
  val[2] = RF_TLM_Data.Ext_ErrCounter;
  val[3] = RF_TLM_Data.Ext_CmdCounter;
  val[4] = 0;
  val[5] = 0;

  for(int i=0;i<4;i++){
    val[i+6] = RF_TLM_Data.byte_group_1[i];
    val[i+10] = RF_TLM_Data.byte_group_2[i];
    val[i+14] = RF_TLM_Data.byte_group_3[i];
    val[i+18] = RF_TLM_Data.byte_group_4[i];
    val[i+22] = RF_TLM_Data.byte_group_5[i];
    val[i+26] = RF_TLM_Data.byte_group_6[i];
  }


  // Send the telemetry payload
  rv = uC_set_bytes(UC_ADDRESS, &val, RF_PAYLOAD_BYTES);
  if(rv == 1 || rv < 0){
    return -1;    // Couldn't open bus or ioctl failed
  }else{
    return 0;     // Succeded
  }
}

int32 genuC_driver_open(){

int rv;
int fd;

// Device registration
rv = i2c_dev_register_uC(
  &bus_path[0],
  &genuC_path[0]
);
if(rv == 0)
  CFE_EVS_SendEvent(RF_TLM_DEV_INF_EID, CFE_EVS_EventType_INFORMATION, "RF: Device registered correctly at %s",
                    genuC_path);

fd = open(&genuC_path[0], O_RDWR);
if(fd >= 0)
  CFE_EVS_SendEvent(RF_TLM_DEV_INF_EID, CFE_EVS_EventType_INFORMATION, "RF: Device opened correctly at %s",
                    genuC_path);
close(fd);

if(rv == 0 && fd >=0){
  return CFE_SUCCESS;
}else{
  return -1;
}

}

int uC_set_bytes(uint16_t chip_address, uint8_t **val, int numBytes){

  int fd;
  int rv;

  if(chip_address == 0){
    chip_address = (uint16_t) UC_ADDRESS;
  }

  uint8_t writebuff[numBytes];

  for(int i = 0; i<numBytes; i++){
    writebuff[i] = (*val)[i];
  }

  i2c_msg msgs[] = {{
    .addr = chip_address,
    .flags = 0,
    .buf = writebuff,
    .len = numBytes,
  }};
  struct i2c_rdwr_ioctl_data payload = {
    .msgs = msgs,
    .nmsgs = sizeof(msgs)/sizeof(msgs[0]),
  };

  fd = open(&bus_path[0], O_RDWR);
  if (fd < 0) {
    printf("Couldn't open bus...\n");
    return 1;
  }

  rv = ioctl(fd, I2C_RDWR, &payload);
  if (rv < 0) {
    perror("ioctl failed");
  }
  close(fd);

  return rv;
}

int i2c_dev_register_uC(const char *bus_path, const char *dev_path){
  i2c_dev *dev;

  dev = i2c_dev_alloc_and_init(sizeof(*dev), bus_path, UC_ADDRESS);
  if (dev == NULL) {
    return -1;
  }

  dev->ioctl = uC_ioctl;

  return i2c_dev_register(dev, dev_path);
}

static int uC_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg){
  int err;

  // Variables for the Send test
  int numBytes = 3;
  uint8_t *val;

  switch (command) {
    case UC_SEND_TEST:

      val = NULL;
      val = malloc(numBytes * sizeof(uint8_t));

      val[0] = 0x03;
      val[1] = 0x06;
      val[2] = 0x09;

      err = uC_set_bytes(UC_ADDRESS, &val, numBytes); //Send 0x03, 0x06 and 0x09 to the uC default address
      break;

    default:
      err = -ENOTTY;
      break;
  }

  return err;
}

int uC_send_test(int fd){
  return ioctl(fd, UC_SEND_TEST, NULL);
}
