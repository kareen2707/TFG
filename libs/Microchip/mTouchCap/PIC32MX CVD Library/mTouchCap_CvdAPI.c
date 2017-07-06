
/**************************************************************************************

* File Name:       mTouchCap_CvdAPI.c
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


Change History:
Author          Date    Comments
---------------------------------------------------------------------------------------
BDB          26-Jun-2008 First release
SB			 22-Oct-2008
NK			 24-Apr-2009 Porting for 18F46J50 Eval Board
MC           22-Ian-2010 Porting for PIC32MX795F512H
**************************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//	Includes
////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "mTouchCap_CvdAPI.h"
#include "mTouchCap_PIC32MX_CVD_Physical.h"
#include "mTouchCap_Timers.h"

////////////////////////////////////////////////////////////////////////////////
//	Variables
////////////////////////////////////////////////////////////////////////////////

BYTE Decode_Method_Used=DECODE_METHOD_PRESS_ASSERT;
extern BYTE	chFilterType	[MAX_ADC_CHANNELS];
WORD ScanChannels[MAX_ADC_CHANNELS]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

unsigned int    curRawData[MAX_ADC_CHANNELS];    //current raw value read from ADC for every channels    
BYTE    Press_State [MAX_ADC_CHANNELS];
unsigned int        tripValue[MAX_ADC_CHANNELS];
unsigned int        hystValue[MAX_ADC_CHANNELS];
BYTE 	Channel_Pressed_Status[MAX_ADC_CHANNELS];                          

unsigned int    pressedCount[MAX_ADC_CHANNELS];  //Storage for count of pressed value for each channel
unsigned int    unpressedCount[MAX_ADC_CHANNELS];//Storage for count of unpressed value for each channel
unsigned int    avg_delay[MAX_ADC_CHANNELS];     //Storage for count for average update for each channel		
unsigned int    averageData[MAX_ADC_CHANNELS];   // running average of CVD channels	
unsigned int    smallAvg[MAX_ADC_CHANNELS];      // current button smallavg
unsigned int    immediateValue;             // current button value



BYTE Previous_Pressed_Channel_Num =0; //stores the channel number of the previous key that was pressed




BYTE ScanChannelIndex =0;  // store the index of the channels that has to be scanned
/********************* CVD Variables Declarations *********************************/

int button_set_number = 0;           //button set
unsigned int 	dataReadyCVD;	     //global indicating the reading of all channels finished
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void mTouchCapAPI_SetUpCVD_Default(WORD ChannelNumber);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************
 * Function:       void mTouchCap_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function is used to initialize the default
 *					settings of ADC and other related modules. 
 *
 * Note:            None
 *****************************************************************************/
void mTouchCap_Init(void)
{
    WORD Index=0;
 
    // init main ports, ADC ... 
    mTouchCapPhy_InitCVD(); 
    
    //initialize the Timer4
    mTouchCapPhy_TickInit();

    /***************************************************************************
	--------------- Demo of API "mTouchCapAPI_SetUpCVD_Default" -----------------------		
	Initialize the ADC module to defaults using the API "mTouchCapAPI_SetUpCVD_Default".
	***************************************************************************/
 	for (Index=0; Index < ScanChannelIndex; Index++) 		  
	{     
        mTouchCapAPI_SetUpCVD_Default(Index);
    }

    mTouchCapPhy_StablizeChannelData();  

}


/******************************************************************************
 * Function:        mTouchCapAPI_SetUpCVD_Default(WORD ChannelNum)
 *
 * PreCondition:    None
 *
 * Input:           Channel Number
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	Performs Initializations for internal variables.
 *
 * Note:            None
 *****************************************************************************/

void mTouchCapAPI_SetUpCVD_Default(WORD ChannelNum)
{
    avg_delay[ChannelNum] = 0; //Initialize delay count to 0 for all channels
    curRawData[ChannelNum] = 0;  // init to zero the current raw data
    averageData[ChannelNum] = 0; // init to zero the average data value
    tripValue[ChannelNum] = DEFAULT_TRIP_VALUE;  //default trip value for all the channels
}


/********************************************************************
 * Function		:    BYTE mTouchCapAPI_GetStatusDirectButton(DirectKey *Button)
 *
 * PreCondition	:    The Channel associatd with the Direct key should  have been set up. 
 *
 * Input		:     Button - Object of the structure associated with the Direct Key
 *
 * Output		:    The Touch Status of the particular key.
 * 					  0 = KEY_NOT_PRESSED
 *					  1 = KEY_PRESSED	
 *
 * Side Effects	:    None
 *
 * Overview		:    This API will provide the status of the Direct key passed which will be used 
 					by the application to perform the related task.
 *
 * Note			: 
 *******************************************************************/

