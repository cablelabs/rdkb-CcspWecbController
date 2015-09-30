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
#include "wecb_upnp_agent.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "syscfg/syscfg.h"
#include "autoconf.h"

UpnpClient_Handle ctrlpt_handle = -1;
pthread_mutex_t device_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static char self_ip[32];
extern pthread_attr_t wecb_attr;
static int max_client = MAX_EXT_NUM;

/********************************************************************************
 * WECB_UPnPHandler
 *
 * Description: 
 *       Handler funciton to arbiterate the calls to real function
 *
 * Parameters:
 *   EventType -- Callback event Type
 *   Event -- Event data structure
 *   Cookie --Additional data ( optional )
 *
 ********************************************************************************/
int WECB_UPnPHandler(Upnp_EventType EventType, void *Event, void *Cookie)
{
	switch ( EventType ) {
	/* SSDP Stuff */
	case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		//printf("UPNP_DISCOVERY_ADVERTISEMENT_ALIVE call back\n");
	case UPNP_DISCOVERY_SEARCH_RESULT: {
        //if(EventType != UPNP_DISCOVERY_ADVERTISEMENT_ALIVE) printf("UPNP_DISCOVERY_SEARCH_RESULT call back.\n");
		//Receive SSDP discovery response or alive notificationm, go to auto-sync flow
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;
		int phy_port = -1;
		char wecb_ip[160], *p;
		
		if (d_event->ErrCode != UPNP_E_SUCCESS) {
         log_printf(LOG_ERR, "Error in Discovery or Alive Callback -- %d\n", d_event->ErrCode);
			return 0;
		}
			
		//if address is USG, ignore it
		/*
		struct  in_addr addr;
		addr.s_addr = ((struct sockaddr_in *)&(d_event->DestAddr))->sin_addr.s_addr;
		if(!strcmp(inet_ntoa(addr), self_ip))
		{
			log_printf(LOG_WARNING, "ignore USG native SSDP response");
			break;
		}
		*/
        log_printf(LOG_INFO, "Find Device %p:Expires: %d  DeviceID:%s  DeviceType:%s  ServiceType:%s  ServiceVer:%s  Location:%s  Os:\"%s\"  Date:%s  Ext:\"%s\"\n", 
        d_event, d_event->Expires, d_event->DeviceId, d_event->DeviceType, d_event->ServiceType, d_event->ServiceVer, d_event->Location, d_event->Os, d_event->Date, d_event->Ext);

        //we only care hnap:WiFiExtender DeviceType
		if(WECB_CheckNoneEmpty(d_event->Location) && WECB_CheckNoneEmpty(d_event->DeviceId) && WECB_CheckNoneEmpty(d_event->DeviceType))  
        {
            //if(strstr(d_event->DeviceType, DEVICE_WIFI_EXTENDER)) 
            if(!strncmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER, strlen(DEVICE_WIFI_EXTENDER))) 
			{
				struct sockaddr_storage *dst_addr = (struct sockaddr_storage*)&d_event->DestAddr;
				if(dst_addr->ss_family == AF_INET)
				{
					p = inet_ntop(AF_INET, &((struct sockaddr_in *)&(d_event->DestAddr))->sin_addr, wecb_ip, sizeof(wecb_ip));
	
					if(!p)
					{
                  log_printf(LOG_ERR, "Convert IP failed\n");
						return 0;
					}
				}
				else if(dst_addr->ss_family == AF_INET6)
				{
					p = inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&(d_event->DestAddr))->sin6_addr, wecb_ip, sizeof(wecb_ip));

					if(!p || strncasecmp(wecb_ip, "fe80", 4))
					{
                  log_printf(LOG_ERR, "Convert IPv6 failed, ignore global v6 ip\n");
						return 0;
					}
				}
				//printf("the wecb ip is %s\n", wecb_ip);	
				//when WECB reports no sync, need to check if physical port has been switched 
				if(!strcmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER))
				{
					phy_port = get_phy_port(wecb_ip);
					if(phy_port == -1)
					{
                  log_printf(LOG_ERR, "Get WECB physical port index failed\n");
						return 0;	
					}
					//printf("WECB physical port is %d\n", phy_port);	
#ifdef CONFIG_CISCO_XHS
					//printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
					if(phy_port == 4)
					{
						log_printf(LOG_ERR, "Port 4 is dedicated for XHS, no sense occupied by WECB\n");
						Hnap_DelDeviceByUUID(&(d_event->DeviceId[5]));
						pthread_mutex_unlock(&device_list_mutex);
						return 0;
					}	
#endif	
				}
				//device id should begin with "uuid:"
            log_printf(LOG_INFO, "Matched Device :Expires: %d  DeviceID:%s  DeviceType:%s  ServiceType:%s  Location:%s\n", 
                    d_event->Expires, d_event->DeviceId, d_event->DeviceType, d_event->ServiceType, d_event->Location);

				PHnapDevice pDevice = NULL;
				
            log_printf(LOG_WARNING, "Device %s report it is %s\n", &(d_event->DeviceId[5]), 
                     !strcmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER_SYNCED) ? "sync" : "not sync");   
				pthread_mutex_lock(&device_list_mutex);
				pDevice	= Hnap_FindDeviceByUUID(&(d_event->DeviceId[5]));

				if(pDevice != NULL)
				{
               log_printf(LOG_WARNING, "got the device in the list, update addr and expire\n");
					if(dst_addr->ss_family == AF_INET)
					{
						pDevice->v4_expires = d_event->Expires;
						strncpy(pDevice->addr, wecb_ip, sizeof(pDevice->addr));
					}
					else if(dst_addr->ss_family == AF_INET6)
					{
						pDevice->v6_expires = d_event->Expires;
						strncpy(pDevice->addr6, wecb_ip, sizeof(pDevice->addr6));
					}
					//printf("v4 addr %s v6 addr %s\n", pDevice->addr, pDevice->addr6);
					
					//when we are doing initial sync, need to ignore upnp status to avoid sync again and again 
					if(!strcmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER) && pDevice->sync_mask != 0xFFFFFFFF)
					{
						pDevice->notify_mask=0xFFFFFFFF;
					}
