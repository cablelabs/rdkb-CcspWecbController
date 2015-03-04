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

    module: cosa_wifi_dml.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaWifiCreate
        *  CosaWifiInitialize
        *  CosaWifiRemove
        *  CosaDmlWifiGetPortMappingNumber
    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/
#include "cosa_apis.h"
#include "cosa_wifi_apis.h"
#include "cosa_wifi_internal.h"
#include "plugin_main_apis.h"
#include "cosa_x_comcast_com_gre_apis.h"
#include "ccsp_psm_helper.h"
#include "syscfg/syscfg.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "wecb_log.h"
#include "wecb_common.h"

struct ExtInfo
{
	int *ext_count;
	struct ExtStatus **ext_status;
};


int notify_wecb(enum PAM_EVENT type, ANSC_HANDLE rt)
{
	int connect_fd;
	struct sockaddr_un srv_addr;
	int ret;
	int i, flags, n, error;
	socklen_t len;  
	fd_set rset, wset;
	struct ExtInfo *ext_info = (struct ExtInfo *)rt;
	struct timeval tval;

	//create client socket
	connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(connect_fd < 0)
	{
		log_printf(LOG_ERR, "ARM WiFi SBAPI create socket failed\n");
		return -1;
	}
  
	//set server sockaddr_un
	srv_addr.sun_family = AF_UNIX;
	strcpy(srv_addr.sun_path, WECB_SK);
 
	if((flags = fcntl(connect_fd, F_GETFL)) < 0) 
	{
		log_printf(LOG_ERR, "fcntl get failed\n");
		close(connect_fd);
		return -1;
	}

	if(fcntl(connect_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		log_printf(LOG_ERR, "fcntl set failed\n");
		close(connect_fd);
		return -1;
	}
 
	//connect to server
	ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret == -1 &&  errno != EINPROGRESS)
	{
		log_printf(LOG_ERR, "ARM WiFi SBAPI connect to server failed!\n");
		close(connect_fd);
		return -1;
	}

	//means not connected on
	if (ret != 0)
	{

		FD_ZERO(&rset);
		FD_SET(connect_fd, &rset);
		wset = rset;  
		tval.tv_sec = 3;
		tval.tv_usec = 0;

		if ((n = select(connect_fd+1, &rset, &wset, NULL, &tval)) == 0) 
		{
			log_printf(LOG_ERR, "ARM WiFi SBAPI connect timeout!\n");
			close(connect_fd);
            return (-1);
        }

        if(FD_ISSET(connect_fd, &rset) || FD_ISSET(connect_fd, &wset)) 
		{
			len = sizeof(error);

            n = getsockopt(connect_fd, SOL_SOCKET, SO_ERROR, &error, &len);
		    if (n < 0 || error)
			{
				log_printf(LOG_ERR, "ARM WiFi SBAPI getsockopt error!\n");
				close(connect_fd);
			    return (-1);
			}
        }
		else
		{
			log_printf(LOG_ERR, "no fd set!\n");
			close(connect_fd);
            return (-1);
        }
	}
	fcntl(connect_fd, F_SETFL, flags);	
	//send message to server
	if(send(connect_fd, &type, sizeof(type), 0) < 0)
    {
		log_printf(LOG_ERR, "ARM WiFi SBAPI send msg error: %s(errno: %d)\n", strerror(errno), errno);
		close(connect_fd);
		return -1;
    }
	
	if (type == QUERY_ALL)
	{
		//Get ExtenderStatus message from socket server
		if (recv(connect_fd, &i, sizeof(int), 0) < 0)
		{
			log_printf(LOG_ERR, "ARM WiFi SBAPI recv extender count error\n");
			close(connect_fd);
			return -1;
		}

		*(ext_info->ext_count) = i;
		*(ext_info->ext_status) = AnscAllocateMemory(sizeof(struct ExtStatus) * i);

		if (*(ext_info->ext_status) == NULL)
		{
			log_printf(LOG_ERR, "ARM WiFi SBAPI no memory\n");
			close(connect_fd);
			return -1;
		}
		i = 0;
		while (i < *(ext_info->ext_count))
		{
			if (recv(connect_fd, *(ext_info->ext_status) + i, sizeof(struct ExtStatus), 0) < 0)
			{
				log_printf(LOG_ERR, "ARM WiFi SBAPI recv extender status error\n");
				AnscFreeMemory(*(ext_info->ext_status));
				*(ext_info->ext_status) = NULL;
				ext_info->ext_count = 0;
				close(connect_fd);
				return -1;
			}
			i++;
		}
	}
	close(connect_fd);
	return 0;
}

unsigned int htoi(char s[])  
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if (tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}  

ULONG GetSsidIndexAsInstanceNumber(ULONG ins_num)
{
	ULONG i;
	char syscfg_indicator[MAX_BUF], buf[MAX_BUF], tmp[MAX_BUF];
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	log_printf(LOG_INFO, "searching ssid instance number %d entry index\n", ins_num);
	//SSID has up to MAX_SSID * 2 entities since each can only effective on single band
	for(i = 0; i < MAX_SSID * 2; i++)
	{
		//Get actual instance number
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VALID, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
		syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

		//syscfg ssid entry is not valid 
		if(buf[0] == '0')
		{
			continue;
		}
		else
		{
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INS_NUM, sizeof(syscfg_indicator) - 3);
			syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
			syscfg_get(NULL, syscfg_indicator, tmp, sizeof(tmp));

			if(atoi(tmp) == ins_num)
			{
				log_printf(LOG_INFO, "Syscfg ssid entry %d with instance number %d\n", i, ins_num);
				return i;
			}	
		}
	}
	return  MAX_SSID * 2;
}


