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

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hdk.h"
#include "autoconf.h"
#include "hdk_data.h"
#include "../wecb_log.h"
#include "../wecb_util.h"
#include "../wecb_common.h"
#include "hdk_mbus.h"
#include "hnap_device.h"
#include "../wecb_hnap.h"
#include "syscfg/syscfg.h"
#include "ccsp_hal_ethsw.h"

static MBusObj_t *mbus = NULL;
//static HDK_ClientContext clientCtx;
static int iTimeoutmSecs = 10000;
static int iTimeoutmSecs_al = 10000;

char* all_wecb_ports[5]= 
	{
		"Device.MoCA.Interface.1",
		"Device.Ethernet.Interface.1",
		"Device.Ethernet.Interface.2",
		"Device.Ethernet.Interface.3",
		"Device.Ethernet.Interface.4"
	};

bool mbus_init()
{
	mbus =  MBus_Create(MBUS_SUBSYSTEM, MBUS_CONF_FILE, MBUS_COMPID_HDK, MBUS_COMPID_CR);

	if(mbus == NULL)
	{
		log_printf(LOG_ERR, "ccsp message initialize failed\n");
		return false;
	}
	return true;
}

bool mbus_uninit()
{
	if(mbus != NULL)
		MBus_Destroy(mbus);
	return true;
}


void* clientctx_init(const char *url, const char *user, const char *pw)
{
	if(!url || !user || !pw)
	{
		log_printf(LOG_ERR, "Input parameters error\n");
		return NULL;
	}

	HDK_ClientContext *pCtx = (HDK_ClientContext *)malloc(sizeof(HDK_ClientContext));

	if(pCtx == NULL)
	{
		log_printf(LOG_ERR, "malloc client context failed\n");
		return NULL;
	}


    memset(pCtx, 0, sizeof(HDK_ClientContext));

	//copy hnap url username password to clientctx struct
	pCtx->pszURL = (char *)malloc(strlen(url) + 1);

	if(pCtx->pszURL == NULL)
	{
		log_printf(LOG_ERR, "Memory error\n");
		goto EXIT;
	}
	strcpy(pCtx->pszURL, url);


	pCtx->pszUsername = (char *)malloc(strlen(user) + 1);

	if(pCtx->pszUsername == NULL)
	{
		log_printf(LOG_ERR, "Memory error\n");
		goto EXIT;
	}
	strcpy(pCtx->pszUsername, user);

	pCtx->pszPassword = (char *)malloc(strlen(pw) + 1);

	if(pCtx->pszPassword == NULL)
	{
		log_printf(LOG_ERR, "Memory error\n");
		goto EXIT;
	}
	strcpy(pCtx->pszPassword, pw);
	
	/*
	if(!HDK_Client_Http_Init())
	{
		log_printf(LOG_ERR, "init http client error\n");
		goto EXIT;
	}
	*/
	return pCtx;

EXIT:
	if(pCtx->pszURL != NULL)
	{
		free(pCtx->pszURL);
		pCtx->pszURL = NULL;
	}

	if(pCtx->pszUsername != NULL)
	{
		free(pCtx->pszUsername);
		pCtx->pszUsername = NULL;
	}
	
	if(pCtx->pszPassword != NULL)
	{
		free(pCtx->pszPassword);
		pCtx->pszPassword = NULL;
	}

	if(pCtx)
	{
		free(pCtx);
		pCtx = NULL;
	}
	return NULL;	
}

bool clientctx_uninit(HDK_ClientContext *pCtx)
{
	if(pCtx == NULL)
	{
		return false;
	}

	if(pCtx->pszURL != NULL)
	{
		free(pCtx->pszURL);
		pCtx->pszURL = NULL;
	}

	if(pCtx->pszUsername != NULL)
	{
		free(pCtx->pszUsername);
		pCtx->pszUsername = NULL;
	}
	
	if(pCtx->pszPassword != NULL)
	{
		free(pCtx->pszPassword);
		pCtx->pszPassword = NULL;
	}

	if(pCtx)
	{
		free(pCtx);
	}
		
	//HDK_Client_Http_Cleanup();

	return true;	
}

