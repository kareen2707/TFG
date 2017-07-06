
/******************************************************************************

* File Name:       mTouchCapAdc.c
* Dependencies:    None
* Processor:       PIC32MX795F512H
* Compiler:        C32
* Company:         Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
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
MC        22-Ian-2010   First release of ADCDriver.c, ADCDriver.h
                        Porting for PIC32MX795F512H
*******************************************************************************/
#include "mTouchCap_Adc.h"

#include "mTouchCap_CvdAPI.h"
#include "mTouchCap_PIC32MX_CVD_Physical.h"


/* call back function, static as it is passed btw Init and ISR */
static void *ADCallBackISR;
extern int button_set_number;                       //button set
extern unsigned int 	dataReadyCVD;	            //global indicating the reading of all channels finished
extern BYTE ScanChannelIndex;                      // store the index of the channels that has to be scanned
extern WORD ScanChannels[MAX_ADC_CHANNELS];


/******************************************************************************
 * Function:        void __ISR(_ADC_VECTOR, IPL2SOFT) Ad1Handler0(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function ADC ISR handler
 *
 * Note:            None
 *****************************************************************************/
void __ISR(_ADC_VECTOR, IPL2SOFT) Ad1Handler0(void)
{
        void (*ptr_fcn)(void);
        /* call the callback function */
        ptr_fcn = (void (*) ())ADCallBackISR;
        /* if ptr function is null, do not call the callback */
		if (ptr_fcn != 0)
        {
            /* callback init in ADC_Init */
			(*ptr_fcn)();
        }    
        /* acknowlege ISR */
        IFS1bits.AD1IF = 0;
}
/******************************************************************************
 * Function:        void InitADC1(void *ADC1CallBackISR)
 *
 * PreCondition:    None
 *
 * Input:           void *ADC1CallBackISR
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function the init of ADC
 *
 * Note:            None
 *****************************************************************************/

void InitADC1(void *ADC1CallBackISR)
{
    // configure and enable the ADC
    /* hardware clears the ASAM bit when ADC ISR is generated */
	/* clearing SAMP bit starts conversion */
	/* result is INT */
	AD1CON1 = 0x0010;
    
	/* do not scan inputs */
	/* interrupt at completion of each conversion */
	/* always use MUXA */
    AD1CON2 = 0x0;
    
	/* clock derived from periph clock */
	/* TAD = 2 * TPB */
    AD1CON3 = 0x0000;
    
	/* reset value for channel select at this point */
    AD1CHS = 0x0;

    //Port pin multiplexed with AN8-AN15 in Analog mode
    AD1PCFG = 0x00F0; 
	
	/* no channel si scanned */
	AD1CSSL = 0x0;
	
	/* initialization of static callback pointer */
    ADCallBackISR = ADC1CallBackISR;
	
	/* interrupt priority is 2 */
	IPC6bits.AD1IP = 2;
	/* interrupt subpriority is 0 */
	IPC6bits.AD1IS = 0;	
	/* interrupt flag is cleared */
	IFS1bits.AD1IF = 0;
	
	/* ENABLE interrupt */
	IEC1bits.AD1IE = 1;

}

/******************************************************************************
 * Function:        void ADC1CallbackFunc(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function is a callback function of the ADC interrupt.
 *                  It executed each time the ADC interrupt occurs, being
 *                  responsible witht the CVD technique data capture/processing.
 *
 * Note:            None
 *****************************************************************************/
void ADC1CallbackFunc(void)
{
    static WORD counter = 0; /* counter up to num of channels * number of reads */
     
    /* reading the external caps */
    /* wrap counte representing the number of channels inside the ReadCVD function */
    counter = mTouchCapPhy_ReadCVD(counter);
    
    /*averaging the data returned by reading */
    mTouchCapPhy_AverageData(ScanChannels[button_set_number]);
    
    counter++; //increment the counter
    button_set_number++; //and increment the button/channel number
    

    if(button_set_number == ScanChannelIndex)
    {
        /* reset the button numbering */
		button_set_number = 0;
    }    

    /* while the average voltage is not calculated this cycle */
	if(dataReadyCVD==FALSE)
    {
        // reschedule the Timer 4 interrupt, as close as possible to the current time.
        SetPeriodTimer4(1);
    }
    
    return;

}

/********************************************************************
 * Function			:    void mTouchCapADC_SetChannelADC(WORD chan_param)
 *
 * PreCondition		:    None
 *
 * Input			:    channel_select - This contains the input select parameter to be 
                     	 configured into the ADCHS register as defined below
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:    This function sets the positive and negative inputs for 
                     	 the sample multiplexers A and B.
 *
 * Note				:
 *******************************************************************/
void mTouchCapADC_SetChannelADC(WORD channel_select)
{
    ADC_CHANNEL_SELECT = channel_select;
}
