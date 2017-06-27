/*******************************************************************************
* File Name: Blinker_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Blinker.h"

static Blinker_backupStruct Blinker_backup;


/*******************************************************************************
* Function Name: Blinker_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  Blinker_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void Blinker_SaveConfig(void) 
{

    #if(!Blinker_UsingFixedFunction)
        #if(!Blinker_PWMModeIsCenterAligned)
            Blinker_backup.PWMPeriod = Blinker_ReadPeriod();
        #endif /* (!Blinker_PWMModeIsCenterAligned) */
        Blinker_backup.PWMUdb = Blinker_ReadCounter();
        #if (Blinker_UseStatus)
            Blinker_backup.InterruptMaskValue = Blinker_STATUS_MASK;
        #endif /* (Blinker_UseStatus) */

        #if(Blinker_DeadBandMode == Blinker__B_PWM__DBM_256_CLOCKS || \
            Blinker_DeadBandMode == Blinker__B_PWM__DBM_2_4_CLOCKS)
            Blinker_backup.PWMdeadBandValue = Blinker_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(Blinker_KillModeMinTime)
             Blinker_backup.PWMKillCounterPeriod = Blinker_ReadKillTime();
        #endif /* (Blinker_KillModeMinTime) */

        #if(Blinker_UseControl)
            Blinker_backup.PWMControlRegister = Blinker_ReadControlRegister();
        #endif /* (Blinker_UseControl) */
    #endif  /* (!Blinker_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Blinker_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  Blinker_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void Blinker_RestoreConfig(void) 
{
        #if(!Blinker_UsingFixedFunction)
            #if(!Blinker_PWMModeIsCenterAligned)
                Blinker_WritePeriod(Blinker_backup.PWMPeriod);
            #endif /* (!Blinker_PWMModeIsCenterAligned) */

            Blinker_WriteCounter(Blinker_backup.PWMUdb);

            #if (Blinker_UseStatus)
                Blinker_STATUS_MASK = Blinker_backup.InterruptMaskValue;
            #endif /* (Blinker_UseStatus) */

            #if(Blinker_DeadBandMode == Blinker__B_PWM__DBM_256_CLOCKS || \
                Blinker_DeadBandMode == Blinker__B_PWM__DBM_2_4_CLOCKS)
                Blinker_WriteDeadTime(Blinker_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(Blinker_KillModeMinTime)
                Blinker_WriteKillTime(Blinker_backup.PWMKillCounterPeriod);
            #endif /* (Blinker_KillModeMinTime) */

            #if(Blinker_UseControl)
                Blinker_WriteControlRegister(Blinker_backup.PWMControlRegister);
            #endif /* (Blinker_UseControl) */
        #endif  /* (!Blinker_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: Blinker_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  Blinker_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void Blinker_Sleep(void) 
{
    #if(Blinker_UseControl)
        if(Blinker_CTRL_ENABLE == (Blinker_CONTROL & Blinker_CTRL_ENABLE))
        {
            /*Component is enabled */
            Blinker_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            Blinker_backup.PWMEnableState = 0u;
        }
    #endif /* (Blinker_UseControl) */

    /* Stop component */
    Blinker_Stop();

    /* Save registers configuration */
    Blinker_SaveConfig();
}


/*******************************************************************************
* Function Name: Blinker_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  Blinker_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void Blinker_Wakeup(void) 
{
     /* Restore registers values */
    Blinker_RestoreConfig();

    if(Blinker_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        Blinker_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
