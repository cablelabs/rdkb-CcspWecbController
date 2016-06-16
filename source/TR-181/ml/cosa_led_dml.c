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

    module: cosa_led_dml.c

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
#include "cosa_led_dml.h"
#include "cosa_led_internal.h"
#include "plugin_main_apis.h"
#include "wecb_common.h"
#include "cosa_wecb_wrapper.h"

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
/***********************************************************************

 APIs for Object:

    Device.X_COMCAST-COM_LED.

    *  LED_GetParamUlongValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        LED_GetParamUlongValue
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
LED_GetParamUlongValue
(
    ANSC_HANDLE                 hInsContext,
    char*                       ParamName,
    ULONG*                      puLong
)
{
    PCOSA_DATAMODEL_LED             pMyObject    = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                    pLED          = pMyObject->pLED;

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    Device. X_COMCAST-COM_LED.Panel.{i}.

    *  Panel_GetEntryCount
    *  Panel_GetEntry
    *  Panel_IsUpdated
    *  Panel_Synchronize
    *  Panel_GetParamBoolValue
    *  Panel_SetParamBoolValue
    *  Panel_GetParamUlongValue
    *  Panel_SetParamUlongValue
    *  Panel_GetParamStringValue
    *  Panel_SetParamStringValue
    *  Panel_Validate
    *  Panel_Commit
    *  Panel_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
Panel_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Panel_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Panel_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    ULONG                         PanelNumberOfEntries = 0;

    CosaDmlLED_GetPanelEntries( &PanelNumberOfEntries );
    pLED->LEDCfg.PanelNumberOfEntries = PanelNumberOfEntries;

    /* Range Extender's Panel LED Indicator Configruation - Sync */
    if( 0 < pLED->LEDCfg.PanelNumberOfEntries )
    {
        PCOSA_DML_PANEL_LED_CONFIG    pPanelLEDCfg  = (PCOSA_DML_PANEL_LED_CONFIG         )NULL;
        ULONG                         uLoopIndex;

        /* Free previously allocated memory for Panel LED configuration */
        if( NULL != pLED->pPanelLEDCfg )
        {
            AnscFreeMemory( pLED->pPanelLEDCfg );
            pLED->pPanelLEDCfg = NULL;
        }

        pPanelLEDCfg = (PCOSA_DML_PANEL_LED_CONFIG)AnscAllocateMemory( \
                             pLED->LEDCfg.PanelNumberOfEntries * \
                             sizeof( COSA_DML_PANEL_LED_CONFIG ) );
        
        /* Return fail code when allocation fail case */
        if ( NULL == pPanelLEDCfg )
        {
            return 1;
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

        /* Assign the allocated address to base structure */
        pLED->pPanelLEDCfg = pPanelLEDCfg;
    }
    else
    {
        /* Free previously allocated memory for Panel LED configuration */
        if( NULL != pLED->pPanelLEDCfg )
        {
            AnscFreeMemory( pLED->pPanelLEDCfg );
            pLED->pPanelLEDCfg = NULL;
        }
    }

    return 0;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Panel_GetEntryCount
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
Panel_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    
    return pLED->LEDCfg.PanelNumberOfEntries;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Panel_GetEntry
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
Panel_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    
    if ( pLED && nIndex < pLED->LEDCfg.PanelNumberOfEntries )
    {
        *pInsNumber = pLED->pPanelLEDCfg[ nIndex ].InstanceNumber;

        return &pLED->pPanelLEDCfg[ nIndex ];
    }
    
    return NULL; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_GetParamBoolValue
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
Panel_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* collect value */
        *pBool = pPanelLEDCfg->bEnable;
        
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_SetParamBoolValue
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
Panel_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        if ( pPanelLEDCfg->bEnable == bValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pPanelLEDCfg->bEnable     = bValue;
        pLED->bPanelLEDCfgChanged = TRUE; 
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_GetParamUlongValue
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
Panel_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Red", TRUE))
    {
        /* collect value */
        *puLong = pPanelLEDCfg->RedDiscreteValue;

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Green", TRUE))
    {
        /* collect value */
        *puLong = pPanelLEDCfg->GreenDiscreteValue;

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Blue", TRUE))
    {
        /* collect value */
        *puLong = pPanelLEDCfg->BlueDiscreteValue;

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Brightness", TRUE))
    {
        /* collect value */
        *puLong = pPanelLEDCfg->BrightnessValue;

        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_SetParamUlongValue
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
Panel_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    /* check the parameter name and set the corresponding value */

    if( AnscEqualString(ParamName, "Red", TRUE))
    {
        if ( pPanelLEDCfg->RedDiscreteValue == uValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pPanelLEDCfg->RedDiscreteValue  = uValue;
        pLED->bPanelLEDCfgChanged         = TRUE; 

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Green", TRUE))
    {
        if ( pPanelLEDCfg->GreenDiscreteValue == uValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pPanelLEDCfg->GreenDiscreteValue = uValue;
        pLED->bPanelLEDCfgChanged          = TRUE; 

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Blue", TRUE))
    {
        if ( pPanelLEDCfg->BlueDiscreteValue == uValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pPanelLEDCfg->BlueDiscreteValue = uValue;
        pLED->bPanelLEDCfgChanged         = TRUE; 

        return TRUE;
    }

    if( AnscEqualString(ParamName, "Brightness", TRUE))
    {
        if ( pPanelLEDCfg->BrightnessValue == uValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pPanelLEDCfg->BrightnessValue = uValue;
        pLED->bPanelLEDCfgChanged       = TRUE; 

        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Panel_GetParamStringValue
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
Panel_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;


    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pPanelLEDCfg->Status) < *pUlSize)
        {
            AnscCopyString(pValue, pPanelLEDCfg->Status);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pPanelLEDCfg->Status)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pPanelLEDCfg->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pPanelLEDCfg->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pPanelLEDCfg->Alias)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pPanelLEDCfg->Name) < *pUlSize)
        {
            AnscCopyString(pValue, pPanelLEDCfg->Name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pPanelLEDCfg->Name)+1;
            return 1;
        }
        return 0;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_SetParamStringValue
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
Panel_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;
   
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pPanelLEDCfg->Alias, pString );
        pLED->bPanelLEDCfgChanged = TRUE;
        
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Panel_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                        pReturnParamName,
                ULONG*                        puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                        pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                        puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
Panel_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                        pReturnParamName,
        ULONG*                        puLength
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Panel_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Panel_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    /* Set the Panel LED Current Configuration */
    if ( TRUE == pLED->bPanelLEDCfgChanged )
    {
        CosaDmlLED_SetPanelLEDConfiguration( pPanelLEDCfg,
                                             pPanelLEDCfg->InstanceNumber - 1 );

        pLED->bPanelLEDCfgChanged = FALSE;
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Panel_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Panel_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_PANEL_LED_CONFIG  pPanelLEDCfg  = ( PCOSA_DML_PANEL_LED_CONFIG )hInsContext;

    CosaDmlLED_GetPanelLEDConfiguration( pPanelLEDCfg,
                                         pPanelLEDCfg->InstanceNumber - 1 );
    
    pLED->bPanelLEDCfgChanged = FALSE;

    return ANSC_STATUS_SUCCESS;
}

/***********************************************************************

 APIs for Object:

    Device. X_COMCAST-COM_LED.RJ45.{i}.

    *  RJ45_GetEntryCount
    *  RJ45_GetEntry
    *  RJ45_IsUpdated
    *  RJ45_Synchronize
    *  RJ45_GetParamBoolValue
    *  RJ45_SetParamBoolValue
    *  RJ45_GetParamStringValue
    *  RJ45_SetParamStringValue
    *  RJ45_Validate
    *  RJ45_Commit
    *  RJ45_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        RJ45_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
RJ45_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        RJ45_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
RJ45_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    ULONG                         RJ45NumberOfEntries = 0;

    CosaDmlLED_GetRJ45Entries( &RJ45NumberOfEntries );
    pLED->LEDCfg.RJ45NumberOfEntries = RJ45NumberOfEntries;
    
    /* Range Extender's RJ45 LED Indicator Configruation - Allocate Memory */
    if( 0 < pLED->LEDCfg.RJ45NumberOfEntries )
    {
        PCOSA_DML_RJ45_LED_CONFIG    pRJ45LEDCfg   = (PCOSA_DML_RJ45_LED_CONFIG            )NULL;
        ULONG                        uLoopIndex      = 0;

        /* Free previously allocated memory for Panel LED configuration */
        if( NULL != pLED->pRJ45LEDCfg )
        {
            AnscFreeMemory( pLED->pRJ45LEDCfg );
            pLED->pRJ45LEDCfg = NULL;
        }

        pRJ45LEDCfg = (PCOSA_DML_RJ45_LED_CONFIG)AnscAllocateMemory( \
                             pLED->LEDCfg.RJ45NumberOfEntries * \
                             sizeof( COSA_DML_RJ45_LED_CONFIG ) );
        
        /* Return error value when allocation fail case */
        if ( NULL == pRJ45LEDCfg )
        {
            return 1;
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

        /* Assign the allocated address to base structure */
        pLED->pRJ45LEDCfg = pRJ45LEDCfg;
    }
    else
    {
        /* Free previously allocated memory for Panel LED configuration */
        if( NULL != pLED->pRJ45LEDCfg )
        {
            AnscFreeMemory( pLED->pRJ45LEDCfg );
            pLED->pRJ45LEDCfg = NULL;
        }
    }

    return 0;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        RJ45_GetEntryCount
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
RJ45_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    
    return pLED->LEDCfg.RJ45NumberOfEntries;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        RJ45_GetEntry
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
RJ45_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
 
    if ( pLED && nIndex < pLED->LEDCfg.RJ45NumberOfEntries )
    {
        *pInsNumber = pLED->pRJ45LEDCfg[ nIndex ].InstanceNumber;

        return &pLED->pRJ45LEDCfg[ nIndex ];
    }
    
    return NULL; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        RJ45_GetParamBoolValue
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
RJ45_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* collect value */
        *pBool = pRJ45LEDCfg->bEnable;
        
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        RJ45_SetParamBoolValue
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
RJ45_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        if ( pRJ45LEDCfg->bEnable == bValue )
        {
            return  TRUE;
        }

        /* save update to backup */
        pRJ45LEDCfg->bEnable     = bValue;
        pLED->bRJ45LEDCfgChanged = TRUE; 
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        RJ45_GetParamStringValue
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
RJ45_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;


    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pRJ45LEDCfg->Status) < *pUlSize)
        {
            AnscCopyString(pValue, pRJ45LEDCfg->Status);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pRJ45LEDCfg->Status)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pRJ45LEDCfg->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pRJ45LEDCfg->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pRJ45LEDCfg->Alias)+1;
            return 1;
        }
        return 0;
    }

    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        /* collect value */
        if ( AnscSizeOfString(pRJ45LEDCfg->Name) < *pUlSize)
        {
            AnscCopyString(pValue, pRJ45LEDCfg->Name);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pRJ45LEDCfg->Name)+1;
            return 1;
        }
        return 0;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        RJ45_SetParamStringValue
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
RJ45_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;
   
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pRJ45LEDCfg->Alias , pString );
        pLED->bRJ45LEDCfgChanged = TRUE; 
        
        return TRUE;
    }

    /* CcspWecbTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        RJ45_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                        pReturnParamName,
                ULONG*                        puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                        pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                        puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
RJ45_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                        pReturnParamName,
        ULONG*                        puLength
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        RJ45_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
RJ45_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;

    /* Set the RJ45 LED Current Configuration */
    if ( TRUE == pLED->bRJ45LEDCfgChanged )
    {
        CosaDmlLED_SetRJ45LEDConfiguration( pRJ45LEDCfg,
                                            pRJ45LEDCfg->InstanceNumber - 1 );

        pLED->bRJ45LEDCfgChanged = FALSE;
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        RJ45_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:    ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
RJ45_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PCOSA_DATAMODEL_LED         pMyObject     = (PCOSA_DATAMODEL_LED)g_pCosaBEManager->hLED;
    PCOSA_DML_LED                pLED           = pMyObject->pLED;
    PCOSA_DML_RJ45_LED_CONFIG   pRJ45LEDCfg   = ( PCOSA_DML_RJ45_LED_CONFIG )hInsContext;

    CosaDmlLED_GetRJ45LEDConfiguration( pRJ45LEDCfg,
                                        pRJ45LEDCfg->InstanceNumber - 1 );
    
    pLED->bRJ45LEDCfgChanged = FALSE;

    return ANSC_STATUS_SUCCESS;
}
