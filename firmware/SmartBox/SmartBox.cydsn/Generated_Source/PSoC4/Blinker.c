/*******************************************************************************
* File Name: Blinker.c
* Version 3.30
*
* Description:
*  The PWM User Module consist of an 8 or 16-bit counter with two 8 or 16-bit
*  comparitors. Each instance of this user module is capable of generating
*  two PWM outputs with the same period. The pulse width is selectable between
*  1 and 255/65535. The period is selectable between 2 and 255/65536 clocks.
*  The compare value output may be configured to be active when the present
*  counter is less than or less than/equal to the compare value.
*  A terminal count output is also provided. It generates a pulse one clock
*  width wide when the counter is equal to zero.
*
* Note:
*
*******************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "Blinker.h"

/* Error message for removed <resource> through optimization */
#ifdef Blinker_PWMUDB_genblk1_ctrlreg__REMOVED
    #error PWM_v3_30 detected with a constant 0 for the enable or \
         constant 1 for reset. This will prevent the component from operating.
#endif /* Blinker_PWMUDB_genblk1_ctrlreg__REMOVED */

uint8 Blinker_initVar = 0u;


/*******************************************************************************
* Function Name: Blinker_Start
********************************************************************************
*
* Summary:
*  The start function initializes the pwm with the default values, the
*  enables the counter to begin counting.  It does not enable interrupts,
*  the EnableInt command should be called if interrupt generation is required.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  Blinker_initVar: Is modified when this function is called for the
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void Blinker_Start(void) 
{
    /* If not Initialized then initialize all required hardware and software */
    if(Blinker_initVar == 0u)
    {
        Blinker_Init();
        Blinker_initVar = 1u;
    }
    Blinker_Enable();

}


