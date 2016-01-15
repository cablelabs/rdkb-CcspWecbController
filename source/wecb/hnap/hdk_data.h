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


#ifndef __HDK_DATA_H__
#define __HDK_DATA_H__

/* HNAP element enumeration */
typedef enum _HDK_Element
{
    HDK_Element__UNKNOWN__ = 0,
    HDK_Element_detail,
    HDK_Element_faultactor,
    HDK_Element_faultcode,
    HDK_Element_faultstring,
    HDK_Element_Cisco_AC,
    HDK_Element_Cisco_ACList,
    HDK_Element_Cisco_ACM,
    HDK_Element_Cisco_AIFSN,
    HDK_Element_Cisco_BSSID,
    HDK_Element_Cisco_BeaconInterval,
    HDK_Element_Cisco_CWMax,
    HDK_Element_Cisco_CWMin,
    HDK_Element_Cisco_Channel,
    HDK_Element_Cisco_ChannelWidth,
    HDK_Element_Cisco_ClientInfo,
    HDK_Element_Cisco_ClientInfoLists,
    HDK_Element_Cisco_Coexistance,
    HDK_Element_Cisco_DTIMInterval,
    HDK_Element_Cisco_DeviceName,
    HDK_Element_Cisco_Dst,
    HDK_Element_Cisco_Enable,
    HDK_Element_Cisco_Encryption,
    HDK_Element_Cisco_Epoch,
    HDK_Element_Cisco_ExtenderState,
    HDK_Element_Cisco_FilterType,
    HDK_Element_Cisco_GetClientInfo,
    HDK_Element_Cisco_GetClientInfoResponse,
    HDK_Element_Cisco_GetClientInfoResult,
    HDK_Element_Cisco_GetExtenderStatus,
    HDK_Element_Cisco_GetExtenderStatusResponse,
    HDK_Element_Cisco_GetExtenderStatusResult,
    HDK_Element_Cisco_GuardInterval,
    HDK_Element_Cisco_Hour,
    HDK_Element_Cisco_MACAddress,
    HDK_Element_Cisco_MACList,
    HDK_Element_Cisco_MaxClients,
    HDK_Element_Cisco_Minutes,
    HDK_Element_Cisco_Mode,
    HDK_Element_Cisco_ModeEnabled,
    HDK_Element_Cisco_Month,
    HDK_Element_Cisco_MonthDay,
    HDK_Element_Cisco_NoACK,
    HDK_Element_Cisco_PINCode,
    HDK_Element_Cisco_Passphrase,
    HDK_Element_Cisco_PreSharedKey,
    HDK_Element_Cisco_Qos,
    HDK_Element_Cisco_QosSettings,
    HDK_Element_Cisco_RadioID,
    HDK_Element_Cisco_RadioList,
    HDK_Element_Cisco_RadioSettings,
    HDK_Element_Cisco_RadiusSecret,
    HDK_Element_Cisco_RadiusServerIP,
    HDK_Element_Cisco_RadiusServerPort,
    HDK_Element_Cisco_RekeyInterval,
    HDK_Element_Cisco_SSID,
    HDK_Element_Cisco_SSIDBroadcast,
    HDK_Element_Cisco_SSIDEnabled,
    HDK_Element_Cisco_SSIDEncryption,
    HDK_Element_Cisco_SSIDIndex,
    HDK_Element_Cisco_SSIDLanBase,
    HDK_Element_Cisco_SSIDList,
    HDK_Element_Cisco_SSIDQoS,
    HDK_Element_Cisco_SSIDRadioID,
    HDK_Element_Cisco_SSIDSettings,
    HDK_Element_Cisco_SSIDVlanID,
    HDK_Element_Cisco_SecondaryChannel,
    HDK_Element_Cisco_Seconds,
    HDK_Element_Cisco_SetDoRestart,
    HDK_Element_Cisco_SetDoRestartResponse,
    HDK_Element_Cisco_SetDoRestartResult,
    HDK_Element_Cisco_SetRadios,
    HDK_Element_Cisco_SetRadiosResponse,
    HDK_Element_Cisco_SetRadiosResult,
    HDK_Element_Cisco_SetSSIDSettings,
    HDK_Element_Cisco_SetSSIDSettingsResponse,
    HDK_Element_Cisco_SetSSIDSettingsResult,
    HDK_Element_Cisco_SetTOD,
    HDK_Element_Cisco_SetTODResponse,
    HDK_Element_Cisco_SetTODResult,
    HDK_Element_Cisco_SetWPS,
    HDK_Element_Cisco_SetWPSResponse,
    HDK_Element_Cisco_SetWPSResult,
    HDK_Element_Cisco_TXOPLimit,
    HDK_Element_Cisco_Type,
    HDK_Element_Cisco_UAPSDEnable,
    HDK_Element_Cisco_WDay,
    HDK_Element_Cisco_WMMEnable,
    HDK_Element_Cisco_WPSEnable,
    HDK_Element_Cisco_WepKey,
    HDK_Element_Cisco_YDay,
    HDK_Element_Cisco_Year,
    HDK_Element_PN_Channel,
    HDK_Element_PN_Channels,
    HDK_Element_PN_DeviceName,
    HDK_Element_PN_Encryptions,
    HDK_Element_PN_FirmwareVersion,
    HDK_Element_PN_Frequency,
    HDK_Element_PN_GetDeviceSettings,
    HDK_Element_PN_GetDeviceSettingsResponse,
    HDK_Element_PN_GetDeviceSettingsResult,
    HDK_Element_PN_GetWLanRadios,
    HDK_Element_PN_GetWLanRadiosResponse,
    HDK_Element_PN_GetWLanRadiosResult,
    HDK_Element_PN_IsDeviceReady,
    HDK_Element_PN_IsDeviceReadyResponse,
    HDK_Element_PN_IsDeviceReadyResult,
    HDK_Element_PN_ModelDescription,
    HDK_Element_PN_ModelName,
    HDK_Element_PN_Name,
    HDK_Element_PN_PresentationURL,
    HDK_Element_PN_RadioID,
    HDK_Element_PN_RadioInfo,
    HDK_Element_PN_RadioInfos,
    HDK_Element_PN_SOAPActions,
    HDK_Element_PN_SecondaryChannels,
    HDK_Element_PN_SecurityInfo,
    HDK_Element_PN_SecurityType,
    HDK_Element_PN_SubDeviceURLs,
    HDK_Element_PN_SupportedModes,
    HDK_Element_PN_SupportedSecurity,
    HDK_Element_PN_TaskExtension,
    HDK_Element_PN_Tasks,
    HDK_Element_PN_Type,
    HDK_Element_PN_URL,
    HDK_Element_PN_VendorName,
    HDK_Element_PN_WideChannel,
    HDK_Element_PN_WideChannels,
    HDK_Element_PN_int,
    HDK_Element_PN_string,
    HDK_Element__BODY__,
    HDK_Element__ENVELOPE__,
    HDK_Element__FAULT__,
    HDK_Element__HEADER__
} HDK_Element;

