/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

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

/*
 * Copyright (c) 2008-2009 Cisco Systems, Inc. All rights reserved.
 *
 * Cisco Systems, Inc. retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have obtained
 * a separate written license from Cisco Systems, Inc., you are not authorized
 * to utilize all or a part of this computer program for any purpose (including
 * reproduction, distribution, modification, and compilation into object code),
 * and you must immediately destroy or return to Cisco Systems, Inc. all copies
 * of this computer program.  If you are licensed by Cisco Systems, Inc., your
 * rights to utilize this computer program are limited by the terms of that
 * license.  To obtain a license, please contact Cisco Systems, Inc.
 *
 * This computer program contains trade secrets owned by Cisco Systems, Inc.
 * and, unless unauthorized by Cisco Systems, Inc. in writing, you agree to
 * maintain the confidentiality of this computer program and related information
 * and to not disclose this computer program and related information to any
 * other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND CISCO
 * SYSTEMS, INC. EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#include "hdk_data.h"
#include "hdk_encode.h"
#include "hdk_client_methods.h"
#include "hdk_internal.h"
#include "hdk_logging.h"
#include "../wecb_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HDK_LOGGING
#  ifdef _MSC_VER
#    include <malloc.h>
#  else /* ndef _MSC_VER */
#    include <alloca.h>
#  endif /* def _MSC_VER */
#endif /* def HDK_LOGGING */

#ifdef _MSC_VER
/* Disable "'function': was declared deprecated" warning. */
#  pragma warning(disable : 4996)
#endif /* def _MSC_VER */

/*
 * Namespaces table
 */

static char* s_namespaces[] =
{
    "",
    "http://cisco.com/HNAPExt/",
    //"http://purenetworks.com/HNAPExt/",
    "http://purenetworks.com/HNAP1/",
    "http://schemas.xmlsoap.org/soap/envelope/",
};

#define s_namespaces_GetString(ixNamespace) \
    (s_namespaces[(int)ixNamespace])

static char* s_namespaces_FindNamespace(char* pszNamespace, char* pszNamespaceEnd, unsigned int* pIXNamespace)
{
    unsigned int cchNamespace = (pszNamespaceEnd ?
                                     (unsigned int)(pszNamespaceEnd - pszNamespace) :
                                     (pszNamespace ?
                                          strlen(pszNamespace) : 0));
    char** ppszNamespace;
    char** ppszNamespaceEnd = s_namespaces + sizeof(s_namespaces) / sizeof(*s_namespaces);
    for (ppszNamespace = s_namespaces; ppszNamespace != ppszNamespaceEnd; ppszNamespace++)
    {
        unsigned int cch = strlen(*ppszNamespace);
        if ((cch == cchNamespace && strncmp(*ppszNamespace, pszNamespace, cchNamespace) == 0))
        {
            *pIXNamespace = ppszNamespace - s_namespaces;
            return *ppszNamespace;
        }
    }
    return 0;
}


/*
 * HNAP element table
 */

typedef struct _HDK_ElementNode
{
    unsigned char ixNamespace;
    char* pszElement;
} HDK_ElementNode;

static int HDK_ExpandElementNodeURI(HDK_ElementNode* pElemNode, char* pszURI, size_t cchURI)
{
    size_t ix;

    const char* rgpsz[3];
    rgpsz[0] = s_namespaces_GetString(pElemNode->ixNamespace);
    rgpsz[1] = "";
    rgpsz[2] = pElemNode->pszElement;

    if (rgpsz[0][strlen(rgpsz[0]) - 1] != '/')
    {
        rgpsz[1] = "/";
    }

    for (ix = 0; ix < sizeof(rgpsz) / sizeof(*rgpsz) && cchURI; ix++)
    {
        for (; *rgpsz[ix] && cchURI; rgpsz[ix]++)
        {
            *pszURI++ = *rgpsz[ix];
            cchURI--;
        }
    }

    if (cchURI)
    {
        /* NULL-terminate the URI (if there is space) */
        *pszURI = 0;
    }

    return !!cchURI;
}

static int HDK_ElementNode_FindElement(char* pszNamespace, char* pszNamespaceEnd,
                                       char* pszElement, char* pszElementEnd,
                                       HDK_ElementNode* pelements, int cElements,
                                       unsigned int* pixElement)
{
    unsigned int ixNamespace;

    /* Find the namespace */
    if (!s_namespaces_FindNamespace(pszNamespace, pszNamespaceEnd, &ixNamespace))
    {
        return 0;
    }

    /* Binary search for the element */
    {
        unsigned int ix1 = 0;
        unsigned int ix2 = cElements - 1;
        while (ix1 <= ix2)
        {
            HDK_ElementNode* pElemNode = &pelements[(*pixElement = (ix1 + ix2) / 2)];
            int result = (ixNamespace < pElemNode->ixNamespace ? -1 :
                          (ixNamespace > pElemNode->ixNamespace ? 1 : 0));
            if (result == 0)
            {
                if (pszElementEnd)
                {
                    unsigned int cchElement = pszElementEnd - pszElement;
                    result = strncmp(pszElement, pElemNode->pszElement, cchElement);
                    if (result == 0 && *(pElemNode->pszElement + cchElement))
                    {
                        result = -1;
                    }
                }
                else
                {
                    result = strcmp(pszElement, pElemNode->pszElement);
                }
            }

            if (result == 0)
            {
                return 1;
            }
            else if (result < 0)
            {
                ix2 = *pixElement - 1;
            }
            else
            {
                ix1 = *pixElement + 1;
            }
        }
    }

    return 0;
}

static HDK_ElementNode s_elements[] =
{
    { /* HDK_Element__UNKNOWN__ */ 0, "" },
    { /* HDK_Element_detail */ 0, "detail" },
    { /* HDK_Element_faultactor */ 0, "faultactor" },
    { /* HDK_Element_faultcode */ 0, "faultcode" },
    { /* HDK_Element_faultstring */ 0, "faultstring" },
    { /* HDK_Element_Cisco_AC */ 1, "AC" },
    { /* HDK_Element_Cisco_ACList */ 1, "ACList" },
    { /* HDK_Element_Cisco_ACM */ 1, "ACM" },
    { /* HDK_Element_Cisco_AIFSN */ 1, "AIFSN" },
    { /* HDK_Element_Cisco_BSSID */ 1, "BSSID" },
    { /* HDK_Element_Cisco_BeaconInterval */ 1, "BeaconInterval" },
    { /* HDK_Element_Cisco_CWMax */ 1, "CWMax" },
    { /* HDK_Element_Cisco_CWMin */ 1, "CWMin" },
    { /* HDK_Element_Cisco_Channel */ 1, "Channel" },
    { /* HDK_Element_Cisco_ChannelWidth */ 1, "ChannelWidth" },
    { /* HDK_Element_Cisco_ClientInfo */ 1, "ClientInfo" },
    { /* HDK_Element_Cisco_ClientInfoLists */ 1, "ClientInfoLists" },
    { /* HDK_Element_Cisco_Coexistance */ 1, "Coexistance" },
    { /* HDK_Element_Cisco_DTIMInterval */ 1, "DTIMInterval" },
    { /* HDK_Element_Cisco_DeviceName */ 1, "DeviceName" },
    { /* HDK_Element_Cisco_Dst */ 1, "Dst" },
    { /* HDK_Element_Cisco_Enable */ 1, "Enable" },
    { /* HDK_Element_Cisco_Encryption */ 1, "Encryption" },
    { /* HDK_Element_Cisco_Epoch */ 1, "Epoch" },
    { /* HDK_Element_Cisco_ExtenderState */ 1, "ExtenderState" },
    { /* HDK_Element_Cisco_FilterType */ 1, "FilterType" },
    { /* HDK_Element_Cisco_GetClientInfo */ 1, "GetClientInfo" },
    { /* HDK_Element_Cisco_GetClientInfoResponse */ 1, "GetClientInfoResponse" },
    { /* HDK_Element_Cisco_GetClientInfoResult */ 1, "GetClientInfoResult" },
    { /* HDK_Element_Cisco_GetExtenderStatus */ 1, "GetExtenderStatus" },
    { /* HDK_Element_Cisco_GetExtenderStatusResponse */ 1, "GetExtenderStatusResponse" },
    { /* HDK_Element_Cisco_GetExtenderStatusResult */ 1, "GetExtenderStatusResult" },
    { /* HDK_Element_Cisco_GuardInterval */ 1, "GuardInterval" },
    { /* HDK_Element_Cisco_Hour */ 1, "Hour" },
    { /* HDK_Element_Cisco_MACAddress */ 1, "MACAddress" },
    { /* HDK_Element_Cisco_MACList */ 1, "MACList" },
    { /* HDK_Element_Cisco_MaxClients */ 1, "MaxClients" },
    { /* HDK_Element_Cisco_Minutes */ 1, "Minutes" },
    { /* HDK_Element_Cisco_Mode */ 1, "Mode" },
    { /* HDK_Element_Cisco_ModeEnabled */ 1, "ModeEnabled" },
    { /* HDK_Element_Cisco_Month */ 1, "Month" },
    { /* HDK_Element_Cisco_MonthDay */ 1, "MonthDay" },
    { /* HDK_Element_Cisco_NoACK */ 1, "NoACK" },
    { /* HDK_Element_Cisco_PINCode */ 1, "PINCode" },
    { /* HDK_Element_Cisco_Passphrase */ 1, "Passphrase" },
    { /* HDK_Element_Cisco_PreSharedKey */ 1, "PreSharedKey" },
    { /* HDK_Element_Cisco_Qos */ 1, "Qos" },
    { /* HDK_Element_Cisco_QosSettings */ 1, "QosSettings" },
    { /* HDK_Element_Cisco_RadioID */ 1, "RadioID" },
    { /* HDK_Element_Cisco_RadioList */ 1, "RadioList" },
    { /* HDK_Element_Cisco_RadioSettings */ 1, "RadioSettings" },
    { /* HDK_Element_Cisco_RadiusSecret */ 1, "RadiusSecret" },
    { /* HDK_Element_Cisco_RadiusServerIP */ 1, "RadiusServerIP" },
    { /* HDK_Element_Cisco_RadiusServerPort */ 1, "RadiusServerPort" },
    { /* HDK_Element_Cisco_RekeyInterval */ 1, "RekeyInterval" },
    { /* HDK_Element_Cisco_SSID */ 1, "SSID" },
    { /* HDK_Element_Cisco_SSIDBroadcast */ 1, "SSIDBroadcast" },
    { /* HDK_Element_Cisco_SSIDEnabled */ 1, "SSIDEnabled" },
    { /* HDK_Element_Cisco_SSIDEncryption */ 1, "SSIDEncryption" },
    { /* HDK_Element_Cisco_SSIDIndex */ 1, "SSIDIndex" },
    { /* HDK_Element_Cisco_SSIDLanBase */ 1, "SSIDLanBase" },
    { /* HDK_Element_Cisco_SSIDList */ 1, "SSIDList" },
    { /* HDK_Element_Cisco_SSIDQoS */ 1, "SSIDQoS" },
    { /* HDK_Element_Cisco_SSIDRadioID */ 1, "SSIDRadioID" },
    { /* HDK_Element_Cisco_SSIDSettings */ 1, "SSIDSettings" },
    { /* HDK_Element_Cisco_SSIDVlanID */ 1, "SSIDVlanID" },
    { /* HDK_Element_Cisco_SecondaryChannel */ 1, "SecondaryChannel" },
    { /* HDK_Element_Cisco_Seconds */ 1, "Seconds" },
    { /* HDK_Element_Cisco_SetDoRestart */ 1, "SetDoRestart" },
    { /* HDK_Element_Cisco_SetDoRestartResponse */ 1, "SetDoRestartResponse" },
    { /* HDK_Element_Cisco_SetDoRestartResult */ 1, "SetDoRestartResult" },
    { /* HDK_Element_Cisco_SetRadios */ 1, "SetRadios" },
    { /* HDK_Element_Cisco_SetRadiosResponse */ 1, "SetRadiosResponse" },
    { /* HDK_Element_Cisco_SetRadiosResult */ 1, "SetRadiosResult" },
    { /* HDK_Element_Cisco_SetSSIDSettings */ 1, "SetSSIDSettings" },
    { /* HDK_Element_Cisco_SetSSIDSettingsResponse */ 1, "SetSSIDSettingsResponse" },
    { /* HDK_Element_Cisco_SetSSIDSettingsResult */ 1, "SetSSIDSettingsResult" },
    { /* HDK_Element_Cisco_SetTOD */ 1, "SetTOD" },
    { /* HDK_Element_Cisco_SetTODResponse */ 1, "SetTODResponse" },
    { /* HDK_Element_Cisco_SetTODResult */ 1, "SetTODResult" },
    { /* HDK_Element_Cisco_SetWPS */ 1, "SetWPS" },
    { /* HDK_Element_Cisco_SetWPSResponse */ 1, "SetWPSResponse" },
    { /* HDK_Element_Cisco_SetWPSResult */ 1, "SetWPSResult" },
    { /* HDK_Element_Cisco_TXOPLimit */ 1, "TXOPLimit" },
    { /* HDK_Element_Cisco_Type */ 1, "Type" },
    { /* HDK_Element_Cisco_UAPSDEnable */ 1, "UAPSDEnable" },
    { /* HDK_Element_Cisco_WDay */ 1, "WDay" },
    { /* HDK_Element_Cisco_WMMEnable */ 1, "WMMEnable" },
    { /* HDK_Element_Cisco_WPSEnable */ 1, "WPSEnable" },
    { /* HDK_Element_Cisco_WepKey */ 1, "WepKey" },
    { /* HDK_Element_Cisco_YDay */ 1, "YDay" },
    { /* HDK_Element_Cisco_Year */ 1, "Year" },
    { /* HDK_Element_PN_Channel */ 2, "Channel" },
    { /* HDK_Element_PN_Channels */ 2, "Channels" },
    { /* HDK_Element_PN_DeviceName */ 2, "DeviceName" },
    { /* HDK_Element_PN_Encryptions */ 2, "Encryptions" },
    { /* HDK_Element_PN_FirmwareVersion */ 2, "FirmwareVersion" },
    { /* HDK_Element_PN_Frequency */ 2, "Frequency" },
    { /* HDK_Element_PN_GetDeviceSettings */ 2, "GetDeviceSettings" },
    { /* HDK_Element_PN_GetDeviceSettingsResponse */ 2, "GetDeviceSettingsResponse" },
    { /* HDK_Element_PN_GetDeviceSettingsResult */ 2, "GetDeviceSettingsResult" },
    { /* HDK_Element_PN_GetWLanRadios */ 2, "GetWLanRadios" },
    { /* HDK_Element_PN_GetWLanRadiosResponse */ 2, "GetWLanRadiosResponse" },
    { /* HDK_Element_PN_GetWLanRadiosResult */ 2, "GetWLanRadiosResult" },
    { /* HDK_Element_PN_IsDeviceReady */ 2, "IsDeviceReady" },
    { /* HDK_Element_PN_IsDeviceReadyResponse */ 2, "IsDeviceReadyResponse" },
    { /* HDK_Element_PN_IsDeviceReadyResult */ 2, "IsDeviceReadyResult" },
    { /* HDK_Element_PN_ModelDescription */ 2, "ModelDescription" },
    { /* HDK_Element_PN_ModelName */ 2, "ModelName" },
    { /* HDK_Element_PN_Name */ 2, "Name" },
    { /* HDK_Element_PN_PresentationURL */ 2, "PresentationURL" },
    { /* HDK_Element_PN_RadioID */ 2, "RadioID" },
    { /* HDK_Element_PN_RadioInfo */ 2, "RadioInfo" },
    { /* HDK_Element_PN_RadioInfos */ 2, "RadioInfos" },
    { /* HDK_Element_PN_SOAPActions */ 2, "SOAPActions" },
    { /* HDK_Element_PN_SecondaryChannels */ 2, "SecondaryChannels" },
    { /* HDK_Element_PN_SecurityInfo */ 2, "SecurityInfo" },
    { /* HDK_Element_PN_SecurityType */ 2, "SecurityType" },
    { /* HDK_Element_PN_SubDeviceURLs */ 2, "SubDeviceURLs" },
    { /* HDK_Element_PN_SupportedModes */ 2, "SupportedModes" },
    { /* HDK_Element_PN_SupportedSecurity */ 2, "SupportedSecurity" },
    { /* HDK_Element_PN_TaskExtension */ 2, "TaskExtension" },
    { /* HDK_Element_PN_Tasks */ 2, "Tasks" },
    { /* HDK_Element_PN_Type */ 2, "Type" },
    { /* HDK_Element_PN_URL */ 2, "URL" },
    { /* HDK_Element_PN_VendorName */ 2, "VendorName" },
    { /* HDK_Element_PN_WideChannel */ 2, "WideChannel" },
    { /* HDK_Element_PN_WideChannels */ 2, "WideChannels" },
    { /* HDK_Element_PN_int */ 2, "int" },
    { /* HDK_Element_PN_string */ 2, "string" },
    { /* HDK_Element__BODY__ */ 3, "Body" },
    { /* HDK_Element__ENVELOPE__ */ 3, "Envelope" },
    { /* HDK_Element__FAULT__ */ 3, "Fault" },
    { /* HDK_Element__HEADER__ */ 3, "Header" },
};