/*******************************************************************************
* Function Name: Blinker_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the
*  customizer of the component placed onto schematic. Usually called in
*  Blinker_Start().
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Blinker_Init(void) 
{
    #if (Blinker_UsingFixedFunction || Blinker_UseControl)
        uint8 ctrl;
    #endif /* (Blinker_UsingFixedFunction || Blinker_UseControl) */

    #if(!Blinker_UsingFixedFunction)
        #if(Blinker_UseStatus)
            /* Interrupt State Backup for Critical Region*/
            uint8 Blinker_interruptState;
        #endif /* (Blinker_UseStatus) */
    #endif /* (!Blinker_UsingFixedFunction) */

    #if (Blinker_UsingFixedFunction)
        /* You are allowed to write the compare value (FF only) */
        Blinker_CONTROL |= Blinker_CFG0_MODE;
        #if (Blinker_DeadBand2_4)
            Blinker_CONTROL |= Blinker_CFG0_DB;
        #endif /* (Blinker_DeadBand2_4) */

        ctrl = Blinker_CONTROL3 & ((uint8 )(~Blinker_CTRL_CMPMODE1_MASK));
        Blinker_CONTROL3 = ctrl | Blinker_DEFAULT_COMPARE1_MODE;

         /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
        Blinker_RT1 &= ((uint8)(~Blinker_RT1_MASK));
        Blinker_RT1 |= Blinker_SYNC;

        /*Enable DSI Sync all all inputs of the PWM*/
        Blinker_RT1 &= ((uint8)(~Blinker_SYNCDSI_MASK));
        Blinker_RT1 |= Blinker_SYNCDSI_EN;

    #elif (Blinker_UseControl)
        /* Set the default compare mode defined in the parameter */
        ctrl = Blinker_CONTROL & ((uint8)(~Blinker_CTRL_CMPMODE2_MASK)) &
                ((uint8)(~Blinker_CTRL_CMPMODE1_MASK));
        Blinker_CONTROL = ctrl | Blinker_DEFAULT_COMPARE2_MODE |
                                   Blinker_DEFAULT_COMPARE1_MODE;
    #endif /* (Blinker_UsingFixedFunction) */

    #if (!Blinker_UsingFixedFunction)
        #if (Blinker_Resolution == 8)
            /* Set FIFO 0 to 1 byte register for period*/
            Blinker_AUX_CONTROLDP0 |= (Blinker_AUX_CTRL_FIFO0_CLR);
        #else /* (Blinker_Resolution == 16)*/
            /* Set FIFO 0 to 1 byte register for period */
            Blinker_AUX_CONTROLDP0 |= (Blinker_AUX_CTRL_FIFO0_CLR);
            Blinker_AUX_CONTROLDP1 |= (Blinker_AUX_CTRL_FIFO0_CLR);
        #endif /* (Blinker_Resolution == 8) */

        Blinker_WriteCounter(Blinker_INIT_PERIOD_VALUE);
    #endif /* (!Blinker_UsingFixedFunction) */

    Blinker_WritePeriod(Blinker_INIT_PERIOD_VALUE);

        #if (Blinker_UseOneCompareMode)
            Blinker_WriteCompare(Blinker_INIT_COMPARE_VALUE1);
        #else
            Blinker_WriteCompare1(Blinker_INIT_COMPARE_VALUE1);
            Blinker_WriteCompare2(Blinker_INIT_COMPARE_VALUE2);
        #endif /* (Blinker_UseOneCompareMode) */

        #if (Blinker_KillModeMinTime)
            Blinker_WriteKillTime(Blinker_MinimumKillTime);
        #endif /* (Blinker_KillModeMinTime) */

        #if (Blinker_DeadBandUsed)
            Blinker_WriteDeadTime(Blinker_INIT_DEAD_TIME);
        #endif /* (Blinker_DeadBandUsed) */

    #if (Blinker_UseStatus || Blinker_UsingFixedFunction)
        Blinker_SetInterruptMode(Blinker_INIT_INTERRUPTS_MODE);
    #endif /* (Blinker_UseStatus || Blinker_UsingFixedFunction) */

    #if (Blinker_UsingFixedFunction)
        /* Globally Enable the Fixed Function Block chosen */
        Blinker_GLOBAL_ENABLE |= Blinker_BLOCK_EN_MASK;
        /* Set the Interrupt source to come from the status register */
        Blinker_CONTROL2 |= Blinker_CTRL2_IRQ_SEL;
    #else
        #if(Blinker_UseStatus)

            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            Blinker_interruptState = CyEnterCriticalSection();
            /* Use the interrupt output of the status register for IRQ output */
            Blinker_STATUS_AUX_CTRL |= Blinker_STATUS_ACTL_INT_EN_MASK;

             /* Exit Critical Region*/
            CyExitCriticalSection(Blinker_interruptState);

            /* Clear the FIFO to enable the Blinker_STATUS_FIFOFULL
                   bit to be set on FIFO full. */
            Blinker_ClearFIFO();
        #endif /* (Blinker_UseStatus) */
    #endif /* (Blinker_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Blinker_Enable
********************************************************************************
*
* Summary:
*  Enables the PWM block operation
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This works only if software enable mode is chosen
*
*******************************************************************************/
void Blinker_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (Blinker_UsingFixedFunction)
        Blinker_GLOBAL_ENABLE |= Blinker_BLOCK_EN_MASK;
        Blinker_GLOBAL_STBY_ENABLE |= Blinker_BLOCK_STBY_EN_MASK;
    #endif /* (Blinker_UsingFixedFunction) */

    /* Enable the PWM from the control register  */
    #if (Blinker_UseControl || Blinker_UsingFixedFunction)
        Blinker_CONTROL |= Blinker_CTRL_ENABLE;
    #endif /* (Blinker_UseControl || Blinker_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: Blinker_Stop