#ifdef CONFIG_CISCO_HOTSPOT				
					if(!strcmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER))
					{
						int i = 0;
						for(; i < HS_SSID_NUM; i++)
						{
							if(!pDevice->bridge_ins[i] || check_wecb_bridge(pDevice->bridge_ins[i], phy_port) == -1)
							{
                        log_printf(LOG_WARNING, "check wecb bridge after unsync failed, retry\n");
								pthread_mutex_unlock(&device_list_mutex);
								return 0;
							}
						}
					}	
#endif
					strncpy(pDevice->location, d_event->Location, MAX_STRING_LEN - 1);
					strncpy(pDevice->device_type, d_event->DeviceType, MAX_STRING_LEN - 1);
					pthread_mutex_unlock(&device_list_mutex);
					return 0;
				}
				else
				{
               log_printf(LOG_WARNING, "can't find %s in list\n", &(d_event->DeviceId[5]));
					if (Hnap_GetDeviceNumber() >= max_client)
					{
                  log_printf(LOG_WARNING, "current WECB number has reached limitation\n");
						pthread_mutex_unlock(&device_list_mutex);
						return 0;
					}
					pDevice	= Hnap_AddDeviceByUUID(&(d_event->DeviceId[5]));
					
					if(pDevice != NULL)
					{
						int ret;
						char *tmp = NULL;
						pthread_t tmp_thread;
						pDevice->sync_mask = 0xFFFFFFFF;
						//pDevice->notify_mask = 0x0;
						if(dst_addr->ss_family == AF_INET)
						{
							pDevice->v4_expires = d_event->Expires;
							strncpy(pDevice->addr, wecb_ip, sizeof(pDevice->addr));
						}
						else if(dst_addr->ss_family == AF_INET6)
						{
							pDevice->v6_expires = d_event->Expires;
							strncpy(pDevice->addr6, wecb_ip, sizeof(pDevice->addr6));
						}

						strncpy(pDevice->location, d_event->Location, MAX_STRING_LEN - 1);
						strncpy(pDevice->device_type, d_event->DeviceType, MAX_STRING_LEN - 1);
#ifdef CONFIG_CISCO_HOTSPOT						
						if(reserve_bridge(pDevice->bridge_ins, pDevice->pvid, phy_port) == -1)
						{
							int i = 0;
                     log_printf(LOG_ERR, "Reserve Bridge for %s failed\n", d_event->DeviceId + 5);

							for(; i < HS_SSID_NUM; i++)
							{
								if(pDevice->bridge_ins[i] != 0)
									disable_specific_bridge(pDevice->bridge_ins[i]);
								pDevice->bridge_ins[i] = 0;
								pDevice->pvid[i] = 0;
							}
							Hnap_DelDeviceByUUID(&(d_event->DeviceId[5]));
							pthread_mutex_unlock(&device_list_mutex);
							return 0;
						}
#endif
						tmp = (char *)malloc(strlen(&(d_event->DeviceId[5])) + 1);
						if(tmp != NULL)
						{
							strcpy(tmp, &(d_event->DeviceId[5]));
							ret = pthread_create(&(pDevice->thread), &wecb_attr, wecb_sync_thread, tmp);
                     log_printf(LOG_WARNING, "create thread return %d\n", ret);
						}
						else
						{
                     log_printf(LOG_ERR, "memory err\n");
						}
						
						pthread_mutex_unlock(&device_list_mutex);
						return 0;
					}
					else
					{
						pthread_mutex_unlock(&device_list_mutex);
                  log_printf(LOG_WARNING, "add device %s failed\n", &(d_event->DeviceId[5]));
						return 0;
					}
				}
				/*
				//SSDP discovery reported it's already synced, we should find it in our device list
				if(!strcmp(d_event->DeviceType, DEVICE_WIFI_EXTENDER_SYNCED))
				{
					pthread_mutex_lock(&device_list_mutex);
					pDevice	= Hnap_FindDeviceByUUID(&(d_event->DeviceId[5]));
					
					if(pDevice != NULL)
					{
						//update the struct content to new
						log_printf(LOG_WARNING, "Device: %s is already synced, sync_mask = %x\n", d_event->DeviceId, pDevice->sync_mask);
						pDevice->expires = d_event->Expires;
						//pDevice->sync_mask = pDevice->notify_mask;
						//pDevice->notify_mask = 0;
						strncpy(pDevice->location, d_event->Location, MAX_STRING_LEN);
						strncpy(pDevice->device_type, d_event->DeviceType, MAX_STRING_LEN);
						pDevice->addr.s_addr = ((struct sockaddr_in *)&(d_event->DestAddr))->sin_addr.s_addr;
						pthread_mutex_unlock(&device_list_mutex);
						break;
					}
					else
					{
						log_printf(LOG_ERR, "we can't find %s in our list, something error, re-sync\n", d_event->DeviceId);
						pthread_mutex_unlock(&device_list_mutex);
					}
				}
				//not sync, need to push all settings down
				pthread_mutex_lock(&device_list_mutex);
				pDevice	= Hnap_AddDeviceByUUID(&(d_event->DeviceId[5]));
					
				if(pDevice != NULL)
				{
					//update the struct content to new
					log_printf(LOG_WARNING, "Device: %s is not synced\n", d_event->DeviceId);
					pDevice->expires = d_event->Expires;
					pDevice->sync_mask = 0xFFFFFFFF;
					//pDevice->notify_mask = 0x0;
					strncpy(pDevice->location, d_event->Location, MAX_STRING_LEN);
					strncpy(pDevice->device_type, d_event->DeviceType, MAX_STRING_LEN);
					pDevice->addr.s_addr = ((struct sockaddr_in *)&(d_event->DestAddr))->sin_addr.s_addr;
					pthread_mutex_unlock(&device_list_mutex);
				
					if(1)//in_sync == false)
					{	
						//malloc new memory to store device uuid for sync 	
						char *tmp = (char *)malloc(strlen(d_event->DeviceId) + 1);
					
						if(tmp != NULL)
						{
							pthread_t tmp_thread;
							strcpy(tmp, d_event->DeviceId + 5);
							pthread_create(&tmp_thread, &wecb_attr, wecb_push_settings, tmp);
							printf("create pthread %d\n", tmp_thread);
							//pthread_detach(tmp_thread);
						}
					}
					else
					{
						printf("malloc error\n");
					}
					break;
				}
				else
				{
					log_printf(LOG_ERR, "unknown error\n");
					pthread_mutex_unlock(&device_list_mutex);
				}*/
            }
            else 
			{
				log_printf(LOG_WARNING, "Not supported Device type: %s\n", d_event->DeviceType);
				return 0;
			};
        }

      //  printf("Device address %p: %ld, %s\n", 
      //      d_event, d_event->DestAddr.sin_addr.s_addr, inet_ntoa(d_event->DestAddr.sin_addr));
		//TvCtrlPointPrintList();
		break;
	}
	case UPNP_DISCOVERY_SEARCH_TIMEOUT:
        //printf("UPNP_DISCOVERY_SEARCH_TIMEOUT call back.\n");
		/* Nothing to do here... */
		break;
	case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: {
        //printf("UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE call back.\n");
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;

		if (d_event->ErrCode != UPNP_E_SUCCESS) {
         log_printf(LOG_ERR, "Error in Discovery ByeBye Callback -- %d\n", d_event->ErrCode);
		}

		log_printf(LOG_INFO, "Device byebye Expires: %d  DeviceID:%s  DeviceType:%s  ServiceType:%s  Location:%s\n", 
                  d_event->Expires, d_event->DeviceId, d_event->DeviceType, d_event->ServiceType, d_event->Location);

		/*
        if( WECB_CheckNoneEmpty(d_event->Location) && WECB_CheckNoneEmpty(d_event->DeviceId) && WECB_CheckNoneEmpty(d_event->DeviceType) )  
        {
            if(strstr(d_event->DeviceType, DEVICE_WIFI_EXTENDER)) {
            //if(strstr(d_event->DeviceType, "Device")) {
				
				//device id should begin with "uuid:"
                if(!strstr(d_event->DeviceId, "uuid:")) break;
				pthread_mutex_lock(&device_list_mutex);
				PHnapDevice pDevice = NULL;
				pDevice	= Hnap_FindDeviceByUUID(&(d_event->DeviceId[5]));

				//after SetDoRestart call, WECB will restart upnp daemon and send out SSDP:byebye
				//this doesn't mean device is going to offline, so we need pass this message
				if(pDevice != NULL && pDevice->restart_pending == false)
				{
					Hnap_DelDeviceByUUID(&(d_event->DeviceId[5]));
				}
				else
				{
					log_printf(LOG_WARNING, "Device:%s is in syncing, ignore byebye notification\n", &(d_event->DeviceId[5]));
					pDevice->restart_pending = false;
				}
				pthread_mutex_unlock(&device_list_mutex);
            }
		}*/

		break;
	}
	/* SOAP Stuff */
	case UPNP_CONTROL_ACTION_COMPLETE: {
        //printf("UPNP_CONTROL_ACTION_COMPLETE call back.\n");
		break;
	}
	case UPNP_CONTROL_GET_VAR_COMPLETE: {
        //printf("UPNP_CONTROL_GET_VAR_COMPLETE call back.\n");
		break;
	}
	/* GENA Stuff */
	case UPNP_EVENT_RECEIVED: {
        //printf("UPNP_EVENT_RECEIVED call back.\n");
		break;
	}
	case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        //printf("UPNP_EVENT_SUBSCRIBE_COMPLETE call back.\n");
	case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
        //printf("UPNP_EVENT_UNSUBSCRIBE_COMPLETE call back.\n");
	case UPNP_EVENT_RENEWAL_COMPLETE: {
        //printf("UPNP_EVENT_RENEWAL_COMPLETE call back.\n");
		break;
	}
	case UPNP_EVENT_AUTORENEWAL_FAILED:
        //printf("UPNP_EVENT_AUTORENEWAL_FAILED call back.\n");
	case UPNP_EVENT_SUBSCRIPTION_EXPIRED: {
        //printf("UPNP_EVENT_SUBSCRIPTION_EXPIRED call back.\n");
		break;
	}
	/* ignore these cases, since this is not a device */
	case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        //printf("UPNP_EVENT_SUBSCRIPTION_REQUEST call back.\n");
	case UPNP_CONTROL_GET_VAR_REQUEST:
        //printf("UPNP_CONTROL_GET_VAR_REQUEST call back.\n");
	case UPNP_CONTROL_ACTION_REQUEST:
        //printf("UPNP_CONTROL_ACTION_REQUEST call back.\n");
		break;
	}

	return 0;
}

