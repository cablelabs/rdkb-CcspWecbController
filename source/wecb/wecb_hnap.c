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

    module: wecb_hnap.c

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
#include "wecb_hnap.h"
#include "wecb_log.h"
#include "wecb_common.h"
#include "hnap/hnap_device.h"
#include "autoconf.h"

static SLIST_HEADER	ssdp_device_list;
//static pthread_mutex_t device_list_mutex		= PTHREAD_MUTEX_INITIALIZER; 

void Hnap_InitDeviceList()
{
	AnscSListInitializeHeader(&ssdp_device_list);
}


int Hnap_GetDeviceNumber()
{
	return AnscSListQueryDepth(&ssdp_device_list);
}

PHnapDevice Hnap_FindDeviceByUUID(char* uuid)
{
	PSINGLE_LINK_ENTRY          pSListEntry;
	PHnapDevice					device = NULL;

	if(uuid == NULL)
	{
		return NULL;
	}
	pSListEntry =   AnscSListGetFirstEntry(&ssdp_device_list);

	while( pSListEntry != NULL )
	{
		device = ACCESS_HNAPDEVICESTRUCT(pSListEntry);
		if ( device != NULL )
		{
			//Got the device match to the specific uuid
			if ( !strcmp(device->device_id, uuid) )
			{
				log_printf(LOG_INFO, "Got the device with uuid: %s\n", uuid);
				return device;
			}
		}
		pSListEntry = AnscSListGetNextEntry(pSListEntry);
	}

    return NULL;
}


PHnapDevice Hnap_AddDeviceByUUID(char* uuid)
{
	if(uuid == NULL)
	{
		return NULL;
	}
    PHnapDevice pDevice = NULL;//Hnap_FindDeviceByUUID(uuid);
    
	pDevice = (PHnapDevice)malloc(sizeof(HnapDevice));

	if(pDevice == NULL)
	{
		log_printf(LOG_ERR, "Add new hnap device failed\n");
		return NULL;
	}
	memset(pDevice, 0, sizeof(HnapDevice));
	//When device first up, we need to push all settings down
	pDevice->sync_mask = 0xFFFFFFFF;
	//Boot up firstly, there's no notification
	pDevice->notify_mask = 0x0;
	pDevice->restart_pending = false;
	//first added, no syncing process
	pDevice->is_syncing = false;
	strncpy(pDevice->device_id, uuid, MAX_STRING_LEN);
	AnscSListPushEntry(&ssdp_device_list, &pDevice->Linkage);
	return pDevice;	
}

int Hnap_DelDeviceByUUID(char* uuid)
{
    PHnapDevice pDevice = Hnap_FindDeviceByUUID(uuid);
    
	if(pDevice == NULL)
	{
		return 0;
	}
	//pthread_cancel(pDevice->thread);
	AnscSListPopEntryByLink(&ssdp_device_list, &pDevice->Linkage);
	free(pDevice);
	pDevice = NULL;
	return 0;	
}


void Hnap_DelAllDevice()
{
	PSINGLE_LINK_ENTRY          pSListEntry;
	PHnapDevice					pDevice = NULL;
	int                         i = 0;

	pSListEntry =   AnscSListGetFirstEntry(&ssdp_device_list);

	while( pSListEntry != NULL )
	{
		pDevice = ACCESS_HNAPDEVICESTRUCT(pSListEntry);
		pSListEntry = AnscSListGetNextEntry(pSListEntry);

		if ( pDevice != NULL )
		{
			//pthread_cancel(pDevice->thread);
			AnscSListPopEntryByLink(&ssdp_device_list, &pDevice->Linkage);
#ifdef CONFIG_CISCO_HOTSPOT
			for(i = 0; i < HS_SSID_NUM; i++)
			{
				disable_specific_bridge(pDevice->bridge_ins[i]);
			}
			memset(pDevice->bridge_ins, 0, sizeof(pDevice->bridge_ins));
			memset(pDevice->pvid, 0, sizeof(pDevice->pvid));
#endif
			free(pDevice);
			pDevice = NULL;
		}
	}
}

