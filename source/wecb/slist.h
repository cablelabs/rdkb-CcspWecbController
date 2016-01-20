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

/**********************************************************************

    module:	slist.h

    ---------------------------------------------------------------

    copyright:

        Cisco Systems, Inc., 1997 ~ 2002
        All Rights Reserved.

    ---------------------------------------------------------------

    description:

        This wrapper file defines all the platform-independent
        common data structures.

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Xuechen Yang

**********************************************************************/
#ifndef _SLIST_H
#define _SLIST_H

#include "ixml.h"
#include <stdio.h>

typedef  struct
_SINGLE_LINK_ENTRY
{
    struct  _SINGLE_LINK_ENTRY*     Next;
}
SINGLE_LINK_ENTRY,  *PSINGLE_LINK_ENTRY;

typedef  struct
_SLIST_HEADER
{
    SINGLE_LINK_ENTRY               Next;
    unsigned short                  Depth;
    unsigned short                  Sequence;
}
SLIST_HEADER,  *PSLIST_HEADER;

#define  AnscSListInitializeHeader(ListHead)                                                \
         {                                                                                  \
            (ListHead)->Next.Next = NULL;                                                   \
            (ListHead)->Depth     = 0;                                                      \
            (ListHead)->Sequence  = 0;                                                      \
         }

#define  AnscSListPushEntry(ListHead, Entry)                                                \
         {                                                                                  \
            (Entry)->Next         = (ListHead)->Next.Next;                                  \
            (ListHead)->Next.Next = (Entry);                                                \
            (ListHead)->Depth++;                                                            \
         }

#define  AnscSListGetFirstEntry(ListHead)                       (ListHead)->Next.Next
#define  AnscSListGetNextEntry(Entry)                           (Entry)->Next

#define  ACCESS_CONTAINER(address, type, field)     \
         ((type*)((char *)(address) - (unsigned long)(&((type*)0)->field)))

#define  AnscSListQueryDepth(ListHead)                          (ListHead)->Depth

BOOL
AnscSListPopEntryByLink
    (
        PSLIST_HEADER               ListHead,
        PSINGLE_LINK_ENTRY          Entry
    );

#endif
