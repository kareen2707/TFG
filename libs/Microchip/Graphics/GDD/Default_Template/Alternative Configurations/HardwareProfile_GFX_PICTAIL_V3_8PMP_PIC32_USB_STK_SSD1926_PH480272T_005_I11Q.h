/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile_xxx.h
 * Dependencies:    None
 * Processor:       PIC24F, PIC24H, dsPIC, PIC32
 * Compiler:        Microchip C32 v1.00 or higher
 *					Microchip C30 v3.01 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2002-2010 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and 
 * distribute: 
 * (i)  the Software when embedded on a Microchip microcontroller or 
 *      digital signal controller product (“Device”) which is 
 *      integrated into Licensee’s product; or
 * (ii) ONLY the Software driver source files ENC28J60.c and 
 *      ENC28J60.h ported to a non-Microchip device used in 
 *      conjunction with a Microchip ethernet controller for the 
 *      sole purpose of interfacing with the ethernet controller. 
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT 
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL 
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS 
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE 
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER 
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 10/03/06	Original, copied from Compiler.h
 * 06/25/09 dsPIC & PIC24H support 
 * 09/15/09 Added PIC24FJ256DA210 Development Board Support
 * 04/07/10 Removed GRAPHICS_HARDWARE_PLATFORM 
 *          Added MMB support
 * 08/20/10 Added check on drivers used.
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
    #define __HARDWARE_PROFILE_H

/*********************************************************************
* GetSystemClock() returns system clock frequency.
*
* GetPeripheralClock() returns peripheral clock frequency.
*
* GetInstructionClock() returns instruction clock frequency.
*
********************************************************************/

/*********************************************************************
* Macro: #define	GetSystemClock() 
*
* Overview: This macro returns the system clock frequency in Hertz.
*			* value is 8 MHz x 4 PLL for PIC24
*			* value is 8 MHz/2 x 18 PLL for PIC32
*
********************************************************************/
    #if defined(__PIC24F__)
        #define GetSystemClock()    (32000000ul)
    #elif defined(__PIC32MX__)
        #define GetSystemClock()    (80000000ul)
    #elif defined(__dsPIC33F__) || defined(__PIC24H__)
        #define GetSystemClock()    (80000000ul)
    #endif

/*********************************************************************
* Macro: #define	GetPeripheralClock() 
*
* Overview: This macro returns the peripheral clock frequency 
*			used in Hertz.
*			* value for PIC24 is <PRE>(GetSystemClock()/2) </PRE> 
*			* value for PIC32 is <PRE>(GetSystemClock()/(1<<OSCCONbits.PBDIV)) </PRE>
*
********************************************************************/
    #if defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__)
        #define GetPeripheralClock()    (GetSystemClock() / 2)
    #elif defined(__PIC32MX__)
        #define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))
    #endif

/*********************************************************************
* Macro: #define	GetInstructionClock() 
*
* Overview: This macro returns instruction clock frequency 
*			used in Hertz.
*			* value for PIC24 is <PRE>(GetSystemClock()/2) </PRE> 
*			* value for PIC32 is <PRE>(GetSystemClock()) </PRE> 
*
********************************************************************/
    #if defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__)
        #define GetInstructionClock()   (GetSystemClock() / 2)
    #elif defined(__PIC32MX__)
        #define GetInstructionClock()   (GetSystemClock())
    #endif

/*********************************************************************
* PIC Device Specific includes
*********************************************************************/
#ifdef __PIC32MX
	#include <p32xxxx.h>
#endif

/*********************************************************************
* UART SETTINGS
*********************************************************************/
	#define BAUDRATE2               115200UL
    #define BRG_DIV2                4
    #define BRGH2                   1

/* ################################################################## */
/*********************************************************************
* START OF GRAPHICS RELATED MACROS
********************************************************************/
/* ################################################################## */

/*********************************************************************
* AUTO GENERATED CODE 
********************************************************************/

//Auto Generated Code
#define GFX_PICTAIL_V3
#define EXPLORER_16
#define USE_8BIT_PMP
#define PIC32_USB_SK_DM320003_1
#define DISPLAY_CONTROLLER SSD1926
#define DISPLAY_PANEL PH480272T_005_I11Q
#define COLOR_DEPTH 16
//End Auto Generated Code



/*********************************************************************
* END OF AUTO GENERATED CODE 
********************************************************************/

#ifdef _GRAPHICS_H

/*********************************************************************
* ERROR CHECKING
*********************************************************************/
#if !defined (GFX_PICTAIL_V3) && !defined (GFX_PICTAIL_V2) && !defined (PIC24FJ256DA210_DEV_BOARD) && !defined(MULTI_MEDIA_BOARD_DM00123)
	#error "Error: Graphics Hardware Platform is not defined! One of the three (GFX_PICTAIL_V3, GFX_PICTAIL_V2, PIC24FJ256DA210_DEV_BOARD) must be defined.
#endif

#if defined (__PIC24FJ128GA010__) || defined (__PIC24FJ256GA110__) ||	\
	defined (__PIC24FJ256GB110__) || defined (__dsPIC33F__)        ||   \
	defined (__PIC24HJ128GP504__)
	#if defined (USE_16BIT_PMP)	
		#error "The device selected does not support 16 bit PMP interface."
	#endif
#endif

#ifndef DISPLAY_CONTROLLER
	#error "Error: DISPLAY_CONTROLLER not defined"
#endif

#if (defined (GFX_PICTAIL_V3) || defined (PIC24FJ256DA210_DEV_BOARD)) && !defined(DISPLAY_PANEL)
	#error "Error: DISPLAY_PANEL not defined"
#endif

/*********************************************************************
* IPU OPERATION MACROS.
*********************************************************************/
/* ********************************************************************
* These defines the cache areas when using IPU of the Graphics Module. 
* The IPU operation will need two cache areas:
* - for compressed data (GFX_COMPRESSED_DATA_RAM_ADDRESS, 
*   GFX_COMPRESSED_BUFFER_SIZE)
* - for decompressed data (GFX_DECOMPRESSED_DATA_RAM_ADDRESS, 
*   GFX_DECOMPRESSED_BUFFER_SIZE)
* These two areas are not part of the memory that the Display Driver 
* uses for the viewable pixels. These two areas will exclusively be 
* used by the IPU operation to perform the decompression.
* The use of the IPU cache areas assumes that the hardware used
* is the PIC24FJ256DA210 Development Board where the EPMP CS1 is hooked
* up to SRAM and EPMP CS2 is hooked up to a parallel flash memory.
* Make sure that the buffer sizes are word aligned since IPU operates 
* only on WORD aligned addresses.
* Users can derive the size of the two areas based on the output of the
* "Graphics Resource Converter" tool used to generate the hex or C array
* of the compressed images. Please refer to the "Graphics Resource 
* Converter" Help file for details.
* If the compressed image is placed in Parallel Flash, then the
* GFX_COMPRESSED_BUFFER_SIZE is not needed since the IPU module
* can access the compressed data directly.
********************************************************************  */

