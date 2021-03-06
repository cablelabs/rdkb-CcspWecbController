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

#ifndef __HDK_LOGGING_H__
#define __HDK_LOGGING_H__

typedef enum _HDK_LogCategory
{
    HDK_LogCategory_General,  /* logging related to general HDK issues */
    HDK_LogCategory_HTTP,     /* logging related to the HTTP and lower layers */
    HDK_LogCategory_XML,      /* logging related to XML (e.g. parsing errors) */
    HDK_LogCategory_SOAP,     /* logging related to the SOAP protocol (e.g. SOAP faults) */
    HDK_LogCategory_HNAP      /* logging related to the HNAP protocol (e.g. schema errors) */
} HDK_LogCategory;

typedef enum _HDK_LogLevel
{
    HDK_LogLevel_Error,
    HDK_LogLevel_Warning,
    HDK_LogLevel_Info,
    HDK_LogLevel_Verbose
} HDK_LogLevel;

#ifdef HDK_LOGGING

extern void HDK_Log(HDK_LogCategory category, HDK_LogLevel level, const char* pszFormat, ...);

#  define HDK_LOG(_category, _level, _str) \
  HDK_Log(_category, _level, _str)

#  define HDK_LOGFMT(_category, _level, _str, ...) \
  HDK_Log(_category, _level, _str, __VA_ARGS__)

#else /* ndef HDK_LOGGING */

#  define HDK_LOG(_category, _level, _str) (void)0

#  define HDK_LOGFMT(_category, _level, _str, ...) (void)0

#endif /* def HDK_LOGGING */


#endif /* __HDK_LOGGING_H__ */
