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

    module: cosa_led_apis.h

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


#ifndef  _COSA_LED_APIS_H
#define  _COSA_LED_APIS_H

#include "cosa_apis.h"

/*
 *  Dynamic portion of Range Extenders LED Configuration
 */
struct
_COSA_DML_LED_CONFIG
{
    ULONG                           PanelNumberOfEntries;
    ULONG                           RJ45NumberOfEntries;
}_struct_pack_;

typedef  struct _COSA_DML_LED_CONFIG COSA_DML_LED_CONFIG,  *PCOSA_DML_LED_CONFIG;

struct
_COSA_DML_PANEL_LED_CONFIG
{
    ULONG                           InstanceNumber;
    BOOLEAN                         bEnable;
    char                            Status[24];
    char                            Alias[64];    
    char                            Name[64];        
    ULONG                           RedDiscreteValue;
    ULONG                           GreenDiscreteValue;
    ULONG                           BlueDiscreteValue;
    ULONG                           BrightnessValue;
}_struct_pack_;

typedef  struct _COSA_DML_PANEL_LED_CONFIG COSA_DML_PANEL_LED_CONFIG,  *PCOSA_DML_PANEL_LED_CONFIG;

struct
_COSA_DML_RJ45_LED_CONFIG
{
    ULONG                           InstanceNumber;
    BOOLEAN                         bEnable;
    char                            Status[24];
    char                            Alias[64];    
    char                            Name[64];        
}_struct_pack_;

typedef  struct _COSA_DML_RJ45_LED_CONFIG COSA_DML_RJ45_LED_CONFIG,  *PCOSA_DML_RJ45_LED_CONFIG;

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

ANSC_STATUS    
CosaDmlLED_GetPanelEntries(ULONG *PanelNumberOfEntries);

ANSC_STATUS    
CosaDmlLED_GetRJ45Entries(ULONG *RJ45NumberOfEntries);

ANSC_STATUS    
CosaDmlLED_GetPanelLEDConfiguration(PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg,
                                    INT                         PanelLEDIndex );
ANSC_STATUS    
CosaDmlLED_SetPanelLEDConfiguration( PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg,
                                     INT                         PanelLEDIndex );
ANSC_STATUS    
CosaDmlLED_GetRJ45LEDConfiguration( PCOSA_DML_RJ45_LED_CONFIG  pRJ45LEDCfg,
                                    INT                        RJ45LEDIndex );
ANSC_STATUS    
CosaDmlLED_SetRJ45LEDConfiguration( PCOSA_DML_RJ45_LED_CONFIG  pRJ45LEDCfg,
                                    INT                        RJ45LEDIndex );
#endif