#define s_elements_GetNode(element) \
    (&s_elements[element])

static HDK_Element s_elements_FindElement(char* pszNamespace, char* pszNamespaceEnd,
                                          char* pszElement, char* pszElementEnd)
{
    unsigned int ixElement;
    if (HDK_ElementNode_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementEnd,
                                    s_elements, sizeof(s_elements) / sizeof(*s_elements),
                                    &ixElement))
    {
        return (HDK_Element)ixElement;
    }

    return HDK_Element__UNKNOWN__;
}

int HDK_ExpandElementURI(HDK_Element element, char* pszURI, unsigned int cchURI)
{
    return HDK_ExpandElementNodeURI(s_elements_GetNode(element), pszURI, cchURI);
}

/*
 * HNAP element structure table
 */

typedef enum _HDK_ElementTreeProperties
{
    HDK_ElementTreeProp_Optional = 0x01,
    HDK_ElementTreeProp_Unbounded = 0x02
} HDK_ElementTreeProperties;

typedef struct _HDK_ElementTreeNode
{
    unsigned int ixParent:8;
    unsigned int element:8;
    unsigned int type:6;
    unsigned int prop:2;
} HDK_ElementTreeNode;

static HDK_ElementTreeNode s_elementTree[] =
{
    { /* 0 */ 0, HDK_Element__ENVELOPE__, HDK_Type__UNKNOWN_ANY__, 0x00 },
    { /* 1 */ 0, HDK_Element__HEADER__, HDK_Type__UNKNOWN_ANY__, 0x00 },
    { /* 2 */ 0, HDK_Element__BODY__, HDK_Type__UNKNOWN__, 0x00 },
    { /* 3 */ 2, HDK_Element__FAULT__, HDK_Type__STRUCT__, 0x00 },
    { /* 4 */ 2, HDK_Element_Cisco_GetClientInfo, HDK_Type__STRUCT__, 0x00 },
    { /* 5 */ 2, HDK_Element_Cisco_GetClientInfoResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 6 */ 2, HDK_Element_Cisco_GetExtenderStatus, HDK_Type__STRUCT__, 0x00 },
    { /* 7 */ 2, HDK_Element_Cisco_GetExtenderStatusResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 8 */ 2, HDK_Element_Cisco_SetDoRestart, HDK_Type__STRUCT__, 0x00 },
    { /* 9 */ 2, HDK_Element_Cisco_SetDoRestartResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 10 */ 2, HDK_Element_Cisco_SetRadios, HDK_Type__STRUCT__, 0x00 },
    { /* 11 */ 2, HDK_Element_Cisco_SetRadiosResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 12 */ 2, HDK_Element_Cisco_SetSSIDSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 13 */ 2, HDK_Element_Cisco_SetSSIDSettingsResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 14 */ 2, HDK_Element_Cisco_SetTOD, HDK_Type__STRUCT__, 0x00 },
    { /* 15 */ 2, HDK_Element_Cisco_SetTODResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 16 */ 2, HDK_Element_Cisco_SetWPS, HDK_Type__STRUCT__, 0x00 },
    { /* 17 */ 2, HDK_Element_Cisco_SetWPSResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 18 */ 2, HDK_Element_PN_GetDeviceSettings, HDK_Type__STRUCT__, 0x00 },
    { /* 19 */ 2, HDK_Element_PN_GetDeviceSettingsResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 20 */ 2, HDK_Element_PN_GetWLanRadios, HDK_Type__STRUCT__, 0x00 },
    { /* 21 */ 2, HDK_Element_PN_GetWLanRadiosResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 22 */ 2, HDK_Element_PN_IsDeviceReady, HDK_Type__STRUCT__, 0x00 },
    { /* 23 */ 2, HDK_Element_PN_IsDeviceReadyResponse, HDK_Type__STRUCT__, 0x00 },
    { /* 24 */ 3, HDK_Element_faultcode, HDK_Type__STRING__, 0x00 },
    { /* 25 */ 3, HDK_Element_faultstring, HDK_Type__STRING__, 0x00 },
    { /* 26 */ 3, HDK_Element_faultactor, HDK_Type__STRING__, 0x01 },
    { /* 27 */ 3, HDK_Element_detail, HDK_Type__STRING__, 0x01 },
    { /* 28 */ 5, HDK_Element_Cisco_GetClientInfoResult, HDK_Type__RESULT__, 0x00 },
    { /* 29 */ 5, HDK_Element_Cisco_ClientInfoLists, HDK_Type__STRUCT__, 0x00 },
    { /* 30 */ 7, HDK_Element_Cisco_GetExtenderStatusResult, HDK_Type__RESULT__, 0x00 },
    { /* 31 */ 7, HDK_Element_Cisco_ExtenderState, HDK_Type__INT__, 0x00 },
    { /* 32 */ 7, HDK_Element_Cisco_RadioList, HDK_Type__STRUCT__, 0x00 },
    { /* 33 */ 7, HDK_Element_Cisco_SSIDList, HDK_Type__STRUCT__, 0x00 },
    { /* 34 */ 9, HDK_Element_Cisco_SetDoRestartResult, HDK_Type__RESULT__, 0x00 },
    { /* 35 */ 10, HDK_Element_Cisco_RadioList, HDK_Type__STRUCT__, 0x00 },
    { /* 36 */ 11, HDK_Element_Cisco_SetRadiosResult, HDK_Type__RESULT__, 0x00 },
    { /* 37 */ 12, HDK_Element_Cisco_SSIDList, HDK_Type__STRUCT__, 0x00 },
    { /* 38 */ 13, HDK_Element_Cisco_SetSSIDSettingsResult, HDK_Type__RESULT__, 0x00 },
    { /* 39 */ 14, HDK_Element_Cisco_Seconds, HDK_Type__INT__, 0x00 },
    { /* 40 */ 14, HDK_Element_Cisco_Minutes, HDK_Type__INT__, 0x00 },
    { /* 41 */ 14, HDK_Element_Cisco_Hour, HDK_Type__INT__, 0x00 },
    { /* 42 */ 14, HDK_Element_Cisco_MonthDay, HDK_Type__INT__, 0x00 },
    { /* 43 */ 14, HDK_Element_Cisco_Month, HDK_Type__INT__, 0x00 },
    { /* 44 */ 14, HDK_Element_Cisco_Year, HDK_Type__INT__, 0x00 },
    { /* 45 */ 14, HDK_Element_Cisco_WDay, HDK_Type__INT__, 0x00 },
    { /* 46 */ 14, HDK_Element_Cisco_YDay, HDK_Type__INT__, 0x00 },
    { /* 47 */ 14, HDK_Element_Cisco_Dst, HDK_Type__BOOL__, 0x00 },
    { /* 48 */ 14, HDK_Element_Cisco_Epoch, HDK_Type__INT__, 0x00 },
    { /* 49 */ 15, HDK_Element_Cisco_SetTODResult, HDK_Type__RESULT__, 0x00 },
    { /* 50 */ 16, HDK_Element_Cisco_WPSEnable, HDK_Type__BOOL__, 0x00 },
    { /* 51 */ 16, HDK_Element_Cisco_PINCode, HDK_Type__STRING__, 0x00 },
    { /* 52 */ 16, HDK_Element_Cisco_SSIDIndex, HDK_Type__STRING__, 0x00 },
    { /* 53 */ 17, HDK_Element_Cisco_SetWPSResult, HDK_Type__RESULT__, 0x00 },
    { /* 54 */ 19, HDK_Element_PN_GetDeviceSettingsResult, HDK_Type__RESULT__, 0x00 },
    { /* 55 */ 19, HDK_Element_PN_Type, HDK_Type_PN_DeviceType, 0x00 },
    { /* 56 */ 19, HDK_Element_PN_DeviceName, HDK_Type__STRING__, 0x00 },
    { /* 57 */ 19, HDK_Element_PN_VendorName, HDK_Type__STRING__, 0x00 },
    { /* 58 */ 19, HDK_Element_PN_ModelDescription, HDK_Type__STRING__, 0x00 },
    { /* 59 */ 19, HDK_Element_PN_ModelName, HDK_Type__STRING__, 0x00 },
    { /* 60 */ 19, HDK_Element_PN_FirmwareVersion, HDK_Type__STRING__, 0x00 },
    { /* 61 */ 19, HDK_Element_PN_PresentationURL, HDK_Type__STRING__, 0x00 },
    { /* 62 */ 19, HDK_Element_PN_SOAPActions, HDK_Type__STRUCT__, 0x00 },
    { /* 63 */ 19, HDK_Element_PN_SubDeviceURLs, HDK_Type__STRUCT__, 0x00 },
    { /* 64 */ 19, HDK_Element_PN_Tasks, HDK_Type__STRUCT__, 0x00 },
    { /* 65 */ 21, HDK_Element_PN_GetWLanRadiosResult, HDK_Type__RESULT__, 0x00 },
    { /* 66 */ 21, HDK_Element_PN_RadioInfos, HDK_Type__STRUCT__, 0x00 },
    { /* 67 */ 23, HDK_Element_PN_IsDeviceReadyResult, HDK_Type__RESULT__, 0x00 },
    { /* 68 */ 29, HDK_Element_Cisco_ClientInfo, HDK_Type__STRUCT__, 0x03 },
    { /* 69 */ 32, HDK_Element_Cisco_RadioSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 70 */ 33, HDK_Element_Cisco_SSIDSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 71 */ 35, HDK_Element_Cisco_RadioSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 72 */ 37, HDK_Element_Cisco_SSIDSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 73 */ 62, HDK_Element_PN_string, HDK_Type__STRING__, 0x03 },
    { /* 74 */ 63, HDK_Element_PN_string, HDK_Type__STRING__, 0x03 },
    { /* 75 */ 64, HDK_Element_PN_TaskExtension, HDK_Type__STRUCT__, 0x03 },
    { /* 76 */ 66, HDK_Element_PN_RadioInfo, HDK_Type__STRUCT__, 0x03 },
    { /* 77 */ 68, HDK_Element_Cisco_MACAddress, HDK_Type__MACADDRESS__, 0x00 },
    { /* 78 */ 68, HDK_Element_Cisco_Type, HDK_Type_Cisco_DeviceInf, 0x00 },
    { /* 79 */ 68, HDK_Element_Cisco_DeviceName, HDK_Type__STRING__, 0x01 },
    { /* 80 */ 69, HDK_Element_Cisco_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 81 */ 69, HDK_Element_Cisco_Enable, HDK_Type__BOOL__, 0x00 },
    { /* 82 */ 69, HDK_Element_Cisco_Mode, HDK_Type_Cisco_WiFiMode, 0x00 },
    { /* 83 */ 69, HDK_Element_Cisco_ChannelWidth, HDK_Type__INT__, 0x00 },
    { /* 84 */ 69, HDK_Element_Cisco_Channel, HDK_Type__INT__, 0x00 },
    { /* 85 */ 69, HDK_Element_Cisco_SecondaryChannel, HDK_Type__INT__, 0x00 },
    { /* 86 */ 69, HDK_Element_Cisco_BeaconInterval, HDK_Type__INT__, 0x00 },
    { /* 87 */ 69, HDK_Element_Cisco_DTIMInterval, HDK_Type__INT__, 0x00 },
    { /* 88 */ 69, HDK_Element_Cisco_GuardInterval, HDK_Type__INT__, 0x00 },
    { /* 89 */ 69, HDK_Element_Cisco_Coexistance, HDK_Type__BOOL__, 0x00 },
    { /* 90 */ 70, HDK_Element_Cisco_SSIDRadioID, HDK_Type__STRING__, 0x00 },
    { /* 91 */ 70, HDK_Element_Cisco_SSIDIndex, HDK_Type__STRING__, 0x00 },
    { /* 92 */ 70, HDK_Element_Cisco_SSID, HDK_Type__STRING__, 0x00 },
    { /* 93 */ 70, HDK_Element_Cisco_BSSID, HDK_Type__STRING__, 0x01 },
    { /* 94 */ 70, HDK_Element_Cisco_SSIDEnabled, HDK_Type__BOOL__, 0x00 },
    { /* 95 */ 70, HDK_Element_Cisco_SSIDBroadcast, HDK_Type__BOOL__, 0x00 },
    { /* 96 */ 70, HDK_Element_Cisco_SSIDVlanID, HDK_Type__INT__, 0x00 },
    { /* 97 */ 70, HDK_Element_Cisco_SSIDLanBase, HDK_Type__IPADDRESS__, 0x00 },
    { /* 98 */ 70, HDK_Element_Cisco_SSIDEncryption, HDK_Type__STRUCT__, 0x01 },
    { /* 99 */ 70, HDK_Element_Cisco_SSIDQoS, HDK_Type__STRUCT__, 0x00 },
    { /* 100 */ 70, HDK_Element_Cisco_MaxClients, HDK_Type__INT__, 0x00 },
    { /* 101 */ 70, HDK_Element_Cisco_ACList, HDK_Type__STRUCT__, 0x01 },
    { /* 102 */ 71, HDK_Element_Cisco_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 103 */ 71, HDK_Element_Cisco_Enable, HDK_Type__BOOL__, 0x00 },
    { /* 104 */ 71, HDK_Element_Cisco_Mode, HDK_Type_Cisco_WiFiMode, 0x00 },
    { /* 105 */ 71, HDK_Element_Cisco_ChannelWidth, HDK_Type__INT__, 0x00 },
    { /* 106 */ 71, HDK_Element_Cisco_Channel, HDK_Type__INT__, 0x00 },
    { /* 107 */ 71, HDK_Element_Cisco_SecondaryChannel, HDK_Type__INT__, 0x00 },
    { /* 108 */ 71, HDK_Element_Cisco_BeaconInterval, HDK_Type__INT__, 0x00 },
    { /* 109 */ 71, HDK_Element_Cisco_DTIMInterval, HDK_Type__INT__, 0x00 },
    { /* 110 */ 71, HDK_Element_Cisco_GuardInterval, HDK_Type__INT__, 0x00 },
    { /* 111 */ 71, HDK_Element_Cisco_Coexistance, HDK_Type__BOOL__, 0x00 },
    { /* 112 */ 72, HDK_Element_Cisco_SSIDRadioID, HDK_Type__STRING__, 0x00 },
    { /* 113 */ 72, HDK_Element_Cisco_SSIDIndex, HDK_Type__STRING__, 0x00 },
    { /* 114 */ 72, HDK_Element_Cisco_SSID, HDK_Type__STRING__, 0x00 },
    { /* 115 */ 72, HDK_Element_Cisco_BSSID, HDK_Type__STRING__, 0x01 },
    { /* 116 */ 72, HDK_Element_Cisco_SSIDEnabled, HDK_Type__BOOL__, 0x00 },
    { /* 117 */ 72, HDK_Element_Cisco_SSIDBroadcast, HDK_Type__BOOL__, 0x00 },
    { /* 118 */ 72, HDK_Element_Cisco_SSIDVlanID, HDK_Type__INT__, 0x00 },
    { /* 119 */ 72, HDK_Element_Cisco_SSIDLanBase, HDK_Type__IPADDRESS__, 0x00 },
    { /* 120 */ 72, HDK_Element_Cisco_SSIDEncryption, HDK_Type__STRUCT__, 0x01 },
    { /* 121 */ 72, HDK_Element_Cisco_SSIDQoS, HDK_Type__STRUCT__, 0x00 },
    { /* 122 */ 72, HDK_Element_Cisco_MaxClients, HDK_Type__INT__, 0x00 },
    { /* 123 */ 72, HDK_Element_Cisco_ACList, HDK_Type__STRUCT__, 0x01 },
    { /* 124 */ 75, HDK_Element_PN_Name, HDK_Type__STRING__, 0x00 },
    { /* 125 */ 75, HDK_Element_PN_URL, HDK_Type__STRING__, 0x00 },
    { /* 126 */ 75, HDK_Element_PN_Type, HDK_Type_PN_TaskExtType, 0x00 },
    { /* 127 */ 76, HDK_Element_PN_RadioID, HDK_Type__STRING__, 0x00 },
    { /* 128 */ 76, HDK_Element_PN_Frequency, HDK_Type__INT__, 0x00 },
    { /* 129 */ 76, HDK_Element_PN_SupportedModes, HDK_Type__STRUCT__, 0x00 },
    { /* 130 */ 76, HDK_Element_PN_Channels, HDK_Type__STRUCT__, 0x00 },
    { /* 131 */ 76, HDK_Element_PN_WideChannels, HDK_Type__STRUCT__, 0x00 },
    { /* 132 */ 76, HDK_Element_PN_SupportedSecurity, HDK_Type__STRUCT__, 0x00 },
    { /* 133 */ 98, HDK_Element_Cisco_ModeEnabled, HDK_Type_Cisco_WiFiSecurity, 0x00 },
    { /* 134 */ 98, HDK_Element_Cisco_Encryption, HDK_Type_Cisco_WiFiEncryption, 0x01 },
    { /* 135 */ 98, HDK_Element_Cisco_WepKey, HDK_Type__STRING__, 0x01 },
    { /* 136 */ 98, HDK_Element_Cisco_PreSharedKey, HDK_Type__STRING__, 0x01 },
    { /* 137 */ 98, HDK_Element_Cisco_Passphrase, HDK_Type__STRING__, 0x01 },
    { /* 138 */ 98, HDK_Element_Cisco_RekeyInterval, HDK_Type__INT__, 0x01 },
    { /* 139 */ 98, HDK_Element_Cisco_RadiusServerIP, HDK_Type__IPADDRESS__, 0x01 },
    { /* 140 */ 98, HDK_Element_Cisco_RadiusServerPort, HDK_Type__INT__, 0x01 },
    { /* 141 */ 98, HDK_Element_Cisco_RadiusSecret, HDK_Type__STRING__, 0x01 },
    { /* 142 */ 99, HDK_Element_Cisco_WMMEnable, HDK_Type__BOOL__, 0x00 },
    { /* 143 */ 99, HDK_Element_Cisco_UAPSDEnable, HDK_Type__BOOL__, 0x00 },
    { /* 144 */ 99, HDK_Element_Cisco_Qos, HDK_Type__STRUCT__, 0x00 },
    { /* 145 */ 101, HDK_Element_Cisco_FilterType, HDK_Type__STRING__, 0x00 },
    { /* 146 */ 101, HDK_Element_Cisco_MACList, HDK_Type__STRUCT__, 0x01 },
    { /* 147 */ 120, HDK_Element_Cisco_ModeEnabled, HDK_Type_Cisco_WiFiSecurity, 0x00 },
    { /* 148 */ 120, HDK_Element_Cisco_Encryption, HDK_Type_Cisco_WiFiEncryption, 0x01 },
    { /* 149 */ 120, HDK_Element_Cisco_WepKey, HDK_Type__STRING__, 0x01 },
    { /* 150 */ 120, HDK_Element_Cisco_PreSharedKey, HDK_Type__STRING__, 0x01 },
    { /* 151 */ 120, HDK_Element_Cisco_Passphrase, HDK_Type__STRING__, 0x01 },
    { /* 152 */ 120, HDK_Element_Cisco_RekeyInterval, HDK_Type__INT__, 0x01 },
    { /* 153 */ 120, HDK_Element_Cisco_RadiusServerIP, HDK_Type__IPADDRESS__, 0x01 },
    { /* 154 */ 120, HDK_Element_Cisco_RadiusServerPort, HDK_Type__INT__, 0x01 },
    { /* 155 */ 120, HDK_Element_Cisco_RadiusSecret, HDK_Type__STRING__, 0x01 },
    { /* 156 */ 121, HDK_Element_Cisco_WMMEnable, HDK_Type__BOOL__, 0x00 },
    { /* 157 */ 121, HDK_Element_Cisco_UAPSDEnable, HDK_Type__BOOL__, 0x00 },
    { /* 158 */ 121, HDK_Element_Cisco_Qos, HDK_Type__STRUCT__, 0x00 },
    { /* 159 */ 123, HDK_Element_Cisco_FilterType, HDK_Type__STRING__, 0x00 },
    { /* 160 */ 123, HDK_Element_Cisco_MACList, HDK_Type__STRUCT__, 0x01 },
    { /* 161 */ 129, HDK_Element_PN_string, HDK_Type_PN_WiFiMode, 0x03 },
    { /* 162 */ 130, HDK_Element_PN_int, HDK_Type__INT__, 0x03 },
    { /* 163 */ 131, HDK_Element_PN_WideChannel, HDK_Type__STRUCT__, 0x03 },
    { /* 164 */ 132, HDK_Element_PN_SecurityInfo, HDK_Type__STRUCT__, 0x03 },
    { /* 165 */ 144, HDK_Element_Cisco_QosSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 166 */ 146, HDK_Element_Cisco_MACAddress, HDK_Type__MACADDRESS__, 0x03 },
    { /* 167 */ 158, HDK_Element_Cisco_QosSettings, HDK_Type__STRUCT__, 0x03 },
    { /* 168 */ 160, HDK_Element_Cisco_MACAddress, HDK_Type__MACADDRESS__, 0x03 },
    { /* 169 */ 163, HDK_Element_PN_Channel, HDK_Type__INT__, 0x00 },
    { /* 170 */ 163, HDK_Element_PN_SecondaryChannels, HDK_Type__STRUCT__, 0x00 },
    { /* 171 */ 164, HDK_Element_PN_SecurityType, HDK_Type_PN_WiFiSecurity, 0x00 },
    { /* 172 */ 164, HDK_Element_PN_Encryptions, HDK_Type__STRUCT__, 0x00 },
    { /* 173 */ 165, HDK_Element_Cisco_AC, HDK_Type__INT__, 0x01 },
    { /* 174 */ 165, HDK_Element_Cisco_ACM, HDK_Type__BOOL__, 0x01 },
    { /* 175 */ 165, HDK_Element_Cisco_AIFSN, HDK_Type__INT__, 0x01 },
    { /* 176 */ 165, HDK_Element_Cisco_CWMin, HDK_Type__INT__, 0x01 },
    { /* 177 */ 165, HDK_Element_Cisco_CWMax, HDK_Type__INT__, 0x01 },
    { /* 178 */ 165, HDK_Element_Cisco_TXOPLimit, HDK_Type__INT__, 0x01 },
    { /* 179 */ 165, HDK_Element_Cisco_NoACK, HDK_Type__BOOL__, 0x01 },
    { /* 180 */ 167, HDK_Element_Cisco_AC, HDK_Type__INT__, 0x01 },
    { /* 181 */ 167, HDK_Element_Cisco_ACM, HDK_Type__BOOL__, 0x01 },
    { /* 182 */ 167, HDK_Element_Cisco_AIFSN, HDK_Type__INT__, 0x01 },
    { /* 183 */ 167, HDK_Element_Cisco_CWMin, HDK_Type__INT__, 0x01 },
    { /* 184 */ 167, HDK_Element_Cisco_CWMax, HDK_Type__INT__, 0x01 },
    { /* 185 */ 167, HDK_Element_Cisco_TXOPLimit, HDK_Type__INT__, 0x01 },
    { /* 186 */ 167, HDK_Element_Cisco_NoACK, HDK_Type__BOOL__, 0x01 },
    { /* 187 */ 170, HDK_Element_PN_int, HDK_Type__INT__, 0x03 },
    { /* 188 */ 172, HDK_Element_PN_string, HDK_Type_PN_WiFiEncryption, 0x03 },
};