BYTE mTouchCapAPI_GetStatusDirectButton (DirectKey *Button)
{

	BYTE DirectKey_Status = KEY_NOT_PRESSED;
	
		//Now check to see if the requested key is pressed
	if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num),(Button->DecodeMethod)))
	{
			Press_State[Button->Channel_Num]= RELEASE;  //reset the flag
			DirectKey_Status= KEY_PRESSED;
	}
      else
	{
			//Channel requested is not detected as pressed.
		DirectKey_Status= KEY_NOT_PRESSED;
	}
	
	return DirectKey_Status;
	
}

/********************************************************************
 * Function		:  BYTE mTouchCapAPI_SetUpChannelDirectKey(DirectKey *Button,CHAR Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
 *
 * PreCondition	:   None
 *
 * Input		       :   Button : Object of the Direct key structure
 					Channel number : channel number of the object Button associated with corresponding direct key.
 					Trip Value: Default Trip value for the channel specified by the channel number. 
 					Filter_Method: The filter method associated with the Direct Key. 
 						FILTER_METHOD_SLOWAVERAGE=0,
						FILTER_METHOD_GATEDAVERAGE,
						FILTER_METHOD_FASTAVERAGE

 					Decode_Method : The Decode method associated with the Direct Key.
 						DECODE_METHOD_MOST_PRESSED,		
						DECODE_METHOD_MULTIPLE_PRESS,		
						DECODE_METHOD_PRESS_AND_RELEASE,	
						DECODE_METHOD_PRESS_ASSERT
 *
 * Output		:     SetUpStatus - Status of the Direct key(TRUE or FALSE).
 *
 * Side Effects	:    None
 
 * Overview		:    This API will setup the channel associated with the Direct key .
 *					The channel number, filter type and decode method are stored in the structure associated with 
 *					the Direct Key.
 *
 * Note			:
 *******************************************************************/