extern int HDK_ExpandElementURI(HDK_Element element, char* pszURI, unsigned int cchURI);

/* HNAP type defines */
#define HDK_TYPE__IPADDRESS__
#define HDK_TYPE__MACADDRESS__
#define HDK_TYPE__RESULT__
#define HDK_TYPE_CISCO_DEVICEINF
#define HDK_TYPE_CISCO_WIFIENCRYPTION
#define HDK_TYPE_CISCO_WIFIMODE
#define HDK_TYPE_CISCO_WIFISECURITY
#define HDK_TYPE_PN_DEVICETYPE
#define HDK_TYPE_PN_TASKEXTTYPE
#define HDK_TYPE_PN_WIFIENCRYPTION
#define HDK_TYPE_PN_WIFIMODE
#define HDK_TYPE_PN_WIFISECURITY
#define HDK_TYPE__BOOL__
#define HDK_TYPE__INT__
#define HDK_TYPE__STRING__

/* HNAP type enumeration */
typedef enum _HDK_Type
{
    HDK_Type__UNKNOWN__ = 0,
    HDK_Type__UNKNOWN_ANY__,
    HDK_Type__STRUCT__,
    HDK_Type__BLANK__,
    HDK_Type__IPADDRESS__,
    HDK_Type__MACADDRESS__,
    HDK_Type__RESULT__,
    HDK_Type_Cisco_DeviceInf,
    HDK_Type_Cisco_WiFiEncryption,
    HDK_Type_Cisco_WiFiMode,
    HDK_Type_Cisco_WiFiSecurity,
    HDK_Type_PN_DeviceType,
    HDK_Type_PN_TaskExtType,
    HDK_Type_PN_WiFiEncryption,
    HDK_Type_PN_WiFiMode,
    HDK_Type_PN_WiFiSecurity,
    HDK_Type__BOOL__,
    HDK_Type__INT__,
    HDK_Type__STRING__
} HDK_Type;