#if defined (PIC24FJ256DA210_DEV_BOARD)
   #if defined (USE_COMP_IPU)
      #if defined (GFX_EPMP_CS1_BASE_ADDRESS)

         #define GFX_COMPRESSED_BUFFER_SIZE              (1546)
         #define GFX_DECOMPRESSED_BUFFER_SIZE            (2074)
         #define GFX_IPU_TEMP_DATA_TRANSFER_ARRAY_SIZE   (1024)

         #ifdef USE_DOUBLE_BUFFERING
            #define GFX_DECOMPRESSED_DATA_RAM_ADDRESS   (GFX_EPMP_CS1_BASE_ADDRESS + (GFX_DISPLAY_BUFFER_LENGTH*2))
         #else
            #define GFX_DECOMPRESSED_DATA_RAM_ADDRESS   (GFX_EPMP_CS1_BASE_ADDRESS + GFX_DISPLAY_BUFFER_LENGTH)
         #endif
   
         #define GFX_COMPRESSED_DATA_RAM_ADDRESS        (GFX_DECOMPRESSED_DATA_RAM_ADDRESS + GFX_DECOMPRESSED_BUFFER_SIZE)     

      #else
         #warning "EPMP CS1 Base Address not defined. If you are using IPU make sure that the GFX_COMPRESSED_DATA_RAM_ADDRESS & GFX_DECOMPRESSED_DATA_RAM_ADDRESS are allocated properly in internal memory."                   
      #endif

      // Error checks
      #if defined (GFX_EPMP_CS2_BASE_ADDRESS) && defined (GFX_COMPRESSED_BUFFER_SIZE)
         #if ((GFX_COMPRESSED_DATA_RAM_ADDRESS+GFX_COMPRESSED_BUFFER_SIZE) > GFX_EPMP_CS2_BASE_ADDRESS)
            #error "Compressed data buffer is overlapping EPMP CS2 space. Adjust the EPMP CS2 Base Address value."
         #endif
      #elif defined (GFX_EPMP_CS2_BASE_ADDRESS) && defined (GFX_DECOMPRESSED_BUFFER_SIZE)
         #if ((GFX_DECOMPRESSED_DATA_RAM_ADDRESS+GFX_DECOMPRESSED_BUFFER_SIZE) > GFX_EPMP_CS2_BASE_ADDRESS)
            #error "Decompressed data buffer is overlapping EPMP CS2 space. Adjust the EPMP CS2 Base Address value."
         #endif
      #endif          
   #endif //#if defined (USE_COMP_IPU)
#endif //#if defined (PIC24FJ256DA210_DEV_BOARD)


/*********************************************************************
* SETTINGS FOR DISPLAY DRIVER FEATURE for PIC24FJ256DA210  Family of
* display controller
*********************************************************************/

	#ifdef USE_DOUBLE_BUFFERING
		#ifdef GFX_DISPLAY_BUFFER_START_ADDRESS
			#undef GFX_DISPLAY_BUFFER_START_ADDRESS
			
			volatile extern  DWORD _drawbuffer;
			#define GFX_DISPLAY_BUFFER_START_ADDRESS _drawbuffer
		#endif
		
		#define GFX_BUFFER1 0x00020000ul
		#define GFX_BUFFER2 0x00050000ul

	#endif

/*********************************************************************
* DISPLAY PARALLEL INTERFACE
*********************************************************************/
	// EPMP is exclusive to GB210 devices and PMP to some devices
	#if defined (__PIC24FJ256DA210__)
		// EPMP is used by graphics controller
	#elif defined (__PIC24FJ256GB210__)
		#define USE_GFX_EPMP
	#else
		#define USE_GFX_PMP
	#endif

/*********************************************************************
* DISPLAY SETTINGS 
********************************************************************/

// -----------------------------------
// For SMART DISPLAYS
// -----------------------------------

// Using LGDP4531 Display Controller
	#if (DISPLAY_CONTROLLER == LGDP4531)
		#define DISP_ORIENTATION    90
        #define DISP_HOR_RESOLUTION 240
        #define DISP_VER_RESOLUTION 320

		#define DISPLAY_PANEL -1
	#endif // #if (DISPLAY_CONTROLLER == LGDP4531)

// Using S6D0129 Display Controller
	#if (DISPLAY_CONTROLLER == S6D0129)
		#define DISP_ORIENTATION    90
        #define DISP_HOR_RESOLUTION 240
        #define DISP_VER_RESOLUTION 320
	#endif // #if (DISPLAY_CONTROLLER == S6D0129)

// -----------------------------------
// For SMART RGB GLASS
// -----------------------------------

// check if DISPLAY_PANEL is not defined set it to -1 to skip the rest
// of the display panel settings.
#if !defined (DISPLAY_PANEL)
	#define DISPLAY_PANEL -1
#endif	

// Using TFT_G240320LTSW_118W_E, TFT2N0369_E, DT032TFT_TS, DT032TFT  Display Panel.
// These panels uses Solomon Systech SSD1289 as their controller. 
	#if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E)	|| (DISPLAY_PANEL == TFT2N0369_E) ||		\
		(DISPLAY_PANEL == DT032TFT_TS)				|| (DISPLAY_PANEL == DT032TFT)

		/* note: For SSD1289 
			
			tHBP (horizontal back porch)= DISP_HOR_BACK_PORCH + DISP_HOR_PULSE_WIDTH
			tVBP (vertical back porch)  = DISP_VER_BACK_PORCH + DISP_VER_PULSE_WIDTH

			Horizontal Cycle (280) = tHBP + DISP_HOR_FRONT_PORCH + DISP_HOR_RESOLUTION
			Vertical Cycle (326)   = tVBP + DISP_VER_FRONT_PORCH + DISP_VER_RESOLUTION

			In summary, the DISP_HOR_PULSE_WIDTH should not exceed tHBP and 
			the DISP_VER_PULSE_WIDTH should not exceed tVBP. See SSD1289 data sheet for 
			details.
		*/
		#define DISP_ORIENTATION		90
        #define DISP_HOR_RESOLUTION		240
        #define DISP_VER_RESOLUTION		320
        #define DISP_DATA_WIDTH			18
        #define DISP_INV_LSHIFT
        #define DISP_HOR_PULSE_WIDTH    25
        #define DISP_HOR_BACK_PORCH     5
        #define DISP_HOR_FRONT_PORCH    10
        #define DISP_VER_PULSE_WIDTH    4
        #define DISP_VER_BACK_PORCH     0
        #define DISP_VER_FRONT_PORCH    2

		#if (DISPLAY_CONTROLLER == MCHP_DA210)
			#define GFX_LCD_TYPE                        GFX_LCD_TFT
            #define GFX_DISPLAYENABLE_ENABLE
            #define GFX_HSYNC_ENABLE
            #define GFX_VSYNC_ENABLE
            #define GFX_DISPLAYPOWER_ENABLE
            #define GFX_CLOCK_POLARITY                  GFX_ACTIVE_HIGH
            #define GFX_DISPLAYENABLE_POLARITY          GFX_ACTIVE_HIGH
            #define GFX_HSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_VSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_DISPLAYPOWER_POLARITY           GFX_ACTIVE_HIGH
        #endif
	#endif // #if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E) || (DISPLAY_PANEL == TFT2N0369_E) ||....

// Using TFT_G320240DTSW_69W_TP_E Display Panel
	#if (DISPLAY_PANEL == TFT_G320240DTSW_69W_TP_E) || (DISPLAY_PANEL == _35QVW0T)  
		#define DISP_ORIENTATION		0
        #define DISP_HOR_RESOLUTION		320
        #define DISP_VER_RESOLUTION		240
        #define DISP_DATA_WIDTH			18
        #define DISP_HOR_PULSE_WIDTH    25
        #define DISP_HOR_BACK_PORCH     8
        #define DISP_HOR_FRONT_PORCH    8
        #define DISP_VER_PULSE_WIDTH    8
        #define DISP_VER_BACK_PORCH     7
        #define DISP_VER_FRONT_PORCH    5
	#endif // #if (DISPLAY_PANEL == TFT_G320240DTSW_69W_TP_E) || (DISPLAY_PANEL == _35QVW0T) 

