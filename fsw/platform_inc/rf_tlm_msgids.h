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
 * Define RF Telemetry Output App Message IDs
 *
 * \note The RF Telemetry Output App assumes default configuration which uses V1 of message id implementation
 */

#ifndef RF_TLM_MSGIDS_H
#define RF_TLM_MSGIDS_H

/* V1 Command Message IDs must be 0x18xx */
#define RF_TLM_CMD_MID     0x18D7
#define RF_TLM_SEND_HK_MID 0x18D8
/* V1 Telemetry Message IDs must be 0x08xx */
#define RF_TLM_HK_TLM_MID 0x08D9

#endif /* RF_TLM_MSGIDS_H */
