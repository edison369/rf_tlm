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

/*
** Includes of the apps that send telemetry
*/
#include "imu_app_msgids.h"
#include "imu_app_msg.h"

#include "blinky_msgids.h"
#include "blinky_msg.h"
/***********************************************************************/

/*
** Include and constants for I2C
*/
#include "gen-uC.h"

static const char bus_path[] = "/dev/i2c-2";
static const char genuC_path[] = "/dev/i2c-2.genuC-0";
/***********************************************************************/
#define RF_TLM_TASK_MSEC 500 /* run at 2 Hz */

#define RF_TLM_UNUSED    CFE_SB_MSGID_RESERVED

#define RF_TLM_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

/**
 * Depth of pipe for telemetry forwarded through the RF_TLM application
 */
#define RF_TLM_TO_PIPE_DEPTH OS_QUEUE_MAX_DEPTH

#define RF_TLM_TBL_ELEMENT_1_MAX 10
/************************************************************************
** Type Definitions
*************************************************************************/


#define RF_PAYLOAD_BYTES 30

/*
** Global Data
*/
typedef struct
{

    bool downlink_on;
    bool suppress_sendto;
    bool tlm_debug;
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    RF_TLM_HkTlm_t HkTlm;                   // Telemetry sent over UDP

    // Private data
    uint8 AppID_H;
    uint8 AppID_L;
    uint8 Ext_CmdCounter;
    uint8 Ext_ErrCounter;
    uint8 byte_group_1[4];
    uint8 byte_group_2[4];
    uint8 byte_group_3[4];
    uint8 byte_group_4[4];
    uint8 byte_group_5[4];
    uint8 byte_group_6[4];

    bool AppReporting1;
    bool AppReporting2;
    bool AppReporting3;
    bool AppReporting4;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;


    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;
    CFE_SB_PipeId_t TlmPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_EVS_BinFilter_t EventFilters[RF_TLM_EVENT_COUNTS];

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
int32 RF_TLM_Noop(const RF_TLM_NoopCmd_t *Msg);
int32 RF_TLM_EnableOutput(const RF_TLM_EnableOutputCmd_t *data);
int32 RF_TLM_Enable_Debug(const RF_TLM_EnableDebugCmd_t *Msg);
int32 RF_TLM_Disable_Debug(const RF_TLM_DisableDebugCmd_t *Msg);

void  RF_TLM_Data_Init(void);
void  RF_TLM_openTLM(void);
void  RF_TLM_forward_telemetry(void);
int32 genuC_driver_open(void);
int32 send_tlm_data(void);

bool RF_TLM_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* RF_TLM_H */