// Using PH480272T_005_I06Q Display Panel
	#if (DISPLAY_PANEL == PH480272T_005_I06Q) 
		#define DISP_ORIENTATION		0
        #define DISP_HOR_RESOLUTION		480
        #define DISP_VER_RESOLUTION		272
        #define DISP_DATA_WIDTH			24
        #define DISP_HOR_PULSE_WIDTH    41
        #define DISP_HOR_BACK_PORCH     2
        #define DISP_HOR_FRONT_PORCH    2
        #define DISP_VER_PULSE_WIDTH    10
        #define DISP_VER_BACK_PORCH     2
        #define DISP_VER_FRONT_PORCH    2
        #if (DISPLAY_CONTROLLER == MCHP_DA210)
            #define GFX_LCD_TYPE                        GFX_LCD_TFT
            #define GFX_DISPLAYENABLE_ENABLE
            #define GFX_HSYNC_ENABLE
            #define GFX_VSYNC_ENABLE
            #define GFX_DISPLAYPOWER_ENABLE
            #define GFX_CLOCK_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_DISPLAYENABLE_POLARITY          GFX_ACTIVE_HIGH
            #define GFX_HSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_VSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_DISPLAYPOWER_POLARITY           GFX_ACTIVE_HIGH
        #endif
	#endif // #if (DISPLAY_PANEL == PH480272T_005_I06Q)

// Using PH480272T_005_I11Q Display Panel
	#if (DISPLAY_PANEL == PH480272T_005_I11Q) 
		#define DISP_ORIENTATION		0
        #define DISP_HOR_RESOLUTION		480
        #define DISP_VER_RESOLUTION		272
        #define DISP_DATA_WIDTH			24
        #define DISP_HOR_PULSE_WIDTH    41
        #define DISP_HOR_BACK_PORCH     2
        #define DISP_HOR_FRONT_PORCH    2
        #define DISP_VER_PULSE_WIDTH    10
        #define DISP_VER_BACK_PORCH     2
        #define DISP_VER_FRONT_PORCH    2
        #if (DISPLAY_CONTROLLER == MCHP_DA210)
            #define GFX_LCD_TYPE                        GFX_LCD_TFT
            #define GFX_DISPLAYENABLE_ENABLE
            #define GFX_HSYNC_ENABLE
            #define GFX_VSYNC_ENABLE
            #define GFX_DISPLAYPOWER_ENABLE
            #define GFX_CLOCK_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_DISPLAYENABLE_POLARITY          GFX_ACTIVE_HIGH
            #define GFX_HSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_VSYNC_POLARITY                  GFX_ACTIVE_LOW
            #define GFX_DISPLAYPOWER_POLARITY           GFX_ACTIVE_HIGH
        #endif
	#endif // #if (DISPLAY_PANEL == PH480272T_005_I11Q)

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

#endif //ifdef _GRAPHICS_H

/*********************************************************************
* Default Hardware Selection 
* - This sets up the default hardware for the selected hardware
*   platform.
********************************************************************/

#ifdef PIC24FJ256DA210_DEV_BOARD
	   #if defined (GFX_EPMP_CS2_BASE_ADDRESS)
	       #define   USE_SST39LF400					   // use parallel flash since EPMP CS 2 is enabled
       #else	
           #define	 USE_SST25VF016                    // use the SPI Flash 
       #endif 
       #define       USE_RESISTIVE_TOUCH               // use the resistive touch 
#endif

#ifdef GFX_PICTAIL_V2
       #define       USE_25LC256                       // use the SPI Flash on Explorer 16 
       #define       USE_SST39VF040                    // use the parallel on GFX Board 
       #define       USE_RESISTIVE_TOUCH               // use the resistive touch 
	   #define		 USE_GFX_PICTAIL_V2_BEEPER         // use the PWM controlled beeper 
#endif

#ifdef GFX_PICTAIL_V3
       #define       USE_SST25VF016                    // use the SPI Flash 
       #define       USE_RESISTIVE_TOUCH               // use the resistive touch 
#endif

#ifdef MULTI_MEDIA_BOARD_DM00123
	   #define       USE_XC2C64A					   // use the CPLD on the board	
       #define       USE_SST25VF016                    // use the SPI Flash 
       #define       USE_RESISTIVE_TOUCH               // use the resistive touch 
	   #define       USE_ACCELEROMETER_BMA150		   // use the accelerometer module	
       #define       USE_JOYSTICK_MEB                  // use the joystick
#endif

