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

    module: wecb_master.c

        For WECB control development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implements the main code path for HNAP control on WECB.

    -------------------------------------------------------------------


    author:

        Rongwei

    -------------------------------------------------------------------

    revision:

        11/02/2012    initial revision.

**************************************************************************/
#include "wecb_log.h"
#include "wecb_upnp_agent.h"
#include "pthread.h"
#include "upnp.h"
#include "wecb_util.h"
#include "wecb_common.h"
#include "hnap/hdk_mbus.h"
#include "hnap/hnap_device.h"

#include <stdio.h>
#include <sys/stat.h>

#include "syscfg/syscfg.h"

extern pthread_attr_t wecb_attr;
extern pthread_mutex_t device_list_mutex;

int main()
{
	int mask = 0, ret = EVENT_ERROR;
	pthread_t pam_sk_thread;
    pthread_t wecb_time_thread;
	pthread_t wecb_sync_thread;
	char lan_ip[64], lan_if[64];
	char **all_device = NULL;
	int device_number = 0, j, event_value = 0;
	bool rt = false;
	FILE *pid_file = NULL, *rotate_file = NULL;
	struct stat statbuf;

	//set pid file to be monitored
	pid_file = fopen("/var/run/wecb_master.pid", "w");
	
	if(pid_file != NULL)
	{
		fprintf(pid_file, "%u", getpid());
		fclose(pid_file);
	}
	
	rotate_file = fopen("/etc/cron/cron.everyminute/sysklogd_rotate.sh", "w");

	if(rotate_file != NULL)
	{
		fprintf(rotate_file, "%s\n", "#!/bin/sh");
		fprintf(rotate_file, "%s\n\n", "SIZE=`ls -l /var/log/remote.log | awk \'{print $3}\'`");
		fprintf(rotate_file, "%s\n", "if [ \"$SIZE\" -gt 204800 ] ; then");
		fprintf(rotate_file, "%s\n", "\tmv /var/log/remote.log /var/log/remote.log.0");
		fprintf(rotate_file, "%s\n", "\tkill -HUP `cat /var/run/syslogd.pid`");
		fprintf(rotate_file, "%s\n", "fi");
		fclose(rotate_file);
	}
	stat("/etc/cron/cron.everyminute/sysklogd_rotate.sh", &statbuf);
	chmod("/etc/cron/cron.everyminute/sysklogd_rotate.sh", statbuf.st_mode | S_IXUSR | S_IXGRP | S_IXOTH);	
	
	while(wecb_event_inits() == EVENT_ERROR) 
	{
        log_printf(LOG_ERR, "sysevent init failed\n");
        sleep(1); // Error 
    }

	wecb_global_init();
	openlog ("wecb_log", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	memset(lan_ip, 0, sizeof(lan_ip));

	//wecb_get_if_ip("brlan0", lan_ip, sizeof(lan_ip));

	syscfg_get(NULL, "lan_ipaddr", lan_ip, sizeof(lan_ip));
	syscfg_get(NULL, "lan_ifname", lan_if, sizeof(lan_if));

	//register signal handler to free global device list
	wecb_reg_signal();

	//start socket to receive message from PAM
	pthread_create(&pam_sk_thread, &wecb_attr, wecb_sk_server, NULL);
	log_printf(LOG_INFO, "create pthread %d\n", pam_sk_thread);
	//pthread_detach(pam_sk_thread);
	pthread_create(&wecb_time_thread, &wecb_attr, wecb_sync_sys_time, NULL);
	log_printf(LOG_INFO, "create pthread %d\n", wecb_time_thread);
	//pthread_detach(wecb_time_thread);

	WECB_UPnPAgentStart(lan_ip, lan_if);
	while(1)
	{
		while(1)
		{
			ret = wecb_event_listen(&event_value);
			switch (ret)
			{
				case EVENT_WAN_UP:
					//When WAN up, USG system time has been synced with CMTS
					mask |= EVENT_WAN_UP;
					wecb_sync_with_pam(TIME_SET);
					log_printf(LOG_WARNING, "receive wan up event\n");
					mask ^= EVENT_WAN_UP;
					break;
				case EVENT_TIMEOUT:
					log_printf(LOG_WARNING, "sysevent timeout\n");
					break;
				case EVENT_WAN_DOWN:
					log_printf(LOG_WARNING, "receive wan down event\n");
					break;
				case EVENT_MOCA_UP:
					mask |= EVENT_MOCA_UP;
					log_printf(LOG_WARNING, "receive MoCA up event\n");
					break;
				case EVENT_MOCA_DOWN:
					log_printf(LOG_WARNING, "receive MoCA down event\n");
					break;
				case EVENT_LAN_IP_SET:
					log_printf(LOG_WARNING, "receive lan ip address set event\n");
					mask |= EVENT_LAN_IP_SET;
					break;
				case EVENT_WECB_HHS_BRIDGE:
					log_printf(LOG_WARNING, "receive lan ip address set event\n");
					mask |= EVENT_WECB_HHS_BRIDGE;
					//handle hhs bridge for MoCA.
					break;
				default:
					log_printf(LOG_WARNING, "something is wrong\n");;
					break;
			}
			/*
			if(!(mask ^ EVENT_LAN_IP_SET))
			{
				log_printf(LOG_WARNING, "lan ip changed, re-start wecb main process\n");
				break;
			}*/
		}

		wecb_event_close();
		break;

		//sleep(1200);
		/*	
		pthread_mutex_lock(&device_list_mutex);
		rt = Hnap_GetAllDevice(&all_device, &device_number);
		pthread_mutex_unlock(&device_list_mutex);
		
		if((rt == true) && (device_number != 0))
		{
			printf("Device number is %d, handle is %x\n", device_number, all_device);
			for(j = 0; j < device_number; j++)
			{
				printf("handle %d is %x\n", j, all_device[j]);
				if(all_device[j] != NULL)
				{
					printf("create thread to sync wecb device\n");
					pthread_create(&wecb_sync_thread, &wecb_attr, wecb_push_settings, all_device[j]);
				}
			}
			free(all_device);	
		}
		else
		{
			log_printf(LOG_WARNING, "get all device falied\n");
		}
		*/
	}

	WECB_UPnPAgentStop();
	wecb_global_uninit();
	return 0;
}