/********************************************************************************
 * WECB_UPnPAgentStart
 *
 * Description: 
 *       Initialize the UPnP library and start the UPnP Agent.
 *
 * Parameters:
 *   ip_address -- LAN ip address. 
 *
 * This api now is ipv4 enable only
 ********************************************************************************/
int WECB_UPnPAgentStart(char * ip_address, char *lan_if)
{
	int rc;
	unsigned short port = 0;
    pthread_t SearchAll_thread;
    pthread_t CheckAllTimer_thread;
	//char *ip_address = "192.168.1.133";
	char val[8];

	memset(val, 0, sizeof(val));
	rc = syscfg_get(NULL, "max_wecb_num", val, sizeof(val));

	if(rc != -1 && atoi(val))
		max_client = atoi(val);

	//printf("current wecb limitation is %d\n", max_client);	
    pthread_mutex_init(&device_list_mutex, 0);
	memset(self_ip, 0, sizeof(self_ip));

    //printf("Initializing UPnP Sdk with\n"
	//		 "\tipaddress = %s port = %u\n",
	//		 ip_address ? ip_address : "{NULL}", port);

	
	rc = UpnpInit(ip_address, port);
	
	if (rc != UPNP_E_SUCCESS) {
        log_printf(LOG_ERR, "UpnpInit() Error: %d\n", rc);
	    UpnpFinish();
        return -1;
	}
	//Adding IPv6 support for libupnp
	/*
   rc = UpnpInit2(lan_if, 0);

	if (rc != UPNP_E_SUCCESS) {
        log_printf(LOG_ERR, "UpnpInit2() Error: %d\n", rc);
	    UpnpFinish();
        return -1;
	}
   */

	if (!ip_address) {
		ip_address = UpnpGetServerIpAddress();
	}
	strncpy(self_ip, ip_address, sizeof(self_ip) - 1);
	if (!port) {
		port = UpnpGetServerPort();
	}

	log_printf(LOG_INFO, "UPnP Initialized with ipaddress = %s port = %u\n", ip_address ? ip_address : "{NULL}", port);
	rc = UpnpRegisterClient(WECB_UPnPHandler, &ctrlpt_handle, &ctrlpt_handle);
	
	if (rc != UPNP_E_SUCCESS) {
        log_printf(LOG_ERR, "Error registering callback function: %d\n", rc);
		UpnpFinish();
		return -1;
	}

	log_printf(LOG_INFO, "WECB UPnPAgent Registered\n");

    pthread_create(&SearchAll_thread, &wecb_attr, WECB_SSDP_DiscoveryAll, NULL);
	log_printf(LOG_INFO, "create pthread %d\n", SearchAll_thread);
	//pthread_detach(SearchAll_thread);
	pthread_create(&CheckAllTimer_thread, &wecb_attr, WECB_SSDP_CheckAllTimer, NULL);
	log_printf(LOG_INFO, "create pthread %d\n", CheckAllTimer_thread);
	//pthread_detach(CheckAllTimer_thread);


    //PLmObjectHnapDevice pHnapDevice = Hnap_AddDeviceByUUID("01.01.001.0001");
    //LanManager_HNAPGetDeviceSettings(pHnapDevice, "http://192.168.1.1/HNAP1/");
	return 0;
}