/*********************************************************************
* IOS FOR THE DISPLAY CONTROLLER
*********************************************************************/
#if defined (GFX_PICTAIL_V2)
    #if (DISPLAY_CONTROLLER == LGDP4531)
        #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)

            // Definitions for reset pin
            #define DisplayResetConfig()        TRISAbits.TRISA4 = 0    
            #define DisplayResetEnable()        LATAbits.LATA4 = 0
            #define DisplayResetDisable()       LATAbits.LATA4 = 1

            // Definitions for RS pin
            #define DisplayCmdDataConfig()      TRISAbits.TRISA1 = 0
            #define DisplaySetCommand()         LATAbits.LATA1 = 0
            #define DisplaySetData()            LATAbits.LATA1 = 1

            // Definitions for CS pin
            #define DisplayConfig()             TRISBbits.TRISB15 = 0             
            #define DisplayEnable()             LATBbits.LATB15 = 0
            #define DisplayDisable()            LATBbits.LATB15 = 1

            // Definitions for FLASH CS pin
            #define DisplayFlashConfig()        TRISAbits.TRISA8 = 0        
            #define DisplayFlashEnable()        LATAbits.LATA8 = 1
            #define DisplayFlashDisable()       LATAbits.LATA8 = 0

            // Definitions for POWER ON pin
            #define DisplayPowerConfig()        TRISAbits.TRISA10 = 0
            #define DisplayPowerOn()            LATAbits.LATA10 = 0
            #define DisplayPowerOff()           LATAbits.LATA10 = 1

        #else

            #ifndef __PIC32MX__
	            // Definitions for reset pin
                #define DisplayResetConfig()        TRISCbits.TRISC1 = 0    
                #define DisplayResetEnable()        LATCbits.LATC1 = 0
                #define DisplayResetDisable()       LATCbits.LATC1 = 1

	            // Definitions for RS pin
                #define DisplayCmdDataConfig()      TRISCbits.TRISC2 = 0
                #define DisplaySetCommand()         LATCbits.LATC2 = 0
                #define DisplaySetData()            LATCbits.LATC2 = 1

	            // Definitions for CS pin
                #define DisplayConfig()             TRISDbits.TRISD10 = 0             
                #define DisplayEnable()             LATDbits.LATD10 = 0
                #define DisplayDisable()            LATDbits.LATD10 = 1

                // Definitions for FLASH CS pin
                #define DisplayFlashConfig()        TRISDbits.TRISD1 = 0        
                #define DisplayFlashEnable()        LATDbits.LATD1 = 1
                #define DisplayFlashDisable()       LATDbits.LATD1 = 0

                // Definitions for POWER ON pin
                #define DisplayPowerConfig()        TRISCbits.TRISC3 = 0
                #define DisplayPowerOn()            LATCbits.LATC3 = 0
                #define DisplayPowerOff()           LATCbits.LATC3 = 1
            #else
            /********
             * PIC32 will use the SET and CLR SFRs for the I/O.
             * These are atomic settings that are recommended when
             * modifying SFRs
             ********/
	            // Definitions for reset pin
                #define DisplayResetConfig()        TRISCCLR = _TRISC_TRISC1_MASK    
                #define DisplayResetEnable()        LATCCLR = _LATC_LATC1_MASK
                #define DisplayResetDisable()       LATCSET = _LATC_LATC1_MASK

	            // Definitions for RS pin
                #define DisplayCmdDataConfig()      TRISCCLR = _TRISC_TRISC2_MASK
                #define DisplaySetCommand()         LATCCLR = _LATC_LATC2_MASK
                #define DisplaySetData()            LATCSET = _LATC_LATC2_MASK

	            // Definitions for CS pin
                #define DisplayConfig()             TRISDCLR = _TRISD_TRISD10_MASK
                #define DisplayEnable()             LATDCLR = _LATD_LATD10_MASK
                #define DisplayDisable()            LATDSET = _LATD_LATD10_MASK

                // Definitions for FLASH CS pin
                #define DisplayFlashConfig()        TRISDCLR = _TRISD_TRISD1_MASK     
                #define DisplayFlashEnable()        LATDCLR = _LATD_LATD1_MASK
                #define DisplayFlashDisable()       LATDSET = _LATD_LATD1_MASK

                // Definitions for POWER ON pin
                #define DisplayPowerConfig()        TRISCCLR = _TRISC_TRISC3_MASK
                #define DisplayPowerOn()            LATCCLR = _LATC_LATC3_MASK 
                #define DisplayPowerOff()           LATCSET = _LATC_LATC3_MASK
            #endif

        #endif
    #elif (DISPLAY_CONTROLLER == SSD1906)

            #ifndef __PIC32MX__
	            // Definitions for reset pin
                #define DisplayResetConfig()        TRISCbits.TRISC1 = 0    
                #define DisplayResetEnable()        LATCbits.LATC1 = 0
                #define DisplayResetDisable()       LATCbits.LATC1 = 1

	            // Definitions for RS pin
                #define DisplayCmdDataConfig()      TRISCbits.TRISC2 = 0
                #define DisplaySetCommand()         LATCbits.LATC2 = 0
                #define DisplaySetData()            LATCbits.LATC2 = 1

	            // Definitions for CS pin
                #define DisplayConfig()             TRISDbits.TRISD10 = 0             
                #define DisplayEnable()             LATDbits.LATD10 = 0
                #define DisplayDisable()            LATDbits.LATD10 = 1

                // Definitions for FLASH CS pin
                #define DisplayFlashConfig()        TRISDbits.TRISD1 = 0        
                #define DisplayFlashEnable()        LATDbits.LATD1 = 1
                #define DisplayFlashDisable()       LATDbits.LATD1 = 0

                // Definitions for POWER ON pin
                #define DisplayPowerConfig()        TRISCbits.TRISC3 = 0
                #define DisplayPowerOn()            LATCbits.LATC3 = 1
                #define DisplayPowerOff()           LATCbits.LATC3 = 0

                // Extended address lines
                #define DisplayAddr0Config()        TRISDbits.TRISD3 = 0
                #define DisplaySetAddr0()           LATDbits.LATD3 = 1
                #define DisplayClearAddr0()         LATDbits.LATD3 = 0

                #define DisplayAddr17Config()       TRISGbits.TRISG14 = 0
                #define DisplaytSetAddr17()         TRISGbits.TRISG14 = 1
                #define DisplayClearAddr17()        TRISGbits.TRISG14 = 0
            #else
            /********
             * PIC32 will use the SET and CLR SFRs for the I/O.
             * These are atomic settings that are recommended when
             * modifying SFRs
             ********/
	            // Definitions for reset pin
                #define DisplayResetConfig()        TRISCCLR = _TRISC_TRISC1_MASK    
                #define DisplayResetEnable()        LATCCLR = _LATC_LATC1_MASK
                #define DisplayResetDisable()       LATCSET = _LATC_LATC1_MASK

	            // Definitions for RS pin
                #define DisplayCmdDataConfig()      TRISCCLR = _TRISC_TRISC2_MASK
                #define DisplaySetCommand()         LATCCLR = _LATC_LATC2_MASK
                #define DisplaySetData()            LATCSET = _LATC_LATC2_MASK

	            // Definitions for CS pin
                #define DisplayConfig()             TRISDCLR = _TRISD_TRISD10_MASK
                #define DisplayEnable()             LATDCLR = _LATD_LATD10_MASK
                #define DisplayDisable()            LATDSET = _LATD_LATD10_MASK

                // Definitions for FLASH CS pin
                #define DisplayFlashConfig()        TRISDCLR = _TRISD_TRISD1_MASK     
                #define DisplayFlashEnable()        LATDCLR = _LATD_LATD1_MASK
                #define DisplayFlashDisable()       LATDSET = _LATD_LATD1_MASK

                // Definitions for POWER ON pin
                #define DisplayPowerConfig()        TRISCCLR = _TRISC_TRISC3_MASK
                #define DisplayPowerOn()            LATCSET = _LATC_LATC3_MASK 
                #define DisplayPowerOff()           LATCCLR = _LATC_LATC3_MASK

                // Extended address lines
                #define DisplayAddr0Config()        TRISDCLR = _TRISD_TRISD3_MASK
                #define DisplaySetAddr0()           LATDCLR = _LATD_LATD3_MASK
                #define DisplayClearAddr0()         LATDSET = _LATD_LATD3_MASK

                #define DisplayAddr17Config()       TRISGCLR = _TRISD_TRISG14_MASK
                #define DisplaytSetAddr17()         LATGCLR = _LATD_LATG14_MASK
                #define DisplayClearAddr17()        LATGSET = _LATD_LATG14_MASK
            #endif

    #else
        #error GRAPHICS CONTROLLER IS NOT SUPPORTED
    #endif // (DISPLAY_CONTROLLER == ...
#elif defined (GFX_PICTAIL_V3)
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)

        // Definitions for reset pin
        #define DisplayResetConfig()        TRISAbits.TRISA4 = 0    
        #define DisplayResetEnable()        LATAbits.LATA4 = 0
        #define DisplayResetDisable()       LATAbits.LATA4 = 1

        // Definitions for RS pin
        #define DisplayCmdDataConfig()      TRISAbits.TRISA1 = 0
        #define DisplaySetCommand()         LATAbits.LATA1 = 0
        #define DisplaySetData()            LATAbits.LATA1 = 1

        // Definitions for CS pin
        #define DisplayConfig()             TRISBbits.TRISB15 = 0             
        #define DisplayEnable()             LATBbits.LATB15 = 0
        #define DisplayDisable()            LATBbits.LATB15 = 1

        // Definitions for FLASH CS pin
        #define DisplayFlashConfig()       
        #define DisplayFlashEnable()       
        #define DisplayFlashDisable()      

        // Definitions for POWER ON pin
        #define DisplayPowerConfig()       
        #define DisplayPowerOn()           
        #define DisplayPowerOff()          

    #else

        #ifndef __PIC32MX__
            // Definitions for reset pin
            #define DisplayResetConfig()        TRISCbits.TRISC1 = 0    
            #define DisplayResetEnable()        LATCbits.LATC1 = 0
            #define DisplayResetDisable()       LATCbits.LATC1 = 1

            // Definitions for RS pin
            #define DisplayCmdDataConfig()      TRISCbits.TRISC2 = 0
            #define DisplaySetCommand()         LATCbits.LATC2 = 0
            #define DisplaySetData()            LATCbits.LATC2 = 1

            // Definitions for CS pin
            #define DisplayConfig()             TRISDbits.TRISD10 = 0             
            #define DisplayEnable()             LATDbits.LATD10 = 0
            #define DisplayDisable()            LATDbits.LATD10 = 1

            // Definitions for FLASH CS pin
            #define DisplayFlashConfig()         
            #define DisplayFlashEnable()        
            #define DisplayFlashDisable()       

            // Definitions for POWER ON pin
            #define DisplayPowerConfig()        
            #define DisplayPowerOn()            
            #define DisplayPowerOff()           
        #else
        /********
         * PIC32 will use the SET and CLR SFRs for the I/O.
         * These are atomic settings that are recommended when
         * modifying SFRs
         ********/
            // Definitions for reset pin
            #define DisplayResetConfig()        TRISCCLR = _TRISC_TRISC1_MASK    
            #define DisplayResetEnable()        LATCCLR = _LATC_LATC1_MASK
            #define DisplayResetDisable()       LATCSET = _LATC_LATC1_MASK

            // Definitions for RS pin
            #define DisplayCmdDataConfig()      TRISCCLR = _TRISC_TRISC2_MASK
            #define DisplaySetCommand()         LATCCLR = _LATC_LATC2_MASK
            #define DisplaySetData()            LATCSET = _LATC_LATC2_MASK

            // Definitions for CS pin
            #define DisplayConfig()             TRISDCLR = _TRISD_TRISD10_MASK
            #define DisplayEnable()             LATDCLR = _LATD_LATD10_MASK
            #define DisplayDisable()            LATDSET = _LATD_LATD10_MASK

            // Definitions for FLASH CS pin
            #define DisplayFlashConfig()          
            #define DisplayFlashEnable()        
            #define DisplayFlashDisable()       

            // Definitions for POWER ON pin
            #define DisplayPowerConfig()        
            #define DisplayPowerOn()            
            #define DisplayPowerOff()           
        #endif
    #endif
