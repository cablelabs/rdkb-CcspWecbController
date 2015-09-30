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

    module: cosa_wifi_dml.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        01/18/2011    initial revision.

**************************************************************************/
#include "ctype.h"
#include "ansc_platform.h"
#include "cosa_wifi_dml.h"
#include "cosa_wifi_internal.h"
#include "plugin_main_apis.h"
#include "wecb_common.h"
#define TIME_NO_NEGATIVE(x) ((long)(x) < 0 ? 0 : (x))

/***********************************************************************
 IMPORTANT NOTE:

 According to TR69 spec:
 On successful receipt of a SetParameterValues RPC, the CPE MUST apply 
 the changes to all of the specified Parameters atomically. That is, either 
 all of the value changes are applied together, or none of the changes are 
 applied at all. In the latter case, the CPE MUST return a fault response 
 indicating the reason for the failure to apply the changes. 
 
 The CPE MUST NOT apply any of the specified changes without applying all 
 of them.

 In order to set parameter values correctly, the back-end is required to
 hold the updated values until "Validate" and "Commit" are called. Only after
 all the "Validate" passed in different objects, the "Commit" will be called.
 Otherwise, "Rollback" will be called instead.

 The sequence in COSA Data Model will be:

 SetParamBoolValue/SetParamIntValue/SetParamUlongValue/SetParamStringValue
 -- Backup the updated values;

 if( Validate_XXX())
 {
     Commit_XXX();    -- Commit the update all together in the same object
 }
 else
 {
     Rollback_XXX();  -- Remove the update at backup;
 }
 
***********************************************************************/

static int isHex (char *string);

static ANSC_STATUS
GetInsNumsByWEPKey64(PCOSA_DML_WEPKEY_64BIT pWEPKey, ULONG *apIns, ULONG *wepKeyIdx)
{
    PCOSA_DATAMODEL_WIFI        pWiFi       = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PSINGLE_LINK_ENTRY          pAPLink     = NULL;
    PCOSA_CONTEXT_LINK_OBJECT   pAPLinkObj  = NULL;
    PCOSA_DML_WIFI_AP           pWiFiAP     = NULL;
    int                         i;

    /* for each Device.WiFi.AccessPoint.{i}. */
    for (   pAPLink = AnscSListGetFirstEntry(&pWiFi->AccessPointQueue);
            pAPLink != NULL;
            pAPLink = AnscSListGetNextEntry(pAPLink)
        )
    {
        pAPLinkObj = ACCESS_COSA_CONTEXT_LINK_OBJECT(pAPLink);
        if (!pAPLinkObj)
            continue;
        pWiFiAP = (PCOSA_DML_WIFI_AP)pAPLinkObj->hContext;

        /* for each Device.WiFi.AccessPoint.{i}.Security.X_CISCO_COM_WEPKey64Bit.{i}. */
        for (i = 0; i < COSA_DML_WEP_KEY_NUM; i++)
        {
            if ((ANSC_HANDLE)pWEPKey == (ANSC_HANDLE)&pWiFiAP->SEC.WEPKey64Bit[i])
            {
                /* found */
                *apIns = pWiFiAP->AP.Cfg.InstanceNumber;
                *wepKeyIdx = i;
                return ANSC_STATUS_SUCCESS;
            }
        }
    }

    return ANSC_STATUS_FAILURE;
}

static ANSC_STATUS
GetInsNumsByWEPKey128(PCOSA_DML_WEPKEY_128BIT pWEPKey, ULONG *apIns, ULONG *wepKeyIdx)
{
    PCOSA_DATAMODEL_WIFI        pWiFi       = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PSINGLE_LINK_ENTRY          pAPLink     = NULL;
    PCOSA_CONTEXT_LINK_OBJECT   pAPLinkObj  = NULL;
    PCOSA_DML_WIFI_AP           pWiFiAP     = NULL;
    int                         i;

    /* for each Device.WiFi.AccessPoint.{i}. */
    for (   pAPLink = AnscSListGetFirstEntry(&pWiFi->AccessPointQueue);
            pAPLink != NULL;
            pAPLink = AnscSListGetNextEntry(pAPLink)
        )
    {
        pAPLinkObj = ACCESS_COSA_CONTEXT_LINK_OBJECT(pAPLink);
        if (!pAPLinkObj)
            continue;
        pWiFiAP = (PCOSA_DML_WIFI_AP)pAPLinkObj->hContext;

        /* for each Device.WiFi.AccessPoint.{i}.Security.X_CISCO_COM_WEPKey128Bit.{i}. */
        for (i = 0; i < COSA_DML_WEP_KEY_NUM; i++)
        {
            if ((ANSC_HANDLE)pWEPKey == (ANSC_HANDLE)&pWiFiAP->SEC.WEPKey128Bit[i])
            {
                /* found */
                *apIns = pWiFiAP->AP.Cfg.InstanceNumber;
                *wepKeyIdx = i;
                return ANSC_STATUS_SUCCESS;
            }
        }
    }

    return ANSC_STATUS_FAILURE;
}

/***********************************************************************

 APIs for Object:

    WiFi.

    *  WiFi_GetParamBoolValue
    *  WiFi_GetParamIntValue
    *  WiFi_GetParamUlongValue
    *  WiFi_GetParamStringValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WiFi_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WiFi_Extender_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;

    if (AnscEqualString(ParamName, "X_CISCO_COM_FactoryReset", TRUE))
    {
        /* always return false when get */
        *pBool = FALSE;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_Radio_Updated", TRUE))
    {
        /* always return false when get */
        *pBool = FALSE;
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_SSID_Updated", TRUE))
    {
        /* always return false when get */
        *pBool = FALSE;
        return TRUE;
    }

	if (AnscEqualString(ParamName, "X_CISCO_COM_WPS_Updated", TRUE))
    {
        /* always return false when get */
        *pBool = FALSE;
        return TRUE;
    }
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WiFi_Extender_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WiFi_Extender_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    /* check the parameter name and return the corresponding value */

    CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WiFi_Extender_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WiFi_Extender_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    /* check the parameter name and return the corresponding value */

    CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        WiFi_Extender_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
WiFi_Extender_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    /* check the parameter name and return the corresponding value */
	PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    
	 /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "X_CISCO_COM_DISCONNECT_CLIENT", TRUE))
    {

        if ( 130 < *pUlSize)
        {
            //AnscCopyString(pValue, pMyObject->disconnect_client);
			CosaDmlWiFi_GetDisconnectClients(pValue);
            return 0;
        }
        else
        {
            *pUlSize = 130;
            return 1;
        }
        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

BOOL
WiFi_Extender_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    
    /* check the parameter name and set the corresponding value */

	if (AnscEqualString(ParamName, "X_CISCO_COM_DISCONNECT_CLIENT", TRUE))
    {
		//At max 8 IPs, each entry is 16 char, 16*8=128
        if (AnscSizeOfString(pString) > 130)
		{
            return FALSE;
		}
	
		CosaDmlWiFi_SetDisconnectClients(pString);			
        //AnscCopyString(pMyObject->disconnect_client, pString);
        return TRUE;
    }

	return FALSE;
}	

BOOL
WiFi_Extender_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;

    if(AnscEqualString(ParamName, "X_CISCO_COM_FactoryReset", TRUE))
    {
        CosaDmlWiFi_FactoryReset();
        return TRUE;
    }

    if(AnscEqualString(ParamName, "X_CISCO_COM_Radio_Updated", TRUE))
    {
        CosaDmlWiFi_RadioUpdated();
        return TRUE;
    }

    if(AnscEqualString(ParamName, "X_CISCO_COM_SSID_Updated", TRUE))
    {
        CosaDmlWiFi_SSIDUpdated();
        return TRUE;
    }
	
	if(AnscEqualString(ParamName, "X_CISCO_COM_WPS_Updated", TRUE))
    {
        CosaDmlWiFi_WPSUpdated();
        return TRUE;
    }

    return FALSE;
}


