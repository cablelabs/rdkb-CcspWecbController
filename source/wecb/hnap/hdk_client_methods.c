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
 * hdk_client_methods.c - HNAP client function definitions
 */

#include "hdk_client_methods.h"
#include "hdk_data.h"
#include "hdk_internal.h"
#include "hdk_client_http_interface.h"
#include "hdk_logging.h"
#include "hdk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _MSC_VER
/* Disable "'function': was declared deprecated" warning. */
#  pragma warning(disable : 4996)

/* Disable "conditional expression is constant" warning. */
#  pragma warning(disable : 4127)
#endif /* def _MSC_VER */


static bool IsUTF8_SingleByte(char c)
{
    unsigned char uc = (unsigned char)c;
    return uc <= 127;
}

static bool IsUTF8_Continuation(char c)
{
    unsigned char uc = (unsigned char)c;
    return 128 <= uc && uc <= 191;
}

static bool IsUTF8_2ByteStart(char c)
{
    unsigned char uc = (unsigned char)c;
    return 192 <= uc && uc <= 223;
}

static bool IsUTF8_3ByteStart(char c)
{
    unsigned char uc = (unsigned char)c;
    return 224 <= uc && uc <= 239;
}

static bool IsUTF8_4ByteStart(char c)
{
    unsigned char uc = (unsigned char)c;
    return 240 <= uc && uc <= 244;
}

static void ReplaceCharacter(char* pc)
{
    pc[0] = '?';
}

static bool IsValidUTF8ContinuationSequence(char* pcBegin, unsigned int celt)
{
    unsigned int index;

    for (index = 0; index < celt; index++)
    {
        if (!IsUTF8_Continuation(pcBegin[index]))
        {
            return false;
        }
    }

    return true;
}

/*
    Given a parse context and buffer, this function will convert all illegal UTF-8 characters of the buffer to '?'.
    It returns the new count of bytes safe to parse from the start of the buffer, the new edge context information
    and lastly whether the current context edge should be parsed first.

    pcEdge should be at least 3 characters long.
 */
static void EnsureUTF8(char* pcBegin, unsigned int cbBuf,
                       char* pcEdge,
                       unsigned int celtEdgeUsed,
                       unsigned int* pcbBufNew,
                       bool* pfParseContextEdge,
                       char* pcEdgeNew,
                       unsigned int* pceltEdgeUsedNew)
{
    *pcbBufNew = cbBuf;
    *pceltEdgeUsedNew = 0;
    *pfParseContextEdge = false;

    unsigned int index = 0;
    if (celtEdgeUsed > 0)
    {
        /* Handle the case that this UTF character spans multiple chunks */
        unsigned int celtContinuation = 0;
        if (IsUTF8_2ByteStart(pcEdge[0]))
        {
            celtContinuation = 1;
        }
        else if (IsUTF8_3ByteStart(pcEdge[0]))
        {
            celtContinuation = 2;
        }
        else if (IsUTF8_4ByteStart(pcEdge[0]))
        {
            celtContinuation = 3;
        }

        if (celtEdgeUsed + cbBuf < 1 + celtContinuation)
        {
            /* copy everything to the new edge */
            memcpy(pcEdgeNew,
                   pcEdge,
                   sizeof(char) * celtEdgeUsed);
            memcpy(&pcEdgeNew[celtEdgeUsed],
                   &pcBegin[0],
                   sizeof(char) * cbBuf);
            *pcbBufNew = 0;
            *pceltEdgeUsedNew = celtEdgeUsed + cbBuf;
            index = cbBuf;
        }
        else
        {
            *pfParseContextEdge = true;
            unsigned int indexEdge;
            for (indexEdge = 0; indexEdge < celtEdgeUsed; indexEdge++)
            {
                if (IsUTF8_SingleByte(pcEdge[indexEdge]))
                {
                    continue;
                }
                else if (IsUTF8_2ByteStart(pcEdge[indexEdge]))
                {
                    celtContinuation = 1;
                }
                else if (IsUTF8_3ByteStart(pcEdge[indexEdge]))
                {
                    celtContinuation = 2;
                }
                else if (IsUTF8_4ByteStart(pcEdge[indexEdge]))
                {
                    celtContinuation = 3;
                }
                else
                {
                    ReplaceCharacter(&pcEdge[indexEdge]);
                    continue;
                }

                if (IsValidUTF8ContinuationSequence(&pcEdge[indexEdge + 1], celtEdgeUsed - indexEdge - 1) &&
                    IsValidUTF8ContinuationSequence(pcBegin, celtContinuation - (celtEdgeUsed - indexEdge - 1)))
                {
                    index = celtContinuation - (celtEdgeUsed - indexEdge - 1);
                    break;
                }
                else
                {
                    ReplaceCharacter(&pcEdge[indexEdge]);
                }
            }
        }
    }

    for (; index < cbBuf; index++)
    {
        unsigned int celtContinuation = 0;
        if (IsUTF8_SingleByte(pcBegin[index]))
        {
            continue;
        }
        else if (IsUTF8_2ByteStart(pcBegin[index]))
        {
            celtContinuation = 1;
        }
        else if (IsUTF8_3ByteStart(pcBegin[index]))
        {
            celtContinuation = 2;
        }
        else if (IsUTF8_4ByteStart(pcBegin[index]))
        {
            celtContinuation = 3;
        }
        else
        {
            ReplaceCharacter(&pcBegin[index]);
            continue;
        }

        /* handle the case when we are on a chunk edge */
        if (index + 1 + celtContinuation > cbBuf)
        {
            memcpy(pcEdgeNew, &pcBegin[index], sizeof(char) * (cbBuf - index));
            *pceltEdgeUsedNew = cbBuf - index;
            *pcbBufNew = index;
            break;
        }
        else if (IsValidUTF8ContinuationSequence(&pcBegin[index + 1], celtContinuation))
        {
            index += celtContinuation;
        }
        else
        {
            ReplaceCharacter(&pcBegin[index]);
        }
    }
}