#elif defined (MULTI_MEDIA_BOARD_DM00123)
        #ifndef __PIC32MX__
            // Definitions for reset pin
            #define DisplayResetConfig()        TRISAbits.TRISA10 = 0    
            #define DisplayResetEnable()        LATAbits.LATA10 = 0
            #define DisplayResetDisable()       LATAbits.LATA10 = 1

            // Definitions for RS pin
            #define DisplayCmdDataConfig()      TRISBbits.TRISB10 = 0
            #define DisplaySetCommand()         LATBbits.LATB10 = 0
            #define DisplaySetData()            LATBbits.LATB10 = 1

            // Definitions for CS pin
            #define DisplayConfig()             TRISGbits.TRISG13 = 0             
            #define DisplayEnable()             LATGbits.LATG13 = 0
            #define DisplayDisable()            LATGbits.LATG13 = 1

            // Definitions for FLASH CS pin
            #define DisplayFlashConfig()         
            #define DisplayFlashEnable()        
            #define DisplayFlashDisable()       

            // Definitions for POWER ON pin
            #define DisplayPowerConfig()        
            #define DisplayPowerOn()            
            #define DisplayPowerOff()           
        #else
        /********
         * PIC32 will use the SET and CLR SFRs for the I/O.
         * These are atomic settings that are recommended when
         * modifying SFRs
         ********/
            // Definitions for reset pin
            #define DisplayResetConfig()        TRISACLR = _TRISA_TRISA10_MASK    
            #define DisplayResetEnable()        LATACLR = _LATA_LATA10_MASK
            #define DisplayResetDisable()       LATASET = _LATA_LATA10_MASK

            // Definitions for RS pin
            #define DisplayCmdDataConfig()      AD1PCFGSET = _AD1PCFG_PCFG10_MASK, TRISBCLR = _TRISB_TRISB10_MASK
            #define DisplaySetCommand()         LATBCLR = _LATB_LATB10_MASK
            #define DisplaySetData()            LATBSET = _LATB_LATB10_MASK

            // Definitions for CS pin
            #define DisplayConfig()             TRISGCLR = _TRISG_TRISG13_MASK
            #define DisplayEnable()             LATGCLR = _LATG_LATG13_MASK
            #define DisplayDisable()            LATGSET = _LATG_LATG13_MASK

            // Definitions for FLASH CS pin
            #define DisplayFlashConfig()          
            #define DisplayFlashEnable()        
            #define DisplayFlashDisable()       

            // Definitions for POWER ON pin
            #define DisplayPowerConfig()        
            #define DisplayPowerOn()            
            #define DisplayPowerOff()           
        #endif

#elif defined (PIC24FJ256DA210_DEV_BOARD)
	
    // Definitions for POWER ON pin
    #define DisplayPowerConfig()        TRISAbits.TRISA5 = 0       
    #define DisplayPowerOn()            LATAbits.LATA5 = 1           
    #define DisplayPowerOff()           LATAbits.LATA5 = 0

#endif //  defined (GFX_PICTAIL_V2)...

/*********************************************************************
* IO FOR THE BEEPER
*********************************************************************/
#if defined (GFX_PICTAIL_V2) 
	#if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
		#define BEEP_TRIS_BIT   TRISBbits.TRISB4
		#define BEEP_LAT_BIT    LATBbits.LATB4
	#else
		#define BEEP_TRIS_BIT   TRISDbits.TRISD0
		#define BEEP_LAT_BIT    LATDbits.LATD0
	#endif
#elif defined (GFX_PICTAIL_V3) || defined (PIC24FJ256DA210_DEV_BOARD)
	#define BEEP_TRIS_BIT   TRISDbits.TRISD0
	#define BEEP_LAT_BIT    LATDbits.LATD0
#endif // #if defined (GFX_PICTAIL_V2)

/*********************************************************************
* IOS FOR THE FLASH/EEPROM SPI
*********************************************************************/
#if defined (EXPLORER_16)
	// When using Explorer 16, the on board EEPROM SPI (25LC256) is present 
	// so we define the chip select signal used. This is needed even 
	// if the memory is not used. Set to de-asserted state to avoid conflicts
	// when not used.
    #if  defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__)
        #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
            #define EEPROM_CS_TRIS  TRISAbits.TRISA0
            #define EEPROM_CS_LAT   LATAbits.LATA0
        #elif defined(__PIC24FJ256GB110__)
			// This PIM has RD12 rerouted to RG0
            #define EEPROM_CS_TRIS  TRISGbits.TRISG0
            #define EEPROM_CS_LAT   LATGbits.LATG0
        #else
            #define EEPROM_CS_TRIS  TRISDbits.TRISD12
            #define EEPROM_CS_LAT   LATDbits.LATD12
        #endif
    #elif defined(__PIC32MX__)
        #define EEPROM_CS_TRIS  TRISDbits.TRISD12
        #define EEPROM_CS_LAT   LATDbits.LATD12
    #endif
    
    #if defined (USE_25LC256)
	    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
	        #define EEPROM_SCK_TRIS TRISCbits.TRISC2
	        #define EEPROM_SDO_TRIS TRISCbits.TRISC0
	        #define EEPROM_SDI_TRIS TRISCbits.TRISC1
	    #else
	        #define EEPROM_SCK_TRIS TRISGbits.TRISG6
	        #define EEPROM_SDO_TRIS TRISGbits.TRISG8
	        #define EEPROM_SDI_TRIS TRISGbits.TRISG7
	    #endif    
    #endif // #if defined (USE_25LC256)
#endif // #if defined (EXPLORER_16)

#if defined (GFX_PICTAIL_V2) 
	#define USE_SST25_SPI2
	// IOs used are defined above (see #if defined (EXPLORER_16) above)