/***********************************************************************

 APIs for Object:

    WiFi.Radio.{i}.

    *  Radio_GetEntryCount
    *  Radio_GetEntry
    *  Radio_GetParamBoolValue
    *  Radio_GetParamIntValue
    *  Radio_GetParamUlongValue
    *  Radio_GetParamStringValue
    *  Radio_SetParamBoolValue
    *  Radio_SetParamIntValue
    *  Radio_SetParamUlongValue
    *  Radio_SetParamStringValue
    *  Radio_Validate
    *  Radio_Commit
    *  Radio_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Radio_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
Radio_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    ULONG                           count         = 0;
    
    return pMyObject->RadioCount;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Radio_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
Radio_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_RADIO            pWifiRadio    = NULL;
    
    if ( pMyObject->pRadio && nIndex < pMyObject->RadioCount )
    {
        pWifiRadio = pMyObject->pRadio+nIndex;

        *pInsNumber = pWifiRadio->Radio.Cfg.InstanceNumber;

        return pWifiRadio;
    }
    
    return NULL; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Coexistance", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.bCoexistance;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.bEnabled;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Upstream", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->StaticInfo.bUpstream;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AutoChannelSupported", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->StaticInfo.AutoChannelSupported;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AutoChannelEnable", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.AutoChannelEnable;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "IEEE80211hSupported", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->StaticInfo.IEEE80211hSupported;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "IEEE80211hEnabled", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.IEEE80211hEnabled;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_APIsolation", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.APIsolation;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_FrameBurst", TRUE))
    {
        /* collect value */
        *pBool = pWifiRadioFull->Cfg.FrameBurst;

        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_ApplySetting", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.ApplySetting; 
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_ReverseDirectionGrant", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.X_CISCO_COM_ReverseDirectionGrant; 
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_AggregationMSDU", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.X_CISCO_COM_AggregationMSDU; 
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_AutoBlockAck", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.X_CISCO_COM_AutoBlockAck; 
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_DeclineBARequest", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.X_CISCO_COM_DeclineBARequest; 
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_STBCEnable", TRUE))
    {
        *pBool = pWifiRadioFull->Cfg.X_CISCO_COM_STBCEnable; 
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "MCS", TRUE))
    {
        /* collect value */
        *pInt = pWifiRadioFull->Cfg.MCS;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TransmitPower", TRUE))
    {
        /* collect value */
        *pInt = pWifiRadioFull->Cfg.TransmitPower;
        
        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_MbssUserControl", TRUE))
    {
        *pInt = pWifiRadioFull->Cfg.MbssUserControl; 
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_CISCO_COM_AdminControl", TRUE))
    {
        *pInt = pWifiRadioFull->Cfg.AdminControl; 
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_CISCO_COM_OnOffPushButtonTime", TRUE))
    {
        *pInt = pWifiRadioFull->Cfg.OnOffPushButtonTime; 
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_CISCO_COM_ObssCoex", TRUE))
    {
        *pInt = pWifiRadioFull->Cfg.ObssCoex; 
        return TRUE;
    }
    if (AnscEqualString(ParamName, "X_CISCO_COM_MulticastRate", TRUE))
    {
        *pInt = pWifiRadioFull->Cfg.MulticastRate; 
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        CosaDmlWiFiRadioGetDinfo((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiRadio->Radio.Cfg.InstanceNumber, &pWifiRadio->Radio.DynamicInfo);
    
        /* collect value */
        *puLong = pWifiRadioFull->DynamicInfo.Status;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LastChange", TRUE))
    {
        CosaDmlWiFiRadioGetDinfo((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiRadio->Radio.Cfg.InstanceNumber, &pWifiRadio->Radio.DynamicInfo);
    
        /* collect value */
        *puLong = AnscGetTimeIntervalInSeconds(pWifiRadioFull->DynamicInfo.LastChange, AnscGetTickInSeconds()); 
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MaxBitRate", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->StaticInfo.MaxBitRate;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SupportedFrequencyBands", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->StaticInfo.SupportedFrequencyBands;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Channel", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.Channel;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AutoChannelRefreshPeriod", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.AutoChannelRefreshPeriod;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "OperatingChannelBandwidth", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.OperatingChannelBandwidth;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ExtensionChannel", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.ExtensionChannel;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "GuardInterval", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.GuardInterval;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_RTSThreshold", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.RTSThreshold;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_FragmentationThreshold", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.FragmentationThreshold;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_DTIMInterval", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.DTIMInterval;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_BeaconInterval", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.BeaconInterval;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_TxRate", TRUE))
    {
        /* collect value */
        *puLong = pWifiRadioFull->Cfg.TxRate;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_BasicRate", TRUE))
    {
        /* collect value */

       *puLong = pWifiRadioFull->Cfg.BasicRate; 
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_CTSProtectionMode", TRUE))
    {
        /* collect value */
        *puLong = (FALSE == pWifiRadioFull->Cfg.CTSProtectionMode) ? 0 : 1;

        return TRUE;
    }

    if (AnscEqualString(ParamName, "X_CISCO_COM_HTTxStream", TRUE))
    {
        *puLong = pWifiRadioFull->Cfg.X_CISCO_COM_HTTxStream; 
        return TRUE;
    }
  
    if (AnscEqualString(ParamName, "X_CISCO_COM_HTRxStream", TRUE))
    {
        *puLong = pWifiRadioFull->Cfg.X_CISCO_COM_HTRxStream; 
        return TRUE;
    }
  
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Radio_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
Radio_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;


    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->Cfg.Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->Cfg.Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->Cfg.Alias)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->StaticInfo.Name) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->StaticInfo.Name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->StaticInfo.Name)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        /*TR-181: Since Radio is a layer 1 interface, 
          it is expected that LowerLayers will not be used
         */
         /* collect value */
        AnscCopyString(pValue, "Not Applicable");
        return 0;
    }

    if( AnscEqualString(ParamName, "OperatingFrequencyBand", TRUE))
    {
        /* collect value */
        if ( 5 < *pUlSize)
        {
            if ( pWifiRadioFull->Cfg.OperatingFrequencyBand == COSA_DML_WIFI_FREQ_BAND_2_4G )
            {
                AnscCopyString(pValue, "2.4GHz");
            }
            else if ( pWifiRadioFull->Cfg.OperatingFrequencyBand == COSA_DML_WIFI_FREQ_BAND_5G )
            {
                AnscCopyString(pValue, "5GHz");
            }
            return 0;
        }
        else
        {
            *pUlSize = 6;
            
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "OperatingStandards", TRUE))
    {
        /* collect value */
        char buf[512] = {0};
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_a )
        {
            strcat(buf, "a");
        }
        
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_b )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",b");
            }
            else
            {
                strcat(buf, "b");
            }
        }
        
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_g )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",g");
            }
            else
            {
                strcat(buf, "g");
            }
        }
        
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_n )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",n");
            }
            else
            {
                strcat(buf, "n");
            }
        }

        if ( AnscSizeOfString(buf) < *pUlSize)
        {
            AnscCopyString(pValue, buf);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(buf)+1;
            
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "PossibleChannels", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->StaticInfo.PossibleChannels) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->StaticInfo.PossibleChannels);
            
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->StaticInfo.PossibleChannels)+1;
            
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "ChannelsInUse", TRUE))
    {
        CosaDmlWiFiRadioGetDinfo((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiRadio->Radio.Cfg.InstanceNumber, &pWifiRadio->Radio.DynamicInfo);
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->DynamicInfo.ChannelsInUse) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->DynamicInfo.ChannelsInUse);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->DynamicInfo.ChannelsInUse)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "TransmitPowerSupported", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->StaticInfo.TransmitPowerSupported) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->StaticInfo.TransmitPowerSupported);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->StaticInfo.TransmitPowerSupported)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "RegulatoryDomain", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiRadioFull->Cfg.RegulatoryDomain) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiRadioFull->Cfg.RegulatoryDomain);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiRadioFull->Cfg.RegulatoryDomain)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "SupportedStandards", TRUE))
    {
        /* collect value */
        char buf[512] = {0};
        if (pWifiRadioFull->StaticInfo.SupportedStandards & COSA_DML_WIFI_STD_a )
        {
            strcat(buf, "a");
        }
        
        if (pWifiRadioFull->StaticInfo.SupportedStandards & COSA_DML_WIFI_STD_b )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",b");
            }
            else
            {
                strcat(buf, "b");
            }
        }
        
        if (pWifiRadioFull->StaticInfo.SupportedStandards & COSA_DML_WIFI_STD_g )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",g");
            }
            else
            {
                strcat(buf, "g");
            }
        }
        
        if (pWifiRadioFull->StaticInfo.SupportedStandards & COSA_DML_WIFI_STD_n )
        {
            if (AnscSizeOfString(buf) != 0)
            {
                strcat(buf, ",n");
            }
            else
            {
                strcat(buf, "n");
            }
        }
        
        if ( AnscSizeOfString(buf) < *pUlSize)
        {
            AnscCopyString(pValue, buf);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(buf)+1;
            
            return 1;
        }
        return 0;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Coexistance", TRUE))
    {
        /* save update to backup */
        pWifiRadioFull->Cfg.bCoexistance = bValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* save update to backup */
        /* Currently Radio is always UP - Dont allow it to be disabled */
        /* pWifiRadioFull->Cfg.bEnabled = bValue; */
        /* return TRUE; */
          
        return FALSE;
    }

    if( AnscEqualString(ParamName, "AutoChannelEnable", TRUE))
    {
        if ( pWifiRadioFull->Cfg.AutoChannelEnable == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.AutoChannelEnable = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "IEEE80211hEnabled", TRUE))
    {
        if ( pWifiRadioFull->Cfg.IEEE80211hEnabled == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.IEEE80211hEnabled = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_APIsolation", TRUE))
    {
        if ( pWifiRadioFull->Cfg.APIsolation == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.APIsolation = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_FrameBurst", TRUE))
    {
        if ( pWifiRadioFull->Cfg.FrameBurst == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.FrameBurst = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_ApplySetting", TRUE))
    {
        if ( pWifiRadioFull->Cfg.ApplySetting == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.ApplySetting = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_ReverseDirectionGrant", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_ReverseDirectionGrant == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_ReverseDirectionGrant = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_AggregationMSDU", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_AggregationMSDU == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_AggregationMSDU = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_AutoBlockAck", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_AutoBlockAck == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_AutoBlockAck = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_DeclineBARequest", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_DeclineBARequest == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_DeclineBARequest = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_STBCEnable", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_STBCEnable == bValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_STBCEnable = bValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "MCS", TRUE))
    {
        if ( pWifiRadioFull->Cfg.MCS == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.MCS = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TransmitPower", TRUE))
    {
        if ( pWifiRadioFull->Cfg.TransmitPower == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.TransmitPower = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_MbssUserControl", TRUE))
    {
        if ( pWifiRadioFull->Cfg.MbssUserControl == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.MbssUserControl = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }
    if( AnscEqualString(ParamName, "X_CISCO_COM_AdminControl", TRUE))
    {
        if ( pWifiRadioFull->Cfg.AdminControl == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.AdminControl = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }
    if( AnscEqualString(ParamName, "X_CISCO_COM_OnOffPushButtonTime", TRUE))
    {
        if ( pWifiRadioFull->Cfg.OnOffPushButtonTime == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.OnOffPushButtonTime = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }
    if( AnscEqualString(ParamName, "X_CISCO_COM_ObssCoex", TRUE))
    {
        if ( pWifiRadioFull->Cfg.ObssCoex == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.ObssCoex = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }
    if( AnscEqualString(ParamName, "X_CISCO_COM_MulticastRate", TRUE))
    {
        if ( pWifiRadioFull->Cfg.MulticastRate == iValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.MulticastRate = iValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Channel", TRUE))
    {
        if ( pWifiRadioFull->Cfg.Channel == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.Channel = uValue;
        pWifiRadioFull->Cfg.AutoChannelEnable = FALSE; /* User has manually set a channel */
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AutoChannelRefreshPeriod", TRUE))
    {
        if ( pWifiRadioFull->Cfg.AutoChannelRefreshPeriod == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.AutoChannelRefreshPeriod = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "OperatingChannelBandwidth", TRUE))
    {
        if ( pWifiRadioFull->Cfg.OperatingChannelBandwidth == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.OperatingChannelBandwidth = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ExtensionChannel", TRUE))
    {
        if ( pWifiRadioFull->Cfg.ExtensionChannel == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.ExtensionChannel = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "GuardInterval", TRUE))
    {
        if ( pWifiRadioFull->Cfg.GuardInterval == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.GuardInterval = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_RTSThreshold", TRUE))
    {
        if ( pWifiRadioFull->Cfg.RTSThreshold == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.RTSThreshold = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_FragmentationThreshold", TRUE))
    {
        if ( pWifiRadioFull->Cfg.FragmentationThreshold == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.FragmentationThreshold = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_DTIMInterval", TRUE))
    {
        if ( pWifiRadioFull->Cfg.DTIMInterval == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.DTIMInterval = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_BeaconInterval", TRUE))
    {
        if ( pWifiRadioFull->Cfg.BeaconInterval == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.BeaconInterval = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_TxRate", TRUE))
    {
        if ( pWifiRadioFull->Cfg.TxRate == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.TxRate = uValue;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_BasicRate", TRUE))
    {
        if ( pWifiRadioFull->Cfg.BasicRate == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.BasicRate = uValue; 
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_CTSProtectionMode", TRUE))
    {
        if ( pWifiRadioFull->Cfg.CTSProtectionMode == (0 != uValue) )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.CTSProtectionMode = (0 == uValue) ? FALSE : TRUE;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_HTTxStream", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_HTTxStream == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_HTTxStream = uValue; 
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_CISCO_COM_HTRxStream", TRUE))
    {
        if ( pWifiRadioFull->Cfg.X_CISCO_COM_HTRxStream == uValue )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        pWifiRadioFull->Cfg.X_CISCO_COM_HTRxStream = uValue; 
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Radio_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiRadioFull->Cfg.Alias, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        /*TR-181: Since Radio is a layer 1 interface, 
          it is expected that LowerLayers will not be used
         */
        /* User shouldnt be able to set a value for this */
        return FALSE;
    }

    if( AnscEqualString(ParamName, "OperatingFrequencyBand", TRUE))
    {
        COSA_DML_WIFI_FREQ_BAND     TmpFreq;
        
        /* save update to backup */
        if (( AnscEqualString(pString, "2.4GHz", TRUE) ) && (1 == pWifiRadioFull->Cfg.InstanceNumber))
        {
            TmpFreq = COSA_DML_WIFI_FREQ_BAND_2_4G;
        }
        else if (( AnscEqualString(pString, "5GHz", TRUE) ) && (2 == pWifiRadioFull->Cfg.InstanceNumber))
        {
            TmpFreq = COSA_DML_WIFI_FREQ_BAND_5G;
        }
        else
    	{
    	    return FALSE; /* Radio can only support these two frequency bands and can not change dynamically */
    	}

        if ( pWifiRadioFull->Cfg.OperatingFrequencyBand == TmpFreq )
        {
            return  TRUE;
        }
        
    	pWifiRadioFull->Cfg.OperatingFrequencyBand = TmpFreq; 
        pWifiRadio->bRadioChanged = TRUE;
    	
        return TRUE;
    }

    if( AnscEqualString(ParamName, "OperatingStandards", TRUE))
    {
        ULONG                       TmpOpStd;
        
        /* save update to backup */
        TmpOpStd = 0;
        if ( AnscCharInString(pString, 'a') )
        {
            TmpOpStd |= COSA_DML_WIFI_STD_a;
        }
        if ( AnscCharInString(pString, 'b') )
        {
            TmpOpStd |= COSA_DML_WIFI_STD_b;
        }
        if ( AnscCharInString(pString, 'g') )
        {
            TmpOpStd |= COSA_DML_WIFI_STD_g;
        }
        if ( AnscCharInString(pString, 'n') )
        {
            TmpOpStd |= COSA_DML_WIFI_STD_n;
        }

        if ( pWifiRadioFull->Cfg.OperatingStandards == TmpOpStd )
        {
            return  TRUE;
        }
        
        pWifiRadioFull->Cfg.OperatingStandards = TmpOpStd;
        pWifiRadio->bRadioChanged = TRUE;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RegulatoryDomain", TRUE))
    {
        if ( AnscEqualString(pWifiRadioFull->Cfg.RegulatoryDomain, pString, TRUE) )
        {
            return  TRUE;
        }
         
        /* save update to backup */
        AnscCopyString( pWifiRadioFull->Cfg.RegulatoryDomain, pString );
        pWifiRadio->bRadioChanged = TRUE;
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Radio_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
Radio_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    PCOSA_DML_WIFI_RADIO            pWifiRadio2    = NULL;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull2= NULL;
    ULONG                           idx            = 0;
    ULONG                           maxCount       = 0;
  
    /*Alias should be non-empty*/
    if (AnscSizeOfString(pWifiRadio->Radio.Cfg.Alias) == 0)
    {
        CcspTraceWarning(("********Radio Validate:Failed Alias \n"));
        AnscCopyString(pReturnParamName, "Alias");
        *puLength = AnscSizeOfString("Alias");
        return FALSE;
    }
 
    for ( idx = 0; idx < pMyObject->RadioCount; idx++)
    {
        if ( pWifiRadio == (pMyObject->pRadio + idx) )
        {
            continue;
        }
        
        pWifiRadio2     = pMyObject->pRadio + idx;
        pWifiRadioFull2 = &pWifiRadio2->Radio;
        
        if ( AnscEqualString(pWifiRadio->Radio.Cfg.Alias, pWifiRadio2->Radio.Cfg.Alias, TRUE) )
        {
            CcspTraceWarning(("********Radio Validate:Failed Alias \n"));
            AnscCopyString(pReturnParamName, "Alias");
            *puLength = AnscSizeOfString("Alias");
            return FALSE;
        }
    }

    if (!(CosaUtilChannelValidate2(pWifiRadioFull->Cfg.OperatingFrequencyBand,pWifiRadioFull->Cfg.Channel,pWifiRadioFull->StaticInfo.PossibleChannels)))
        {
            CcspTraceWarning(("********Radio Validate:Failed Channel\n"));
            AnscCopyString(pReturnParamName, "Channel");
            *puLength = AnscSizeOfString("Channel");
            return FALSE;
        }

    
    if ( pWifiRadioFull->Cfg.OperatingFrequencyBand == COSA_DML_WIFI_FREQ_BAND_2_4G )
    {
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_a)
        {
            CcspTraceWarning(("********Radio Validate:Failed OperatingStandards\n"));
            AnscCopyString(pReturnParamName, "OperatingStandards");
            *puLength = AnscSizeOfString("OperatingStandards");
            return FALSE;
        }
    }
    
    if ( pWifiRadioFull->Cfg.OperatingFrequencyBand == COSA_DML_WIFI_FREQ_BAND_5G )
    {
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_b)
        {
            CcspTraceWarning(("********Radio Validate:Failed OperatingStandards\n"));
            AnscCopyString(pReturnParamName, "OperatingStandards");
            *puLength = AnscSizeOfString("OperatingStandards");
            return FALSE;
        }
        
        if (pWifiRadioFull->Cfg.OperatingStandards & COSA_DML_WIFI_STD_g)
        {
            CcspTraceWarning(("********Radio Validate:Failed OperatingStandards\n"));
            AnscCopyString(pReturnParamName, "OperatingStandards");
            *puLength = AnscSizeOfString("OperatingStandards");
            return FALSE;
        }
    }
/*
    if( (pWifiRadioFull->Cfg.TransmitPower != 25) && (pWifiRadioFull->Cfg.TransmitPower != 50) && (pWifiRadioFull->Cfg.TransmitPower != 100) )
    {
         CcspTraceWarning(("********Radio Validate:Failed Transmit Power\n"));
         AnscCopyString(pReturnParamName, "TransmitPower");
         *puLength = AnscSizeOfString("TransmitPower");
         return FALSE;
    }    
*/
    if( (pWifiRadioFull->Cfg.BeaconInterval < 0) || (pWifiRadioFull->Cfg.BeaconInterval > 65535) )
    {
         CcspTraceWarning(("********Radio Validate:Failed BeaconInterval\n"));
         AnscCopyString(pReturnParamName, "BeaconInterval");
         *puLength = AnscSizeOfString("BeaconInterval");
         return FALSE;
    }

    if( (pWifiRadioFull->Cfg.DTIMInterval < 0) || (pWifiRadioFull->Cfg.DTIMInterval > 255) )
    {
         CcspTraceWarning(("********Radio Validate:Failed DTIMInterval\n"));
         AnscCopyString(pReturnParamName, "DTIMInterval");
         *puLength = AnscSizeOfString("DTIMInterval");
         return FALSE;
    }

// need to fix temporary in order for the set to work
// Value of 0 == off
/*
    if( (pWifiRadioFull->Cfg.FragmentationThreshold > 2346) )
    {
         CcspTraceWarning(("********Radio Validate:Failed FragThreshhold\n"));
         AnscCopyString(pReturnParamName, "FragmentationThreshold");
         *puLength = AnscSizeOfString("FragmentationThreshold");
         return FALSE;
    }

    if( (pWifiRadioFull->Cfg.RTSThreshold > 2347) )
    {
         CcspTraceWarning(("********Radio Validate:Failed RTSThreshhold\n"));
         AnscCopyString(pReturnParamName, "RTSThreshold");
         *puLength = AnscSizeOfString("RTSThreshold");
         return FALSE;
    }

    maxCount = (60 * 71582787);
    if(pWifiRadioFull->Cfg.AutoChannelRefreshPeriod > maxCount)
    {
         CcspTraceWarning(("********Radio Validate:Failed AutoChannelRefreshPeriod\n"));
         AnscCopyString(pReturnParamName, "AutoChannelRefreshPeriod");
         *puLength = AnscSizeOfString("AutoChannelRefreshPeriod");
         return FALSE;
    }*/

#if 0
    if(pWifiRadioFull->Cfg.RegulatoryDomain[2] != 'I')
    {
         /* Currently driver supports only Inside a country code */
         CcspTraceWarning(("********Radio Validate:Failed Regulatory Domain \n"));
         AnscCopyString(pReturnParamName, "RegulatoryDomain");
         *puLength = AnscSizeOfString("RegulatoryDomain");
         return FALSE;
    }
#endif

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Radio_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Radio_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = hInsContext;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    PCOSA_DML_WIFI_RADIO_CFG        pWifiRadioCfg  = &pWifiRadioFull->Cfg;

    if ( 0 )//!pWifiRadio->bRadioChanged )
    {
        return  ANSC_STATUS_SUCCESS;
    }
    else
    {
        pWifiRadio->bRadioChanged = FALSE;
        CcspTraceInfo(("WiFi Radio -- apply the change...\n"));
    }
    
    return CosaDmlWiFiRadioSetCfg((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiRadioCfg);
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Radio_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Radio_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_RADIO            pWifiRadio     = pMyObject->pRadio;
    PCOSA_DML_WIFI_RADIO_FULL       pWifiRadioFull = &pWifiRadio->Radio;
    ULONG                           idx            = 0;    
    PCOSA_DML_WIFI_RADIO            pWifiRadio2    = hInsContext;
    
    for (idx=0; idx < pMyObject->RadioCount; idx++)
    {
        if ( pWifiRadio2 == pWifiRadio+idx )
        {
            return CosaDmlWiFiRadioGetCfg((ANSC_HANDLE)pMyObject->hPoamWiFiDm, &pWifiRadio2->Radio.Cfg) ;
        }
    }
    
    return ANSC_STATUS_SUCCESS;
}

/***********************************************************************

 APIs for Object:

    WiFi.SSID.{i}.

    *  SSID_GetEntryCount
    *  SSID_GetEntry
    *  SSID_AddEntry
    *  SSID_DelEntry
    *  SSID_GetParamBoolValue
    *  SSID_GetParamIntValue
    *  SSID_GetParamUlongValue
    *  SSID_GetParamStringValue
    *  SSID_SetParamBoolValue
    *  SSID_SetParamIntValue
    *  SSID_SetParamUlongValue
    *  SSID_SetParamStringValue
    *  SSID_Validate
    *  SSID_Commit
    *  SSID_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SSID_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
SSID_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    ULONG                           entryCount    = 0;
    
    entryCount = AnscSListQueryDepth(&pMyObject->SsidQueue);
    
    return entryCount;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        SSID_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
SSID_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj      = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry   = (PSINGLE_LINK_ENTRY       )NULL;

    pSLinkEntry = AnscQueueGetEntryByIndex(&pMyObject->SsidQueue, nIndex);
    
    if (pSLinkEntry)
    {
        pLinkObj = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
        
        *pInsNumber = pLinkObj->InstanceNumber;
    }
    
    return pLinkObj; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        SSID_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/
ANSC_HANDLE
SSID_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{

    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj      = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PCOSA_DML_WIFI_SSID             pWifiSsid     = (PCOSA_DML_WIFI_SSID      )NULL;
    PCOSA_DML_WIFI_SSID_FULL        pWifiSsidFull = (PCOSA_DML_WIFI_SSID_FULL )NULL;

    pLinkObj                   = (PCOSA_CONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(COSA_CONTEXT_LINK_OBJECT));
    if (!pLinkObj)
    {
        return NULL;
    }
    
    pWifiSsid                  = AnscAllocateMemory(sizeof(COSA_DML_WIFI_SSID));
    if (!pWifiSsid)
    {
        AnscFreeMemory(pLinkObj);
        return NULL;
    }
    
    if (TRUE)
    {
        pLinkObj->InstanceNumber           = pMyObject->ulSsidNextInstance;
    
        pWifiSsid->SSID.Cfg.InstanceNumber = pMyObject->ulSsidNextInstance;
    
        pMyObject->ulSsidNextInstance++;

        if ( pMyObject->ulSsidNextInstance == 0 )
        {
            pMyObject->ulSsidNextInstance = 1;
        }
    
        /*Set default Name, SSID & Alias*/
        _ansc_sprintf(pWifiSsid->SSID.StaticInfo.Name, "SSID%d", *pInsNumber);
        _ansc_sprintf(pWifiSsid->SSID.Cfg.Alias, "SSID%d", *pInsNumber);
        _ansc_sprintf(pWifiSsid->SSID.Cfg.SSID, "Cisco-SSID-%d", *pInsNumber);
    
        pLinkObj->hContext         = (ANSC_HANDLE)pWifiSsid;
        pLinkObj->hParentTable     = NULL;
        pLinkObj->bNew             = TRUE;
        
        pWifiSsidFull              = &pWifiSsid->SSID;
        
        CosaSListPushEntryByInsNum((PSLIST_HEADER)&pMyObject->SsidQueue, pLinkObj);

        CosaWifiRegAddSsidInfo((ANSC_HANDLE)pMyObject, (ANSC_HANDLE)pLinkObj);
    }
    
    *pInsNumber = pLinkObj->InstanceNumber;
    
    return (ANSC_HANDLE)pLinkObj; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SSID_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SSID_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{

    PCOSA_DATAMODEL_WIFI            pMyObject    = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInstance;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry  = (PSINGLE_LINK_ENTRY       )NULL;
    PCOSA_CONTEXT_LINK_OBJECT       pAPLinkObj   = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PCOSA_DML_WIFI_AP               pWifiAp      = (PCOSA_DML_WIFI_AP        )NULL;
    UCHAR                           PathName[64] = {0};
   
    if(pLinkObj)
        pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    
#if 0    
    /*Reset the SSIDReference in AccessPoint table*/
    sprintf(PathName, "Device.WiFi.SSID.%d.", pLinkObj->InstanceNumber);
    
    pSLinkEntry = AnscQueueGetFirstEntry(&pMyObject->AccessPointQueue);
    while ( pSLinkEntry )
    {
        pAPLinkObj   = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
        pWifiAp      = pAPLinkObj->hContext;
        
        if (AnscEqualString(pWifiAp->AP.Cfg.SSID, PathName, TRUE))
        {
            memset(pWifiAp->AP.Cfg.SSID, 0, sizeof(pWifiAp->AP.Cfg.SSID));
            
            pAPLinkObj->bNew = TRUE;
            
            CosaWifiRegAddAPInfo((ANSC_HANDLE)pMyObject, (ANSC_HANDLE)pAPLinkObj);
        }
        
        pSLinkEntry             = AnscQueueGetNextEntry(pSLinkEntry);
    }
#endif

    if (pLinkObj->bNew)
    {
        CosaWifiRegDelSsidInfo((ANSC_HANDLE)pMyObject, (ANSC_HANDLE)pLinkObj);
    }
	else
	{
		if ( CosaDmlWiFiSsidDelEntry((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiSsid->SSID.Cfg.InstanceNumber) != ANSC_STATUS_SUCCESS )
		{
			return ANSC_STATUS_FAILURE;
		}
	}
	AnscQueuePopEntryByLink(&pMyObject->SsidQueue, &pLinkObj->Linkage);
	
    if (pLinkObj->hContext)
    {
        AnscFreeMemory(pLinkObj->hContext);
    }
    
    if (pLinkObj)
    {
        AnscFreeMemory(pLinkObj);
    }

    return 0; /* succeeded */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* collect value */
        *pBool = pWifiSsid->SSID.Cfg.bEnabled;

        return TRUE;
    }

    if( AnscEqualString(ParamName, "SSIDBroadcast", TRUE))
    {
        /* collect value */
        *pBool = pWifiSsid->SSID.Cfg.bSSIDBroadcast;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    /* check the parameter name and return the corresponding value */

    CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject    = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;

    CosaDmlWiFiSsidGetDinfo((ANSC_HANDLE)pMyObject->hPoamWiFiDm, pWifiSsid->SSID.Cfg.InstanceNumber, &pWifiSsid->SSID.DynamicInfo);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        /* collect value */
        *puLong  = pWifiSsid->SSID.DynamicInfo.Status;
        return TRUE;
    }

	/*
    if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        *puLong  = pWifiSsid->SSID.Cfg.SSIDIndex;
        return TRUE;
    }
	*/

    if( AnscEqualString(ParamName, "SSIDVlanID", TRUE))
    {
        /* collect value */
        *puLong  = pWifiSsid->SSID.Cfg.SSIDVlanID;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SSIDLanBase", TRUE))
    {
        /* collect value */
        *puLong  = pWifiSsid->SSID.Cfg.SSIDLanBase.Value;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MaxClients", TRUE))
    {
        /* collect value */
        *puLong  = pWifiSsid->SSID.Cfg.MaxClients;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LastChange", TRUE))
    {
        /* collect value */
        *puLong  = AnscGetTimeIntervalInSeconds(pWifiSsid->SSID.DynamicInfo.LastChange, AnscGetTickInSeconds());
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SSID_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
SSID_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    PUCHAR                          pLowerLayer  = NULL;
    
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.Alias)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        /* collect value */
        //if ( AnscSizeOfString(pWifiSsid->SSID.StaticInfo.Name) < *pUlSize)
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.WiFiRadioName) < *pUlSize)
        {
            //AnscCopyString(pValue, pWifiSsid->SSID.StaticInfo.Name);
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.WiFiRadioName);
            return 0;
        }
        else
        {
            //*pUlSize = AnscSizeOfString(pWifiSsid->SSID.StaticInfo.Name)+1;
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.WiFiRadioName)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.SSIDIndex) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.SSIDIndex);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.SSIDIndex)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        /* collect value */
        pLowerLayer = CosaUtilGetLowerLayers("Device.WiFi.Radio.", pWifiSsid->SSID.Cfg.WiFiRadioName);
        
        if (pLowerLayer != NULL)
        {
            AnscCopyString(pValue, pLowerLayer);
            
            AnscFreeMemory(pLowerLayer);
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "BSSID", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.StaticInfo.BSSID) < *pUlSize)
        {
	    _ansc_sprintf
            (
                pValue,
                "%02X:%02X:%02X:%02X:%02X:%02X",
		pWifiSsid->SSID.StaticInfo.BSSID[0],
                pWifiSsid->SSID.StaticInfo.BSSID[1],
                pWifiSsid->SSID.StaticInfo.BSSID[2],
                pWifiSsid->SSID.StaticInfo.BSSID[3],
                pWifiSsid->SSID.StaticInfo.BSSID[4],
                pWifiSsid->SSID.StaticInfo.BSSID[5]
            );
            *pUlSize = AnscSizeOfString(pValue);

            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.StaticInfo.BSSID)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "MACAddress", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.StaticInfo.MacAddress) < *pUlSize)
        {
	    _ansc_sprintf
            (
                pValue,
                "%02X:%02X:%02X:%02X:%02X:%02X",
		pWifiSsid->SSID.StaticInfo.MacAddress[0],
                pWifiSsid->SSID.StaticInfo.MacAddress[1],
                pWifiSsid->SSID.StaticInfo.MacAddress[2],
                pWifiSsid->SSID.StaticInfo.MacAddress[3],
                pWifiSsid->SSID.StaticInfo.MacAddress[4],
                pWifiSsid->SSID.StaticInfo.MacAddress[5]
            );

            *pUlSize = AnscSizeOfString(pValue);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.StaticInfo.MacAddress)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "SSID", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.SSID) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.SSID);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.SSID)+1;
            return 1;
        }
        return 0;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        if ( pWifiSsid->SSID.Cfg.bEnabled == bValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pWifiSsid->SSID.Cfg.bEnabled = bValue;
        pWifiSsid->bSsidChanged = TRUE; 
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SSIDBroadcast", TRUE))
    {
        /* collect value */
        pWifiSsid->SSID.Cfg.bSSIDBroadcast = bValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject    = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
	/*
    if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        pWifiSsid->SSID.Cfg.SSIDIndex = uValue;
        return TRUE;
    }*/

    if( AnscEqualString(ParamName, "SSIDVlanID", TRUE))
    {
        /* collect value */
        pWifiSsid->SSID.Cfg.SSIDVlanID = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SSIDLanBase", TRUE))
    {
        /* collect value */
        pWifiSsid->SSID.Cfg.SSIDLanBase.Value = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MaxClients", TRUE))
    {
        /* collect value */
        pWifiSsid->SSID.Cfg.MaxClients = uValue;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSID_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj              = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid             = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    ULONG                           ulEntryNameLen        = 256;
    UCHAR                           ucEntryParamName[256] = {0};
    UCHAR                           ucEntryNameValue[256] = {0};
   
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.Alias, pString );

        return TRUE;
    }

	if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.SSIDIndex, pString );

        return TRUE;
    }
#if 0
    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        /* save update to backup */
    #ifdef _COSA_SIM_
        _ansc_sprintf(ucEntryParamName, "%s%s", pString, "Name");
        
        if ( ( 0 == CosaGetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
             (AnscSizeOfString(ucEntryNameValue) != 0) )
        {
            AnscCopyString(pWifiSsid->SSID.Cfg.WiFiRadioName, ucEntryNameValue);
            
            return TRUE;
        }
    #else
    	if(0 == strlen(pWifiSsid->SSID.Cfg.WiFiRadioName)) { /*Lower layers can only be specified during creation */
    
                _ansc_sprintf(ucEntryParamName, "%s%s", pString, "Name");
            
                if ( ( 0 == g_GetParamValueString(ucEntryParamName, ucEntryNameValue, &ulEntryNameLen)) &&
                     (AnscSizeOfString(ucEntryNameValue) != 0) )
                {
                    AnscCopyString(pWifiSsid->SSID.Cfg.WiFiRadioName, ucEntryNameValue);
                    return TRUE;
                }
        } else
            return FALSE;
    #endif
    }
#endif

    if ( AnscEqualString(ParamName, "SSID", TRUE) )
    {
        if ( AnscEqualString(pWifiSsid->SSID.Cfg.SSID, pString, TRUE) )
        {
            return  TRUE;
        }
        
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.SSID, pString );
        pWifiSsid->bSsidChanged = TRUE; 
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SSID_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
SSID_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid     = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    PSINGLE_LINK_ENTRY              pSLinkEntry   = (PSINGLE_LINK_ENTRY       )NULL;
    PCOSA_CONTEXT_LINK_OBJECT       pSSIDLinkObj  = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PCOSA_DML_WIFI_SSID             pWifiSsid2    = (PCOSA_DML_WIFI_SSID      )NULL;
  
    /*Alias should be non-empty*/
    if (AnscSizeOfString(pWifiSsid->SSID.Cfg.Alias) == 0)
    {
        AnscCopyString(pReturnParamName, "Alias");
        *puLength = AnscSizeOfString("Alias");
        return FALSE;
    }

    /* Lower Layers has to be non-empty */
	/*
    if (AnscSizeOfString(pWifiSsid->SSID.Cfg.WiFiRadioName) == 0)
    {
        AnscCopyString(pReturnParamName, "LowerLayers");
        *puLength = AnscSizeOfString("LowerLayers");
        return FALSE;
    }
	*/
    pSLinkEntry = AnscQueueGetFirstEntry(&pMyObject->SsidQueue);
    
    while ( pSLinkEntry )
    {
        pSSIDLinkObj = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
        pSLinkEntry  = AnscQueueGetNextEntry(pSLinkEntry);
        pWifiSsid2   = pSSIDLinkObj->hContext;

        if (pSSIDLinkObj == pLinkObj)
        {
            continue;
        }
        
        if (AnscEqualString(pWifiSsid->SSID.Cfg.SSID, pWifiSsid2->SSID.Cfg.SSID, TRUE))
        {
            AnscCopyString(pReturnParamName, "SSID");

            *puLength = AnscSizeOfString("SSID");
            return FALSE;
        }
    }
    
    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SSID_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SSID_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject    = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj     = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid    = (PCOSA_DML_WIFI_SSID      )pLinkObj->hContext;
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    
    if (pLinkObj->bNew)
    {
        pLinkObj->bNew = FALSE;
        
        returnStatus = CosaDmlWiFiSsidAddEntry((ANSC_HANDLE)pMyObject->hPoamWiFiDm, &pWifiSsid->SSID);
        
        if (returnStatus != ANSC_STATUS_SUCCESS)
        {
            return ANSC_STATUS_FAILURE;
        }
        
        CosaWifiRegDelSsidInfo((ANSC_HANDLE)pMyObject, (ANSC_HANDLE)pLinkObj);
    }
    else
    {
        if ( 0 )//!pWifiSsid->bSsidChanged )
        {
            return  ANSC_STATUS_SUCCESS;
        }
        else
        {
            pWifiSsid->bSsidChanged = FALSE;
            CcspTraceInfo(("WiFi SSID -- apply the changes...\n"));
        }
        return CosaDmlWiFiSsidSetCfg((ANSC_HANDLE)pMyObject->hPoamWiFiDm, &pWifiSsid->SSID.Cfg);
    }
    
    return 0;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SSID_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SSID_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj      = (PCOSA_CONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PSINGLE_LINK_ENTRY              pSLinkEntry   = (PSINGLE_LINK_ENTRY       )NULL;
    PCOSA_CONTEXT_LINK_OBJECT       pSSIDLinkObj  = (PCOSA_CONTEXT_LINK_OBJECT)NULL;
    PCOSA_DML_WIFI_SSID             pWifiSsid     = (PCOSA_DML_WIFI_SSID      )NULL;
    ULONG                           idx           = 0;
    
    pSLinkEntry = AnscQueueGetFirstEntry(&pMyObject->SsidQueue);
    
    while ( pSLinkEntry )
    {
        pSSIDLinkObj = ACCESS_COSA_CONTEXT_LINK_OBJECT(pSLinkEntry);
        
        if (pSSIDLinkObj == pLinkObj)
        {
            break;
        }
        idx++;
        
        pSLinkEntry             = AnscQueueGetNextEntry(pSLinkEntry);
    }
    
    pWifiSsid   = (PCOSA_DML_WIFI_SSID)pLinkObj->hContext;
    
    return CosaDmlWiFiSsidGetEntry((ANSC_HANDLE)pMyObject->hPoamWiFiDm, idx, &pWifiSsid->SSID);  
}

/***********************************************************************

 APIs for Object:

    WiFi.SSID.{i}.SSIDEncryption..

    *  SSIDEncryption_GetParamBoolValue
    *  SSIDEncryption_GetParamIntValue
    *  SSIDEncryption_GetParamUlongValue
    *  SSIDEncryption_GetParamStringValue

***********************************************************************/
BOOL
SSIDEncryption_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;
    
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "ModeEnabled", TRUE))
    {
        /* collect value */
        *puLong = pWifiSsid->SSID.Cfg.EncryptionInfo.ModeEnabled;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Encryption", TRUE))
    {
        /* collect value */
        *puLong = pWifiSsid->SSID.Cfg.EncryptionInfo.Encryption;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RekeyInterval", TRUE))
    {
        /* collect value */
        *puLong = pWifiSsid->SSID.Cfg.EncryptionInfo.RekeyInterval;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RadiusServerIP", TRUE))
    {
        /* collect value */
        *puLong = pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusServerIP.Value;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RadiusServerPort", TRUE))
    {
        /* collect value */
        *puLong = pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusServerPort;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

ULONG
SSIDEncryption_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "WepKey", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.WepKey) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.EncryptionInfo.WepKey);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.WepKey)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "PreSharedKey", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.PreSharedKey) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.EncryptionInfo.PreSharedKey);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.PreSharedKey)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Passphrase", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.Passphrase) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.EncryptionInfo.Passphrase);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.Passphrase)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "RadiusSecret", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusSecret) < *pUlSize)
        {
            AnscCopyString(pValue, pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusSecret);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusSecret)+1;
            return 1;
        }
        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

BOOL
SSIDEncryption_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ModeEnabled", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.EncryptionInfo.ModeEnabled = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Encryption", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.EncryptionInfo.Encryption = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RekeyInterval", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.EncryptionInfo.RekeyInterval = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RadiusServerIP", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusServerIP.Value = uValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RadiusServerPort", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusServerPort = uValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
SSIDEncryption_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "WepKey", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.EncryptionInfo.WepKey, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PreSharedKey", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.EncryptionInfo.PreSharedKey, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "Passphrase", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.EncryptionInfo.Passphrase, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RadiusSecret", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pWifiSsid->SSID.Cfg.EncryptionInfo.RadiusSecret, pString );
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
SSIDEncryption_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}  

ULONG
SSIDEncryption_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    return CosaDmlWiFiSSIDEncryptionSetCfg(NULL, pWifiSsid->SSID.Cfg.InstanceNumber, &(pWifiSsid->SSID.Cfg.EncryptionInfo));
}

ULONG
SSIDEncryption_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    CosaDmlWiFiSSIDEncryptionGetCfg(NULL, pWifiSsid->SSID.Cfg.InstanceNumber, &(pWifiSsid->SSID.Cfg.EncryptionInfo));

    return ANSC_STATUS_SUCCESS;
}

/***********************************************************************

 APIs for Object:

    WiFi.SSID.{i}.QosInfo.

    *  SSIDQoS_GetParamBoolValue
    *  SSIDQoS_GetParamIntValue
    *  SSIDQoS_GetParamUlongValue
    *  SSIDQoS_GetParamStringValue

***********************************************************************/
BOOL
SSIDQoS_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "WMMEnable", TRUE))
    {
        /* collect value */
        *pBool = pWifiSsid->SSID.Cfg.QosInfo.WMMEnable;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UAPSDEnable", TRUE))
    {
        /* collect value */
        *pBool = pWifiSsid->SSID.Cfg.QosInfo.UAPSDEnable;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
SSIDQoS_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "WMMEnable", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.QosInfo.WMMEnable = bValue;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UAPSDEnable", TRUE))
    {
        /* save update to backup */
        pWifiSsid->SSID.Cfg.QosInfo.UAPSDEnable = bValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
SSIDQoS_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}  

ULONG
SSIDQoS_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    return CosaDmlWiFiSSIDQoSSetCfg(NULL, pWifiSsid->SSID.Cfg.InstanceNumber, &(pWifiSsid->SSID.Cfg.QosInfo));
}

ULONG
SSIDQoS_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject      = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;

    CosaDmlWiFiSSIDQoSGetCfg(NULL, pWifiSsid->SSID.Cfg.InstanceNumber, &(pWifiSsid->SSID.Cfg.QosInfo));

    return ANSC_STATUS_SUCCESS;
}

/***********************************************************************

 APIs for Object:

    WiFi.SSID.{i}.SSIDQoS.QosSettings.{i}.

    *  QosSettings_GetParamBoolValue
    *  QosSettings_GetParamIntValue
    *  QosSettings_GetParamUlongValue
    *  QosSettings_GetParamStringValue

***********************************************************************/
ULONG
QosSettings_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj      = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid     = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;
    ULONG                           count         = 0;
    
    count = CosaDmlWiFiSSIDQosSettingGetCount(NULL, pWifiSsid->SSID.Cfg.InstanceNumber);

    return count;
}

ANSC_HANDLE
QosSettings_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_CONTEXT_LINK_OBJECT       pLinkObj       = (PCOSA_CONTEXT_LINK_OBJECT )hInsContext;
    PCOSA_DML_WIFI_SSID             pWifiSsid      = (PCOSA_DML_WIFI_SSID       )pLinkObj->hContext;
    
    CosaDmlWiFiSSIDQosSettingGetEntry(NULL, pWifiSsid->SSID.Cfg.InstanceNumber, nIndex, &(pWifiSsid->SSID.Cfg.QosInfo.QosSetting[nIndex]));

    pWifiSsid->SSID.Cfg.QosInfo.QosSetting[nIndex].SSIDInstanceNumber = pWifiSsid->SSID.Cfg.InstanceNumber;

    *pInsNumber = pWifiSsid->SSID.Cfg.QosInfo.QosSetting[nIndex].InstanceNumber;
    
    return &(pWifiSsid->SSID.Cfg.QosInfo.QosSetting[nIndex]); /* return the handle */
}

BOOL
QosSettings_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

	CosaDmlWiFiSSIDQosSettingGetCfg(NULL, pQosSetting->SSIDInstanceNumber, pQosSetting);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "ACM", TRUE))
    {
        /* collect value */
        *pBool = pQosSetting->ACM;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "NoACK", TRUE))
    {
        /* collect value */
        *pBool = pQosSetting->NoACK;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
QosSettings_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

	CosaDmlWiFiSSIDQosSettingGetCfg(NULL, pQosSetting->SSIDInstanceNumber, pQosSetting);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "AC", TRUE))
    {
        /* collect value */
        *puLong = pQosSetting->AC;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AIFSN", TRUE))
    {
        /* collect value */
        *puLong = pQosSetting->AIFSN;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CWMin", TRUE))
    {
        /* collect value */
        *puLong = pQosSetting->CWMin;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CWMax", TRUE))
    {
        /* collect value */
        *puLong = pQosSetting->CWMax;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TXOPLimit", TRUE))
    {
        /* collect value */
        *puLong = pQosSetting->TXOPLimit;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
QosSettings_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "ACM", TRUE))
    {
        /* collect value */
        pQosSetting->ACM = bValue;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "NoACK", TRUE))
    {
        /* collect value */
        pQosSetting->NoACK = bValue;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL
QosSettings_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

    if( AnscEqualString(ParamName, "AC", TRUE))
    {
        /* collect value */
        pQosSetting->AC = uValue;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "AIFSN", TRUE))
    {
        /* collect value */
        pQosSetting->AIFSN = uValue;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CWMin", TRUE))
    {
        /* collect value */
        pQosSetting->CWMin = uValue;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CWMax", TRUE))
    {
        /* collect value */
        pQosSetting->CWMax = uValue;
        
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TXOPLimit", TRUE))
    {
        /* collect value */
        pQosSetting->TXOPLimit = uValue;
        
        return TRUE;
    }

    return FALSE;
}

BOOL
QosSettings_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}  

ULONG
QosSettings_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

    return CosaDmlWiFiSSIDQosSettingSetCfg(NULL, pQosSetting->SSIDInstanceNumber, pQosSetting);
}

ULONG
QosSettings_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_SSID_QosSetting  pQosSetting   = (PCOSA_DML_WIFI_SSID_QosSetting)hInsContext;

    CosaDmlWiFiSSIDQosSettingGetCfg(NULL, pQosSetting->SSIDInstanceNumber, pQosSetting);

    return ANSC_STATUS_SUCCESS;
}

/***********************************************************************

 APIs for Object:

    WiFi.WPS.

    *  WPS_GetParamBoolValue
    *  WPS_GetParamUlongValue
    *  WPS_GetParamStringValue
    *  WPS_SetParamBoolValue
    *  WPS_SetParamUlongValue
    *  WPS_SetParamStringValue
    *  WPS_Validate
    *  WPS_Commit
    *  WPS_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WPS_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* collect value */
        CosaDmlWiFiWPSGetCfg(NULL, pWPS);
        *pBool = pWPS->bEnabled;;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WPS_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);

    /* check the parameter name and return the corresponding value */
	/*
    if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        CosaDmlWiFiWPSGetCfg(NULL, pWPS);
        *puLong = pWPS->SSIDIndex;
        return TRUE;
    }
	*/
	return TRUE;
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        WPS_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
WPS_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);
    
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "X_CISCO_COM_Pin", TRUE))
    {
        /* collect value */
        CosaDmlWiFiWPSGetCfg(NULL, pWPS);

        if ( AnscSizeOfString(pWPS->X_CISCO_COM_Pin) < *pUlSize)
        {
            AnscCopyString(pValue, pWPS->X_CISCO_COM_Pin);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWPS->X_CISCO_COM_Pin)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        /* collect value */
        CosaDmlWiFiWPSGetCfg(NULL, pWPS);

        if ( AnscSizeOfString(pWPS->SSIDIndex) < *pUlSize)
        {
            AnscCopyString(pValue, pWPS->SSIDIndex);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pWPS->SSIDIndex)+1;
            return 1;
        }
        return 0;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WPS_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* save update to backup */
        pWPS->bEnabled = bValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WPS_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);

    /* check the parameter name and set the corresponding value */

	/*
    if( AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        pWPS->SSIDIndex = uValue;
        return TRUE;
    }
	*/
	return TRUE;
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
WPS_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);
    
    /* check the parameter name and set the corresponding value */
    if (AnscEqualString(ParamName, "X_CISCO_COM_Pin", TRUE))
    {
        if (AnscSizeOfString(pString) > sizeof(pWPS->X_CISCO_COM_Pin) - 1)
            return FALSE;

        AnscCopyString(pWPS->X_CISCO_COM_Pin, pString);
        return TRUE;
    }
	
	if (AnscEqualString(ParamName, "SSIDIndex", TRUE))
    {
        if (AnscSizeOfString(pString) > sizeof(pWPS->SSIDIndex) - 1)
            return FALSE;

        AnscCopyString(pWPS->SSIDIndex, pString);
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        WPS_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
WPS_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        WPS_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
WPS_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);
   
   CosaDmlWiFiWPSSetCfg(NULL, pWPS);

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        WPS_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
WPS_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI)g_pCosaBEManager->hWifi;
    PCOSA_DML_WIFI_WPS              pWPS          = (PCOSA_DML_WIFI_WPS  )&(pMyObject->WPS);

    CosaDmlWiFiWPSGetCfg(NULL, pWPS);
    
    return ANSC_STATUS_SUCCESS;
}

ULONG
ExtenderDevice_GetEntryCount
	(
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    return pMyObject->ext_count;
}

ANSC_HANDLE
ExtenderDevice_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;

    if (nIndex < pMyObject->ext_count)
    {
        *pInsNumber  = nIndex + 1;

        return pMyObject->ext_status + nIndex * sizeof(struct ExtStatus);
    }

    return NULL; /* return the handle */
}

BOOL
ExtenderDevice_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
	return TRUE;
    if ( !pMyObject->ext_update_time ) 
    {
        pMyObject->ext_update_time = AnscGetTickInSeconds();

        return TRUE;
    }
    
    if ( pMyObject->ext_update_time >= TIME_NO_NEGATIVE(AnscGetTickInSeconds() - WECB_EXT_REFRESH_INTERVAL) )
    {
        return FALSE;
    }
    else 
    {
        pMyObject->ext_update_time = AnscGetTickInSeconds();

        return TRUE;
    }
}

ULONG
ExtenderDevice_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;

    if ( pMyObject->ext_status )
    {
        AnscFreeMemory(pMyObject->ext_status);
        pMyObject->ext_status = NULL;
    }
    
    pMyObject->ext_count = 0;
    
    returnStatus = CosaDmlWiFi_GetExtStatus(&(pMyObject->ext_count), &(pMyObject->ext_status));

    if ( returnStatus != ANSC_STATUS_SUCCESS )
    {
        pMyObject->ext_status = NULL;
        pMyObject->ext_count = 0;
    }
    
    return 0;
}

BOOL
ExtenderDevice_GetParamUlongValue
	(
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
	struct ExtStatus               *pMyObject     = (struct ExtStatus *)hInsContext;

	if( AnscEqualString(ParamName, "Status", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->status;
        
        return TRUE;
    }

	return FALSE;
}

ULONG
ExtenderDevice_GetParamStringValue
	(
		ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
	struct ExtStatus               *pMyObject     = (struct ExtStatus *)hInsContext;

	if( AnscEqualString(ParamName, "IPAddress", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->ip) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->ip);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->ip)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "DeviceName", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->device_name) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->device_name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->device_name)+1;
            return 1;
        }
        return 0;
    }
	
	if( AnscEqualString(ParamName, "VendorName", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->vendor_name) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->vendor_name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->vendor_name)+1;
            return 1;
        }
        return 0;
    }
	
	if( AnscEqualString(ParamName, "ModelNumber", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->model_name) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->model_name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->model_name)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "FirmwareVersion", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->fw_version) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->fw_version);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->fw_version)+1;
            return 1;
        }
        return 0;
    }

	return -1;
}

