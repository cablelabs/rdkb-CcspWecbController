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

    module: cosa_nat_apis.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/


#ifndef  _COSA_WIFI_APIS_H
#define  _COSA_WIFI_APIS_H

#include "cosa_apis.h"

#define  COSA_DML_WIFI_MAX_MAC_FILTER_NUM           50

#define MAX_BUF 256

#define COSA_DML_WEP_KEY_NUM                        4

#define SYSCFG_MOCA_EXT_RADIO_INS_NUM			"MoCAExtRadioInstanceNumber"
#define SYSCFG_MOCA_EXT_RADIO_RADIOID			"MoCAExtRadioID"
#define SYSCFG_MOCA_EXT_RADIO_ENABLE			"MoCAExtRadioEnable"
#define SYSCFG_MOCA_EXT_RADIO_MODE				"MoCAExtRadioMode"
#define SYSCFG_MOCA_EXT_RADIO_CHANNEL_WD		"MoCAExtRadioChannelWidth"
#define SYSCFG_MOCA_EXT_RADIO_CHANNEL			"MoCAExtRadioChannel"
#define SYSCFG_MOCA_EXT_RADIO_SEC_CHANNEL		"MoCAExtRadioSecChannel"
#define SYSCFG_MOCA_EXT_RADIO_BEACON_INV		"MoCAExtRadioBeaconInterval"
#define SYSCFG_MOCA_EXT_RADIO_DTIM_INV			"MoCAExtRadioDTIMInterval"
#define SYSCFG_MOCA_EXT_RADIO_GUARD_INV			"MoCAExtRadioGuardInterval"
#define SYSCFG_MOCA_EXT_RADIO_COEXIT			"MoCAExtRadioCoexistance"


#define SYSCFG_MOCA_EXT_SSID_NUM				"MoCAExtSsidNum"
#define SYSCFG_MOCA_EXT_SSID_INS_NUM			"MoCAExtSsidInstanceNumber"
#define SYSCFG_MOCA_EXT_SSID_VALID				"MoCAExtSsidValid"
#define SYSCFG_MOCA_EXT_SSID_ALIAS				"MoCAExtSsidAlias"
#define SYSCFG_MOCA_EXT_SSID_RADIOID			"MoCAExtSsidRadioID"
#define SYSCFG_MOCA_EXT_SSID_INDEX				"MoCAExtSsidIndex"
#define SYSCFG_MOCA_EXT_SSID_NAME				"MoCAExtSsidName"
#define SYSCFG_MOCA_EXT_SSID_ENABLE				"MoCAExtSsidEnable"
#define SYSCFG_MOCA_EXT_SSID_BROADCAST			"MoCAExtSsidBroadcast"
#define SYSCFG_MOCA_EXT_SSID_VLANID				"MoCAExtSsidVlanID"
#define SYSCFG_MOCA_EXT_SSID_LANBASE			"MoCAExtSsidLanBase"
#define SYSCFG_MOCA_EXT_SSID_ENCRYPTION			"MoCAExtSsidEncryption"
#define SYSCFG_MOCA_EXT_SSID_QOS				"MoCAExtSsidQos"
#define SYSCFG_MOCA_EXT_SSID_MAXCLIENTS			"MoCAExtSsidMaxClients"

#define SYSCFG_MOCA_EXT_SSID_ENCRY				"MoCAExtSsidEncryption"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_MODE			"MoCAExtSsidEncryptionMode"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_TYPE			"MoCAExtSsidEncryptionType"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_WEPKEY		"MoCAExtSsidEncryptionWepKey"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_PREKEY		"MoCAExtSsidEncryptionPreSharedKey"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_PASSPHASE	"MoCAExtSsidEncryptionPassPhase"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_INV			"MoCAExtSsidEncryptionKeyInterval"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_RSIP			"MoCAExtSsidEncryptionRadiusIP"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_RSPORT		"MoCAExtSsidEncryptionRadiusPort"
#define SYSCFG_MOCA_EXT_SSID_ENCRY_RSSEC		"MoCAExtSsidEncryptionRadiusSecret"


#define SYSCFG_MOCA_EXT_SSID_QOS				"MoCAExtSsidQos"
#define SYSCFG_MOCA_EXT_SSID_QOS_WMM			"MoCAExtSsidQosWMMEnable"
#define SYSCFG_MOCA_EXT_SSID_QOS_UAPSD			"MoCAExtSsidQosUAPSDEnable"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING		"MoCAExtSsidQosSettings"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_INS_NUM		"MoCAExtSsidQosSettings%dInstanceNumber"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_SSIDINS_NUM	"MoCAExtSsidQosSettings%dSsidInstanceNumber"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AC		"MoCAExtSsidQosSettings%dAC"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_ACM	"MoCAExtSsidQosSettings%dACM"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_AIFSN	"MoCAExtSsidQosSettings%dAIFSN"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMIN	"MoCAExtSsidQosSettings%dCWMin"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_CWMAX	"MoCAExtSsidQosSettings%dCWMax"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_TXOP	"MoCAExtSsidQosSettings%dTXOPLimit"
#define SYSCFG_MOCA_EXT_SSID_QOS_SETTING_NOACK	"MoCAExtSsidQosSettings%dNOACK"