#define s_elementTree_GetNode(ixNode) \
    (&s_elementTree[ixNode])

static HDK_ElementTreeNode* s_elementTree_GetChildNode(unsigned int ixParent, HDK_Element element, unsigned int* pixChild)
{
    unsigned int ix1 = 0;
    unsigned int ix2 = sizeof(s_elementTree) / sizeof(*s_elementTree) - 1;
    while (ix1 <= ix2)
    {
        unsigned int ix = (ix1 + ix2) / 2;
        HDK_ElementTreeNode* pNode = &s_elementTree[ix];
        if (ixParent == pNode->ixParent)
        {
            HDK_ElementTreeNode* pElementTreeEnd = s_elementTree + sizeof(s_elementTree) / sizeof(*s_elementTree);
            HDK_ElementTreeNode* pNode2;
            for (pNode2 = pNode; pNode2 >= s_elementTree && pNode2->ixParent == ixParent; pNode2--)
            {
                if ((HDK_Element)pNode2->element == element)
                {
                    *pixChild = pNode2 - s_elementTree;
                    return pNode2;
                }
            }
            for (pNode2 = pNode + 1; pNode2 < pElementTreeEnd && pNode2->ixParent == ixParent; pNode2++)
            {
                if ((HDK_Element)pNode2->element == element)
                {
                    *pixChild = pNode2 - s_elementTree;
                    return pNode2;
                }
            }
            break;
        }
        else if (ixParent < pNode->ixParent)
        {
            ix2 = ix - 1;
        }
        else
        {
            ix1 = ix + 1;
        }
    }

    return 0;
}

