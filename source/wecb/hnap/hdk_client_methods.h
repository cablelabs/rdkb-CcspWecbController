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

/*
 * hdk_client_methods.h - HNAP client function prototypes
 */

#ifndef __HDK_CLIENT_METHODS_H__
#define __HDK_CLIENT_METHODS_H__

#include "hdk_data.h"

typedef enum _HDK_ClientError
{
    HDK_ClientError_OK = 0,       /* No error */
    HDK_ClientError_HnapParse,    /* Error parsing the HNAP response. */
    HDK_ClientError_SoapFault,    /* SOAP fault returned by server. */
    HDK_ClientError_HttpAuth,     /* Invalid HTTP authentication credentials. */
    HDK_ClientError_HttpUnknown,  /* Unknown HTTP response code. */
    HDK_ClientError_Connection,   /* Transport-layer connection error. */
    HDK_ClientError_EnsureLatency, /* Error ensuring hnap send latency */
    HDK_ClientError_Unknown       /* Unknown error */
} HDK_ClientError;

typedef struct _HDK_ClientContext
{
    char* pszURL;                 /* URL of the HNAP service.  E.g. https://192.168.0.1/HNAP1 */
    char* pszUsername;            /* HTTP Basic Authentication username. */
    char* pszPassword;            /* HTTP Basic Authentication password. */
    void* pStreamCtx;             /* Caller-defined stream context, passed to stream callbacks. */
} HDK_ClientContext;


/* HNAP client method function defines */
#define HDK_CLIENTMETHOD_CISCO_GETCLIENTINFO
#define HDK_CLIENTMETHOD_CISCO_GETEXTENDERSTATUS
#define HDK_CLIENTMETHOD_CISCO_SETDORESTART
#define HDK_CLIENTMETHOD_CISCO_SETRADIOS
#define HDK_CLIENTMETHOD_CISCO_SETSSIDSETTINGS
#define HDK_CLIENTMETHOD_CISCO_SETTOD
#define HDK_CLIENTMETHOD_CISCO_SETWPS
#define HDK_CLIENTMETHOD_PN_GETDEVICESETTINGS
#define HDK_CLIENTMETHOD_PN_GETWLANRADIOS
#define HDK_CLIENTMETHOD_PN_ISDEVICEREADY

/* HNAP client method function prototypes */
HDK_ClientError HDK_ClientMethod_Cisco_GetClientInfo(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_GetExtenderStatus(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_SetDoRestart(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_SetRadios(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pInput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_SetSSIDSettings(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pInput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_SetTOD(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pInput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_Cisco_SetWPS(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pInput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_PN_GetDeviceSettings(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_PN_GetWLanRadios(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult);
HDK_ClientError HDK_ClientMethod_PN_IsDeviceReady(HDK_ClientContext* pClientCtx, int timeoutMsecs, HDK_Enum_Result* pResult);

#endif /* #ifndef __HDK_CLIENT_METHODS_H__ */
