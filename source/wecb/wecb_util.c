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

    module: wecb_util.c

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
#include "wecb_util.h"
#include <syslog.h>
#include "wecb_upnp_agent.h"
#include "hnap/hnap_device.h"
#include "libxml/parser.h"
#include "hnap/hdk.h"
#include "hnap/hdk_client_http_interface.h"
#include "sysevent/libsysevent_internal.h"
#include <openssl/crypto.h>
#include "syscfg/syscfg.h"
#include "autoconf.h"

sem_t sem;
int	recv_signal = 0;
pthread_attr_t wecb_attr;
extern pthread_mutex_t device_list_mutex;
static int se_fd1 = 0, ext_count = 0; 
static token_t token1;
static async_id_t async_id;
static short server_port;
static char  server_ip[19];

// For DM and SNMP MIB using, this struct is no lock-protected, may unconsistent
static struct ExtStatus wecb_status[MAX_EXT];

static int entry_index;
static pthread_mutex_t index_mutex;

static pthread_mutex_t *lock_cs;
static long *lock_count;
static void threads_locking_cb(int mode,int type,char *file,int line);
static unsigned long threads_id_cb(void );

void CRYPTO_thread_setup(void)
{
	int i;

	lock_cs = malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	lock_count = malloc(CRYPTO_num_locks() * sizeof(long));

	for (i=0; i<CRYPTO_num_locks(); i++)
	{
		lock_count[i]=0;
		pthread_mutex_init(&(lock_cs[i]),NULL);
	}

	CRYPTO_set_id_callback((unsigned long (*)())threads_id_cb);
	CRYPTO_set_locking_callback((void (*)())threads_locking_cb);
}

void threads_locking_cb(int mode, int type, char *file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		pthread_mutex_lock(&(lock_cs[type]));
		lock_count[type]++;
	}
	else
	{
		pthread_mutex_unlock(&(lock_cs[type]));
	}
}

unsigned long threads_id_cb(void)
{
	return (unsigned long)pthread_self();
}

bool WECB_CheckNoneEmpty(const char * src)
{
    if(!src) return FALSE;
    char * p = (char *)src;
    while(*p == ' ') p++;
    if(strlen(p) <= 0) return FALSE;
    return TRUE;
}

void wecb_reg_signal()
{
	signal(SIGINT,  wecb_sig_handler);
	signal(SIGQUIT, wecb_sig_handler);
	signal(SIGABRT, wecb_sig_handler);
	signal(SIGTERM, wecb_sig_handler);
	signal(SIGSEGV, wecb_sig_handler);
	signal(SIGCHLD, SIG_IGN);
}



//ip should be long enough to contain ip address
int wecb_get_if_ip(char *if_name, char *ip, int len)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char *p = NULL;

	if(if_name == NULL || !strlen(if_name))
	{
		log_printf(LOG_ERR, "bad interface name\n");
		return -1;
	}

	if (getifaddrs(&ifaddr) < 0) 
	{
		log_printf(LOG_ERR, "getifaddrs failed\n");
		return -1;
    }

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
		{
			log_printf(LOG_ERR, "interface %s has no address\n", ifa->ifa_name);
			continue;
		}
		family = ifa->ifa_addr->sa_family;

		//only support ipv4 now
		if(family == AF_INET)
		{
			p = inet_ntop(AF_INET, (const void *)&(((struct sockaddr_in *)((*ifa).ifa_addr))->sin_addr), ip, len);
			//s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if(p == NULL) 
			{
				log_printf(LOG_ERR, "Convert IP failed\n");
				freeifaddrs(ifaddr);
				return -1;
			}
			else if(!strcmp(ifa->ifa_name, if_name))
			{
				log_printf(LOG_INFO, "Got the ip address %s: %s\n", if_name, ip);
				//strncpy(ip, host, len - 1);
				freeifaddrs(ifaddr);
				return 0;
			}
		}/*
		else if(family == AF_INET6)
		{
			p = inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)((*ifa).ifa_addr))->sin6_addr), ip, len);
			//s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if(p == NULL) 
			{
				log_printf(LOG_ERR, "Convert IP failed\n");
				freeifaddrs(ifaddr);
				return -1;
			}
			else if(!strcmp(ifa->ifa_name, if_name))
			{
				log_printf(LOG_INFO, "Got the ip address %s: %s\n", if_name, ip);
				//strncpy(ip, host, len - 1);
				freeifaddrs(ifaddr);
				return 0;
			}
		}*/


	}
	return -1;
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

