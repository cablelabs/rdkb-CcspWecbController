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

    module: cosa_led_internal.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    copyright:

        Cisco Systems, Inc.
        All Rights Reserved.

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  CosaLEDCreate
        *  CosaLEDInitialize
        *  CosaLEDReInitialize        
        *  CosaLEDRemove
    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        Richard Yang

    -------------------------------------------------------------------

    revision:

        01/11/2011    initial revision.

**************************************************************************/

#include "cosa_apis.h"
#include "cosa_led_internal.h"
#include "plugin_main_apis.h"

extern void *                       g_pDslhDmlAgent;

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        CosaLEDCreate
            (
            );

    description:

        This function constructs cosa wifi object and return handle.

    argument:  

    return:     newly created wifi object.

**********************************************************************/

ANSC_HANDLE
CosaLEDCreate
    (
        VOID
    )
{
    ANSC_STATUS                    returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_LED            pMyObject    = (PCOSA_DATAMODEL_LED)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PCOSA_DATAMODEL_LED)AnscAllocateMemory(sizeof(COSA_DATAMODEL_LED));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = COSA_DATAMODEL_LED_OID;
    pMyObject->Create            = CosaLEDCreate;
    pMyObject->Remove            = CosaLEDRemove;
    pMyObject->Initialize        = CosaLEDInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaLEDInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa wifi object and return handle.

    argument:    ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaLEDInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_LED             pMyObject     = (PCOSA_DATAMODEL_LED)hThisObject;
    PCOSA_DML_LED                   pLED          = (PCOSA_DML_LED          )NULL;
    PCOSA_DML_PANEL_LED_CONFIG      pPanelLEDCfg  = (PCOSA_DML_PANEL_LED_CONFIG          )NULL;
    PCOSA_DML_RJ45_LED_CONFIG       pRJ45LEDCfg   = (PCOSA_DML_RJ45_LED_CONFIG          )NULL;
    ULONG                           uLoopIndex      = 0,
                                    PanelNumberOfEntries = 0,
                                    RJ45NumberOfEntries = 0;

    /* Range Extender's LED Indicator Configruation - Allocate Memory */
    pLED = (PCOSA_DML_LED)AnscAllocateMemory( sizeof( COSA_DML_LED ) );

    /* Return fail when allocation fail case */
    if ( NULL == pLED )
    {
        return ANSC_STATUS_RESOURCES;
    }

    /* Getting LED Object Configuration */
    memset( pLED, 0, sizeof( COSA_DML_LED ) );

    CosaDmlLED_GetPanelEntries( &PanelNumberOfEntries );
    pLED->LEDCfg.PanelNumberOfEntries = PanelNumberOfEntries;

    CosaDmlLED_GetRJ45Entries( &RJ45NumberOfEntries );
    pLED->LEDCfg.RJ45NumberOfEntries = RJ45NumberOfEntries;

    /* Range Extender's Panel LED Indicator Configruation - Allocate Memory */
    if( 0 < pLED->LEDCfg.PanelNumberOfEntries )
    {
        pPanelLEDCfg = (PCOSA_DML_PANEL_LED_CONFIG)AnscAllocateMemory( \
                             pLED->LEDCfg.PanelNumberOfEntries * \
                             sizeof( COSA_DML_PANEL_LED_CONFIG ) );
        
        /* Free previously allocated memory when allocation fail case */
        if ( NULL == pPanelLEDCfg )
        {
            AnscFreeMemory( pLED );
            pLED = NULL;
            return ANSC_STATUS_RESOURCES;
        }

        memset( pPanelLEDCfg, 
                0, 
                ( pLED->LEDCfg.PanelNumberOfEntries * \
                  sizeof( COSA_DML_PANEL_LED_CONFIG ) ) );

        /* Getting Panel LED Object Configuration based on index */
        for ( uLoopIndex = 0; 
              uLoopIndex < pLED->LEDCfg.PanelNumberOfEntries;
              ++uLoopIndex ) 
        {
           /* Instance number always from 1 */
           pPanelLEDCfg[uLoopIndex].InstanceNumber = uLoopIndex + 1;
           CosaDmlLED_GetPanelLEDConfiguration( &pPanelLEDCfg[uLoopIndex],
                                                  uLoopIndex );
          }
    }

    /* Range Extender's RJ45 LED Indicator Configruation - Allocate Memory */
    if( 0 < pLED->LEDCfg.RJ45NumberOfEntries )
    {
        pRJ45LEDCfg = (PCOSA_DML_RJ45_LED_CONFIG)AnscAllocateMemory( \
                             pLED->LEDCfg.RJ45NumberOfEntries * \
                             sizeof( COSA_DML_RJ45_LED_CONFIG ) );
        
        /* Free previously allocated memory when allocation fail case */
        if ( NULL == pRJ45LEDCfg )
        {
            AnscFreeMemory( pPanelLEDCfg );
            pPanelLEDCfg = NULL;
            
            AnscFreeMemory( pLED );
            pLED = NULL;

            return ANSC_STATUS_RESOURCES;
        }

        memset( pRJ45LEDCfg, 
                0, 
                ( pLED->LEDCfg.RJ45NumberOfEntries * \
                  sizeof( COSA_DML_RJ45_LED_CONFIG ) ) );

        /* Getting RJ45 LED Object Configuration based on index */
        for ( uLoopIndex = 0; 
              uLoopIndex < pLED->LEDCfg.RJ45NumberOfEntries;
              ++uLoopIndex ) 
        {
           /* Instance number always from 1 */
           pRJ45LEDCfg[uLoopIndex].InstanceNumber = uLoopIndex + 1;
           CosaDmlLED_GetRJ45LEDConfiguration( &pRJ45LEDCfg[uLoopIndex],
                                                uLoopIndex );
        }
    }

    /* Assign allocated address to base configuration */
    pLED->pRJ45LEDCfg   = pRJ45LEDCfg;
    pLED->pPanelLEDCfg  = pPanelLEDCfg;
    pMyObject->pLED     = pLED;

EXIT:
    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaLEDReInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa wifi object and return handle.

    argument:    ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
CosaLEDReInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    
EXIT:
    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        CosaLEDRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa wifi object and return handle.

    argument:    ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/
ANSC_STATUS
CosaLEDRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_LED             pMyObject    = (PCOSA_DATAMODEL_LED)hThisObject;
    PCOSA_DML_LED                   pLED         = (PCOSA_DML_LED)NULL;

    /* Free Allocated Memory for LED object */
    pLED = pMyObject->pLED;

    if( NULL != pLED )
    {
        AnscFreeMemory((ANSC_HANDLE)pLED->pRJ45LEDCfg);
        AnscFreeMemory((ANSC_HANDLE)pLED->pPanelLEDCfg);
        AnscFreeMemory((ANSC_HANDLE)pLED);
    }

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}
