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
#define HDK_LIBXML2
#include "hdk.h"
#include "hdk_internal.h"

#ifdef HDK_LIBXML2
#include <libxml/parser.h>
#else
#include <expat.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * I/O helper functions
 */

/* Stream buffer-write function */
int HDK_WriteBuf(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, char* pBuf, int cbBuf)
{
    if (!fNoWrite)
    {
        pfnWrite(pDeviceCtx, pBuf, cbBuf);
    }
    return cbBuf;
}

/* Encoded write function */
int HDK_WriteBuf_Encode(void* pEncodeCtx, HDK_WriteFn pfnWrite, char* pBuf, unsigned int cbBuf)
{
    HDK_WriteBuf_EncodeContext* pCtx = (HDK_WriteBuf_EncodeContext*)pEncodeCtx;
    return HDK_WriteBuf(pCtx->pDeviceCtx, pfnWrite, pCtx->fNoWrite, pBuf, cbBuf);
}

/* Stream null-terminated string write function */
int HDK_Write(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, char* pszStr)
{
    int cbStr = strlen(pszStr);
    if (!fNoWrite)
    {
        pfnWrite(pDeviceCtx, pszStr, cbStr);
    }
    return cbStr;
}

/* Stream formatted write function */
int HDK_Format(void* pDeviceCtx, HDK_WriteFn pfnWrite, int fNoWrite, char* pszStr, ...)
{
    va_list args;
    int cbBuf;
    char szBuf[128];

    va_start(args, pszStr);

    /* Format the string to the buffer */
    cbBuf = vsprintf(szBuf, pszStr, args);
    if (cbBuf < 0)
    {
        cbBuf = 0;
    }
    else if (!fNoWrite)
    {
        pfnWrite(pDeviceCtx, szBuf, cbBuf);
    }

    va_end(args);

    return cbBuf;
}


/*
 * XML parsing
 */

#ifdef HDK_LIBXML2
static void ElementStartHandler(void* pDeviceCtx,
                                const xmlChar* pszElement,
                                const xmlChar* pszPrefix,
                                const xmlChar* pszNamespace,
                                int nNamespaces,
                                const xmlChar** ppszNamespaces,
                                int nAttributes,
                                int nDefaulted,
                                const xmlChar** ppszAttributes)
#else
static void ElementStartHandler(void* pDeviceCtx,
                                const char* pszElement,
                                const char** ppszAttributes)