#define SYSCFG_MOCA_EXT_SSID_WPS				"MoCAExtSsidWps"
#define SYSCFG_MOCA_EXT_SSID_WPS_ENABLE			"MoCAExtSsidWpsEnabled"
#define SYSCFG_MOCA_EXT_SSID_WPS_PIN			"MoCAExtSsidWpsPinCode"
#define SYSCFG_MOCA_EXT_SSID_WPS_SSID_INDEX		"MoCAExtSsidWpsSsidIndex"

#define SYSCFG_MOCA_EXT_WIFI_DISCONNECT_CLIENTS "MoCAExtWiFiDisconnectClients"

typedef  enum
_COSA_DML_WIFI_FREQ_BAND
{
    COSA_DML_WIFI_FREQ_BAND_2_4G        = 1,
    COSA_DML_WIFI_FREQ_BAND_5G          = 2
}
COSA_DML_WIFI_FREQ_BAND, *PCOSA_DML_WIFI_FREQ_BAND;


typedef  enum
_COSA_DML_WIFI_STD
{
    COSA_DML_WIFI_STD_a             = 1,
    COSA_DML_WIFI_STD_b             = 2,
    COSA_DML_WIFI_STD_g             = 4,
    COSA_DML_WIFI_STD_n             = 8
}
COSA_DML_WIFI_STD, *PCOSA_DML_WIFI_STD;


typedef  enum
_COSA_DML_WIFI_CHAN_BW
{
    COSA_DML_WIFI_CHAN_BW_20M           = 1,
    COSA_DML_WIFI_CHAN_BW_40M
}
COSA_DML_WIFI_CHAN_BW, *PCOSA_DML_WIFI_CHAN_BW;


typedef  enum
_COSA_DML_WIFI_EXT_CHAN
{
    COSA_DML_WIFI_EXT_CHAN_Above        = 1,
    COSA_DML_WIFI_EXT_CHAN_Below,
    COSA_DML_WIFI_EXT_CHAN_Auto
}
COSA_DML_WIFI_EXT_CHAN, *PCOSA_DML_WIFI_EXT_CHAN;


typedef  enum
_COSA_DML_WIFI_GUARD_INTVL
{
    COSA_DML_WIFI_GUARD_INTVL_400ns     = 1,
    COSA_DML_WIFI_GUARD_INTVL_800ns,
    COSA_DML_WIFI_GUARD_INTVL_Auto
}
COSA_DML_WIFI_GUARD_INTVL, *PCOSA_DML_WIFI_GUARD_INTVL;

typedef  enum
_COSA_DML_WIFI_BASICRATE
{
    COSA_DML_WIFI_BASICRATE_Default      = 1,
    COSA_DML_WIFI_BASICRATE_1_2Mbps,
    COSA_DML_WIFI_BASICRATE_All
}
COSA_DML_WIFI_BASICRATE, *PCOSA_DML_WIFI_BASICRATE;

typedef  enum
_COSA_DML_WIFI_TXRATE
{
    COSA_DML_WIFI_TXRATE_Auto               = 1,
    COSA_DML_WIFI_TXRATE_6M,
    COSA_DML_WIFI_TXRATE_9M,
    COSA_DML_WIFI_TXRATE_12M,
    COSA_DML_WIFI_TXRATE_18M,
    COSA_DML_WIFI_TXRATE_24M,
    COSA_DML_WIFI_TXRATE_36M,
    COSA_DML_WIFI_TXRATE_48M,
    COSA_DML_WIFI_TXRATE_54M
}
COSA_DML_WIFI_TXRATE, *PCOSA_DML_WIFI_TXRATE;


typedef  enum
_COSA_DML_WIFI_AP_STATUS
{
    COSA_DML_WIFI_AP_STATUS_Disabled    = 1,
    COSA_DML_WIFI_AP_STATUS_Enabled,
    COSA_DML_WIFI_AP_STATUS_Error_Misconfigured,
    COSA_DML_WIFI_AP_STATUS_Error
}
COSA_DML_WIFI_AP_STATUS, *PCOSA_DML_WIFI_AP_STATUS;