#elif defined (GFX_PICTAIL_V3)
	#define USE_SST25_SPI2
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
        #define SST25_CS_TRIS   TRISAbits.TRISA8
        #define SST25_CS_LAT    LATAbits.LATA8
        #define SST25_SCK_TRIS  TRISCbits.TRISC2
        #define SST25_SDO_TRIS  TRISCbits.TRISC0
        #define SST25_SDI_TRIS  TRISCbits.TRISC1
    #else
        #define SST25_CS_TRIS   TRISDbits.TRISD1
        #define SST25_CS_LAT    LATDbits.LATD1
        #define SST25_SCK_TRIS  TRISGbits.TRISG6
        #define SST25_SDO_TRIS  TRISGbits.TRISG8
        #define SST25_SDI_TRIS  TRISGbits.TRISG7
		#define SST25_SDI_ANS   ANSGbits.ANSG7
    #endif
#elif defined (PIC24FJ256DA210_DEV_BOARD)
	#define USE_SST25_SPI2
    #define SST25_CS_TRIS   TRISAbits.TRISA14
    #define SST25_CS_LAT    LATAbits.LATA14
    #define SST25_SCK_TRIS  TRISDbits.TRISD8
    #define SST25_SDO_TRIS  TRISBbits.TRISB1
    #define SST25_SDI_TRIS  TRISBbits.TRISB0
    #define SST25_SDI_ANS   ANSBbits.ANSB0
    #define SST25_SDO_ANS   ANSBbits.ANSB1
#elif defined (MULTI_MEDIA_BOARD_DM00123)

	#if defined (PIC32_GP_SK_DM320001) || defined (PIC32_USB_SK_DM320003_1)  
		#define USE_SST25_SPI2
	#elif defined (PIC32_USB_SK_DM320003_2)
		#define USE_SST25_SPI2A
	#elif defined (PIC32_ENET_SK_DM320004)
		#define USE_SST25_SPI3A
	#else
		#error "Please define the starter kit that you are using"
	#endif

	// define the CPLD SPI selection and chip select
	#ifdef USE_SST25_SPI2
		#define SST25_CS_TRIS   TRISGbits.TRISG9
		#define SST25_CS_LAT    LATGbits.LATG9
		#define SPI_FLASH_CHANNEL   CPLD_SPI2
	#elif defined (USE_SST25_SPI2A)
		#define SST25_CS_TRIS   TRISGbits.TRISG9
		#define SST25_CS_LAT    LATGbits.LATG9
		#define SPI_FLASH_CHANNEL   CPLD_SPI2A
	#elif defined (USE_SST25_SPI3A)
		#define SST25_CS_TRIS   TRISFbits.TRISF12
		#define SST25_CS_LAT    LATFbits.LATF12
		#define SPI_FLASH_CHANNEL   CPLD_SPI3A
	#else
		#error "SPI Channel can't be used for SPI Flash"
	#endif

#endif

/*********************************************************************
* IOS FOR THE GRAPHICS PICTAIL PARALLEL FLASH MEMORY
*********************************************************************/
#if defined(GFX_PICTAIL_V2)
	#if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__) 
		#define SST39_CS_TRIS    TRISAbits.TRISA8
		#define SST39_CS_LAT     LATAbits.LATA8
		#define SST39_A18_TRIS   TRISAbits.TRISA1
		#define SST39_A18_LAT    LATAbits.LATA1
		#define SST39_A17_TRIS   TRISCbits.TRISC5
		#define SST39_A17_LAT    LATCbits.LATC5
		#define SST39_A16_TRIS   TRISCbits.TRISC4
		#define SST39_A16_LAT    LATCbits.LATC4
	#else
		#define SST39_CS_TRIS    TRISDbits.TRISD1
		#define SST39_CS_LAT     LATDbits.LATD1
		#define SST39_A18_TRIS   TRISCbits.TRISC2
		#define SST39_A18_LAT    LATCbits.LATC2
		#define SST39_A17_TRIS   TRISGbits.TRISG15
		#define SST39_A17_LAT    LATGbits.LATG15
		#define SST39_A16_TRIS   TRISGbits.TRISG14
		#define SST39_A16_LAT    LATGbits.LATG14
	#endif
#endif

/*********************************************************************
* IOS FOR THE TOUCH SCREEN
*********************************************************************/

// ADC Status
#define TOUCH_ADC_DONE			AD1CON1bits.DONE

// ADC channel constants
#if defined (PIC24FJ256DA210_DEV_BOARD)
	#define ADC_TEMP    4
	#define ADC_POT     5
#elif defined (MULTI_MEDIA_BOARD_DM00123)
	/* The Multi-Media Development Board does not have a
     * analog pot or temperature sensor (see BMA150 for temperature
     * sensor data
     */
#else
	#if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
		#define ADC_TEMP        4
		#define ADC_POT         0
		#define ADC_POT_TRIS    TRISAbits.TRISA0
		#define ADC_POT_PCFG    AD1PCFGLbits.PCFG0
	#elif defined(__PIC32MX__)
		#define ADC_TEMP    ADC_CH0_POS_SAMPLEA_AN4
		#define ADC_POT     ADC_CH0_POS_SAMPLEA_AN5
	#else
		#define ADC_TEMP    4
		#define ADC_POT     5
	#endif
#endif 

#if defined (GFX_PICTAIL_V2)
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
        #define ADC_XPOS    5
        #define ADC_YPOS    4
    #elif defined(__PIC32MX__)
        #define ADC_XPOS    ADC_CH0_POS_SAMPLEA_AN11
        #define ADC_YPOS    ADC_CH0_POS_SAMPLEA_AN10
    #else
        #define ADC_XPOS    11
        #define ADC_YPOS    10
    #endif
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)

		// Y port definitions
        #define ADPCFG_XPOS AD1PCFGLbits.PCFG5
        #define LAT_XPOS    LATBbits.LATB3
        #define LAT_XNEG    LATCbits.LATC9
        #define TRIS_XPOS   TRISBbits.TRISB3
        #define TRIS_XNEG   TRISCbits.TRISC9

		// X port definitions
        #define ADPCFG_YPOS AD1PCFGLbits.PCFG4
        #define LAT_YPOS    LATBbits.LATB2
        #define LAT_YNEG    LATCbits.LATC8
        #define TRIS_YPOS   TRISBbits.TRISB2
        #define TRIS_YNEG   TRISCbits.TRISC8

    #else

		// Y port definitions
        #define ADPCFG_XPOS AD1PCFGbits.PCFG11
        #define LAT_XPOS    LATBbits.LATB11
        #define LAT_XNEG    LATGbits.LATG13
        #define TRIS_XPOS   TRISBbits.TRISB11
        #define TRIS_XNEG   TRISGbits.TRISG13

		// X port definitions
        #define ADPCFG_YPOS AD1PCFGbits.PCFG10
        #define LAT_YPOS    LATBbits.LATB10
        #define LAT_YNEG    LATGbits.LATG12
        #define TRIS_YPOS   TRISBbits.TRISB10
        #define TRIS_YNEG   TRISGbits.TRISG12
    #endif