#endif
{
    HDK_ParseContext* pParseCtx;
    char* pszNamespaceEnd = 0;

#ifdef HDK_LIBXML2
    /* Unused parameters */
    (void)pszPrefix;
    (void)nNamespaces;
    (void)ppszNamespaces;
    (void)nAttributes;
    (void)nDefaulted;
    (void)ppszAttributes;
#else
    char* pszNamespace;

    /* Unused parameters */
    (void)ppszAttributes;

    /* Locate the end of the namespace */
    pszNamespace = (char*)pszElement;
    for (pszNamespaceEnd = (char*)pszElement; *pszNamespaceEnd != '!'; ++pszNamespaceEnd);
    pszElement = pszNamespaceEnd + 1;
#endif

    /* Handle the element open */
    pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementOpen(pParseCtx, (char*)pszNamespace, pszNamespaceEnd, (char*)pszElement, 0);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser(pParseCtx->pXMLParser);
#else
        XML_StopParser(pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

#ifdef HDK_LIBXML2
static void ElementEndHandler(void* pDeviceCtx,
                              const xmlChar* pszElement,
                              const xmlChar* pszPrefix,
                              const xmlChar* pszNamespace)
#else
static void ElementEndHandler(void* pDeviceCtx,
                              const char* pszElement)
#endif
{
    HDK_ParseContext* pParseCtx;

#ifdef HDK_LIBXML2
    /* Unused parameters */
    (void)pszElement;
    (void)pszPrefix;
    (void)pszNamespace;
#else
    /* Unused parameters */
    (void)pszElement;
#endif

    /* Handle the element close */
    pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementClose(pParseCtx);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser(pParseCtx->pXMLParser);
#else
        XML_StopParser(pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

#ifdef HDK_LIBXML2
static void ElementValueHandler(void* pDeviceCtx,
                                const xmlChar* pValue,
                                int cbValue)
#else
static void ElementValueHandler(void* pDeviceCtx,
                                const char* pValue,
                                int cbValue)
#endif
{
    /* Handle the element value (part) */
    HDK_ParseContext* pParseCtx = (HDK_ParseContext*)pDeviceCtx;
    HDK_Parse_ElementValue(pParseCtx, (char*)pValue, cbValue);

    /* Stop the parser on error */
    if (pParseCtx->parseError != HDK_ParseError_OK)
    {
#ifdef HDK_LIBXML2
        xmlStopParser(pParseCtx->pXMLParser);
#else
        XML_StopParser(pParseCtx->pXMLParser, XML_FALSE);
#endif
    }
}

HDK_ParseError HDK_ParseBegin(void* pDeviceCtx, HDK_ParseContext* pParseCtx, HDK_Struct* pStruct)
{
#ifdef HDK_LIBXML2
    xmlSAXHandler saxHandler;
#endif

    /* "New" the parse context */
    HDK_Parse_Init(pParseCtx, pStruct, pDeviceCtx);

    /* Create the XML parser */
#ifdef HDK_LIBXML2
    memset(&saxHandler, 0, sizeof(saxHandler));
    saxHandler.initialized = XML_SAX2_MAGIC;
    saxHandler.startElementNs = ElementStartHandler;
    saxHandler.endElementNs = ElementEndHandler;
    saxHandler.characters = ElementValueHandler;

    pParseCtx->pXMLParser = xmlCreatePushParserCtxt(&saxHandler, pParseCtx, 0, 0, 0);
#else
    pParseCtx->pXMLParser = XML_ParserCreateNS(NULL, '!');
#endif
    if (pParseCtx->pXMLParser)
    {
#ifndef HDK_LIBXML2
        XML_SetUserData(pParseCtx->pXMLParser, pParseCtx);
        XML_SetElementHandler(pParseCtx->pXMLParser, ElementStartHandler, ElementEndHandler);
        XML_SetCharacterDataHandler(pParseCtx->pXMLParser, ElementValueHandler);
#endif

        return HDK_ParseError_OK;
    }
    else
    {
        /* Error - out of memory */
        return HDK_ParseError_500_OutOfMemory;
    }
}

HDK_ParseError HDK_ParseData(HDK_ParseContext* pParseCtx, char* pData, unsigned int cbData)
{
	int fDone = 0;
	if (!cbData || pData[cbData-1] == '\0')
		fDone = 1;
	/*
	int i = 0;

	for(; i < cbData; i++)
	{
		printf("%c", pData[i]);
	}
	printf("\n");*./

    /* Parse the XML in the buffer */
#ifdef HDK_LIBXML2
    xmlParserErrors xmlErrorCode;
    if ((xmlErrorCode = xmlParseChunk(pParseCtx->pXMLParser, pData, cbData, fDone)) != XML_ERR_OK)
    {
#else
		
    if (!XML_Parse(pParseCtx->pXMLParser, pData, cbData, fDone))
    {
        enum XML_Error xmlErrorCode = XML_GetErrorCode(pParseCtx->pXMLParser);
#endif

        /* Error - XML error */
        switch (xmlErrorCode)
        {
#ifdef HDK_LIBXML2
            case XML_ERR_NO_MEMORY:
#else
            case XML_ERROR_NO_MEMORY:
#endif
                return HDK_ParseError_500_OutOfMemory;

            default:
                if (pParseCtx->parseError != HDK_ParseError_OK)
                {
                    return pParseCtx->parseError;
                }
                else
                {
                    return HDK_ParseError_500_XMLInvalid;
                }
                break;
        }
    }
    else if (pParseCtx->parseError != HDK_ParseError_OK)
    {
        return pParseCtx->parseError;
    }

    if (fDone && !pParseCtx->fHaveInput)
    {
        /* No input? */
        return HDK_ParseError_500_NoInput;
    }
    return HDK_ParseError_OK;
}

void HDK_ParseEnd(HDK_ParseContext* pParseCtx)
{
    /* Free the XML parser */
    if (pParseCtx->pXMLParser)
    {
#ifdef HDK_LIBXML2
		xmlDocPtr doc = ((xmlParserCtxtPtr)pParseCtx->pXMLParser)->myDoc;  
		if(doc)
			xmlFreeDoc(doc);
        xmlFreeParserCtxt(pParseCtx->pXMLParser);
        //xmlCleanupParser();
#else
        XML_ParserFree(pParseCtx->pXMLParser);
#endif
    }

    /* Free the parse context */
    HDK_Parse_Free(pParseCtx);
}

/* Parse the HNAP request */
HDK_ParseError HDK_Parse(void* pDeviceCtx, HDK_ReadFn pfnRead,
                         HDK_Struct* pStruct, unsigned int cbContentLength)
{
    HDK_ParseContext parseContext;
    HDK_ParseError parseError = HDK_ParseBegin(pDeviceCtx, &parseContext, pStruct);

    /* Parse the XML content */
    if (parseError == HDK_ParseError_OK)
    {
        char buf[1024];

        /* Compute the bytes remaining to read of the content */
        unsigned int cbRemaining = cbContentLength;
        while (cbRemaining > 0)
        {
            /* Read data... */
            int cbRead = (cbRemaining < sizeof(buf) ? cbRemaining : sizeof(buf));
            cbRead = pfnRead(pDeviceCtx, buf, cbRead);
            if (cbRead <= 0)
            {
                parseError = HDK_ParseError_500_IOError;
                break;
            }
            cbRemaining -= cbRead;

            parseError = HDK_ParseData(&parseContext, buf, cbRead);
            if (HDK_ParseError_OK != parseError)
            {
                break;
            }
        }

        /* Finish off the XML parsing. */
        if (HDK_ParseError_OK == parseError)
        {
            parseError = HDK_ParseData(&parseContext, 0, 0);
        }
    }

    /* End the data parsing. */
    HDK_ParseEnd(&parseContext);

    return parseError;
}