typedef  enum
_COSA_DML_WIFI_SECURITY
{
    COSA_DML_WIFI_SECURITY_None                 = 0x00000001,
    COSA_DML_WIFI_SECURITY_WEP_64               = 0x00000002,
    COSA_DML_WIFI_SECURITY_WEP_128              = 0x00000004,
    COSA_DML_WIFI_SECURITY_WPA_Personal         = 0x00000008,
    COSA_DML_WIFI_SECURITY_WPA2_Personal        = 0x00000010,
    COSA_DML_WIFI_SECURITY_WPA_WPA2_Personal    = 0x00000020,
    COSA_DML_WIFI_SECURITY_WPA_Enterprise       = 0x00000040,
    COSA_DML_WIFI_SECURITY_WPA2_Enterprise      = 0x00000080,
    COSA_DML_WIFI_SECURITY_WPA_WPA2_Enterprise  = 0x00000100
}
COSA_DML_WIFI_SECURITY, *PCOSA_DML_WIFI_SECURITY;

typedef  enum
_COSA_DML_WIFI_AP_SEC_ENCRYPTION
{
    COSA_DML_WIFI_AP_SEC_TKIP    = 1,
    COSA_DML_WIFI_AP_SEC_AES,
    COSA_DML_WIFI_AP_SEC_AES_TKIP
}
COSA_DML_WIFI_AP_SEC_ENCRYPTION, *PCOSA_DML_WIFI_AP_SEC_ENCRYPTION;

typedef  enum
_COSA_DML_WIFI_WPS_METHOD
{
    COSA_DML_WIFI_WPS_METHOD_UsbFlashDrive      = 0x00000001,
    COSA_DML_WIFI_WPS_METHOD_Ethernet           = 0x00000002,
    COSA_DML_WIFI_WPS_METHOD_ExternalNFCToken   = 0x00000004,
    COSA_DML_WIFI_WPS_METHOD_IntgratedNFCToken  = 0x00000008,
    COSA_DML_WIFI_WPS_METHOD_NFCInterface       = 0x00000010,
    COSA_DML_WIFI_WPS_METHOD_PushButton         = 0x00000020,
    COSA_DML_WIFI_WPS_METHOD_Pin                = 0x00000040
}
COSA_DML_WIFI_WPS_METHOD, *PCOSA_DML_WIFI_WPS_METHOD;

typedef enum
_COSA_DML_WIFI_AP_SEC_WEPMODE
{
    COSA_DML_WIFI_WEPMODE_WEP64 = 1,
    COSA_DML_WIFI_WEPMODE_WEP128,
}
COSA_DML_WIFI_AP_SEC_WEPMODE, *PCOSA_DML_WIFI_AP_SEC_WEPMODE;

/*
 *  Structure definitions for WiFi Radio
 */