static unsigned int ReceiveHeader_Callback(void* pReceiveHeaderCtx, char* pBuf, unsigned int cbBuf)
{
    /* Unused parameters */
    (void)pReceiveHeaderCtx;
    (void)pBuf;

    return cbBuf;
}

typedef struct _ReceiveContext
{
    HDK_ParseContext parseContext;
    HDK_ParseError parseError;
} ReceiveContext;

static unsigned int Receive_Callback(void* pReceiveCtx, char* pBuf, unsigned int cbBuf)
{
    ReceiveContext* pCtx = (ReceiveContext*)pReceiveCtx;


    if (HDK_ParseError_OK == pCtx->parseError)
    {
        pCtx->parseError = HDK_ParseData(&pCtx->parseContext, pBuf, cbBuf);
    }

    return (HDK_ParseError_OK == pCtx->parseError) ? cbBuf : 0;
}

#ifdef HDK_LOGGING
typedef struct _LoggingContext
{
    char* pData;
    size_t cbAllocated;
    size_t cbData;
} LoggingContext;

static void LoggingContext_AppendData(void* pRequestCtx, char* pData, int cbData)
{
    LoggingContext* pCtx = (LoggingContext*)pRequestCtx;

    /* Grow the buffer as needed... */
    size_t cbDataRequired = pCtx->cbData + (size_t)cbData + 1 /* for '\0' */;
    if (cbDataRequired > pCtx->cbAllocated)
    {
        size_t cbNewAlloc = 2 * pCtx->cbAllocated;
        if (cbDataRequired > cbNewAlloc)
        {
            cbNewAlloc = cbDataRequired;
        }
        pCtx->pData = (char*)realloc(pCtx->pData, cbDataRequired);
        if (pCtx->pData)
        {
            pCtx->cbAllocated = cbDataRequired;
        }
    }
    if (pCtx->pData)
    {
        memcpy(&pCtx->pData[pCtx->cbData], pData, cbData);
        pCtx->cbData += cbData;

        pCtx->pData[pCtx->cbData] = '\0';
    }
}

static void LoggingContext_Init(LoggingContext* pCtx)
{
    memset(pCtx, 0, sizeof(*pCtx));
}

static void LoggingContext_Cleanup(LoggingContext* pCtx)
{
    free(pCtx->pData);
}
#endif /* def HDK_LOGGING */

