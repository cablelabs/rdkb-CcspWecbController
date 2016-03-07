/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/


/**************************************************************************

    module: cosa_led_dml.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/18/2011    initial revision.

**************************************************************************/


#ifndef  _COSA_LED_DML_H
#define  _COSA_LED_DML_H

/***********************************************************************

 APIs for Object:

    Device.X_COMCAST-COM_LED.

    *  LED_GetParamUlongValue

***********************************************************************/
BOOL
LED_GetParamUlongValue

(
    ANSC_HANDLE                 hInsContext,
    char*                       ParamName,
    ULONG*                      puLong
);

/***********************************************************************

 APIs for Object:

    Device. X_COMCAST-COM_LED.Panel.{i}.

    *  Panel_GetEntryCount
    *  Panel_GetEntry
    *  Panel_IsUpdated
    *  Panel_Synchronize
    *  Panel_GetParamBoolValue
    *  Panel_SetParamBoolValue
    *  Panel_GetParamUlongValue
    *  Panel_SetParamUlongValue
    *  Panel_GetParamStringValue
    *  Panel_SetParamStringValue
    *  Panel_Validate
    *  Panel_Commit
    *  Panel_Rollback

***********************************************************************/
BOOL
Panel_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Panel_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Panel_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    );

ANSC_HANDLE
Panel_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
Panel_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
Panel_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
Panel_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
Panel_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

ULONG
Panel_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
Panel_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
Panel_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
Panel_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Panel_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

/***********************************************************************

 APIs for Object:

    Device. X_COMCAST-COM_LED.RJ45.{i}.

    *  RJ45_GetEntryCount
    *  RJ45_GetEntry
    *  RJ45_IsUpdated
    *  RJ45_Synchronize
    *  RJ45_GetParamBoolValue
    *  RJ45_SetParamBoolValue
    *  RJ45_GetParamStringValue
    *  RJ45_SetParamStringValue
    *  RJ45_Validate
    *  RJ45_Commit
    *  RJ45_Rollback

***********************************************************************/
BOOL
RJ45_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
RJ45_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
RJ45_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    );

ANSC_HANDLE
RJ45_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
RJ45_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
RJ45_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

ULONG
RJ45_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
RJ45_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
RJ45_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
RJ45_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
RJ45_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );
#endif
