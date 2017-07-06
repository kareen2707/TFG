
/******************************************************************************

* File Name:       mTouchCap_Timers.c
* Dependencies:    None
* Processor:       PIC32MX795F512H
* Compiler:        C32
* Company:         Microchip Technology, Inc.

Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the “Company”) for its PICmicro® Microcontroller is intended and
supplied to you, the Company’s customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
software is owned by the Company and/or its supplier, and is
protected under applicable copyright laws. All rights are reserved.
Any use in violation of the foregoing restrictions may subject the
user to criminal sanctions under applicable laws, as well as to
civil liability for the breach of the terms and conditions of this
license.

THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

Author          Date    Comments
--------------------------------------------------------------------------------
BDB          26-Jun-2008 First release
NK			 24-Apr-2009 Porting for 18F46J50 Eval Board
MC           22-Ian-2010 First release of TMRDriver.c, TMRDriver.h
                         Porting for PIC32MX795F512H
*******************************************************************************/

#include "mTouchCap_Timers.h"
#include "p32xxxx.h"
#include "mTouchCap_CvdAPI.h"
#include <peripheral/timer.h>

/* timer 4 callback ISR local - init in Initialization function */
static void *T4CallBackISR;
extern WORD ScanChannels[MAX_ADC_CHANNELS];
extern int button_set_number;           //button set


/******************************************************************************
 * Function:       void SetPeriodTimer4(unsigned int time)
 *
 * PreCondition:    None
 *
 * Input:           unsigned int time
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	Set the period for timer 4 depending on input param time
 *
 * Note:            Stops and restarts the timer when setting the new period
 *****************************************************************************/
void InitTimer4(void)
{

    // set up the timer interrupt with a priority of 2	
	IPC4bits.T4IP = 2;
	// interrupt subpriority is 0
	IPC4bits.T4IS = 0;	
	// clear the interrupt flag
	IFS0bits.T4IF = 0;
	
	// initialize the callback pointer with the value passed as argument
	T4CallBackISR = Timer4CallbackFunc;
	
    /* enable the interrupt */
    IEC0bits.T4IE = 1;

    TMR4 = 0;
   	PR4 = 1000;
   	// restart the timer
   	T4CONbits.TON = 1; 

    return;
}    

/******************************************************************************
 * Function:       void SetPeriodTimer4(unsigned int time)
 *
 * PreCondition:    None
 *
 * Input:           unsigned int time
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	Set the period for timer 4 depending on input param time
 *
 * Note:            Stops and restarts the timer when setting the new period
 *****************************************************************************/
void SetPeriodTimer4(unsigned int time)
{
   	// stop the timer
	T4CONbits.TON = 0;   
    /* load the time to higher and lower byte of the timer register */
	TMR4 = 0;
   	PR4 = time;
   	// restart the timer
   	T4CONbits.TON = 1;   

}

/******************************************************************************
 * Function:       void __ISR(_TIMER_4_VECTOR, IPL2SOFT) Tmr4Handler0(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This is timer 4 interrupt handler - acknoledge and callback
 *
 * Note:            None
 *****************************************************************************/
void __ISR(_TIMER_4_VECTOR, IPL2SOFT) Tmr4Handler0(void)
{
    void (*ptr_fcn)(void);
    /* init local wiht the callback function ptr */
    ptr_fcn = (void (*) ())T4CallBackISR;
    if (ptr_fcn != 0)
    {
        // call the callback isr
        (*ptr_fcn)();
    }    
    /* acknowlege */
    IFS0bits.T4IF = 0;
}

/******************************************************************************
 * Function:       void Timer4CallbackFunc(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function is a callback interrupt function. It is executed
 *                  each time the TMR4 interrupt occurs. On it, the state machine
 *                  machine of CVD technique is executed, so that it needs to 
 *                  occur at fixed and acurate timing.
 *
 * Note:            None
 *****************************************************************************/
void Timer4CallbackFunc(void)
{
        // initialize CVD state machine
        static T_STATE_ENUM state_machine = T_STATE_CHARGE_HOLD_CAP;
        // if charge hold cap
        if(state_machine == T_STATE_CHARGE_HOLD_CAP)
        {

                
#if defined(USE_SLIDER_2CHNL)  || defined(USE_SLIDER_4CHNL)                 
                    
                TRISB &= 0xFFF0;  //ground the sensor cap for CH0-CH3
                PORTB &= 0xFFF0;                      

#endif

#if defined(USE_DIRECT_KEYS)  || defined(USE_MATRIX_KEYS) 

                TRISB &= 0x00FF;  //ground the sensor cap for CH8-CH15
                PORTB &= 0x00FF;
#endif
               
                //Charging the Chold CAP through AN4
                TRISBbits.TRISB4 = 0;
                PORTBbits.RB4 = 1;
                
                //Select Channel AN4 on internal ADC MUX
                AD1CHSbits.CH0SA = 4;

                //Start Chold charging Process
                AD1CON1bits.SAMP = 1;
                
                // setup an interrupt of the timer after precharging of CHOLD
                // and grounding of CSENSOR is done
                SetPeriodTimer4(PRECHARGING_DELAY);
                
                //switch to the next state
                state_machine = T_STATE_CHARGE_SENSOR_CAP;
        }
        // if charge sensor capacitor state
        else if(state_machine == T_STATE_CHARGE_SENSOR_CAP)
        {
            
            // setup an interrupt of the timer after charge is distributted 
            // on CHOLD and CSENSOR 
            SetPeriodTimer4(SAMPLING_DELAY_REF);
            //switch the state machine to the next state
            state_machine = T_STATE_START_CONVERSION;
 
            mTouchCapPhy_ChannelSetup(ScanChannels[button_set_number]);

#ifndef BEGIN_CONVERSION_IMMEDIATELY
        }
        else
        {
#endif
            //Start Conversion process
            AD1CON1bits.SAMP = 0;
            //stop the timer scheduling the whole process,
            //after conversion is finished, expect the ADC interrupt to occur,
            //where the timer is rescheduled
            Set_ScanTimer_ON_Bit_State(DISABLE);  
            //set the state to the initial state
            state_machine = T_STATE_CHARGE_HOLD_CAP;
        }    
    return;

}


