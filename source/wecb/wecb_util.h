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

    module: wecb_util.h

        For WECB control development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implements wecb utility.

    -------------------------------------------------------------------


    author:

        Rongwei

    -------------------------------------------------------------------

    revision:

        11/02/2012    initial revision.

**************************************************************************/

#ifndef __WECB_UTIL_H
#define __WECB_UTIL_H

#define MAX_STRING_LEN  256
#define WECB_IF "eth4"

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 4
#endif 

#include "string.h"
//#include "ixml.h"
#include "signal.h"
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include "hnap/hdk_data.h"
#include "hnap/hdk.h"
#include <sysevent/sysevent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include "hnap/hdk.h"
#include "wecb_common.h"
#include <stdbool.h>

#define WECB_STACK_SIZE 1024*1024

enum {EVENT_ERROR=-1, EVENT_OK, EVENT_TIMEOUT, EVENT_WAN_UP=0x10, EVENT_WAN_DOWN, EVENT_MOCA_UP=0x20, EVENT_MOCA_DOWN, EVENT_LAN_IP_SET=0x40, EVENT_WECB_HHS_BRIDGE};


bool WECB_CheckNoneEmpty(const char * src);
void wecb_reg_signal();
int wecb_get_if_ip(char *if_name, char *ip, int len);
unsigned int htoi(char s[]);
bool stringtoip(char s[], HDK_IPAddress *ip);
int wecb_event_listen(int *val);
int wecb_event_inits();
int wecb_event_close();
int wecb_sync_with_pam(unsigned int);
int wecb_sk_server();
bool wecb_push_settings(char *device_id);
int wecb_sk_server();
void wecb_global_uninit();
void wecb_global_init();
void wecb_sync_sys_time();
bool stringtoip(char *s, HDK_IPAddress *ip);
bool wecb_sync_thread(char *id);
void hs_bridge_notify(char *evt, char *val);

#endif