/* Returns the HTTP response code, or -1 on error. */
static int SendHNAPRequest(HDK_Struct* pInput, HDK_Struct* pOutput, HDK_ParseError* pParseError,
                           const char* pszURL, int fGet,
                           const char* pszUsername, const char* pszPassword,
                           int iTimeoutSecs,
                           void* pStreamCtx)
{
    int httpResponseCode = -1;
    void* pRequestCtx = 0;

    do
    {
        ReceiveContext receiveCtx;

        char szSOAPAction[64] = { '\0' };
        char szSOAPActionHeader[64];

        pRequestCtx = HDK_Client_Http_RequestCreate(pszURL, fGet);
        if (!pRequestCtx)
        {
            break;
        }

        /* Construct the request */

        if (pszUsername || pszPassword)
        {
            if (!HDK_Client_Http_RequestSetBasicAuth(pRequestCtx, pszUsername, pszPassword))
            {
                break;
            }
        }
        if (!HDK_Client_Http_RequestSetTimeout(pRequestCtx, iTimeoutSecs))
        {
            break;
        }

        if (!fGet)
        {
            size_t ixWrite;
            struct
            {
                HDK_WriteFn pfnWrite;
                void* pCtx;
            }
#ifndef HDK_LOGGING
            rgWriteFunctions[1];
#else /* def HDK_LOGGING */
            rgWriteFunctions[2];
            LoggingContext loggingCtx;
            LoggingContext_Init(&loggingCtx);
#endif /* ndef HDK_LOGGING */

            rgWriteFunctions[0].pfnWrite = HDK_Client_Http_RequestAppendData;
            rgWriteFunctions[0].pCtx = pRequestCtx;
#ifdef HDK_LOGGING
            rgWriteFunctions[1].pfnWrite = LoggingContext_AppendData;
            rgWriteFunctions[1].pCtx = &loggingCtx;
#endif /* def HDK_LOGGING */

            /*
             * Generate the SOAP action header.
             *   E.g. SOAPAction: "http://purenetworks.com/HNAP1/GetDeviceSettings2"
             */
            if (!HDK_ExpandElementURI(pInput->node.element, szSOAPAction, sizeof(szSOAPAction) / sizeof(*szSOAPAction)))
            {
                break;
            }
            sprintf(szSOAPActionHeader, "SOAPAction: \"%s\"", szSOAPAction);

            if (!HDK_Client_Http_RequestAddHeader(pRequestCtx, szSOAPActionHeader))
            {
                break;
            }

            //if (!HDK_Client_Http_RequestAddHeader(pRequestCtx, "Content-Type: text/xml; charset=\"utf-8\""))
            if (!HDK_Client_Http_RequestAddHeader(pRequestCtx, "Content-Type: text/xml"))
            {
                break;
            }

            for (ixWrite = 0; ixWrite < sizeof(rgWriteFunctions) / sizeof(*rgWriteFunctions); ixWrite++)
            {
                /* POST content */
                HDK_Write(rgWriteFunctions[ixWrite].pCtx, rgWriteFunctions[ixWrite].pfnWrite, 0,
                          "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                          "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n"
                          "<soap:Body>\n");

                HDK_Serialize(rgWriteFunctions[ixWrite].pCtx, rgWriteFunctions[ixWrite].pfnWrite, 0, pInput, pInput);

                HDK_Write(rgWriteFunctions[ixWrite].pCtx, rgWriteFunctions[ixWrite].pfnWrite, 0,
                          "</soap:Body>\n"
                          "</soap:Envelope>\n");
            }
#ifdef HDK_LOGGING
            HDK_LOGFMT(HDK_LogCategory_HNAP, HDK_LogLevel_Verbose, "%s SOAP action %s request data:\n%s", pszURL, szSOAPAction, loggingCtx.pData);
            LoggingContext_Cleanup(&loggingCtx);
#endif /* def HDK_LOGGING */
        }

        receiveCtx.parseError = HDK_ParseBegin(pStreamCtx, &receiveCtx.parseContext, pOutput);
        if (HDK_ParseError_OK == receiveCtx.parseError)
        {
            httpResponseCode = HDK_Client_Http_RequestSend(pRequestCtx,
                                                           ReceiveHeader_Callback, 0,
                                                           Receive_Callback, &receiveCtx);
            if (httpResponseCode > 0)
            {
                HDK_LOGFMT(HDK_LogCategory_HTTP, HDK_LogLevel_Info, "Request to %s with SOAP action %s completed with HTTP status %d\n", pszURL, szSOAPAction, httpResponseCode);

                /* Successfully got a response, though it may have failed to parse. */
                if (HDK_ParseError_OK == receiveCtx.parseError)
                {
                    /* Finish parsing the response. */
                    receiveCtx.parseError = HDK_ParseData(&receiveCtx.parseContext, 0, 0);
                }
            }

            /* Cleanup the parsing context. */
            HDK_ParseEnd(&receiveCtx.parseContext);
        }

        *pParseError = receiveCtx.parseError;
    }
    while (0);

    if (pRequestCtx)
    {
        HDK_Client_Http_RequestDestroy(pRequestCtx);
    }

    return httpResponseCode;
}

