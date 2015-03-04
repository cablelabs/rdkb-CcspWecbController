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

#ifndef __HDK_ENCODE_H__
#define __HDK_ENCODE_H__

#include "hdk_internal.h"

/* Encoding output functions */
typedef int (*HDK_EncodeFn)(void* pEncodeCtx, HDK_WriteFn pfnWrite, char* pBuf, unsigned int cbBuf);
extern int HDK_EncodeToBuffer(void* pEncodeCtx, HDK_WriteFn pfnWrite, char* pBuf, unsigned int cbBuf);

/* Encoding/decoding utilities */
extern int HDK_EncodeString(HDK_EncodeFn pfnEncode, void* pEncodeCtx, HDK_WriteFn pfnWrite, char* pBuf, unsigned int cbBuf);
extern int HDK_EncodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, HDK_WriteFn pfnWrite, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev);
extern int HDK_EncodeBase64Done(HDK_EncodeFn pfnEncode, void* pEncodeCtx, HDK_WriteFn pfnWrite, int state, int prev);
extern int HDK_DecodeBase64(HDK_EncodeFn pfnEncode, void* pEncodeCtx, char* pBuf, unsigned int cbBuf, int* pState, int* pPrev);
extern int HDK_DecodeBase64Done(int state);

#endif /* __HDK_ENCODE_H__ */
