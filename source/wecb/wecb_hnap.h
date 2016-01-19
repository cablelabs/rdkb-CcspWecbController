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

/**************************************************************************

    module: wecb_hnap.h

        For WECB control development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

		This file implements wecb HNAP device manipulation.

    -------------------------------------------------------------------


    author:

        Rongwei

    -------------------------------------------------------------------

    revision:

        11/02/2012    initial revision.

**************************************************************************/
#ifndef _WECB_HNAP_H
#define _WECB_HNAP_H


#include "ixml.h"
#include "slist.h" 
#include "string.h" 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include "wecb_util.h"
#include <stdbool.h>


typedef  struct
_HnapDevice
{
	SINGLE_LINK_ENTRY		Linkage;
	unsigned int	sync_mask;      //this mask used for sync, the settings pushed down are determinded by this mask
	unsigned int	notify_mask;    //this mask used to save notifications such as SSID changed, periodically time sync
	int v4_expires;            //ipv4 ssdp advertisement expired time
	int v6_expires;            //ipv6 ssdp advertisement expired time

	char location[MAX_STRING_LEN];        //Device description Document url
	char device_id[MAX_STRING_LEN];       //unique id
	char device_type[MAX_STRING_LEN];	  //ssdp device type
	bool is_syncing;
	bool restart_pending;
	int  bridge_ins[HS_SSID_NUM]; // for AP isolation used, only supports Hotspot
	int  pvid[HS_SSID_NUM];
	pthread_t thread;

    char addr[32];  /* IP Address from d_event->DestAddr.sin_addr*/
    char addr6[160];  /* IP v6 Address from d_event->DestAddr.sin6_addr*/
}
HnapDevice,  *PHnapDevice;

#define  ACCESS_HNAPDEVICESTRUCT(p)                         \
         ACCESS_CONTAINER(p, HnapDevice, Linkage)



PHnapDevice Hnap_FindDeviceByUUID(char* uuid);
PHnapDevice Hnap_AddDeviceByUUID(char* uuid);
int Hnap_DelDeviceByUUID(char* uuid);
void Hnap_DelAllDevice();
void Hnap_CheckAllDeviceAlive(int interval);
void Hnap_InitDeviceList();
int Hnap_GetDeviceNumber();
void Hnap_UpdateAllDevice_NotifyMask(int mask);
bool Hnap_GetAllDevice(char ***p, int *num);
int get_device_bridge_ins(char *uuid, int ofs);
int get_device_pvid(char *uuid, int ofs);
#endif