struct 
_COSA_DML_WIFI_RADIO_CFG
{
    ULONG                           InstanceNumber;
    char                            Alias[COSA_DML_ALIAS_NAME_LENGTH];
    BOOLEAN                         bCoexistance;
    BOOLEAN                         bEnabled;
    COSA_DML_WIFI_FREQ_BAND         OperatingFrequencyBand;
    ULONG                           OperatingStandards;         /* Bitmask of COSA_DML_WIFI_STD */
    ULONG                           Channel;
    BOOLEAN                         AutoChannelEnable;
    ULONG                           AutoChannelRefreshPeriod;
    COSA_DML_WIFI_CHAN_BW           OperatingChannelBandwidth;
    COSA_DML_WIFI_EXT_CHAN          ExtensionChannel;
    COSA_DML_WIFI_GUARD_INTVL       GuardInterval;
    int                             MCS;
    int                             TransmitPower;
    BOOLEAN                         IEEE80211hEnabled;
    char                            RegulatoryDomain[4];
    /* Below is Cisco Extensions */
    COSA_DML_WIFI_BASICRATE         BasicRate;
    COSA_DML_WIFI_TXRATE            TxRate;
    BOOLEAN                         APIsolation;
    BOOLEAN                         FrameBurst;
    BOOLEAN                         CTSProtectionMode;
    ULONG                           BeaconInterval;
    ULONG                           DTIMInterval;
    ULONG                           FragmentationThreshold;
    ULONG                           RTSThreshold;
    /* USGv2 Extensions */
    ULONG                           LongRetryLimit;
    int                             MbssUserControl;
    int                             AdminControl;
    int                             OnOffPushButtonTime;
    int                             ObssCoex;
    int                             MulticastRate;
    BOOL                            ApplySetting;
    BOOL                            X_CISCO_COM_ReverseDirectionGrant;
    BOOL                            X_CISCO_COM_AggregationMSDU;
    BOOL                            X_CISCO_COM_AutoBlockAck;
    BOOL                            X_CISCO_COM_DeclineBARequest;
    ULONG                           X_CISCO_COM_HTTxStream;
    ULONG                           X_CISCO_COM_HTRxStream;
    BOOL                            X_CISCO_COM_STBCEnable;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_RADIO_CFG COSA_DML_WIFI_RADIO_CFG,  *PCOSA_DML_WIFI_RADIO_CFG;

/*
 *  Static portion of WiFi radio info
 */
struct
_COSA_DML_WIFI_RADIO_SINFO
{
    char                            Name[COSA_DML_ALIAS_NAME_LENGTH];
    BOOLEAN                         bUpstream;
    ULONG                           MaxBitRate;
    ULONG                           SupportedFrequencyBands;    /* Bitmask of COSA_DML_WIFI_FREQ_BAND */
    ULONG                           SupportedStandards;         /* Bitmask of COSA_DML_WIFI_STD */
    char                            PossibleChannels[512];
    BOOLEAN                         AutoChannelSupported;
    char                            TransmitPowerSupported[64];
    BOOLEAN                         IEEE80211hSupported;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_RADIO_SINFO COSA_DML_WIFI_RADIO_SINFO,  *PCOSA_DML_WIFI_RADIO_SINFO;


/*
 *  Dynamic portion of WiFi radio info
 */
struct
_COSA_DML_WIFI_RADIO_DINFO
{
    COSA_DML_IF_STATUS              Status;
    ULONG                           LastChange;
    char                            ChannelsInUse[512];
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_RADIO_DINFO COSA_DML_WIFI_RADIO_DINFO,  *PCOSA_DML_WIFI_RADIO_DINFO;

struct
_COSA_DML_WIFI_RADIO_FULL
{
    COSA_DML_WIFI_RADIO_CFG         Cfg;
    COSA_DML_WIFI_RADIO_SINFO       StaticInfo;
    COSA_DML_WIFI_RADIO_DINFO       DynamicInfo;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_RADIO_FULL COSA_DML_WIFI_RADIO_FULL, *PCOSA_DML_WIFI_RADIO_FULL;

struct
_COSA_DML_WIFI_RADIO_STATS
{
    ULONG                           BytesSent;
    ULONG                           BytesReceived;
    ULONG                           PacketsSent;
    ULONG                           PacketsReceived;
    ULONG                           ErrorsSent;
    ULONG                           ErrorsReceived;
    ULONG                           DiscardPacketsSent;
    ULONG                           DiscardPacketsReceived;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_RADIO_STATS COSA_DML_WIFI_RADIO_STATS, *PCOSA_DML_WIFI_RADIO_STATS;



/*
 *  Structure definitions for WiFi SSID
 */
struct
_COSA_DML_WIFI_SSID_EncryptionInfo
{
    ULONG                           ModeEnabled;
    ULONG                           Encryption;
    char                            WepKey[128];
    char                            PreSharedKey[128];
    char                            Passphrase[128];
    ULONG                           RekeyInterval;
    ANSC_IPV4_ADDRESS               RadiusServerIP;
    ULONG                           RadiusServerPort;
    char                            RadiusSecret[128];
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_EncryptionInfo COSA_DML_WIFI_SSID_EncryptionInfo, *PCOSA_DML_WIFI_SSID_EncryptionInfo;

struct
_COSA_DML_WIFI_SSID_QosSetting
{
    ULONG                           InstanceNumber;
    ULONG                           SSIDInstanceNumber;
    ULONG                           AC;
    BOOLEAN                         ACM;
    ULONG                           AIFSN;
    ULONG                           CWMin;
    ULONG                           CWMax;
    ULONG                           TXOPLimit;
    BOOLEAN                         NoACK;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_QosSetting COSA_DML_WIFI_SSID_QosSetting, *PCOSA_DML_WIFI_SSID_QosSetting;

struct
_COSA_DML_WIFI_SSID_QosInfo 
{
    BOOLEAN                         WMMEnable;
    BOOLEAN                         UAPSDEnable;
    COSA_DML_WIFI_SSID_QosSetting   QosSetting[4];
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_QosInfo COSA_DML_WIFI_SSID_QosInfo, *PCOSA_DML_WIFI_SSID_QosInfo;

struct
_COSA_DML_WIFI_WPS
{
    BOOLEAN                         bEnabled;
    char                            X_CISCO_COM_Pin[64];
    char                            SSIDIndex[17];
}_struct_pack_;

typedef struct _COSA_DML_WIFI_WPS COSA_DML_WIFI_WPS,  *PCOSA_DML_WIFI_WPS;

struct
_COSA_DML_WIFI_SSID_CFG
{
    ULONG                           InstanceNumber;
    char                            Alias[COSA_DML_ALIAS_NAME_LENGTH];

    BOOLEAN                         bEnabled;
    char                            WiFiRadioName[COSA_DML_ALIAS_NAME_LENGTH]; /* Points to the underlying WiFi Radio */
    char                            SSID[32];
    char                            SSIDIndex[17];
    BOOLEAN                         bSSIDBroadcast;
    ULONG                           SSIDVlanID;
    ANSC_IPV4_ADDRESS               SSIDLanBase;
    ULONG                           MaxClients;
    COSA_DML_WIFI_SSID_EncryptionInfo EncryptionInfo;
    COSA_DML_WIFI_SSID_QosInfo      QosInfo;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_CFG COSA_DML_WIFI_SSID_CFG,  *PCOSA_DML_WIFI_SSID_CFG;


/*
 *  Static portion of WiFi SSID info
 */
struct
_COSA_DML_WIFI_SSID_SINFO
{
    char                            Name[COSA_DML_ALIAS_NAME_LENGTH];
    UCHAR                           BSSID[6];
    UCHAR                           MacAddress[6];
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_SINFO COSA_DML_WIFI_SSID_SINFO,  *PCOSA_DML_WIFI_SSID_SINFO;


/*
 *  Dynamic portion of WiFi SSID info
 */
struct
_COSA_DML_WIFI_SSID_DINFO
{
    COSA_DML_IF_STATUS              Status;
    ULONG                           LastChange;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_SSID_DINFO COSA_DML_WIFI_SSID_DINFO,  *PCOSA_DML_WIFI_SSID_DINFO;

struct
_COSA_DML_WIFI_SSID_FULL
{
    COSA_DML_WIFI_SSID_CFG          Cfg;
    COSA_DML_WIFI_SSID_SINFO        StaticInfo;
    COSA_DML_WIFI_SSID_DINFO        DynamicInfo;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_SSID_FULL COSA_DML_WIFI_SSID_FULL, *PCOSA_DML_WIFI_SSID_FULL;

struct
_COSA_DML_WIFI_SSID_STATS
{
    ULONG                           BytesSent;
    ULONG                           BytesReceived;
    ULONG                           PacketsSent;
    ULONG                           PacketsReceived;
    ULONG                           ErrorsSent;
    ULONG                           ErrorsReceived;
    ULONG                           UnicastPacketsSent;
    ULONG                           UnicastPacketsReceived;
    ULONG                           DiscardPacketsSent;
    ULONG                           DiscardPacketsReceived;
    ULONG                           MulticastPacketsSent;
    ULONG                           MulticastPacketsReceived;
    ULONG                           BroadcastPacketsSent;
    ULONG                           BroadcastPacketsReceived;
    ULONG                           UnknownProtoPacketsReceived;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_SSID_STATS COSA_DML_WIFI_SSID_STATS, *PCOSA_DML_WIFI_SSID_STATS;

/*
 *  Structure definitions for WiFi AP
 *  WiFi AP is always associated with a SSID in the system, thus,
 *  it is identified by the SSID.
 *
 *  Middle layer handles orphan AP (not associated with a SSID) internally.
 *  Those APs are not passed down to backend system.
 */
struct
_COSA_DML_WIFI_AP_CFG
{
    ULONG                           InstanceNumber;
    char                            Alias[COSA_DML_ALIAS_NAME_LENGTH];
    char                            SSID[32];           /* Reference to SSID name */

    BOOLEAN                         bEnabled;
    BOOLEAN                         SSIDAdvertisementEnabled;
    ULONG                           RetryLimit;
    BOOLEAN                         WMMEnable;
    BOOLEAN                         UAPSDEnable;

    /* USGv2 Extensions */ 
    int                             WmmNoAck;
    int                             MulticastRate;
    int                             BssMaxNumSta;
    BOOL                            BssCountStaAsCpe;
    BOOL                            BssHotSpot;
    ULONG                           LongRetryLimit;
    BOOLEAN                         KickAssocDevices;

}_struct_pack_;

typedef struct _COSA_DML_WIFI_AP_CFG COSA_DML_WIFI_AP_CFG,  *PCOSA_DML_WIFI_AP_CFG;

struct
_COSA_DML_WIFI_AP_INFO
{
    COSA_DML_WIFI_AP_STATUS         Status;
    BOOLEAN                         WMMCapability;
    BOOLEAN                         UAPSDCapability;
    /* USGv2 Extensions */ 
    int                             BssUserStatus;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_AP_INFO COSA_DML_WIFI_AP_INFO,  *PCOSA_DML_WIFI_AP_INFO;

struct
_COSA_DML_WIFI_AP_FULL
{
    COSA_DML_WIFI_AP_CFG            Cfg;
    COSA_DML_WIFI_AP_INFO           Info;

    /* USGv2 Extensions */ 
    QUEUE_HEADER                    MacFilterList;
    ULONG                           ulMacFilterNextInsNum;
    ANSC_HANDLE                     hIrepFolderMacFilt;
    ANSC_HANDLE                     hPoamMacFlitDm;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_AP_FULL COSA_DML_WIFI_AP_FULL, *PCOSA_DML_WIFI_AP_FULL;


/*
 *  Structure definitions for WiFi AP Security
 */
struct
_COSA_DML_WIFI_APSEC_CFG
{
    COSA_DML_WIFI_SECURITY          ModeEnabled;
    UCHAR                           WEPKeyp[13];
    UCHAR                           PreSharedKey[64+1];
    UCHAR                           KeyPassphrase[64+1];
    ULONG                           RekeyingInterval;
    COSA_DML_WIFI_AP_SEC_ENCRYPTION EncryptionMethod;
    ANSC_IPV4_ADDRESS               RadiusServerIPAddr;
    ULONG                           RadiusServerPort;
    char                            RadiusSecret[64];
    /* USGv2 Extensions */
    int                             RadiusReAuthInterval;
    int                             DefaultKey;
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_APSEC_CFG COSA_DML_WIFI_APSEC_CFG,  *PCOSA_DML_WIFI_APSEC_CFG;

struct
_COSA_DML_WIFI_APSEC_INFO
{
    ULONG                           ModesSupported;     /* Bitmask of COSA_DML_WIFI_SECURITY */
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_APSEC_INFO COSA_DML_WIFI_APSEC_INFO,  *PCOSA_DML_WIFI_APSEC_INFO;

typedef struct
_COSA_DML_WEPKEY_64BIT
{
    char                            WEPKey[5 * 2 + 1];
}
COSA_DML_WEPKEY_64BIT, *PCOSA_DML_WEPKEY_64BIT;

typedef struct
_COSA_DML_WEPKEY_128BIT
{
    char                            WEPKey[13 * 2 + 1];
}
COSA_DML_WEPKEY_128BIT, *PCOSA_DML_WEPKEY_128BIT;

struct
_COSA_DML_WIFI_APSEC_FULL
{
    COSA_DML_WIFI_APSEC_CFG         Cfg;
    COSA_DML_WIFI_APSEC_INFO        Info;

    /* USGv2 Extensions */
    COSA_DML_WEPKEY_64BIT           WEPKey64Bit[COSA_DML_WEP_KEY_NUM];
    COSA_DML_WEPKEY_128BIT          WEPKey128Bit[COSA_DML_WEP_KEY_NUM];
}_struct_pack_;

typedef  struct _COSA_DML_WIFI_APSEC_FULL COSA_DML_WIFI_APSEC_FULL, *PCOSA_DML_WIFI_APSEC_FULL;


/*
 *  Structure definitions for WiFi AP WPS
 */
struct
_COSA_DML_WIFI_APWPS_CFG
{
    BOOLEAN                         bEnabled;
    ULONG                           ConfigMethodsEnabled;   /* Bitmask of COSA_DML_WIFI_WPS_METHOD */
    /* USGv2 Extensions */
    int                             WpsPushButton;
    BOOLEAN                         X_CISCO_COM_ActivatePushButton;
    char                            X_CISCO_COM_ClientPin[64];
    BOOLEAN                         X_CISCO_COM_CancelSession;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_APWPS_CFG COSA_DML_WIFI_APWPS_CFG,  *PCOSA_DML_WIFI_APWPS_CFG;


struct
_COSA_DML_WIFI_APWPS_INFO
{
    ULONG                           ConfigMethodsSupported; /* Bitmask of COSA_DML_WIFI_WPS_METHOD */
    char                            X_CISCO_COM_Pin[64];
    BOOL                            X_Comcast_com_Configured;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_APWPS_INFO COSA_DML_WIFI_APWPS_INFO,  *PCOSA_DML_WIFI_APWPS_INFO;

struct
_COSA_DML_WIFI_APWPS_FULL
{
    COSA_DML_WIFI_APWPS_CFG         Cfg;
    COSA_DML_WIFI_APWPS_INFO        Info;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_APWPS_FULL COSA_DML_WIFI_APWPS_FULL, *PCOSA_DML_WIFI_APWPS_FULL;


/*
 *  Structure definitions for WiFi AP Associated Device
 */
struct
_COSA_DML_WIFI_AP_ASSOC_DEVICE
{
    UCHAR                           MacAddress[6];
    BOOLEAN                         AuthenticationState;
    ULONG                           LastDataDownlinkRate;
    ULONG                           LastDataUplinkRate;
    int                             SignalStrength;
    ULONG                           Retransmissions;
    BOOLEAN                         Active;
}_struct_pack_;

typedef struct _COSA_DML_WIFI_AP_ASSOC_DEVICE COSA_DML_WIFI_AP_ASSOC_DEVICE,  *PCOSA_DML_WIFI_AP_ASSOC_DEVICE;


/*
 * Structure definitions for WiFi AP MAC filter
 */

struct
_COSA_DML_WIFI_AP_MF_CFG
{
    BOOLEAN                         bEnabled;
    BOOLEAN                         FilterAsBlackList;
    ULONG                           NumberMacAddrList;
    UCHAR                           MacAddrList[6*COSA_DML_WIFI_MAX_MAC_FILTER_NUM];
}_struct_pack_;

typedef struct _COSA_DML_WIFI_AP_MF_CFG COSA_DML_WIFI_AP_MF_CFG, *PCOSA_DML_WIFI_AP_MF_CFG;

typedef struct 
_COSA_DML_WIFI_AP_MAC_FILTER
{
    ULONG                           InstanceNumber;
    char                            Alias[COSA_DML_ALIAS_NAME_LENGTH];

    char                            MACAddress[18];
    char                            DeviceName[64];
}
COSA_DML_WIFI_AP_MAC_FILTER, *PCOSA_DML_WIFI_AP_MAC_FILTER;

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

ANSC_STATUS
CosaDmlWiFiInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    );
    
ANSC_STATUS
CosaDmlWiFi_FactoryReset
    (
       void
    );
 
ANSC_STATUS	
CosaDmlWiFi_SetDisconnectClients
	(
	 char *p
	);


 
ANSC_STATUS	
CosaDmlWiFi_GetDisconnectClients
	(
	 char *p
	);	

/*
 *  Description:
 *     The API retrieves the number of WiFi radios in the system.
 */
ULONG
CosaDmlWiFiRadioGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    );
    
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
    );

ANSC_STATUS
CosaDmlWiFiRadioSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );

ANSC_STATUS
CosaDmlWiFiRadioSetDefaultCfgValues
    (
        ANSC_HANDLE                 hContext,
        unsigned long               ulIndex,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg
    );
    
ANSC_STATUS
CosaDmlWiFiRadioSetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg          /* Identified by InstanceNumber */
    );

ANSC_STATUS
CosaDmlWiFiRadioGetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_RADIO_CFG    pCfg          /* Identified by InstanceNumber */
    );

ANSC_STATUS
CosaDmlWiFiRadioGetDinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_RADIO_DINFO  pInfo
    );

ANSC_STATUS
CosaDmlWiFiRadioGetStats
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_RADIO_STATS  pStats
    );

/* WiFi SSID */
/* Description:
 *	The API retrieves the number of WiFi SSIDs in the system.
 */
ULONG
CosaDmlWiFiSsidGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    );

    
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
    );

ANSC_STATUS
CosaDmlWiFiSsidSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );    
    
/* Description:
 *	The API adds a new WiFi SSID into the system. 
 * Arguments:
 *	ulIndex		Indicates the index number of the entry.
 *	pEntry		Caller pass in the configuration through pEntry->Cfg field and gets back the generated pEntry->StaticInfo.Name, MACAddress, etc.
 */
ANSC_STATUS
CosaDmlWiFiSsidAddEntry
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_FULL    pEntry
    );


ANSC_STATUS
CosaDmlWiFiSsidDelEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    );

ANSC_STATUS
CosaDmlWiFiSsidSetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_CFG     pCfg         /* Identified by InstanceNumber */
    );

ANSC_STATUS
CosaDmlWiFiSsidGetCfg
    (
        ANSC_HANDLE                 hContext,
        PCOSA_DML_WIFI_SSID_CFG     pCfg         /* Identified by InstanceNumber */
    );

ANSC_STATUS
CosaDmlWiFiSsidGetDinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_DINFO   pInfo
    );

ANSC_STATUS
CosaDmlWiFiSsidGetSinfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_SINFO   pInfo
    );

ANSC_STATUS
CosaDmlWiFiSsidGetStats
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PCOSA_DML_WIFI_SSID_STATS   pStats
    );

ANSC_STATUS
CosaDmlWiFiSSIDEncryptionGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_EncryptionInfo pEncryption
);

ANSC_STATUS
CosaDmlWiFiSSIDEncryptionSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_EncryptionInfo pEncryption
);

ANSC_STATUS
CosaDmlWiFiSSIDQoSGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosInfo pQos
);

