
/*****************************************************************************
 * FileName:        mTouchCap_PIC32MX_CVD_Physical.c
 * Dependencies:
 * Processor:       PIC32MX
 * Compiler:       	C32
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright © 2010 Microchip Technology Inc.  
 * Microchip licenses this software to you solely for use with Microchip products, according to the terms of the  
 * accompanying click-wrap software license.  Microchip and its licensors retain all right, title and interest in 
 * and to the software.  All rights reserved. This software and any accompanying information is for suggestion only.  
 * It shall not be deemed to modify Microchip’s standard warranty for its products.  It is your responsibility to ensure 
 * that this software meets your requirements.

 * SOFTWARE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT 
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP OR ITS LICENSORS BE LIABLE 
 * FOR ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO INCIDENTAL, 
 * SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, 
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, OR ANY CLAIMS BY THIRD PARTIES 
 * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.  
 * The aggregate and cumulative liability of Microchip and its licensors for damages related to the use of the software 
 * will in no event exceed $1000 or the amount you paid Microchip for the software, whichever is greater.

 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS AND 
 * THE TERMS OF THE ACCOMPANYING CLICK-WRAP SOFTWARE LICENSE.
 * 
 *
 * Author               		 Date        		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Naveen. M				     14 Apr 2009		Version 0.1 Initial Draft
 * Nithin Kumar. M			     14 May 2009		Version 0.2 Updates
 * Sasha. M	/ Naveen. M	      	 4 June 2009  		Version 0.3 Updates
 * Sasha. M	/ Naveen. M			 11 Nov 2009  		Version 1.0 Release
 * Sasha. M	/ Nithin. 			 10 April 2010  	Version 1.20 Release
 * MC                            10 August 2010     Version 1.21 Release
 *****************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//	Includes
////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "mTouchCap_CvdAPI.h"
#include "mTouchCap_PIC32MX_CVD_Physical.h"
#include "mTouchCap_Timers.h"


/* if sampling delay is 0, make sure that the conversion starts simultaneous */
#if (SAMPLING_DELAY == 0)

#define BEGIN_CONVERSION_IMMEDIATELY
#else
#define SAMPLING_DELAY_REF         SAMPLING_DELAY
#undef BEGIN_CONVERSION_IMMEDIATELY
#endif

extern unsigned int    averageData[MAX_ADC_CHANNELS];   // running average of CVD channels
extern unsigned int    tripValue[MAX_ADC_CHANNELS];
extern unsigned int    hystValue[MAX_ADC_CHANNELS];
extern unsigned int    curRawData[MAX_ADC_CHANNELS];    //current raw value read from ADC for every channels

volatile unsigned int  BufferA[MAX_ADC_CHANNELS * NUM_HF_READS] ; //ADC buffer
unsigned long          Vavg[MAX_ADC_CHANNELS];             //voltage average, calculated using NUM_HF_READS samples
extern unsigned int    dataReadyCVD;	            //global indicating the reading of all channels finished

extern int      button_set_number;           //button set


BYTE	chFilterType	[MAX_ADC_CHANNELS];	// Channel filter type
WORD	avg_delay 		[MAX_ADC_CHANNELS];//Storage for count for average update for each channel
WORD	smallAvg    	[MAX_ADC_CHANNELS];	// current button smallavg

extern WORD ScanChannels[MAX_ADC_CHANNELS];
extern BYTE ScanChannelIndex;  // store the index of the channels that has to be scanned



/********************************************************************
 * Function			:    void mTouchCapPhy_ReadCVD(WORD ChannelNumber) 
 *
 * PreCondition		:    None
 *
 * Input			:    ChannelNumber

 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Scans the CVD channel for ADC voltage. It updates the "curRawData" and "actualValue" buffers.
 *
 *
 * Note			:
 *******************************************************************/		
WORD mTouchCapPhy_ReadCVD(WORD ChannelNumber) 
{

   int ButtonsIndex;
   int SampleIndex;
 
   if (ChannelNumber < ScanChannelIndex * NUM_HF_READS)
    {                
        // store the read data into the corresponding buffer
        BufferA[ChannelNumber] = ReadADC10(ReadActiveBufferADC10());

        if(ChannelNumber == ScanChannelIndex * NUM_HF_READS - 1)
        {
            dataReadyCVD = 1;

            //it counter indicates, for each button/channel, calculate the average value
            for(ButtonsIndex=0;ButtonsIndex<ScanChannelIndex;ButtonsIndex++)
            {
                Vavg[ButtonsIndex] = 0;
                
                //accumulate the stored values per each button/channel
                for(SampleIndex = 0 ; SampleIndex < NUM_HF_READS; SampleIndex++)
                {
                    Vavg[ButtonsIndex] =  Vavg[ButtonsIndex] + BufferA[ButtonsIndex + SampleIndex*ScanChannelIndex];
                }
                // average them, dividing the sum to the number of samples
                Vavg[ButtonsIndex] = Vavg[ButtonsIndex]/NUM_HF_READS;
                
                // store it to the raw data vector
                curRawData[ScanChannels[ButtonsIndex]] = Vavg[ButtonsIndex]; 
    
            }
        } 
    }
    else
    {
        ChannelNumber = 0;   //reset the counter
        
        // initiate the read of the next set
        BufferA[ChannelNumber] = ReadADC10(ReadActiveBufferADC10());
    }
	return ChannelNumber;
} //end ReadCVD()

