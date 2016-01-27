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
 * hdk_client_http_curl.c
 *
 *    libcURL-based implementation of hdk_client_http_interface.h
 */
#include "hdk_client_http_interface.h"

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>


#define CURL_SUCCEEDED(_code) (CURLE_OK == _code)
#define CURL_FAILED(_code) (!CURL_SUCCEEDED(_code))

typedef struct _CURLRequestContext
{
    CURL* pCURL;
    struct curl_slist* pHeaderList;
    int fGet;
    char* pData;
    size_t cbData;
    size_t cbAllocated;
	char *pAuth;
} CURLRequestContext;


/*
 * hdk_client_http_interface.h
 */

int HDK_Client_Http_Init(void)
{
    CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

    return CURL_SUCCEEDED(code);
}

void HDK_Client_Http_Cleanup(void)
{
    curl_global_cleanup();
}

void* HDK_Client_Http_RequestCreate(const char* pszURL, int fGet)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)malloc(sizeof(CURLRequestContext));
    if (pCtx)
    {
        memset(pCtx, 0, sizeof(*pCtx));

        pCtx->fGet = !!fGet;

        pCtx->pCURL = curl_easy_init();
        if (pCtx->pCURL)
        {
            do
            {
                /* Set the request URL and HTTP action */
                CURLcode code = curl_easy_setopt(pCtx->pCURL, CURLOPT_URL, pszURL);
                if (CURL_FAILED(code))
                {
                    break;
                }

                code = curl_easy_setopt(pCtx->pCURL, CURLOPT_POST, !(pCtx->fGet));
                if (CURL_FAILED(code))
                {
                    break;
                }

                code = curl_easy_setopt(pCtx->pCURL, CURLOPT_HTTPGET, pCtx->fGet);
                if (CURL_FAILED(code))
                {
                    break;
                }

                return pCtx;
            }
            while (0);
        }
    }

    HDK_Client_Http_RequestDestroy(pCtx);

    return 0;
}

void HDK_Client_Http_RequestDestroy(void* pRequestCtx)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;
    if (pCtx)
    {
        curl_slist_free_all(pCtx->pHeaderList);
        curl_easy_cleanup(pCtx->pCURL);

        free(pCtx->pData);
        free(pCtx->pAuth);
    }

    free(pCtx);
}

int HDK_Client_Http_RequestAddHeader(void* pRequestCtx, const char* pszHeader)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;
    pCtx->pHeaderList = curl_slist_append(pCtx->pHeaderList, pszHeader);

    return (0 != pCtx->pHeaderList);
}

int HDK_Client_Http_RequestSetTimeout(void* pRequestCtx, unsigned int uiTimeoutMsecs)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;
/* Mac OS X projects MUST define this __MAC_OSX__ macro to compile */
#ifdef __MAC_OSX__
    if (uiTimeoutMsecs > 0)
    {
        uiTimeoutMsecs = uiTimeoutMsecs / 1000; // Make it seconds!
        if (uiTimeoutMsecs == 0)
            uiTimeoutMsecs = 1; // everything from 1 to 999 milliseconds rounds up to 1 second!
    }
    CURLcode code = curl_easy_setopt(pCtx->pCURL, CURLOPT_TIMEOUT, uiTimeoutMsecs);
#else
    CURLcode code = curl_easy_setopt(pCtx->pCURL, CURLOPT_TIMEOUT_MS, uiTimeoutMsecs);
#endif
	if(CURL_FAILED(code))
	{
		return CURL_FAILED(code);
	}
	//disable signal in libcurl to make sure thread safe
	code =curl_easy_setopt(pCtx->pCURL, CURLOPT_NOSIGNAL, 1);

    return CURL_SUCCEEDED(code);
}

int HDK_Client_Http_RequestSetBasicAuth(void* pRequestCtx,
                                        const char* pszUsername, const char* pszPassword)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;

    CURLcode code;
    char* pszUserPwd = 0;
    do
    {
        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        if (CURL_FAILED(code))
        {
            break;
        }
		int tmp = strlen(pszUsername) + strlen(pszPassword) + 2;
        pszUserPwd = (char*)malloc(tmp);

        if (!pszUserPwd)
        {
            code = CURLE_OUT_OF_MEMORY;
            break;
        }
		pCtx->pAuth = pszUserPwd;
        sprintf(pszUserPwd, "%s:%s",
                (pszUsername) ? pszUsername : "",
                (pszPassword) ? pszPassword : "");

        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_USERPWD, pszUserPwd);
        if (CURL_FAILED(code))
        {
            break;
        }
    }
    while (0);

    return CURL_SUCCEEDED(code);
}