ANSC_STATUS
CosaDmlWiFiSSIDQoSSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosInfo pQos
);

ULONG
CosaDmlWiFiSSIDQosSettingGetCount
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber
);

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingGetEntry
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    ULONG                       nIndex,                  /* Identified by Index */
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting
);

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingGetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting          /* Identified by Instance Number */
);

ANSC_STATUS
CosaDmlWiFiSSIDQosSettingSetCfg
(
    ANSC_HANDLE                 hContext,
    ULONG                       SSIDInstanceNumber,
    PCOSA_DML_WIFI_SSID_QosSetting pQosSetting          /* Identified by Instance Number */
);

ANSC_STATUS
CosaDmlWiFiWPSGetCfg
(
    ANSC_HANDLE                 hContext,
    PCOSA_DML_WIFI_WPS          pWPS
);

ANSC_STATUS
CosaDmlWiFiWPSSetCfg
(
    ANSC_HANDLE                 hContext,
    PCOSA_DML_WIFI_WPS          pWPS
);
ULONG GetSsidIndexAsInstanceNumber(ULONG ins_num);

/* WiFi AP is always associated with a SSID in the system */
/* Description:
 *  The API retrieves the number of WiFi APs in the system.
 */
ULONG
CosaDmlWiFiAPGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    );
    