ANSC_STATUS
CosaDmlWiFiInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    )
{	
	openlog ("wecb_log", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS
CosaDmlWiFi_FactoryReset()
{
    return ANSC_STATUS_SUCCESS;
}

#ifdef CONFIG_CISCO_HOTSPOT
static void* circuit_id_update_thread(void* arg) {
    hotspot_update_circuit_ids(1,1);
    return NULL;
}
#endif

ANSC_STATUS
CosaDmlWiFi_RadioUpdated()
{
	if(notify_wecb(RADIO_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}
	return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFi_SSIDUpdated()
{
    void * params = NULL;
	if(notify_wecb(SSID_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}
	
#ifdef CONFIG_CISCO_HOTSPOT
	// TODO check for only hotspot ssids, and if hotspot is enabled
	AnscCreateTask(circuit_id_update_thread, USER_DEFAULT_TASK_STACK_SIZE, USER_DEFAULT_TASK_PRIORITY, params, "CircuitIDUpdateThread");
#endif
	return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFi_WPSUpdated()
{
	if(notify_wecb(WPS_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}
	return ANSC_STATUS_SUCCESS;
}

/*
 *  Description:
 *     The API retrieves the number of WiFi radios in the system.
 */
ULONG
CosaDmlWiFiRadioGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    )
{
	/* Rongwei mod
	 * Current, we wil always show 2 for this parameter, 
	 * when we sync the MoCA extenders, will poll how many radios they have
	 * and discard the unsupported one
	 */
	log_printf(LOG_WARNING, "All WECB Radio will be parsed from WiFi DM\n");
   printf("%s() All WECB Radio will be parsed from WiFi DM\n", __func__);
    return 0;
}    
    
ANSC_STATUS
CosaDmlWiFiRadioGetSinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_RADIO_SINFO  pInfo
    )
{
    return ANSC_STATUS_SUCCESS;
}

    
/* Description:
 *	The API retrieves the complete info of the WiFi radio designated by index. 
 *	The usual process is the caller gets the total number of entries, 
 *	then iterate through those by calling this API.
 * Arguments:
 * 	ulIndex		Indicates the index number of the entry.
 * 	pEntry		To receive the complete info of the entry.
 */
ANSC_STATUS
CosaDmlWiFiRadioGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PCOSA_DML_WIFI_RADIO_FULL   pEntry
    )
{
	//For Radio, simply treats instance number as 1, 2, also index + 1
	pEntry->Cfg.InstanceNumber = ulIndex + 1;
	log_printf(LOG_INFO, "Getting the %drd radio entry\n", ulIndex+1);
	return CosaDmlWiFiRadioGetCfg(hContext, &(pEntry->Cfg));
}

ANSC_STATUS
CosaDmlWiFiRadioSetDefaultCfgValues
    (
        ANSC_HANDLE                 hContext,
        unsigned long               ulIndex,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg
    )
{
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiRadioSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    )
{
        return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiRadioSetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg        /* Identified by InstanceNumber */
    )
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];
	int index = 0;

	memset(syscfg_indicator, 0, sizeof(syscfg_indicator));
	memset(buf, 0, sizeof(buf));

	if (pCfg == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
        return ANSC_STATUS_FAILURE; 
	}

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	log_printf(LOG_INFO, "Get Wifi radiocfg instance number %d\n", pCfg->InstanceNumber);
	
	index = pCfg->InstanceNumber - 1;

	//construct syscfg indicator, which contains common string and syscfg subfix	
	//Set radio enabled value	
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_ENABLE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->bEnabled);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "radio enable is %c\n", buf[0]);

	//Set raido mode
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_MODE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%lu", pCfg->OperatingStandards);
	syscfg_set(NULL, syscfg_indicator, buf);

	//Set Radio Channel width
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_CHANNEL_WD, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->OperatingChannelBandwidth);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "Radio channel width is %s\n", buf);

	//Set radio channel
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_CHANNEL, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->Channel);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "Radio channel is %s\n", buf);
	
	//Set radio second channel
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_SEC_CHANNEL, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->ExtensionChannel);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "Second channel is %s\n", buf);
	
	//Set radio beacon interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_BEACON_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->BeaconInterval);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "beacon interval is %s\n", buf);

	//Set Radio DTIM interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_DTIM_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->DTIMInterval);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "DTIM interval is %s\n", buf);

	//Get Radio guard interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_GUARD_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->GuardInterval);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "radio guard interval is %s\n", buf);

	//Get radio coexistance
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_COEXIT, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	sprintf(buf, "%d", pCfg->bCoexistance);
	syscfg_set(NULL, syscfg_indicator, buf);

	syscfg_commit();	
	if(notify_wecb(RADIO_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}

	return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiRadioGetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg        /* Identified by InstanceNumber */
    )
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];
	int index = 0;

	memset(syscfg_indicator, 0, sizeof(syscfg_indicator));
	memset(buf, 0, sizeof(buf));

	if (pCfg == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
        return ANSC_STATUS_FAILURE; 
	}

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	log_printf(LOG_INFO, "Get Wifi radiocfg instance number %d\n", pCfg->InstanceNumber);

	//Get instance number, Radio has two configurations with subfix 0 or 1
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_INS_NUM, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

	if(pCfg->InstanceNumber == atoi(buf))
	{
		log_printf(LOG_INFO, "Got the correspond syscfg subfix 0\n");
	}
	else
	{
		index++;
		log_printf(LOG_INFO, "syscfg subfix is 1\n");
	}

	//construct syscfg indicator, which contains common string and syscfg subfix	
	//Get RadioID
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_RADIOID, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	AnscCopyString(pCfg->Alias, buf);

	//Set band accord to radio id
	if (strstr(buf, "5GHz"))
	{
		pCfg->OperatingFrequencyBand = COSA_DML_WIFI_FREQ_BAND_5G;
	}
	else
	{
		pCfg->OperatingFrequencyBand = COSA_DML_WIFI_FREQ_BAND_2_4G;
	}


	//Get radio enabled value	
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_ENABLE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->bEnabled = (buf[0] == '0') ? FALSE : TRUE;
	log_printf(LOG_INFO, "radio enable is %s\n", buf);

	//Get raido mode
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_MODE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->OperatingStandards = atoi(buf);
	log_printf(LOG_INFO, "radio mode is %s\n", buf);

	//Get Radio Channel width
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_CHANNEL_WD, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	
	if (!strncmp(buf, "1", 1))
	{
		log_printf(LOG_INFO, "Radio channel is 20MHz\n");
		pCfg->OperatingChannelBandwidth = COSA_DML_WIFI_CHAN_BW_20M;
	}
	else if(!strncmp(buf, "2", 1))
	{
		log_printf(LOG_INFO, "Radio channel is 40MHz\n");
		pCfg->OperatingChannelBandwidth = COSA_DML_WIFI_CHAN_BW_40M;
	}
	else if(!strncmp(buf, "3", 1))
	{
		log_printf(LOG_INFO, "Radio channel width is auto selected\n");
		//don't know if CCSP framework can handle this value
		pCfg->OperatingChannelBandwidth = 3;
	}

	//Get radio channel
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_CHANNEL, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

	if (!strncmp(buf, "0", 1))
	{
		log_printf(LOG_INFO, "auto channel\n");
		pCfg->Channel = 0;
		pCfg->AutoChannelEnable = TRUE;
	}
	else 
	{
		log_printf(LOG_INFO, "Radio channel is %s\n", buf);
		pCfg->Channel = atoi(buf);
		pCfg->AutoChannelEnable = FALSE;
	}

	//Get radio second channel if width is enable
	if (pCfg->OperatingChannelBandwidth != 2)
	{
		log_printf(LOG_INFO, "Second channel is unavaible or auto selected due to channel width\n");
		pCfg->ExtensionChannel = COSA_DML_WIFI_EXT_CHAN_Auto;
	}
	else
	{
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_SEC_CHANNEL, sizeof(syscfg_indicator) - 2);
		syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
		syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
		log_printf(LOG_INFO, "Second channel is %s\n", buf);

		if (!strncmp(buf, "0", 1) || atoi(buf) > 3)
		{
			log_printf(LOG_INFO, "Treat unrecoganized values as auto\n");
			strcpy(buf, "3");
		}
		pCfg->ExtensionChannel = (COSA_DML_WIFI_EXT_CHAN)atoi(buf);
	}
	
	//Get radio beacon interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_BEACON_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->BeaconInterval = atoi(buf);
	log_printf(LOG_INFO, "beacon interval is %s\n", buf);

	//Get Radio DTIM interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_DTIM_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->DTIMInterval = atoi(buf);
	log_printf(LOG_INFO, "DTIM interval is %s\n", buf);

	//Get Radio guard interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_GUARD_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

	if (!strncmp(buf, "0", 1) || !strncmp(buf, "3", 1))
	{
		log_printf(LOG_INFO, "radio guard interval is auto selected\n");
		pCfg->GuardInterval = COSA_DML_WIFI_GUARD_INTVL_Auto;
	}
	else if (!strncmp(buf, "1", 1))
	{
		log_printf(LOG_INFO, "radio guard interval is 400ns\n");
		pCfg->GuardInterval = COSA_DML_WIFI_GUARD_INTVL_400ns;
	}
	else if (!strncmp(buf, "2", 1))
	{
		log_printf(LOG_INFO, "radio guard interval is 800ns\n");
		pCfg->GuardInterval = COSA_DML_WIFI_GUARD_INTVL_800ns;
	}
	else
	{
		log_printf(LOG_INFO, "radio guard interval unkown, treat as auto selected\n");
		pCfg->GuardInterval = COSA_DML_WIFI_GUARD_INTVL_Auto;
	}	

	//Get radio coexistance
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_RADIO_COEXIT, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->bCoexistance = buf[0] == '0' ? FALSE : TRUE;

	return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiRadioGetDinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_RADIO_DINFO  pInfo
    )
{
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiRadioGetStats
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_RADIO_STATS  pStats
    )
{
    return ANSC_STATUS_SUCCESS;
}