/* HNAP generic structure member node */
typedef struct _HDK_Member
{
    HDK_Element element;               /* XML element */
    HDK_Type type;                     /* Element type */
    struct _HDK_Member* pNext;
} HDK_Member;

/* HNAP struct */
typedef struct _HDK_Member_Struct
{
    HDK_Member node;
    HDK_Member* pHead;
    HDK_Member* pTail;
} HDK_Struct;

/* HNAP generic member functions */
extern HDK_Member* HDK_Copy_Member(HDK_Struct* pStructDst, HDK_Element elementDst,
                                   HDK_Member* pMemberSrc, int fAppend);
extern HDK_Member* HDK_Get_Member(HDK_Struct* pStruct, HDK_Element element, HDK_Type type);

/* HNAP struct type */
extern HDK_Struct* HDK_Set_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Set_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct);
extern HDK_Struct* HDK_Append_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Append_StructEx(HDK_Struct* pStructDst, HDK_Element element, HDK_Struct* pStruct);
extern HDK_Struct* HDK_Get_Struct(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Struct* HDK_Get_StructMember(HDK_Member* pMember);
void HDK_Detach_Struct(HDK_Member* pMember, HDK_Member* pMember2);


/* HNAP struct stack initialization/free */
void HDK_Struct_Init(HDK_Struct* pStruct);
void HDK_Struct_Free(HDK_Struct* pStruct);

/* HNAP explicit blank element - use sparingly */
extern HDK_Member* HDK_Set_Blank(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Member* HDK_Append_Blank(HDK_Struct* pStruct, HDK_Element element);

/* HNAP bool type */
extern HDK_Member* HDK_Set_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue);
extern HDK_Member* HDK_Append_Bool(HDK_Struct* pStruct, HDK_Element element, int fValue);
extern int* HDK_Get_Bool(HDK_Struct* pStruct, HDK_Element element);
extern int HDK_Get_BoolEx(HDK_Struct* pStruct, HDK_Element element, int fDefault);
extern int* HDK_Get_BoolMember(HDK_Member* pMember);

/* HNAP int type */
extern HDK_Member* HDK_Set_Int(HDK_Struct* pStruct, HDK_Element element, int iValue);
extern HDK_Member* HDK_Append_Int(HDK_Struct* pStruct, HDK_Element element, int iValue);
extern int* HDK_Get_Int(HDK_Struct* pStruct, HDK_Element element);
extern int HDK_Get_IntEx(HDK_Struct* pStruct, HDK_Element element, int iDefault);
extern int* HDK_Get_IntMember(HDK_Member* pMember);

/* HNAP string type */
extern HDK_Member* HDK_Set_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue);
extern HDK_Member* HDK_Append_String(HDK_Struct* pStruct, HDK_Element element, char* pszValue);
extern char* HDK_Get_String(HDK_Struct* pStruct, HDK_Element element);
extern char* HDK_Get_StringEx(HDK_Struct* pStruct, HDK_Element element, char* pszDefault);
extern char* HDK_Get_StringMember(HDK_Member* pMember);

