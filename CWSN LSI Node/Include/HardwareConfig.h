/* 
 * File:   AutoConfByPlatformSelect.h
 * Author: agus
 *
 * Created on July 16, 2013, 1:08 PM
 */

#ifndef HW_BASICCONFIG_H
#define	HW_BASICCONFIG_H

//CHOOSE A PLATFORM
#define cNGD_PLATFORM
//#define FCD_Exp_PLATFORM
//#define MANUAL_PLATFORM

/**************************************************************************/
// ENABLE_CONSOLE will enable the print out on the hyper terminal this
// definition is very helpful in the debugging process
/**************************************************************************/
#define ENABLE_CONSOLE


#if defined cNGD_PLATFORM


//*********** CONSOLE INTERFACE *****************************************************
#if defined ENABLE_CONSOLE
#define DEBUG_UART6
//#define DEBUG_USB
#endif

//*********** INTERFACE N1 *****************************************************
#define MRF49XA_1

#define MRF49XA_1_IN_434
//#define MRF49XA_1_IN_868

#define MRF49XA_1_IN_SPI1
#define MRF49XA_1_USES_INT1

#define MIWI_0434_RI

//*********** INTERFACE N2 *****************************************************
#define MRF49XA_2

#define MRF49XA_2_IN_868

//#define MRF49XA_2_IN_434


#define MRF49XA_2_IN_SPI3
#define MRF49XA_2_USES_INT3

#define MIWI_0868_RI
//*********** INTERFACE N3 *****************************************************
#define MRF24J40

#define MRF24J40_IN_SPI4
#define MRF24J40_USES_INT4

#define MIWI_2400_RI

#elif defined FCD_Exp_PLATFORM


//*********** CONSOLE INTERFACE *****************************************************
#if defined ENABLE_CONSOLE
    #define DEBUG_UART4
#endif

//*********** INTERFACE N1 *****************************************************
#define MRF49XA_1
#define MRF49XA_1_IN_434

#define MRF49XA_1_IN_SPI2
#define MRF49XA_1_USES_INT1

#define MIWI_0434_RI

//*********** INTERFACE N2 *****************************************************
#define MRF24J40

#define MRF24J40_IN_SPI4
#define MRF24J40_USES_INT3

#define MIWI_2400_RI


#elif defined MANUAL_PLATFORM

//----------------------------------------------------------------------------//
// Definition of RADIO INTERFACES
//----------------------------------------------------------------------------//
#define MIWI_0434_RI     //Comment if it is not available in the target board.
//#define MIWI_0868_RI     //Comment if it is not available in the target board.
//#define MIWI_2400_RI     //Comment if it is not available in the target board.
//#define WIFI_2400_RI     //Comment if it is not available in the target board.

//----------------------------------------------------------------------------//
// Definition of CONSOLE CONFIGURATION
//----------------------------------------------------------------------------//
#if defined ENABLE_CONSOLE //|| defined STACK_USE_UART
//#define DEBUG_UART1       //IMPORTANT: Do not define if SPI3 is in use.
//#define DEBUG_UART2       //IMPORTANT: Do not define if SPI4 is in use.
//#define DEBUG_UART3       //IMPORTANT: Do not define if SPI2 is in use.
//#define DEBUG_UART4         //IMPORTANT: Do not define if SPI3 is in use.
//Use UART4 for debugging with the old node.
//#define DEBUG_UART5       //IMPORTANT: Do not define if SPI4 is in use.
//#define DEBUG_UART6       //IMPORTANT: Do not define if SPI2 is in use.
//#define DEBUG_USB
#endif

//--------------------------------------------------------------------------
// Definition of RF Transceivers.
//--------------------------------------------------------------------------
// #define MRF24J40
/**********************************************************************/
// Definition of MRF24J40 enables the application to use Microchip
// MRF24J40 2.4GHz IEEE 802.15.4 compliant RF transceiver. ONLY ONE RF
// transceiver of this type can be defined.
/**********************************************************************/