/* Description:
 *	The API retrieves the number of WiFi SSIDs in the system.
 */
ULONG
CosaDmlWiFiSsidGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    )
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	//In current release, we won't support this feature
	return 0;

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	memset(buf, 0, sizeof(buf));
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NUM, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

	log_printf(LOG_INFO, "ssid number is %s\n", buf);
	return atoi(buf);
	
}

/* Description:
 *	The API retrieves the complete info of the WiFi SSID designated by index. The usual process is the caller gets the total number of entries, then iterate through those by calling this API.
 * Arguments:
 * 	ulIndex		Indicates the index number of the entry.
 * 	pEntry		To receive the complete info of the entry.
 */
ANSC_STATUS
CosaDmlWiFiSsidGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PCOSA_DML_WIFI_SSID_FULL    pEntry
    )
{
	int i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	log_printf(LOG_INFO, "Get index %d\n", ulIndex);

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	for(i = 0; i < MAX_SSID * 2; i++)
	{
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VALID, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
		syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
		if(buf[0] != '0')
		{
			if(!ulIndex)
			{
				log_printf(LOG_INFO, "Got ssid entry %d\n", i);
				break;
			}
			ulIndex--;
		}
	}

	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find correspond ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INS_NUM, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

	pEntry->Cfg.InstanceNumber = atoi(buf);
	
	log_printf(LOG_INFO, "instance number is %s\n", buf);
    return CosaDmlWiFiSsidGetCfg(hContext, &(pEntry->Cfg));
}

ANSC_STATUS
CosaDmlWiFiSsidSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    )
{
    return ANSC_STATUS_SUCCESS;
}    

