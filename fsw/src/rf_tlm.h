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
 * @file
 *
 * Main header file for the SAMPLE application
 */

#ifndef RF_TLM_H
#define RF_TLM_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "rf_tlm_perfids.h"
#include "rf_tlm_msgids.h"
#include "rf_tlm_msg.h"

/***********************************************************************/
#define RF_TLM_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

#define RF_TLM_NUMBER_OF_TABLES 1 /* Number of Table(s) */

/* Define filenames of default data images for tables */
#define RF_TLM_TABLE_FILE "/cf/rf_tlm_tbl.tbl"

#define RF_TLM_TABLE_OUT_OF_RANGE_ERR_CODE -1

#define RF_TLM_TBL_ELEMENT_1_MAX 10
/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    RF_TLM_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_TBL_Handle_t TblHandles[RF_TLM_NUMBER_OF_TABLES];
} RF_TLM_Data_t;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (RF_TLM_Main), these
**       functions are not called from any other source module.
*/
void  RF_TLM_Main(void);
int32 RF_TLM_Init(void);
void  RF_TLM_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr);
void  RF_TLM_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr);
int32 RF_TLM_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg);
int32 RF_TLM_ResetCounters(const RF_TLM_ResetCountersCmd_t *Msg);
int32 RF_TLM_Process(const RF_TLM_ProcessCmd_t *Msg);
int32 RF_TLM_Noop(const RF_TLM_NoopCmd_t *Msg);
void  RF_TLM_GetCrc(const char *TableName);

int32 RF_TLM_TblValidationFunc(void *TblData);

bool RF_TLM_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* RF_TLM_H */