#elif defined (GFX_PICTAIL_V3)
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
        #define ADC_XPOS    5
        #define ADC_YPOS    4
    #elif defined(__PIC32MX__)
        #define ADC_XPOS    ADC_CH0_POS_SAMPLEA_AN11
        #define ADC_YPOS    ADC_CH0_POS_SAMPLEA_AN10
    #else
        #define ADC_XPOS    11
        #define ADC_YPOS    10
    #endif
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)

		// Y port definitions
        #define ADPCFG_XPOS AD1PCFGLbits.PCFG5
        #define LAT_XPOS    LATBbits.LATB3
        #define LAT_XNEG    LATCbits.LATC9
        #define TRIS_XPOS   TRISBbits.TRISB3
        #define TRIS_XNEG   TRISCbits.TRISC9

		// X port definitions
        #define ADPCFG_YPOS AD1PCFGLbits.PCFG4
        #define LAT_YPOS    LATBbits.LATB2
        #define LAT_YNEG    LATCbits.LATC8
        #define TRIS_YPOS   TRISBbits.TRISB2
        #define TRIS_YNEG   TRISCbits.TRISC8

    #else

		// Y port definitions
        #define ADPCFG_XPOS AD1PCFGbits.PCFG11
        #define LAT_XPOS    LATBbits.LATB11
        #define TRIS_XPOS   TRISBbits.TRISB11

        #if defined(_USB) && !defined (PIC32_USB_SK_DM320003_1) && !defined (PIC32_ENET_SK_DM320004)
            #define LAT_XNEG    LATGbits.LATG15
            #define TRIS_XNEG   TRISGbits.TRISG15
        #else
            #define LAT_XNEG    LATDbits.LATD9
            #define TRIS_XNEG   TRISDbits.TRISD9
        #endif

		// X port definitions
        #define ADPCFG_YPOS AD1PCFGbits.PCFG10
        #define LAT_YPOS    LATBbits.LATB10
        #define LAT_YNEG    LATDbits.LATD8
        #define TRIS_YPOS   TRISBbits.TRISB10
        #define TRIS_YNEG   TRISDbits.TRISD8
    #endif
#elif defined (PIC24FJ256DA210_DEV_BOARD)
    #if defined(__PIC24FJ256DA210__)
        #define ADC_XPOS    16
        #define ADC_YPOS    18

		// Y port definitions
        #define ADPCFG_XPOS ANSCbits.ANSC4
        #define LAT_XPOS    LATCbits.LATC4
        #define TRIS_XPOS   TRISCbits.TRISC4
        #define LAT_XNEG    LATAbits.LATA2
        #define TRIS_XNEG   TRISAbits.TRISA2
     
		// X port definitions
        #define ADPCFG_YPOS ANSGbits.ANSG7
        #define LAT_YPOS    LATGbits.LATG7
        #define TRIS_YPOS   TRISGbits.TRISG7
    	#define LAT_YNEG    LATAbits.LATA1
        #define TRIS_YNEG   TRISAbits.TRISA1
    #endif
#elif defined (MULTI_MEDIA_BOARD_DM00123)
	#define ADC_XPOS    ADC_CH0_POS_SAMPLEA_AN11
	#define ADC_YPOS    ADC_CH0_POS_SAMPLEA_AN14

	// X port definitions
	#define ADPCFG_XPOS AD1PCFGbits.PCFG11
	#define LAT_XPOS    LATBbits.LATB11
	#define TRIS_XPOS   TRISBbits.TRISB11
	#define LAT_XNEG    LATBbits.LATB13
	#define TRIS_XNEG   AD1PCFGbits.PCFG13 = 1, TRISBbits.TRISB13

	// Y port definitions
	#define ADPCFG_YPOS AD1PCFGbits.PCFG14
	#define LAT_YPOS    LATBbits.LATB14
	#define TRIS_YPOS   TRISBbits.TRISB14
	#define LAT_YNEG    LATBbits.LATB12 
	#define TRIS_YNEG   AD1PCFGbits.PCFG12 = 1, TRISBbits.TRISB12
#endif

/*********************************************************************
* IOS FOR THE SWITCHES (SIDE BUTTONS)
*********************************************************************/
typedef enum
{
    HW_BUTTON_PRESS = 0,
    HW_BUTTON_RELEASE = 1
}HW_BUTTON_STATE;

#if defined (PIC24FJ256DA210_DEV_BOARD)
    #if defined(__PIC24FJ256DA210__)
        #define HardwareButtonInit()
        #define GetHWButtonTouchCal()       (PORTGbits.RG8)
        #define GetHWButtonProgram()        (PORTEbits.RE9)
        #define GetHWButtonScanDown()       (HW_BUTTON_RELEASE)
        #define GetHWButtonScanUp()         (HW_BUTTON_RELEASE) 
        #define GetHWButtonCR()             (PORTGbits.RG8)
        #define GetHWButtonFocus()          (PORTEbits.RE9)
      #endif 
 
#elif defined (MULTI_MEDIA_BOARD_DM00123)
    #define HardwareButtonInit()        (AD1PCFGSET = _AD1PCFG_PCFG1_MASK | _AD1PCFG_PCFG0_MASK | _AD1PCFG_PCFG3_MASK | _AD1PCFG_PCFG4_MASK | _AD1PCFG_PCFG15_MASK,\
                                         CNPUESET = _CNPUE_CNPUE2_MASK | _CNPUE_CNPUE3_MASK | _CNPUE_CNPUE5_MASK | _CNPUE_CNPUE6_MASK | _CNPUE_CNPUE12_MASK)
    #define GetHWButtonTouchCal()       (PORTBbits.RB15)
    #define GetHWButtonProgram()        (PORTBbits.RB15)
    #define GetHWButtonScanDown()       (PORTBbits.RB3)
    #define GetHWButtonScanUp()         (PORTBbits.RB1)  
    #define GetHWButtonCR()             (PORTBbits.RB15)
    #define GetHWButtonFocus()          (PORTBbits.RB0 & PORTBbits.RB4)
#else
    #if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
        #define HardwareButtonInit()
        #define GetHWButtonProgram()    (_RA9)
        #define GetHWButtonScanDown()   (_RA9)
        #define GetHWButtonScanUp()		(HW_BUTTON_RELEASE)
        #define GetHWButtonCR()			(HW_BUTTON_RELEASE)
        #define GetHWButtonFocus()  	(HW_BUTTON_RELEASE)
      #elif defined (PIC32_USB_SK_DM320003_1) || defined (PIC32_USB_SK_DM320003_2) || defined (PIC32_GP_SK_DM320001) 
        #ifdef USE_16BIT_PMP
            #define HardwareButtonInit()
            #define GetHWButtonProgram()        (HW_BUTTON_RELEASE)
            #define GetHWButtonTouchCal()       (HW_BUTTON_RELEASE)
            #define GetHWButtonScanDown()       (HW_BUTTON_RELEASE)
            #define GetHWButtonScanUp()         (HW_BUTTON_RELEASE)
            #define GetHWButtonCR()             (HW_BUTTON_RELEASE)
            #define GetHWButtonFocus()          (HW_BUTTON_RELEASE)
        #else
            #define HardwareButtonInit()        (CNPUESET = _CNPUE_CNPUE16_MASK | _CNPUE_CNPUE15_MASK | _CNPUE_CNPUE19_MASK)
            #define GetHWButtonTouchCal()       (PORTDbits.RD6)
            #define GetHWButtonProgram()        (HW_BUTTON_RELEASE)
            #define GetHWButtonScanDown()       (HW_BUTTON_RELEASE)
            #define GetHWButtonScanUp()         (HW_BUTTON_RELEASE)  
            #define GetHWButtonCR()             (PORTDbits.RD6)
            #define GetHWButtonFocus()          (PORTDbits.RD7 & PORTDbits.RD13)
        #endif
      #else
        #define HardwareButtonInit()
        #ifdef USE_16BIT_PMP
            #define GetHWButtonTouchCal()       (PORTAbits.RA7)
            #define GetHWButtonProgram()        (PORTAbits.RA7)
            #define GetHWButtonScanDown()       (HW_BUTTON_RELEASE)
            #define GetHWButtonScanUp()         (HW_BUTTON_RELEASE)
            #define GetHWButtonCR()             (HW_BUTTON_RELEASE)
            #define GetHWButtonFocus()          (HW_BUTTON_RELEASE)
        #else
            #define GetHWButtonTouchCal()       (PORTDbits.RD6)
            #define GetHWButtonProgram()        (PORTDbits.RD7)
            #define GetHWButtonScanDown()       (PORTDbits.RD13)
            #define GetHWButtonScanUp()         (PORTAbits.RA7)  
            #define GetHWButtonCR()             (PORTDbits.RD6)
            #define GetHWButtonFocus()          (PORTDbits.RD7)
        #endif
    #endif