#define MRF49XA_1   //If needed, define this one first.
#if defined MRF49XA_1
//#define MRF49XA_2   //Then, if needed, define this one too.
#endif
/**********************************************************************/
// Definition of MRF49XA enables the application to use Microchip
// MRF49XA sub_GHz proprietary RF transceiver. ONLY TWO RF transceivers
// of this type can be defined to operate in different frequency bands.
// This transceiver is designed for 434 MHz or 868 MHz (or 915MHz USA)
/**********************************************************************/
#if !defined MRF49XA_2
//#define MRF89XA
//MICROCHIP_LSI STACK NOT FULLY ADAPTED YET! DO NOT USE MRF89XA!
#endif
/**********************************************************************/
// Definition of MRF89XA enables the application to use Microchip
// MRF89XA sub_GHz proprietary RF transceiver. Only one RF transceiver
// of this type can be defined.
// This transceiver is designed for 868 MHz (915MHz USA & 955MHz opt.)
// This transceiver is not available when TWO MRF49XAs are included.
/**********************************************************************/
// #define MRF24WB0M
/****************** WIFI *************/


//----------------------------------------------------------------------------//
// FREQUENCY BANDS
//----------------------------------------------------------------------------//

#if defined MRF49XA_1
/**************************************************************************/
// FREQUENCY BAND: BAND_915, BAND_868 or BAND_434 are three supported
// frequency bands for Microchip MRF49XA. Only one of them must be defined.
// As long as there might be 2 MRF49XAs, the variables definitions can't be
// named as in MiWi original stack in order to make the distinction. In the
// case of 2 MRF49XA transceivers, they must define different bands.
/**************************************************************************/
//#define MRF49XA_1_IN_915
//#define MRF49XA_1_IN_868
#define MRF49XA_1_IN_434
#endif

#if defined MRF49XA_2 //ONLY FOR HAVING TWO MRF49XAs
/**************************************************************************/
// FREQUENCY BAND: BAND_915, BAND_868 or BAND_434 are three supported
// frequency bands for Microchip MRF49XA. Only one of them must be defined.
// As long as there might be 2 MRF49XAs, the variables definitions can't be
// named as in MiWi original stack in order to make the distinction. In the
// case of 2 MRF49XA transceivers, they must define different bands.
/**************************************************************************/
//#define MRF49XA_2_IN_915
#define MRF49XA_2_IN_868
//#define MRF49XA_2_IN_434
#endif

#if defined (MRF89XA) //MICROCHIP LSI CWSN STACK NOT ADAPTED YET!!
/**************************************************************************/
// FREQUENCY BAND: BAND_902, BAND_915 or BAND_863 (or BAND_950 - circuit
// dependent) are three supported frequency band for Microchip MRF89XA. Only
// one of them must be defined
/**************************************************************************/
//#define BAND_902      //Choose BAND_902 and BAND_915 for FCC and IC
//Supports frequencies between 902 - 915MHz
//#define BAND_915        //Supports frequencies between 915 - 928MHz
#define BAND_863      //Choose this for Europe ETSI 868MHz Freq. band
//Supports frequencies between 863MHz - 870MHz
#endif

//----------------------------------------------------------------------------//
// BOARD CONFIGURATION
//----------------------------------------------------------------------------//

#if defined MRF49XA_1
// CHOOSE ONLY ONE. The other transceivers must use a different SPI module.
//#define MRF49XA_1_IN_SPI1     //Module not available in PIC32MX795F512H
#define MRF49XA_1_IN_SPI2       //Use this for Guilja's extension board, conn. slot 1
//#define MRF49XA_1_IN_SPI3
//#define MRF49XA_1_IN_SPI4

// CHOOSE ONLY ONE. The other transceivers must use another external interrupt.
//#define MRF49XA_1_USES_INT0
#define MRF49XA_1_USES_INT1     //Use this for  Guilja's extension board, conn. slot 1
//#define MRF49XA_1_USES_INT2
//#define MRF49XA_1_USES_INT3
//#define MRF49XA_1_USES_INT4
#endif
#if defined MRF49XA_2
// CHOOSE ONLY ONE. The other transceivers must use a different SPI module.
//#define MRF49XA_2_IN_SPI1     //Module not available in PIC32MX795F512H
//#define MRF49XA_2_IN_SPI2
//#define MRF49XA_2_IN_SPI3
//#define MRF49XA_2_IN_SPI4

// CHOOSE ONLY ONE. The other transceivers must use another external interrupt.
//#define MRF49XA_2_USES_INT0
//#define MRF49XA_2_USES_INT1
//#define MRF49XA_2_USES_INT2
//#define MRF49XA_2_USES_INT3
//#define MRF49XA_2_USES_INT4
#endif
#if defined MRF89XA         //NOT ADAPTED FOR MICROCHIP_LSI MIWI STACK YET.
// CHOOSE ONLY ONE. The other transceivers must use a different SPI module.
//#define MRF89XA_IN_SPI1       //Module not available in PIC32MX795F512H
//#define MRF89XA_IN_SPI2
//#define MRF89XA_IN_SPI3
//#define MRF89XA_IN_SPI4