static HDK_ElementTreeNode* s_elementTree_GetChildNodes(unsigned int ixParent, unsigned int* pixChildBegin, unsigned int* pixChildEnd)
{
    unsigned int ix1 = 0;
    unsigned int ix2 = sizeof(s_elementTree) / sizeof(*s_elementTree) - 1;
    while (ix1 <= ix2)
    {
        unsigned int ix = (ix1 + ix2) / 2;
        HDK_ElementTreeNode* pNode = &s_elementTree[ix];
        if (ixParent == pNode->ixParent)
        {
            HDK_ElementTreeNode* pElementTreeEnd = s_elementTree + sizeof(s_elementTree) / sizeof(*s_elementTree);
            HDK_ElementTreeNode* pNodeBegin;
            HDK_ElementTreeNode* pNodeEnd;
            for (pNodeBegin = pNode - 1; pNodeBegin >= s_elementTree && pNodeBegin->ixParent == ixParent; pNodeBegin--) {}
            *pixChildBegin = pNodeBegin - s_elementTree + 1;
            for (pNodeEnd = pNode + 1; pNodeEnd < pElementTreeEnd && pNodeEnd->ixParent == ixParent; pNodeEnd++) {}
            *pixChildEnd = pNodeEnd - s_elementTree;
            return pNodeBegin + 1;
        }
        else if (ixParent < pNode->ixParent)
        {
            ix2 = ix - 1;
        }
        else
        {
            ix1 = ix + 1;
        }
    }
    return 0;
}


/*
 * HNAP type interface
 */

typedef HDK_Member* (*HDK_TypeFn_New)(void);
typedef void (*HDK_TypeFn_Free)(HDK_Member* pMember);
typedef int (*HDK_TypeFn_Serialize)(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember);
typedef int (*HDK_TypeFn_Deserialize)(HDK_Member* pMember, char* pszValue);

typedef struct _HDK_TypeInfo
{
    HDK_TypeFn_New pfnNew;
    HDK_TypeFn_Free pfnFree;
    HDK_TypeFn_Serialize pfnSerialize;
    HDK_TypeFn_Deserialize pfnDeserialize;
} HDK_TypeInfo;

static HDK_TypeInfo* s_types_GetInfo(HDK_Type type);


/*
 * HNAP generic member functions
 */

HDK_Member* HDK_Copy_Member(HDK_Struct* pStructDst, HDK_Element elementDst,
                            HDK_Member* pMemberSrc, int fAppend)
{
    HDK_Member* pMemberDst = 0;

    switch (pMemberSrc->type)
    {
        case HDK_Type__STRUCT__:
            if (!fAppend)
            {
                pMemberDst = (HDK_Member*)HDK_Set_Struct(pStructDst, elementDst);
            }
            else
            {
                pMemberDst = (HDK_Member*)HDK_Append_Struct(pStructDst, elementDst);
            }
            if (pMemberDst)
            {
                HDK_Member* pChild;
                for (pChild = ((HDK_Struct*)pMemberSrc)->pHead; pChild; pChild = pChild->pNext)
                {
                    if (!HDK_Copy_Member((HDK_Struct*)pMemberDst, pChild->element, pChild, 1))
                    {
                        pMemberSrc = 0;
                        break;
                    }
                }
            }
            break;

        case HDK_Type__BLANK__:
            if (!fAppend)
            {
                pMemberDst = HDK_Set_Blank(pStructDst, elementDst);
            }
            else
            {
                pMemberDst = HDK_Append_Blank(pStructDst, elementDst);
            }
            break;

        case HDK_Type__IPADDRESS__:
            {
                HDK_IPAddress* pIPAddress = HDK_Get_IPAddressMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_IPAddress(pStructDst, elementDst, pIPAddress);
                }
                else
                {
                    pMemberDst = HDK_Append_IPAddress(pStructDst, elementDst, pIPAddress);
                }
            }
            break;

        case HDK_Type__MACADDRESS__:
            {
                HDK_MACAddress* pMACAddress = HDK_Get_MACAddressMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_MACAddress(pStructDst, elementDst, pMACAddress);
                }
                else
                {
                    pMemberDst = HDK_Append_MACAddress(pStructDst, elementDst, pMACAddress);
                }
            }
            break;

        case HDK_Type__BOOL__:
            {
                int fValue = *HDK_Get_BoolMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Bool(pStructDst, elementDst, fValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Bool(pStructDst, elementDst, fValue);
                }
            }
            break;

        case HDK_Type__INT__:
            {
                int iValue = *HDK_Get_IntMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Int(pStructDst, elementDst, iValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Int(pStructDst, elementDst, iValue);
                }
            }
            break;

        case HDK_Type__STRING__:
            {
                char* pszValue = HDK_Get_StringMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_String(pStructDst, elementDst, pszValue);
                }
                else
                {
                    pMemberDst = HDK_Append_String(pStructDst, elementDst, pszValue);
                }
            }
            break;

        case HDK_Type__RESULT__:
            {
                HDK_Enum_Result eValue = *HDK_Get_ResultMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Result(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Result(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_Cisco_DeviceInf:
            {
                HDK_Enum_Cisco_DeviceInf eValue = *HDK_Get_Cisco_DeviceInfMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Cisco_DeviceInf(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Cisco_DeviceInf(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_Cisco_WiFiEncryption:
            {
                HDK_Enum_Cisco_WiFiEncryption eValue = *HDK_Get_Cisco_WiFiEncryptionMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Cisco_WiFiEncryption(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Cisco_WiFiEncryption(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_Cisco_WiFiMode:
            {
                HDK_Enum_Cisco_WiFiMode eValue = *HDK_Get_Cisco_WiFiModeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Cisco_WiFiMode(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Cisco_WiFiMode(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_Cisco_WiFiSecurity:
            {
                HDK_Enum_Cisco_WiFiSecurity eValue = *HDK_Get_Cisco_WiFiSecurityMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_Cisco_WiFiSecurity(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_Cisco_WiFiSecurity(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_DeviceType:
            {
                HDK_Enum_PN_DeviceType eValue = *HDK_Get_PN_DeviceTypeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_DeviceType(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_DeviceType(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_TaskExtType:
            {
                HDK_Enum_PN_TaskExtType eValue = *HDK_Get_PN_TaskExtTypeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_TaskExtType(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_TaskExtType(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiEncryption:
            {
                HDK_Enum_PN_WiFiEncryption eValue = *HDK_Get_PN_WiFiEncryptionMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiEncryption(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiEncryption(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiMode:
            {
                HDK_Enum_PN_WiFiMode eValue = *HDK_Get_PN_WiFiModeMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiMode(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiMode(pStructDst, elementDst, eValue);
                }
            }
            break;

        case HDK_Type_PN_WiFiSecurity:
            {
                HDK_Enum_PN_WiFiSecurity eValue = *HDK_Get_PN_WiFiSecurityMember(pMemberSrc);
                if (!fAppend)
                {
                    pMemberDst = HDK_Set_PN_WiFiSecurity(pStructDst, elementDst, eValue);
                }
                else
                {
                    pMemberDst = HDK_Append_PN_WiFiSecurity(pStructDst, elementDst, eValue);
                }
            }
            break;

        /* Note: Copying streams is not supported */
        default:
            break;
    }

    return pMemberDst;
}

HDK_Member* HDK_Get_Member(HDK_Struct* pStruct, HDK_Element element, HDK_Type type)
{
    if (pStruct)
    {
        HDK_Member* pMember;
        for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
        {
            if (pMember->element == element && pMember->type == type)
            {
                return pMember;
            }
        }
    }
    return 0;
}

static void HDK_Append_Member(HDK_Struct* pStruct, HDK_Member* pMember)
{
    /* Find the list tail */
    if (!pStruct->pHead)
    {
        pStruct->pHead = pMember;
        pStruct->pTail = pMember;
    }
    else
    {
        pStruct->pTail->pNext = pMember;
        pStruct->pTail = pMember;
    }
}

/* Free a member node */
static void HDK_FreeMember(HDK_Member* pMember, int fStackStruct)
{
    if (pMember->type == HDK_Type__STRUCT__)
    {
        HDK_Member* pChildMember = ((HDK_Struct*)pMember)->pHead;
        while (pChildMember)
        {
            HDK_Member* pFree = pChildMember;
            pChildMember = pChildMember->pNext;
            HDK_FreeMember(pFree, 0);
        }
        if (!fStackStruct)
        {
            free(pMember);
        }
    }
    else
    {
        HDK_TypeInfo* pTypeInfo = s_types_GetInfo(pMember->type);
        if (pTypeInfo && pTypeInfo->pfnFree)
        {
            pTypeInfo->pfnFree(pMember);
        }
    }
}


/* Detach a member node */
void HDK_Detach_Struct(HDK_Member* pMember, HDK_Member* pMember2)
{
    if (pMember->type == HDK_Type__STRUCT__)
    {
        HDK_Member* pChildMember = ((HDK_Struct*)pMember)->pHead;
        while (pChildMember)
        {
			if (pChildMember->pNext == pMember2)
			{
				pChildMember->pNext = pMember2->pNext;
				break;
				
			}
			pChildMember = pChildMember->pNext;
        }
		free(pMember2);
    }
}

/*
 * HNAP generic structure functions
 */

HDK_Struct* HDK_Set_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Struct* pMember = HDK_Get_Struct(pStruct, element);
    if (pMember)
    {
        HDK_Struct_Free(pMember);
        return (HDK_Struct*)pMember;
    }
    else
    {
        return HDK_Append_Struct(pStruct, element);
    }
}

HDK_Struct* HDK_Set_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct)
{
    return (HDK_Struct*)HDK_Copy_Member(pStructDst, element, (HDK_Member*)pStruct, 0);
}

HDK_Struct* HDK_Append_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Struct* pMember = (HDK_Struct*)malloc(sizeof(HDK_Struct));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__STRUCT__;
        pMember->node.pNext = 0;
        pMember->pHead = 0;
        pMember->pTail = 0;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Struct*)pMember;
}

HDK_Struct* HDK_Append_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct)
{
    return (HDK_Struct*)HDK_Copy_Member(pStructDst, element, (HDK_Member*)pStruct, 1);
}

HDK_Struct* HDK_Get_Struct(HDK_Struct* pStruct, HDK_Element element)
{
    return (HDK_Struct*)HDK_Get_Member(pStruct, element, HDK_Type__STRUCT__);
}

HDK_Struct* HDK_Get_StructMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__STRUCT__ ? (HDK_Struct*)pMember : 0);
}


/*
 * HNAP struct stack initialization/free
 */

void HDK_Struct_Init(HDK_Struct* pStruct)
{
    pStruct->node.element = HDK_Element__UNKNOWN__;
    pStruct->node.type = HDK_Type__STRUCT__;
    pStruct->node.pNext = 0;
    pStruct->pHead = 0;
    pStruct->pTail = 0;
}

void HDK_Struct_Free(HDK_Struct* pStruct)
{
    HDK_FreeMember((HDK_Member*)pStruct, 1);
    pStruct->pHead = 0;
    pStruct->pTail = 0;
}

/*
 * HNAP explicit blank element - use sparingly
 */

static void HDK_Type__BLANK__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

HDK_Member* HDK_Set_Blank(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__BLANK__);
    if (pMember)
    {
        return pMember;
    }
    else
    {
        return HDK_Append_Blank(pStruct, element);
    }
}

HDK_Member* HDK_Append_Blank(HDK_Struct* pStruct, HDK_Element element)
{
    HDK_Member* pMember = (HDK_Member*)malloc(sizeof(HDK_Member));
    if (pMember)
    {
        pMember->element = element;
        pMember->type = HDK_Type__BLANK__;
        pMember->pNext = 0;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return pMember;
}


/*
 * HNAP int type
 */

typedef struct _HDK_Member__INT__
{
    HDK_Member node;
    int iValue;
} HDK_Member__INT__;

static HDK_Member* HDK_Type__INT__NEW__(void)
{
    HDK_Member__INT__* pMember = (HDK_Member__INT__*)malloc(sizeof(HDK_Member__INT__));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__INT__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__INT__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char cEnd;
    return (sscanf(pszValue, "%d%c", &((HDK_Member__INT__*)pMember)->iValue, &cEnd) == 1);
}

static int HDK_Type__INT__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite,
                                      int fNoWrite, HDK_Member* pMember)
{
    int iValue = ((HDK_Member__INT__*)pMember)->iValue;
    return HDK_Format(pDeviceCtx, pfnWrite, fNoWrite, "%d", iValue);
}

static HDK_Member* HDK_Append_IntHelper(HDK_Struct* pStruct, HDK_Element element, HDK_Type type, int iValue)
{
    HDK_Member__INT__* pMember = (HDK_Member__INT__*)malloc(sizeof(HDK_Member__INT__));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = type;
        pMember->node.pNext = 0;
        pMember->iValue = iValue;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

static HDK_Member* HDK_Set_IntHelper(HDK_Struct* pStruct, HDK_Element element, HDK_Type type, int iValue)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, type);
    if (pMember)
    {
        ((HDK_Member__INT__*)pMember)->iValue = iValue;
        return pMember;
    }
    else
    {
        return HDK_Append_IntHelper(pStruct, element, type, iValue);
    }
}

HDK_Member* HDK_Set_Int(HDK_Struct* pStruct, HDK_Element element, int iValue)
{
    return HDK_Set_IntHelper(pStruct, element, HDK_Type__INT__, iValue);
}

HDK_Member* HDK_Append_Int(HDK_Struct* pStruct, HDK_Element element, int iValue)
{
    return HDK_Append_IntHelper(pStruct, element, HDK_Type__INT__, iValue);
}

int* HDK_Get_Int(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_IntMember(HDK_Get_Member(pStruct, element, HDK_Type__INT__));
}

int HDK_Get_IntEx(HDK_Struct* pStruct, HDK_Element element, int iDefault)
{
    int* piValue = HDK_Get_Int(pStruct, element);
    return (piValue ? *piValue : iDefault);
}

int* HDK_Get_IntMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__INT__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * Bool member type
 */

static HDK_Member* HDK_Type__BOOL__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type__BOOL__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type__BOOL__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    if (strcmp(pszValue, "true") == 0)
    {
        ((HDK_Member__INT__*)pMember)->iValue = 1;
        return 1;
    }
    else if (strcmp(pszValue, "false") == 0)
    {
        ((HDK_Member__INT__*)pMember)->iValue = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

static int HDK_Type__BOOL__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    int fValue = ((HDK_Member__INT__*)pMember)->iValue;
    return HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, fValue ? (char*)"true" : (char*)"false");
}

HDK_Member* HDK_Set_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue)
{
    return HDK_Set_IntHelper(pStruct, element, HDK_Type__BOOL__, fValue);
}

HDK_Member* HDK_Append_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue)
{
    return HDK_Append_IntHelper(pStruct, element, HDK_Type__BOOL__, fValue);
}

int* HDK_Get_Bool(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_BoolMember(HDK_Get_Member(pStruct, element, HDK_Type__BOOL__));
}

int HDK_Get_BoolEx(HDK_Struct* pStruct, HDK_Element element, int fDefault)
{
    int* pfValue = HDK_Get_Bool(pStruct, element);
    return (pfValue ? *pfValue : fDefault);
}

int* HDK_Get_BoolMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__BOOL__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * String member type
 */

typedef struct _HDK_Member__STRING__
{
    HDK_Member node;
    char* pszValue;
} HDK_Member__STRING__;

static HDK_Member* HDK_Type__STRING__NEW__(void)
{
    HDK_Member__STRING__* pMember = (HDK_Member__STRING__*)malloc(sizeof(HDK_Member__STRING__));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__STRING__FREE__(HDK_Member* pMember)
{
    free(((HDK_Member__STRING__*)pMember)->pszValue);
    free(pMember);
}

static int HDK_Type__STRING__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    int cchValue = strlen(pszValue) + 1;
    ((HDK_Member__STRING__*)pMember)->pszValue = (char*)malloc(cchValue);
    if (((HDK_Member__STRING__*)pMember)->pszValue)
    {
        strcpy(((HDK_Member__STRING__*)pMember)->pszValue, pszValue);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int HDK_Type__STRING__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char* pBuf = ((HDK_Member__STRING__*)pMember)->pszValue;
    HDK_WriteBuf_EncodeContext encodeCtx;
    encodeCtx.pDeviceCtx = pDeviceCtx;
    encodeCtx.fNoWrite = fNoWrite;
    return HDK_EncodeString(HDK_WriteBuf_Encode, &encodeCtx, pfnWrite, pBuf, strlen(pBuf));
}

HDK_Member* HDK_Set_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue)
{
    size_t cchValue;
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__STRING__);
    if (pMember)
    {
        /* Free the old string */
        free(((HDK_Member__STRING__*)pMember)->pszValue);

        /* Duplicate the string */
        cchValue = strlen(pszValue);
        ((HDK_Member__STRING__*)pMember)->pszValue = (char*)malloc(cchValue + 1);
        if (((HDK_Member__STRING__*)pMember)->pszValue)
        {
            strcpy(((HDK_Member__STRING__*)pMember)->pszValue, pszValue);
        }

        return pMember;
    }
    else
    {
        return HDK_Append_String(pStruct, element, pszValue);
    }
}

HDK_Member* HDK_Append_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue)
{
    size_t cchValue;
    HDK_Member__STRING__* pMember = (HDK_Member__STRING__*)malloc(sizeof(HDK_Member__STRING__));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__STRING__;
        pMember->node.pNext = 0;

        /* Duplicate the string */
        cchValue = strlen(pszValue);
        pMember->pszValue = (char*)malloc(cchValue + 1);
        if (pMember->pszValue)
        {
            strcpy(pMember->pszValue, pszValue);
            HDK_Append_Member(pStruct, (HDK_Member*)pMember);
        }
        else
        {
            free(pMember);
            pMember = 0;
        }
    }

    return (HDK_Member*)pMember;
}

char* HDK_Get_String(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_StringMember(HDK_Get_Member(pStruct, element, HDK_Type__STRING__));
}

char* HDK_Get_StringEx(HDK_Struct* pStruct, HDK_Element element, char* pszDefault)
{
    char* pszValue = HDK_Get_String(pStruct, element);
    return (pszValue ? pszValue : pszDefault);
}

char* HDK_Get_StringMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__STRING__ ? ((HDK_Member__STRING__*)pMember)->pszValue : 0);
}


/*
 * IPAddress member type
 */

typedef struct _HDK_Member_IPAddress
{
    HDK_Member node;
    HDK_IPAddress ipAddress;
} HDK_Member_IPAddress;

static HDK_Member* HDK_Type__IPADDRESS__NEW__(void)
{
    HDK_Member_IPAddress* pMember = (HDK_Member_IPAddress*)malloc(sizeof(HDK_Member_IPAddress));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__IPADDRESS__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__IPADDRESS__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    HDK_Member_IPAddress* pIPAddress;
    unsigned int a, b, c, d;
    char cEnd;

    /* Blank values are allowed... */
    if (!*pszValue)
    {
        a = b = c = d = 0;
    }
    else if (sscanf(pszValue, "%u.%u.%u.%u%c", &a, &b, &c, &d, &cEnd) != 4 ||
             a > 255 || b > 255 || c > 255 || d > 255)
    {
        return 0;
    }

    /* Add the IPAddress */
    pIPAddress = (HDK_Member_IPAddress*)pMember;
    pIPAddress->ipAddress.a = (unsigned char)a;
    pIPAddress->ipAddress.b = (unsigned char)b;
    pIPAddress->ipAddress.c = (unsigned char)c;
    pIPAddress->ipAddress.d = (unsigned char)d;
    return 1;
}

static int HDK_Type__IPADDRESS__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite,
                                            int fNoWrite, HDK_Member* pMember)
{
    HDK_Member_IPAddress* pIPAddress = (HDK_Member_IPAddress*)pMember;
    unsigned int a = pIPAddress->ipAddress.a;
    unsigned int b = pIPAddress->ipAddress.b;
    unsigned int c = pIPAddress->ipAddress.c;
    unsigned int d = pIPAddress->ipAddress.d;
    return HDK_Format(pDeviceCtx, pfnWrite, fNoWrite, "%u.%u.%u.%u", a, b, c, d);
}

HDK_Member* HDK_Set_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__IPADDRESS__);
    if (pMember)
    {
        ((HDK_Member_IPAddress*)pMember)->ipAddress = *pIPAddress;
        return pMember;
    }
    else
    {
        return HDK_Append_IPAddress(pStruct, element, pIPAddress);
    }
}

HDK_Member* HDK_Append_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress)
{
    HDK_Member_IPAddress* pMember = (HDK_Member_IPAddress*)malloc(sizeof(HDK_Member_IPAddress));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__IPADDRESS__;
        pMember->node.pNext = 0;
        pMember->ipAddress = *pIPAddress;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

HDK_IPAddress* HDK_Get_IPAddress(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_IPAddressMember(HDK_Get_Member(pStruct, element, HDK_Type__IPADDRESS__));
}

HDK_IPAddress* HDK_Get_IPAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pDefault)
{
    HDK_IPAddress* pValue = HDK_Get_IPAddress(pStruct, element);
    return (pValue ? pValue : pDefault);
}

HDK_IPAddress* HDK_Get_IPAddressMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__IPADDRESS__ ? &((HDK_Member_IPAddress*)pMember)->ipAddress : 0);
}