/* HNAP IPAddress type */
typedef struct _HDK_IPAddress
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
} HDK_IPAddress;

extern HDK_Member* HDK_Set_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress);
extern HDK_Member* HDK_Append_IPAddress(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pIPAddress);
extern HDK_IPAddress* HDK_Get_IPAddress(HDK_Struct* pStruct, HDK_Element element);
extern HDK_IPAddress* HDK_Get_IPAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_IPAddress* pDefault);
extern HDK_IPAddress* HDK_Get_IPAddressMember(HDK_Member* pMember);

/* HNAP MACAddress type */
typedef struct _HDK_MACAddress
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
} HDK_MACAddress;

extern HDK_Member* HDK_Set_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress);
extern HDK_Member* HDK_Append_MACAddress(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pMACAddress);
extern HDK_MACAddress* HDK_Get_MACAddress(HDK_Struct* pStruct, HDK_Element element);
extern HDK_MACAddress* HDK_Get_MACAddressEx(HDK_Struct* pStruct, HDK_Element element, HDK_MACAddress* pDefault);
extern HDK_MACAddress* HDK_Get_MACAddressMember(HDK_Member* pMember);

/* HDK_Type__RESULT__ enumeration type */
typedef enum _HDK_Enum_Result
{
    HDK_Enum_Result__UNKNOWN__ = 0,
    HDK_Enum_Result_OK,
    HDK_Enum_Result_REBOOT,
    HDK_Enum_Result_ERROR,
    HDK_Enum_Result_ERROR_BAD_BEACONINTERVAL,
    HDK_Enum_Result_ERROR_BAD_CHANNEL,
    HDK_Enum_Result_ERROR_BAD_CHANNEL_WIDTH,
    HDK_Enum_Result_ERROR_BAD_DTIMINTERVAL,
    HDK_Enum_Result_ERROR_BAD_GUARDINTERVAL,
    HDK_Enum_Result_ERROR_BAD_MODE,
    HDK_Enum_Result_ERROR_BAD_RADIOID,
    HDK_Enum_Result_ERROR_BAD_SECONDARY_CHANNEL,
    HDK_Enum_Result_ERROR_BAD_SSID,
    HDK_Enum_Result_ERROR_BAD_SSID_ID,
    HDK_Enum_Result_ERROR_BAD_VLANID,
    HDK_Enum_Result_ERROR_ENCRYPTION,
    HDK_Enum_Result_ERROR_QOS,
    HDK_Enum_Result_ERROR_RadioUnsupported
} HDK_Enum_Result;

#define HDK_SUCCEEDED(result) ((result) > HDK_Enum_Result__UNKNOWN__ && (result) < HDK_Enum_Result_ERROR)
#define HDK_FAILED(result) (!HDK_SUCCEEDED(result))

extern const char* HDK_Enum_ResultToString(HDK_Enum_Result result);

extern HDK_Member* HDK_Set_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue);
extern HDK_Member* HDK_Append_Result(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eValue);
extern HDK_Enum_Result* HDK_Get_Result(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Result HDK_Get_ResultEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Result eDefault);
extern HDK_Enum_Result* HDK_Get_ResultMember(HDK_Member* pMember);

/* HDK_Type_Cisco_DeviceInf enumeration type */
typedef enum _HDK_Enum_Cisco_DeviceInf
{
    HDK_Enum_Cisco_DeviceInf__UNKNOWN__ = 0,
    HDK_Enum_Cisco_DeviceInf_WiFi_2_4G,
    HDK_Enum_Cisco_DeviceInf_WiFi_5_0G,
    HDK_Enum_Cisco_DeviceInf_Eth,
    HDK_Enum_Cisco_DeviceInf_
} HDK_Enum_Cisco_DeviceInf;

