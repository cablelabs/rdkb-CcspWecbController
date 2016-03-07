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

#ifndef  _COSA_LED_INTERNAL_H
#define  _COSA_LED_INTERNAL_H
#include "cosa_led_apis.h"
#include "poam_irepfo_interface.h"
#include "sys_definitions.h"

/* Collection */
typedef  struct
_COSA_DML_LED
{
     COSA_DML_LED_CONFIG        	LEDCfg;
	 PCOSA_DML_PANEL_LED_CONFIG 	pPanelLEDCfg;
	 PCOSA_DML_RJ45_LED_CONFIG  	pRJ45LEDCfg;
	 BOOLEAN						bPanelLEDCfgChanged;
	 BOOLEAN						bRJ45LEDCfgChanged;
}
COSA_DML_LED, *PCOSA_DML_LED;

#define  COSA_DATAMODEL_LED_CLASS_CONTENT                                                  \
    /* duplication of the base object class content */                                      \
    COSA_BASE_CONTENT                                                                       \
    /* start of LED object class content */                                                \
	PCOSA_DML_LED					pLED;

typedef  struct
_COSA_DATAMODEL_LED                                               
{
	COSA_DATAMODEL_LED_CLASS_CONTENT
}
COSA_DATAMODEL_LED,  *PCOSA_DATAMODEL_LED;

/*
    Standard function declaration 
*/
ANSC_HANDLE
CosaLEDCreate
    (
        VOID
    );

ANSC_STATUS
CosaLEDInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
CosaLEDReInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
CosaLEDRemove
    (
        ANSC_HANDLE                 hThisObject
    );
#endif 