********************************************************************************
*
* Summary:
*  The stop function halts the PWM, but does not change any modes or disable
*  interrupts.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  If the Enable mode is set to Hardware only then this function
*  has no effect on the operation of the PWM
*
*******************************************************************************/
void Blinker_Stop(void) 
{
    #if (Blinker_UseControl || Blinker_UsingFixedFunction)
        Blinker_CONTROL &= ((uint8)(~Blinker_CTRL_ENABLE));
    #endif /* (Blinker_UseControl || Blinker_UsingFixedFunction) */

    /* Globally disable the Fixed Function Block chosen */
    #if (Blinker_UsingFixedFunction)
        Blinker_GLOBAL_ENABLE &= ((uint8)(~Blinker_BLOCK_EN_MASK));
        Blinker_GLOBAL_STBY_ENABLE &= ((uint8)(~Blinker_BLOCK_STBY_EN_MASK));
    #endif /* (Blinker_UsingFixedFunction) */
}

#if (Blinker_UseOneCompareMode)
    #if (Blinker_CompareMode1SW)


        /*******************************************************************************
        * Function Name: Blinker_SetCompareMode
        ********************************************************************************
        *
        * Summary:
        *  This function writes the Compare Mode for the pwm output when in Dither mode,
        *  Center Align Mode or One Output Mode.
        *
        * Parameters:
        *  comparemode:  The new compare mode for the PWM output. Use the compare types
        *                defined in the H file as input arguments.
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void Blinker_SetCompareMode(uint8 comparemode) 
        {
            #if(Blinker_UsingFixedFunction)

                #if(0 != Blinker_CTRL_CMPMODE1_SHIFT)
                    uint8 comparemodemasked = ((uint8)((uint8)comparemode << Blinker_CTRL_CMPMODE1_SHIFT));
                #else
                    uint8 comparemodemasked = comparemode;
                #endif /* (0 != Blinker_CTRL_CMPMODE1_SHIFT) */

                Blinker_CONTROL3 &= ((uint8)(~Blinker_CTRL_CMPMODE1_MASK)); /*Clear Existing Data */
                Blinker_CONTROL3 |= comparemodemasked;

            #elif (Blinker_UseControl)

                #if(0 != Blinker_CTRL_CMPMODE1_SHIFT)
                    uint8 comparemode1masked = ((uint8)((uint8)comparemode << Blinker_CTRL_CMPMODE1_SHIFT)) &
                                                Blinker_CTRL_CMPMODE1_MASK;
                #else
                    uint8 comparemode1masked = comparemode & Blinker_CTRL_CMPMODE1_MASK;
                #endif /* (0 != Blinker_CTRL_CMPMODE1_SHIFT) */

                #if(0 != Blinker_CTRL_CMPMODE2_SHIFT)
                    uint8 comparemode2masked = ((uint8)((uint8)comparemode << Blinker_CTRL_CMPMODE2_SHIFT)) &
                                               Blinker_CTRL_CMPMODE2_MASK;
                #else
                    uint8 comparemode2masked = comparemode & Blinker_CTRL_CMPMODE2_MASK;
                #endif /* (0 != Blinker_CTRL_CMPMODE2_SHIFT) */

                /*Clear existing mode */
                Blinker_CONTROL &= ((uint8)(~(Blinker_CTRL_CMPMODE1_MASK |
                                            Blinker_CTRL_CMPMODE2_MASK)));
                Blinker_CONTROL |= (comparemode1masked | comparemode2masked);

            #else
                uint8 temp = comparemode;
            #endif /* (Blinker_UsingFixedFunction) */
        }
    #endif /* Blinker_CompareMode1SW */

#else /* UseOneCompareMode */

    #if (Blinker_CompareMode1SW)


        /*******************************************************************************
        * Function Name: Blinker_SetCompareMode1
        ********************************************************************************
        *
        * Summary:
        *  This function writes the Compare Mode for the pwm or pwm1 output
        *
        * Parameters:
        *  comparemode:  The new compare mode for the PWM output. Use the compare types
        *                defined in the H file as input arguments.
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void Blinker_SetCompareMode1(uint8 comparemode) 
        {
            #if(0 != Blinker_CTRL_CMPMODE1_SHIFT)
                uint8 comparemodemasked = ((uint8)((uint8)comparemode << Blinker_CTRL_CMPMODE1_SHIFT)) &
                                           Blinker_CTRL_CMPMODE1_MASK;
            #else
                uint8 comparemodemasked = comparemode & Blinker_CTRL_CMPMODE1_MASK;
            #endif /* (0 != Blinker_CTRL_CMPMODE1_SHIFT) */

            #if (Blinker_UseControl)
                Blinker_CONTROL &= ((uint8)(~Blinker_CTRL_CMPMODE1_MASK)); /*Clear existing mode */
                Blinker_CONTROL |= comparemodemasked;
            #endif /* (Blinker_UseControl) */
        }
    #endif /* Blinker_CompareMode1SW */