// CHOOSE ONLY ONE. The other transceivers must use another external interrupt.
//#define MRF89XA_USES_INT0
//#define MRF89XA_USES_INT1
//#define MRF89XA_USES_INT2
//#define MRF89XA_USES_INT3
//#define MRF89XA_USES_INT4
#endif
#if defined MRF24J40
// CHOOSE ONLY ONE. The other transceivers must use a different SPI module.
//#define MRF24J40_IN_SPI1      //Module not available in PIC32MX795F512H
//#define MRF24J40_IN_SPI2
//#define MRF24J40_IN_SPI3
#define MRF24J40_IN_SPI4        //Use this for the "old" node. (by F. Lopez)

// CHOOSE ONLY ONE. The other transceivers must use another external interrupt.
//#define MRF24J40_USES_INT0
//#define MRF24J40_USES_INT1
//#define MRF24J40_USES_INT2
#define MRF24J40_USES_INT3      //Use this for the "old" node. (by F. Lopez)
//#define MRF24J40_USES_INT4
#endif
#if defined MRF24WB0M
//CHOOSE ONLY ONE. The other transceivers must use a different SPI module.

//#define MRF24WB0M_IN_SPI1
//#define MRF24WB0M_IN_SPI2     //Use this for the "old" node. (by F. Lopez)
#define MRF24WB0M_IN_SPI3
//#define MRF24WB0M_IN_SPI4

//#define MRF24WB0M_USES_INT0
//#define MRF24WB0M_USES_INT1   //Use this for the "old" node. (by F. Lopez)
#define MRF24WB0M_USES_INT2
//#define MRF24WB0M_USES_INT3
//#define MRF24WB0M_USES_INT4
#endif
#endif




/*****************AUTOMATIC PROCESSED DEFINITION. DONT MODIFY****************/

#if defined MRF49XA_1_IN_SPI1 || defined MRF49XA_2_IN_SPI1 || \
    defined MRF89XA_IN_SPI1   || defined MRF24J40_IN_SPI1  || \
    defined MRF24WB0M_IN_SPI1

    #define SPI1_IN_USE

#endif

#if defined MRF49XA_1_IN_SPI2 || defined MRF49XA_2_IN_SPI2 || \
    defined MRF89XA_IN_SPI2   || defined MRF24J40_IN_SPI2  || \
    defined MRF24WB0M_IN_SPI2

    #define SPI2_IN_USE

#endif

#if defined MRF49XA_1_IN_SPI3 || defined MRF49XA_2_IN_SPI3 || \
    defined MRF89XA_IN_SPI3   || defined MRF24J40_IN_SPI3  || \
    defined MRF24WB0M_IN_SPI3

    #define SPI3_IN_USE

#endif

#if defined MRF49XA_1_IN_SPI4 || defined MRF49XA_2_IN_SPI4 || \
    defined MRF89XA_IN_SPI4   || defined MRF24J40_IN_SPI4  || \
    defined MRF24WB0M_IN_SPI4

    #define SPI4_IN_USE

#endif

#if defined MRF49XA_1_USES_INT1 || defined MRF49XA_2_USES_INT1 || \
    defined MRF89XA_USES_INT1   || defined MRF24J40_USES_INT1  || \
    defined MRF24WB0M_USES_INT1

    #define INT1_IN_USE

#endif

#if defined MRF49XA_1_USES_INT2 || defined MRF49XA_2_USES_INT2 || \
    defined MRF89XA_USES_INT2   || defined MRF24J40_USES_INT2  || \
    defined MRF24WB0M_USES_INT2

    #define INT2_IN_USE

#endif

#if defined MRF49XA_1_USES_INT3 || defined MRF49XA_2_USES_INT3 || \
    defined MRF89XA_USES_INT3   || defined MRF24J40_USES_INT3  || \
    defined MRF24WB0M_USES_INT3

    #define INT3_IN_USE

#endif

#if defined MRF49XA_1_USES_INT4 || defined MRF49XA_2_USES_INT4 || \
    defined MRF89XA_USES_INT4   || defined MRF24J40_USES_INT4  || \
    defined MRF24WB0M_USES_INT4

    #define INT4_IN_USE

#endif

#endif	/* AUTOCONFBYPLATFORMSELECT_H */