ULONG
ExtenderSSID_GetEntryCount
	(
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
	struct ExtStatus                *pStatus      = (struct ExtStatus *)hInsContext;

    if (!pMyObject->ext_count)
		return 0;
	
	if (pStatus)
		return pStatus->ext_ssid_num;
	
	return 0;
}

ANSC_HANDLE
ExtenderSSID_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
	struct ExtStatus                *pStatus      = (struct ExtStatus *)hInsContext;

    if (nIndex < pStatus->ext_ssid_num)
    {
        *pInsNumber  = nIndex + 1;

        return &pStatus->ssid[nIndex];
    }

    return NULL; /* return the handle */
}

BOOL
ExtenderSSID_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return TRUE;
}

ULONG
ExtenderSSID_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

BOOL
ExtenderSSID_GetParamUlongValue
	(
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
	struct ExtSSID               *pMyObject     = (struct ExtSSID *)hInsContext;

	if( AnscEqualString(ParamName, "Channel", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->channel;
        
        return TRUE;
    }

	if( AnscEqualString(ParamName, "Mode", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->mode;
        
        return TRUE;
    }

	if( AnscEqualString(ParamName, "Band", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->band;
        
        return TRUE;
    }
	
	if( AnscEqualString(ParamName, "SecurityMode", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->sec_mode;
        
        return TRUE;
    }

	if( AnscEqualString(ParamName, "Encryption", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->encry_mode;
        
        return TRUE;
    }

	return FALSE;
}

ULONG
ExtenderSSID_GetParamStringValue
	(
		ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
	struct ExtSSID               *pMyObject     = (struct ExtSSID *)hInsContext;

	if( AnscEqualString(ParamName, "SSID", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->ssid) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->ssid);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->ssid)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "BSSID", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->bssid) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->bssid);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->bssid)+1;
            return 1;
        }
        return 0;
    }
	
	return -1;
}


