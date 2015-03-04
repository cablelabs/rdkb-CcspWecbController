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

/**************************************************************************

    module: wecb_upnp_agent.c

        For WECB control development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implements ssdp discovery of wecb device.

    -------------------------------------------------------------------


    author:

        Rongwei

    -------------------------------------------------------------------

    revision:

        11/02/2012    initial revision.

**************************************************************************/

#ifndef _WECB_UPNP_AGENT_H
#define _WECB_UPNP_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wecb_log.h"
#include "wecb_util.h"
#include "pthread.h"
#include "upnp.h"
#include "wecb_hnap.h"
#include "hnap/hnap_device.h"
#include "string.h"

#include <stdio.h>

int WECB_UPnPAgentStart(char * ip_address, char *lan_if);
int WECB_UPnPAgentStop();

//ssdp discovery max response time, should between MIN_SEARCH_TIME 
//adn MAX_SEARCH_TIME in libupnp 
#define WECB_MX 10

void* WECB_SSDP_DiscoveryAll(void *);
void* WECB_SSDP_CheckAllTimer(void *);
void wecb_sig_handler(int signal);

#ifdef __cplusplus
};
#endif

#endif