bool SetTOD(HDK_ClientContext *pCtx, void *rt)
{
	HDK_Struct input;
	time_t cur_time;
	struct tm *p_time;
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;


	//Get current time using native linxu api
	cur_time = time(NULL);

	if(cur_time == (time_t)-1)
	{
		log_printf(LOG_ERR, "Get native time failed\n");
		return false;
	}
	p_time = localtime(&cur_time);

	HDK_Struct_Init(&input);

	HDK_Set_Int(&input, HDK_Element_Cisco_Seconds, p_time->tm_sec);
	HDK_Set_Int(&input, HDK_Element_Cisco_Minutes, p_time->tm_min);
	HDK_Set_Int(&input, HDK_Element_Cisco_Hour, p_time->tm_hour);
	HDK_Set_Int(&input, HDK_Element_Cisco_MonthDay, p_time->tm_mday);
	HDK_Set_Int(&input, HDK_Element_Cisco_Month, p_time->tm_mon);
	HDK_Set_Int(&input,	HDK_Element_Cisco_Year, p_time->tm_year);
	HDK_Set_Int(&input, HDK_Element_Cisco_YDay, p_time->tm_yday);
	HDK_Set_Int(&input, HDK_Element_Cisco_WDay, p_time->tm_wday);
	
	if(p_time->tm_isdst > 0)
	{
		HDK_Set_Bool(&input, HDK_Element_Cisco_Dst, true);
	}
	else if(p_time->tm_isdst == 0)
	{
		HDK_Set_Bool(&input, HDK_Element_Cisco_Dst, false);
	}
	else
	{
		log_printf(LOG_INFO, "dst info is unknown, treat as disable\n");
		HDK_Set_Bool(&input, HDK_Element_Cisco_Dst, false);
	}
	HDK_Set_Int(&input, HDK_Element_Cisco_Epoch, cur_time);

	error = HDK_ClientMethod_Cisco_SetTOD(pCtx, iTimeoutmSecs, &input, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "SetTOD success\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "SetTOD returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

	HDK_Struct_Free(&input);
	return ret;
}

bool IsDeviceReady(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;

	error = HDK_ClientMethod_PN_IsDeviceReady(pCtx, iTimeoutmSecs, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "Device is ready\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "IsDeviceReady returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

	return ret;
}

bool SetDoRestart(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;

	error = HDK_ClientMethod_Cisco_SetDoRestart(pCtx, iTimeoutmSecs_al, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "Restart success\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "SetDoRestart returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

	return ret;
}	

bool GetDeviceSettings(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct output;
	struct ExtStatus *p_ext_info = (struct ExtStatus *)rt;
	char *pStr = NULL;

	HDK_Struct_Init(&output);
	
	error = HDK_ClientMethod_PN_GetDeviceSettings(pCtx, iTimeoutmSecs, &output, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "GetDeviceSettings success\n");
				ret = true;

				if(p_ext_info == NULL)
				{
					log_printf(LOG_WARNING, "Reach maximum extender\n");
					goto EXIT;
				}
	
				if (p_ext_info != NULL)
				{
					pStr = HDK_Get_String(&output, HDK_Element_PN_DeviceName);
					if (pStr != NULL)
					{
						strncpy(p_ext_info->device_name, pStr, sizeof(p_ext_info->device_name) - 1);
						p_ext_info->device_name[sizeof(p_ext_info->device_name) - 1] = '\0';
					}
					else
						log_printf(LOG_WARNING, "No DeviceName gotten\n");

					pStr = HDK_Get_String(&output, HDK_Element_PN_VendorName);
					if (pStr != NULL)
					{
						strncpy(p_ext_info->vendor_name, pStr, sizeof(p_ext_info->vendor_name) - 1);
						p_ext_info->vendor_name[sizeof(p_ext_info->vendor_name) - 1] = '\0';
					}
					else
						log_printf(LOG_WARNING, "No VendorName gotten\n");
					
					pStr = HDK_Get_String(&output, HDK_Element_PN_ModelName);
					if (pStr != NULL)
					{
						strncpy(p_ext_info->model_name, pStr, sizeof(p_ext_info->model_name) - 1);
						p_ext_info->model_name[sizeof(p_ext_info->model_name) - 1] = '\0';
					}
					else
						log_printf(LOG_WARNING, "No ModelName gotten\n");

					pStr = HDK_Get_String(&output, HDK_Element_PN_FirmwareVersion);
					if (pStr != NULL)
					{
						strncpy(p_ext_info->fw_version, pStr, sizeof(p_ext_info->fw_version) - 1);
						p_ext_info->fw_version[sizeof(p_ext_info->fw_version) - 1] = '\0';
					}
					else
						log_printf(LOG_WARNING, "No FirmwareVersion gotten\n");
				}
			}
			else
			{
				log_printf(LOG_ERR, "GetDeviceSettings returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}
EXIT:	
	HDK_Struct_Free(&output);
	return ret;
}	

bool GetWlanRadios(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct output;

	if(rt == NULL)
	{
		log_printf(LOG_ERR, "error input handle\n");
		return false;
	}
	
	HDK_Struct_Init(&output);
	
	error = HDK_ClientMethod_PN_GetWLanRadios(pCtx, iTimeoutmSecs, &output, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "GetWlanRadios success\n");

				//@phase-1, we only care the radio number
				HDK_Struct *pStr = NULL, *pTmp = NULL, *pStr1 = NULL;
				HDK_Member *pMem = NULL, *pMem1 = NULL;
				int *tmp;
				HDK_Enum_PN_WiFiMode *wifi_mode;
				struct radio_cfg *cfg = (struct radio_cfg *)rt;
				memset(cfg, 0, sizeof(struct radio_cfg));

				pStr = HDK_Get_Struct(&output, HDK_Element_PN_RadioInfos);
				
				if(pStr != NULL)
				{
					for(pMem = pStr->pHead; pMem != NULL; pMem = pMem->pNext)
					{
						pTmp = HDK_Get_StructMember(pMem);

						if(pTmp != NULL)
						{
							tmp = HDK_Get_Int(pTmp, HDK_Element_PN_Frequency);
							if(tmp !=  NULL)
							{
								//2.4G existed, correspond TR181 instance number 1
								if(*tmp == 2)
									cfg->band_mask |= 0x1;
								//5G existed, instance number 2
								if(*tmp == 5)
									cfg->band_mask |= 0x2;
							}
							pStr1 = HDK_Get_Struct(pTmp, HDK_Element_PN_SupportedModes);
						
							if(pStr1 != NULL)
							{
								for(pMem1 = pStr1->pHead; pMem1 != NULL; pMem1 = pMem1->pNext)
								{
									wifi_mode = HDK_Get_PN_WiFiModeMember(pMem1);

									//printf("wifi mode is %d\n", *wifi_mode);
									if(wifi_mode != NULL && (*wifi_mode == HDK_Enum_PN_WiFiMode_802_11ac))
									{
										if(tmp && *tmp == 2)
											cfg->mode_mask |= 0x2;
										
										if(tmp && *tmp == 5)
											cfg->mode_mask |= 0x1;
									}
								}
							}	
						}
					}
				}
				else
				{
					log_printf(LOG_ERR, "can't get RadioInfos\n");
				}
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "GetWlanRadios returns %d\n", result);
				*(int *)rt = 0;
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}
	
	HDK_Struct_Free(&output);
	return ret;
}	

bool GetExtenderStatus(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct output;
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL, *pStr3 = NULL;
	HDK_Member *pMember = NULL;
	char *p = NULL;
	int *bEnable = NULL, *bChannel;
	struct ExtStatus *p_ext_info = (struct ExtStatus *)rt;
	HDK_Enum_Cisco_WiFiMode *operating_mode;
	HDK_Enum_Cisco_WiFiMode mode24 = HDK_Enum_Cisco_WiFiMode__UNKNOWN__, mode5 = HDK_Enum_Cisco_WiFiMode__UNKNOWN__;
	int channel24 = 0, channel5 = 0, i = 0;
	HDK_Enum_Cisco_WiFiEncryption *wifi_encry;
	HDK_Enum_Cisco_WiFiSecurity *wifi_sec;
		
	HDK_Struct_Init(&output);
	
	error = HDK_ClientMethod_Cisco_GetExtenderStatus(pCtx, iTimeoutmSecs, &output, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "GetExtenderStatus success\n");
				ret = true;

				if(p_ext_info == NULL)
				{
					log_printf(LOG_WARNING, "Reach maximum extender\n");
					goto EXIT;
				}
				
				//traverse radio information
				pStr1 = HDK_Get_Struct(&output, HDK_Element_Cisco_RadioList);

				if(pStr1 == NULL)
				{
					ret = false;
					log_printf(LOG_ERR, "can't find radio list information\n");
					break;
				}

				p_ext_info->radio_mask = 0;
				for (pMember = pStr1->pHead; pMember != NULL; pMember = pMember->pNext)
				{
					if ((pStr2 = HDK_Get_StructMember(pMember)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find radio settings information\n");
					 	continue;
					}

					if ((p = HDK_Get_String(pStr2, HDK_Element_Cisco_RadioID)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find radio ID information\n");
						continue;
					}

					if ((bEnable = HDK_Get_Bool(pStr2, HDK_Element_Cisco_Enable)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find radio enable information\n");
						continue;
					}

					if(*bEnable == true)
					{
						if(strstr(p, "2.4GHz"))
						{
							p_ext_info->radio_mask |= 0x10000;
						}

						if(strstr(p, "5GHz"))
						{
							p_ext_info->radio_mask |= 0x1;
						}

					}

					operating_mode = HDK_Get_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode);
					if(operating_mode == NULL)
					{
						log_printf(LOG_WARNING, "can't find radio mode information\n");
						continue;
					}
					

					if ((bChannel = HDK_Get_Int(pStr2, HDK_Element_Cisco_Channel)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find radio channel information\n");
						continue;
					}


					if(strstr(p, "2.4GHz"))
					{
						mode24 = *operating_mode;
						channel24 = *bChannel;
					}

					if(strstr(p, "5GHz"))
					{
						mode5 = *operating_mode;
						channel5 = *bChannel;
					}
				}

				//traverse ssid list 
				pStr1 = HDK_Get_Struct(&output, HDK_Element_Cisco_SSIDList);
				if(pStr1 == NULL)
				{
					ret = false;
					log_printf(LOG_ERR, "can't find ssid list information\n");
					break;
				}

				for (i = 0, pMember = pStr1->pHead; pMember != NULL && i < MAX_EXT_SSID; pMember = pMember->pNext)
				{
					if((pStr2 = HDK_Get_StructMember(pMember)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid settings information\n");
					 	continue;
					}

					if((pStr3 = HDK_Get_Struct(pStr2, HDK_Element_Cisco_SSIDEncryption)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid encryption information\n");
					 	continue;
					}

					if ((bEnable = HDK_Get_Bool(pStr2, HDK_Element_Cisco_SSIDEnabled)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid enable information\n");
						continue;
					}

					if (*bEnable != true)
					{
						log_printf(LOG_WARNING, "ssid is disabled, skip it\n");
						continue;
					}

					if((p = HDK_Get_String(pStr2, HDK_Element_Cisco_SSID)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid name\n");
						continue;
					}

					log_printf(LOG_INFO, "Extender return SSID name is %s\n", p);
					strncpy(p_ext_info->ssid[i].ssid, p, sizeof(p_ext_info->ssid[0].ssid) - 1);
					p_ext_info->ssid[i].ssid[sizeof(p_ext_info->ssid[0].ssid) - 1] = '\0';
					
					if((p = HDK_Get_String(pStr2, HDK_Element_Cisco_BSSID)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find bssid\n");
						continue;
					}
					
					log_printf(LOG_INFO, "Extender return BSSID name is %s\n", p);
					strncpy(p_ext_info->ssid[i].bssid, p, sizeof(p_ext_info->ssid[0].bssid) - 1);
					p_ext_info->ssid[i].bssid[sizeof(p_ext_info->ssid[0].bssid) - 1] = '\0';

					if((p = HDK_Get_String(pStr2, HDK_Element_Cisco_SSIDRadioID)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid radio id\n");
						continue;
					}
				
					if((wifi_sec = HDK_Get_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ssid security\n");
						continue;
					}
					
					p_ext_info->ssid[i].sec_mode = *wifi_sec;

					if(*wifi_sec == HDK_Enum_Cisco_WiFiSecurity_None)
					{
						p_ext_info->ssid[i].encry_mode = HDK_Enum_Cisco_WiFiEncryption__UNKNOWN__;
					}
					else if(*wifi_sec == HDK_Enum_Cisco_WiFiSecurity_WEP_64)
					{
						p_ext_info->ssid[i].encry_mode = HDK_Enum_Cisco_WiFiEncryption_WEP_64;
					}
					else if(*wifi_sec == HDK_Enum_Cisco_WiFiSecurity_WEP_128)
					{
						p_ext_info->ssid[i].encry_mode = HDK_Enum_Cisco_WiFiEncryption_WEP_128;
					}
					else
					{
						if((wifi_encry = HDK_Get_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption)) == NULL)
						{
							log_printf(LOG_WARNING, "can't find ssid encryption\n");
							continue;
						}

						p_ext_info->ssid[i].encry_mode = *wifi_encry;
					}

					if(strstr(p, "2.4GHz"))
					{
						log_printf(LOG_INFO, "Extender return SSID is 2.4G\n");
						p_ext_info->ssid[i].channel = channel24;
						p_ext_info->ssid[i].mode = mode24;
						p_ext_info->ssid[i].band = 1;
					}

					if(strstr(p, "5GHz"))
					{
						log_printf(LOG_INFO, "Extender return SSID is 5G\n");
						p_ext_info->ssid[i].channel = channel5;
						p_ext_info->ssid[i].mode = mode5;
						p_ext_info->ssid[i].band = 2;
					}

					i++;
				}
				p_ext_info->ext_ssid_num = i;

			}
			else
			{
				log_printf(LOG_ERR, "GetExtenderStatus returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}
EXIT:	
	HDK_Struct_Free(&output);
	return ret;
}	


bool GetClientInfo(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct output;
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL;
	HDK_Member *pMember = NULL;
	char *p = NULL;
	HDK_MACAddress* pMACAddress = NULL;
	int i = 0;
	struct ExtStatus *p_ext_info = (struct ExtStatus *)rt;
	HDK_Enum_Cisco_DeviceInf *pInf = NULL;
		
	HDK_Struct_Init(&output);
	
	error = HDK_ClientMethod_Cisco_GetClientInfo(pCtx, iTimeoutmSecs, &output, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "GetClientInfo success\n");
				ret = true;

				if(p_ext_info == NULL)
				{
					log_printf(LOG_WARNING, "Reach maximum extender\n");
					goto EXIT;
				}

				//now we only focus on the radio enable paramter, using rt as a flag
				pStr1 = HDK_Get_Struct(&output, HDK_Element_Cisco_ClientInfoLists);

				if(pStr1 == NULL)
				{
					ret = false;
					log_printf(LOG_ERR, "can't find client list information\n");
					break;
				}

				for (i = 0, pMember = pStr1->pHead; pMember != NULL && i < MAX_EXT_CLIENT; pMember = pMember->pNext)
				{
					if ((pStr2 = HDK_Get_StructMember(pMember)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find ClientInfo struct\n");
					 	continue;
					}

					if ((p = HDK_Get_String(pStr2, HDK_Element_Cisco_DeviceName)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find clientinfo device name\n");
						//continue;
					}

					if((pMACAddress = HDK_Get_MACAddress(pStr2, HDK_Element_Cisco_MACAddress)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find clientinfo mac address\n");
						continue;
					}
					
					if((pInf = HDK_Get_Cisco_DeviceInf(pStr2, HDK_Element_Cisco_Type)) == NULL)
					{
						log_printf(LOG_WARNING, "can't find clientinfo device interface\n");
						continue;
					}

					p_ext_info->clients[i].inf = *pInf;
					if(p != NULL)
						strncpy(p_ext_info->clients[i].name, p, sizeof(p_ext_info->clients[i].name) - 1);
					else
						strcpy(p_ext_info->clients[i].name, "");
					p_ext_info->clients[i].name[sizeof(p_ext_info->clients[i].name) - 1] = '\0';
					
					sprintf(p_ext_info->clients[i].mac, "%02X:%02X:%02X:%02X:%02X:%02X", pMACAddress->a,  pMACAddress->b, 
							pMACAddress->c, pMACAddress->d, pMACAddress->e, pMACAddress->f);
					i++;
				}
				p_ext_info->ext_client_num = i;
			}
			else
			{
				log_printf(LOG_ERR, "GetClientInfo returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}
EXIT:	
	HDK_Struct_Free(&output);
	return ret;
}	

bool SetRadios(HDK_ClientContext *pCtx , void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct input;
	int mask, i, *radio_down = rt;
	char radio_path[MAX_BUF], val[MAX_BUF];
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL;
	struct radio_cfg cfg;
	
	
	if(GetWlanRadios(pCtx, &cfg) == false)
	{
		log_printf(LOG_ERR, "GetWlanRaios failed\n");
		return false;
	}

	//radio_mask = 0x3;

	if(mbus == NULL)
	{
		log_printf(LOG_ERR, "Why ccsp bus handle is invalid\n");
		return false;
	}
	
	HDK_Struct_Init(&input);
	pStr1 = HDK_Set_Struct(&input, HDK_Element_Cisco_RadioList);

	for(i = 0; cfg.band_mask != 0; i++)
	{
		mask = 0;
		if((cfg.band_mask & 0x1) == 0)
		{
			log_printf(LOG_INFO, "No 2.4G radio found\n");
		}
		else 
		{
			mask = 0x1;
			cfg.band_mask &= 0x2;
		}

		if(!mask) 
		{
			if((cfg.band_mask & 0x2) == 0)
			{
				log_printf(LOG_INFO, "No 5G radio found\n");
				continue;
			}
			else 
			{
				mask = 0x2;
				cfg.band_mask &= 0x1;
			}
		}
		if(i == 0)
			pStr2 = HDK_Set_Struct(pStr1, HDK_Element_Cisco_RadioSettings);
		else
			pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_RadioSettings);
		memset(radio_path, 0, sizeof(radio_path));
		strncpy(radio_path, USG_PRE, sizeof(radio_path));
		strcat(radio_path, "Radio.");

		if(mask == 0x1)
		{
			HDK_Set_String(pStr2, HDK_Element_Cisco_RadioID, "RADIO_2.4GHz");
		}
		else if(mask == 0x2)
		{
			HDK_Set_String(pStr2, HDK_Element_Cisco_RadioID, "RADIO_5GHz");
		}

		if(GetParamValueForIns(mbus, radio_path, "Enable", mask, val, sizeof(val)))
		{
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_Enable, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "Radio %d Enable is %s\n", mask, val);
		}
		else
		{
			log_printf(LOG_ERR, "cant't get radio %d Enable\n", mask);
			goto EXIT;
		}
	
		if(radio_down && *radio_down == 1)
		{
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_Enable, false);
		}	

		if(GetParamValueForIns(mbus, radio_path, "OperatingStandards", mask, val, sizeof(val)))
		{
			if(strlen(val)  == 1)
			{
				switch(val[0])
				{
					case 'a':
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11a);
						log_printf(LOG_INFO, "Radio %d mode is 802.11a\n", mask);
						break;
					case 'b':
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11b);
						log_printf(LOG_INFO, "Radio %d mode is 802.11b\n", mask);
						break;
					case 'g':
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11g);
						log_printf(LOG_INFO, "Radio %d mode is 802.11g\n", mask);
						break;
					case 'n':
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11n);
						log_printf(LOG_INFO, "Radio %d mode is 802.11n\n", mask);
						break;
					default:
						log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
						goto EXIT;
				}
			}
			//ac
			else if(strlen(val) == 2)
			{
				if(cfg.mode_mask)
				{
					HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11ac);
				}
				else
					HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11n);

				log_printf(LOG_INFO, "Radio %d mode is 802.11ac\n", mask);
			}
			//such as a,n b,g b,n g,n a,n
			else if(strlen(val) == 3)
			{
				if(strchr(val, 'n'))
				{
					if(strchr(val, 'a'))
					{
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11an);
						log_printf(LOG_INFO, "Radio %d mode is 802.11an\n", mask);
					}
					else if(strchr(val, 'b'))
					{
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11bn);
						log_printf(LOG_INFO, "Radio %d mode is 802.11bn\n", mask);
					}
					else if(strchr(val, 'g'))
					{
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11gn);
						log_printf(LOG_INFO, "Radio %d mode is 802.11gn\n", mask);
					}
					else
					{
						log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
						goto EXIT;
					}
				}
				else
				{
					if(strchr(val, 'g') && strchr(val, 'n'))
					{
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11bg);
						log_printf(LOG_INFO, "Radio %d mode is 802.11bg\n", mask);
					}
					else
					{
						log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
						goto EXIT;
					}
				}	
			}
			//such as "n,ac", "a,ac"
			else if(strlen(val) == 4)
			{
				if(strstr(val, "ac") && strchr(val, 'n'))
				{
					if(cfg.mode_mask)
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11nac);
					else
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11n);
					log_printf(LOG_INFO, "Radio %d mode is 802.11nac\n", mask);
				}
				else
				{
					if(cfg.mode_mask)
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11aac);
					else
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11a);
					log_printf(LOG_INFO, "Radio %d mode is 802.11aac\n", mask);
				}
			}
			else if(strlen(val) == 5)
			{
				if(strchr(val, 'g') && strchr(val, 'b') && strchr(val, 'n'))
				{
					HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11bgn);
					log_printf(LOG_INFO, "Radio %d mode is 802.11bgn\n", mask);
				}
				else
				{
					log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
					goto EXIT;
				}
			}
			//such as "g,n,ac", "a,n,ac"
			else if(strlen(val) == 6)
			{
				if(strstr(val, "ac") && strchr(val, 'n') && strchr(val, 'g'))
				{
					if(cfg.mode_mask)
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11gnac);
					else
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11gn);
					log_printf(LOG_INFO, "Radio %d mode is 802.11gnac\n", mask);
				}
				else
				{
					if(cfg.mode_mask)
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11anac);
					else
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11an);
					log_printf(LOG_INFO, "Radio %d mode is 802.11anac\n", mask);
				}
			}
			//"b,g,n,ac"
			else if(strlen(val) == 8)
			{
				if(strstr(val, "ac") && strchr(val, 'n') && strchr(val, 'b') && strchr(val, 'g'))
				{
					if(cfg.mode_mask)
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11bgnac);
					else
						HDK_Set_Cisco_WiFiMode(pStr2, HDK_Element_Cisco_Mode, HDK_Enum_Cisco_WiFiMode_802_11bgn);
					log_printf(LOG_INFO, "Radio %d mode is 802.11bgnac\n", mask);
				}
				else
				{
					log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
					goto EXIT;
				}
			}
			else
			{
				log_printf(LOG_ERR, "Radio %d mode unknown\n", mask);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get radiod %d mode\n", mask);
			goto EXIT;
		}

		if(GetParamValueForIns(mbus, radio_path, "OperatingChannelBandwidth", mask, val, sizeof(val)))
		{	
			if(!strcmp(val, "20MHz"))
			{
				log_printf(LOG_INFO, "radio %d channel width is 20MHz\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 1);
			}
			else if(!strcmp(val, "40MHz"))
			{
				log_printf(LOG_INFO, "radio %d channel width is 40MHz\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 2);
				//WECB doesn;t support 40M, so we will pass it as Auto
				//HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 3);
			}
			else if(!strcmp(val, "Auto"))
			{
				log_printf(LOG_INFO, "radio %d channel width is Auto\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 3);
			}
			else if(!strcmp(val, "80MHz"))
			{
				log_printf(LOG_INFO, "radio %d channel width is 80MHz\n", mask);
				if(cfg.band_mask)
					HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 4);
				else
					HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 3);
			}
			else if(!strcmp(val, "160MHz"))
			{
				log_printf(LOG_INFO, "radio %d channel width is 160MHz\n", mask);
				if(cfg.band_mask)
					HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 5);
				else
					HDK_Set_Int(pStr2, HDK_Element_Cisco_ChannelWidth, 3);
			}
			else
			{
				log_printf(LOG_ERR, "Radio %d channel bandwidth unknown\n", mask);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "cant't get radio %d channel bandwidth\n", mask);
			goto EXIT;
		}

		if(GetParamValueForIns(mbus, radio_path, "Channel", mask, val, sizeof(val)))
		{	
			log_printf(LOG_INFO, "radio %d channel is %s\n", mask, val);
			//HDK_Set_Int(pStr2, HDK_Element_Cisco_Channel, atoi(val));
			//pass down 0 to let wecb do auto-selection
			HDK_Set_Int(pStr2, HDK_Element_Cisco_Channel, 0);
		}
		else
		{
			log_printf(LOG_ERR, "cant't get radio %d channel\n", mask);
			goto EXIT;
		}

		if(GetParamValueForIns(mbus, radio_path, "ExtensionChannel", mask, val, sizeof(val)))
		{
			//currently we push channel 0 to WECB, so extension must be Auto, otherwise there will be error
			HDK_Set_Int(pStr2, HDK_Element_Cisco_SecondaryChannel, 3);
			/*
			if(!strcmp(val, "AboveControlChannel"))
			{
				log_printf(LOG_INFO, "radio %d second channel is above\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_SecondaryChannel, 1);
			}
			else if(!strcmp(val, "BelowControlChannel"))
			{
				log_printf(LOG_INFO, "radio %d second channel is below\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_SecondaryChannel, 2);
			}
			else if(!strcmp(val, "Auto"))
			{
				log_printf(LOG_INFO, "radio %d second channel is Auto\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_SecondaryChannel, 3);
			}
			else
			{
				log_printf(LOG_ERR, "Radio %d second channel unknown\n", mask);
				goto EXIT;
			}
			*/
		}
		else
		{
			log_printf(LOG_ERR, "cant't get radio %d second channel\n", mask);
			goto EXIT;
		}
		
		if(GetParamValueForIns(mbus, radio_path, "X_COMCAST-COM_BeaconInterval", mask, val, sizeof(val)))
		{	
			log_printf(LOG_INFO, "radio %d beacon interval is %s\n", mask, val);
			HDK_Set_Int(pStr2, HDK_Element_Cisco_BeaconInterval, atoi(val));
		}
		else
		{
			log_printf(LOG_INFO, "cant't get radio %d beacon interval\n", mask);
			goto EXIT;
		}

		if(GetParamValueForIns(mbus, radio_path, "X_CISCO_COM_DTIMInterval", mask, val, sizeof(val)))
		{	
			log_printf(LOG_INFO, "radio %d DTIM interval is %s\n", mask, val);
			HDK_Set_Int(pStr2, HDK_Element_Cisco_DTIMInterval, atoi(val));
		}
		else
		{
			log_printf(LOG_INFO, "cant't get radio %d DTIM interval\n", mask);
			goto EXIT;
		}

		if(GetParamValueForIns(mbus, radio_path, "GuardInterval", mask, val, sizeof(val)))
		{
			if(!strcmp(val, "400nsec"))
			{
				log_printf(LOG_INFO, "radio %d guard interval is 400nsec\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_GuardInterval, 1);
			}
			else if(!strcmp(val, "800nsec"))
			{
				log_printf(LOG_INFO, "radio %d guard interval is 800nsec\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_GuardInterval, 2);
			}
			else if(!strcmp(val, "Auto"))
			{
				log_printf(LOG_INFO, "radio %d guard interval is Auto\n", mask);
				HDK_Set_Int(pStr2, HDK_Element_Cisco_GuardInterval, 3);
			}
			else
			{
				log_printf(LOG_ERR, "Radio %d guard interval unknown\n", mask);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_INFO, "cant't get radio %d guard interval\n", mask);
			goto EXIT;
		}
		
		if(GetParamValueForIns(mbus, radio_path, "X_CISCO_COM_ObssCoex", mask, val, sizeof(val)))
		{	
			log_printf(LOG_INFO, "radio %d coexistance is %s\n", mask, val);
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_Coexistance, strcmp(val, "true") ? false : true);
		}
		else
		{
			log_printf(LOG_INFO, "cant't get radio %d coexistance\n", mask);
			goto EXIT;
		}
	}


	error = HDK_ClientMethod_Cisco_SetRadios(pCtx, iTimeoutmSecs, &input, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "SetRadios success\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "SetRadios returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

EXIT:
	HDK_Struct_Free(&input);
	return ret;
}