ANSC_STATUS
CosaDmlWiFiApGetEntry
    (
        ANSC_HANDLE                 hContext,
        char                        *pSsid,
        PCOSA_DML_WIFI_AP_FULL      pEntry
    );

ANSC_STATUS
CosaDmlWiFiApSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );
    
ANSC_STATUS
CosaDmlWiFiApSetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_AP_CFG       pCfg
    );

ANSC_STATUS
CosaDmlWiFiApGetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_AP_CFG       pCfg
    );

ANSC_STATUS
CosaDmlWiFiApGetInfo
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_AP_INFO      pInfo
    );

ANSC_STATUS
CosaDmlWiFiApSecGetEntry
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APSEC_FULL   pEntry
    );

ANSC_STATUS
CosaDmlWiFiApSecSetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APSEC_CFG    pCfg
    );

ANSC_STATUS
CosaDmlWiFiApSecGetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APSEC_CFG    pCfg
    );

ANSC_STATUS
CosaDmlWiFiApWpsGetEntry
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APWPS_FULL   pEntry
    );

ANSC_STATUS
CosaDmlWiFiApWpsSetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APWPS_CFG    pCfg
    );

ANSC_STATUS
CosaDmlWiFiApWpsGetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APWPS_CFG    pCfg
    );

ANSC_STATUS
CosaDmlWiFiApWpsGetInfo
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APWPS_INFO   pInfo
    );