BYTE mTouchCapAPI_SetUpChannelDirectKey(DirectKey *Button,BYTE Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
{
	CHAR SetUpStatus = TRUE;

	Button->Channel_Num = Channel_number;                // Stores Channel Number
	Button->FilterType = Filter_Method;
	chFilterType[Channel_number] = Filter_Method;
	Button->DecodeMethod = Decode_Method;
	
	LoadChannelNumber(Channel_number);
	
	return SetUpStatus;
}	

/********************************************************************
 * Function		:   BYTE mTouchCapAPI_SetUpChannelMatrixKey(MatrixKey *Button,BYTE Row_Channel_number,BYTE Col_Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
 *
 * PreCondition	:   None
 *
 * Input		:   Button - Object of the Matrix key structure
 					Row_Channel_number - channel number of the Row associated with corresponding matrix key.
 					Col_Channel_number - channel number of the Column associated with corresponding matrix key.
 					Trip Value - Default trip value for the channel associated with corresponding matrix key 
 					Filter_Method - The filter method associated with the corresponding Matrix key
 					Decode_Method - The Decode method associated with the corresponding Matrix key
 *
 * Output		:   SetUpStatus - Status of the Matrix key(TRUE or FALSE).
 *
 * Side Effects	:   None
 *
 * Overview		:  	This API will setup the channels of the Row and Column associated with the Matrix key.
 *					The channel number of the Row and Column, filter type and decode method are stored in the structure associated with 
 *					the corresponding channel.
 *
 *
 * Note			: 
 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelMatrixKey(MatrixKey *Button,BYTE Row_Channel_Number,BYTE Col_Channel_Number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
{
	BYTE SetUpStatus = TRUE;

	Button->Channel_Num[0] = Row_Channel_Number;  // Stores Channel Number of the Row asscoiated with the Matrix key
	Button->Channel_Num[1] = Col_Channel_Number;  // Stores Channel Number of the Column asscoiated with the Matrix key
	
	Button->FilterType = Filter_Method;   //Store the Filter type which will be common for both the multiplexed channel(Row and col)
	chFilterType [Row_Channel_Number] = Filter_Method;
	chFilterType [Col_Channel_Number] = Filter_Method;	
	Button->DecodeMethod = Decode_Method; //Store the Decode method which will be common for both the multiplexed channel(Row and col)
	
	//store the channels numbers in the global array for all the Matrix Key channels 
	
	//Store the Row and column channel numbers in the global array which will be used by the ISR
	// for reading the ADC value associated with the channels which are multiplxed to form a single key
	LoadChannelNumber(Row_Channel_Number);
	LoadChannelNumber(Col_Channel_Number);

	return SetUpStatus;
}	



/********************************************************************
 * Function			:    BYTE mTouchCapAPI_GetStatusMatrixButton (MatrixKey *Button)
 *
 * PreCondition		:    The Channels of the Row and Column associatd with the Matrix key should  have been already set up 
 *
 * Input			:     Button - Object of the Structure associated with the Matrix Key
 *
 * Output		    :    The Touch Status of the particular key.
 * 							0 = KEY_NOT_PRESSED
 *					 		1 = KEY_PRESSED	
 *
 * Side Effects		:    None
 *
 * Overview			:  	 This API will provide the status of the Matrix key passed which will be used 
 						by the application to perform the related task.
 *
 * Note			: 
 *******************************************************************/

	#ifdef	USE_MATRIX_KEYS
	BYTE mTouchCapAPI_GetStatusMatrixButton (MatrixKey *Button)
	{
		BYTE MatrixKey_Status = KEY_NOT_PRESSED;
		BYTE RowFlag = KEY_NOT_PRESSED;
		BYTE ColFlag = KEY_NOT_PRESSED;
	
		/* Check if the channel used as Row for corresponding Matrix key is pressed */
		//Now check to see if the requested row is pressed

		if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num[0]),(Button->DecodeMethod) ))
		{
			RowFlag=KEY_PRESSED;	// Used for determining ROW match
		}
		else
		{
			//Channel requested is not detected as pressed.
			RowFlag= KEY_NOT_PRESSED; 
		}

	 	//Now check to see if the channel used as column for corresponding Matrix key is pressed
		if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num[1]),(Button->DecodeMethod) ))
		{
		     ColFlag=KEY_PRESSED;	// Used for determining COL match
		}
		else
		{
			//Channel requested is not detected as pressed.
		    ColFlag= KEY_NOT_PRESSED; 
		}
		
		// check if both the row and column corresponding to the matrix key is pressed
		
				
		if( (ColFlag == KEY_PRESSED) && (RowFlag == KEY_PRESSED) )
			{
			MatrixKey_Status= KEY_PRESSED;		//matrix key is pressed
			Press_State[Button->Channel_Num[0]]= RELEASE;  //reset the flag
			Press_State[Button->Channel_Num[1]]= RELEASE;  //reset the flag
			}
		else
			{
			//If both channels are not pressed, the key is not pressed
			MatrixKey_Status= KEY_NOT_PRESSED; //key is not pressed
			}
	
		return MatrixKey_Status;
	}
	
	#endif
/********************************************************************
 * Function		:    BYTE mTouchCapAPI_SetUpChannelSlider2Ch(Slider2Ch *Slider, BYTE Slider_Channel1_number,BYTE Slider_Channel2_number, WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)
 *
 *
 * Input		:   		Slider - Object of the Matrix key structure
 					Channel number - channel number of the object Slider associated with corresponding 2-channel slider.
 					Trip Value - Trip value for the channels associated with the  2-channel slider 
 					Filter_Method - One of the filter method for the 2-channel slider
 					Decode_Method -The Decode method for the 2-channel slider
 *
 * Output		:   SetUpStatus - Status of the 2-channel slider(TRUE or FALSE).
 *
 * Side Effects	:   None
 *
 * Overview		:  	This API will setup the 2 channels associated with the 2-channel Slider.
 *					The 2 channel numbers, filter type and decode method are stored in the structure associated with
 *					the corresponding 2-Channel Slider
 *
 *
 * Note			: 
 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelSlider2Ch(Slider2Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)


{
	BYTE SetUpStatus = TRUE;

	Slider->Channel_Num[0] = Slider_Channel1_Number;  // Stores first Channel Number of the 2-channel slider
	Slider->Channel_Num[1] = Slider_Channel2_Number;  // Stores Second Channel Number of the 2-channel slider
	
	Slider->FilterType = Filter_Method;   //Store the Filter type which will be common for both the channels in the 2-channel slider
	
	chFilterType [Slider_Channel1_Number] = Filter_Method;
	chFilterType [Slider_Channel2_Number] = Filter_Method;
	
	Slider->DecodeMethod = Decode_Method; //Store the Decode method which will be common for both the channels in the 2-channel slider
	
	//store the channels numbers in the global array which are associated with the 2-channel slider 
	
	//Store the Channel 1 and Channel 2 numbers of the 2-channel slider in the global array which will be used by the ISR
	// for reading the ADC value associated 
	
	LoadChannelNumber(Slider_Channel1_Number);
	LoadChannelNumber(Slider_Channel2_Number);

	return SetUpStatus;
}

/**********************************************************************************
 Function		:	SHORT  mTouchCapAPI_GetStatusSlider2Ch (Slider2Ch *Slider)
	 
 PreCondition	:	 2-channel Slider setup is complete.
	
 Input			:	Slider - Object of the 2-channel slider
						
 Output 		:	sliderLevel gives the Slider percent level of the touch.
	 
 Side Effects	:	None
	 
 Overview		: 	This API gets the percentage level of a particular 2-channel slider passed.
 					The output is ratio-metrically calculated from 0% to 100% proportional to the finger on the slider.

	 
 **********************************************************************************/
 #if defined(USE_SLIDER_2CHNL) 
	
