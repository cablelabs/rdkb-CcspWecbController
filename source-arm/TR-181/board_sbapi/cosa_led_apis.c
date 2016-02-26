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

    module: cosa_led_apis.c

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

        01/11/2011    initial revision.

**************************************************************************/
#include "cosa_apis.h"
#include "plugin_main_apis.h"
#include "cosa_led_apis.h"
#include "syscfg/syscfg.h"
#include <ctype.h>
#include "wecb_log.h"
#include "wecb_common.h"
#include "autoconf.h"

ANSC_STATUS    
CosaDmlLED_GetPanelEntries(ULONG *PanelNumberOfEntries)
{
    /* Validate received param */
    if( NULL == PanelNumberOfEntries )
    {
        log_printf(LOG_ERR, "unexpected parameters\n");
        return ANSC_STATUS_FAILURE;
    }

    /* Now Hardcoded due to hal API dependency */
    *PanelNumberOfEntries = 2;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS    
CosaDmlLED_GetRJ45Entries(ULONG *RJ45NumberOfEntries)
{
    /* Validate received param */
    if( NULL == RJ45NumberOfEntries )
    {
        log_printf(LOG_ERR, "unexpected parameters\n");
        return ANSC_STATUS_FAILURE;
    }

    /* Now Hardcoded due to hal API dependency */
    *RJ45NumberOfEntries = 3;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS    
CosaDmlLED_GetPanelLEDConfiguration( PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg,
                                     INT                         PanelLEDIndex )
{
    char AliasNVMBuffer[64]   = { 0 },
    AliasNVMID[64]       = { 0 }; 

    /* Validate received param */
    if( NULL == pPanelLEDCfg )
    {
        log_printf(LOG_ERR, "unexpected parameters\n");
        return ANSC_STATUS_FAILURE;
    }

    /* 
    * Set all the params based on index - Now Hardcoded due to hal API dependency.
    * so you need to call proper HAL API to ensure get/set functionality
    */
    pPanelLEDCfg->bEnable    = TRUE;
    sprintf( pPanelLEDCfg->Status,"%s","Up" );

    /* 
    * Get Alias value From NVRAM. If fails then need to assign default alias 
    * name  
    */
    sprintf( AliasNVMID, "Panel_Alias_%d", PanelLEDIndex + 1 );

    if( 0 == syscfg_get( NULL, AliasNVMID, AliasNVMBuffer, sizeof(AliasNVMBuffer)) )
    {
        sprintf( pPanelLEDCfg->Alias, "%s", AliasNVMBuffer );
    }
    else
    {
        /* Set Alias Values into DB */
        sprintf( pPanelLEDCfg->Alias, "PanelLED_%d", PanelLEDIndex + 1 );
        
        if ( syscfg_set( NULL, AliasNVMID, pPanelLEDCfg->Alias ) != 0 ) 
        {
            log_printf(LOG_ERR, "syscfg_set failed\n");
        }
        else 
        {
            if ( syscfg_commit( ) != 0 ) 
            {
                log_printf(LOG_ERR, "syscfg_commit failed\n");
            }
        }
    }

    sprintf( pPanelLEDCfg->Name, "Device.X_COMCAST-COM_LED.Panel.%d.", PanelLEDIndex + 1 );
    pPanelLEDCfg->RedDiscreteValue         = 100;
    pPanelLEDCfg->GreenDiscreteValue       = 120;    
    pPanelLEDCfg->BlueDiscreteValue        = 140;
    pPanelLEDCfg->BrightnessValue          = 50;

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS    
CosaDmlLED_SetPanelLEDConfiguration( PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg,
                                     INT                         PanelLEDIndex )
{
    char AliasNVMID[64] = { 0 }; 

    /* Validate received param */
    if( NULL == pPanelLEDCfg )
    {
        log_printf(LOG_ERR, "unexpected parameters\n");
        return ANSC_STATUS_FAILURE;
    }

    /* Set all the params based on index - Need to call HAL API for reflecting into h/w level */

    /* Set Alias Values into DB */
    sprintf( AliasNVMID, "Panel_Alias_%d", PanelLEDIndex + 1 );

    if ( syscfg_set( NULL, AliasNVMID, pPanelLEDCfg->Alias ) != 0 ) 
    {
        log_printf(LOG_ERR, "syscfg_set failed\n");
    }
    else 
    {
        if ( syscfg_commit( ) != 0 ) 
        {
            log_printf(LOG_ERR, "syscfg_commit failed\n");
        }
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS    
CosaDmlLED_GetRJ45LEDConfiguration( PCOSA_DML_RJ45_LED_CONFIG  pRJ45LEDCfg,
                                    INT                        RJ45LEDIndex )
{
    char AliasNVMBuffer[64]   = { 0 },
    AliasNVMID[64]       = { 0 }; 

    /* Validate received param */
    if( NULL == pRJ45LEDCfg )
    {
    log_printf(LOG_ERR, "unexpected parameters\n");
    return ANSC_STATUS_FAILURE;
    }

    /* 
    * Fill all the params based on index - Now Hardcoded due to hal API dependency. 
    * so you need to call proper HAL API to ensure get/set functionality
    */
    pRJ45LEDCfg->bEnable    = TRUE;
    sprintf( pRJ45LEDCfg->Status,"%s","Up" );

    /* 
    * Get Alias value From NVRAM. If fails then need to assign default alias 
    * name  
    */
    sprintf( AliasNVMID, "RJ45_Alias_%d", RJ45LEDIndex + 1 );

    if( 0 == syscfg_get( NULL, AliasNVMID, AliasNVMBuffer, sizeof(AliasNVMBuffer)) )
    {
        sprintf( pRJ45LEDCfg->Alias, "%s", AliasNVMBuffer );
    }
    else
    {
        /* Set Alias Values into DB */
        sprintf( pRJ45LEDCfg->Alias, "RJ45LED_%d", RJ45LEDIndex + 1 );
        
        if ( syscfg_set( NULL, AliasNVMID, pRJ45LEDCfg->Alias ) != 0 ) 
        {
            log_printf(LOG_ERR, "syscfg_set failed\n");
        }
        else 
        {
            if ( syscfg_commit( ) != 0 ) 
            {
                log_printf(LOG_ERR, "syscfg_commit failed\n");
            }
        }
    }

    sprintf( pRJ45LEDCfg->Name, "Device.X_COMCAST-COM_LED.RJ45.%d.", RJ45LEDIndex + 1 );

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS    
CosaDmlLED_SetRJ45LEDConfiguration( PCOSA_DML_RJ45_LED_CONFIG  pRJ45LEDCfg,
     INT  RJ45LEDIndex )
{
    char AliasNVMID[64] = { 0 }; 

    /* Validate received param */
    if( NULL == pRJ45LEDCfg )
    {
        log_printf(LOG_ERR, "unexpected parameters\n");
        return ANSC_STATUS_FAILURE;
    }

    /* Set all the params based on index - Need to call HAL API for reflecting into h/w level */

    /* Set Alias Values into DB */
    sprintf( AliasNVMID, "RJ45_Alias_%d", RJ45LEDIndex + 1 );

        if ( syscfg_set( NULL, AliasNVMID, pRJ45LEDCfg->Alias ) != 0 ) 
    {
        log_printf(LOG_ERR, "syscfg_set failed\n");
    }
    else 
    {
        if ( syscfg_commit( ) != 0 ) 
        {
            log_printf(LOG_ERR, "syscfg_commit failed\n");
        }
    }
    
    return ANSC_STATUS_SUCCESS;
}
