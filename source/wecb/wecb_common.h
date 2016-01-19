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

#ifndef WECB_COMMON_H
#define WECB_COMMON_H

//We won't save any information about USG native ssid in MoCA DM
#define MAX_SSID									4
#define SSID_QOS_NUM								4
#define MAX_RETRY									2
#define TIME_SYNC_INTERVAL							240*3600
#define SSDP_INTERVAL								60
#define SYNC_INTERVAL								60
#define ALIVE_CHCEK_INTERVAL						5
#define WECB_SK										"/tmp/wecb_pam_socket"
#define MAX_EXT_CLIENT								64
#define MAX_EXT										8
#define MAX_EXT_SSID								32
#define MAX_EXT_NUM									8

//For AP isolation, only supports Hotspot SSID now
#define HS_SSID_NUM									2
#define HS_SSID_INS									5
#define WECB_HS_BRIDGE_INS							"wecb_hhs_bridge_ins"
#define WECB_HS_BRLAN_INS							"wecb_hhs_brlan_ins"
#define WECB_HS_PVID_INS							"wecb_hhs_pvid_ins"
#define BR_PORT_NUM									3
#define WECB_HS_PSM_ENTRY							"dmsb.wecb.hhs_extra_bridges"

enum PAM_EVENT{SSID_SET=0x1, RADIO_SET=0x2, WPS_SET=0x4, TIME_SET=0x8, QUERY_ALL=0xFFFFFFFF};
//which interface clients connected on WECB
enum EXT_INF{WIFI_24=1, WIFI5, ETH};
enum EXT_STATUS{INIT=1, SYNCING, SYNCED};


struct ExtClient
{
	char name[36];
	char mac[20];
	int inf;
};

struct ExtSSID
{
	char ssid[64];
	char bssid[20];
	int channel;
   	int mode;
	int band;
	int sec_mode;
	int encry_mode;
};

struct ExtStatus
{

	char ip[80];
	char device_name[36];
	char vendor_name[36];
	char model_name[36];
	char fw_version[36];
	int radio_mask;
	int ext_ssid_num;
	int ext_client_num;
	int status;
	struct ExtClient clients[MAX_EXT_CLIENT];
	struct ExtSSID ssid[MAX_EXT_SSID];
};


#endif
