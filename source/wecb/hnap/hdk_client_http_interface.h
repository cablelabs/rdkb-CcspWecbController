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

#ifndef __HDK_CLIENT_HTTP_INTERFACE_H__
#define __HDK_CLIENT_HTTP_INTERFACE_H__

/* Initialization and cleanup of HTTP libs */
extern int HDK_Client_Http_Init(void);
void HDK_Client_Http_Cleanup();

/* Create an HTTP request context object */
extern void* HDK_Client_Http_RequestCreate(const char* pszURL, int fGet);

/* Destroy an HTTP request context object */
extern void HDK_Client_Http_RequestDestroy(void* pRequestCtx);

/* Add a header to an HTTP request */
extern int HDK_Client_Http_RequestAddHeader(void* pRequestCtx, const char* pszHeader);

/* Set the timeout */
extern int HDK_Client_Http_RequestSetTimeout(void* pRequestCtx, unsigned int uiTimeoutMsecs);

/* Set HTTP Basic Auth */
extern int HDK_Client_Http_RequestSetBasicAuth(void* pRequestCtx,
                                               const char* pszUsername, const char* pszPassword);

/* Append data to a request */
extern void HDK_Client_Http_RequestAppendData(void* pRequestCtx, char* pData, int cbData);

/*
 * Callbacks for handling responses.
 * The return value is the number of bytes handled.  Returning fewer than cbData bytes
 * will cause the request to be aborted, though the response code is still returned.
 */
typedef unsigned int (*HDK_Client_Http_ReadHeaderFn)(void* pCtx, char* pData, unsigned int cbData);
typedef unsigned int (*HDK_Client_Http_ReadFn)(void* pCtx, char* pData, unsigned int cbData);

/* Send an HTTP request.  Returns the HTTP response code or -1 on error. */
extern int HDK_Client_Http_RequestSend(void* pRequestCtx,
                                       HDK_Client_Http_ReadHeaderFn pfnReadHeader, void* pReadHeaderCtx,
                                       HDK_Client_Http_ReadFn pfnRead, void* pReadCtx);

#endif /* __HDK_CLIENT_HTTP_INTERFACE_H__ */