/* HNAP client method function helper */
static HDK_ClientError HDK_Client__MethodHelper__(HDK_ClientContext* pClientCtx,
                                                  int iTimeoutSecs,
                                                  HDK_Element inputElement, HDK_Struct* pInput,
                                                  HDK_Element outputElement, HDK_Struct* pOutput,
                                                  HDK_Element resultElement, HDK_Enum_Result* pResult)
{
    HDK_ClientError error = HDK_ClientError_OK;
    if (HDK_Element__UNKNOWN__ == pInput->node.element)
    {
        pInput->node.element = inputElement;
    }

    /* Validate the input structure from the caller. */
    if (!HDK_Struct_Validate(pInput, pInput->node.element))
    {
        *pResult = HDK_Enum_Result_ERROR;
    }
    else
    {
        HDK_ParseError parseError = HDK_ParseError_OK;
        int httpResponse = SendHNAPRequest(pInput, pOutput, &parseError,
                                           pClientCtx->pszURL, 0,//HDK_Element_PN_GetDeviceSettings == inputElement,
                                           pClientCtx->pszUsername, pClientCtx->pszPassword,
                                           iTimeoutSecs,
                                           pClientCtx->pStreamCtx);

        if (httpResponse > 0)
        {
            /* We successfully sent the request and received a response. */

            switch (httpResponse)
            {
                case 200: /* HTTP 200 OK */
                {
                    if (HDK_ParseError_OK == parseError)
                    {
                        /*
                         * Get the method result.
                         * The output is only valid if the result is OK, at least under the HDK
                         * server implementations, therefore the result must be checked prior to
                         * validating the output data.
                         * Note: This technically violates the HNAP spec, as it omits required elements
                         * when the result is not "OK", but that is the current behavior.
                         */
                        *pResult = HDK_Get_ResultEx(pOutput, resultElement, HDK_Enum_Result_ERROR);
                        if (HDK_SUCCEEDED(*pResult))
                        {
                            /* Validate the output struct.  Invalid responses are considered errors. */
                            if (!HDK_Struct_Validate(pOutput, outputElement))
                            {
                                error = HDK_ClientError_HnapParse;
                            }
                        }

#ifdef HDK_LOGGING
                        if (HDK_ClientError_OK == error)
                        {
                            char szSOAPAction[64];
                            HDK_LogLevel level = HDK_SUCCEEDED(*pResult) ? HDK_LogLevel_Info : HDK_LogLevel_Warning;

                            szSOAPAction[0] = '\0';
                            HDK_ExpandElementURI(pInput->node.element, szSOAPAction, sizeof(szSOAPAction) / sizeof(*szSOAPAction));

                            HDK_LOGFMT(HDK_LogCategory_HNAP, level, "%s completed with result %s\n", szSOAPAction, HDK_Enum_ResultToString(*pResult));
                        }
#endif /* def HDK_LOGGING */

                    }
                    else
                    {
                        error = HDK_ClientError_HnapParse;
                    }
                    break;
                }
                case 500: /* HTTP 500 Server Fault */
                {
                    /* Validate the SOAP fault struct.  Invalid responses are considered errors. */
                    if (!HDK_Struct_Validate(pOutput, HDK_Element__FAULT__))
                    {
                        error = HDK_ClientError_HnapParse;
                    }
                    else
                    {
                        HDK_LOGFMT(HDK_LogCategory_SOAP, HDK_LogLevel_Error, "SOAP Fault:\n" \
                                                                             "    faultcode: %s\n" \
                                                                             "    faultstring: %s\n" \
                                                                             "    faultactor: %s\n" \
                                                                             "    detail: %s\n", \
                                                                             HDK_Get_StringEx(pOutput, HDK_Element_faultcode, ""), \
                                                                             HDK_Get_StringEx(pOutput, HDK_Element_faultstring, ""), \
                                                                             HDK_Get_StringEx(pOutput, HDK_Element_faultactor, ""), \
                                                                             HDK_Get_StringEx(pOutput, HDK_Element_detail, ""));
                        error = HDK_ClientError_SoapFault;
                    }
                    break;
                }
                case 401: /* HTTP 401 Unauthorized */
                {
                    error = HDK_ClientError_HttpAuth;
                    break;
                }
                default:
                {
                    error = HDK_ClientError_HttpUnknown;
                    break;
                }
            }
        }
        else
        {
            error = HDK_ClientError_Connection;
        }
    }

    return error;
}