extern HDK_Member* HDK_Set_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eValue);
extern HDK_Member* HDK_Append_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eValue);
extern HDK_Enum_Cisco_DeviceInf* HDK_Get_Cisco_DeviceInf(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Cisco_DeviceInf HDK_Get_Cisco_DeviceInfEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_DeviceInf eDefault);
extern HDK_Enum_Cisco_DeviceInf* HDK_Get_Cisco_DeviceInfMember(HDK_Member* pMember);

/* HDK_Type_Cisco_WiFiEncryption enumeration type */
typedef enum _HDK_Enum_Cisco_WiFiEncryption
{
    HDK_Enum_Cisco_WiFiEncryption__UNKNOWN__ = 0,
    HDK_Enum_Cisco_WiFiEncryption_WEP_64,
    HDK_Enum_Cisco_WiFiEncryption_WEP_128,
    HDK_Enum_Cisco_WiFiEncryption_AES,
    HDK_Enum_Cisco_WiFiEncryption_TKIP,
    HDK_Enum_Cisco_WiFiEncryption_TKIPORAES,
    HDK_Enum_Cisco_WiFiEncryption_
} HDK_Enum_Cisco_WiFiEncryption;

extern HDK_Member* HDK_Set_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eValue);
extern HDK_Member* HDK_Append_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eValue);
extern HDK_Enum_Cisco_WiFiEncryption* HDK_Get_Cisco_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Cisco_WiFiEncryption HDK_Get_Cisco_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiEncryption eDefault);
extern HDK_Enum_Cisco_WiFiEncryption* HDK_Get_Cisco_WiFiEncryptionMember(HDK_Member* pMember);

/* HDK_Type_Cisco_WiFiMode enumeration type */
typedef enum _HDK_Enum_Cisco_WiFiMode
{
    HDK_Enum_Cisco_WiFiMode__UNKNOWN__ = 0,
    HDK_Enum_Cisco_WiFiMode_802_11a,
    HDK_Enum_Cisco_WiFiMode_802_11b,
    HDK_Enum_Cisco_WiFiMode_802_11g,
    HDK_Enum_Cisco_WiFiMode_802_11n,
    HDK_Enum_Cisco_WiFiMode_802_11bg,
    HDK_Enum_Cisco_WiFiMode_802_11bn,
    HDK_Enum_Cisco_WiFiMode_802_11bgn,
    HDK_Enum_Cisco_WiFiMode_802_11gn,
    HDK_Enum_Cisco_WiFiMode_802_11an,
	HDK_Enum_Cisco_WiFiMode_802_11bgnac, 
    HDK_Enum_Cisco_WiFiMode_802_11gnac,
    HDK_Enum_Cisco_WiFiMode_802_11nac, 
    HDK_Enum_Cisco_WiFiMode_802_11ac,
    HDK_Enum_Cisco_WiFiMode_802_11anac,
    HDK_Enum_Cisco_WiFiMode_802_11aac,
    HDK_Enum_Cisco_WiFiMode_
} HDK_Enum_Cisco_WiFiMode;

extern HDK_Member* HDK_Set_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eValue);
extern HDK_Member* HDK_Append_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eValue);
extern HDK_Enum_Cisco_WiFiMode* HDK_Get_Cisco_WiFiMode(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Cisco_WiFiMode HDK_Get_Cisco_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiMode eDefault);
extern HDK_Enum_Cisco_WiFiMode* HDK_Get_Cisco_WiFiModeMember(HDK_Member* pMember);