#if (Blinker_CompareMode2SW)


    /*******************************************************************************
    * Function Name: Blinker_SetCompareMode2
    ********************************************************************************
    *
    * Summary:
    *  This function writes the Compare Mode for the pwm or pwm2 output
    *
    * Parameters:
    *  comparemode:  The new compare mode for the PWM output. Use the compare types
    *                defined in the H file as input arguments.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_SetCompareMode2(uint8 comparemode) 
    {

        #if(0 != Blinker_CTRL_CMPMODE2_SHIFT)
            uint8 comparemodemasked = ((uint8)((uint8)comparemode << Blinker_CTRL_CMPMODE2_SHIFT)) &
                                                 Blinker_CTRL_CMPMODE2_MASK;
        #else
            uint8 comparemodemasked = comparemode & Blinker_CTRL_CMPMODE2_MASK;
        #endif /* (0 != Blinker_CTRL_CMPMODE2_SHIFT) */

        #if (Blinker_UseControl)
            Blinker_CONTROL &= ((uint8)(~Blinker_CTRL_CMPMODE2_MASK)); /*Clear existing mode */
            Blinker_CONTROL |= comparemodemasked;
        #endif /* (Blinker_UseControl) */
    }
    #endif /*Blinker_CompareMode2SW */

#endif /* UseOneCompareMode */


#if (!Blinker_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: Blinker_WriteCounter
    ********************************************************************************
    *
    * Summary:
    *  Writes a new counter value directly to the counter register. This will be
    *  implemented for that currently running period and only that period. This API
    *  is valid only for UDB implementation and not available for fixed function
    *  PWM implementation.
    *
    * Parameters:
    *  counter:  The period new period counter value.
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  The PWM Period will be reloaded when a counter value will be a zero
    *
    *******************************************************************************/
    void Blinker_WriteCounter(uint16 counter) \
                                       
    {
        CY_SET_REG16(Blinker_COUNTER_LSB_PTR, counter);
    }


    /*******************************************************************************
    * Function Name: Blinker_ReadCounter
    ********************************************************************************
    *
    * Summary:
    *  This function returns the current value of the counter.  It doesn't matter
    *  if the counter is enabled or running.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  The current value of the counter.
    *
    *******************************************************************************/
    uint16 Blinker_ReadCounter(void) 
    {
        /* Force capture by reading Accumulator */
        /* Must first do a software capture to be able to read the counter */
        /* It is up to the user code to make sure there isn't already captured data in the FIFO */
          (void)CY_GET_REG8(Blinker_COUNTERCAP_LSB_PTR_8BIT);

        /* Read the data from the FIFO */
        return (CY_GET_REG16(Blinker_CAPTURE_LSB_PTR));
    }

    #if (Blinker_UseStatus)


        /*******************************************************************************
        * Function Name: Blinker_ClearFIFO
        ********************************************************************************
        *
        * Summary:
        *  This function clears all capture data from the capture FIFO
        *
        * Parameters:
        *  None
        *
        * Return:
        *  None
        *
        *******************************************************************************/
        void Blinker_ClearFIFO(void) 
        {
            while(0u != (Blinker_ReadStatusRegister() & Blinker_STATUS_FIFONEMPTY))
            {
                (void)Blinker_ReadCapture();
            }
        }

    #endif /* Blinker_UseStatus */

#endif /* !Blinker_UsingFixedFunction */