bool SetSSIDSettings(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false, wmm_enable = false, ssid_enable = false;
	HDK_Struct input;
	char path[MAX_BUF], val[MAX_BUF], ssid_index[11], open_flag = 0, val1[MAX_BUF], val2[MAX_BUF];
	HDK_Struct *pStr1 = NULL, *pStr2 = NULL, *pStr3 = NULL, *pStr4 = NULL;
	int ssid_num = MAX_SSID * 2, i = 0, j = 0, index24 = 0, index5 = 0, ssid_num1 = MAX_SSID * 2, k = 0, vlan_id = 0, primary_vlan, ofs = 0; 
	char insPath[MAX_INSTANCE][MAX_PATH_NAME], insPath1[MAX_INSTANCE][MAX_PATH_NAME];
	char *uuid = (char *)rt;

	if(mbus == NULL)
	{
		log_printf(LOG_ERR, "Why ccsp bus handle is invalid\n");
		return false;
	}
	
	memset(path, 0, sizeof(path));
	strncpy(path, USG_PRE, sizeof(path));
	strcat(path, "SSID.");
	
	if (MBus_FindObjectIns(mbus, path, NULL, NULL, insPath, &ssid_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate ssid instance number\n");
		return false;
	}
	
	log_printf(LOG_INFO, "SSID number is %d\n", ssid_num);

	/*
	for(i = 0; i < ssid_num; i++)
	{
		log_printf(LOG_INFO, "%s\n", insPath[i]);
	}
	*/
	HDK_Struct_Init(&input);
	pStr1 = HDK_Set_Struct(&input, HDK_Element_Cisco_SSIDList);

        /* RDKB-5091 - Workaround for IoT enabling of SSID 7 and beyond until they are available. */
        for(k = 0; (k < ssid_num) && (k <= HS_SSID_INS); k++)
	{
		if(sscanf(insPath[k], "Device.WiFi.SSID.%d.", &i) != 1)
		{
			log_printf(LOG_ERR, "parse ssid index error\n");
			goto EXIT;	
		}
		
		if(k == 0)
			pStr2 = HDK_Set_Struct(pStr1, HDK_Element_Cisco_SSIDSettings);
		else
			pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_SSIDSettings);

		if(pStr2 == NULL)
		{
			log_printf(LOG_ERR, "can't set SSIDSettings in HDK struct\n");			
			goto EXIT;
		}

		//printf("SSID index is %d, num is %d, k is %d\n", i, ssid_num, k);
		//we won't pass any disabled ssid to WECB
		/*
		snprintf(path, sizeof(path), "%s%d.Enable", "Device.WiFi.SSID.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			log_printf(LOG_INFO, "USG native SSID %d enable is %s\n", i, val);

			if(strcmp(val, "true"))
			{
				log_printf(LOG_WARNING, "SSID %d is disabled, not handle\n", i);
				continue;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d enable\n", i);
			goto EXIT;
		}
		*/
		//find out the ssid' frequency band
		snprintf(path, sizeof(path), "%s%d.LowerLayers", "Device.WiFi.SSID.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			strcpy(ssid_index, "0x00000000");
			//2.4G 
			if(!strcmp(val, "Device.WiFi.Radio.1."))
			{	
				ssid_index[2 + (index24 >> 2)] += pow(2, index24);
				index24++;
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, "RADIO_2.4GHz");
			}
			else if(!strcmp(val, "Device.WiFi.Radio.2."))
			{
				ssid_index[6 + (index5 >> 2)] += pow(2, index5);
				index5++;
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, "RADIO_5GHz");
			}
			else
			{
				log_printf(LOG_ERR, "Error SSID %d LowerLayers\n", i);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d LowerLayers\n", i);
			goto EXIT;
		}

		//printf("SSID %d index is %s\n", i, ssid_index);
		HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDIndex, ssid_index);

		//Firstly get usg native SSID settings
		snprintf(path, sizeof(path), "%s%d.SSID", "Device.WiFi.SSID.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, ""))
			{
				sprintf(val, "SSID%d", i);
			}
			HDK_Set_String(pStr2, HDK_Element_Cisco_SSID, val);
			log_printf(LOG_INFO, "USG native SSID %d name is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d name\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Enable", "Device.WiFi.SSID.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_EnableOnline", "Device.WiFi.SSID.", i);
			if(MBus_GetParamVal(mbus, path, val1, sizeof(val1)))
			{	
				log_printf(LOG_ERR, "can't get USG native SSID %d X_CISCO_COM_EnableOnline\n", i);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_RouterEnabled", "Device.WiFi.SSID.", i);
			if(MBus_GetParamVal(mbus, path, val2, sizeof(val2)))
			{	
				log_printf(LOG_ERR, "can't get USG native SSID %d X_CISCO_COM_RouterEnabled\n", i);
				goto EXIT;
			}

			if(!strcmp(val, "true") && (!strcmp(val1, "false") ||(!strcmp(val1, "true") && !strcmp(val2, "true"))))
			{
				HDK_Set_Bool(pStr2, HDK_Element_Cisco_SSIDEnabled, true);
				ssid_enable = true;
				log_printf(LOG_INFO, "USG native SSID %d enable is true\n", i);
			}
			else
			{
				HDK_Set_Bool(pStr2, HDK_Element_Cisco_SSIDEnabled, false);
				ssid_enable = false;
				log_printf(LOG_INFO, "USG native SSID %d enable is false\n", i);
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d enable\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.SSIDAdvertisementEnabled", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_SSIDBroadcast, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "USG native SSID %d broadcast is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d enable\n", i);
			goto EXIT;
		}
		
		HDK_IPAddress tmp;
		strcpy(val, "10.0.0.1");
		if(stringtoip(val, &tmp) == false)
		{
			goto EXIT;
		}
		HDK_Set_IPAddress(pStr2, HDK_Element_Cisco_SSIDLanBase, &tmp);

		HDK_Set_String(pStr2, HDK_Element_Cisco_BSSID, "00:01:02:03:04:05");

		snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_BssMaxNumSta", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			snprintf(path, sizeof(path), "%s%d.IsolationEnable", "Device.WiFi.AccessPoint.", i);
			if(MBus_GetParamVal(mbus, path, val1, sizeof(val1)) == 0)
			{
				if(i >= HS_SSID_INS && i < (HS_SSID_INS + HS_SSID_NUM))
					strcpy(val1, "true");
				else 
					strcpy(val1, "false");
				HDK_Set_Int(pStr2, HDK_Element_Cisco_MaxClients, atoi(val) + (!strcmp(val1, "true") ? 256 : 0));
				log_printf(LOG_INFO, "USG native SSID %d max clients is %s, isolation is %s\n", i, val, val1);
			}
			else
			{
				log_printf(LOG_ERR, "can't get USG native SSID %d client isolation setting\n", i);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d max clients\n", i);
			goto EXIT;
		}

                /* RDKB-5091 - Workaround for IoT enabling of SSID 7 and beyond until they are available. */
                if(ssid_enable == false || i < HS_SSID_INS)
		{
			primary_vlan = get_primary_lan_pvid();
			vlan_id = find_ssid_pvid(i);
			if (vlan_id == -1)
			{
				if(ssid_enable == false)
				{
					log_printf(LOG_WARNING, "SSID %d is disabled, treat is as untagged\n", i);
					vlan_id = primary_vlan;
				}
				else
				{
					log_printf(LOG_ERR, "Get SSID %d Vlan ID failed\n", i);
					goto EXIT;
				}
			}
			if (primary_vlan == -1 || vlan_id == primary_vlan)
				vlan_id = 0;
		}
		//only support HS SSID AP isolation 
		else if(i >= HS_SSID_INS && i < (HS_SSID_INS + HS_SSID_NUM))
		{
			//check AP isolation
			if(uuid && !strcmp(val1, "true"))
			{
				vlan_id = get_device_pvid(uuid, i - HS_SSID_INS);

				if(!vlan_id)
				{
					log_printf(LOG_ERR, "get ssid %d HS VLAN failed\n", i);
					goto EXIT;
				}
				ofs++;
			}
		}
		HDK_Set_Int(pStr2, HDK_Element_Cisco_SSIDVlanID, vlan_id);

		//output Encryption to HDK struct
		pStr3 = HDK_Set_Struct(pStr2, HDK_Element_Cisco_SSIDEncryption);

		if(pStr3 == NULL)
		{
			log_printf(LOG_ERR, "Can't set encryption in SSIDSettings");
			goto EXIT;
		}
		
		open_flag = 0;
		snprintf(path, sizeof(path), "%s%d.Security.ModeEnabled", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, "None"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_None);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is None\n", i);
				open_flag = 1;
			}
			else if(!strcmp(val, "WEP-64"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WEP_64);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WEP-64\n", i);
				open_flag = 2;
			}
			else if(!strcmp(val, "WEP-128"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WEP_128);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WEP-128\n", i);
				open_flag = 3;
			}
			else if(!strcmp(val, "WPA-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_Personal);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WPA-Personal\n", i);
			}
			else if(!strcmp(val, "WPA2-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA2_Personal);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WPA2-Personal\n", i);
			}
			else if(!strcmp(val, "WPA-WPA2-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Personal);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WPA-WPA2-Personal\n", i);
			}
			else if(!strcmp(val, "WPA-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_Enterprise);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA-Enterprise\n", i);
			}
			else if(!strcmp(val, "WPA2-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA2_Enterprise);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WPA2-Enterprise\n", i);
			}
			else if(!strcmp(val, "WPA-WPA2-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Enterprise);
				log_printf(LOG_INFO, "USG native SSID %d encryption mode is WPA-WPA2-Enterprise\n", i);
			}
			else
			{	
				log_printf(LOG_ERR, "USG native SSID %d encryption mode unknown\n", i);
				goto EXIT;
			}

		}
		else
		{
			log_printf(LOG_ERR, "can't get USG natvie SSID %d encryption mode\n", i);
			goto EXIT;
		}
	
		snprintf(path, sizeof(path), "%s%d.Security.X_CISCO_COM_EncryptionMethod", "Device.WiFi.AccessPoint.", i);
		if (open_flag == 1)
		{
			//ssid is open, no security and encryption
			HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_AES);
			log_printf(LOG_INFO, "USG native SSID %d encryption type is open, pass AES as result\n", i);
		}
		else if(open_flag == 2)
		{
			HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_64);
		}
		else if(open_flag == 3)
		{
			HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_128);
		}
		else if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, "WEP-64"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_64);
				log_printf(LOG_INFO, "USG native SSID %d encryption type is WEP-64\n", i);
			}
			else if(!strcmp(val, "WEP-128"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_128);
				log_printf(LOG_INFO, "USG native SSID %d encryption type is WEP-128\n", i);
			}
			else if(!strcmp(val, "AES"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_AES);
				log_printf(LOG_INFO, "USG native SSID %d encryption type is AES\n", i);
			}
			else if(!strcmp(val, "TKIP"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_TKIP);
				log_printf(LOG_INFO, "USG native SSID %d encryption type is TKIP\n", i);
			}
			else if(!strcmp(val, "AES+TKIP"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
				log_printf(LOG_INFO, "USG native SSID %d encryption type is TKIPORAES\n", i);
			}
			else
			{	
				log_printf(LOG_ERR, "USG native SSID %d encryption type unknown\n", i);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption type\n", i);
			goto EXIT;
		}
		
		snprintf(path, sizeof(path), "%s%d.Security.X_CISCO_COM_WEPKey64Bit.1.WEPKey", "Device.WiFi.AccessPoint.", i);
		if(open_flag == 3)
			snprintf(path, sizeof(path), "%s%d.Security.X_CISCO_COM_WEPKey128Bit.1.WEPKey", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_WepKey, val);
			log_printf(LOG_INFO, "USG native SSID %d encryption wep key %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption wep key\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Security.PreSharedKey", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_PreSharedKey, val);
			log_printf(LOG_INFO, "USG native SSID %d encryption preshared key %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption preshared key\n", i);
			goto EXIT;
		}
		
		snprintf(path, sizeof(path), "%s%d.Security.X_COMCAST-COM_KeyPassphrase", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_Passphrase, val);
			log_printf(LOG_INFO, "USG native SSID %d encryption passphase %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption passphase\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Security.RekeyingInterval", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr3, HDK_Element_Cisco_RekeyInterval, atoi(val));
			log_printf(LOG_INFO, "USG native SSID %d encryption rekey interval %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption rekey interval\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Security.RadiusServerIPAddr", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_IPAddress tmp;
			if(stringtoip(val, &tmp) == false)
			{
				goto EXIT;
			}
			HDK_Set_IPAddress(pStr3, HDK_Element_Cisco_RadiusServerIP, &tmp);
			log_printf(LOG_INFO, "USG native SSID %d encryption Radius Server IP %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption Radius Server IP\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Security.RadiusServerPort", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr3, HDK_Element_Cisco_RadiusServerPort, atoi(val));
			log_printf(LOG_INFO, "USG native SSID %d encryption Radius Server Port %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption Radius Server Port\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.Security.RadiusSecret", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_RadiusSecret, val);
			log_printf(LOG_INFO, "USG native SSID %d encryption Radius Server secret %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d encryption Radius Server secret\n", i);
			goto EXIT;
		}

		//output mac filter
		pStr3 = HDK_Set_Struct(pStr2, HDK_Element_Cisco_ACList);

		if(pStr3 == NULL)
		{
			log_printf(LOG_ERR, "Can't set ACList in SSIDSettings");
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_MACFilter.Enable", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, "false"))
			{
				 HDK_Set_String(pStr3, HDK_Element_Cisco_FilterType, "None");
				 HDK_Set_Struct(pStr3, HDK_Element_Cisco_MACList);
			}
			else
			{
				snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_MACFilter.FilterAsBlackList", "Device.WiFi.AccessPoint.", i);
				if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
				{
					if(!strcmp(val, "true"))
					{
						HDK_Set_String(pStr3, HDK_Element_Cisco_FilterType, "Deny");
					}
					else
					{
						HDK_Set_String(pStr3, HDK_Element_Cisco_FilterType, "Allow");
					}

					ssid_num1 = 15;
					sprintf(path, "Device.WiFi.AccessPoint.%d.X_CISCO_COM_MacFilterTable.", i);
					if (MBus_FindObjectIns(mbus, path, NULL, NULL, insPath1, &ssid_num1) != 0)
					{
						log_printf(LOG_ERR, "can't locate ssid %d MAC filter\n", i);
						goto EXIT;
					}

					pStr4 = HDK_Set_Struct(pStr3, HDK_Element_Cisco_MACList);

					if(pStr4 == NULL)
					{
						log_printf(LOG_ERR, "Can't set ssid %d MAC list\n", i);
						goto EXIT;
					}

					if(ssid_num1 >= 15)
						ssid_num1 = 15;
					for(j = 0; j < ssid_num1; j++)
					{
						snprintf(path, sizeof(path), "%sMACAddress", insPath1[j]);
						if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
						{
							HDK_MACAddress macAddr;
							if (sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
									&macAddr.a, &macAddr.b, &macAddr.c,
									&macAddr.d, &macAddr.e, &macAddr.f) != 6)
						        bzero(&macAddr, sizeof(macAddr));
			
							if(!j)
							{
								HDK_Set_MACAddress(pStr4, HDK_Element_Cisco_MACAddress, &macAddr);
							}
							else
								HDK_Append_MACAddress(pStr4, HDK_Element_Cisco_MACAddress, &macAddr);
						}
						else
						{
							log_printf(LOG_ERR, "Can't get %s MACAddress\n", insPath1[j]);
							goto EXIT;
						}
					}
	
				}
				else
				{
					log_printf(LOG_ERR, "can't get USG native SSID %d FilterAsBlackList\n", i);
					goto EXIT;
				}

			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d MAC filter enable\n", i);
			goto EXIT;
		}

		//output QoSinfo to HDK Struct
		pStr3 = HDK_Set_Struct(pStr2, HDK_Element_Cisco_SSIDQoS);

		snprintf(path, sizeof(path), "%s%d.UAPSDEnable", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_UAPSDEnable, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "USG native SSID %d QoS UAPSD is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d QoS UAPSD\n", i);
			goto EXIT;
		}
		
		snprintf(path, sizeof(path), "%s%d.WMMEnable", "Device.WiFi.AccessPoint.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_WMMEnable, strcmp(val, "true") ? false : true);
			wmm_enable = strcmp(val, "true") ? false : true;
			log_printf(LOG_INFO, "USG native SSID %d QoS WMMEnable is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d QoS WMMEnable\n", i);
			goto EXIT;
		}

		pStr4 = HDK_Set_Struct(pStr3, HDK_Element_Cisco_Qos);
		
		if(pStr4 == NULL)
		{
			log_printf(LOG_ERR, "can't set QoS struct\n");
			goto EXIT;
		}
		//QoSSettings has 4 entries
		for(j = 1; j <= SSID_QOS_NUM; j++)
		{
			if(wmm_enable == false)
			{
				log_printf(LOG_WARNING, "USG native ssid %d WMM is disable, no need to set QoSSettings struct\n", i);
				//HDK_Set_Struct(pStr4, HDK_Element_Cisco_QosSettings);
				break;
			}
			
			if(j == 1)
				pStr3 = HDK_Set_Struct(pStr4, HDK_Element_Cisco_QosSettings);
			else
				pStr3 = HDK_Append_Struct(pStr4, HDK_Element_Cisco_QosSettings);

			if(pStr3 == NULL)
			{
				log_printf(LOG_ERR, "can't set QoSSettings struct\n");
				goto EXIT;
			}
			HDK_Set_Int(pStr3, HDK_Element_Cisco_AC, j-1);
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_ACM, true);
			HDK_Set_Int(pStr3, HDK_Element_Cisco_AIFSN, 5);
			HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMin, 5);
			HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMax, 15);
			HDK_Set_Int(pStr3, HDK_Element_Cisco_TXOPLimit, 0);
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_NoACK, true);



			/*
			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.AC", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_AC, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d AC is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d AC\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.ACM", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Bool(pStr3, HDK_Element_Cisco_ACM, strcmp(val, "true") ? false : true);
				log_printf(LOG_INFO, "SSID %d QoSSettings %d ACM is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d ACM\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.AIFSN", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_AIFSN, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d AIFSN is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d AIFSN\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.CWMin", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMin, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d CWMin is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d CWMin\n", i, j);
				goto EXIT;
			}
	
			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.CWMax", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMax, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d CWMax is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d CWMax\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.TXOPLimit", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_TXOPLimit, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d TXOPLimit is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d TXOPLimit\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%s%d.X_CISCO_COM_WmmNoAck", "Device.WiFi.AccessPoint.", i);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Bool(pStr3, HDK_Element_Cisco_NoACK, strcmp(val, "true") ? false : true);
				log_printf(LOG_INFO, "SSID %d QoSSettings %d NoACK is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d NoACK\n", i, j);
				goto EXIT;
			}
		*/	
		}
	}

	memset(path, 0, sizeof(path));
	strncpy(path, WECB_PRE, sizeof(path));
	strcat(path, "SSID.");

	ssid_num = MAX_SSID * 2;	
	if (MBus_FindObjectIns(mbus, path, NULL, NULL, insPath, &ssid_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate ssid instance number\n");
		return false;
	}
	// This DM is not activated now
	ssid_num = 0;
	//log_printf(LOG_INFO, "WECB private SSID number is %d\n", ssid_num);

	for(i = 0; i < ssid_num; i++)
	{
		snprintf(path, sizeof(path), "%sSSIDIndex", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{	
			int index = htoi(val);

			if (index & 0xFFFF)
			{
				index5++;
			}
			else if(index & 0xFFFF0000)
			{
				index24++;
			}

			if (index5 > MAX_SSID || index24 > MAX_SSID)
			{
				log_printf(LOG_ERR, "Too many ssid instances, ignore it\n");
				continue;
			}
		}

		pStr2 = HDK_Append_Struct(pStr1, HDK_Element_Cisco_SSIDSettings);
		
		if(pStr2 == NULL)
		{
			log_printf(LOG_ERR, "can't set SSIDSettings in HDK struct\n");
			goto EXIT;
		}
	
		//CCSP DM has bug, it's cache won't be updated if we change syscfg only
		/*	
		snprintf(path, sizeof(path), "%sName", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, val);
			log_printf(LOG_INFO, "SSID %d Id is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get ssid %d radio id\n", i);
			goto EXIT;
		}*/

		snprintf(path, sizeof(path), "%sSSIDIndex", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			char tmp[32], *s;
			//has 0x prefix for hex number
			if(!strncmp(val, "0x", 2))
			{ 
				s = val + 2;
				while(*s) 
				{
					*s = toupper(*s);
					s++;
				}
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDIndex, val);
			}
			//convert the upcase to lowercase
			else if(!strncmp(val, "0X", 2))
			{
				val[1] = 'x';
				s = val + 2;
				while(*s) 
				{
					*s = toupper(*s);
					s++;
				}
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDIndex, val);
			}
			else
			{
				strcpy(tmp, "0x");
				s = val;
				while(*s) 
				{
					*s = toupper(*s);
					s++;
				}
				strcat(tmp, val);
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDIndex, tmp);
			}

			int index = htoi(val);
			if(index & 0xFFFF)
			{
				if(index & 0xFFFF0000)
				{
					HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, "Radio_2.4GHz & Radio_5GHz");
				}
				else
				{
					HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, "Radio_5GHz");
				}
			}
			else if(index & 0xFFFF0000)
			{
				HDK_Set_String(pStr2, HDK_Element_Cisco_SSIDRadioID, "Radio_2.4GHz");
			}
			else
			{
				log_printf(LOG_ERR, "unknown ssid index\n");
				goto EXIT;
			}
			log_printf(LOG_INFO, "SSID %d index is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d index\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSID", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr2, HDK_Element_Cisco_SSID, val);
			log_printf(LOG_INFO, "SSID %d name is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d name\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sEnable", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_SSIDEnabled, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "SSID %d Enable is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d Enable\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDBroadcast", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr2, HDK_Element_Cisco_SSIDBroadcast, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "SSID %d broadcast is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d broadcast\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDVlanID", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr2, HDK_Element_Cisco_SSIDVlanID, atoi(val));
			log_printf(LOG_INFO, "SSID %d VlanID is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d VlanID\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDLanBase", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_IPAddress tmp;
			if(stringtoip(val, &tmp) == false)
			{
				goto EXIT;
			}
			HDK_Set_IPAddress(pStr2, HDK_Element_Cisco_SSIDLanBase, &tmp);
			log_printf(LOG_INFO, "SSID %d lan base address is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d lan base address\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sMaxClients", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr2, HDK_Element_Cisco_MaxClients, atoi(val));
			log_printf(LOG_INFO, "SSID %d max clients is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d max clients\n", i);
			goto EXIT;
		}

		//output Encryption to HDK struct
		pStr3 = HDK_Set_Struct(pStr2, HDK_Element_Cisco_SSIDEncryption);

		if(pStr3 == NULL)
		{
			log_printf(LOG_ERR, "Can't set encryption in QosSettingn");
			goto EXIT;
		}
	
		open_flag = 0;	
		snprintf(path, sizeof(path), "%sSSIDEncryption.ModeEnabled", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, "None"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_None);
				log_printf(LOG_INFO, "SSID %d encryption mode is None\n", i);
				open_flag = 1;
			}
			else if(!strcmp(val, "WEP-64"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WEP_64);
				log_printf(LOG_INFO, "SSID %d encryption mode is WEP-64\n", i);
			}
			else if(!strcmp(val, "WEP-128"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WEP_128);
				log_printf(LOG_INFO, "SSID %d encryption mode is WEP-128\n", i);
			}
			else if(!strcmp(val, "WPA-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_Personal);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA-Personal\n", i);
			}
			else if(!strcmp(val, "WPA2-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA2_Personal);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA2-Personal\n", i);
			}
			else if(!strcmp(val, "WPA-WPA2-Personal"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Personal);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA-WPA2-Personal\n", i);
			}
			else if(!strcmp(val, "WPA-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_Enterprise);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA-Enterprise\n", i);
			}
			else if(!strcmp(val, "WPA2-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_Enterprise);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA2-Enterprise\n", i);
			}
			else if(!strcmp(val, "WPA-WPA2-Enterprise"))
			{	
				HDK_Set_Cisco_WiFiSecurity(pStr3, HDK_Element_Cisco_ModeEnabled, HDK_Enum_Cisco_WiFiSecurity_WPA_WPA2_Enterprise);
				log_printf(LOG_INFO, "SSID %d encryption mode is WPA-WPA2-Enterprise\n", i);
			}
			else
			{	
				log_printf(LOG_ERR, "SSID %d encryption mode unknown\n", i);
				goto EXIT;
			}

		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption mode\n", i);
			goto EXIT;
		}
	
		snprintf(path, sizeof(path), "%sSSIDEncryption.Encryption", insPath[i]);
		if (open_flag == 1)
		{
			//ssid is open, no security and encryption
			HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_AES);
			log_printf(LOG_INFO, "USG native SSID %d encryption type is open, pass AES as result\n", i);
		}
		else if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			if(!strcmp(val, "WEP-64"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_64);
				log_printf(LOG_INFO, "SSID %d encryption type is WEP-64\n", i);
			}
			else if(!strcmp(val, "WEP-128"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_WEP_128);
				log_printf(LOG_INFO, "SSID %d encryption type is WEP-128\n", i);
			}
			else if(!strcmp(val, "AES"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_AES);
				log_printf(LOG_INFO, "SSID %d encryption type is AES\n", i);
			}
			else if(!strcmp(val, "TKIP"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_TKIP);
				log_printf(LOG_INFO, "SSID %d encryption type is TKIP\n", i);
			}
			else if(!strcmp(val, "TKIPORAES"))
			{
				HDK_Set_Cisco_WiFiEncryption(pStr3, HDK_Element_Cisco_Encryption, HDK_Enum_PN_WiFiEncryption_TKIPORAES);
				log_printf(LOG_INFO, "SSID %d encryption type is TKIPORAES\n", i);
			}
			else
			{	
				log_printf(LOG_ERR, "SSID %d encryption type unknown\n", i);
				goto EXIT;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption type\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.WepKey", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_WepKey, val);
			log_printf(LOG_INFO, "SSID %d encryption wep key %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption wep key\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.PreSharedKey", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_PreSharedKey, val);
			log_printf(LOG_INFO, "SSID %d encryption preshared key %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption preshared key\n", i);
			goto EXIT;
		}
		
		snprintf(path, sizeof(path), "%sSSIDEncryption.Passphrase", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_Passphrase, val);
			log_printf(LOG_INFO, "SSID %d encryption passphase %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption passphase\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.RekeyInterval", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr3, HDK_Element_Cisco_RekeyInterval, atoi(val));
			log_printf(LOG_INFO, "SSID %d encryption rekey interval %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption rekey interval\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.RadiusServerIP", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_IPAddress tmp;
			if(stringtoip(val, &tmp) == false)
			{
				goto EXIT;
			}
			HDK_Set_IPAddress(pStr3, HDK_Element_Cisco_RadiusServerIP, &tmp);
			log_printf(LOG_INFO, "SSID %d encryption Radius Server IP %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption Radius Server IP\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.RadiusServerPort", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Int(pStr3, HDK_Element_Cisco_RadiusServerPort, atoi(val));
			log_printf(LOG_INFO, "SSID %d encryption Radius Server Port %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption Radius Server Port\n", i);
			goto EXIT;
		}

		snprintf(path, sizeof(path), "%sSSIDEncryption.RadiusSecret", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_String(pStr3, HDK_Element_Cisco_RadiusSecret, val);
			log_printf(LOG_INFO, "SSID %d encryption Radius Server secret %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d encryption Radius Server secret\n", i);
			goto EXIT;
		}

		//output QoSinfo to HDK Struct
		pStr3 = HDK_Set_Struct(pStr2, HDK_Element_Cisco_SSIDQoS);

		snprintf(path, sizeof(path), "%sSSIDQoS.UAPSDEnable", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_UAPSDEnable, strcmp(val, "true") ? false : true);
			log_printf(LOG_INFO, "SSID %d QoS UAPSD is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d QoS UAPSD\n", i);
			goto EXIT;
		}
		
		snprintf(path, sizeof(path), "%sSSIDQoS.WMMEnable", insPath[i]);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			HDK_Set_Bool(pStr3, HDK_Element_Cisco_WMMEnable, strcmp(val, "true") ? false : true);
			wmm_enable = strcmp(val, "true") ? false : true;
			log_printf(LOG_INFO, "SSID %d QoS WMMEnable is %s\n", i, val);
		}
		else
		{
			log_printf(LOG_ERR, "can't get SSID %d QoS WMMEnable\n", i);
			goto EXIT;
		}

		pStr4 = HDK_Set_Struct(pStr3, HDK_Element_Cisco_Qos);
		
		if(pStr4 == NULL)
		{
			log_printf(LOG_ERR, "can't set QoS struct\n");
			goto EXIT;
		}
		//QoSSettings has 4 entries
		for(j = 1; j <= SSID_QOS_NUM; j++)
		{
			if(wmm_enable == false)
			{
				log_printf(LOG_WARNING, "ssid %d WMM is disable, no need to set QoSSettings struct\n", i);
				//HDK_Set_Struct(pStr4, HDK_Element_Cisco_QosSettings);
				break;
			}
			
			if(j == 1)
				pStr3 = HDK_Set_Struct(pStr4, HDK_Element_Cisco_QosSettings);
			else
				pStr3 = HDK_Append_Struct(pStr4, HDK_Element_Cisco_QosSettings);

			if(pStr3 == NULL)
			{
				log_printf(LOG_ERR, "can't set QoSSettings struct\n");
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.AC", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_AC, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d AC is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d AC\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.ACM", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Bool(pStr3, HDK_Element_Cisco_ACM, strcmp(val, "true") ? false : true);
				log_printf(LOG_INFO, "SSID %d QoSSettings %d ACM is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d ACM\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.AIFSN", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_AIFSN, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d AIFSN is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d AIFSN\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.CWMin", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMin, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d CWMin is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d CWMin\n", i, j);
				goto EXIT;
			}
	
			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.CWMax", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_CWMax, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d CWMax is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d CWMax\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.TXOPLimit", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Int(pStr3, HDK_Element_Cisco_TXOPLimit, atoi(val));
				log_printf(LOG_INFO, "SSID %d QoSSettings %d TXOPLimit is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d TXOPLimit\n", i, j);
				goto EXIT;
			}

			snprintf(path, sizeof(path), "%sSSIDQoS.QosSettings.%d.NoACK", insPath[i], j);
			if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
			{
				HDK_Set_Bool(pStr3, HDK_Element_Cisco_NoACK, strcmp(val, "true") ? false : true);
				log_printf(LOG_INFO, "SSID %d QoSSettings %d NoACK is %s\n", i, j, val);
			}
			else
			{
				log_printf(LOG_ERR, "can't get SSID %d QoSSettings %d NoACK\n", i, j);
				goto EXIT;
			}	
		}
	}


	error = HDK_ClientMethod_Cisco_SetSSIDSettings(pCtx, iTimeoutmSecs_al, &input, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "SetSSIDSettings success\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "SetSSIDSettings returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

EXIT:	
	HDK_Struct_Free(&input);
	return ret;
}