/* HDK_Type_Cisco_WiFiSecurity enumeration type */
typedef enum _HDK_Enum_Cisco_WiFiSecurity
{
    HDK_Enum_Cisco_WiFiSecurity__UNKNOWN__ = 0,
    HDK_Enum_Cisco_WiFiSecurity_None,
    HDK_Enum_Cisco_WiFiSecurity_WEP_64,
    HDK_Enum_Cisco_WiFiSecurity_WEP_128,
    HDK_Enum_Cisco_WiFiSecurity_WPA_Personal,
    HDK_Enum_Cisco_WiFiSecurity_WPA2_Personal,
    HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Personal,
    HDK_Enum_Cisco_WiFiSecurity_WPA_Enterprise,
    HDK_Enum_Cisco_WiFiSecurity_WPA2_Enterprise,
    HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Enterprise,
    HDK_Enum_Cisco_WiFiSecurity_
} HDK_Enum_Cisco_WiFiSecurity;

extern HDK_Member* HDK_Set_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eValue);
extern HDK_Member* HDK_Append_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eValue);
extern HDK_Enum_Cisco_WiFiSecurity* HDK_Get_Cisco_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_Cisco_WiFiSecurity HDK_Get_Cisco_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_Cisco_WiFiSecurity eDefault);
extern HDK_Enum_Cisco_WiFiSecurity* HDK_Get_Cisco_WiFiSecurityMember(HDK_Member* pMember);

/* HDK_Type_PN_DeviceType enumeration type */
typedef enum _HDK_Enum_PN_DeviceType
{
    HDK_Enum_PN_DeviceType__UNKNOWN__ = 0,
    HDK_Enum_PN_DeviceType_Computer,
    HDK_Enum_PN_DeviceType_ComputerServer,
    HDK_Enum_PN_DeviceType_WorkstationComputer,
    HDK_Enum_PN_DeviceType_LaptopComputer,
    HDK_Enum_PN_DeviceType_Gateway,
    HDK_Enum_PN_DeviceType_GatewayWithWiFi,
    HDK_Enum_PN_DeviceType_DigitalDVR,
    HDK_Enum_PN_DeviceType_DigitalJukebox,
    HDK_Enum_PN_DeviceType_MediaAdapter,
    HDK_Enum_PN_DeviceType_NetworkCamera,
    HDK_Enum_PN_DeviceType_NetworkDevice,
    HDK_Enum_PN_DeviceType_NetworkDrive,
    HDK_Enum_PN_DeviceType_NetworkGameConsole,
    HDK_Enum_PN_DeviceType_NetworkPDA,
    HDK_Enum_PN_DeviceType_NetworkPrinter,
    HDK_Enum_PN_DeviceType_NetworkPrintServer,
    HDK_Enum_PN_DeviceType_PhotoFrame,
    HDK_Enum_PN_DeviceType_VOIPDevice,
    HDK_Enum_PN_DeviceType_WiFiAccessPoint,
    HDK_Enum_PN_DeviceType_SetTopBox,
    HDK_Enum_PN_DeviceType_WiFiBridge,
    HDK_Enum_PN_DeviceType_WiFiExtender
} HDK_Enum_PN_DeviceType;

extern HDK_Member* HDK_Set_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue);
extern HDK_Member* HDK_Append_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eValue);
extern HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceType(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_DeviceType HDK_Get_PN_DeviceTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_DeviceType eDefault);
extern HDK_Enum_PN_DeviceType* HDK_Get_PN_DeviceTypeMember(HDK_Member* pMember);

/* HDK_Type_PN_TaskExtType enumeration type */
typedef enum _HDK_Enum_PN_TaskExtType
{
    HDK_Enum_PN_TaskExtType__UNKNOWN__ = 0,
    HDK_Enum_PN_TaskExtType_Browser,
    HDK_Enum_PN_TaskExtType_MessageBox,
    HDK_Enum_PN_TaskExtType_PUI,
    HDK_Enum_PN_TaskExtType_Silent
} HDK_Enum_PN_TaskExtType;

