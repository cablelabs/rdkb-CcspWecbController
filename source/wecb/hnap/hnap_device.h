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

#ifndef _HNAP_DEVICE_H
#define _HNAP_DEVICE_H

#include "hdk.h"
#include "hdk_client_http_interface.h"
#include "hdk_client_methods.h"
#include "../wecb_common.h"
#include <ctype.h>
#include <math.h>
#include <strings.h>

// HNAP device type supported in WECB
#define DEVICE_WIFI_EXTENDER                                                "hnap:WiFiExtender"
#define DEVICE_WIFI_EXTENDER_V2                                             "hnap:WiFiExtenderV2" 
#define DEVICE_WIFI_EXTENDER_SYNCED                                         "hnap:WiFiExtenderSynced"
#define DEVICE_WIFI_EXTENDER_SYNCED_V2                                      "hnap:WiFiExtenderSyncedV2"
#define	WECB_USER								"cusadmin"
#define WECB_PW								    "Xfinity"
struct radio_cfg
{
	int band_mask; //0x1 2.4G, 0x2 5G
	int mode_mask; //0 for non-ac, 1 5G only ac, 3 for 2.4/5G ac
};
bool SetTOD(HDK_ClientContext *pCtx, void *rt);
bool IsDeviceReady(HDK_ClientContext *pCtx, void *rt);
bool SetDoRestart(HDK_ClientContext *pCtx, void *rt);
bool GetDeviceSettings(HDK_ClientContext *pCtx, void *rt);
bool GetClientInfo(HDK_ClientContext *pCtx, void *rt, bool is_v2);
bool GetWlanRadios(HDK_ClientContext *pCtx, void *rt);
bool GetExtenderStatus(HDK_ClientContext *pCtx, void *rt);
void* clientctx_init(const char *url, const char *user, const char *pw);
bool clientctx_uninit(HDK_ClientContext *pCtx);
bool SetWPS(HDK_ClientContext *pCtx, void *rt);
//using rt to indicate if we need to force radio down
bool SetRadios(HDK_ClientContext *pCtx, void *rt);
bool SetSSIDSettings(HDK_ClientContext *pCtx, void *rt);
bool mbus_uninit();
bool mbus_init();
int set_moca_bridge();
int get_primary_lan_pvid();
int find_ssid_pvid(int ssid_ins);
int get_bridge_factory_state();
int reserve_bridge(int ins[HS_SSID_NUM], int pvid[HS_SSID_NUM], int port_index);
int disable_all_wecb_bridge();
int get_native_radio_status(int *rt);
static int ensure_bridge_setting(const char *path, int port_index);
int disable_specific_bridge(int ins);
int get_phy_port(char *target_ip);
#if 0
void update_online_client(char *ip, char *mac, char flag);
void Clean_DM();
#endif
int force_radio_down(char *ip);
int check_wecb_bridge(int i, int port_index);
#endif