/*******************************************************************************
* Function Name: Blinker_WritePeriod
********************************************************************************
*
* Summary:
*  This function is used to change the period of the counter.  The new period
*  will be loaded the next time terminal count is detected.
*
* Parameters:
*  period:  Period value. May be between 1 and (2^Resolution)-1.  A value of 0
*           will result in the counter remaining at zero.
*
* Return:
*  None
*
*******************************************************************************/
void Blinker_WritePeriod(uint16 period) 
{
    #if(Blinker_UsingFixedFunction)
        CY_SET_REG16(Blinker_PERIOD_LSB_PTR, (uint16)period);
    #else
        CY_SET_REG16(Blinker_PERIOD_LSB_PTR, period);
    #endif /* (Blinker_UsingFixedFunction) */
}

#if (Blinker_UseOneCompareMode)


    /*******************************************************************************
    * Function Name: Blinker_WriteCompare
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare1 value when the PWM is in Dither
    *  mode. The compare output will reflect the new value on the next UDB clock.
    *  The compare output will be driven high when the present counter value is
    *  compared to the compare value based on the compare mode defined in
    *  Dither Mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  This function is only available if the PWM mode parameter is set to
    *  Dither Mode, Center Aligned Mode or One Output Mode
    *
    *******************************************************************************/
    void Blinker_WriteCompare(uint16 compare) \
                                       
    {
        #if(Blinker_UsingFixedFunction)
            CY_SET_REG16(Blinker_COMPARE1_LSB_PTR, (uint16)compare);
        #else
            CY_SET_REG16(Blinker_COMPARE1_LSB_PTR, compare);
        #endif /* (Blinker_UsingFixedFunction) */

        #if (Blinker_PWMMode == Blinker__B_PWM__DITHER)
            #if(Blinker_UsingFixedFunction)
                CY_SET_REG16(Blinker_COMPARE2_LSB_PTR, (uint16)(compare + 1u));
            #else
                CY_SET_REG16(Blinker_COMPARE2_LSB_PTR, (compare + 1u));
            #endif /* (Blinker_UsingFixedFunction) */
        #endif /* (Blinker_PWMMode == Blinker__B_PWM__DITHER) */
    }


#else


    /*******************************************************************************
    * Function Name: Blinker_WriteCompare1
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare1 value.  The compare output will
    *  reflect the new value on the next UDB clock.  The compare output will be
    *  driven high when the present counter value is less than or less than or
    *  equal to the compare register, depending on the mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_WriteCompare1(uint16 compare) \
                                        
    {
        #if(Blinker_UsingFixedFunction)
            CY_SET_REG16(Blinker_COMPARE1_LSB_PTR, (uint16)compare);
        #else
            CY_SET_REG16(Blinker_COMPARE1_LSB_PTR, compare);
        #endif /* (Blinker_UsingFixedFunction) */
    }


    /*******************************************************************************
    * Function Name: Blinker_WriteCompare2
    ********************************************************************************
    *
    * Summary:
    *  This funtion is used to change the compare value, for compare1 output.
    *  The compare output will reflect the new value on the next UDB clock.
    *  The compare output will be driven high when the present counter value is
    *  less than or less than or equal to the compare register, depending on the
    *  mode.
    *
    * Parameters:
    *  compare:  New compare value.
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_WriteCompare2(uint16 compare) \
                                        
    {
        #if(Blinker_UsingFixedFunction)
            CY_SET_REG16(Blinker_COMPARE2_LSB_PTR, compare);
        #else
            CY_SET_REG16(Blinker_COMPARE2_LSB_PTR, compare);
        #endif /* (Blinker_UsingFixedFunction) */
    }
#endif /* UseOneCompareMode */