/*
 * MACAddress member type
 */

typedef struct _HDK_Member_MACAddress
{
    HDK_Member node;
    HDK_MACAddress macAddress;
} HDK_Member_MACAddress;

static HDK_Member* HDK_Type__MACADDRESS__NEW__(void)
{
    HDK_Member_MACAddress* pMember = (HDK_Member_MACAddress*)malloc(sizeof(HDK_Member_MACAddress));
    if (pMember)
    {
        memset(pMember, 0, sizeof(*pMember));
    }
    return (HDK_Member*)pMember;
}

static void HDK_Type__MACADDRESS__FREE__(HDK_Member* pMember)
{
    free(pMember);
}

static int HDK_Type__MACADDRESS__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    HDK_Member_MACAddress* pMACAddress;
    unsigned int a, b, c, d, e, f;
    char cEnd;

    /* Blank values are allowed... */
    if (sscanf(pszValue, "%02X:%02X:%02X:%02X:%02X:%02X%c", &a, &b, &c, &d, &e, &f, &cEnd) != 6 ||
             a > 255 || b > 255 || c > 255 || d > 255 || e > 255 || f > 255)
    {
        return 0;
    }

    /* Add the MACAddress */
    pMACAddress = (HDK_Member_MACAddress*)pMember;
    pMACAddress->macAddress.a = (unsigned char)a;
    pMACAddress->macAddress.b = (unsigned char)b;
    pMACAddress->macAddress.c = (unsigned char)c;
    pMACAddress->macAddress.d = (unsigned char)d;
    pMACAddress->macAddress.e = (unsigned char)e;
    pMACAddress->macAddress.f = (unsigned char)f;
    return 1;
}

static int HDK_Type__MACADDRESS__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite,
                                             int fNoWrite, HDK_Member* pMember)
{
    HDK_MACAddress* pMACAddress = &((HDK_Member_MACAddress*)pMember)->macAddress;
    unsigned int a = pMACAddress->a;
    unsigned int b = pMACAddress->b;
    unsigned int c = pMACAddress->c;
    unsigned int d = pMACAddress->d;
    unsigned int e = pMACAddress->e;
    unsigned int f = pMACAddress->f;
    return HDK_Format(pDeviceCtx, pfnWrite, fNoWrite, "%02X:%02X:%02X:%02X:%02X:%02X", a, b, c, d, e, f);
}

HDK_Member* HDK_Set_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress)
{
    HDK_Member* pMember = HDK_Get_Member(pStruct, element, HDK_Type__MACADDRESS__);
    if (pMember)
    {
        ((HDK_Member_MACAddress*)pMember)->macAddress = *pMACAddress;
        return pMember;
    }
    else
    {
        return HDK_Append_MACAddress(pStruct, element, pMACAddress);
    }
}

HDK_Member* HDK_Append_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress)
{
    HDK_Member_MACAddress* pMember = (HDK_Member_MACAddress*)malloc(sizeof(HDK_Member_MACAddress));
    if (pMember)
    {
        pMember->node.element = element;
        pMember->node.type = HDK_Type__MACADDRESS__;
        pMember->node.pNext = 0;
        pMember->macAddress = *pMACAddress;
        HDK_Append_Member(pStruct, (HDK_Member*)pMember);
    }
    return (HDK_Member*)pMember;
}

HDK_MACAddress* HDK_Get_MACAddress(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_MACAddressMember(HDK_Get_Member(pStruct, element, HDK_Type__MACADDRESS__));
}

HDK_MACAddress* HDK_Get_MACAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pDefault)
{
    HDK_MACAddress* pValue = HDK_Get_MACAddress(pStruct, element);
    return (pValue ? pValue : pDefault);
}

HDK_MACAddress* HDK_Get_MACAddressMember(HDK_Member* pMember)
{
    return (pMember && pMember->type == HDK_Type__MACADDRESS__ ? &((HDK_Member_MACAddress*)pMember)->macAddress : 0);
}


/*
 * Enumeration deserialize and serialize helper functions
 */

static int HDK_Type__ENUM__DESERIALIZE__(HDK_Member* pMember, char* pszValue, char** ppszBegin, char** ppszEnd)
{
    char** ppszValue;
    for (ppszValue = ppszBegin; ppszValue != ppszEnd; ppszValue++)
    {
        if (*ppszValue && strcmp(*ppszValue, pszValue) == 0)
        {
            ((HDK_Member__INT__*)pMember)->iValue = ppszValue - ppszBegin;
            return 1;
        }
    }

    ((HDK_Member__INT__*)pMember)->iValue = 0;
    return 1;
}

static int HDK_Type__ENUM__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite,
                                       HDK_Member* pMember, char** ppszBegin, char** ppszEnd)
{
    char** ppszValue = ppszBegin + ((HDK_Member__INT__*)pMember)->iValue;
    if (ppszValue >= ppszBegin && ppszValue < ppszEnd && *ppszValue)
    {
        return HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, *ppszValue);
    }
    return 0;
}


/*
 * HDK_Type__RESULT__ enumeration type
 */

static char* s_HDK_Enum_Result__VALUESTRINGS__[] =
{
    /* HDK_Enum_Result__UNKNOWN__ */ 0,
    /* HDK_Enum_Result_OK */ "OK",
    /* HDK_Enum_Result_REBOOT */ "REBOOT",
    /* HDK_Enum_Result_ERROR */ "ERROR",
    /* HDK_Enum_Result_ERROR_BAD_BEACONINTERVAL */ "ERROR_BAD_BEACONINTERVAL",
    /* HDK_Enum_Result_ERROR_BAD_CHANNEL */ "ERROR_BAD_CHANNEL",
    /* HDK_Enum_Result_ERROR_BAD_CHANNEL_WIDTH */ "ERROR_BAD_CHANNEL_WIDTH",
    /* HDK_Enum_Result_ERROR_BAD_DTIMINTERVAL */ "ERROR_BAD_DTIMINTERVAL",
    /* HDK_Enum_Result_ERROR_BAD_GUARDINTERVAL */ "ERROR_BAD_GUARDINTERVAL",
    /* HDK_Enum_Result_ERROR_BAD_MODE */ "ERROR_BAD_MODE",
    /* HDK_Enum_Result_ERROR_BAD_RADIOID */ "ERROR_BAD_RADIOID",
    /* HDK_Enum_Result_ERROR_BAD_SECONDARY_CHANNEL */ "ERROR_BAD_SECONDARY_CHANNEL",
    /* HDK_Enum_Result_ERROR_BAD_SSID */ "ERROR_BAD_SSID",
    /* HDK_Enum_Result_ERROR_BAD_SSID_ID */ "ERROR_BAD_SSID_ID",
    /* HDK_Enum_Result_ERROR_BAD_VLANID */ "ERROR_BAD_VLANID",
    /* HDK_Enum_Result_ERROR_ENCRYPTION */ "ERROR_ENCRYPTION",
    /* HDK_Enum_Result_ERROR_QOS */ "ERROR_QOS",
    /* HDK_Enum_Result_ERROR_RadioUnsupported */ "ERROR_RadioUnsupported",
};