void HDK_Client_Http_RequestAppendData(void* pRequestCtx, char* pData, int cbData)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;
    size_t cbDataNew = pCtx->cbData + (size_t)cbData;

    /* Grow the buffer as needed... */
    if (cbDataNew > pCtx->cbAllocated)
    {
        size_t cbNewAlloc = 2 * pCtx->cbAllocated;
        if (cbDataNew > cbNewAlloc)
        {
            cbNewAlloc = cbDataNew;
        }
        pCtx->pData = realloc(pCtx->pData, cbNewAlloc);
        if (pCtx->pData)
        {
            pCtx->cbAllocated = cbNewAlloc;
        }
    }
    if (pCtx->pData)
    {
        memcpy(&pCtx->pData[pCtx->cbData], pData, cbData);
        pCtx->cbData += cbData;
    }
}

typedef struct _ReadHeaderContext
{
    HDK_Client_Http_ReadHeaderFn pfnReadHeader;
    void* pReadHeaderCtx;
} ReadHeaderContext;

typedef struct _ReadContext
{
    HDK_Client_Http_ReadFn pfnRead;
    void* pReadCtx;
} ReadContext;

static size_t WriteHeader_Callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    ReadHeaderContext* pCtx = (ReadHeaderContext*)stream;
    return (size_t)pCtx->pfnReadHeader(pCtx->pReadHeaderCtx, (char*)ptr, size * nmemb);
}

static size_t Write_Callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    ReadContext* pCtx = (ReadContext*)stream;
    return (size_t)pCtx->pfnRead(pCtx->pReadCtx, (char*)ptr, size * nmemb);
}

int HDK_Client_Http_RequestSend(void* pRequestCtx,
                                HDK_Client_Http_ReadHeaderFn pfnReadHeader, void* pReadHeaderCtx,
                                HDK_Client_Http_ReadFn pfnRead, void* pReadCtx)
{
    CURLRequestContext* pCtx = (CURLRequestContext*)pRequestCtx;

    long httpResponseCode = -1;
    do
    {
        CURLcode code;
        ReadHeaderContext readHeaderCtx;
        ReadContext readCtx;

        readHeaderCtx.pfnReadHeader = pfnReadHeader;
        readHeaderCtx.pReadHeaderCtx = pReadHeaderCtx;

        readCtx.pfnRead = pfnRead;
        readCtx.pReadCtx = pReadCtx;

        /* Add the POST data (if POSTing) */
        if (!pCtx->fGet)
        {
            code = curl_easy_setopt(pCtx->pCURL, CURLOPT_POSTFIELDSIZE, pCtx->cbData);
            if (CURL_FAILED(code))
            {
                break;
            }

            code = curl_easy_setopt(pCtx->pCURL, CURLOPT_POSTFIELDS, pCtx->pData);
            if (CURL_FAILED(code))
            {
                break;
            }
        }

        /* Set the read headers callback. */
        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_WRITEHEADER, &readHeaderCtx);
        if (CURL_FAILED(code))
        {
            break;
        }

        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_HEADERFUNCTION, WriteHeader_Callback);
        if (CURL_FAILED(code))
        {
          break;
        }

        /* Set the read callback. */
        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_WRITEDATA, &readCtx);
        if (CURL_FAILED(code))
        {
            break;
        }

        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_WRITEFUNCTION, Write_Callback);
        if (CURL_FAILED(code))
        {
          break;
        }
	//let curl not verify server's cert	
	code = curl_easy_setopt(pCtx->pCURL, CURLOPT_SSL_VERIFYPEER, 0);
	if (CURL_FAILED(code))
        {
            break;
	}

        //set ipv6 interface for curl link local HNAP
        char lan_if[64];
        int rc;
        strcpy(lan_if, "if!");
        rc = syscfg_get(NULL, "lan_ifname", lan_if + strlen(lan_if), sizeof(lan_if) - strlen(lan_if));
        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_INTERFACE, lan_if);
        if (rc || CURL_FAILED(code))
        {
            break;
        }

        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
        if (CURL_FAILED(code))
        {
            break;
        }

        code = curl_easy_setopt(pCtx->pCURL, CURLOPT_HTTPHEADER, pCtx->pHeaderList);
        if (CURL_FAILED(code))
        {
            break;
	}

        /*
         * If the client-supplied callbacks aborted the operation (and we get CURLE_WRITE_ERROR),
         * continue on so we can still return the correct HTTP response code.
         */
        code = curl_easy_perform(pCtx->pCURL);
        if (CURL_FAILED(code) && (CURLE_WRITE_ERROR != code))
        {
            break;
        }

        code = curl_easy_getinfo(pCtx->pCURL, CURLINFO_RESPONSE_CODE, &httpResponseCode);
        if (CURL_FAILED(code))
        {
            break;
        }
    }
    while (0);

    return (int)httpResponseCode;
}
