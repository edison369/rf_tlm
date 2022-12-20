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
 *  The RF Telemetry Output App header file containing version information
 */

#ifndef RF_TLM_VERSION_H
#define RF_TLM_VERSION_H

/* Development Build Macro Definitions */

#define RF_TLM_BUILD_NUMBER 35 /*!< Development Build: Number of commits since baseline */
#define RF_TLM_BUILD_BASELINE \
    "v1.3.0-rc4" /*!< Development Build: git tag that is the base for the current development */

/*
 * Version Macros, see \ref cfsversions for definitions.
 */
#define RF_TLM_MAJOR_VERSION 1  /*!< @brief Major version number. */
#define RF_TLM_MINOR_VERSION 1  /*!< @brief Minor version number. */
#define RF_TLM_REVISION      99 /*!< @brief Revision version number. Value of 99 indicates a development version.*/

/*!
 * @brief Mission revision.
 *
 * Reserved for mission use to denote patches/customizations as needed.
 * Values 1-254 are reserved for mission use to denote patches/customizations as needed. NOTE: Reserving 0 and 0xFF for
 * cFS open-source development use (pending resolution of nasa/cFS#440)
 */
#define RF_TLM_MISSION_REV 0xFF

#define RF_TLM_STR_HELPER(x) #x /*!< @brief Helper function to concatenate strings from integer macros */
#define RF_TLM_STR(x) \
    RF_TLM_STR_HELPER(x) /*!< @brief Helper function to concatenate strings from integer macros */

/*! @brief Development Build Version Number.
 * @details Baseline git tag + Number of commits since baseline. @n
 * See @ref cfsversions for format differences between development and release versions.
 */
#define RF_TLM_VERSION RF_TLM_BUILD_BASELINE "+dev" RF_TLM_STR(RF_TLM_BUILD_NUMBER)

/*! @brief Development Build Version String.
 * @details Reports the current development build's baseline, number, and name. Also includes a note about the latest
 * official version. @n See @ref cfsversions for format differences between development and release versions.
 */
#define RF_TLM_VERSION_STRING                       \
    " RF Telemetry Output App DEVELOPMENT BUILD " RF_TLM_VERSION \
    ", Last Official Release: v1.1.0" /* For full support please use this version */

#endif /* RF_TLM_VERSION_H */