/* Description:
 *	The API adds a new WiFi SSID into the system. 
 * Arguments:
 *	hContext	reserved.
 *	pEntry		Caller pass in the configuration through pEntry->Cfg field and gets back the generated pEntry->StaticInfo.Name, MACAddress, etc.
 */
ANSC_STATUS
CosaDmlWiFiSsidAddEntry
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_FULL    pEntry
    )
{
	ULONG i, j;
	char syscfg_indicator[MAX_BUF], buf[MAX_BUF];
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	
	//SSID has up to MAX_SSID * 2 entities since each can only effective on single band
	for(i = 0; i < MAX_SSID * 2; i++)
	{
		int ret = 0;
		memset(buf, 0, sizeof(buf));
		//Get actual instance number
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VALID, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
		ret = syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

		//syscfg ssid entry is not valid or unexisted
		if(ret == -1 || buf[0] == '0')
		{
			break;
		}
	}
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "all syscfg entries are full\n");
		return ANSC_STATUS_FAILURE;
	}
	
	//Set the entry to be valid
	syscfg_set(NULL, syscfg_indicator, "1");
	log_printf(LOG_INFO, "Rongwei set ssid entry %d to be valid\n", i);

	//set ssid instance number
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INS_NUM, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pEntry->Cfg.InstanceNumber);
	log_printf(LOG_INFO, "Rongwei set ssid instance number %d\n", pEntry->Cfg.InstanceNumber);
	syscfg_set(NULL, syscfg_indicator, buf);

	if(CosaDmlWiFiSsidSetCfg(NULL, &(pEntry->Cfg)) != ANSC_STATUS_SUCCESS)
	{
		log_printf(LOG_ERR,"Set ssid cfg failed\n");
		syscfg_set(NULL, syscfg_indicator, "0");
		return ANSC_STATUS_FAILURE;
	}

	for(j = 0; j < SSID_QOS_NUM; j++)
	{
		if(CosaDmlWiFiSSIDQosSettingSetCfg(NULL, pEntry->Cfg.InstanceNumber, pEntry->Cfg.QosInfo.QosSetting + j) != ANSC_STATUS_SUCCESS)
		{
			syscfg_set(NULL, syscfg_indicator, "0");	
			return ANSC_STATUS_FAILURE;	
		}
	}

	//add one to ssid number
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NUM, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	j = atoi(buf) + 1;
	sprintf(buf, "%d", j);
	log_printf(LOG_INFO, "Rongwei set ssid number %d\n", j);
	syscfg_set(NULL, syscfg_indicator, buf);

	syscfg_commit();
	return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSsidDelEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    )
{
	int index = ulInstanceNumber, i, j;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	
	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//Set the valid flag in syscfg
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VALID, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, "0");
	log_printf(LOG_INFO, "delete ssid %d entry %d\n", ulInstanceNumber, i);

	//del one to ssid number
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NUM, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	j = atoi(buf) - 1;
	sprintf(buf, "%d", j);
	syscfg_set(NULL, syscfg_indicator, buf);

	syscfg_commit();	
	if(notify_wecb(SSID_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSsidSetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_CFG     pCfg
    )
{
	int index = pCfg->InstanceNumber, i, j;
	ULONG mask1, mask2;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF], tmp[MAX_BUF];
	ULONG ssid_index;
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	
	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//make sure the ssid name is not conflict

	for(j = 0; j < MAX_SSID * 2; j++)
	{
		memset(buf, 0, sizeof(buf));
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NAME, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", j);
		syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));

		if (!strcmp(buf, pCfg->SSID))
		{
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VALID, sizeof(syscfg_indicator) - 3);
			syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
			ret = syscfg_get(NULL, syscfg_indicator, tmp, sizeof(tmp));

			//syscfg ssid entry is not valid or unexisted
			if(ret == -1 || tmp[0] == '0')
			{
				//means this entry is not valid
				break;
			}

			log_printf(LOG_ERR, "Instance %d ssid name is the same with syscfg %d\n", index, j);
			return ANSC_STATUS_FAILURE;
		}
	}

	//Set Alias	
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ALIAS, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pCfg->Alias);
	log_printf(LOG_INFO, "ssid entry %d Alias %s\n", i, pCfg->Alias);

	//Set bEnabled
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENABLE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pCfg->bEnabled);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d bEnabled %d\n", i, pCfg->bEnabled);

	//Set radio id
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_RADIOID, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	//2.4G and 5G first ssid resides in Device.WiFi. and can't be overwriten 
	ssid_index = htoi(pCfg->SSIDIndex) & 0xEFFFEFFF;
	log_printf(LOG_INFO, "ssid index is %x\n", ssid_index);
	
	if(ssid_index & 0xFFFF0000)
	{
		if(ssid_index & 0x0000FFFF)
		{
			syscfg_set(NULL, syscfg_indicator, "RADIO_2.4GHz & RADIO_5GHz");
		}
		else
		{
			syscfg_set(NULL, syscfg_indicator, "RADIO_2.4GHz");
		}
	}
	else
	{
		if(ssid_index & 0x0000FFFF)
		{
			syscfg_set(NULL, syscfg_indicator, "RADIO_5GHz");
		}
		else
		{
			syscfg_set(NULL, syscfg_indicator, "");
		}
	}

	//Set ssid index
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INDEX, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	//sprintf(buf, "%u", pCfg->SSIDIndex);
	if(!strncmp(pCfg->SSIDIndex, "0x", 2) || !strncmp(pCfg->SSIDIndex, "0X", 2))
		syscfg_set(NULL, syscfg_indicator, pCfg->SSIDIndex + 2);
	else	
		syscfg_set(NULL, syscfg_indicator, pCfg->SSIDIndex);
	log_printf(LOG_INFO, "ssid entry %d SSIDIndex %s\n", i, pCfg->SSIDIndex);

	//check all ssid entry
	for(j = 0; j < MAX_SSID * 2; j++)
	{
		//don't need upate current set entry
		if( j == i)
		{
			continue;
		}
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INDEX, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", j);
		syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
		mask1 = htoi(buf);
		ssid_index = htoi(pCfg->SSIDIndex);
		mask2 = mask1 & ssid_index;
		log_printf(LOG_INFO, "ssid entry %d index %x current index %x\n", j, mask1, ssid_index);
		mask1 ^= mask2;
		sprintf(buf, "%x", mask1);
		log_printf(LOG_INFO, "ssid entry %d was updated by mask %16x, result %s\n", j, mask2, buf);
		syscfg_set(NULL, syscfg_indicator, buf);

		
		strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_RADIOID, sizeof(syscfg_indicator) - 3);
		syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
		sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", j);
		if(mask1 & 0xFFFF0000)
		{
			if(mask1 & 0x0000FFFF)
			{
				syscfg_set(NULL, syscfg_indicator, "RADIO_2.4GHz & RADIO_5GHz");
			}
			else
			{
				syscfg_set(NULL, syscfg_indicator, "RADIO_2.4GHz");
			}
		}
		else
		{
			if(mask1 & 0x0000FFFF)
			{
				syscfg_set(NULL, syscfg_indicator, "RADIO_5GHz");
			}
			else
			{
				syscfg_set(NULL, syscfg_indicator, "");
			}
		}
	}

	//Set ssid name
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NAME, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pCfg->SSID);
	log_printf(LOG_INFO, "ssid entry %d SSID name %s\n", i, pCfg->SSID);

	//Set ssid broadcast
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_BROADCAST, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pCfg->bSSIDBroadcast);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d bSSIDBroadcast %d\n", i, pCfg->bSSIDBroadcast);
	
	//Set ssid Vlan id
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VLANID, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pCfg->SSIDVlanID);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d SSIDVlanID %d\n", i, pCfg->SSIDVlanID);
	
	//Set ssid max clients
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_MAXCLIENTS, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pCfg->MaxClients);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d MaxClients %d\n", i, pCfg->MaxClients);
	
	//Set ssid lan base address
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_LANBASE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d.%d.%d.%d", pCfg->SSIDLanBase.Dot[0], pCfg->SSIDLanBase.Dot[1], pCfg->SSIDLanBase.Dot[2], pCfg->SSIDLanBase.Dot[3]);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d SSIDLanBase %s\n", i, buf);

	if(CosaDmlWiFiSSIDEncryptionSetCfg(hContext, pCfg->InstanceNumber, &(pCfg->EncryptionInfo)) != ANSC_STATUS_SUCCESS)
	{
		log_printf(LOG_ERR, "Set %d encryption info failed\n", i);
		return ANSC_STATUS_FAILURE;
	}
	return CosaDmlWiFiSSIDQoSSetCfg(hContext, pCfg->InstanceNumber, &(pCfg->QosInfo));
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSsidGetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_CFG     pCfg
    )
{
	int index = pCfg->InstanceNumber, tmp, i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF], *p, *p1;
	ANSC_STATUS ret;

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//Get real instance number	
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INS_NUM, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->InstanceNumber = atoi(buf);
	log_printf(LOG_INFO, "ssid entry %d instance number %s\n", i, buf);

	//Get alias
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ALIAS, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strcpy(pCfg->Alias, buf);
	log_printf(LOG_INFO,"alias is %s\n", buf);

	//Get ssid radio id
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_RADIOID, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strcpy(pCfg->WiFiRadioName, buf);
	log_printf(LOG_INFO,"radio id is %s\n", buf);

	//Get ssid index
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_INDEX, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strncpy(pCfg->SSIDIndex, buf, sizeof(pCfg->SSIDIndex) - 1);
	pCfg->SSIDIndex[sizeof(pCfg->SSIDIndex) - 1] = '\0';
	log_printf(LOG_INFO,"ssid index is %s\n", buf);
	
	//Get ssid name
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_NAME, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strcpy(pCfg->SSID, buf);
	log_printf(LOG_INFO,"ssid name is %s\n", buf);

	//Get ssid enable
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENABLE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->bEnabled = buf[0] == '0' ? false : true;
	log_printf(LOG_INFO,"ssid enable is %s\n", buf);
	
	//Get ssid broadcast
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_BROADCAST, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->bSSIDBroadcast = buf[0] == '0' ? false : true;
	log_printf(LOG_INFO,"ssid broadcast is %s\n", buf);	

	//Get ssid vlan id
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_VLANID, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->SSIDVlanID = atoi(buf);
	log_printf(LOG_INFO,"ssid vlan id is %s\n", buf);

	//Get ssid vlan base address
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_LANBASE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	log_printf(LOG_INFO,"ssid vlan lan base address is %s\n", buf);	
	//Parse string style ip address such as 10.0.0.1
	p = strchr(buf, '.');
	if(p == NULL)
	{
		log_printf(LOG_ERR, "lan base address is wrong\n");
		return ANSC_STATUS_FAILURE;
	}
	*p++ = '\0';
	pCfg->SSIDLanBase.Dot[0] = atoi(buf);
	p1 = strchr(p, '.');
	if(p1 == NULL)
	{
		log_printf(LOG_ERR, "lan base address is wrong\n");
		return ANSC_STATUS_FAILURE;
	}
	*p1++ = '\0';
	pCfg->SSIDLanBase.Dot[1] = atoi(p);
	p = strchr(p1, '.');
	if(p == NULL)
	{
		log_printf(LOG_ERR, "lan base address is wrong\n");
		return ANSC_STATUS_FAILURE;
	}
	*p++ = '\0';
	pCfg->SSIDLanBase.Dot[2] = atoi(p1);
	pCfg->SSIDLanBase.Dot[3] = atoi(p);

	//Get ssid vlan base address
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_MAXCLIENTS, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pCfg->MaxClients = atoi(buf);
	log_printf(LOG_INFO, "client limitation is %s\n", buf);
	
	//Get Encryption info, pass instance number
	ret = CosaDmlWiFiSSIDEncryptionGetCfg(hContext, index, &(pCfg->EncryptionInfo));
	if(ret != ANSC_STATUS_SUCCESS)
		return ret;	
	//Get QOS info, pass instance number
	return CosaDmlWiFiSSIDQoSGetCfg(hContext, index, &(pCfg->QosInfo)); 
}