bool SetWPS(HDK_ClientContext *pCtx, void *rt)
{
	HDK_ClientError error = HDK_ClientError_OK;
	HDK_Enum_Result result;
	bool ret = false;
	HDK_Struct input;
	char path[MAX_BUF], val[MAX_BUF], val1[MAX_BUF];
	char insPath[MAX_INSTANCE][MAX_PATH_NAME], wps_index[11];
	int ssid_num = 2, i = 0, j = 0, k = 0, wps24 = 16, wps5 = 0;

	if(mbus == NULL)
	{
		log_printf(LOG_ERR, "Why ccsp bus handle is invalid\n");
		return false;
	}
	
	memset(path, 0, sizeof(path));
	strcpy(wps_index, "0x00000000");

	HDK_Struct_Init(&input);
	
	if (MBus_FindObjectIns(mbus, "Device.WiFi.SSID.", NULL, NULL, insPath, &ssid_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate ssid instance number\n");
		return false;
	}

	//USG has 2 bugs about WPS pin which will cause not sync
	HDK_Set_String(&input, HDK_Element_Cisco_PINCode, "12345670");

        /* RDKB-5091 - Workaround for IoT enabling of SSID 7 and beyond until they are available. */
        for(j = 0; (j < ssid_num) && (j <= HS_SSID_INS); j++)
	{
		if(sscanf(insPath[j], "Device.WiFi.SSID.%d.", &i) != 1)
		{
			log_printf(LOG_ERR, "parse ssid index error\n");
			goto EXIT;	
		}
		
		//printf("SSID index is %d\n", i);
		//we won't pass any disabled ssid to WECB
		snprintf(path, sizeof(path), "%s%d.Enable", "Device.WiFi.SSID.", i);
		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			log_printf(LOG_INFO, "USG native SSID %d enable is %s\n", i, val);

			if(strcmp(val, "true"))
			{
				log_printf(LOG_WARNING, "SSID %d is disabled, not handle\n", i);
				continue;
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d enable\n", i);
			goto EXIT;
		}

		//Parse wps index
		snprintf(path, sizeof(path), "%s%d.WPS.Enable", "Device.WiFi.AccessPoint.", i);

		if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
		{
			log_printf(LOG_INFO, "USG native SSID %d WPS enable is %s\n", i, val);

			snprintf(path, sizeof(path), "%sLowerLayers", insPath[j]);

			if(MBus_GetParamVal(mbus, path, val1, sizeof(val1)) == 0)
			{
				log_printf(LOG_INFO, "USG native SSID %d lower layer is %s\n", i, val1);

				if(strstr(val1, "Radio.1"))
				{
					if(!strcmp(val, "true"))
						k += pow(2, wps24);
				    wps24++;	
				}
				if(strstr(val1, "Radio.2"))
				{
					if(!strcmp(val, "true"))
						k += pow(2, wps5);
				    wps5++;	
				}
			}
			else
			{
				log_printf(LOG_ERR, "can't get USG native SSID %d lower layers\n", i);
				goto EXIT;
			}

			if(!strcmp(val, "true"))
			{
				snprintf(path, sizeof(path), "Device.WiFi.AccessPoint.%d.WPS.X_CISCO_COM_Pin", i);

				if(MBus_GetParamVal(mbus, path, val1, sizeof(val1)) == 0)
				{
					log_printf(LOG_INFO, "USG native SSID %d WPS Pin is %s\n", i, val1);
					if (strlen(val1) >= 8)
						HDK_Set_String(&input, HDK_Element_Cisco_PINCode, val1);
				}
				else
				{
					log_printf(LOG_ERR, "can't get USG native SSID %d WPS Pin\n", i);
					goto EXIT;
				}
			}
		}
		else
		{
			log_printf(LOG_ERR, "can't get USG native SSID %d WPS enable\n", i);
			goto EXIT;
		}

	}

	if(k == 0)
	{
		HDK_Set_Bool(&input, HDK_Element_Cisco_WPSEnable, false);
		HDK_Set_String(&input, HDK_Element_Cisco_PINCode, "12345670");
		HDK_Set_String(&input, HDK_Element_Cisco_SSIDIndex, "0x10001000");
	}	
	else
	{
		HDK_Set_Bool(&input, HDK_Element_Cisco_WPSEnable, true);
		//until now, wecb only supports wps enable on home ssid
		k = k & 0x10001;
		k = k << 12;
		if(!(k >> 16))
		{
			sprintf(wps_index + 6, "%x", k);
		}
		else
		{
			sprintf(wps_index + 2, "%x", k);
		}
		//printf("WPS Pin is %s\n", wps_index);
		HDK_Set_String(&input, HDK_Element_Cisco_SSIDIndex, wps_index);
	}
	

	/*
	snprintf(path, sizeof(path), "%sWPS.Enable", WECB_PRE);
	if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
	{
		HDK_Set_Bool(&input, HDK_Element_Cisco_WPSEnable, strcmp(val, "true") ? false : true);
		log_printf(LOG_INFO, "WPS enable is %s\n", val);
	}
	else
	{
		log_printf(LOG_ERR, "can't get wps enable\n");
		goto EXIT;
	}

	snprintf(path, sizeof(path), "%sWPS.SSIDIndex", WECB_PRE);
	if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
	{
		char tmp[32], *s;
		//has 0x prefix for hex number
		if(!strncmp(val, "0x", 2))
		{ 
			s = val + 2;
			while(*s) 
			{
				*s = toupper(*s);
				s++;
			}
			HDK_Set_String(&input, HDK_Element_Cisco_SSIDIndex, val);
		}
		//convert the upcase to lowercase
		else if(!strncmp(val, "0X", 2))
		{
			val[1] = 'x';
			s = val + 2;
			while(*s) 
			{
				*s = toupper(*s);
				s++;
			}
			HDK_Set_String(&input, HDK_Element_Cisco_SSIDIndex, val);
		}
		else
		{
			strcpy(tmp, "0x");
			s = val;
			while(*s) 
			{
				*s = toupper(*s);
				s++;
			}
			strcat(tmp, val);
			HDK_Set_String(&input, HDK_Element_Cisco_SSIDIndex, tmp);
		}

		log_printf(LOG_INFO, "WPS ssid index is %s\n", val);
	}
	else
	{
		log_printf(LOG_ERR, "can't get wps ssid index\n");
		goto EXIT;
	}
	
	//snprintf(path, sizeof(path), "%sWPS.X_CISCO_COM_Pin", WECB_PRE);
	strcpy(path, "Device.WiFi.AccessPoint.1.WPS.X_CISCO_COM_Pin");
	if(MBus_GetParamVal(mbus, path, val, sizeof(val)) == 0)
	{
		HDK_Set_String(&input, HDK_Element_Cisco_PINCode, val);
		log_printf(LOG_INFO, "WPS pin is %s\n", val);
	}
	else
	{
		log_printf(LOG_ERR, "can't get wps pin\n");
		goto EXIT;
	}
*/

	error = HDK_ClientMethod_Cisco_SetWPS(pCtx, iTimeoutmSecs, &input, &result);

	switch (error)
    {
        case HDK_ClientError_OK:
        {
			if(result == HDK_Enum_Result_OK)
			{
				log_printf(LOG_INFO, "SetWPS success\n");
				ret = true;
			}
			else
			{
				log_printf(LOG_ERR, "SetWPS returns %d\n", result);
			}
            break;
        }
        case HDK_ClientError_HnapParse:
        {
            log_printf(LOG_ERR, "failed to parse server response\n");
            break;
        }
        case HDK_ClientError_SoapFault:
        {
            log_printf(LOG_ERR, "server responded with SOAP fault\n");
            break;
        }
        case HDK_ClientError_HttpAuth:
        {
            log_printf(LOG_ERR, "invalid HTTP authentication: %s:%s\n", pCtx->pszUsername, pCtx->pszPassword);
            break;
        }
        case HDK_ClientError_HttpUnknown:
        {
            log_printf(LOG_ERR, "unknown HTTP error\n");
            break;
        }
        case HDK_ClientError_Connection:
        {
            log_printf(LOG_ERR, "connection error\n");
            break;
        }
        default:
        {
            log_printf(LOG_ERR, "unknown error %u\n", error);
            break;
        }
	}

EXIT:
	HDK_Struct_Free(&input);
	return ret;
}
#if 0
//update DM X_CISCO_COM_ONLINE_CLIENT for web gui, flag 0 indicates delete, 1 means add
void update_online_client(char *ip, char *mac, char flag)
{
	char path[MAX_BUF], val[1024], val1[1024];
	char insPath[MAX_INSTANCE][MAX_PATH_NAME];
	int insNum = 60;
	char *p = NULL;

	if (MBus_GetParamVal(mbus, "Device.MoCA.X_CISCO_COM_WiFi_Extender.X_CISCO_COM_ONLINE_CLIENT", val1, sizeof(val1)) != 0)
	{
		log_printf(LOG_ERR, "Get X_CISCO_COM_ONLINE_CLIENT failed\n");
		return;
	}

	//we are deleting, mac must be filled
	if(!flag)
	{ 
		if(!strlen(mac))
		{
			log_printf(LOG_ERR, "error input for update_online_client\n");
			return;
		}
		p = strstr(val1, mac);
		if(p != NULL)
		{
			//we have only one client
			if(!strcmp(val1, mac))
			{
				strcpy(val1, "");
			}
			else
			{
				//mac address is 17 bytes, one comma seperate
				if (strcmp(p, mac))
				{
					//not the last one
					memmove(p, p + 18, strlen(p) - 17);
				}
				else
				{
					*(p-1) = '\0';
				}
			}
		}
		else
		{
			log_printf(LOG_WARNING, "Devce %s:%s is already offline\n", ip, mac);
			return;
		}
	}
	//means we are adding
	else
	{
		if(MBus_FindObjectIns(mbus, "Device.DHCPv4.Server.Pool.1.Client.", "IPv4Address.1.IPAddress", ip, insPath, &insNum) != 0 || insNum == 0)
		{
			log_printf(LOG_ERR, "%s %s failed X_CISCO_COM_ONLINE_CLIENT when query DHCP\n", flag ? "Add" : "Delete", ip);
			return;
		}

		snprintf(path, MAX_PATH_NAME, "%sChaddr", insPath[0]);
    
		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s correspond mac address failed\n", ip);
			return;
		}

		strcpy(mac, val);
		
		p = strstr(val1, mac);
		if(p == NULL)
		{
			if(!strlen(val1))
			{
				strcpy(val1, mac);
			}
			else
			{
				strcat(val1, ",");
				strcat(val1, mac);
			}
		}
		else
		{
			log_printf(LOG_WARNING, "Devce %s:%s is already online\n", ip, mac);
			return;
		}
	}

	if (MBus_SetParamVal(mbus, "Device.MoCA.X_CISCO_COM_WiFi_Extender.X_CISCO_COM_ONLINE_CLIENT", MBUS_PT_STRING, val1, 1) != 0)
	{
		log_printf(LOG_ERR, "Set X_CISCO_COM_ONLINE_CLIENT failed\n");
		return;
	}
}
#endif