ULONG
ExtenderClient_GetEntryCount
	(
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_WIFI            pMyObject     = (PCOSA_DATAMODEL_WIFI     )g_pCosaBEManager->hWifi;
	struct ExtStatus                *pStatus      = (struct ExtStatus *)hInsContext;
	 
    if (!pMyObject->ext_count)
		return 0;
	
	if (pStatus)
		return pStatus->ext_client_num;
	
	return 0;
}

ANSC_HANDLE
ExtenderClient_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
	struct ExtStatus                *pStatus      = (struct ExtStatus *)hInsContext;

    if (nIndex < pStatus->ext_client_num)
    {
        *pInsNumber  = nIndex + 1;

        return &pStatus->clients[nIndex];
    }

    return NULL; /* return the handle */
}

BOOL
ExtenderClient_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return TRUE;
}

ULONG
ExtenderClient_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

BOOL
ExtenderClient_GetParamUlongValue
	(
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
	struct ExtClient               *pMyObject     = (struct ExtClient *)hInsContext;

	if( AnscEqualString(ParamName, "Interface", TRUE))
    {
        /* collect value */
        *puLong = pMyObject->inf;
        
        return TRUE;
    }

	return FALSE;
}

ULONG
ExtenderClient_GetParamStringValue
	(
		ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
	struct ExtClient              *pMyObject     = (struct ExtClient *)hInsContext;

	if( AnscEqualString(ParamName, "Name", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->name) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->name)+1;
            return 1;
        }
        return 0;
    }

	if( AnscEqualString(ParamName, "MACAddress", TRUE))
    {
		 /* collect value */
        if ( AnscSizeOfString(pMyObject->mac) < *pUlSize)
        {
            AnscCopyString(pValue, pMyObject->mac);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pMyObject->mac)+1;
            return 1;
        }
        return 0;
    }
	
	return -1;
}
