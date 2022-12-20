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
#include "rf_tlm_table.h"

/* The sample_lib module provides the SAMPLE_LIB_Function() prototype */
#include <string.h>
#include "sample_lib.h"

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
    }

    /*
    ** SAMPLE Runloop
    */
    while (CFE_ES_RunLoop(&RF_TLM_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(RF_TLM_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, RF_TLM_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(RF_TLM_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            RF_TLM_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(RF_TLM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: SB Pipe Read Error, App Will Exit");

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

    RF_TLM_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    RF_TLM_Data.CmdCounter = 0;
    RF_TLM_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    RF_TLM_Data.PipeDepth = RF_TLM_PIPE_DEPTH;

    strncpy(RF_TLM_Data.PipeName, "RF_TLM_CMD_PIPE", sizeof(RF_TLM_Data.PipeName));
    RF_TLM_Data.PipeName[sizeof(RF_TLM_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(RF_TLM_HK_TLM_MID),
                 sizeof(RF_TLM_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&RF_TLM_Data.CommandPipe, RF_TLM_Data.PipeDepth, RF_TLM_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RF_TLM_SEND_HK_MID), RF_TLM_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(RF_TLM_CMD_MID), RF_TLM_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return status;
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&RF_TLM_Data.TblHandles[0], "RFTlmTable", sizeof(RF_TLM_Table_t),
                              CFE_TBL_OPT_DEFAULT, RF_TLM_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return status;
    }
    else
    {
        status = CFE_TBL_Load(RF_TLM_Data.TblHandles[0], CFE_TBL_SRC_FILE, RF_TLM_TABLE_FILE);
    }

    CFE_EVS_SendEvent(RF_TLM_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "RF Tlm App Initialized.%s",
                      RF_TLM_VERSION_STRING);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
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
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void RF_TLM_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" SAMPLE app ground commands
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

        case RF_TLM_PROCESS_CC:
            if (RF_TLM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RF_TLM_ProcessCmd_t)))
            {
                RF_TLM_Process((RF_TLM_ProcessCmd_t *)SBBufPtr);
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
    int i;

    /*
    ** Get command execution counters...
    */
    RF_TLM_Data.HkTlm.Payload.CommandErrorCounter = RF_TLM_Data.ErrCounter;
    RF_TLM_Data.HkTlm.Payload.CommandCounter      = RF_TLM_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(RF_TLM_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < RF_TLM_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(RF_TLM_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 RF_TLM_Noop(const RF_TLM_NoopCmd_t *Msg)
{
    RF_TLM_Data.CmdCounter++;

    CFE_EVS_SendEvent(RF_TLM_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s",
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

    CFE_EVS_SendEvent(RF_TLM_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 RF_TLM_Process(const RF_TLM_ProcessCmd_t *Msg)
{
    int32               status;
    RF_TLM_Table_t *TblPtr;
    const char *        TableName = "RF_TLM.RFTlmTable";

    /* Sample Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, RF_TLM_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("RF Telemetry Output App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    RF_TLM_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(RF_TLM_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    /* Invoke a function provided by RF_TLM_LIB */
    SAMPLE_LIB_Function();

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
/* Verify contents of First Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 RF_TLM_TblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    RF_TLM_Table_t *TblDataPtr = (RF_TLM_Table_t *)TblData;

    /*
    ** Sample Table Validation
    */
    if (TblDataPtr->Int1 > RF_TLM_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = RF_TLM_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RF_TLM_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("RF Telemetry Output App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("RF Telemetry Output App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}
