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
 * Define RF Tlm App  Messages and info
 */

#ifndef RF_TLM_MSG_H
#define RF_TLM_MSG_H

/*
** RF Tlm App command codes
*/
#define RF_TLM_NOOP_CC           0
#define RF_TLM_RESET_COUNTERS_CC 1
#define RF_TLM_OUTPUT_ENABLE_CC  2
#define RF_TLM_OUTPUT_DISABLE_CC 3
#define RF_TLM_DEBUG_ENABLE_CC   4
#define RF_TLM_DEBUG_DISABLE_CC  5

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
} RF_TLM_NoArgsCmd_t;

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef RF_TLM_NoArgsCmd_t RF_TLM_NoopCmd_t;
typedef RF_TLM_NoArgsCmd_t RF_TLM_ResetCountersCmd_t;
typedef RF_TLM_NoArgsCmd_t RF_TLM_EnableOutputCmd_t;
typedef RF_TLM_NoArgsCmd_t RF_TLM_DisableOutputCmd_t;
typedef RF_TLM_NoArgsCmd_t RF_TLM_EnableDebugCmd_t;
typedef RF_TLM_NoArgsCmd_t RF_TLM_DisableDebugCmd_t;

/*************************************************************************/
/*
** Type definition (RF Tlm App housekeeping)
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    uint8_t AppID_H;
    uint8_t AppID_L;
    uint8 CommandCounter;
    uint8 CommandErrorCounter;
    uint8 spare[2];
    uint8 byte_group_1[4];
    uint8 byte_group_2[4];
    uint8 byte_group_3[4];
    uint8 byte_group_4[4];
    uint8 byte_group_5[4];
    uint8 byte_group_6[4];
} SUBS_APP_OutData_t;

typedef struct
{
    uint8 CommandCounter;
    uint8 CommandErrorCounter;
    uint8 spare[2];
    int PcktCounter;
    int PcktErrCounter;
} RF_TLM_UDP_HkTlm_Payload_t;   // Telemetry sent over UDP (to_lab)

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    RF_TLM_UDP_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} RF_TLM_HkTlm_t;

#endif /* RF_TLM_MSG_H */