static HDK_Member* HDK_Type__RESULT__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type__RESULT__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type__RESULT__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Result__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Result__VALUESTRINGS__ + sizeof(s_HDK_Enum_Result__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Result__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type__RESULT__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Result__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Result__VALUESTRINGS__ + sizeof(s_HDK_Enum_Result__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Result__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    if (eValue != HDK_Enum_Result__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type__RESULT__, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    if (eValue != HDK_Enum_Result__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type__RESULT__, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Result* HDK_Get_Result(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_ResultMember(HDK_Get_Member(pStruct, element, HDK_Type__RESULT__));
}

HDK_Enum_Result HDK_Get_ResultEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue)
{
    HDK_Enum_Result* peDefault = HDK_Get_Result(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Result* HDK_Get_ResultMember(HDK_Member* pMember)
{
    return (HDK_Enum_Result*)(pMember && pMember->type == HDK_Type__RESULT__ ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}

const char* HDK_Enum_ResultToString(HDK_Enum_Result result)
{
    if (sizeof(s_HDK_Enum_Result__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Result__VALUESTRINGS__) > (size_t)result)
    {
        return s_HDK_Enum_Result__VALUESTRINGS__[result];
    }
    return 0;
}


/*
 * HDK_Type_Cisco_DeviceInf enumeration type
 */

static char* s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__[] =
{
    /* HDK_Enum_Cisco_DeviceInf__UNKNOWN__ */ 0,
    /* HDK_Enum_Cisco_DeviceInf_WiFi_2_4G */ "WiFi_2.4G",
    /* HDK_Enum_Cisco_DeviceInf_WiFi_5_0G */ "WiFi_5.0G",
    /* HDK_Enum_Cisco_DeviceInf_Eth */ "Eth",
    /* HDK_Enum_Cisco_DeviceInf_ */ "",
};

static HDK_Member* HDK_Type_Cisco_DeviceInf__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_Cisco_DeviceInf__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_Cisco_DeviceInf__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_Cisco_DeviceInf__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_DeviceInf__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eValue)
{
    if (eValue != HDK_Enum_Cisco_DeviceInf__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_Cisco_DeviceInf, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eValue)
{
    if (eValue != HDK_Enum_Cisco_DeviceInf__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_Cisco_DeviceInf, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Cisco_DeviceInf* HDK_Get_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_Cisco_DeviceInfMember(HDK_Get_Member(pStruct, element, HDK_Type_Cisco_DeviceInf));
}

HDK_Enum_Cisco_DeviceInf HDK_Get_Cisco_DeviceInfEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eValue)
{
    HDK_Enum_Cisco_DeviceInf* peDefault = HDK_Get_Cisco_DeviceInf(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Cisco_DeviceInf* HDK_Get_Cisco_DeviceInfMember(HDK_Member* pMember)
{
    return (HDK_Enum_Cisco_DeviceInf*)(pMember && pMember->type == HDK_Type_Cisco_DeviceInf ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_Cisco_WiFiEncryption enumeration type
 */

static char* s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__[] =
{
    /* HDK_Enum_Cisco_WiFiEncryption__UNKNOWN__ */ 0,
    /* HDK_Enum_Cisco_WiFiEncryption_WEP_64 */ "WEP-64",
    /* HDK_Enum_Cisco_WiFiEncryption_WEP_128 */ "WEP-128",
    /* HDK_Enum_Cisco_WiFiEncryption_AES */ "AES",
    /* HDK_Enum_Cisco_WiFiEncryption_TKIP */ "TKIP",
    /* HDK_Enum_Cisco_WiFiEncryption_TKIPORAES */ "TKIPORAES",
    /* HDK_Enum_Cisco_WiFiEncryption_ */ "",
};

static HDK_Member* HDK_Type_Cisco_WiFiEncryption__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_Cisco_WiFiEncryption__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_Cisco_WiFiEncryption__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_Cisco_WiFiEncryption__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Cisco_WiFiEncryption* HDK_Get_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_Cisco_WiFiEncryptionMember(HDK_Get_Member(pStruct, element, HDK_Type_Cisco_WiFiEncryption));
}

HDK_Enum_Cisco_WiFiEncryption HDK_Get_Cisco_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eValue)
{
    HDK_Enum_Cisco_WiFiEncryption* peDefault = HDK_Get_Cisco_WiFiEncryption(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Cisco_WiFiEncryption* HDK_Get_Cisco_WiFiEncryptionMember(HDK_Member* pMember)
{
    return (HDK_Enum_Cisco_WiFiEncryption*)(pMember && pMember->type == HDK_Type_Cisco_WiFiEncryption ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_Cisco_WiFiMode enumeration type
 */

static char* s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__[] =
{
    /* HDK_Enum_Cisco_WiFiMode__UNKNOWN__ */ 0,
    /* HDK_Enum_Cisco_WiFiMode_802_11a */ "802.11a",
    /* HDK_Enum_Cisco_WiFiMode_802_11b */ "802.11b",
    /* HDK_Enum_Cisco_WiFiMode_802_11g */ "802.11g",
    /* HDK_Enum_Cisco_WiFiMode_802_11n */ "802.11n",
    /* HDK_Enum_Cisco_WiFiMode_802_11bg */ "802.11bg",
    /* HDK_Enum_Cisco_WiFiMode_802_11bn */ "802.11bn",
    /* HDK_Enum_Cisco_WiFiMode_802_11bgn */ "802.11bgn",
    /* HDK_Enum_Cisco_WiFiMode_802_11gn */ "802.11gn",
    /* HDK_Enum_Cisco_WiFiMode_802_11an */ "802.11an",
    /* HDK_Enum_Cisco_WiFiMode_802_11bgnac */ "802.11bgnac",
    /* HDK_Enum_Cisco_WiFiMode_802_11gnac */ "802.11gnac",
    /* HDK_Enum_Cisco_WiFiMode_802_11nac */ "802.11nac",
    /* HDK_Enum_Cisco_WiFiMode_802_11ac */ "802.11ac",
    /* HDK_Enum_Cisco_WiFiMode_802_11anac */ "802.11anac",
    /* HDK_Enum_Cisco_WiFiMode_802_11aac */ "802.11aac",
    /* HDK_Enum_Cisco_WiFiMode_ */ "",
};

static HDK_Member* HDK_Type_Cisco_WiFiMode__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_Cisco_WiFiMode__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_Cisco_WiFiMode__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_Cisco_WiFiMode__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiMode__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiMode__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Cisco_WiFiMode* HDK_Get_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_Cisco_WiFiModeMember(HDK_Get_Member(pStruct, element, HDK_Type_Cisco_WiFiMode));
}

HDK_Enum_Cisco_WiFiMode HDK_Get_Cisco_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eValue)
{
    HDK_Enum_Cisco_WiFiMode* peDefault = HDK_Get_Cisco_WiFiMode(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Cisco_WiFiMode* HDK_Get_Cisco_WiFiModeMember(HDK_Member* pMember)
{
    return (HDK_Enum_Cisco_WiFiMode*)(pMember && pMember->type == HDK_Type_Cisco_WiFiMode ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_Cisco_WiFiSecurity enumeration type
 */

static char* s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__[] =
{
    /* HDK_Enum_Cisco_WiFiSecurity__UNKNOWN__ */ 0,
    /* HDK_Enum_Cisco_WiFiSecurity_None */ "None",
    /* HDK_Enum_Cisco_WiFiSecurity_WEP_64 */ "WEP-64",
    /* HDK_Enum_Cisco_WiFiSecurity_WEP_128 */ "WEP-128",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA_Personal */ "WPA-Personal",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA2_Personal */ "WPA2-Personal",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Personal */ "WPA-WPA2-Personal",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA_Enterprise */ "WPA-Enterprise",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA2_Enterprise */ "WPA2-Enterprise",
    /* HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Enterprise */ "WPA-WPA2-Enterprise",
    /* HDK_Enum_Cisco_WiFiSecurity_ */ "",
};

static HDK_Member* HDK_Type_Cisco_WiFiSecurity__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_Cisco_WiFiSecurity__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_Cisco_WiFiSecurity__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_Cisco_WiFiSecurity__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_Cisco_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_Cisco_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_Cisco_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_Cisco_WiFiSecurity* HDK_Get_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_Cisco_WiFiSecurityMember(HDK_Get_Member(pStruct, element, HDK_Type_Cisco_WiFiSecurity));
}

HDK_Enum_Cisco_WiFiSecurity HDK_Get_Cisco_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eValue)
{
    HDK_Enum_Cisco_WiFiSecurity* peDefault = HDK_Get_Cisco_WiFiSecurity(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_Cisco_WiFiSecurity* HDK_Get_Cisco_WiFiSecurityMember(HDK_Member* pMember)
{
    return (HDK_Enum_Cisco_WiFiSecurity*)(pMember && pMember->type == HDK_Type_Cisco_WiFiSecurity ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_DeviceType enumeration type
 */

static char* s_HDK_Enum_PN_DeviceType__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_DeviceType__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_DeviceType_Computer */ "Computer",
    /* HDK_Enum_PN_DeviceType_ComputerServer */ "ComputerServer",
    /* HDK_Enum_PN_DeviceType_WorkstationComputer */ "WorkstationComputer",
    /* HDK_Enum_PN_DeviceType_LaptopComputer */ "LaptopComputer",
    /* HDK_Enum_PN_DeviceType_Gateway */ "Gateway",
    /* HDK_Enum_PN_DeviceType_GatewayWithWiFi */ "GatewayWithWiFi",
    /* HDK_Enum_PN_DeviceType_DigitalDVR */ "DigitalDVR",
    /* HDK_Enum_PN_DeviceType_DigitalJukebox */ "DigitalJukebox",
    /* HDK_Enum_PN_DeviceType_MediaAdapter */ "MediaAdapter",
    /* HDK_Enum_PN_DeviceType_NetworkCamera */ "NetworkCamera",
    /* HDK_Enum_PN_DeviceType_NetworkDevice */ "NetworkDevice",
    /* HDK_Enum_PN_DeviceType_NetworkDrive */ "NetworkDrive",
    /* HDK_Enum_PN_DeviceType_NetworkGameConsole */ "NetworkGameConsole",
    /* HDK_Enum_PN_DeviceType_NetworkPDA */ "NetworkPDA",
    /* HDK_Enum_PN_DeviceType_NetworkPrinter */ "NetworkPrinter",
    /* HDK_Enum_PN_DeviceType_NetworkPrintServer */ "NetworkPrintServer",
    /* HDK_Enum_PN_DeviceType_PhotoFrame */ "PhotoFrame",
    /* HDK_Enum_PN_DeviceType_VOIPDevice */ "VOIPDevice",
    /* HDK_Enum_PN_DeviceType_WiFiAccessPoint */ "WiFiAccessPoint",
    /* HDK_Enum_PN_DeviceType_SetTopBox */ "SetTopBox",
    /* HDK_Enum_PN_DeviceType_WiFiBridge */ "WiFiBridge",
    /* HDK_Enum_PN_DeviceType_WiFiExtender */ "WiFiExtender",
};

static HDK_Member* HDK_Type_PN_DeviceType__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_DeviceType__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_DeviceType__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_DeviceType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_DeviceType__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_DeviceType__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_DeviceType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_DeviceType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_DeviceType__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    if (eValue != HDK_Enum_PN_DeviceType__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_DeviceType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    if (eValue != HDK_Enum_PN_DeviceType__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_DeviceType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_DeviceTypeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_DeviceType));
}

HDK_Enum_PN_DeviceType HDK_Get_PN_DeviceTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue)
{
    HDK_Enum_PN_DeviceType* peDefault = HDK_Get_PN_DeviceType(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceTypeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_DeviceType*)(pMember && pMember->type == HDK_Type_PN_DeviceType ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_TaskExtType enumeration type
 */

static char* s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_TaskExtType__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_TaskExtType_Browser */ "Browser",
    /* HDK_Enum_PN_TaskExtType_MessageBox */ "MessageBox",
    /* HDK_Enum_PN_TaskExtType_PUI */ "PUI",
    /* HDK_Enum_PN_TaskExtType_Silent */ "Silent",
};

static HDK_Member* HDK_Type_PN_TaskExtType__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_TaskExtType__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_TaskExtType__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_TaskExtType__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_TaskExtType__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    if (eValue != HDK_Enum_PN_TaskExtType__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_TaskExtType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    if (eValue != HDK_Enum_PN_TaskExtType__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_TaskExtType, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_TaskExtTypeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_TaskExtType));
}

HDK_Enum_PN_TaskExtType HDK_Get_PN_TaskExtTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue)
{
    HDK_Enum_PN_TaskExtType* peDefault = HDK_Get_PN_TaskExtType(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtTypeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_TaskExtType*)(pMember && pMember->type == HDK_Type_PN_TaskExtType ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiEncryption enumeration type
 */

static char* s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiEncryption__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiEncryption_WEP_64 */ "WEP-64",
    /* HDK_Enum_PN_WiFiEncryption_WEP_128 */ "WEP-128",
    /* HDK_Enum_PN_WiFiEncryption_AES */ "AES",
    /* HDK_Enum_PN_WiFiEncryption_TKIP */ "TKIP",
    /* HDK_Enum_PN_WiFiEncryption_TKIPORAES */ "TKIPORAES",
    /* HDK_Enum_PN_WiFiEncryption_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiEncryption__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiEncryption__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiEncryption__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiEncryption__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiEncryption__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_PN_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    if (eValue != HDK_Enum_PN_WiFiEncryption__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiEncryption, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiEncryptionMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiEncryption));
}

HDK_Enum_PN_WiFiEncryption HDK_Get_PN_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue)
{
    HDK_Enum_PN_WiFiEncryption* peDefault = HDK_Get_PN_WiFiEncryption(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryptionMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiEncryption*)(pMember && pMember->type == HDK_Type_PN_WiFiEncryption ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiMode enumeration type
 */

static char* s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiMode__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiMode_802_11a */ "802.11a",
    /* HDK_Enum_PN_WiFiMode_802_11b */ "802.11b",
    /* HDK_Enum_PN_WiFiMode_802_11g */ "802.11g",
    /* HDK_Enum_PN_WiFiMode_802_11n */ "802.11n",
    /* HDK_Enum_PN_WiFiMode_802_11bg */ "802.11bg",
    /* HDK_Enum_PN_WiFiMode_802_11bn */ "802.11bn",
    /* HDK_Enum_PN_WiFiMode_802_11bgn */ "802.11bgn",
    /* HDK_Enum_PN_WiFiMode_802_11gn */ "802.11gn",
    /* HDK_Enum_PN_WiFiMode_802_11an */ "802.11an",
	/* HDK_Enum_PN_WiFiMode_802_11bgnac */ "802.11bgnac",
    /* HDK_Enum_PN_WiFiMode_802_11gnac */ "802.11gnac",
    /* HDK_Enum_PN_WiFiMode_802_11nac */ "802.11nac",
    /* HDK_Enum_PN_WiFiMode_802_11ac */ "802.11ac",
    /* HDK_Enum_PN_WiFiMode_802_11anac */ "802.11anac",
    /* HDK_Enum_PN_WiFiMode_802_11aac */ "802.11aac",
    /* HDK_Enum_PN_WiFiMode_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiMode__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiMode__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiMode__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiMode__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiMode__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    if (eValue != HDK_Enum_PN_WiFiMode__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    if (eValue != HDK_Enum_PN_WiFiMode__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiMode, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiModeMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiMode));
}

HDK_Enum_PN_WiFiMode HDK_Get_PN_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue)
{
    HDK_Enum_PN_WiFiMode* peDefault = HDK_Get_PN_WiFiMode(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiModeMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiMode*)(pMember && pMember->type == HDK_Type_PN_WiFiMode ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HDK_Type_PN_WiFiSecurity enumeration type
 */

static char* s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__[] =
{
    /* HDK_Enum_PN_WiFiSecurity__UNKNOWN__ */ 0,
    /* HDK_Enum_PN_WiFiSecurity_NONE */ "NONE",
    /* HDK_Enum_PN_WiFiSecurity_WEP_64 */ "WEP-64",
    /* HDK_Enum_PN_WiFiSecurity_WEP_128 */ "WEP-128",
    /* HDK_Enum_PN_WiFiSecurity_WPA_Personal */ "WPA-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA2_Personal */ "WPA2-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal */ "WPA-WPA2-Personal",
    /* HDK_Enum_PN_WiFiSecurity_WPA_Enterprise */ "WPA-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise */ "WPA2-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise */ "WPA-WPA2-Enterprise",
    /* HDK_Enum_PN_WiFiSecurity_ */ "",
};

static HDK_Member* HDK_Type_PN_WiFiSecurity__NEW__(void)
{
    return HDK_Type__INT__NEW__();
}

static void HDK_Type_PN_WiFiSecurity__FREE__(HDK_Member* pMember)
{
    HDK_Type__INT__FREE__(pMember);
}

static int HDK_Type_PN_WiFiSecurity__DESERIALIZE__(HDK_Member* pMember, char* pszValue)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__DESERIALIZE__(pMember, pszValue, ppszBegin, ppszEnd);
}

static int HDK_Type_PN_WiFiSecurity__SERIALIZE__(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Member* pMember)
{
    char** ppszBegin = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__;
    char** ppszEnd = s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__ + sizeof(s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__) / sizeof(*s_HDK_Enum_PN_WiFiSecurity__VALUESTRINGS__);
    return HDK_Type__ENUM__SERIALIZE__(pDeviceCtx, pfnWrite, fNoWrite, pMember, ppszBegin, ppszEnd);
}

HDK_Member* HDK_Set_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_PN_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Set_IntHelper(pStruct, element, HDK_Type_PN_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Member* HDK_Append_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    if (eValue != HDK_Enum_PN_WiFiSecurity__UNKNOWN__)
    {
        return HDK_Append_IntHelper(pStruct, element, HDK_Type_PN_WiFiSecurity, eValue);
    }
    else
    {
        return 0;
    }
}

HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element)
{
    return HDK_Get_PN_WiFiSecurityMember(HDK_Get_Member(pStruct, element, HDK_Type_PN_WiFiSecurity));
}

HDK_Enum_PN_WiFiSecurity HDK_Get_PN_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue)
{
    HDK_Enum_PN_WiFiSecurity* peDefault = HDK_Get_PN_WiFiSecurity(pStruct, element);
    return (peDefault ? *peDefault : eValue);
}

HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurityMember(HDK_Member* pMember)
{
    return (HDK_Enum_PN_WiFiSecurity*)(pMember && pMember->type == HDK_Type_PN_WiFiSecurity ? &((HDK_Member__INT__*)pMember)->iValue : 0);
}


/*
 * HNAP type info table
 */

static HDK_TypeInfo s_types[] =
{
    { /* HDK_Type__UNKNOWN__ */ 0, 0, 0, 0 },
    { /* HDK_Type__UNKNOWN_ANY__ */ 0, 0, 0, 0 },
    { /* HDK_Type__STRUCT__ */ 0, 0, 0, 0 },
    { /* HDK_Type__BLANK__ */ 0, HDK_Type__BLANK__FREE__, 0, 0 },
    { /* HDK_Type__IPADDRESS__ */ HDK_Type__IPADDRESS__NEW__, HDK_Type__IPADDRESS__FREE__, HDK_Type__IPADDRESS__SERIALIZE__, HDK_Type__IPADDRESS__DESERIALIZE__ },
    { /* HDK_Type__MACADDRESS__ */ HDK_Type__MACADDRESS__NEW__, HDK_Type__MACADDRESS__FREE__, HDK_Type__MACADDRESS__SERIALIZE__, HDK_Type__MACADDRESS__DESERIALIZE__ },
    { /* HDK_Type__RESULT__ */ HDK_Type__RESULT__NEW__, HDK_Type__RESULT__FREE__, HDK_Type__RESULT__SERIALIZE__, HDK_Type__RESULT__DESERIALIZE__ },
    { /* HDK_Type_Cisco_DeviceInf */ HDK_Type_Cisco_DeviceInf__NEW__, HDK_Type_Cisco_DeviceInf__FREE__, HDK_Type_Cisco_DeviceInf__SERIALIZE__, HDK_Type_Cisco_DeviceInf__DESERIALIZE__ },
    { /* HDK_Type_Cisco_WiFiEncryption */ HDK_Type_Cisco_WiFiEncryption__NEW__, HDK_Type_Cisco_WiFiEncryption__FREE__, HDK_Type_Cisco_WiFiEncryption__SERIALIZE__, HDK_Type_Cisco_WiFiEncryption__DESERIALIZE__ },
    { /* HDK_Type_Cisco_WiFiMode */ HDK_Type_Cisco_WiFiMode__NEW__, HDK_Type_Cisco_WiFiMode__FREE__, HDK_Type_Cisco_WiFiMode__SERIALIZE__, HDK_Type_Cisco_WiFiMode__DESERIALIZE__ },
    { /* HDK_Type_Cisco_WiFiSecurity */ HDK_Type_Cisco_WiFiSecurity__NEW__, HDK_Type_Cisco_WiFiSecurity__FREE__, HDK_Type_Cisco_WiFiSecurity__SERIALIZE__, HDK_Type_Cisco_WiFiSecurity__DESERIALIZE__ },
    { /* HDK_Type_PN_DeviceType */ HDK_Type_PN_DeviceType__NEW__, HDK_Type_PN_DeviceType__FREE__, HDK_Type_PN_DeviceType__SERIALIZE__, HDK_Type_PN_DeviceType__DESERIALIZE__ },
    { /* HDK_Type_PN_TaskExtType */ HDK_Type_PN_TaskExtType__NEW__, HDK_Type_PN_TaskExtType__FREE__, HDK_Type_PN_TaskExtType__SERIALIZE__, HDK_Type_PN_TaskExtType__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiEncryption */ HDK_Type_PN_WiFiEncryption__NEW__, HDK_Type_PN_WiFiEncryption__FREE__, HDK_Type_PN_WiFiEncryption__SERIALIZE__, HDK_Type_PN_WiFiEncryption__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiMode */ HDK_Type_PN_WiFiMode__NEW__, HDK_Type_PN_WiFiMode__FREE__, HDK_Type_PN_WiFiMode__SERIALIZE__, HDK_Type_PN_WiFiMode__DESERIALIZE__ },
    { /* HDK_Type_PN_WiFiSecurity */ HDK_Type_PN_WiFiSecurity__NEW__, HDK_Type_PN_WiFiSecurity__FREE__, HDK_Type_PN_WiFiSecurity__SERIALIZE__, HDK_Type_PN_WiFiSecurity__DESERIALIZE__ },
    { /* HDK_Type__BOOL__ */ HDK_Type__BOOL__NEW__, HDK_Type__BOOL__FREE__, HDK_Type__BOOL__SERIALIZE__, HDK_Type__BOOL__DESERIALIZE__ },
    { /* HDK_Type__INT__ */ HDK_Type__INT__NEW__, HDK_Type__INT__FREE__, HDK_Type__INT__SERIALIZE__, HDK_Type__INT__DESERIALIZE__ },
    { /* HDK_Type__STRING__ */ HDK_Type__STRING__NEW__, HDK_Type__STRING__FREE__, HDK_Type__STRING__SERIALIZE__, HDK_Type__STRING__DESERIALIZE__ },
};

static HDK_TypeInfo* s_types_GetInfo(HDK_Type type)
{
    return &s_types[type];
}


/*
 * Generic member node utilities
 */

/* Helper function for HDK_Serialize */
static int HDK_Serialize_Helper(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite,
                                HDK_Member* pMember, HDK_Struct* pInputStruct,
                                unsigned int ixTreeMember, int fResponse)
{
    int iContentLength = 0;

    /* Serialize the element open */
    HDK_ElementTreeNode* pMemberNode = s_elementTree_GetNode(ixTreeMember);
    HDK_ElementNode* pMemberElement = s_elements_GetNode(pMember->element);
    HDK_ElementTreeNode* pParentNode = s_elementTree_GetNode(pMemberNode->ixParent);
    HDK_ElementNode* pParentElement = s_elements_GetNode(pParentNode->element);
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, "<");
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, pMemberElement->pszElement);
    if (pParentElement->ixNamespace != pMemberElement->ixNamespace)
    {
        iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, " xmlns=\"");
        iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, s_namespaces_GetString(pMemberElement->ixNamespace));
        iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, "\"");
    }
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, ">");

    /* Serialize the element value */
    if (pMember->type == HDK_Type__STRUCT__)
    {
        unsigned int ixChildBegin;
        unsigned int ixChildEnd;

        /* Output a newline after the struct open tag */
        iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, "\n");

        /* Get the struct's child tree nodes */
        if (s_elementTree_GetChildNodes(ixTreeMember, &ixChildBegin, &ixChildEnd))
        {
            /* The result element guaranteed to be the first schema child of a response */
            HDK_Member* pChildBegin = ((HDK_Struct*)pMember)->pHead;
            int fIsResult = (fResponse && pChildBegin && pChildBegin->type == HDK_Type__RESULT__);

            /* Iterate the child tree nodes */
            unsigned int ixChild;
            for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
            {
                /* Output the matching members */
                HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);
                HDK_Member* pChild;
                for (pChild = pChildBegin; pChild; pChild = pChild->pNext)
                {
                    if (pChild->element == (HDK_Element)pChildNode->element)
                    {
                        iContentLength += HDK_Serialize_Helper(pDeviceCtx, pfnWrite, fNoWrite, pChild, pInputStruct, ixChild, 0);

                        /* Update child begin - we won't match this member again */
                        if (pChild == pChildBegin)
                        {
                            pChildBegin = pChild->pNext;
                        }

                        /* Stop if not unbounded */
                        if (!(pChildNode->prop & HDK_ElementTreeProp_Unbounded))
                        {
                            break;
                        }
                    }
                }

                /* Stop if the result is an error */
                if (fIsResult && pChild)
                {
                    HDK_Enum_Result* pResult = HDK_Get_ResultMember(pChild);
                    if (pResult && HDK_FAILED(*pResult))
                    {
                        break;
                    }
                }
                fIsResult = 0;
            }
        }
    }
    else
    {
        HDK_TypeInfo* pTypeInfo = s_types_GetInfo(pMember->type);
        if (pTypeInfo && pTypeInfo->pfnSerialize)
        {
            iContentLength += pTypeInfo->pfnSerialize(pDeviceCtx, pfnWrite, fNoWrite, pMember);
        }
    }

    /* Serialize the element close */
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, "</");
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, pMemberElement->pszElement);
    iContentLength += HDK_Write(pDeviceCtx, pfnWrite, fNoWrite, ">\n");

    return iContentLength;
}

/* Serialize an HNAP struct */
int HDK_Serialize(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, HDK_Struct* pStruct, HDK_Struct* pInputStruct)
{
    /* Find the top level element in the XML element tree */
    unsigned int ixTreeStruct;
    if (!s_elementTree_GetChildNode(2, pStruct->node.element, &ixTreeStruct))
    {
        return 0;
    }

    return HDK_Serialize_Helper(pDeviceCtx, pfnWrite, fNoWrite, (HDK_Member*)pStruct, pInputStruct,
                                ixTreeStruct, !!pInputStruct);
}


/*
 * HNAP de-serialization
 */

/* Initialize XML parsing context */
void HDK_Parse_Init(HDK_ParseContext* pParseCtx, HDK_Struct* pStruct, void* pDecodeCtx)
{
    memset(pParseCtx, 0, sizeof(*pParseCtx));
    pParseCtx->pInputStack[0] = pStruct;
    pParseCtx->cbValueBuf = 128;
    pParseCtx->pDecodeCtx = pDecodeCtx;
}

/* Free XML parsing context */
void HDK_Parse_Free(HDK_ParseContext* pParseCtx)
{
    if (pParseCtx->pszValue)
    {
        free(pParseCtx->pszValue);
    }
}

/* Handle an XML element open */
void HDK_Parse_ElementOpen(HDK_ParseContext* pParseCtx, char* pszNamespace, char* pszNamespaceEnd,
                           char* pszElement, char* pszElementEnd)
{
    HDK_Element element;
    HDK_ElementTreeNode* pTreeNode;
    unsigned int ixElementOpen = 0;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Keep count of elements */
#ifndef HDK_MAX_ELEMENTS
#define HDK_MAX_ELEMENTS 1024
#endif
    if (++pParseCtx->cElements > HDK_MAX_ELEMENTS)
    {
        pParseCtx->parseError = HDK_ParseError_500_XMLInvalidRequest;

        log_printf(LOG_ERR, "Exceeded maximum element count %s\n", HDK_MAX_ELEMENTS);
        return;
    }

    /* Search for the element enum */
    element = s_elements_FindElement(pszNamespace, pszNamespaceEnd, pszElement, pszElementEnd);
    if (element == HDK_Element__UNKNOWN__)
    {
        /* Unknown element - is any element allowed here? */
        HDK_ElementTreeNode* pTreeNodeParent = s_elementTree_GetNode(pParseCtx->ixElement);

//#ifdef HDK_LOGGING
        /* Allocate a '\0' terminated string on the stack. */
        size_t cchElement = (pszElementEnd) ? (size_t)(pszElementEnd - pszElement) : ((pszElement) ? strlen(pszElement) : 0);
        char* pszElementNT = (char*)malloc((cchElement + 1) * sizeof(char));
		if (pszElementNT != NULL)
		{
			memcpy(pszElementNT, pszElement, cchElement * sizeof(char));
			pszElementNT[cchElement] = '\0';
		}
//#endif /* def HDK_LOGGING */

        if (pTreeNodeParent->type == HDK_Type__UNKNOWN_ANY__)
        {
            pParseCtx->cAnyElement++;

            /*log_printf(LOG_WARNING,"Allowing unknown element '%s' as child of element '%s'\n", pszElementNT, 
					   s_elements_GetNode(pTreeNodeParent->element)->pszElement);*/
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLUnknownElement;

			if (pszElementNT != NULL)
			{
				log_printf(LOG_ERR, "Unknown element '%s'\n", pszElementNT);
				free(pszElementNT);
			}
        }
        return;
    }

    /* Search for the element immediately under the current node */
    pTreeNode = s_elementTree_GetChildNode(pParseCtx->ixElement, element, &ixElementOpen);
    if (!pTreeNode)
    {
        /* Unknown element - is any element allowed here? */
        HDK_ElementTreeNode* pTreeNodeParent = s_elementTree_GetNode(pParseCtx->ixElement);
        if (pTreeNodeParent->type == HDK_Type__UNKNOWN_ANY__)
        {
            pParseCtx->cAnyElement++;

            log_printf(LOG_WARNING,"Allowing unexpected HNAP element '%s' as child of element '%s'\n", s_elements_GetNode(element)->pszElement, s_elements_GetNode(pTreeNodeParent->element)->pszElement);
            return;
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLUnexpectedElement;

            log_printf(LOG_ERR, "Unexpected HNAP element '%s' as child of element '%s'\n", s_elements_GetNode(element)->pszElement, s_elements_GetNode(pTreeNodeParent->element)->pszElement);
            return;
        }
    }

    /* Create a new struct, if necessary */
    if (pTreeNode->type == HDK_Type__STRUCT__)
    {
        /* If we're at the top... */
        if (pParseCtx->ixStack == 0)
        {
            HDK_Struct* pStructCur = pParseCtx->pInputStack[0];

            /* Make sure we don't get multiple input structs */
            if (pParseCtx->fHaveInput)
            {
                pParseCtx->parseError = HDK_ParseError_500_XMLInvalid;

                log_printf(LOG_ERR, "Encountered second root HNAP element '%s'\n",  s_elements_GetNode(element)->pszElement);
                return;
            }

            pStructCur->node.element = element;

            /* Increment the the input stack index - top is one less */
            pParseCtx->ixStack++;
            pParseCtx->fHaveInput = 1;
        }
        else if (pParseCtx->ixStack >= sizeof(pParseCtx->pInputStack) / sizeof(*pParseCtx->pInputStack))
        {
            /* Too deep... */
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
            return;
        }
        else
        {
            /* Create the new struct */
            HDK_Struct* pStructCur = pParseCtx->pInputStack[pParseCtx->ixStack - 1];
            HDK_Struct* pStructNew = HDK_Append_Struct(pStructCur, element);
            if (!pStructNew)
            {
                pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
                return;
            }
            pParseCtx->pInputStack[pParseCtx->ixStack++] = pStructNew;
        }
    }

    /* Update the current element index */
    pParseCtx->ixElement = (unsigned short)ixElementOpen;
    if (pParseCtx->pszValue)
    {
        pParseCtx->pszValue[0] = '\0';
        pParseCtx->cbValue = 0;
    }
#ifdef HDK_LOGGING
    {
        size_t cchIndent = 4 * (pParseCtx->ixStack + ((HDK_Type__UNKNOWN__ != pTreeNode->type) &&
                                                      (HDK_Type__UNKNOWN_ANY__ != pTreeNode->type) &&
                                                      (HDK_Type__STRUCT__ != pTreeNode->type)));
        char* pszIndent = (char*)alloca((cchIndent + 1) * sizeof(char));
        memset(pszIndent, ' ', cchIndent);
        pszIndent[cchIndent] = '\0';
        log_printf(LOG_INFO,"%s<%s>\n", pszIndent, s_elements_GetNode(element)->pszElement);
    }
#endif /* def HDK_LOGGING */
}

/* Handle an XML element close */
void HDK_Parse_ElementClose(HDK_ParseContext* pParseCtx)
{
    HDK_ElementTreeNode* pTreeNode;
    HDK_TypeInfo* pTypeInfo;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Deserialize the type */
    pTreeNode = s_elementTree_GetNode(pParseCtx->ixElement);
    pTypeInfo = s_types_GetInfo((HDK_Type)pTreeNode->type);
    if (pTypeInfo && pTypeInfo->pfnNew && pTypeInfo->pfnDeserialize)
    {
        HDK_Member* pMember = pTypeInfo->pfnNew();
        if (pMember)
        {
            pMember->element = (HDK_Element)pTreeNode->element;
            pMember->type = (HDK_Type)pTreeNode->type;
            pMember->pNext = 0;
            if (pTypeInfo->pfnDeserialize(pMember, (pParseCtx->pszValue ? pParseCtx->pszValue : (char*)"")))
            {
                HDK_Append_Member(pParseCtx->pInputStack[pParseCtx->ixStack - 1], pMember);
            }
            else
            {
               log_printf(LOG_ERR, "Failed to deserialize value '%s' from element '%s'\n", pParseCtx->pszValue, s_elements_GetNode(pMember->element)->pszElement);
                HDK_FreeMember(pMember, 0);
                pParseCtx->parseError = (HDK_ParseError_500_XMLInvalidValue);

               //log_printf(LOG_ERR, "Failed to deserialize value '%s' from element '%s'\n", pParseCtx->pszValue, s_elements_GetNode(pMember->element)->pszElement);
            }
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
        }
    }

    /* Update the input stack index, if necessary */
    if (pTreeNode->type == HDK_Type__STRUCT__)
    {
        /* Too shallow? */
        if (pParseCtx->ixStack < 1)
        {
            pParseCtx->parseError = HDK_ParseError_500_XMLInvalid;
            return;
        }

        pParseCtx->ixStack--;
    }

    /* Update the current element index */
    if (pTreeNode->type == HDK_Type__UNKNOWN_ANY__ && pParseCtx->cAnyElement)
    {
        pParseCtx->cAnyElement--;
    }
    else
    {
        pParseCtx->ixElement = (unsigned short)pTreeNode->ixParent;
    }

#ifdef HDK_LOGGING
    {
        char* pszIndent = 0;
        size_t cchIndent = 4 * (pParseCtx->ixStack + ((HDK_Type__UNKNOWN__ != pTreeNode->type) &&
                                                      (HDK_Type__UNKNOWN_ANY__ != pTreeNode->type)));

        pszIndent = (char*)alloca((cchIndent + 1) * sizeof(char));
        memset(pszIndent, ' ', cchIndent);
        pszIndent[cchIndent] = '\0';

        if (pTypeInfo->pfnDeserialize)
        {
            log_printf(LOG_INFO,"%s  %s\n", pszIndent, pParseCtx->pszValue);
        }
        log_printf(LOG_INFO,"%s</%s>\n", pszIndent, s_elements_GetNode(pTreeNode->element)->pszElement);
    }
#endif /* def HDK_LOGGING */
}

/* Handle an XML element value */
void HDK_Parse_ElementValue(HDK_ParseContext* pParseCtx, char* pszValue, int cbValue)
{
    HDK_ElementTreeNode* pTreeNode;
    unsigned int cbValueNew;

    /* Already an error? */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return;
    }

    /* Does this element have a value? */
    pTreeNode = s_elementTree_GetNode(pParseCtx->ixElement);
    if (pTreeNode->type == HDK_Type__UNKNOWN__ ||
        pTreeNode->type == HDK_Type__UNKNOWN_ANY__ ||
        pTreeNode->type == HDK_Type__STRUCT__)
    {
        return;
    }

    /* Has the value grown too large? */
#ifndef HDK_MAX_VALUESIZE
#define HDK_MAX_VALUESIZE (16 * 1024)
#endif
    cbValueNew = pParseCtx->cbValue + cbValue + 1;
    if (cbValueNew > HDK_MAX_VALUESIZE)
    {
        log_printf(LOG_ERR, "Exceeded maximum value size %s\n", HDK_MAX_VALUESIZE);
        pParseCtx->parseError = HDK_ParseError_500_XMLInvalidRequest;
        return;
    }

    /* Grow the buffer to fit the value, if needed */
    if (!pParseCtx->pszValue || cbValueNew > pParseCtx->cbValueBuf)
    {
        char* pValueRealloc;

        /* Calculate the new buffer size */
        if (pParseCtx->pszValue)
        {
            pParseCtx->cbValueBuf *= 2;
        }
        if (cbValueNew > pParseCtx->cbValueBuf)
        {
            pParseCtx->cbValueBuf = cbValueNew;
        }

        /* Allocate/reallocate the buffer */
        pValueRealloc = (char*)realloc(pParseCtx->pszValue, pParseCtx->cbValueBuf);
        if (pValueRealloc)
        {
            pParseCtx->pszValue = pValueRealloc;
        }
        else
        {
            pParseCtx->parseError = HDK_ParseError_500_OutOfMemory;
            return;
        }
    }

    /* Append to the value string */
    memcpy(pParseCtx->pszValue + pParseCtx->cbValue, pszValue, cbValue);
    pParseCtx->cbValue += cbValue;
    *(pParseCtx->pszValue + pParseCtx->cbValue) = '\0';
}