int WECB_UPnPAgentStop(void)
{
	UpnpUnRegisterClient(ctrlpt_handle);
	pthread_mutex_lock(&device_list_mutex);
    Hnap_DelAllDevice();
	pthread_mutex_unlock(&device_list_mutex);
	UpnpFinish();

	return 0;
}


void* WECB_SSDP_DiscoveryAll(void *P)
{
    while(1){
        /* Wait for 10 seconds. */
        int res = UpnpSearchAsync(
            ctrlpt_handle,
            WECB_MX,
            "hnap:all", //both ssdp:all and hnap:all work for HNAP devices.
            //"hnap:WiFiExtender",
            NULL 
        );
        if(res != UPNP_E_SUCCESS) {
            log_printf(LOG_ERR, "WECB_SSDP_DiscoveryAll return error %d\n", res);
            break;
        }

        sleep(SSDP_INTERVAL);
    };
	return NULL;
}

void* WECB_SSDP_CheckAllTimer(void *P)
{
    while(1)
	{
        sleep(ALIVE_CHCEK_INTERVAL);
		pthread_mutex_lock(&device_list_mutex);
		Hnap_CheckAllDeviceAlive(ALIVE_CHCEK_INTERVAL);
		pthread_mutex_unlock(&device_list_mutex);
    };
	return NULL;
}


void wecb_sig_handler(int signal)
{
	log_printf(LOG_ERR, "Received signal %d\n", signal);
	// can's use pthread mutex in signal handler
	//WECB_UPnPAgentStop();
	//UpnpUnRegisterClient(ctrlpt_handle);
	//UpnpFinish();
	//wecb_global_uninit();
#ifdef CONFIG_CISCO_HOTSPOT
	if(disable_all_wecb_bridge() == -1)
	{
		log_printf(LOG_ERR, "Disable all WECB dedicated bridge failed\n");
	}
#endif
	exit(1);
}



