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
 * Define RF Tlm App Events IDs
 */

#ifndef RF_TLM_EVENTS_H
#define RF_TLM_EVENTS_H

#define RF_TLM_RESERVED_EID          0
#define RF_TLM_STARTUP_INF_EID       1
#define RF_TLM_COMMAND_ERR_EID       2
#define RF_TLM_COMMANDNOP_INF_EID    3
#define RF_TLM_COMMANDRST_INF_EID    4
#define RF_TLM_INVALID_MSGID_ERR_EID 5
#define RF_TLM_LEN_ERR_EID           6
#define RF_TLM_PIPE_ERR_EID          7
#define RF_TLM_SUBSCRIBE_ERR_EID     8
#define RF_TLM_TLMOUTSTOP_ERR_EID    9
#define RF_TLM_GENUC_ERR_EID         10
#define RF_TLM_TLMOUTENA_INF_EID     11
#define RF_TLM_DEV_INF_EID           12

#define RF_TLM_EVENT_COUNTS          12

#endif /* RF_TLM_EVENTS_H */