SHORT  mTouchCapAPI_GetStatusSlider2Ch (Slider2Ch *Slider)
 {
			
		SHORT SliderState;
		WORD Percent_Level = 0;
	 
	
		#ifdef USE_SLIDER_2CHNL
				 
				if( (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[0]),(Slider->DecodeMethod) )) || 
				    (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[1]),(Slider->DecodeMethod) )) )
				{
					
					//#ifdef USE_SLIDER_2CHNL
						/* Use the 2ch slider algorithm to get the Slider Level */
						Percent_Level = mTouchCapApp_TwoChannelSliderLogic(Slider);
						SliderState = (SHORT)Percent_Level;
					//#endif
				}	
				else
				{
					SliderState = -1;
					
				}	
				
							
		#endif // USE_SLIDER_2CHNL
							
		/* Get the Level */
		
	
		return SliderState;
 }
 #endif // end of  #if defined(USE_SLIDER_2CHNL) 

/********************************************************************
 * Function		:    BYTE mTouchCapAPI_SetUpChannelSlider4Ch(Slider4Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, BYTE Slider_Channel3_Number,BYTE Slider_Channel4_Number,WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)
 *
 * PreCondition	:    None
 *
 * Input		:   Slider : object of the 4-channel Slider structure
 					Channel number : channel number of the object Slider associated with 4-channel slider.
 					Trip Value: Trip value for the channels associated with the 4-channel slider.
 					Filter_Method: The filter method for the 4-channel slider. 
 					Decode_Method:The Decode method for the 4-channel slider.
 *
 * Output		:   SetUpStatus - Status of the 4-channel slider(TRUE or FALSE).
 *
 * Side Effects	:    None
 *
 * Overview		:  	This API will setup the 4 channels associated with the 4-channel Slider.
 *					The 4 channel numbers, filter type and decode method are stored in the structure associated with
 *					the corresponding 4-Channel Slider

 *
 * Note			:
 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelSlider4Ch(Slider4Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, BYTE Slider_Channel3_Number,BYTE Slider_Channel4_Number,WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)

{
	BYTE SetUpStatus = TRUE;

	Slider->Channel_Num[0] = Slider_Channel1_Number;  // Stores first Channel Number of the 4-channel slider
	Slider->Channel_Num[1] = Slider_Channel2_Number;  // Stores Second Channel Number of the 4-channel slider
	Slider->Channel_Num[2] = Slider_Channel3_Number;  // Stores third  Channel Number of the 4-channel slider
	Slider->Channel_Num[3] = Slider_Channel4_Number;  // Stores fourth Channel Number of the 4-channel slider
	
	Slider->FilterType = Filter_Method;   //Store the Filter type which will be common for all the channels in the 4 channel slider
	
	chFilterType [Slider_Channel1_Number] = Filter_Method;
	chFilterType [Slider_Channel2_Number] = Filter_Method;
	chFilterType [Slider_Channel3_Number] = Filter_Method;
	chFilterType [Slider_Channel4_Number] = Filter_Method;
	
	
	Slider->DecodeMethod = Decode_Method; //Store the Decode method which will be common for all the channels in the 4 channel slider
	
	//store the channels numbers in the global array which are associated with the 4-channel slider 
	
	//Store the Channel 1,Channel 2,Channel 3 and Channel4  numbers of the 4-channel slider in the global array which will be used by the ISR
	// for reading the ADC value associated 
	
	LoadChannelNumber(Slider_Channel1_Number);
	LoadChannelNumber(Slider_Channel2_Number);
	LoadChannelNumber(Slider_Channel3_Number);
	LoadChannelNumber(Slider_Channel4_Number);
 
	return SetUpStatus;
}



/**********************************************************************************
  Function		 :	SHORT	mTouchCapAPI_GetStatusSlider4Ch (Slider4Ch *Slider)
	  
  PreCondition	 :	 4-channel Slider setup is complete
	 
 Input		 	 :	Slider - Object of the 4-channel slider
						 
 Output		 	 :	sliderLevel gives the Slider percent level of the touch.
	  
 Side Effects	 :	None
	  
 Overview		 : 	This API gets the percentage level of a particular 4-channel slider passed.
 					The output is ratio-metrically calculated from 0% to 100% proportional to the finger on the slider.

 					
 **********************************************************************************/
 #if defined(USE_SLIDER_4CHNL)	
	
	SHORT  mTouchCapAPI_GetStatusSlider4Ch (Slider4Ch *Slider)
	{
			
		SHORT SliderState;
		WORD Percent_Level = 0;
	
		
	#ifdef USE_SLIDER_4CHNL
					 
	if( (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[0]),(Slider->DecodeMethod) )) || 
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[1]),(Slider->DecodeMethod) )) ||
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[2]),(Slider->DecodeMethod) )) || 
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[3]),(Slider->DecodeMethod) )) 
	  )
	  {
			/* Use the 4ch slider algorithm to get the Slider Level */
			Percent_Level = mTouchCapApp_FourChannelSliderLogic(Slider);
			SliderState = (SHORT)Percent_Level;
	   }
	else
	   {
			SliderState = -1; // For KEY_NOT_PRESSED
   	   }	
	
	#endif			
			
	  return SliderState;
 }
 #endif // end of  #if defined(USE_SLIDER_4CHNL)