#endif 

/*********************************************************************
* IOS FOR THE TCON 
*********************************************************************/
#if defined (PIC24FJ256DA210_DEV_BOARD)
	#if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E)
		#define CS_PORT     _RA0
		#define CS_TRIS     _TRISA0
		#define CS_DIG()
	    
		#define SCL_PORT    _RD8
		#define SCL_TRIS    _TRISD8
		#define SCL_DIG()
	    
		#define SDO_PORT    _RB1
		#define SDO_TRIS    _TRISB1
		#define SDO_DIG()   _ANSB1 = 0;
	    
		#define DC_PORT     _RB0
		#define DC_TRIS     _TRISB0
		#define DC_DIG()    _ANSB0 = 0;

	#endif // #if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E)
#endif // #if defined (DA210_DEV_BOARD)...

/*********************************************************************
* EPMP Settings 
*********************************************************************/
#ifdef PIC24FJ256DA210_DEV_BOARD

	#if defined (GFX_EPMP_CS1_BASE_ADDRESS)
		#define EPMPCS1_CS_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS1_WR_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS1_RD_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS1_BE_POLARITY	GFX_ACTIVE_LOW

		// macros to define the access timing of the parallel device in EPMP CS1
		#define EPMPCS1_DWAITB		0		// chip select 1 data set up before read/write strobe
		#define EPMPCS1_DWAITM		0		// chip select 1 read/write strobe wait states
		#define EPMPCS1_DWAITE		0		// chip select 1 data hold after read/write strobe
		#define EPMPCS1_AMWAIT		0		// chip select 1 Alternate Master wait state 

	#endif

	#if defined (GFX_EPMP_CS2_BASE_ADDRESS)
		#define EPMPCS2_CS_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS2_WR_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS2_RD_POLARITY	GFX_ACTIVE_LOW
		#define EPMPCS2_BE_POLARITY	GFX_ACTIVE_LOW

		// macros to define the access timing of the parallel device in EPMP CS2
		#define EPMPCS2_DWAITB		0		// chip select 2 data set up before read/write strobe
		#define EPMPCS2_DWAITM		4		// chip select 2 read/write strobe wait states
		#define EPMPCS2_DWAITE		0		// chip select 2 data hold after read/write strobe
		#define EPMPCS2_AMWAIT		3		// chip select 6 Alternate Master wait state 

	#endif
#endif

/*********************************************************************
* IOS FOR THE UART
*********************************************************************/
#if defined(__dsPIC33FJ128GP804__) || defined(__PIC24HJ128GP504__)
    #define TX_TRIS TRISCbits.TRISC1
    #define RX_TRIS TRISCbits.TRISC0
#elif defined(__PIC24FJ256DA210__)    
    #define TX_TRIS TRISFbits.TRISF3
    #define RX_TRIS TRISDbits.TRISD0
#else
    #define TX_TRIS TRISFbits.TRISF5
    #define RX_TRIS TRISFbits.TRISF4
#endif

/*********************************************************************
* RTCC DEFAULT INITIALIZATION (these are values to initialize the RTCC
*********************************************************************/
#define RTCC_DEFAULT_DAY        15      // 15th
#define RTCC_DEFAULT_MONTH      10      // October
#define RTCC_DEFAULT_YEAR       10      // 2010
#define RTCC_DEFAULT_WEEKDAY    05      // Friday
#define RTCC_DEFAULT_HOUR       10      // 10:10:01
#define RTCC_DEFAULT_MINUTE     10
#define RTCC_DEFAULT_SECOND     01

#if defined (MULTI_MEDIA_BOARD_DM00123)

#include <plib.h>

/*********************************************************************
* Configuration for the CPLD
*********************************************************************/
#ifdef USE_16BIT_PMP
#define GRAPHICS_HW_CONFIG     CPLD_GFX_CONFIG_16BIT
#else
#define GRAPHICS_HW_CONFIG     CPLD_GFX_CONFIG_8BIT
#endif

/*********************************************************************
* MMB LEDs
*********************************************************************/
typedef enum
{
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_10
}MMB_LED;

extern inline void __attribute__((always_inline)) SetLEDDirection(void)
{
   PORTSetPinsDigitalOut(IOPORT_D, (BIT_1 | BIT_2 | BIT_3));
    PORTSetPinsDigitalOut(IOPORT_C, (BIT_1 | BIT_2));
}

extern inline void __attribute__((always_inline)) TurnLEDOn(MMB_LED led)
{
    if(led == LED_2)
      PORTSetBits(IOPORT_D, BIT_1);

    if(led == LED_3)
      PORTSetBits(IOPORT_D, BIT_2);

    if(led == LED_4)
      PORTSetBits(IOPORT_D, BIT_3);

    if(led == LED_5)
      PORTSetBits(IOPORT_C, BIT_1);

    if(led == LED_10)
      PORTSetBits(IOPORT_C, BIT_2);

}

extern inline void __attribute__((always_inline)) TurnLEDOff(MMB_LED led)
{
    if(led == LED_2)
      PORTClearBits(IOPORT_D, BIT_1);

    if(led == LED_3)
      PORTClearBits(IOPORT_D, BIT_2);

    if(led == LED_4)
      PORTClearBits(IOPORT_D, BIT_3);

    if(led == LED_5)
      PORTClearBits(IOPORT_C, BIT_1);

    if(led == LED_10)
      PORTClearBits(IOPORT_C, BIT_2);

}

extern inline void __attribute__((always_inline)) ToggleLED(MMB_LED led)
{
    if(led == LED_2)
      PORTToggleBits(IOPORT_D, BIT_1);

    if(led == LED_3)
      PORTToggleBits(IOPORT_D, BIT_2);

    if(led == LED_4)
      PORTToggleBits(IOPORT_D, BIT_3);

    if(led == LED_5)
      PORTToggleBits(IOPORT_C, BIT_1);

    if(led == LED_10)
      PORTToggleBits(IOPORT_C, BIT_2);

}

extern inline void __attribute__((always_inline)) TurnLEDAllOn(void)
{
      PORTSetBits(IOPORT_D, BIT_1);
      PORTSetBits(IOPORT_D, BIT_2);
      PORTSetBits(IOPORT_D, BIT_3);
      PORTSetBits(IOPORT_C, BIT_1);
      PORTSetBits(IOPORT_C, BIT_2);

}

extern inline void __attribute__((always_inline)) TurnLEDAllOff(void)
{
      PORTClearBits(IOPORT_D, BIT_1);
      PORTClearBits(IOPORT_D, BIT_2);
      PORTClearBits(IOPORT_D, BIT_3);
      PORTClearBits(IOPORT_C, BIT_1);
      PORTClearBits(IOPORT_C, BIT_2);

}

#endif // #ifdef (MULTI_MEDIA_BOARD_DM00123)

#endif // __HARDWARE_PROFILE_H