int force_radio_down(char *addr)
{
	char path[MAX_BUF], val[1024], *p;
        int len = 0;
	
	if(!strlen(addr))
	{
		log_printf(LOG_ERR, "error input for force_radio_down\n");
		return 0;
	}

	strcpy(path, "Device.MoCA.X_CISCO_COM_WiFi_Extender.X_CISCO_COM_DISCONNECT_CLIENT");
    
	if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get WECB disconnect client failed\n");
		return 0;
	}

        len = strlen(addr);
        if((p = strstr(val, addr)) && ((*(p+len) < '0') || (*(p+len) > '9')))
	{
		/*
		if (MBus_SetParamVal(mbus, "Device.MoCA.X_CISCO_COM_WiFi_Extender.X_CISCO_COM_Radio_Updated", MBUS_PT_BOOL, true, 1) != 0)
		{
			log_printf(LOG_ERR, "Set X_CISCO_COM_Radio_Updated failed\n");
			return 0;
		}
		*/
		return 1;
	}

	return 0;
}
#if 0
void Clean_DM()
{
	if (MBus_SetParamVal(mbus, "Device.MoCA.X_CISCO_COM_WiFi_Extender.X_CISCO_COM_ONLINE_CLIENT", MBUS_PT_STRING, "", 1) != 0)
	{
		log_printf(LOG_ERR, "Set X_CISCO_COM_ONLINE_CLIENT failed\n");
		return;
	}
}
#endif 