extern HDK_Member* HDK_Set_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue);
extern HDK_Member* HDK_Append_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eValue);
extern HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtType(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_TaskExtType HDK_Get_PN_TaskExtTypeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_TaskExtType eDefault);
extern HDK_Enum_PN_TaskExtType* HDK_Get_PN_TaskExtTypeMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiEncryption enumeration type */
typedef enum _HDK_Enum_PN_WiFiEncryption
{
    HDK_Enum_PN_WiFiEncryption__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiEncryption_WEP_64,
    HDK_Enum_PN_WiFiEncryption_WEP_128,
    HDK_Enum_PN_WiFiEncryption_AES,
    HDK_Enum_PN_WiFiEncryption_TKIP,
    HDK_Enum_PN_WiFiEncryption_TKIPORAES,
    HDK_Enum_PN_WiFiEncryption_
} HDK_Enum_PN_WiFiEncryption;

extern HDK_Member* HDK_Set_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue);
extern HDK_Member* HDK_Append_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eValue);
extern HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryption(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiEncryption HDK_Get_PN_WiFiEncryptionEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiEncryption eDefault);
extern HDK_Enum_PN_WiFiEncryption* HDK_Get_PN_WiFiEncryptionMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiMode enumeration type */
typedef enum _HDK_Enum_PN_WiFiMode
{
    HDK_Enum_PN_WiFiMode__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiMode_802_11a,
    HDK_Enum_PN_WiFiMode_802_11b,
    HDK_Enum_PN_WiFiMode_802_11g,
    HDK_Enum_PN_WiFiMode_802_11n,
    HDK_Enum_PN_WiFiMode_802_11bg,
    HDK_Enum_PN_WiFiMode_802_11bn,
    HDK_Enum_PN_WiFiMode_802_11bgn,
    HDK_Enum_PN_WiFiMode_802_11gn,
    HDK_Enum_PN_WiFiMode_802_11an,
	HDK_Enum_PN_WiFiMode_802_11bgnac, 
    HDK_Enum_PN_WiFiMode_802_11gnac,
    HDK_Enum_PN_WiFiMode_802_11nac, 
    HDK_Enum_PN_WiFiMode_802_11ac,
    HDK_Enum_PN_WiFiMode_802_11anac,
    HDK_Enum_PN_WiFiMode_802_11aac,
    HDK_Enum_PN_WiFiMode_
} HDK_Enum_PN_WiFiMode;

extern HDK_Member* HDK_Set_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue);
extern HDK_Member* HDK_Append_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eValue);
extern HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiMode(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiMode HDK_Get_PN_WiFiModeEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiMode eDefault);
extern HDK_Enum_PN_WiFiMode* HDK_Get_PN_WiFiModeMember(HDK_Member* pMember);

/* HDK_Type_PN_WiFiSecurity enumeration type */
typedef enum _HDK_Enum_PN_WiFiSecurity
{
    HDK_Enum_PN_WiFiSecurity__UNKNOWN__ = 0,
    HDK_Enum_PN_WiFiSecurity_NONE,
    HDK_Enum_PN_WiFiSecurity_WEP_64,
    HDK_Enum_PN_WiFiSecurity_WEP_128,
    HDK_Enum_PN_WiFiSecurity_WPA_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA2_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Personal,
    HDK_Enum_PN_WiFiSecurity_WPA_Enterprise,
    HDK_Enum_PN_WiFiSecurity_WPA2_Enterprise,
    HDK_Enum_PN_WiFiSecurity_WPA_WPA2_Enterprise,
    HDK_Enum_PN_WiFiSecurity_
} HDK_Enum_PN_WiFiSecurity;

extern HDK_Member* HDK_Set_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue);
extern HDK_Member* HDK_Append_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eValue);
extern HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurity(HDK_Struct* pStruct, HDK_Element element);
extern HDK_Enum_PN_WiFiSecurity HDK_Get_PN_WiFiSecurityEx(HDK_Struct* pStruct, HDK_Element element, HDK_Enum_PN_WiFiSecurity eDefault);
extern HDK_Enum_PN_WiFiSecurity* HDK_Get_PN_WiFiSecurityMember(HDK_Member* pMember);
int HDK_Struct_Validate(HDK_Struct* pStruct, HDK_Element topElement);

#endif /* __HDK_DATA_H__ */