/********************************************************************
 * Function			:    void mTouchCapPhy_ChannelSetup(WORD ChannelNum)
 *
 * PreCondition		:    None
 *
 * Input			:    ChannelNum
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 *
 * Overview			: 	This function will setup the ADC channels that are used by the CVD
 *				   		Eval Board.
 *
 * Note				:	Does the necessary CVD port setup.
 *******************************************************************/
void mTouchCapPhy_ChannelSetup(WORD ChannelNum)	 
{

                  //switch the reading button (channel) - one read per function call
            switch (ChannelNum)

            {
                case CHANNEL_AN0:
                {
                    TRIS_CHANNEL_AN0 = 1;
                    AD1CHSbits.CH0SA = 0;

                    break;
                }
                case CHANNEL_AN1:
                {
                    TRIS_CHANNEL_AN1 = 1;
                    AD1CHSbits.CH0SA = 1;
                    break;
                }
                case CHANNEL_AN2:
                {
                    TRIS_CHANNEL_AN2 = 1;
                    AD1CHSbits.CH0SA = 2;
                    break;
                }
                case CHANNEL_AN3:
                {
                    TRIS_CHANNEL_AN3 = 1;
                    AD1CHSbits.CH0SA = 3;
                    break;
                }

                case CHANNEL_AN4:
                {
					TRIS_CHANNEL_AN4 = 1;
                    AD1CHSbits.CH0SA = 4;

                    break;
                }
                case CHANNEL_AN5:
                {
					TRIS_CHANNEL_AN5 = 1;
                    AD1CHSbits.CH0SA = 5;
                    break;
                }
                case CHANNEL_AN6:
                {
					TRIS_CHANNEL_AN6 = 1;
                    AD1CHSbits.CH0SA = 6;
                    break;
                }
                case CHANNEL_AN7:
                {
					TRIS_CHANNEL_AN7 = 1;
                    AD1CHSbits.CH0SA = 7;
                    break;
                }
                case CHANNEL_AN8:
                {
					TRIS_CHANNEL_AN8 = 1;
                    AD1CHSbits.CH0SA = 8;
                    break;
                }
                case CHANNEL_AN9:
                {
					TRIS_CHANNEL_AN9 = 1;
                    AD1CHSbits.CH0SA = 9;
                    break;
                }
                case CHANNEL_AN10:
                {
					TRIS_CHANNEL_AN10 = 1;
                    AD1CHSbits.CH0SA = 10;
                    break;
                }
                case CHANNEL_AN11:
                {
					TRIS_CHANNEL_AN11 = 1;
                    AD1CHSbits.CH0SA = 11;
                    break;
                }
                case CHANNEL_AN12:
                {
					TRIS_CHANNEL_AN12 = 1;
                    AD1CHSbits.CH0SA = 12;
                    break;
                }
                case CHANNEL_AN13:
                {
					TRIS_CHANNEL_AN13 = 1;
                    AD1CHSbits.CH0SA = 13;
                    break;
                }
                case CHANNEL_AN14:
                {
					TRIS_CHANNEL_AN14 = 1;
                    AD1CHSbits.CH0SA = 14;
                    break;
                }
                case CHANNEL_AN15:
                {
					TRIS_CHANNEL_AN15 = 1;
                    AD1CHSbits.CH0SA = 15;
                    break;
                }
                default: break;


}


}

/********************************************************************
 * Function			:    void mTouchCapPhy_AverageData(WORD Index)
 *
 * PreCondition		:    None
 *
 * Input			:    Channel Number 
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	This function is used to average the Current Raw ADC value and the 
 *						Average ADC value based on the type of the decode method that is 
 *						selected by the application.
 * Note				:
 *******************************************************************/
 