bool stringtoip(char *s, HDK_IPAddress *ip)
{
	int i = 0;
	char *p1, *p2;
	
	if(s == NULL || ip == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
		return false;
	}

	p1 = strchr(s, '.');
	if(p1 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p1++ = '\0';
	ip->a = atoi(s);

	p2 = strchr(p1, '.');
	if(p2 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p2++ = '\0';
	ip->b = atoi(p1);

	p1 = strchr(p2, '.');
	if(p1 == NULL)
	{
		log_printf(LOG_ERR, "wrong ip adress string\n");
		return false;
	}
	*p1++ = '\0';
	ip->c = atoi(p2);
	ip->d = atoi(p1);
	log_printf(LOG_INFO, "string %s, ip address %d.%d.%d.%d", s, ip->a, ip->b, ip->c, ip->d);
	return true;
}

void hs_bridge_notify(char *evt, char *val)
{
	int hs_fd;
    token_t hs_token;

	hs_fd = sysevent_open("127.0.0.1", SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, "hhs_notify", &hs_token);

	if (!hs_fd) 
	{
		log_printf(LOG_ERR, "open sysevent handle failed\n");
	}
	else
	{
		if(sysevent_set(hs_fd, hs_token, evt, val, 0))
		{
			log_printf(LOG_ERR, "send event %s failed\n", evt);
		}
		sysevent_close(hs_fd, hs_token);
	}
}

/*
 * Initialize sysevnt 
 *   return 0 if success and -1 if failture.
 */
int wecb_event_inits()
{
    int rc;

    snprintf(server_ip, sizeof(server_ip), "127.0.0.1");
    server_port = SE_SERVER_WELL_KNOWN_PORT;

    se_fd1 = sysevent_open(server_ip, server_port, SE_VERSION, "wecb_master", &token1);
    if (!se_fd1) {
        log_printf(LOG_ERR, "Unable to register with sysevent daemon.\n");
        return(EVENT_ERROR);
    }

	//register wan up event
    //sysevent_set_options(se_fd1, token1, "phylink_wan_state", TUPLE_FLAG_SERIAL);
    sysevent_set_options(se_fd1, token1, "phylink_wan_state", TUPLE_FLAG_EVENT);
    rc = sysevent_setnotification(se_fd1, token1, "phylink_wan_state", &async_id);
    if (rc) {
       log_printf(LOG_ERR, "cannot set request for client %s events %s\n", "wecb_master", "phylink_wan_state");
       log_printf(LOG_ERR, "                Reason (%d) %s\n", rc, SE_strerror(rc));
       return(EVENT_ERROR);
    } 

	//register moca up event
    //sysevent_set_options(se_fd1, token1, "desired_moca_link_state", TUPLE_FLAG_SERIAL);
    sysevent_set_options(se_fd1, token1, "desired_moca_link_state", TUPLE_FLAG_EVENT);
    rc = sysevent_setnotification(se_fd1, token1, "desired_moca_link_state", &async_id);
    if (rc) {
       log_printf(LOG_ERR, "cannot set request for client %s events %s\n", "wecb_master", "desired_moca_link_state");
       log_printf(LOG_ERR, "                Reason (%d) %s\n", rc, SE_strerror(rc));
       return(EVENT_ERROR);
    } 

	//register lan ip set event
    sysevent_set_options(se_fd1, token1, "current_lan_ipaddr", TUPLE_FLAG_EVENT);
    rc = sysevent_setnotification(se_fd1, token1, "current_lan_ipaddr", &async_id);
    if (rc) {
       log_printf(LOG_ERR, "cannot set request for client %s events %s\n", "wecb_master", "current_lan_ipaddr");
       log_printf(LOG_ERR, "                Reason (%d) %s\n", rc, SE_strerror(rc));
       return(EVENT_ERROR);
    } 
	
	//register wecb_hhs_bridges set event
    sysevent_set_options(se_fd1, token1, " wecb_hhs_bridges", TUPLE_FLAG_EVENT);
    rc = sysevent_setnotification(se_fd1, token1, "wecb_hhs_bridges", &async_id);
    if (rc) {
       log_printf(LOG_ERR, "cannot set request for client %s events %s\n", "wecb_master", "wecb_hhs_bridges");
       log_printf(LOG_ERR, "                Reason (%d) %s\n", rc, SE_strerror(rc));
       return(EVENT_ERROR);
    } 

	log_printf(LOG_INFO, "event_inits: Ready to receive updates to %s\n", "wecb_master");
    
    return(EVENT_OK);
}

int wecb_event_listen(int *val)
{
    int ret=EVENT_TIMEOUT;
    fd_set rfds;
    struct timeval tv;
    int retval;


        tv.tv_sec = 30;
        tv.tv_usec=0;
        FD_ZERO(&rfds);
        FD_SET(se_fd1, &rfds);

        log_printf(LOG_INFO, "Waiting for event ... \n");
        retval=select(se_fd1+1, &rfds, NULL, NULL, NULL);

        if(retval) {
            se_buffer            msg_buffer;
            se_notification_msg *msg_body = (se_notification_msg *)msg_buffer;
            unsigned int         msg_size;
            token_t              from;
            int                  msg_type;

            msg_size  = sizeof(msg_buffer);
            msg_type = SE_msg_receive(se_fd1, msg_buffer, &msg_size, &from);
            // if not a notification message then ignore it
            if (SE_MSG_NOTIFICATION == msg_type) {
               // extract the name and value from the return message data
              int   name_bytes;
              int   value_bytes;
              char *name_str;
              char *value_str;
              char *data_ptr;

              data_ptr   = (char *)&(msg_body->data);
              name_str   = (char *)SE_msg_get_string(data_ptr, &name_bytes);
              data_ptr  += name_bytes;
              value_str =  (char *)SE_msg_get_string(data_ptr, &value_bytes);

              log_printf(LOG_INFO, "Received event <%s %s>\n", name_str, value_str);
			  if(!strcmp(name_str, "desired_moca_link_state"))
			  {
			      if (!strncmp(value_str, "up", 2)) 
                      ret = EVENT_MOCA_UP;
				  else if (!strncmp(value_str, "down", 4)) 
                      ret = EVENT_MOCA_DOWN;
			  }
			  else if(!strcmp(name_str, "phylink_wan_state"))
			  {
			      if (!strncmp(value_str, "up", 2)) 
                      ret = EVENT_WAN_UP;
				  else if (!strncmp(value_str, "down", 4)) 
                      ret = EVENT_WAN_DOWN;
			  }
			  else if(!strcmp(name_str, "ipv4_4-status"))//"current_lan_ipaddr"))
			  {
				  /*
				  HDK_IPAddress lan_ip;
				  
				  if(stringtoip(value_str, &lan_ip) == true)
				  {
					  ret = EVENT_LAN_IP_SET;
				  }*/
				  //Primary lan stopped, need to quit itself
				  if(!strcmp(value_str, "down"))
				  {
					  ret = EVENT_LAN_IP_SET;
				  }
			  }
			  else if(!strcmp(name_str, "wecb_hhs_bridges"))
			  {
				  *val = atoi(value_str);
				  ret = EVENT_WECB_HHS_BRIDGE;
			  }
            } else {
               log_printf(LOG_WARNING, "Received msg that is not a SE_MSG_NOTIFICATION (%d)\n", msg_type);
            }
        } else {
           log_printf(LOG_ERR, "Received no event retval=%d\n", retval);
        }
    return ret;
}

int wecb_event_close()
{
    /* we are done with this notification, so unregister it using async_id provided earlier */
    sysevent_rmnotification(se_fd1, token1, async_id);

    /* close this session with syseventd */
    sysevent_close(se_fd1, token1);

    return (EVENT_OK);
}

int wecb_sk_server()
{
	int lsn_fd, pam_fd;
	struct sockaddr_un srv_addr;
	struct sockaddr_un clt_addr;
	socklen_t clt_len;
	int ret;
	int i, j = 0;
	enum PAM_EVENT event;
	int opt = SO_REUSEADDR;

	//create socket to bind local IP and PORT
	lsn_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(lsn_fd < 0)
	{
		log_printf(LOG_ERR, "can't create communication socket!");
		return -1;
	}
  
	unlink(WECB_SK);
	//create local IP and PORT
	srv_addr.sun_family = AF_UNIX;
	strncpy(srv_addr.sun_path, WECB_SK, sizeof(srv_addr.sun_path) - 1);
	setsockopt(lsn_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

	//bind sockfd and sockaddr
	ret = bind(lsn_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret == -1)
	{
		log_printf(LOG_ERR, "can't bind local sockaddr!");
		close(lsn_fd);
		return 1;
	}
  
	//listen lsn_fd, try listen up to 10 connections
	ret = listen(lsn_fd, 10);
	log_printf(LOG_INFO, "server is waiting on %d\n", lsn_fd);
	
	while(1)
	{
		event = 0;
		pam_fd = accept(lsn_fd, (struct sockaddr*)&clt_addr, &clt_len);
		
		if(pam_fd < 0)
		{
			log_printf(LOG_ERR, "can't listen client connect request\n");
		}
		else
		{
			ret = recv(pam_fd, &event, sizeof(event), 0);
			//received WiFi updates from ATOM
			if(event != QUERY_ALL)
				wecb_sync_with_pam(event);
			//ARM is querying WECB status
			else
			{
				pthread_mutex_lock(&index_mutex);
			
				if (send(pam_fd, &ext_count, sizeof(ext_count), 0) < 0)
				{
					log_printf(LOG_ERR, "wecb_master send extender count failed\n");
				}
				else
				{
					for(i = 1, j = 0; j < MAX_EXT && i <= 0xFF; i *= 2, j++)
					{
						if ((entry_index ^ i) & i)
						{
							if (send(pam_fd, wecb_status + j, sizeof(struct ExtStatus), 0) < 0)
							{		
								log_printf(LOG_ERR, "wecb_master send extender count failed\n");
								break;
							}
						}
					}
				}
				pthread_mutex_unlock(&index_mutex);
			}
			log_printf(LOG_INFO, "received %d from pam\n", event);
			close(pam_fd);
		}
	}
	return 0;
}


int wecb_sync_with_pam(unsigned int cur_event)
{

	if(cur_event & RADIO_SET)
	{
		log_printf(LOG_INFO, "PAM radio set changed, need re-sync WECB device\n");
	}
	if(cur_event & SSID_SET)
	{
		log_printf(LOG_INFO, "PAM radio set changed, need re-sync WECB device\n");
	}
	if(cur_event & WPS_SET)
	{
		log_printf(LOG_INFO, "PAM WPS set changed, need re-sync WECB device\n");
	}
	if(cur_event & TIME_SET)
	{
		log_printf(LOG_INFO, "Periodicaly sync WECB device time\n");
	}
	pthread_mutex_lock(&device_list_mutex);
	Hnap_UpdateAllDevice_NotifyMask(cur_event);
	pthread_mutex_unlock(&device_list_mutex);
	return 0;
}
#if 0
bool wecb_push_settings(char *id)
{
	if(id == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
		return false;
	}
	char device_id[MAX_STRING_LEN];
	memset(device_id, 0, sizeof(device_id));
	strncpy(device_id, id, MAX_STRING_LEN - 1);
	free(id);

	PHnapDevice					pDevice = NULL;
	unsigned int mask = 0;
	char uri[128], *p;
	HDK_ClientContext *pCtx; 
	int hnap_retry = 0;
	memset(uri, 0, sizeof(uri));
	int ret = false;
	char addr[32];

	pthread_mutex_lock(&device_list_mutex);
	pDevice = Hnap_FindDeviceByUUID(device_id);
	if(pDevice == NULL)
	{
		pthread_mutex_unlock(&device_list_mutex);
		log_printf(LOG_WARNING, "Device %s is offline, no need to sync\n");
		goto EXIT1;
	}
	//another thread is syncing this device
	log_printf(LOG_WARNING, "check if device is syncing\n");
	if(pDevice->is_syncing == true)
	{
		pthread_mutex_unlock(&device_list_mutex);
		log_printf(LOG_WARNING, "Device %s is syncing\n", device_id);
		goto EXIT1;
	}
	pDevice->sync_mask |= pDevice->notify_mask;
	mask = pDevice->sync_mask;
	pDevice->notify_mask = 0;
	if(pDevice->sync_mask != 0)
	{
		pDevice->is_syncing = true;
	}
	else
	{
		pthread_mutex_unlock(&device_list_mutex);
		log_printf(LOG_WARNING, "no need to sync device\n");
		goto EXIT1;
	}
	//p = inet_ntoa(pDevice->addr);
	p = inet_ntop(AF_INET, (void *)&pDevice->addr, addr, sizeof(addr));
	pthread_mutex_unlock(&device_list_mutex);

	if(p != NULL)
	{
		sprintf(uri, "https://%s:2241/HNAP1/", addr);
		log_printf(LOG_WARNING, "begin to push settings to %s\n", uri);
	}
	else
	{
		log_printf(LOG_ERR, "Error network address\n");
		goto EXIT1;
	}

	pCtx = clientctx_init(uri, WECB_USER, WECB_PW);

	if(pCtx == NULL)
	{
		log_printf(LOG_ERR, "init client context failed\n");
		goto EXIT1;
	}

	while(hnap_retry < MAX_RETRY)	
	{

		if(IsDeviceReady(pCtx, NULL) == false)
		//if(GetDeviceSettings(pCtx, NULL) == false)
		{
			log_printf(LOG_WARNING, "Device is not ready\n");
			hnap_retry++;
			if(hnap_retry >= MAX_RETRY)
			{
				log_printf(LOG_ERR, "Device:%s is not ready\n", uri);
				pthread_mutex_lock(&device_list_mutex);
				pDevice = Hnap_FindDeviceByUUID(device_id);
				if(pDevice != NULL)
				{
					//pDevice->is_syncing = false;
				}
				pthread_mutex_unlock(&device_list_mutex);
				goto EXIT;
			}
		}
		else
		{
			hnap_retry = 0;
			break;
		}
	}

	pthread_mutex_lock(&device_list_mutex);
	pDevice = Hnap_FindDeviceByUUID(device_id);
	if(pDevice != NULL)
	{
		//pDevice->sync_mask = 0;
		pDevice->is_syncing = false;
	}
	pthread_mutex_unlock(&device_list_mutex);
	goto EXIT;

	if((mask == 0xFFFFFFFF) || (mask & WPS_SET))
	{
		while(hnap_retry < MAX_RETRY)	
		{

			if(SetWPS(pCtx, NULL) == false)
			{
				log_printf(LOG_WARNING, "SetWPS failed\n");
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "Device:%s SetWPS failed\n", uri);
					pthread_mutex_lock(&device_list_mutex);
					pDevice = Hnap_FindDeviceByUUID(device_id);
					if(pDevice != NULL)
					{
						//pDevice->is_syncing = false;
					}
					pthread_mutex_unlock(&device_list_mutex);
					//goto EXIT;
					hnap_retry = 0;
					break;
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}

	}

	if((mask == 0xFFFFFFFF) || (mask & RADIO_SET))
	{
		while(hnap_retry < MAX_RETRY)	
		{

			if(SetRadios(pCtx, NULL) == false)
			{
				log_printf(LOG_WARNING, "SetRadios failed\n");
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "Device:%s SetRadios failed\n", uri);
					pthread_mutex_lock(&device_list_mutex);
					pDevice = Hnap_FindDeviceByUUID(device_id);
					if(pDevice != NULL)
					{
						//pDevice->is_syncing = false;
					}
					pthread_mutex_unlock(&device_list_mutex);
					//goto EXIT;
					hnap_retry = 0;
					break;
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}

	}

	if((mask == 0xFFFFFFFF) || (mask & SSID_SET))
	{
		while(hnap_retry < MAX_RETRY)	
		{

			if(SetSSIDSettings(pCtx, NULL) == false)
			{
				log_printf(LOG_WARNING, "SetSSIDSettings failed\n");
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "Device:%s SetSSIDSettings failed\n", uri);
					pthread_mutex_lock(&device_list_mutex);
					pDevice = Hnap_FindDeviceByUUID(device_id);
					if(pDevice != NULL)
					{
						//pDevice->is_syncing = false;
					}
					pthread_mutex_unlock(&device_list_mutex);
					//goto EXIT;
					hnap_retry = 0;
					break;
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}

	}

	if((mask == 0xFFFFFFFF) || (mask & TIME_SET))
	{
		while(hnap_retry < MAX_RETRY)	
		{

			if(SetTOD(pCtx, NULL) == false)
			{
				log_printf(LOG_WARNING, "SetTOD failed\n");
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "Device:%s SetTOD failed\n", uri);
					pthread_mutex_lock(&device_list_mutex);
					pDevice = Hnap_FindDeviceByUUID(device_id);
					if(pDevice != NULL)
					{
						//pDevice->is_syncing = false;
					}
					pthread_mutex_unlock(&device_list_mutex);
					//goto EXIT;
					hnap_retry = 0;
					break;
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}

	}


	while(hnap_retry < MAX_RETRY)	
	{

		if(SetDoRestart(pCtx, NULL) == false)
		{
			log_printf(LOG_WARNING, "SetDoRestart failed\n");
			hnap_retry++;
			if(hnap_retry >= MAX_RETRY)
			{
				log_printf(LOG_ERR, "Device:%s SetDoRestart\n", uri);
				pthread_mutex_lock(&device_list_mutex);
				pDevice = Hnap_FindDeviceByUUID(device_id);
				if(pDevice != NULL)
				{
					//pDevice->is_syncing = false;
				}
				pthread_mutex_unlock(&device_list_mutex);
				//goto EXIT;
				hnap_retry = 0;
				break;
			}
		}
		else
		{
			hnap_retry = 0;
			break;
		}
	}

	while(hnap_retry < MAX_RETRY)	
	{

		if(IsDeviceReady(pCtx, NULL) == false)
		{
			log_printf(LOG_WARNING, "Device is not ready\n");
			hnap_retry++;
			if(hnap_retry >= MAX_RETRY)
			{
				log_printf(LOG_ERR, "Device:%s is not ready\n", uri);
				pthread_mutex_lock(&device_list_mutex);
				pDevice = Hnap_FindDeviceByUUID(device_id);
				if(pDevice != NULL)
				{
					//pDevice->is_syncing = false;
				}
				pthread_mutex_unlock(&device_list_mutex);
				//goto EXIT;
				hnap_retry = 0;
				break;
			}
		}
		else
		{
			break;
		}
	}
	//setting pushed success
	ret = true;
	pthread_mutex_lock(&device_list_mutex);
	pDevice = Hnap_FindDeviceByUUID(device_id);
	if(pDevice != NULL)
	{
		//pDevice->sync_mask = 0;
		pDevice->is_syncing = false;
	}
	pthread_mutex_unlock(&device_list_mutex);


EXIT:
	clientctx_uninit(pCtx);
EXIT1:
	//free(device_id);
	return true;
}
#endif

bool wecb_sync_thread(char *id)
{
	char device_id[MAX_STRING_LEN];
	if(id == NULL)
	{
		log_printf(LOG_ERR, "error input\n");
		return false;
	}
	memset(device_id, 0, sizeof(device_id));
	strncpy(device_id, id, MAX_STRING_LEN - 1);
	free(id);

	PHnapDevice	pDevice = NULL;
	unsigned int mask = 0;
	char uri[128], *p;
	HDK_ClientContext *pCtx = NULL; 
	int hnap_retry = 0;
	memset(uri, 0, sizeof(uri));
	int ret = false, radio_down = 0, i = 0, j = 0, index = -1;
	char addr[160]; 
        bool v2_cap = false;

	pthread_t self = pthread_self();	

	while(pCtx == NULL)
	{
		pthread_mutex_lock(&device_list_mutex);
		pDevice = Hnap_FindDeviceByUUID(device_id);
		if(pDevice == NULL)
		{
			pthread_mutex_unlock(&device_list_mutex);
			log_printf(LOG_WARNING, "Device %s is offline, no need to sync\n", addr);
			goto EXIT;
		}
		
		//pDevice->thread = self;
		if(pDevice->v6_expires > 0)
			strncpy(addr, pDevice->addr6, sizeof(addr) -1);
		else if(pDevice->v4_expires > 0)
			strncpy(addr, pDevice->addr, sizeof(addr) -1);

		pthread_mutex_unlock(&device_list_mutex);
		//p = inet_ntoa(pDevice->addr);

		if(strchr(addr, ':'))
			sprintf(uri, "https://[%s]:2241/HNAP1/", addr);
		else
			sprintf(uri, "https://%s:2241/HNAP1/", addr);
		pCtx = clientctx_init(uri, WECB_USER, WECB_PW);

		if(pCtx == NULL)
		{
			log_printf(LOG_ERR, "init client context failed\n");
			sleep(1);
		}
                else
                {
                        log_printf(LOG_WARNING, "Device %s is connected\n", uri);
                }
	}

	while(1)
	{
		pthread_mutex_lock(&device_list_mutex);
		pDevice = Hnap_FindDeviceByUUID(device_id);
		
		if(pDevice == NULL || !pDevice->thread || !pthread_equal(pDevice->thread, self))
		{
			log_printf(LOG_WARNING, "Device %s is offline, exit myself\n", addr);
			pthread_mutex_unlock(&device_list_mutex);
			goto EXIT;
		}
		pDevice->sync_mask |= pDevice->notify_mask;
		mask = pDevice->sync_mask;
                //in case v2 & v1 mixed upnp, we treat as v2
                if(!v2_cap && strstr(pDevice->device_type, "V2"))
                {
                    v2_cap = true;
                }
		pDevice->notify_mask = 0;
		pDevice->restart_pending = false;

		//if(memcmp(&device_addr, &(pDevice->addr), sizeof(device_addr)))
		//in case device has been move to another VLAN
		if((pDevice->v6_expires > 0 && strcmp(addr, pDevice->addr6)) || 
		   (pDevice->v6_expires <= 0 && strcmp(addr, pDevice->addr)))
		{
			if(pDevice->v6_expires > 0)
				strncpy(addr, pDevice->addr6, sizeof(addr) -1);
			else if(pDevice->v4_expires > 0)
				strncpy(addr, pDevice->addr, sizeof(addr) -1);

			pthread_mutex_unlock(&device_list_mutex);
			while(1)
			{
				clientctx_uninit(pCtx);
				pCtx = NULL;
				if(strchr(addr, ':'))
					sprintf(uri, "https://[%s]:2241/HNAP1/", addr);
				else
					sprintf(uri, "https://%s:2241/HNAP1/", addr);
				pCtx = clientctx_init(uri, WECB_USER, WECB_PW);

				if(pCtx == NULL)
				{
					log_printf(LOG_ERR, "init client context failed\n");
					sleep(1);
					continue;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			pthread_mutex_unlock(&device_list_mutex);
		}

		//find empty entry for this device
		if(index == -1)
		{
			pthread_mutex_lock(&index_mutex);
			if(entry_index)
			{
				for(index = -1, j = 0, i = 1; i <= 0xFF; i *= 2, j++)
				{
					if (entry_index & i)
					{
						index = j;	
						entry_index ^= i;
						ext_count++;
						break;
					}
				}
			}
			pthread_mutex_unlock(&index_mutex);

			if(index != -1)
				memset(&wecb_status[index], 0, sizeof(wecb_status[index]));
		}

		if(index != -1)
		{
			if(mask)
			{
				if(mask == 0xFFFFFFFF)
					wecb_status[index].status = INIT;
				else
					wecb_status[index].status = SYNCING;
			}

			memcpy(wecb_status[index].ip, addr, sizeof(wecb_status[index].ip));	
                        wecb_status[index].is_v2 = v2_cap;
		}
		hnap_retry = 0;

		while(hnap_retry < MAX_RETRY)	
		{
				
			if(IsDeviceReady(pCtx, NULL) == false)
			//if(GetDeviceSettings(pCtx, NULL) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "Device:%s is not ready\n", uri);
					sleep(SYNC_INTERVAL);
					break;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}
		//GetDeviceSettings, GetExtenderStatus, GetClientInfo shouldn't block auto-sync process
		while(hnap_retry < MAX_RETRY)	
		{
				
			if(GetDeviceSettings(pCtx, index == -1 ? NULL : &wecb_status[index]) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "GetDeviceSettings %s failed\n", uri);
					sleep(SYNC_INTERVAL);
					break;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}
		
		hnap_retry = 0;
		while(hnap_retry < MAX_RETRY)	
		{
				
			if(GetClientInfo(pCtx, index == -1 ? NULL : &wecb_status[index], v2_cap) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
                                        log_printf(LOG_ERR, "GetClientInfo %s failed\n", uri);
					sleep(SYNC_INTERVAL);
					break;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}

		hnap_retry = 0;
		while(hnap_retry < MAX_RETRY)	
		{
				
			if(GetExtenderStatus(pCtx, index == -1 ? NULL : &wecb_status[index]) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
					log_printf(LOG_ERR, "GetExtenderStatus %s failed\n", uri);
					sleep(SYNC_INTERVAL);
					break;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}
		
		//hnap_retry = 0;

		//check if this client is forced disconnected by web page
		radio_down = force_radio_down(addr);
		if(index != -1)
		{
			int native_radio_staus = 0;

			//check if USG shutdown radio
			if (hnap_retry < MAX_RETRY && !get_native_radio_status(&native_radio_staus))
			{
				if(radio_down == 1)
				{
					//when disconnect WECB, no need to check USG radio state 
					if(wecb_status[index].radio_mask)
					{
                  log_printf(LOG_WARNING, "Bring down %s radio\n", addr);
						mask |= RADIO_SET;
					}
				}
				else
				{
					//when bring up, take care USG native status
					if(wecb_status[index].radio_mask != native_radio_staus)
					{
						mask |= RADIO_SET;
                  log_printf(LOG_WARNING, "Bring up %s radio\n", addr);
					}
				}
			}
		}

		if(mask == 0)
		{
			log_printf(LOG_INFO, "not settings changed, sleep");
			sleep(SYNC_INTERVAL);
			continue;
		}
		hnap_retry = 0;
		if((mask == 0xFFFFFFFF) || (mask & WPS_SET))
		{
			while(hnap_retry < MAX_RETRY)	
			{

				if(SetWPS(pCtx, NULL) == false)
				{
					hnap_retry++;
					if(hnap_retry >= MAX_RETRY)
					{
                  log_printf(LOG_ERR, "Device:%s SetWPS failed\n", uri);
						sleep(SYNC_INTERVAL);
						continue;
					}
					else
					{
						sleep(1);
					}
				}
				else
				{
					hnap_retry = 0;
					//mask ^= WPS_SET;
					break;
				}
			}

		}
		
		//hnap_retry = 0;
		if((mask == 0xFFFFFFFF) || (mask & RADIO_SET))
		{
			while(hnap_retry < MAX_RETRY)	
			{

				if(SetRadios(pCtx, &radio_down) == false)
				{
					hnap_retry++;
					if(hnap_retry >= MAX_RETRY)
					{
                  log_printf(LOG_ERR, "Device:%s SetRadios failed\n", uri);
						sleep(SYNC_INTERVAL);
						continue;
					}
					else
					{
						sleep(1);
					}
				}
				else
				{
					hnap_retry = 0;
					//mask ^= RADIO_SET;
					break;
				}
			}
		}
		//hnap_retry = 0;

		if((mask == 0xFFFFFFFF) || (mask & SSID_SET))
		{
			while(hnap_retry < MAX_RETRY)	
			{

				if(SetSSIDSettings(pCtx, device_id) == false)
				{
					hnap_retry++;
					if(hnap_retry >= MAX_RETRY)
					{
                  log_printf(LOG_ERR, "Device:%s SetSSIDSettings failed\n", uri);
						sleep(SYNC_INTERVAL);
						continue;
					}
					else
					{
						sleep(1);
					}
				}
				else
				{
					hnap_retry = 0;
					//mask ^= SSID_SET;
					break;
				}
			}
		}
		//hnap_retry = 0;

		if((mask == 0xFFFFFFFF) || (mask & TIME_SET))
		{
			while(hnap_retry < MAX_RETRY)	
			{

				if(SetTOD(pCtx, NULL) == false)
				{
					hnap_retry++;
					if(hnap_retry >= MAX_RETRY)
					{
                  log_printf(LOG_ERR, "Device:%s SetTOD failed\n", uri);
						sleep(SYNC_INTERVAL);
						continue;
					}
					else
					{
						sleep(1);
					}
				}
				else
				{
					hnap_retry = 0;
					//mask ^= TIME_SET;
					break;
				}
			}
		}
		//hnap_retry = 0;

		//after SetDoRestart call, wecb will restart upnp daemon and send out ssdp:byebye
		//using this to tell upnp agent ignore the notification
		pthread_mutex_lock(&device_list_mutex);
		pDevice = Hnap_FindDeviceByUUID(device_id);
		if(pDevice != NULL && pDevice->thread && pthread_equal(pDevice->thread, self))
		{
			pDevice->restart_pending = true;
		}
		else
		{
         log_printf(LOG_WARNING, "New thread started, exit myself: %s\n", addr);
			pthread_mutex_unlock(&device_list_mutex);
			goto EXIT;
		}
		pthread_mutex_unlock(&device_list_mutex);

		while(hnap_retry < MAX_RETRY)	
		{

			if(SetDoRestart(pCtx, NULL) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
               log_printf(LOG_ERR, "Device:%s SetDoRestart failed\n", uri);
					sleep(SYNC_INTERVAL);
					continue;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				hnap_retry = 0;
				break;
			}
		}
		//hnap_retry = 0;
		//After SetDoRestart, WECB will apply settings and restart internal modules, this is time consuming
		//First just try wait 15s at this position
		sleep(15);
		
		while(hnap_retry < MAX_RETRY)	
		{

			if(IsDeviceReady(pCtx, NULL) == false)
			{
				hnap_retry++;
				if(hnap_retry >= MAX_RETRY)
				{
               log_printf(LOG_ERR, "Device:%s is not ready\n", uri);
					sleep(SYNC_INTERVAL);
					hnap_retry = 0;
					break;
				}
				else
				{
					sleep(1);
				}
			}
			else
			{
				pthread_mutex_lock(&device_list_mutex);
				pDevice = Hnap_FindDeviceByUUID(device_id);
				if(pDevice != NULL && pDevice->thread && pthread_equal(pDevice->thread, self))
				{
					pDevice->sync_mask = 0;
					pDevice->restart_pending = false;
               log_printf(LOG_WARNING, "Device:%s sync success\n", uri);
				}
				else
				{
					pthread_mutex_unlock(&device_list_mutex);
               log_printf(LOG_WARNING, "New thread started, exit myself: %s\n", addr);
					goto EXIT;
				}
				pthread_mutex_unlock(&device_list_mutex);
				wecb_status[index].status = SYNCED;
				hnap_retry = 0;
				sleep(SYNC_INTERVAL);
				break;
			}
		}
	}


EXIT:
	//release the entry
	pthread_mutex_lock(&index_mutex);
	if(index != -1)
	{
		entry_index |= 1 << index;
      log_printf(LOG_WARNING, "entry index is 0x%x after %d offline\n", entry_index, index);
	}
	ext_count--;
	pthread_mutex_unlock(&index_mutex);

	if(pCtx != NULL)
		clientctx_uninit(pCtx);
	return true;
}



void wecb_global_init()
{
	int status, pvid;
	Hnap_InitDeviceList();
	pthread_mutex_init(&index_mutex, NULL);
	//max 8 entries supported
	entry_index = 0xFF;
	char buf[512];

	while(mbus_init() == false)
	{
		log_printf(LOG_ERR, "why mbus can initialize\n");
		sleep(1);
	}
	
	syscfg_init();
	sem_init(&sem, 0, 0);
	/*adding MoCA port to support Multi-LAN
	pvid = get_primary_lan_pvid();
	while(pvid == -1)
	{
		log_printf(LOG_ERR, "can't primary bridge vlan id\n");
		sleep(2);
		pvid = get_primary_lan_pvid();
	}

	
	sprintf(buf, "mocactl -c 8 -e 1 -v %d", pvid);
	printf("moca command is %s\n", buf);
	while(1)
	{
		status = system(buf);
		
		if(status >= 0 && WIFEXITED(status))
		{
			break;
		}
		log_printf(LOG_ERR, "Configuing MoCA multi-vlan support failed\n");
		sleep(2);
	}
	*/
	while(1)
	{
		if(set_moca_bridge() != -1)
		{
			break;
		}
		log_printf(LOG_ERR, "Configuing MoCA into xhs-bridge failed\n");
		sleep(2);
	}

	/*
	while (get_bridge_factory_state() == -1)
	{
		log_printf(LOG_ERR, "get bridge factory state failed\n");
		sleep(1);
	}
	*/
#ifdef CONFIG_CISCO_HOTSPOT
	while(disable_all_wecb_bridge() == -1)
	{
		log_printf(LOG_ERR, "Disable all WECB dedicated bridge failed\n");
		sleep(1);
	}
#endif

	while(!HDK_Client_Http_Init())
	{
		log_printf(LOG_ERR, "init http client error\n");
		sleep(1);
	}

	while(pthread_attr_init(&wecb_attr))
	{
		log_printf(LOG_ERR, "init wecb pthread attribute failed\n");
	}

	while(pthread_attr_setstacksize(&wecb_attr, WECB_STACK_SIZE))
	{
		log_printf(LOG_ERR, "set pthread stack size attribute failed\n");
		sleep(1);
	}

	while(pthread_attr_setdetachstate(&wecb_attr,PTHREAD_CREATE_DETACHED))
	{
		log_printf(LOG_ERR, "set pthread detached attribute failed\n");
		sleep(1);
	}

	//set up locking and id function to openssl to make it thread safe
	CRYPTO_thread_setup();
	xmlInitParser();
}

void wecb_global_uninit()
{
   int l;

	//closelog();
#ifdef CONFIG_CISCO_HOTSPOT
	if(disable_all_wecb_bridge() == -1)
	{
		log_printf(LOG_ERR, "Disable all WECB dedicated bridge failed\n");
	}
#endif
	pthread_attr_destroy(&wecb_attr);
	mbus_uninit();
	HDK_Client_Http_Cleanup();
	xmlCleanupParser();
	sem_destroy(&sem);  
   CRYPTO_set_locking_callback(NULL);
   for (l=0; l<CRYPTO_num_locks(); l++)
   {
      pthread_mutex_destroy(&(lock_cs[l]));
   }

   free(lock_cs);
   free(lock_count);
}

void wecb_sync_sys_time()
{
	while(1)
	{
		sleep(TIME_SYNC_INTERVAL);
		wecb_sync_with_pam(TIME_SET);
		log_printf(LOG_WARNING, "need re-sync wecb device time\n");
	}
}

void wecb_signal()
{
	sem_wait(&sem);
	printf("************* APS ************* \n");
	printf("************* %d  ************* \n", recv_signal);
	printf("************* APS ************* \n");
	log_printf(LOG_ERR, "************* APS ************* \n");
	log_printf(LOG_ERR, "************* %d  ************* \n", recv_signal);
	log_printf(LOG_ERR, "************* APS ************* \n");
	exit(1);
}