void Hnap_CheckAllDeviceAlive(int interval)
{
	PSINGLE_LINK_ENTRY          pSListEntry;
	PHnapDevice					pDevice = NULL;
	int							i = 0;

	pSListEntry =   AnscSListGetFirstEntry(&ssdp_device_list);

	while( pSListEntry != NULL )
	{
		pDevice = ACCESS_HNAPDEVICESTRUCT(pSListEntry);
		pSListEntry = AnscSListGetNextEntry(pSListEntry);

		if ( pDevice != NULL )
		{
			pDevice->v4_expires -= interval;
			pDevice->v6_expires -= interval;

			if(pDevice->v4_expires <= 0 && pDevice->v6_expires <= 0)
			{
#ifdef CONFIG_CISCO_HOTSPOT
				for(i = 0; i < HS_SSID_NUM; i++)
				{
					disable_specific_bridge(pDevice->bridge_ins[i]);
				}
				memset(pDevice->bridge_ins, 0, sizeof(pDevice->bridge_ins));
				memset(pDevice->pvid, 0, sizeof(pDevice->pvid));
#endif
				log_printf(LOG_INFO, "Device: %s is dead without byebye message received\n", pDevice->device_id);
				AnscSListPopEntryByLink(&ssdp_device_list, &pDevice->Linkage);
				free(pDevice);
				pDevice = NULL;
			}
			else if(pDevice->v4_expires <= 0)
			{
				memset(pDevice->addr, 0 , sizeof(pDevice->addr));
				//printf("v4 device %s expire\n", pDevice->addr);
			}
			else if(pDevice->v6_expires <= 0)
			{
				//printf("v6 device %s expire\n", pDevice->addr6);
				memset(pDevice->addr6, 0 , sizeof(pDevice->addr6));
			}
		}
	}
}

void Hnap_UpdateAllDevice_NotifyMask(int mask)
{
	PSINGLE_LINK_ENTRY          pSListEntry;
	PHnapDevice					pDevice = NULL;

	pSListEntry =   AnscSListGetFirstEntry(&ssdp_device_list);
	log_printf(LOG_WARNING, "update all device notify mask with %x\n", mask);

	while( pSListEntry != NULL )
	{
		pDevice = ACCESS_HNAPDEVICESTRUCT(pSListEntry);
		pSListEntry = AnscSListGetNextEntry(pSListEntry);

		if ( pDevice != NULL )
		{
			pDevice->notify_mask |= mask;
		}
	}
}


bool Hnap_GetAllDevice(char ***alive_device, int *device_num)
{
	PSINGLE_LINK_ENTRY          pSListEntry;
	PHnapDevice					pDevice = NULL;
	char						*tmp = NULL;
	int							i = 0;
	
	*device_num = Hnap_GetDeviceNumber();
	if(*device_num == 0)
	{
		log_printf(LOG_INFO, "no device online\n");
		return false;
	}

	*alive_device = calloc(*device_num, sizeof(char **));

	if(*alive_device == NULL)
	{
		log_printf(LOG_ERR, "memory error");
		return false;
	}

	pSListEntry =   AnscSListGetFirstEntry(&ssdp_device_list);

	while( pSListEntry != NULL )
	{
		pDevice = ACCESS_HNAPDEVICESTRUCT(pSListEntry);
		pSListEntry = AnscSListGetNextEntry(pSListEntry);

		//printf("syncing is %d\n", pDevice->is_syncing);

		if (pDevice != NULL)
		{
			tmp = (char *)malloc(strlen(pDevice->device_id) + 1);
			if(tmp != NULL)
			{
				strcpy(tmp, pDevice->device_id);
				printf("Device %s is in list\n", pDevice->device_id);
				(*alive_device)[i] = tmp;
				i++;
				if(i >= *device_num)
				{
					break;
				}
			}
		}
	}
	return true;
}

int get_device_pvid(char *uuid, int ofs)
{
	if (ofs >= HS_SSID_NUM)
	{
		log_printf(LOG_ERR, "illegal input\n");
		return 0;
	}
	PHnapDevice pDevice = Hnap_FindDeviceByUUID(uuid);
    
	if(pDevice == NULL)
	{
		return 0;
	}
	
	return pDevice->pvid[ofs];
}

int get_device_bridge_ins(char *uuid, int ofs)
{
	if (ofs >= HS_SSID_NUM)
	{
		log_printf(LOG_ERR, "illegal input\n");
		return 0;
	}
	PHnapDevice pDevice = Hnap_FindDeviceByUUID(uuid);
    
	if(pDevice == NULL)
	{
		return 0;
	}
	
	return pDevice->bridge_ins[ofs];
}