int get_primary_lan_pvid()
{	
	char path[MAX_BUF], val[MAX_BUF];
	
	if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge", val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge failed\n");
		return -1;
	}

	strcpy(path, val);
	strcat(path, ".Port.1.PVID");

	if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge failed\n");
		return -1;
	}
	
	return atoi(val);
}

int add_moca_in_bridge()
{
	char insPath[MAX_INSTANCE][MAX_PATH_NAME], insPath2[MAX_INSTANCE][MAX_PATH_NAME];
	int i = 0, j, bridge_num = MAX_INSTANCE, port_num = MAX_INSTANCE, inf_num = MAX_INSTANCE, newIns;
	char path[MAX_BUF], val[MAX_BUF], val1[MAX_BUF];
	char *moca_inf = "Device.MoCA.Interface.1", *moca_mode = "PassThrough";

	if (MBus_FindObjectIns(mbus, "Device.Bridging.Bridge.", NULL, NULL, insPath, &bridge_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate Device.Bridging.Bridge.\n");
		return -1;
	}

	for(i = 0; i < bridge_num; i++, port_num = MAX_INSTANCE, inf_num = MAX_INSTANCE)
	{
		strcpy(path, insPath[i]);
		strcat(path, "Port.");
		/*
		if (MBus_FindObjectIns(mbus, path, NULL, NULL, insPath1, &port_num) != 0)
		{
			log_printf(LOG_ERR, "can't locate %s\n", path);
			return -1;
		}
		*/
		//for(j = 0; j < port_num; j++, inf_num = MAX_INSTANCE)
		{
			if (MBus_FindObjectIns(mbus, path, "LowerLayers", moca_inf, insPath2, &inf_num) != 0)
			{
				log_printf(LOG_ERR, "can't locate %s\n", path);
				return -1;
			}

			//we need add moca interface into this bridge
			if (inf_num == 0)
			{
				if (MBus_AddObjectIns(mbus, path, &newIns) != 0)
				{
					log_printf(LOG_ERR, "Adding moca interface into %s failed\n", path);
                    return -1;
				}

				sprintf(val, "%s%d.LowerLayers", path, newIns);

				if (MBus_SetParamVal(mbus, val, MBUS_PT_STRING, moca_inf, 1) != 0)
				{
					log_printf(LOG_ERR, "Set %s failed\n", val);
					return -1;
				}

				sprintf(val, "%s%d.X_CISCO_COM_Mode", path, newIns);

				if (MBus_SetParamVal(mbus, val, MBUS_PT_STRING, moca_mode, 1) != 0)
				{
					log_printf(LOG_ERR, "Set %s failed\n", val);
					return -1;
				}	

				sprintf(val, "%s%d.Enable", path, newIns);

				if (MBus_SetParamVal(mbus, val, MBUS_PT_BOOL, "true", 1) != 0)
				{
					log_printf(LOG_ERR, "Set %s failed\n", val);
					return -1;
				}				
			}
			else
			//check moca interface parameters
			{
				sprintf(val, "%sLowerLayers", insPath2[0]);

				if (MBus_GetParamVal(mbus, val, val1, sizeof(val1)) != 0)
				{
					log_printf(LOG_ERR, "Get %s failed\n", val);
					return -1;
				}
				
				if (strcmp(val1, moca_inf))
				{
					if (MBus_SetParamVal(mbus, val, MBUS_PT_STRING, moca_inf, 1) != 0)
					{
						log_printf(LOG_ERR, "Set %s failed\n", val);
						return -1;
					}
				}

				sprintf(val, "%sX_CISCO_COM_Mode", insPath2[0]);

				if (MBus_GetParamVal(mbus, val, val1, sizeof(val1)) != 0)
				{
					log_printf(LOG_ERR, "Get %s failed\n", val);
					return -1;
				}
				
				if (strcmp(val1, moca_mode))
				{
					if (MBus_SetParamVal(mbus, val, MBUS_PT_STRING, moca_mode, 1) != 0)
					{
						log_printf(LOG_ERR, "Set %s failed\n", val);
						return -1;
					}
				}

				sprintf(val, "%sEnable", insPath2[0]);

				if (MBus_GetParamVal(mbus, val, val1, sizeof(val1)) != 0)
				{
					log_printf(LOG_ERR, "Get %s failed\n", val);
					return -1;
				}
				
				if (strcmp(val1, "true"))
				{
					if (MBus_SetParamVal(mbus, val, MBUS_PT_BOOL, "true", 1) != 0)
					{
						log_printf(LOG_ERR, "Set %s failed\n", val);
						return -1;
					}
				}
			}
		}
	}
	return 0;
}



int set_moca_bridge()
{
	char insPath[MAX_INSTANCE][MAX_PATH_NAME];
	int i = 0, j, bridge_num = MAX_INSTANCE, newIns;
	char path[MAX_BUF], val[MAX_BUF];
	char *moca_inf = "Device.MoCA.Interface.1", *moca_mode = "PassThrough", *primary_mode = "Tagging";

#ifdef CONFIG_VENDOR_CUSTOMER_COMCAST
	//This old issue only existed on Comcast product
	//we need to be compatible with old VLAN implementation on WECB, take care of MoCA interface mode in primary lan
	if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge", val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", "Device.X_CISCO_COM_MultiLAN.PrimaryLANBridge");
		return -1;
	}

	strcat(val, ".Port.");

	if (MBus_FindObjectIns(mbus, val, "LowerLayers", moca_inf, insPath, &bridge_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate %s\n", val);
		return -1;
	}


	//we need add moca interface into this bridge
	if (bridge_num == 0)
	{
		log_printf(LOG_ERR, "Something wrong, why can't find MoCA interface in Primary bridge\n");
		return -1;
	}
	else
	//check moca interface parameters
	{
		sprintf(path, "%sLowerLayers", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
		if (strcmp(val, moca_inf))
		{
			if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, moca_inf, 1) != 0)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}
		}

		sprintf(path, "%sX_CISCO_COM_Mode", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
		if (strcmp(val, primary_mode))
		{
			if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, primary_mode, 1) != 0)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}
		}

		sprintf(path, "%sEnable", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
		if (strcmp(val, "true"))
		{
			if (MBus_SetParamVal(mbus, path, MBUS_PT_BOOL, "true", 1) != 0)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}
		}
	}