ANSC_STATUS
CosaDmlWiFiSsidGetDinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_DINFO   pInfo
    )
{
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSsidGetSinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_SINFO   pInfo
    )
{
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSsidGetStats
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_STATS   pStats
    )
{
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDEncryptionGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_EncryptionInfo pEncryption
)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF], *p, *p1;
	int index, i = 0;

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	index = GetSsidIndexAsInstanceNumber(SSIDInstanceNumber);

	log_printf(LOG_INFO, "syscfg ssid index is %d", index);
	//Get encryption mode
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_MODE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pEncryption->ModeEnabled = atoi(buf);
	log_printf(LOG_INFO, "ssid security is %s\n", buf);

	//The other parameters are valid only when enable WiFiSecurity
	switch(buf[0])
	{
		//WEP-64
		case '2':
		//WEP-128
		case '3':
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_WEPKEY, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			strncpy(pEncryption->WepKey, buf, 128);
			log_printf(LOG_INFO, "ssid WEP key is %s\n", buf);
			break;

		//WPA-Personal
		case '4':
		//WPA2-Personal
		case '5':
		//WPA-WPA2-Personal
		case '6':
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_PREKEY, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			strncpy(pEncryption->PreSharedKey, buf, 128);
			log_printf(LOG_INFO, "ssid presharedkey is %s\n", buf);

			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_PASSPHASE, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			strncpy(pEncryption->Passphrase, buf, 128);
			log_printf(LOG_INFO, "ssid passphase is %s\n", buf);
			break;
		
		//WPA-Enterprise
		case '7':
		//WPA2-Enterprise
		case '8':
		//WPA-WPA2-Enterprise
		case '9':
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSIP, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			log_printf(LOG_INFO, "ssid radius ip is %s\n", buf);
			//Parse string style ip address such as 10.0.0.1
			p = strchr(buf, '.');
			if(p == NULL)
			{
				log_printf(LOG_ERR, "lan base address is wrong\n");
				return ANSC_STATUS_FAILURE;
			}
			*p++ = '\0';
			pEncryption->RadiusServerIP.Dot[0] = atoi(buf);
			p1 = strchr(p, '.');
			if(p1 == NULL)
			{
				log_printf(LOG_ERR, "lan base address is wrong\n");
				return ANSC_STATUS_FAILURE;
			}
			*p1++ = '\0';
			strncpy(pEncryption->Passphrase, buf, 128);
			pEncryption->RadiusServerIP.Dot[1] = atoi(p);
			p = strchr(p1, '.');
			if(p == NULL)
			{
				log_printf(LOG_ERR, "lan base address is wrong\n");
				return ANSC_STATUS_FAILURE;
			}
			*p++ = '\0';
			pEncryption->RadiusServerIP.Dot[2] = atoi(p1);
			pEncryption->RadiusServerIP.Dot[3] = atoi(p);

			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSPORT, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			pEncryption->RadiusServerPort = atoi(buf);
			log_printf(LOG_INFO, "ssid radius port is %s\n", buf);
			
			strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSSEC, sizeof(syscfg_indicator) - 2);
			syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
			sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
			syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
			strncpy(pEncryption->RadiusSecret, buf, 128);
			log_printf(LOG_INFO, "ssid radius secret is %s\n", buf);


	}
	//Get encryption type
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_TYPE, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pEncryption->Encryption = atoi(buf);
	log_printf(LOG_INFO, "ssid encryption is %s\n", buf);

	//Get key retry interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_INV, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pEncryption->RekeyInterval = atoi(buf);
	log_printf(LOG_INFO, "ssid encryption is %s\n", buf);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDEncryptionSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_EncryptionInfo pEncryption
)
{
	int index = SSIDInstanceNumber, i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	memset(buf, 0, sizeof(buf));
	
	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}
	
	//Set mode
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_MODE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pEncryption->ModeEnabled);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d ModeEnabled %d\n", i, pEncryption->ModeEnabled);

	//Set encryption
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_TYPE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pEncryption->Encryption);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d Encryption %d\n", i, pEncryption->Encryption);

	//Set wep key
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_WEPKEY, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pEncryption->WepKey);
	log_printf(LOG_INFO, "ssid entry %d WepKey %s\n", i, pEncryption->WepKey);

	//Set preshared key
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_PREKEY, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pEncryption->PreSharedKey);
	log_printf(LOG_INFO, "ssid entry %d PreSharedKey %s\n", i, pEncryption->PreSharedKey);

	//Set passphase
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_PASSPHASE, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pEncryption->Passphrase);
	log_printf(LOG_INFO, "ssid entry %d Passphrase %s\n", i, pEncryption->Passphrase);
	
	//Set key retry interval
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_INV, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pEncryption->RekeyInterval);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d RekeyInterval %d\n", i, pEncryption->RekeyInterval);

	//Set radius ip
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSIP, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d.%d.%d.%d", pEncryption->RadiusServerIP.Dot[0], pEncryption->RadiusServerIP.Dot[1], 
			pEncryption->RadiusServerIP.Dot[2],	pEncryption->RadiusServerIP.Dot[3]);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d radius ip %s\n", i, buf);

	//Set radius port
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSPORT, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pEncryption->RadiusServerPort);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d RadiusServerPort %d\n", i, pEncryption->RadiusServerPort);

	//Set radius secret
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_ENCRY_RSSEC, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_set(NULL, syscfg_indicator, pEncryption->RadiusSecret);
	log_printf(LOG_INFO, "ssid entry %d RadiusSecret %s\n", i, pEncryption->RadiusSecret);

	syscfg_commit();	
	if(notify_wecb(SSID_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDQoSGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosInfo pQos
)
{
	int index;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];
	
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	index = GetSsidIndexAsInstanceNumber(SSIDInstanceNumber);

	if(index >= MAX_SSID * 2)
	{
		log_printf(LOG_INFO, "can't find correspond ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_WMM, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQos->WMMEnable = (buf[0] == '0') ? 0 : 1;
	log_printf(LOG_INFO, "ssid entry %d WMM %s\n", index, buf);

	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_UAPSD, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", index);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQos->UAPSDEnable = (buf[0] == '0') ? 0 : 1;
	log_printf(LOG_INFO, "ssid entry %d UAPSD %s\n", index, buf);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDQoSSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosInfo pQos
)
{
	int index = SSIDInstanceNumber, i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	
	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//Set WMM
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_WMM, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pQos->WMMEnable);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d WMMEnable %d\n", i, pQos->WMMEnable);

	//Set UAPSD
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_UAPSD, sizeof(syscfg_indicator) - 3);
	syscfg_indicator[sizeof(syscfg_indicator) - 3] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	sprintf(buf, "%d", pQos->UAPSDEnable);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid entry %d UAPSDEnable %d\n", i, pQos->UAPSDEnable);

	syscfg_commit();	
	if(notify_wecb(SSID_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}

    return ANSC_STATUS_SUCCESS;
}

ULONG
CosaDmlWiFiSSIDQosSettingGetCount
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber
)
{
	/*
	ULONG i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	i = GetSsidIndexAsInstanceNumber(SSIDInstanceNumber);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return 0;
	}
	*/
	
	//CCSP framework doesn't support dynamica writable table.
	//So we need return 4 always.
	/*
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_WMM, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", i);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	
	if(buf[0] == '0')
	{
		log_printf(LOG_INFO, "SSID %d wmm is disabled\n", i);
		return 0;
	}
	else
	{
		log_printf(LOG_INFO, "SSID %d wmm is enabled\n", i);
		return SSID_QOS_NUM;
	}*/
        
	return SSID_QOS_NUM;
	return 0;
}

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingGetEntry
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    ULONG                       nIndex,                  /* Identified by Index */
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting
)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];
	ULONG current_ssid;

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	current_ssid = GetSsidIndexAsInstanceNumber(SSIDInstanceNumber);
	
	if(current_ssid >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssdi entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//Get qos settins instance number
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_INS_NUM, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->InstanceNumber = atoi(buf);
	// instance number should be 1 2 3 4
	log_printf(LOG_INFO, "ssid %d, qos %d, instance number %s\n", current_ssid, nIndex, buf);

    return CosaDmlWiFiSSIDQosSettingGetCfg(NULL, SSIDInstanceNumber, pQosSetting);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting          /* Identified by Instance Number */
)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];
	ULONG current_ssid, nIndex = pQosSetting->InstanceNumber - 1;

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	current_ssid = GetSsidIndexAsInstanceNumber(SSIDInstanceNumber);
	
	if(current_ssid >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_QOS_WMM, sizeof(syscfg_indicator) - 2);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", current_ssid);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	
	if(buf[0] == '0')
	{
		log_printf(LOG_INFO, "SSID %d wmm is disabled, no need to get cfg\n", current_ssid);
		return ANSC_STATUS_SUCCESS;
	}
		
	log_printf(LOG_INFO, "SSID %d wmm is enabled, get actual settings\n", current_ssid);

	//Get ssid instance number
	pQosSetting->SSIDInstanceNumber = SSIDInstanceNumber;

	//Get qos settins AC
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AC, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->AC = atoi(buf);
	log_printf(LOG_INFO, "ssid %d, qos %d, AC %s\n", current_ssid, nIndex, buf);

	//Get qos settins ACM
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_ACM, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->ACM = buf[0] == '0' ? false : true;
	log_printf(LOG_INFO, "ssid %d, qos %d, ACM %s\n", current_ssid, nIndex, buf);

	//Get qos settins AIFSN
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AIFSN, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->AIFSN = atoi(buf);
	log_printf(LOG_INFO, "ssid %d, qos %d, AIFSN %s\n", current_ssid, nIndex, buf);

	//Get qos settins CWMin
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMIN, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->CWMin = atoi(buf);
	log_printf(LOG_INFO, "ssid %d, qos %d, CWMin %s\n", current_ssid, nIndex, buf);
	
	//Get qos settins CWMax
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMAX, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->CWMax = atoi(buf);
	log_printf(LOG_INFO, "ssid %d, qos %d, CWMax %s\n", current_ssid, nIndex, buf);

	//Get qos settins TXOPLimit
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_TXOP, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->TXOPLimit = atoi(buf);
	log_printf(LOG_INFO, "ssid %d, qos %d, TXOPLimit %s\n", current_ssid, nIndex, buf);

	//Get qos settins NoACK
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_NOACK, current_ssid);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", nIndex);
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pQosSetting->NoACK = buf[0] == '0' ? false : true;
	log_printf(LOG_INFO, "ssid %d, qos %d, NoACK %s\n", current_ssid, nIndex, buf);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting          /* Identified by Instance Number */
)
{
	int index = SSIDInstanceNumber, i;
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }
	
	i = GetSsidIndexAsInstanceNumber(index);
	
	if(i >= MAX_SSID * 2)
	{
		log_printf(LOG_ERR, "Can't find valid ssid entry\n");
		return ANSC_STATUS_FAILURE;
	}

	//Set qos settings instance number
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_INS_NUM, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->InstanceNumber);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, instance number %d\n", i, pQosSetting->InstanceNumber);

	//Set qos settings ssid instance number
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_SSIDINS_NUM, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", SSIDInstanceNumber);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, SSIDInstanceNumber %d\n", i, SSIDInstanceNumber);

	//Set qos settings AC
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AC, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->AC);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, AC %d\n", i, pQosSetting->AC);

	//Set qos settings ACM
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_ACM, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->ACM);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, ACM %d\n", i, pQosSetting->ACM);

	//Set qos settings AIFSN
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AIFSN, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->AIFSN);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, instance number %d\n", i, pQosSetting->AIFSN);

	//Set qos settings CWMin
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMIN, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->CWMin);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, CWMin %d\n", i, pQosSetting->CWMin);
	
	//Set qos settings CWMax
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMAX, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->CWMax);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, CWMax %d\n", i, pQosSetting->CWMax);

	//Set qos settings TXOPLimit
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_TXOP, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->TXOPLimit);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, TXOPLimit %d\n", i, pQosSetting->TXOPLimit);

	//Set qos settings NoACK
	snprintf(syscfg_indicator, sizeof(syscfg_indicator) - 2, SYSCFG_MOCA_EXT_SSID_QOS_SETTING_NOACK, i);
	syscfg_indicator[sizeof(syscfg_indicator) - 2] = '\0';
	sprintf(syscfg_indicator + strlen(syscfg_indicator), "%d", pQosSetting->InstanceNumber - 1);
	sprintf(buf, "%d", pQosSetting->NoACK);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "ssid %d, NoACK %d\n", i, pQosSetting->NoACK);

	syscfg_commit();	
	if(notify_wecb(SSID_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiWPSGetCfg
(
    ANSC_HANDLE                 hContext,
    PCOSA_DML_WIFI_WPS          pWPS
)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	//Get wps enable
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_ENABLE, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	pWPS->bEnabled = buf[0] == '0' ? 0 : 1;
	log_printf(LOG_INFO, "wps enable is %s\n", buf);

	//Get wps pin
	memset(buf, 0, sizeof(buf));
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_PIN, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	memcpy(pWPS->X_CISCO_COM_Pin, buf, sizeof(pWPS->X_CISCO_COM_Pin));
	log_printf(LOG_INFO, "wps pin is %s %s\n", buf, pWPS->X_CISCO_COM_Pin);
	
	//Get wps ssid index 
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_SSID_INDEX, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strncpy(pWPS->SSIDIndex, buf, sizeof(pWPS->SSIDIndex) - 1);
	pWPS->SSIDIndex[sizeof(pWPS->SSIDIndex) - 1] = '\0';
	log_printf(LOG_INFO, "wps ssid index is %s\n", buf);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
CosaDmlWiFiWPSSetCfg
(
    ANSC_HANDLE                 hContext,
    PCOSA_DML_WIFI_WPS          pWPS
)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	//Set wps enable
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_ENABLE, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	sprintf(buf, "%d", pWPS->bEnabled);
	syscfg_set(NULL, syscfg_indicator, buf);
	log_printf(LOG_INFO, "wps enable is %d\n", pWPS->bEnabled);

	//Set wps pin
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_PIN, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_set(NULL, syscfg_indicator, pWPS->X_CISCO_COM_Pin);
	log_printf(LOG_INFO, "wps pin %s", pWPS->X_CISCO_COM_Pin);

	//Set wps ssid index
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_SSID_WPS_SSID_INDEX, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	//sprintf(buf, "%d", pWPS->SSIDIndex);
	syscfg_set(NULL, syscfg_indicator, pWPS->SSIDIndex);
	log_printf(LOG_INFO, "wps ssid index is %s\n", pWPS->SSIDIndex);

	syscfg_commit();	
	if(notify_wecb(WPS_SET, NULL))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
	}


    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS	
CosaDmlWiFi_SetDisconnectClients(char *p)
{
	char syscfg_indicator[MAX_BUF];

	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	//Set Disconnect clients
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_WIFI_DISCONNECT_CLIENTS, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';
	syscfg_set(NULL, syscfg_indicator, p);
	syscfg_commit();
	log_printf(LOG_INFO, "WiFi disconnect clients are %s\n", p);

    return ANSC_STATUS_SUCCESS; 
}

ANSC_STATUS	
CosaDmlWiFi_GetDisconnectClients(char *p)
{
	char syscfg_indicator[MAX_BUF];
	char buf[MAX_BUF];

	memset(buf, 0, sizeof(buf));
	if (syscfg_init() == -1) 
	{
        log_printf(LOG_ERR, "syscfg or sysevent init failed\n");
        return ANSC_STATUS_FAILURE; 
    }

	//Get WiFi Disconnect clients
	strncpy(syscfg_indicator, SYSCFG_MOCA_EXT_WIFI_DISCONNECT_CLIENTS, sizeof(syscfg_indicator) - 1);
	syscfg_indicator[sizeof(syscfg_indicator) - 1] = '\0';

	syscfg_get(NULL, syscfg_indicator, buf, sizeof(buf));
	strcpy(p, buf);
	log_printf(LOG_INFO, "WiFi disconnect clients are %s\n", buf);
    return ANSC_STATUS_SUCCESS; 
}

ANSC_STATUS	
CosaDmlWiFi_GetExtStatus(int *ext_count, ANSC_HANDLE ext_status)
{
	if (ext_count == NULL || ext_status == NULL)
	{
		log_printf(LOG_ERR, "unexpected parameters\n");
		return ANSC_STATUS_FAILURE;
	}
	struct ExtInfo ext_info;
	ext_info.ext_count = ext_count;
	ext_info.ext_status = ext_status;

	if(notify_wecb(QUERY_ALL, &ext_info))
	{
		log_printf(LOG_ERR, "communication with WECB failed\n");
		return ANSC_STATUS_FAILURE;
	}
	return ANSC_STATUS_SUCCESS;

}