void mTouchCapPhy_AverageData(WORD Index)
{

     BYTE Filter_type; 

	 /* Filtering based on channel specefic filter type */
	Filter_type = chFilterType[Index];

	/* Check the opted filter type of the channel. Copy if within limits. Else, the default type would be "1BY20" method. */
	if ( (Filter_type > FILTER_METHOD_FASTAVERAGE) )
	{
   		 Filter_type = FILTER_METHOD_SLOWAVERAGE; 
	}

	/* Process the average based on filter type selected by user in APi-mTouchCapAPI_SetUpChannel*/
	switch(Filter_type)
	{
	   case FILTER_METHOD_SLOWAVERAGE:
			// 7. Average in the new value (channel based delay)
					// Always Average (all buttons one after the other in a loop)
	
			if (curRawData[Index]  > averageData[Index])
			{
				averageData[Index] = curRawData[Index];				// If curRawData is above Average, reset to high average.
			}
	
			if(avg_delay[Index] < NUM_AVG)
			{
				avg_delay[Index]++;						// Counting 0..8 has effect of every 9th count cycling the next button.
			}
			else
			{
				avg_delay[Index] = 0;					// Counting 0..4 will average faster and also can use 0..4*m, m=0,1,2,3..
			}
			
			if(avg_delay[Index] == NUM_AVG)
			{
					smallAvg[Index] = averageData[Index] / NUM_AVG; 	// SMALLAVG is the current average right shifted 4 bits
					// Average in raw value.
					averageData[Index] = averageData[Index] + ((curRawData[Index] / NUM_AVG) - smallAvg[Index]);
			}
				
	  break;


	  case FILTER_METHOD_GATEDAVERAGE:
			//Filter Method CASE: Gated Average
			// 7. Average in the new value (channel based delay)
			// Always Average (all buttons one after the other in a loop)
	
			if(avg_delay[Index] < NUM_AVG)
			{
				avg_delay[Index]++;						// Counting 0..8 has effect of every 9th count cycling the next button.
			}
			else
			{
				avg_delay[Index] = 0;					// Counting 0..4 will average faster and also can use 0..4*m, m=0,1,2,3..
			}
			if(avg_delay[Index] == NUM_AVG)
			{
				/* Gated average only of no key pressed. */
				/* Stop averaging when press is being sensed. //STD:*/
				if ( KEY_NOT_PRESSED == mTouchCapAPI_getChannelTouchStatus(Index,DECODE_METHOD_PRESS_ASSERT ))
				{
					smallAvg[Index] = averageData[Index] / NUM_AVG; 	// SMALLAVG is the current average right shifted 4 bits
					// Average in raw value.
					averageData[Index] = averageData[Index] + ((curRawData[Index] / NUM_AVG) - smallAvg[Index]);
				}
			}
     break;

	case FILTER_METHOD_FASTAVERAGE:
			/* The latest current raw data would be the latest average data */
		if (curRawData[Index]  > averageData[Index])
		{
			averageData[Index] = curRawData[Index];				// If curRawData is above Average, reset to high average.
		}

	break;

  }

	/* Dynamic trip value calculation . STD: */
	 DynamicTripValueCalculation(Index);

}



/******************************************************************************
 * Function:        void mTouchCapPhy_StablizeChannelData(void)
 *
 * PreCondition:    None
 *
 * Input:           Channel Number
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function will read the current Rawdata and the untouched
 					data on the channels and will determine whether there
 					was any touch on the keypads.
 *
 * Note:            None
 *****************************************************************************/

void mTouchCapPhy_StablizeChannelData(void)
{

  	WORD i;
 
    static int             startupCount;    // variable to 'discard' first N samples
	
	startupCount	= INITIAL_STARTUP_COUNT;

	while (startupCount > 0)
	{
		while(!dataReadyCVD);

		dataReadyCVD = 0;			//clear flag

		Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear timer 4 SHORT flag
		Set_ScanTimer_IE_Bit_State(DISABLE);              //Disable interrupt
		Set_ScanTimer_ON_Bit_State(DISABLE);              //Stop timer

		startupCount--;							// Decr. N # times to establish startup


		for(i=0; i<ScanChannelIndex; i++)	
    	{
			// During start up time, set Average each pass.
			averageData[ScanChannels[i]] = curRawData[ScanChannels[i]];		
		}

		Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear flag
		Set_ScanTimer_IE_Bit_State(ENABLE);              //Enable interrupt
    	Set_ScanTimer_ON_Bit_State(ENABLE);              //Run timer
	}

}


/********************************************************************
 * Function		:    void DynamicTripValueCalculation(WORD Index)
 *
 * PreCondition	:    None
 *
 * Input		:    Index- Channel Number
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 calculate the dynamic trip value.
 *
 *
 * Note			: 
 *******************************************************************/
void DynamicTripValueCalculation(WORD Index)
{

    tripValue[Index] = (averageData[Index] / KEYTRIPDIV);
    hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE);
    
}

/********************************************************************
 * Function			:    void mTouchCapPhy_AdcSetup(void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	This function will setup the ADC module
 *
 * Note				: 	Does the necessary ADC peripheral setup.
 *******************************************************************/
void mTouchCapPhy_AdcSetup(void)
{
    InitADC1(ADC1CallbackFunc);
}

/********************************************************************
 * Function			:    void mTouchCapPhy_TickInit(void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	This function will setup the Timer4 module
 *
 * Note				: 	Does the necessary Timer4 peripheral setup except period.
 *******************************************************************/
void mTouchCapPhy_TickInit(void)
{
    InitTimer4();
}
/********************************************************************
 * Function			:    void mTouchCapPhy_InitCVD(void) 
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Does the initialization of all peripherals/ports necessary for CVD operation.
 *
 *
 * Note				:
 *******************************************************************/
void mTouchCapPhy_InitCVD(void) 
{

    /* Initialize PORTs  */
	mTouchCapApp_PortSetup();
    
    /* Initialize ADC  */
    mTouchCapPhy_AdcSetup();
    
    Set_Adc_Enable_State(ENABLE);
    
					
}