ANSC_STATUS
CosaDmlWiFiApWpsSetInfo
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_APWPS_INFO    pInfo
    );

/* Description:
 *  This routine is to retrieve the complete list of currently associated WiFi devices, 
 *  which is a dynamic table.
 * Arguments:
 *  pSsid           Indicate which SSID to operate on.
 *  pulCount        To receive the actual number of entries.
 * Return:
 * The pointer to the array of WiFi associated devices, allocated by callee. 
 * If no entry is found, NULL is returned.
 */
PCOSA_DML_WIFI_AP_ASSOC_DEVICE
CosaDmlWiFiApGetAssocDevices
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PULONG                      pulCount
    );

/* Description:
 *  This routine is to retrieve the complete list of currently associated WiFi devices
 *  and kick them to force them to disassociate.
 * Arguments:
 *  pSsid           Indicate which SSID to operate on.
 * Return:
 * Status
 */
ANSC_STATUS
CosaDmlWiFiApKickAssocDevices
    (
        char*                       pSsid
    );

/*
 * WiFi AP MAC Filter
 */
ANSC_STATUS
CosaDmlWiFiApMfSetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_AP_MF_CFG    pCfg
    );

ANSC_STATUS
CosaDmlWiFiApMfGetCfg
    (
        ANSC_HANDLE                 hContext,
        char*                       pSsid,
        PCOSA_DML_WIFI_AP_MF_CFG    pCfg
    );


/*
 *  Extension functions used in unexpected way
 */
ANSC_STATUS
CosaDmlWiFi_FactoryReset
    (
       void
    );
    
ANSC_STATUS
CosaDmlWiFi_RadioUpdated();

ANSC_STATUS
CosaDmlWiFi_SSIDUpdated();


ANSC_STATUS
CosaDmlWiFi_WPSUpdated();

ANSC_STATUS	
CosaDmlWiFi_GetExtStatus(int *ext_count, ANSC_HANDLE ext_status);

#endif