/* Helper function for HDK_Struct_Validate */
static int HDK_Struct_Validate_Helper(HDK_Struct* pStruct, unsigned int ixTreeStruct)
{
    /* Get the struct's child tree nodes */
    unsigned int ixChildBegin;
    unsigned int ixChildEnd;
    if (s_elementTree_GetChildNodes(ixTreeStruct, &ixChildBegin, &ixChildEnd))
    {
        unsigned int ixChild;
        HDK_Member* pMember;

        /* Iterate the child tree nodes */
        for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
        {
            HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);

            /* Count the matching members */
            unsigned int nChild = 0;
            for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
            {
                if (pMember->element == (HDK_Element)pChildNode->element &&
                    (pMember->type == (HDK_Type)pChildNode->type || pMember->type == HDK_Type__BLANK__))
                {
                    nChild++;
                }
            }

            /* Does the count match the element tree? */
            if ((nChild == 0 && !(pChildNode->prop & HDK_ElementTreeProp_Optional)) ||
                (nChild > 1 && !(pChildNode->prop & HDK_ElementTreeProp_Unbounded)))
            {
                log_printf(LOG_ERR, "Unexpected count %d for element '%s'\n", nChild, s_elements_GetNode(pChildNode->element)->pszElement);
                return 0;
            }
        }

        /* Iterate the members */
        for (pMember = pStruct->pHead; pMember; pMember = pMember->pNext)
        {
            /* Ensure the member is allowed in this struct */
            int fAllowed = 0;
            for (ixChild = ixChildBegin; ixChild < ixChildEnd; ixChild++)
            {
                HDK_ElementTreeNode* pChildNode = s_elementTree_GetNode(ixChild);
                if (pMember->element == (HDK_Element)pChildNode->element &&
                    (pMember->type == (HDK_Type)pChildNode->type || pMember->type == HDK_Type__BLANK__))
                {
                    fAllowed = 1;

                    /* Validate struct members */
                    if (pMember->type == HDK_Type__STRUCT__)
                    {
                        if (!HDK_Struct_Validate_Helper(HDK_Get_StructMember(pMember), ixChild))
                        {
                            return 0;
                        }
                    }

                    break;
                }
            }
            if (!fAllowed)
            {
                return 0;
            }
        }
    }
    else
    {
        /* No child tree nodes - ensure there are no members */
        if (pStruct->pHead)
        {
            return 0;
        }
    }

    return 1;
}

/* Validate a request input struct */
int HDK_Struct_Validate(HDK_Struct* pStruct, HDK_Element topElement)
{
    unsigned int ixTopElement;

    /* Ensure that the top element matches struct's element */
    if (pStruct->node.element != topElement)
    {
        log_printf(LOG_ERR, "Unexpected element '%s' %d, top is %d\n",  s_elements_GetNode(pStruct->node.element)->pszElement, pStruct->node.element, topElement);
        return 0;
    }

    /* Find the top level element in the XML element tree */
    if (!s_elementTree_GetChildNode(2, topElement, &ixTopElement))
    {
        log_printf(LOG_ERR, "Unexpected level for element '%s'\n",  s_elements_GetNode(pStruct->node.element)->pszElement);
        return 0;
    }

    /* Validate the struct */
    return HDK_Struct_Validate_Helper(pStruct, ixTopElement);
}