#if (Blinker_DeadBandUsed)


    /*******************************************************************************
    * Function Name: Blinker_WriteDeadTime
    ********************************************************************************
    *
    * Summary:
    *  This function writes the dead-band counts to the corresponding register
    *
    * Parameters:
    *  deadtime:  Number of counts for dead time
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_WriteDeadTime(uint8 deadtime) 
    {
        /* If using the Dead Band 1-255 mode then just write the register */
        #if(!Blinker_DeadBand2_4)
            CY_SET_REG8(Blinker_DEADBAND_COUNT_PTR, deadtime);
        #else
            /* Otherwise the data has to be masked and offset */
            /* Clear existing data */
            Blinker_DEADBAND_COUNT &= ((uint8)(~Blinker_DEADBAND_COUNT_MASK));

            /* Set new dead time */
            #if(Blinker_DEADBAND_COUNT_SHIFT)
                Blinker_DEADBAND_COUNT |= ((uint8)((uint8)deadtime << Blinker_DEADBAND_COUNT_SHIFT)) &
                                                    Blinker_DEADBAND_COUNT_MASK;
            #else
                Blinker_DEADBAND_COUNT |= deadtime & Blinker_DEADBAND_COUNT_MASK;
            #endif /* (Blinker_DEADBAND_COUNT_SHIFT) */

        #endif /* (!Blinker_DeadBand2_4) */
    }


    /*******************************************************************************
    * Function Name: Blinker_ReadDeadTime
    ********************************************************************************
    *
    * Summary:
    *  This function reads the dead-band counts from the corresponding register
    *
    * Parameters:
    *  None
    *
    * Return:
    *  Dead Band Counts
    *
    *******************************************************************************/
    uint8 Blinker_ReadDeadTime(void) 
    {
        /* If using the Dead Band 1-255 mode then just read the register */
        #if(!Blinker_DeadBand2_4)
            return (CY_GET_REG8(Blinker_DEADBAND_COUNT_PTR));
        #else

            /* Otherwise the data has to be masked and offset */
            #if(Blinker_DEADBAND_COUNT_SHIFT)
                return ((uint8)(((uint8)(Blinker_DEADBAND_COUNT & Blinker_DEADBAND_COUNT_MASK)) >>
                                                                           Blinker_DEADBAND_COUNT_SHIFT));
            #else
                return (Blinker_DEADBAND_COUNT & Blinker_DEADBAND_COUNT_MASK);
            #endif /* (Blinker_DEADBAND_COUNT_SHIFT) */
        #endif /* (!Blinker_DeadBand2_4) */
    }
#endif /* DeadBandUsed */

#if (Blinker_UseStatus || Blinker_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: Blinker_SetInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  This function configures the interrupts mask control of theinterrupt
    *  source status register.
    *
    * Parameters:
    *  uint8 interruptMode: Bit field containing the interrupt sources enabled
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_SetInterruptMode(uint8 interruptMode) 
    {
        CY_SET_REG8(Blinker_STATUS_MASK_PTR, interruptMode);
    }


    /*******************************************************************************
    * Function Name: Blinker_ReadStatusRegister
    ********************************************************************************
    *
    * Summary:
    *  This function returns the current state of the status register.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8 : Current status register value. The status register bits are:
    *  [7:6] : Unused(0)
    *  [5]   : Kill event output
    *  [4]   : FIFO not empty
    *  [3]   : FIFO full
    *  [2]   : Terminal count
    *  [1]   : Compare output 2
    *  [0]   : Compare output 1
    *
    *******************************************************************************/
    uint8 Blinker_ReadStatusRegister(void) 
    {
        return (CY_GET_REG8(Blinker_STATUS_PTR));
    }

#endif /* (Blinker_UseStatus || Blinker_UsingFixedFunction) */


#if (Blinker_UseControl)


    /*******************************************************************************
    * Function Name: Blinker_ReadControlRegister
    ********************************************************************************
    *
    * Summary:
    *  Returns the current state of the control register. This API is available
    *  only if the control register is not removed.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8 : Current control register value
    *
    *******************************************************************************/
    uint8 Blinker_ReadControlRegister(void) 
    {
        uint8 result;

        result = CY_GET_REG8(Blinker_CONTROL_PTR);
        return (result);
    }


    /*******************************************************************************
    * Function Name: Blinker_WriteControlRegister
    ********************************************************************************
    *
    * Summary:
    *  Sets the bit field of the control register. This API is available only if
    *  the control register is not removed.
    *
    * Parameters:
    *  uint8 control: Control register bit field, The status register bits are:
    *  [7]   : PWM Enable
    *  [6]   : Reset
    *  [5:3] : Compare Mode2
    *  [2:0] : Compare Mode2
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_WriteControlRegister(uint8 control) 
    {
        CY_SET_REG8(Blinker_CONTROL_PTR, control);
    }

#endif /* (Blinker_UseControl) */