#endif 
#ifdef CONFIG_CISCO_XHS
	
	for(j = 0; j < 4; j++)
	{
		bridge_num = MAX_INSTANCE;
		if (MBus_GetParamVal(mbus, "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridge", val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", "Device.X_CISCO_COM_MultiLAN.HomeSecurityBridge");
			return -1;
		}

	strcat(val, ".Port.");

		if (MBus_FindObjectIns(mbus, val, "LowerLayers", all_wecb_ports[j], insPath, &bridge_num) != 0)
		{
			log_printf(LOG_ERR, "can't locate %s\n", val);
			return -1;
		}

		//we need add interface into this bridge
		if (bridge_num == 0)
		{
			if (MBus_AddObjectIns(mbus, val, &newIns) != 0)
			{
				log_printf(LOG_ERR, "Adding interface %d into %s failed\n", j, val);
				return -1;
			}

		sprintf(path, "%s%d.LowerLayers", val, newIns);

			if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, all_wecb_ports[j], 1) != 0)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}

		sprintf(path, "%s%d.X_CISCO_COM_Mode", val, newIns);

		if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, moca_mode, 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", path);
			return -1;
		}	

		sprintf(path, "%s%d.Enable", val, newIns);

		if (MBus_SetParamVal(mbus, path, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", path);
			return -1;
		}				
	}
	else
	//check moca interface parameters
	{
		sprintf(path, "%sLowerLayers", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
			if (strcmp(val, all_wecb_ports[j]))
			{
				if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, all_wecb_ports[j], 1) != 0)
				{
					log_printf(LOG_ERR, "Set %s failed\n", path);
					return -1;
				}
			}

		sprintf(path, "%sX_CISCO_COM_Mode", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
		if (strcmp(val, moca_mode))
		{
			if (MBus_SetParamVal(mbus, path, MBUS_PT_STRING, moca_mode, 1) != 0)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}
		}

		sprintf(path, "%sEnable", insPath[0]);

		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		
			if (strcmp(val, "true"))
			{
				if (MBus_SetParamVal(mbus, path, MBUS_PT_BOOL, "true", 1) != 0)
				{
					log_printf(LOG_ERR, "Set %s failed\n", path);
					return -1;
				}
			}
		}
	}
#endif
	return 0;
}