/********************************************************************
 * Function		:    void LoadChannelNumber (BYTE Channel_Number_Passed)
 *
 * PreCondition	:    None
 *
 * Input		:    Channel Number
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 This function will load the active Touchkey Channel number in the 
 *				  	 global array ScanChannels[ScanChannelIndex++] and will also
 *  			  	 store the total channels used in the application
 *
 * Note			: 
 *******************************************************************/
void LoadChannelNumber (BYTE Channel_Number_Passed)
{
 BYTE index;
 BYTE result = FALSE;

   for(index = 0; index<ScanChannelIndex; index++)
   {
	   if(ScanChannels[index] == Channel_Number_Passed)
       {
	     result = TRUE;
	   }
    } 

	if(!result)
	{
	    ScanChannels[ScanChannelIndex++] = Channel_Number_Passed; 
	   	if(ScanChannelIndex > MAX_ADC_CHANNELS)// the ScanChannelIndex should not be incremented if the index is greater than the no of ADC channels used 
		// If the index is greater than the number of channels in the direct key board, then the index should
		// not be incremented. reset the index value
		ScanChannelIndex = 0;
		
	 }            	    
  
}

/********************************************************************
 * Function			:    BYTE mTouchCapAPI_getChannelTouchStatus(WORD ChIndex, BYTE Decode_Method)
 *
 * PreCondition		:    None
 *
 * Input				:    ChIndex -The Channel number. 
 					     Decode_Method-The type of Decode Method associated with that channel
 					     	DECODE_METHOD_MOST_PRESSED,		
						DECODE_METHOD_MULTIPLE_PRESS,		
						DECODE_METHOD_PRESS_AND_RELEASE,	
						DECODE_METHOD_PRESS_ASSERT
 *
 * Output			:    TouchStatus-Whether the key associated with the Channel is pressed or not
 					      KEY_NOT_PRESSED,
						KEY_PRESSED
 *
 * Side Effects		:    None
 *
 * Overview			:  	 This API will determine if the channel which is associated with a particular 
 *				   		 key is touched or not. It will output the pressed or unpressed status of the channel based on the
 *			             Decode method which is associated with the channel.
 *
 *
 * Note				:
 *******************************************************************/