#if (!Blinker_UsingFixedFunction)


    /*******************************************************************************
    * Function Name: Blinker_ReadCapture
    ********************************************************************************
    *
    * Summary:
    *  Reads the capture value from the capture FIFO.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: The current capture value
    *
    *******************************************************************************/
    uint16 Blinker_ReadCapture(void) 
    {
        return (CY_GET_REG16(Blinker_CAPTURE_LSB_PTR));
    }

#endif /* (!Blinker_UsingFixedFunction) */


#if (Blinker_UseOneCompareMode)


    /*******************************************************************************
    * Function Name: Blinker_ReadCompare
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare output when the PWM Mode parameter is
    *  set to Dither mode, Center Aligned mode, or One Output mode.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value
    *
    *******************************************************************************/
    uint16 Blinker_ReadCompare(void) 
    {
        #if(Blinker_UsingFixedFunction)
            return ((uint16)CY_GET_REG16(Blinker_COMPARE1_LSB_PTR));
        #else
            return (CY_GET_REG16(Blinker_COMPARE1_LSB_PTR));
        #endif /* (Blinker_UsingFixedFunction) */
    }

#else


    /*******************************************************************************
    * Function Name: Blinker_ReadCompare1
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare1 output.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value.
    *
    *******************************************************************************/
    uint16 Blinker_ReadCompare1(void) 
    {
        return (CY_GET_REG16(Blinker_COMPARE1_LSB_PTR));
    }


    /*******************************************************************************
    * Function Name: Blinker_ReadCompare2
    ********************************************************************************
    *
    * Summary:
    *  Reads the compare value for the compare2 output.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8/uint16: Current compare value.
    *
    *******************************************************************************/
    uint16 Blinker_ReadCompare2(void) 
    {
        return (CY_GET_REG16(Blinker_COMPARE2_LSB_PTR));
    }

#endif /* (Blinker_UseOneCompareMode) */


/*******************************************************************************
* Function Name: Blinker_ReadPeriod
********************************************************************************
*
* Summary:
*  Reads the period value used by the PWM hardware.
*
* Parameters:
*  None
*
* Return:
*  uint8/16: Period value
*
*******************************************************************************/
uint16 Blinker_ReadPeriod(void) 
{
    #if(Blinker_UsingFixedFunction)
        return ((uint16)CY_GET_REG16(Blinker_PERIOD_LSB_PTR));
    #else
        return (CY_GET_REG16(Blinker_PERIOD_LSB_PTR));
    #endif /* (Blinker_UsingFixedFunction) */
}

#if ( Blinker_KillModeMinTime)


    /*******************************************************************************
    * Function Name: Blinker_WriteKillTime
    ********************************************************************************
    *
    * Summary:
    *  Writes the kill time value used by the hardware when the Kill Mode
    *  is set to Minimum Time.
    *
    * Parameters:
    *  uint8: Minimum Time kill counts
    *
    * Return:
    *  None
    *
    *******************************************************************************/
    void Blinker_WriteKillTime(uint8 killtime) 
    {
        CY_SET_REG8(Blinker_KILLMODEMINTIME_PTR, killtime);
    }


    /*******************************************************************************
    * Function Name: Blinker_ReadKillTime
    ********************************************************************************
    *
    * Summary:
    *  Reads the kill time value used by the hardware when the Kill Mode is set
    *  to Minimum Time.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  uint8: The current Minimum Time kill counts
    *
    *******************************************************************************/
    uint8 Blinker_ReadKillTime(void) 
    {
        return (CY_GET_REG8(Blinker_KILLMODEMINTIME_PTR));
    }

#endif /* ( Blinker_KillModeMinTime) */

/* [] END OF FILE */