/*
 * http://cisco.com/HNAPExt/GetClientInfo
 */
HDK_ClientError HDK_ClientMethod_Cisco_GetClientInfo(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct_Init(&sInput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_GetClientInfo, &sInput, HDK_Element_Cisco_GetClientInfoResponse, pOutput, HDK_Element_Cisco_GetClientInfoResult, pResult);
    HDK_Struct_Free(&sInput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/GetExtenderStatus
 */
HDK_ClientError HDK_ClientMethod_Cisco_GetExtenderStatus(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct_Init(&sInput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_GetExtenderStatus, &sInput, HDK_Element_Cisco_GetExtenderStatusResponse, pOutput, HDK_Element_Cisco_GetExtenderStatusResult, pResult);
    HDK_Struct_Free(&sInput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetDoRestart
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetDoRestart(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sInput);
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetDoRestart, &sInput, HDK_Element_Cisco_SetDoRestartResponse, &sOutput, HDK_Element_Cisco_SetDoRestartResult, pResult);
    HDK_Struct_Free(&sInput);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetRadios
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetRadios(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pInput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetRadios, pInput, HDK_Element_Cisco_SetRadiosResponse, &sOutput, HDK_Element_Cisco_SetRadiosResult, pResult);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetSSIDSettings
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetSSIDSettings(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pInput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetSSIDSettings, pInput, HDK_Element_Cisco_SetSSIDSettingsResponse, &sOutput, HDK_Element_Cisco_SetSSIDSettingsResult, pResult);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetTOD
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetTOD(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pInput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetTOD, pInput, HDK_Element_Cisco_SetTODResponse, &sOutput, HDK_Element_Cisco_SetTODResult, pResult);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetWPS
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetWPS(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pInput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetWPS, pInput, HDK_Element_Cisco_SetWPSResponse, &sOutput, HDK_Element_Cisco_SetWPSResult, pResult);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/GetLEDStatus
 */
HDK_ClientError HDK_ClientMethod_Cisco_GetLEDStatus(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct_Init(&sInput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_GetLEDStatus, &sInput, HDK_Element_Cisco_GetLEDStatusResponse, pOutput, HDK_Element_Cisco_GetLEDStatusResult, pResult);
    HDK_Struct_Free(&sInput);
    return error;
}

/*
 * http://cisco.com/HNAPExt/SetLEDs
 */
HDK_ClientError HDK_ClientMethod_Cisco_SetLEDs(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pInput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_Cisco_SetLEDs, pInput, HDK_Element_Cisco_SetLEDsResponse, &sOutput, HDK_Element_Cisco_SetLEDsResult, pResult);
    HDK_Struct_Free(&sOutput);
    return error;
}

/*
 * http://purenetworks.com/HNAP1/GetDeviceSettings
 */
HDK_ClientError HDK_ClientMethod_PN_GetDeviceSettings(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct_Init(&sInput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_PN_GetDeviceSettings, &sInput, HDK_Element_PN_GetDeviceSettingsResponse, pOutput, HDK_Element_PN_GetDeviceSettingsResult, pResult);
    HDK_Struct_Free(&sInput);
    return error;
}

/*
 * http://purenetworks.com/HNAP1/GetWLanRadios
 */
HDK_ClientError HDK_ClientMethod_PN_GetWLanRadios(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Struct* pOutput, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct_Init(&sInput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_PN_GetWLanRadios, &sInput, HDK_Element_PN_GetWLanRadiosResponse, pOutput, HDK_Element_PN_GetWLanRadiosResult, pResult);
    HDK_Struct_Free(&sInput);
    return error;
}

/*
 * http://purenetworks.com/HNAP1/IsDeviceReady
 */
HDK_ClientError HDK_ClientMethod_PN_IsDeviceReady(HDK_ClientContext* pClientCtx, int iTimeoutSecs, HDK_Enum_Result* pResult)
{
    HDK_ClientError error;
    HDK_Struct sInput;
    HDK_Struct sOutput;
    HDK_Struct_Init(&sInput);
    HDK_Struct_Init(&sOutput);
    error = HDK_Client__MethodHelper__(pClientCtx, iTimeoutSecs, HDK_Element_PN_IsDeviceReady, &sInput, HDK_Element_PN_IsDeviceReadyResponse, &sOutput, HDK_Element_PN_IsDeviceReadyResult, pResult);
    HDK_Struct_Free(&sInput);
    HDK_Struct_Free(&sOutput);
    return error;
}