BYTE mTouchCapAPI_getChannelTouchStatus(WORD ChIndex, BYTE Decode_Method)
{
	BYTE temp;
	BYTE TouchStatus = KEY_NOT_PRESSED;

	/* Check for valid channel number being passed */
	if (!(ChIndex < MAX_ADC_CHANNELS) )
	{
		/* return the error value if not a valid channel number */
		TouchStatus = KEY_NOT_PRESSED;
	}
	else	 //Handle the touch status for a valid channel
	{
		

	 switch (Decode_Method)
	 {
				 
		 case DECODE_METHOD_MOST_PRESSED:

			 
			    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					   Decode_Method_Used=DECODE_METHOD_MOST_PRESSED;
					   TouchStatus = KEY_PRESSED;
					    for(temp = 0; temp<ScanChannelIndex; temp++)
		                   {
			                  if(ScanChannels[temp]!=ChIndex)
			                    {
			    					if (curRawData[ScanChannels[temp]] < (averageData[ScanChannels[temp]] - tripValue[ScanChannels[temp]])) 
									{
				                     	   if ( curRawData[ChIndex] > curRawData[ScanChannels[temp]])    
			                               TouchStatus = KEY_NOT_PRESSED;
			                       			                    
				                    }               
			                    }           
			               } 
								
					}
				}else
				{
					pressedCount[ChIndex] = 0;
					TouchStatus = KEY_NOT_PRESSED;
				}
				if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
				{		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
					unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
					if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
					{ 	
						TouchStatus = KEY_NOT_PRESSED;
					}

				}
				else
				{
					unpressedCount[ChIndex] = 0;
				}		 
			 
		
		 break;

		 // This decode method will give the press status when the corresponding key is released after it is pressed
		 //The status TouchStatus is updated based on status of the key
		 //As long as the key is pressed, the status will remain as KEY_NOT_PRESSED. 
		 //The status will change to KEY_PRESSED as long as the key is released after it has been pressed
		 case DECODE_METHOD_PRESS_AND_RELEASE:

		 	    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					    TouchStatus = KEY_NOT_PRESSED;
						// This flag will indicate the channel that has been pressed so that the information about
						// the previous pressed key is stored.
						Press_State[ChIndex] = PRESS;		
						// the status of the previous pressed key should be reset when the new key is pressed
						Channel_Pressed_Status[Previous_Pressed_Channel_Num] =0; 
					}
				}else
				{ 
							//Check if the current channel has been pressed 
					if (Press_State[ChIndex])
					 { 					
					   if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
					   {		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
						pressedCount[ChIndex] = 0;
						unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
						if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
						{ 	
								
									pressedCount[ChIndex] = 0;
									//store the pressed status of the key so that the pressed status till
									//another key is pressed
									
									Channel_Pressed_Status[ChIndex] = KEY_PRESSED;

									TouchStatus = KEY_PRESSED ;
									//store the pressed key channel number so that it can be used in the application
									// to store the status of the channel of the pressed key
									Previous_Pressed_Channel_Num = ChIndex;
									
									Decode_Method_Used=DECODE_METHOD_PRESS_AND_RELEASE;
						}
					  }
					 }//end of if (Press_State[ChIndex])
				    else
					{
						TouchStatus = KEY_NOT_PRESSED;
					    unpressedCount[ChIndex] = 0;
					    pressedCount[ChIndex] = 0;
					    Press_State[ChIndex] = RELEASE;
					}
               }         
				
			
		 break;
		 		 

		 case DECODE_METHOD_MULTIPLE_PRESS:
		 case DECODE_METHOD_PRESS_ASSERT:
		 	    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					   TouchStatus = KEY_PRESSED;
					   Decode_Method_Used=DECODE_METHOD_PRESS_ASSERT;
								
					}
				}else
				{
					pressedCount[ChIndex] = 0;
					TouchStatus = KEY_NOT_PRESSED;
				}
				if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
				{		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
					unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
					if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
					{ 	
						TouchStatus = KEY_NOT_PRESSED;
					}

				}
				else
				{
					unpressedCount[ChIndex] = 0;
				}

		 break;
		
		 default:
		 TouchStatus = KEY_NOT_PRESSED;
		 break;	
		
	 }	
	}
			 		// 6. Implement quick-release for a released button
		 		/* Untouched value - used as a average data */
	if (curRawData[ChIndex]  > averageData[ChIndex])
	   {
			averageData[ChIndex] = curRawData[ChIndex];				// If curRawData is above Average, reset to high average.
	   }

	return TouchStatus;
}