int find_ssid_pvid(int ssid_ins)
{
	
	char insPath[MAX_INSTANCE][MAX_PATH_NAME], insPath1[MAX_INSTANCE][MAX_PATH_NAME]; 
	int i = 0, j, bridge_num = MAX_INSTANCE, port_num = MAX_INSTANCE, inf_num = MAX_INSTANCE;
	char path[MAX_BUF], val[MAX_BUF], val1[MAX_BUF], path1[MAX_BUF]; 

	if(ssid_ins <= 0)
	{
		log_printf(LOG_ERR, "illegal parameters\n");
		return -1;
	}

	//syscfg_get(NULL, WECB_HS_BRIDGE_INS, val, sizeof(val));
	//bridge_num = atoi(val);

	sprintf(val, "Device.WiFi.SSID.%d", ssid_ins);

	if (MBus_FindObjectIns(mbus, "Device.Bridging.Bridge.", NULL, NULL, insPath, &bridge_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate Device.Bridging.Bridge.\n");
		return -1;
	}

	for(i = 0; i < bridge_num; i++, port_num = MAX_INSTANCE)
	{
		sprintf(path, "%sPort.", insPath[i]);	
		if (MBus_FindObjectIns(mbus, path, "LowerLayers", val, insPath1, &port_num) != 0)
		{
			log_printf(LOG_ERR, "can't locate %s\n", path);
			return -1;
		}

		if(port_num == 0)
		{
			log_printf(LOG_INFO, "%s%d. doesn't contain %s\n", path, i, val);
			continue;
		}
		else
		{
			sprintf(path1, "%s1.PVID", path);
			if (MBus_GetParamVal(mbus, path1, val1, sizeof(val1)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", path);
				return -1;
			}
			else
			{
				return atoi(val1);
			}
		}
	}
	return -1;
}

int get_native_radio_status(int *rt)
{
	char val[MAX_BUF];

	if (rt == NULL)
	{
		log_printf(LOG_ERR, "invalid input\n");
		return -1;
	}
	*rt = 0;

	if (MBus_GetParamVal(mbus, "Device.WiFi.Radio.1.Enable", val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get radio 1 status failed\n");
		return -1;
	}

	if(!strcmp(val, "true"))
	{
		(*rt) |= 0x10000;
	}

	if (MBus_GetParamVal(mbus, "Device.WiFi.Radio.2.Enable", val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get radio 2 status failed\n");
		return -1;
	}

	if(!strcmp(val, "true"))
	{
		(*rt) |= 0x1;
	}

	return 0;
}

int get_bridge_factory_state()
{
	int ret = -1, bridge_num = MAX_INSTANCE, ins_num;
	char buf[MAX_BUF], insPath[MAX_INSTANCE][MAX_PATH_NAME], path[MAX_BUF], val1[MAX_BUF];

	while (syscfg_init() == -1)
	{
		log_printf(LOG_ERR, "init syscfg for wecb failed\n");
		sleep(1);
	}

	//handle the bridge instance number after factory reset
	memset(buf, 0, sizeof(buf));
	ret = syscfg_get(NULL, WECB_HS_BRIDGE_INS, buf, sizeof(buf));

	if(ret == -1 || atoi(buf) == 0)
	{
		//locate the last bridge instance number in DM
		if (MBus_FindObjectIns(mbus, "Device.Bridging.Bridge.", NULL, NULL, insPath, &bridge_num) != 0)
		{
			log_printf(LOG_ERR, "can't locate Device.Bridging.Bridge.\n");
			return -1;
		}
	
		if(sscanf(insPath[bridge_num - 1], "Device.Bridging.Bridge.%d.", &ins_num) != 1)
		{	
			log_printf(LOG_ERR, "get %s instance number failed\n", insPath[bridge_num - 1]);
			return -1;
		}
		sprintf(buf, "%d", ins_num + 1);
		printf("WECB bridge will start from %s\n", buf);
		syscfg_set(NULL, WECB_HS_BRIDGE_INS, buf);
		syscfg_commit();
	}

	/*
	//handle brlan instance number
	memset(buf, 0, sizeof(buf));
	ret = syscfg_get(NULL, WECB_HS_BRLAN_INS, buf, sizeof(buf));

	if(ret == -1 || atoi(buf) == 0)
	{
		sprintf(path, "%sPort.1.Name", insPath[bridge_num - 1]);
		
		if (MBus_GetParamVal(mbus, path, val1, sizeof(val1)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
	
		if(sscanf(val1, "brlan%d", &ins_num) != 1);
		{	
			log_printf(LOG_ERR, "get %s instance number failed\n", val1);
			return -1;
		}

		sprintf(buf, "%d", ins_num + 1);	
		printf("WECB brlan will start from %s\n", buf);
		syscfg_set(NULL, WECB_HS_BRLAN_INS, buf);
		syscfg_commit();
	}

	//handle PVID instance number
	memset(buf, 0, sizeof(buf));
	ret = syscfg_get(NULL, WECB_HS_PVID_INS, buf, sizeof(buf));

	if(ret == -1 || atoi(buf) == 0)
	{
		sprintf(path, "%sVLAN.1.VLANID", insPath[bridge_num - 1]);
		
		if (MBus_GetParamVal(mbus, path, val1, sizeof(val1)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
	
		ins_num = atoi(val1);
		sprintf(buf, "%d", ins_num + 1);	
		printf("WECB PVID will start from %s\n", buf);
		syscfg_set(NULL, WECB_HS_PVID_INS, buf);
		syscfg_commit();
	}*/

	return 0;
}

int disable_all_wecb_bridge()
{
	char buf[MAX_BUF], path[MAX_BUF], cmd[MAX_BUF]; 
	int j, i, len;
	FILE *fp = NULL;
	char *ptr = NULL, *saveptr;
	
	//got existed WECB dedicated bridge instance number from PSM
	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));
	sprintf(cmd, "psmcli get %s", WECB_HS_PSM_ENTRY);

	fp = popen(cmd, "r");

	if(fp == NULL)
	{
		log_printf(LOG_ERR, "open shell pipeline failed\n");
		return -1;
	}

	fread(buf, sizeof(char), sizeof(buf) - 1, fp);
	pclose(fp);
	ptr = buf;
	len = strlen(buf);	
	saveptr = buf;
	
	for(i = 0; i < len; i++)
	{
		if(buf[i] == '\n')
			buf[i] = '\0';
	}	

	for(i = 0; ptr && strlen(ptr) && i < HS_SSID_NUM; i++)
	{
		for(ptr = strsep(&saveptr, ",:"); ptr != NULL; ptr = strsep(&saveptr, ",:"))
		{
			if(!strlen(ptr)) 
				continue;
			if(ptr[0] == ':')
				ptr++;
			if(ptr[0] < '0' || ptr[0] > '9')
				continue;

			sprintf(path, "Device.Bridging.Bridge.%s.Enable", ptr);
		
			if (MBus_SetParamVal(mbus, path, MBUS_PT_BOOL, "false", 1) != 0)
			{
				log_printf(LOG_ERR, "Disable %s failed\n", path);
				return -1;
			}
		}
    }
	
	return 0;
}

int disable_specific_bridge(int ins)
{
	char path[MAX_BUF];

	if(ins <= 0)
		return -1;

	sprintf(path, "Device.Bridging.Bridge.%d.Enable", ins);

	if (MBus_SetParamVal(mbus, path, MBUS_PT_BOOL, "false", 1) != 0)
	{
		log_printf(LOG_ERR, "Disable %d failed\n", path);
		return -1;
	}
	return 0;
}


static int ensure_bridge_setting(const char *path, int port_index)
{
	int port_num, i = 0, newIns; 
	char buf[MAX_BUF], pt[MAX_BUF], val[MAX_BUF];

	sprintf(pt, "%sPortNumberOfEntries", path);

	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	sprintf(pt, "%sPort.", path);

	if(atoi(val) > BR_PORT_NUM)
	{
		for(i = atoi(val); i > BR_PORT_NUM; i--)
		{
			if (MBus_DelObjectIns(mbus, pt, i) != 0)
			{
				log_printf(LOG_ERR, "Delete extra bridge port failed\n");
				return -1;
			}
		}
	}
	else
	{
		for(i = atoi(val); i < BR_PORT_NUM; i++)
		{
			if (MBus_AddObjectIns(mbus, pt, &newIns) != 0)
			{
				log_printf(LOG_ERR, "Adding new bridge port failed\n");
				return -1;
			}
		}
	}
	/*	
	sprintf(pt, "%sVLAN.1.Name", path);
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	sprintf(pt, "%sPort.1.Name", path);
	if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, val, 1) != 0)
	{
		log_printf(LOG_ERR, "Set %s failed\n", pt);
		return -1;
	}*/

	sprintf(pt, "%sPort.1.ManagementPort", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	sprintf(pt, "%sPort.1.AcceptableFrameTypes", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	if(strcmp(val, "AdmitAll"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "AdmitAll", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.1.X_CISCO_COM_Mode", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "Tagging"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "Tagging", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.1.Enable", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	sprintf(pt, "%sPort.2.LowerLayers", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	if(strcmp(val, "Device.X_CISCO_COM_GRE.Interface.1"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "Device.X_CISCO_COM_GRE.Interface.1", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	sprintf(pt, "%sPort.2.AcceptableFrameTypes", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	if(strcmp(val, "AdmitAll"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "AdmitAll", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.2.X_CISCO_COM_Mode", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "PassThrough"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "PassThrough", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.2.Enable", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	
	sprintf(pt, "%sPort.3.LowerLayers", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, all_wecb_ports[port_index]))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, all_wecb_ports[port_index], 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.3.X_CISCO_COM_Mode", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "PassThrough"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_STRING, "PassThrough", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}
	
	sprintf(pt, "%sPort.3.Enable", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	sprintf(pt, "%sEnable", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}

	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}

	/*
	sprintf(pt, "%sPort.1.ManagementPort", path);
	
	if (MBus_GetParamVal(mbus, pt, val, sizeof(val)) != 0)
	{
		log_printf(LOG_ERR, "Get %s failed\n", pt);
		return -1;
	}
	
	if(strcmp(val, "true"))
	{
		if (MBus_SetParamVal(mbus, pt, MBUS_PT_BOOL, "true", 1) != 0)
		{
			log_printf(LOG_ERR, "Set %s failed\n", pt);
			return -1;
		}
	}*/
	
	//printf("setting complete\n");
	return 0;
}

int reserve_bridge(int ins[HS_SSID_NUM], int pvid[HS_SSID_NUM], int wecb_port)
{
	char buf[MAX_BUF], path[MAX_BUF], val[MAX_BUF], cmd[MAX_BUF], val1[MAX_BUF]; 
	int i, ret, j = HS_SSID_NUM, newIns, k;
	FILE *fp = NULL;
	char *saveptr = NULL, *ptr = NULL;
	
	//got existed WECB dedicated bridge instance number from PSM
	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));
	sprintf(cmd, "psmcli get %s", WECB_HS_PSM_ENTRY);

	fp = popen(cmd, "r");

	if(fp == NULL)
	{
		log_printf(LOG_ERR, "open shell pipeline failed\n");
		return -1;
	}

	ret = fread(buf, sizeof(char), sizeof(buf) - 1, fp);
	pclose(fp);
	ptr = buf;

	for(i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '\n')
			buf[i] = '\0';
	}	

	if(!strchr(buf, ':'))
	{
		for(i = 1; i < HS_SSID_NUM; i++)
		{
			strcat(buf, ":");
		}
	}
	strcpy(val1, buf);
	saveptr = buf;
	
	for(i = 0; ptr && strlen(ptr) && i < HS_SSID_NUM;)
	{
		for(ptr = strsep(&saveptr, ",:"); ptr != NULL; ptr = strsep(&saveptr, ",:"))
		{
			if(!strlen(ptr))
				continue;
			if(ptr[0] < '0' || ptr[0] > '9')
				continue;

			sprintf(path, "Device.Bridging.Bridge.%s.Enable", ptr);
		
			if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", path);
				return -1;
			}
		
			if(!strcmp(val, "false"))
			{
				ins[i] = atoi(ptr);
			}
			else
			{
				continue;
			}

			sprintf(path, "Device.Bridging.Bridge.%s.VLAN.1.VLANID", ptr);
		
			if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
			{
				log_printf(LOG_ERR, "Get %s failed\n", path);
				return -1;
			}
			pvid[i] = atoi(val);

			sprintf(path, "Device.Bridging.Bridge.%s.", ptr);
			if (ensure_bridge_setting(path, wecb_port) == -1)
			{
				log_printf(LOG_ERR, "Set %s failed\n", path);
				return -1;
			}

			//found an idle bridge for HHS
			i++;
			
			if(i < HS_SSID_NUM)
			{
				//printf("*************saveptr is %x\n", saveptr);
				if(saveptr && strchr(saveptr, ':'))
				{
					saveptr = strchr(saveptr, ':');
					if(saveptr)
						saveptr++;
				}
				//printf("++++++++++++++++saveptr is %x\n", saveptr);
				if(saveptr && i == HS_SSID_NUM - 1)
					strcat(saveptr, ",");
			}
			break;
		}
    }

	for(j = i; j < HS_SSID_NUM; j++)
	{
		if (MBus_AddObjectIns(mbus, "Device.Bridging.Bridge.", &newIns) != 0)
		{
			log_printf(LOG_ERR, "Adding new bridge failed\n");
			break;
		}

		//printf("********   %d\n", newIns);
		ins[j] = newIns;
		sprintf(path, "Device.Bridging.Bridge.%d.", newIns);
		if(ensure_bridge_setting(path, wecb_port) == -1)
		{
			log_printf(LOG_ERR, "config %s failed\n", path);
			break;
		}

		sprintf(path, "Device.Bridging.Bridge.%d.VLAN.1.VLANID", newIns);
		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			break;
		}
		pvid[j] = atoi(val);
		ins[j] = newIns;

		sprintf(val, "%d", newIns);
		if(j == 0)
		{
			//strcat(val, ",");
			if(strcmp(val1, ":"))
			{
				strcat(val, ",");
			}
			strcat(val, val1);
			strcpy(val1, val);
		}
		else
		{
			if(val1[strlen(val1) - 1] != ':')
			{
				strcat(val1, ",");
			}
			strcat(val1, val);
			//strcat(val1, ",");
		}
	}

	if(j < HS_SSID_NUM)
	{
		for(j = HS_SSID_NUM - 1; j >= i; j--)
		{
			if(ins[j] && MBus_DelObjectIns(mbus, "Device.Bridging.Bridge.", ins[j]))
			{
				log_printf(LOG_ERR, "Delete object Device.Bridging.Bridge.%d. failed\n", ins[j]);
			}
			ins[j] = 0;
			pvid[j] = 0;
		}
		return -1;
	}	
	if(i < HS_SSID_NUM)
	{
		sprintf(cmd, "psmcli set %s %s", WECB_HS_PSM_ENTRY, val1);
		fp = popen(cmd, "r");

		if(fp == NULL)
		{
			log_printf(LOG_ERR, "open shell pipeline failed\n");
			return -1;
		}

		memset(buf, 0, sizeof(buf));
		fread(buf, sizeof(char), sizeof(buf) - 1, fp);
		pclose(fp);
		if(atoi(buf) != 100)
		{
			log_printf(LOG_ERR, "psm save failed\n");
			for(j = i; j < HS_SSID_NUM; j++)
			{
				if(MBus_DelObjectIns(mbus, "Device.Bridging.Bridge.", ins[j]))
				{
					log_printf(LOG_ERR, "Delete object Device.Bridging.Bridge.%d failed\n", ins[j]);
				}
				pvid[j] = 0;
				ins[i] = 0;
			}
			return -1;
		}
	}
	//accord to lei, GRE only has 1 instance
	hs_bridge_notify("hotspot-update_bridges", "1");
	//system("sysevent set hotspot-update_bridges 1");
	return 0;
}

//Find which port WECB is connected on, 0: MoCA, 1-4: Ethernet port
int get_phy_port(char *target_ip)
{
	char                mac[6];
    CCSP_HAL_ETHSW_PORT EthswPort   = 0;

    if ( CcspHalEthSwInit() != RETURN_OK )
	{
		log_printf(LOG_ERR, "Init EthSw lib failed\n");
		return -1;
    }

	char insPath[MAX_INSTANCE][MAX_PATH_NAME], moca_device_path[1][MAX_PATH_NAME];
	int device_num = MAX_INSTANCE, i = 0, sw_port = -1, moca_wecb_num = 1, j;
	char path[MAX_BUF], val[MAX_BUF];
	char buf[MAX_BUF], cmd[MAX_BUF]; 
	FILE *fp = NULL;
        char lan_ifname[64];

	if(!target_ip)
	{
		log_printf(LOG_ERR, "illegal WECB IP address\n");
		return -1;
	}

        //force usg to refresh ARP cache for APS1
        syscfg_get(NULL, "lan_ifname", lan_ifname, sizeof(lan_ifname));
        sprintf(buf, "ping %s -c 2 -I %s -W 1 >/dev/null 2>&1", target_ip, lan_ifname);
        system(buf);
	/*
	if (MBus_FindObjectIns(mbus, "Device.DHCPv4.Server.Pool.1.Client.", "IPv4Address.1.IPAddress", target_ip, insPath, &device_num) != 0)
	{
        log_printf(LOG_ERR, "can't locate Device.DHCPv4.Server.Pool.1.Client\n");
		return -1;
	}

	for(i = 0; i < device_num; i++)
	{
		sprintf(path, "%sChaddr", insPath[i]);
		if (MBus_GetParamVal(mbus, path, val, sizeof(val)) != 0)
		{
			log_printf(LOG_ERR, "Get %s failed\n", path);
			return -1;
		}
		*/

	//got existed WECB dedicated bridge instance number from PSM
	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));
	sprintf(cmd, "ip nei | grep %s | awk '{print $5}'", target_ip);

	fp = popen(cmd, "r");

	if(fp == NULL)
	{
		log_printf(LOG_ERR, "open shell pipeline failed\n");
		return -1;
	}

	fread(buf, sizeof(char), sizeof(buf) - 1, fp);
	pclose(fp);

	for(j = 0; j < MAX_BUF; j++)
	{
		if(buf[j] == '\n')
			buf[j] = '\0';
		buf[j] = toupper(buf[j]);
	}

	//printf("********%s*****\n", buf);
	if (MBus_FindObjectIns(mbus, "Device.MoCA.Interface.1.AssociatedDevice.", "MACAddress", buf, moca_device_path, &moca_wecb_num) != 0)
	{
		log_printf(LOG_ERR, "can't locate Device.MoCA.Interface.1.AssociatedDevice\n");
		return -1;
	}

	//printf("%s:%d\n", val, moca_wecb_num);
	if(moca_wecb_num)
		return 0;

	memset(mac, 0, sizeof(mac));
	sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	//printf("*******%x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	//find the MAC address in Ethernet switch
    if ( CcspHalEthSwLocatePortByMacAddress(mac, &EthswPort) != RETURN_OK ) 
	{
		log_printf(LOG_ERR, "CcspHalEthSwLocatePortByMacAddress failed\n");
		return -1;
	}
    else
    {
        sw_port = EthswPort - CCSP_HAL_ETHSW_EthPort1 + 1;
        return sw_port;
    }
}

int check_wecb_bridge(int i, int port_index)
{
	char path[MAX_BUF];

	sprintf(path, "Device.Bridging.Bridge.%d.", i);
	return ensure_bridge_setting(path, port_index);
}
