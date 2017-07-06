/*******************************************************************************
 * File:   NodeHAL.c
 * Author: Juan Domingo Rebollo - Laboratorio de Sistemas Integrados (LSI) - UPM
 *
 * File Description: Node Hardware Abstraction Level.
 * Implements an API for application and cognitive layers.
 * It's the top level of the LSI-CWSN Microchip MiWi Stack.
 *
 * Change History:
 * Rev   Date         Description
 ******************************************************************************/
/* INCLUDES *******************************************************************/
#include "NodeHAL.h"            //LSI. HAL Definitions, data types, functions
#include "Compiler.h"           //General MCHP.

//USB                   //NOT ADAPTED
//#include "USB/usb.h"                //Stacks
//#include "usb_config.h"             //Stacks
//#include "USB/usb_device.h"         //Stacks
//#include "uart2.h"                  //Stacks
//#include "CircularBuffer.h"         //LSI. Circular buffer helper.
//#include "USB/usb_function_cdc.h"   //Stacks

//MIWI
#include "WirelessProtocols/ConfigApp.h"    //Stacks. LSI MiWi stack config.
#include "WirelessProtocols/Console.h"      //Stacks. RS232 (UART) for debugging
#include "WirelessProtocols/SymbolTime.h"   //Stacks. Timing issues.
#include "Transceivers/Security.h"          //Stacks. Ciphering, security...
#include "WirelessProtocols/MCHP_API.h"     //Stacks. MiWi Stack App. Interface

//WIFI
#ifdef WIFI_2400_RI     //NOT ADAPTED
    #include "TCPIP Stack/TCPIPConfig MRF24WB0M.h"  //Stacks. TCPIP stack config
    #include "TCPIP Stack/TCPIP.h"          //Stacks. File includes & config.
    #include "TCPIP Stack/WF_Config.h"      //Stacks. MRF24WB0M driver, passwds...
    #include "TCPIP Stack/WFEasyConfig.h"   //Stacks.
    #include "TCPIP Stack/WFApi.h"
    #if defined (STACK_USE_ZEROCONF_LINK_LOCAL)
        #include "TCPIP Stack/ZeroconfLinkLocal.h"
    #endif
    #if defined (STACK_USE_ZEROCONF_MDNS_SD)
        #include "TCPIP Stack/ZeroconfMulticastDNS.h"
    #endif
    #if defined (WF_CONSOLE)
        #include "TCPIP Stack/WFConsole.h"  //Stacks
        #include "WiFi/IperfApp.h"          //Stacks
    #endif
#endif

/* END INCLUDES ***************************************************************/

/* CONFIGURATION **************************************************************/
#if defined(__32MX795F512H__) || defined (__32MX795F512L__) || (__32MX675F256L__)  //LSI TARGET BOARD.
    //DEVCFG3 - USERID = No Setting ------------------------------------------//
    //DEVCFG2 ----------------------------------------------------------------//
    //SYS_CLK = 8 MHz (XT Ext. Ref. Osc.) * 20 (FPLLMUL) / 2 (FPLLIDIV) = 80 MHz
    #pragma config FPLLIDIV = DIV_2  //PLL Input Divider: /2 Divider
    #pragma config FPLLODIV = DIV_1  //SYSCLK PLL Output Divider: /1 Divider
    #pragma config FPLLMUL = MUL_20  //PLL Multiplier: x20 Multiplier
    #pragma config UPLLIDIV = DIV_2 //USB PLL Input Divider: /12 Divider
    #pragma config UPLLEN = ON     //USB PLL Enable: Disabled and Bypassed  //AGUS - ON
    // DEVCFG1 ---------------------------------------------------------------//
    #pragma config FNOSC = PRIPLL    //Oscillator Selection: Primary + PLL *
  #if defined WAKE_FROM_SLEEP_SOSC_T1
      #pragma config FSOSCEN  = ON   //Secondary Oscillator Enable: Enabled
  #else
      #pragma config FSOSCEN  = OFF  //Secondary Oscillator Enable: Disabled
  #endif
    #pragma config IESO = OFF        //Internal/External Switch Over: Disabled
    #pragma config POSCMOD = HS      //Primary Oscillator Config.: Crystal
    #pragma config OSCIOFNC = OFF    //CLKO Enable Configuration: Disabled
    #pragma config FPBDIV = DIV_4    //Peripheral Bus CLK Divider: /4 Divider
    #pragma config FCKSM = CSDCMD    //CLK Switching and FSCLK Monitor: Disabled
    // (*) WDTPS (For SOSC_FREQ = 32kHz, PSx => T = x ms, x=2^n, n from 1 to 20)
    #pragma config WDTPS = PS4096   //Watchdog Timer Postscaler (1:4096) (*)
    #pragma config FWDTEN = OFF      //Watchdog Timer Enable: Disabled (*)
    //DEVCFG0 ----------------------------------------------------------------//
    #pragma config DEBUG = ON        //Background Debugger Enable: Enabled
    #pragma config ICESEL = ICS_PGx1 //ICE/ICD Comm Channel Select: PGED1/PGEC1
    #pragma config PWP = OFF         //Program Flash Write Protect: Disabled
    #pragma config BWP = OFF         //Boot Flash Write Protect bit: Disabled
    #pragma config CP = OFF          //Code Protect: Disabled
#else
    #error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif
/* END CONFIGURATION **********************************************************/

/* DEFINITIONS ****************************************************************/
#if defined MIWI_0434_RI
    #define MIWI0434_TX_BUF_SIZE    MRF49XA_TX_BUF_SIZE
#endif
#if defined MIWI_0868_RI
    #define MIWI0868_TX_BUF_SIZE    MRF49XA_TX_BUF_SIZE
#endif
#if defined MIWI_2400_RI
    #define MIWI2400_TX_BUF_SIZE    MRF24J40_TX_BUF_SIZE
#endif
#if defined WIFI_2400_RI
    #define WIFI2400_TX_BUF_SIZE    1450    //M�x: 1500(Max.MAC)-20(IP)-20(TCP)
    #define WF_MODULE_NUMBER        WF_MODULE_NODE_HAL //Used for Wi-Fi assertions
#endif

//SLEEP TIME RANGE DEFINITION
#if defined WAKE_FROM_SLEEP_SOSC_T1
    //16-bit timer, 256 timer PS max, 32-bit SleepEventCounter => 2^(16+8+32)
    #define MAX_SLPTIME_MS ((UINT64)1<<56)/(1000*SOSC_FREQ_HZ)
    #if (SOSC_FREQ_HZ >= 1000)
        #define MIN_SLPTIME_MS (1000/SOSC_FREQ_HZ)
    #else
        #define MIN_SLPTIME_MS 1
    #endif
#elif defined WAKE_FROM_SLEEP_WDT // watchdog
    #define MAX_SLPTIME_MS ((UINT64)1<<32)*(1<<ReadPostscalerWDT())
    #define MIN_SLPTIME_MS (1<<ReadPostscalerWDT())
#endif
/* END DEFINITIONS ************************************************************/

/* VARIABLES ******************************************************************/
//HAL
nodeStatus NodeStatus;
UINT32 SleepEventCounter;
//unsigned int coreTMRvals[11];
BYTE coreTMRptr;

//MIWI
#if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
    //MIWI
    /**************************************************************************/
    //AdditionalNodeID variable array defines the additional information to
    //identify a device on a P2P connection. This array will be transmitted with
    //the P2P_CONNECTION_REQUEST command to initiate the connection between the
    //two devices. Along with the long address of this device, this variable
    //array will be stored in the P2P Connection Entry structure of the partner
    //device. The size of this array is ADDITIONAL_CONNECTION_PAYLOAD, defined
    //in ConfigApp.h.
    /**************************************************************************/
    #if ADDITIONAL_NODE_ID_SIZE > 0
        BYTE AdditionalNodeID [ADDITIONAL_NODE_ID_SIZE] =
        {(BYTE)(0x00FF&MyOwnDeviceAddress), (BYTE)(0x00FF&(MyOwnDeviceAddress >>8))};
        //LSI: 2 positions for storing the device address.
    #endif
#endif

#ifdef MIWI_0434_RI
static BYTE MIWI0434_PowerOutput;
static BYTE MIWI0434_datacount;     //Data bytes written in TX buffer
static BYTE MIWI0434_payloadToRead; //Data bytes to be read in RX buffer
static UINT16 MIWI0434_sentPckts;   //MIWI 434 MHz sent packets. Statistics
static UINT16 MIWI0434_procPckts;   //MIWI 434 MHz processed packets. Statistics
#endif
#ifdef MIWI_0868_RI
static BYTE MIWI0868_PowerOutput;
static BYTE MIWI0868_datacount;     //Data bytes written in TX buffer
static BYTE MIWI0868_payloadToRead; //Data bytes to be read in RX buffer
static UINT16 MIWI0868_sentPckts;   //MIWI 868 MHz sent packets. Statistics
static UINT16 MIWI0868_procPckts;   //MIWI 868 MHz processed packets. Statistics
#endif
#ifdef MIWI_2400_RI
static BYTE MIWI2400_PowerOutput;
static BYTE MIWI2400_datacount;     //Data bytes written in TX buffer
static BYTE MIWI2400_payloadToRead; //Data bytes to be read in RX buffer
static UINT16 MIWI2400_sentPckts;   //MIWI 2,4 GHz sent packets. Statistics
static UINT16 MIWI2400_procPckts;   //MIWI 2,4 GHz processed packets. Statistics
#endif


//WIFI
#ifdef WIFI_2400_RI
    //Declare AppConfig structure and some other supporting stack variables
    APP_CONFIG AppConfig;
    //Checksum of the ROM defaults for AppConfig
    static unsigned short wOriginalAppConfigChecksum;
    BYTE AN0String[8];
    extern BYTE myLongAddress[];
    static MAC_ADDR MyMACAddr;
    static IP_ADDR IPAddr;
    UDP_SOCKET skt;
    NODE_INFO sensorInfo;

    static BYTE WIFI2400_datacount;
    static BYTE WIFI2400_payloadToRead;
    static UINT16 WIFI2400_sentPckts;
    static UINT16 WIFI2400_procPckts;
    static BYTE WIFI2400_PowerOutput;
#endif
/* END VARIABLES **************************************************************/

/* INTERNAL FUNCTIONS DECLARATION *********************************************/
//Initialization
static void InitVariables();

//Low Power Mode
static void TimedPICSleep();

//Stacks Maintenance
#if defined NODE_DOES_MAINTENANCE_TASKS
    static void AllStacksTasks();
#endif

//MiWi Interfaces
#if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
    static BYTE InitMIWI();
    static BYTE SendMIWI(miwi_band mb, BOOL isBroadcast, BYTE *Address, BOOL isLongAddr);
#endif

//WiFi Interface
#if defined WIFI_2400_RI
    static BYTE InitWIFI();
    static void InitBoardWIFI();
    static void InitAppConfig();
    static BYTE SendWIFI();
    static BOOL WIFIPcktAvailable();
    //#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS) || defined (EZ_CONFIG_STORE)
    static void RestoreWifiConfig();            //Depends on config.
    void WF_Connect();
#endif

////////////////////////////////////////////////////////////////////////////////
/****************   HAL FUNCTIONS (FOR THE APPLICATION CODE)   ****************/
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Function: InitNode()
 * Input:    None
 * Output:   NO_ERROR if success. HAL error code otherwise.
 * Overview: Node initialization function. PCB_Board, WIFI and MIWI stacks,
 *           default configuration, etc.
 ******************************************************************************/
BYTE InitNode(){
    InitVariables();  // SW variables.

    //#if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
        BoardInit();            //MIWI hardware.
        ConsoleInit();          //PIC ---(UART - RS232)---> PC (debugging).
    //#endif


    INTEnableSystemMultiVectoredInt();




    //Radio Interfaces Protocols Initialization
    #if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
        InitMIWI();
    #endif
    #if defined (WIFI_2400_RI)
	InitWIFI();
    #endif

    //Enable Auto-Maintenance Tasks if the node is in charge of them.
    #if defined NODE_DOES_MAINTENANCE_TASKS
        T1CONSET = 0x8000;  //Enable timer 1 for periodic stacks maintenance.
                            //Check T1 Period in HardwareProfile.h
    #endif
    return NO_ERROR;
}

/*******************************************************************************
 * Function:    GetStatusFlags()
 * Input:       None
 * Output:      Returns the byte containing the status flags.
 * Overview:    Simple function to get (read) the status flags.
 ******************************************************************************/
BYTE GetStatusFlags(){
    return  NodeStatus.flags.statusFlags;
}

UINT16 GetSentPckts(radioInterface ri){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0434_sentPckts;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0868_sentPckts;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI2400_sentPckts;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return WIFI2400_sentPckts;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Choose a single radioInterface.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

UINT16 GetProcPckts(radioInterface ri){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0434_procPckts;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0868_procPckts;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI2400_procPckts;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return WIFI2400_procPckts;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Choose a single radioInterface.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}


/*******************************************************************************
 * Function:    GetOpChannel(radioInterface ri)
 * Input:       radioInterface ri - The radio interface whose operating channel
 *                                  will be returned. NONE, ALL_MIWI and ALL are
 *                                  not valid options for this function.
 * Output:      No Error: A byte containing the number of the operating channel
 *              of the radio interface selected.
 *              Error: HAL error code.
 * Overview:    This function is invoked when the app. code needs to know which
 *              channel is in use by a single radio interface.
 ******************************************************************************/
BYTE GetOpChannel(radioInterface ri){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NodeStatus.MIWI0434_OpChannel;
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NodeStatus.MIWI0868_OpChannel;
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NodeStatus.MIWI2400_OpChannel;
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NodeStatus.WIFI2400_OpChannel;
            #endif

        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Operating Channel of each Radio Interface must be "
                   "accessed one by one.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetScanResult(radioInterface ri,BYTE channel, BYTE *storeItHere)
 * Input:       radioInterface ri - radio interface chosen.
 *                                  NONE, ALL_MIWI and ALL are not valid options
 *              BYTE channel      - the channel chosen. It has to be a channel
 *                                  within the valid range.
 *              BYTE *storeItHere - where the RSSI value will be stored.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use this function to read the RSSI channel noise value stored
 *              after the last channel scanning. If channel hasn't been scanned
 *              yet, the result will still have the initial zero value.
 ******************************************************************************/
BYTE GetScanResult(radioInterface ri, BYTE channel, BYTE *storeItHere){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI0434NumChannels){
                    *storeItHere = NodeStatus.scanMIWI0434[channel];
                    return NO_ERROR;
                }
                else return INVALID_CHANNEL_ERROR;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI0868NumChannels){
                    *storeItHere = NodeStatus.scanMIWI0868[channel];
                    return NO_ERROR;
                }
                else return INVALID_CHANNEL_ERROR;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI2400NumChannels){
                    *storeItHere = NodeStatus.scanMIWI2400[channel];
                    return NO_ERROR;
                }
                else return INVALID_CHANNEL_ERROR;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < WIFI2400NumChannels){
                    *storeItHere = NodeStatus.scanWIFI2400[channel];
                    return NO_ERROR;
                }
                else return INVALID_CHANNEL_ERROR;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Scan results of each Radio Interface must be "
                   "accessed one by one.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetPayloadToRead(radioInterface ri)
 * Input:       radioInterface ri - radio interface chosen. NONE is not valid.
 * Output:      Number of bytes pending to be read if success. HAL error code
 *              otherwise.
 * Overview:    If success, this function returns how many payload bytes haven't
 *              been read by the application code (from the RX buffer of the
 *              selected radio interface). Invoking this function with "ALL" or
 *              "ALL_MIWI" returns the sum of unread payload data from all or
 *              all miwi radio interfaces available.
 *              IMPORTANT! If output = 127 (0x01111111), there are AT LEAST 127
 *              pending bytes to be read (there could be more than 127).
 *              Outputs > 127 are for HAL error codes in case of failure.
 ******************************************************************************/
BYTE GetPayloadToRead(radioInterface ri){
    UINT16 aux;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0434_payloadToRead;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI0868_payloadToRead;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return MIWI2400_payloadToRead;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return WIFI2400_payloadToRead;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            //Return the sum of all pending bytes to be read.
            aux = 0;
            #ifdef MIWI_0434_RI
                aux += MIWI0434_payloadToRead;
            #endif
            #ifdef MIWI_0868_RI
                aux += MIWI0868_payloadToRead;
            #endif
            #ifdef MIWI_2400_RI
                aux += MIWI2400_payloadToRead;
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
                    aux += WIFI2400_payloadToRead;
                #endif
            }
            if(aux < 128)           //0b0XXXXXXX => Not an error code value.
                return (BYTE)aux;
            else
                return 0x7F;        //127 or more bytes pending. 0b01111111
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetRSSI(radioInterface ri, BYTE *storeItHere)
 *              RSSI: received signal strength (power) indication (in dBm)
 * Input:       radioInterface ri - radio interface chosen.
 *                                  NONE, ALL_MIWI and ALL are not valid options
 *              BYTE *storeItHere - byte where the RSSI will be stored.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use GetRSSI function to read the RSSI value of the last received
 *              packet in a single radio interface. Once the packet is discarded
 *              the RSSI value is not available, so be sure of invoking this
 *              function before reading the last payload byte with GetRXData.
 ******************************************************************************/
BYTE GetRSSI(radioInterface ri, BYTE *storeItHere){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0434_RXbuf_isEmpty){
                    return UNAVAILABLE_RSSI_ERROR;
                }
                else{
                    *storeItHere = MIWI0434_rxMessage.PacketRSSI;
                    return NO_ERROR;
                }
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0868_RXbuf_isEmpty){
                    return UNAVAILABLE_RSSI_ERROR;
                }
                else{
                    *storeItHere = MIWI0868_rxMessage.PacketRSSI;
                    return NO_ERROR;
                }
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI2400_RXbuf_isEmpty){
                    return UNAVAILABLE_RSSI_ERROR;
                }
                else{
                    *storeItHere = MIWI2400_rxMessage.PacketRSSI;
                    return NO_ERROR;
                }
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NO_ERROR;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Select a single Radio Interface to get the RSSI "
                   "value of its last received packet.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetLQI(radioInterface ri, BYTE *storeItHere)
 *              link quality indicator
 * Input:       radioInterface ri - radio interface chosen.
 *                                  NONE, ALL_MIWI and ALL are not valid options
 *              BYTE *storeItHere - byte where the RSSI will be stored.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use GetLQI function to read the LQI value of the last received
 *              packet in a single radio interface. Once the packet is discarded
 *              the LQI value is not available, so be sure of invoking this
 *              function before reading the last payload byte with GetRXData.
 ******************************************************************************/
BYTE GetLQI(radioInterface ri, BYTE *storeItHere){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0434_RXbuf_isEmpty){
                    return UNAVAILABLE_LQI_ERROR;
                }
                else{
                    *storeItHere = MIWI0434_rxMessage.PacketLQI;
                    return NO_ERROR;
                }
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0868_RXbuf_isEmpty){
                    return UNAVAILABLE_LQI_ERROR;
                }
                else{
                    *storeItHere = MIWI0868_rxMessage.PacketLQI;
                    return NO_ERROR;
                }
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI2400_RXbuf_isEmpty){
                    return UNAVAILABLE_LQI_ERROR;
                }
                else{
                    *storeItHere = MIWI2400_rxMessage.PacketLQI;
                    return NO_ERROR;
                }
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return NO_ERROR;
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Select a single Radio Interface to get the LQI "
                   "value of its last received packet.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetFreeTXBufSpace(radioInterface ri)
 * Input:       radioInterface ri - radio interface chosen.
 *                                  NONE, ALL_MIWI and ALL are not valid options
 * Output:      Number of free positions in TX buffer or HAL error code
 * Overview:    Returns how many free positions are left in the TX buffer of the
 *              selected radio interface in case of success or HAL error code if
 *              failure.
 ******************************************************************************/
BYTE GetFreeTXBufSpace(radioInterface ri){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return (MIWI0434_TX_BUF_SIZE - MIWI0434_datacount);
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return (MIWI0868_TX_BUF_SIZE - MIWI0868_datacount);
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return (MIWI2400_TX_BUF_SIZE - MIWI2400_datacount);
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                return (WIFI2400_TX_BUF_SIZE - WIFI2400_datacount);
            #endif
        case NONE:
            //NOP.
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Select a single Radio Interface to get the number "
                   "of free positions in its TX buffer.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    SleepRadioInterface(radioInterface ri)
 * Input:       radioInterface ri - radio interface chosen. NONE is not valid.
 * Output:      NO_ERROR if success. HAL error code, otherwise.
 * Overview:    This function is used for changing transceivers' power state to
 *              SLEEP mode. If the radio interface is not going to be used for a
 *              period of time, node can reduce power dissipation switching off
 *              its transceiver.
 ******************************************************************************/
BYTE SleepRadioInterface(radioInterface ri){
    BYTE aux;
    UINT8 spi_prev_ec;
    switch (ri){
        case MIWI_0434:
             #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 434 MHz must remain awake.");
                return AWAKE_IS_MANDATORY_ERROR;
            #else
                //Sleep MiWi at 434 MHz Interface
                if(NodeStatus.flags.bits.MIWI0434isON == 0){
                    Printf("\r\nMiWi at 434 MHz interface still remains asleep.");
                    return NO_ERROR;
                }
                else {
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_434);
                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI0434isON = 0;
                            //Printf("\r\nMiWi at 434 MHz interface is now asleep.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        case HAL_INTERNAL_ERROR:
                            return aux;
                        default:
                            aux = MIWI0434_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to sleep Miwi 434 MHz interface. "
                           "HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 868 MHz must remain awake.");
                return AWAKE_IS_MANDATORY_ERROR;
            #else
                //Sleep MiWi at 868 MHz Interface
                if(NodeStatus.flags.bits.MIWI0868isON == 0){
                    Printf("\r\nMiWi at 868 MHz interface still remains asleep.");
                    return NO_ERROR;
                }
                else {
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_868);
                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI0868isON = 0;
                            //Printf("\r\nMiWi at 868 MHz interface is now asleep.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        default:
                            aux = MIWI0868_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to sleep Miwi 868 MHz interface. "
                           "HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 2,4 GHz must remain awake.");
                return AWAKE_IS_MANDATORY_ERROR;
            #else
                //Sleep MiWi at 2,4 GHz Interface
                if(NodeStatus.flags.bits.MIWI2400isON == 0){
                    Printf("\r\nMiWi at 2,4 GHz interface still remains asleep.");
                    return NO_ERROR;
                }
                else {
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_2G4);
                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI2400isON = 0;
                            //Printf("\r\nMiWi at 2,4 GHz interface is now asleep.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        default:
                            aux = MIWI2400_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to sleep Miwi 2,4 GHz interface. "
                           "HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
//FALTAN COSAS!!!//Sleep WiFi Interface
                NodeStatus.flags.bits.WIFI2400isON = 0;
                return NO_ERROR;
            #endif
        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                #ifndef ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 434 MHz must remain awake.");
                #else
                //Sleep MiWi at 434 MHz Interface
                    if(NodeStatus.flags.bits.MIWI0434isON == 0){
                        Printf("\r\nMiWi at 434 MHz interface still remains asleep.");
                    }
                    else {
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_434);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI0434isON = 0;
                                //Printf("\r\nMiWi at 434 MHz interface is now asleep.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI0434_STACK_ERROR;
                                break;
                        }
                        if(aux != NO_ERROR){
                            Printf("\r\nNode failed to sleep Miwi 434 MHz interface. "
                                   "HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            #ifdef MIWI_0868_RI
                #ifndef ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 868 MHz must remain awake.");
                #else
                    //Sleep MiWi at 868 MHz Interface
                    if(NodeStatus.flags.bits.MIWI0868isON == 0){
                        Printf("\r\nMiWi at 868 MHz interface still remains asleep.");
                    }
                    else {
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_868);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI0868isON = 0;
                                //Printf("\r\nMiWi at 868 MHz interface is now asleep.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI0868_STACK_ERROR;
                                break;
                        }
                        if (aux != NO_ERROR){
                            Printf("\r\nNode failed to sleep Miwi 868 MHz interface. "
                                   "HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            #ifdef MIWI_2400_RI
                #ifndef ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 2,4 GHz must remain awake.");
                #else
                    //Sleep MiWi at 2,4 GHz Interface
                    if(NodeStatus.flags.bits.MIWI2400isON == 0){
                        Printf("\r\nMiWi at 2,4 GHz interface still remains asleep.");
                    }
                    else {
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_SLEEP, ISM_2G4);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI2400isON = 0;
                                //Printf("\r\nMiWi at 2,4 GHz interface is now asleep.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI2400_STACK_ERROR;
                                break;
                        }
                        if (aux != NO_ERROR){
                            Printf("\r\nNode failed to sleep Miwi 2,4 GHz interface. "
                                   "HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
//FALTAN COSAS!!!!//Sleep WiFi Interface
                    NodeStatus.flags.bits.WIFI2400isON = 0;
                #endif
            }
            return NO_ERROR;    //All interfaces are switched off now.
        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected to be "
                   "switched off");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    WakeUpRadioInterface(radioInterface ri)
 * Input:       radioInterface ri - Radio interface to be switched on. NONE is
 *                                  not a valid option for this function.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use WakeUpRadioInterface to change transceivers' power state
 *              from a sleep mode to operation mode.
 ******************************************************************************/
BYTE WakeUpRadioInterface(radioInterface ri){
    BYTE aux;
    UINT8 spi_prev_ec;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 434 MHz is always awake.");
                return NO_ERROR;
            #else
                //Wake up MiWi at 434 MHz Interface
                if(NodeStatus.flags.bits.MIWI0434isON == 1){
                    Printf("\r\nMiWi at 434 MHz is still awake.");
                    return NO_ERROR;
                }
                else{
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP_DR, ISM_434);
                    //MiWi interface offers a different way for waking up the
                    //node (POWER_STATE_WAKEUP_DR). This option also implies
                    //sending out a Data Request to its main associated device
                    //to ask for incoming data. See MiWi Documentation available
                    //for further information (AN1284, ...).

                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }

                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI0434isON = 1;
                            //Printf("\r\nMiWi at 434 MHz interface is now awake.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        default:
                            aux = MIWI0434_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to wake MiWi 434 MHz interface up."
                           " HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 868 MHz is always awake.");
                return NO_ERROR;
            #else
                //Wake up MiWi at 868 MHz Interface
                if(NodeStatus.flags.bits.MIWI0868isON == 1){
                    Printf("\r\nMiWi at 868 MHz is still awake.");
                    return NO_ERROR;
                }
                else{
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP_DR, ISM_868);
                    //MiWi interface offers a different way for waking up the
                    //node (POWER_STATE_WAKEUP_DR). This option also implies
                    //sending out a Data Request to its main associated device
                    //to ask for incoming data. See MiWi Documentation available
                    //for further information (AN1284, ...).

                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }

                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI0868isON = 1;
                            //Printf("\r\nMiWi at 868 MHz interface is now awake.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        default:
                            aux = MIWI0868_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to wake Miwi 868 MHz interface up."
                           " HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #elif !defined ENABLE_SLEEP
                Printf("\r\nDue to config issues, MiWi at 2,4 GHz is always awake.");
                return NO_ERROR;
            #else
                //Wake up MiWi at 2,4 GHz Interface
                if(NodeStatus.flags.bits.MIWI2400isON == 1){
                    Printf("\r\nMiWi at 2,4 GHz is still awake.");
                    return 0x00;
                }
                else{
                    spi_prev_ec = GetSPIErrorCounter();
                    aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP_DR, ISM_2G4);
                    //MiWi interface offers a different way for waking up the
                    //node (POWER_STATE_WAKEUP_DR). This option also implies
                    //sending out a Data Request to its main associated device
                    //to ask for incoming data. See MiWi Documentation available
                    //for further information (AN1284, ...).
                    if (GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    switch (aux){
                        case 0x00:     //SUCCESS
                            NodeStatus.flags.bits.MIWI2400isON = 1;
                            //Printf("\r\nMiWi at 2,4 GHz interface is now awake.");
                            return NO_ERROR;
                        case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                            aux = TRANSCEIVER_PM_ERROR;
                            break;
                        case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_TX_ERROR;
                            break;
                        case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                            aux = DATA_REQUEST_RX_ERROR;
                            break;
                        case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                            aux = INVALID_POWER_MODE_ERROR;
                            break;
                        default:
                            aux = MIWI2400_STACK_ERROR;
                            break;
                    }
                    Printf("\r\nNode failed to wake Miwi 2,4 GHz interface up."
                           " HAL error code: ");
                    PrintChar(aux);
                    return aux;
                }
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
//Wake up Wifi Interface
                NodeStatus.flags.bits.WIFI2400isON = 1;
                return NO_ERROR;
            #endif

        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                #if !defined ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 434 MHz is always awake.");
                #else
                    //Wake up MiWi at 434 MHz Interface
                    if(NodeStatus.flags.bits.MIWI0434isON == 1){
                        Printf("\r\nMiWi at 434 MHz is still awake.");
                    }
                    else{
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP, ISM_434);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI0434isON = 1;
                                //Printf("\r\nMiWi at 434 MHz interface is now awake.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI0434_STACK_ERROR;
                                break;
                        }
                        if(aux != NO_ERROR){
                            Printf("\r\nNode failed to wake MiWi 434 MHz interface up."
                                   " HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            #ifdef MIWI_0868_RI
                #if !defined ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 868 MHz is always awake.");
                #else
                    //Wake up MiWi at 868 MHz Interface
                    if(NodeStatus.flags.bits.MIWI0868isON == 1){
                        Printf("\r\nMiWi at 868 MHz is still awake.");
                        return NO_ERROR;
                    }
                    else{
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP, ISM_868);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI0868isON = 1;
                                //Printf("\r\nMiWi at 868 MHz interface is now awake.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI0868_STACK_ERROR;
                                break;
                        }
                        if(aux != NO_ERROR){
                            Printf("\r\nNode failed to wake Miwi 868 MHz interface up."
                                   " HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            #ifdef MIWI_2400_RI
                #if !defined ENABLE_SLEEP
                    Printf("\r\nDue to config issues, MiWi at 2,4 GHz is always awake.");
                #else
                    //Wake up MiWi at 2,4 GHz Interface
                    if(NodeStatus.flags.bits.MIWI2400isON == 1){
                        Printf("\r\nMiWi at 2,4 GHz is still awake.");
                        return 0x00;
                    }
                    else{
                        spi_prev_ec = GetSPIErrorCounter();
                        aux=MiApp_TransceiverPowerState(POWER_STATE_WAKEUP, ISM_2G4);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            return SPI_ERROR;
                        }
                        switch (aux){
                            case 0x00:     //SUCCESS
                                NodeStatus.flags.bits.MIWI2400isON = 1;
                                Printf("\r\nMiWi at 2,4 GHz interface is now awake.");
                                aux = NO_ERROR;
                                break;
                            case 0x01:     //ERR_TXR_FAIL [See MCHP_API.h]
                                aux = TRANSCEIVER_PM_ERROR;
                                break;
                            case 0x02:     //ERR_TX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_TX_ERROR;
                                break;
                            case 0x03:     //ERR_RX_FAIL [See MCHP_API.h]
                                aux = DATA_REQUEST_RX_ERROR;
                                break;
                            case 0xFF:      //ERR_INVALID_INPUT [See MCHP_API.h]
                                aux = INVALID_POWER_MODE_ERROR;
                                break;
                            default:
                                aux = MIWI2400_STACK_ERROR;
                                break;
                        }
                        if(aux != NO_ERROR){
                            Printf("\r\nNode failed to wake Miwi 2,4 GHz interface up."
                                   " HAL error code: ");
                            PrintChar(aux);
                            return aux;
                        }
                    }
                #endif
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
//Wake up Wifi Interface
                    NodeStatus.flags.bits.WIFI2400isON = 1;
                    return NO_ERROR;
                #endif
            }
            return NO_ERROR;

        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected to be "
                   "switched on");
            return INVALID_INTERFACE_ERROR;

        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

///*******************************************************************************
// * Function:    WakeUpNode(radioInterface ri)
// * Input:       radioInterface ri - Interfaces that will be turned on in the
// *                                  process. Every input option is valid.
// * Output:      NO_ERROR if success. HAL error code otherwise.
// * Overview:    Wake up the chosen radio interfaces and...
// ******************************************************************************/
//BYTE WakeUpNode(radioInterface ri){
//    BYTE aux;
//    UINT8 spi_prev_ec;
//    if(NodeStatus.flags.bits.NodeAsleep){
//        //Additional tasks for waking up the node.
//
//        //Switch on requested radio interfaces.
//        if (ri != NONE){
//            spi_prev_ec = GetSPIErrorCounter();
//            aux = WakeUpRadioInterface(ri);
//            if (GetSPIErrorCounter() != spi_prev_ec){
//                ResetSPIErrorCounter();
//                return SPI_ERROR;
//            }
//        }
//
//        if(aux != NO_ERROR){
//            return aux;
//        }
//
//        NodeStatus.flags.bits.NodeAsleep = 0;
//        return NO_ERROR;
//    }
//    else{
//        Printf("\r\nThe node is already awake.");
//        return NO_ERROR;
//    }
//}

/*******************************************************************************
 * Function:    SleepNode()
 * Input:       None
 * Output:      None
 * Overview:    Switch off all radio interfaces and...
 ******************************************************************************/
BYTE SleepNode(radioInterface forceWakeUp, UINT32 slpTime_ms){
    BYTE i, aux;
    if (NodeStatus.flags.bits.NodeAsleep == 0){
      //Check if slpTime_ms is within the valid range.
        if (slpTime_ms > MAX_SLPTIME_MS){
            return MAX_SLPTIME_EXCEEDED_ERROR;
        }else if (slpTime_ms < MIN_SLPTIME_MS){
            return MIN_SLPTIME_REQUIRED_ERROR;
        }
        aux = 0;
      //Switch off radio interfaces which don't wake up the node.
        #if defined MIWI_0434_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_0434_RI_MASK){
                aux = SleepRadioInterface(MIWI_0434);
            }
        #endif
        #if defined MIWI_0868_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_0868_RI_MASK){
                aux = SleepRadioInterface(MIWI_0868);
            }
        #endif
        #if defined MIWI_2400_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_2400_RI_MASK){
                aux = SleepRadioInterface(MIWI_2400);
            }
        #endif
        #if defined WIFI_2400_RI
            if((ALL-forceWakeUp) & WIFI_2400_RI_MASK){
                aux = SleepRadioInterface(WIFI_2400);
            }
        #endif
      //Disable interrupts for configuring enabled interruptions in sleep mode.
        INTDisableInterrupts();
        //Node interrupt sources: T1?, T2&3, T4?, SPI1?, SPI2?, SPI3?, SPI4?,
        //INT1?, INT2?, INT3?, INT4?, UARTx?, CN?, USB?...
        DWORD oldIEC0, oldIEC1, oldIEC2;
        //Save enabled interrupts state.
        oldIEC0 = IEC0;
        oldIEC1 = IEC1;
        oldIEC2 = IEC2;
        //Disable all single interrupts enable.
        IEC0CLR = 0xFFFFFFFF;
        IEC1CLR = 0xFFFFFFFF;
        IEC2CLR = 0xFFFFFFFF;
        //Clear all interrupts flag.
        IFS0CLR = 0xFFFFFFFF;
        IFS1CLR = 0xFFFFFFFF;
        IFS2CLR = 0xFFFFFFFF;
        //Set all the permitted radio interfaces' IE during sleep mode.
        #if defined MIWI_0434_RI
            if(forceWakeUp & MIWI_0434_RI_MASK){
                #if defined MRF49XA_1_IN_434
                    MRF49XA_1_IE = 1;
                #elif defined MRF49XA_2_IN_434
                    MRF49XA_2_IE = 1;
                #endif
            }
        #endif
        #if defined MIWI_0868_RI
            if(forceWakeUp & MIWI_0868_RI_MASK){
                #if defined MRF49XA_1_IN_868
                    MRF49XA_1_IE = 1;
                #elif defined MRF49XA_2_IN_868
                    MRF49XA_2_IE = 1;
                #endif
            }
        #endif
        #if defined MIWI_2400_RI
            if(forceWakeUp & MIWI_2400_RI_MASK){
                #if defined MRF24J40
                    MRF24J40_IE = 1;
                #endif
            }
        #endif
        #if defined WIFI_2400_RI
            if(forceWakeUp & WIFI_2400_RI_MASK){
                #if defined MRF24WB0MA
                    WF_INT_IE = 1;
                #endif
            }
        #endif

        //Configure Timer1 and/or Sleep Variables for exiting sleep mode after
        //the given time.
        UINT64 aux;
        SleepEventCounter = 1;
        T1CON = 0x0002;             //Disable timer, SOSC source, PS=1
        TMR1  = 0x0000;             //Reset count
        #if defined WAKE_FROM_SLEEP_SOSC_T1
            //T1 CLOCKED BY SOSC. SOSC FREQ. MUST BE A DEFINED CONSTANT
            //aux = Time count rounding .500 up and .499 down
            aux = (((UINT64)(SOSC_FREQ_HZ*slpTime_ms)+500)/1000);
            if (aux < (1<<16)){         //PS 1 is valid
                //There's no need to count T1 events. T1 can cover this range.
                T1CONbits.TCKPS = 0;    //Set PS to 1:1
                PR1 = (UINT16) aux;     //Load period (time count desired)
            } else{
                aux /= 8;               //Needed at least PS 8
                if (aux < (1<<16)){
                    //There's no need to count T1 events. It covers this range.
                    T1CONbits.TCKPS = 1;
                    PR1 = (UINT16) aux;     //Load period (time count updated)
                } else{
                    aux /= 8;               //Needed at least PS 64
                    if (aux < (1<<16)){
                        //No need to count T1 events. T1 can cover this range.
                        T1CONbits.TCKPS = 2;    //Set PS to 1:64
                        PR1 = (UINT16) aux;     //Period (time count updated)
                    } else{
                        aux /= 4;                   //Needed PS 256
                        T1CONbits.TCKPS = 3;        //Set PS to 1:256
                        if (aux < (1<<16)){
                            //There's no need to count T1 events, but we are
                            //getting closer to the limit...
                            PR1 = (UINT16) aux;
                        }else{
                            //We do need to count timer events...
                            SleepEventCounter = (UINT32)((aux>>16) + 1);
                            //We need (aux/(2^16)) events.
                            PR1 = aux/SleepEventCounter;    //Resulting period
                        }
                    }
                }
            }
            /*******************************************************************
             *Juan (!): There is a Hardware problem in the old node that has to
             *do with secondary oscillator capacitors. After a lot of debugging,
             *I've got the timer 1 to work properly, but 2 things must be done:
             *  1) Touch a Y1 pin with an oscilloscope probe. This is like a
             *     funny hand effect. In fact, if you touch the pin with your
             *     finger, secondary oscillator fades away.
             *  2) TSYNC bit has to be set to 1. This may be related to the HW
             *     issue, but it's strange. TSYNC bit is used to synchronize the
             *     secondary oscillator with the system clock (primary) and this
             *     clock is swiched off during sleep mode. The instruction below
             *     sets this bit in T1CON. Comment this line if the new node HW
             *     has succesfully addressed this issue.
             ******************************************************************/
            T1CONbits.TSYNC = 1;        //Strange, but needed for the old node.

            IEC0SET = 0x00000010;   //Set T1IE
        #elif defined WAKE_FROM_SLEEP_WDT
            //WDT CLOCKED BY LPRC (32 kHz typ.). WDTPS is related to the period:
            //T(ms) = (1<<ReadPostscalerWDT())   => aux = (slpTime_ms + T/2)/T
            aux = (slpTime_ms+(1<<(ReadPostscalerWDT()-1)))/(1<<ReadPostscalerWDT());
            if(aux){
                SleepEventCounter = aux;
            }
            //Else: SleepEventCounter has been set to 1 before.
        #endif

        //Set wake button interruption for manually exiting sleep mode.
        IEC1SET = 0x00000001;       //Enable the CN Interrupt.

      //Update Node status.
        NodeStatus.flags.bits.NodeAsleep = 1;

      //GO TO SLEEP!!!
        INTEnableInterrupts();
        TimedPICSleep();
        INTDisableInterrupts();
      //NODE IS AWAKE AND BACK!!!

      //Update Node status.
        NodeStatus.flags.bits.NodeAsleep = 0;
        SleepEventCounter = 0;

      //Restore all single interrupts enable.
        IEC0 = oldIEC0;
        IEC1 = oldIEC1;
        IEC2 = oldIEC2;
      //Switch on radio interfaces which were sleeping.
        aux = 0;
        #if defined MIWI_0434_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_0434_RI_MASK){
                aux = WakeUpRadioInterface(MIWI_0434);
            }
        #endif
        #if defined MIWI_0868_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_0868_RI_MASK){
                aux = WakeUpRadioInterface(MIWI_0868);
            }
        #endif
        #if defined MIWI_2400_RI && defined ENABLE_SLEEP
            if((ALL-forceWakeUp) & MIWI_2400_RI_MASK){
                aux = WakeUpRadioInterface(MIWI_2400);
            }
        #endif
        #if defined WIFI_2400_RI
            if((ALL-forceWakeUp) & WIFI_2400_RI_MASK){
                aux = WakeUpRadioInterface(WIFI_2400);
            }
        #endif

      //Restore Timer1 for stacks tasks if needed.
        #if defined NODE_DOES_MAINTENANCE_TASKS
            T1CON = 0x0070;             //Disable timer, PBCLK source, PS=256
            TMR1  = 0x0000;             //Reset count
            PR1   = MAINTENANCE_PERIOD; //Set period.
        #endif
      //Enable interrupts again and we're done!
        INTEnableInterrupts();
        #if defined NODE_DOES_MAINTENANCE_TASKS
            T1CONSET = 0x8000;
        #endif
        return NO_ERROR;
    }
    else{
        Printf("\r\nThe node is already asleep.");
        return ASLEEP_NODE_ERROR;
    }
}

/*******************************************************************************
 * Function:    SetTXPower(radioInterface ri, BYTE powerOutput)
 * Input:       radioInterface ri - The selected radio interface to change its
 *                                  output power. NONE, ALL_MIWI and ALL are
 *                                  invalid here.
 *              BYTE power        - The relative output power. 0x00 represents
 *                                  the maximum power output, whereas 0xFF means
 *                                  setting the minimum output power.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use this function to change the output power of a single radio
 *              interface.
 *              VERY IMPORTANT! Due to the different transceivers issues, the
 *              output power can only be adjusted to some fixed values. Thus,
 *              small changes in the input may produce no variation in output
 *              power. Generally speaking, there are equally spaced power steps.
 *              IMPORTANT! Dividing by two the input power level DOES NOT IMPLY
 *              doubling the output power. The function is inherently not linear
 *              because of the steps, but also because the step is not constant
 *              for some transceivers:
 *                  MRF49XAs: 8 values from 0 dBm to -17,5 dBm in 2,5 dB steps.
 *                  MRF24J40: 32 values from 0 dBm to -36,3 dBm in steps varying
 *                            from 0,5 dB up to 3,7 dB.
 *              INFO: The output power can be different from the theorical
 *                    values because of antenna circuit misadaptation.
 ******************************************************************************/
BYTE SetTXPower(radioInterface ri, BYTE power){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    BYTE aux;
    BOOL ok;
    UINT8 spi_prev_ec;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //Only 8 possible values. Shift and extract only the 3 MS bits
                aux = ((power >> 5) & 0x07);
                spi_prev_ec = GetSPIErrorCounter();
                #if defined MRF49XA_1_IN_434
                    ok = MiMAC_MRF49XA_SetPower(aux, 1);
                #elif defined MRF49XA_2_IN_434
                    ok = MiMAC_MRF49XA_SetPower(aux, 2);
                #endif
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if (ok){
                    //Printf("    HALPower = ");
                    //PrintChar(power);
                    MIWI0434_PowerOutput = (BYTE)(aux << 5);
                    return NO_ERROR;
                }
                Printf("\r\nError: MiWi at 434 MHz Power output was not changed.");
                return MIWI0434_STACK_ERROR;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //Only 8 possible values. Shift and extract only the 3 MS bits
                aux = ((power >> 5) & 0x07);
                spi_prev_ec = GetSPIErrorCounter();
                #if defined MRF49XA_1_IN_868
                    ok = MiMAC_MRF49XA_SetPower(aux, 1);
                #elif defined MRF49XA_2_IN_868
                    ok = MiMAC_MRF49XA_SetPower(aux, 2);
                #endif
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if (ok){
                    //Printf("    HALPower = ");
                    //PrintChar(power);
                    MIWI0868_PowerOutput = (BYTE)(aux << 5);
                    return NO_ERROR;
                }
                Printf("\r\nError: MiWi at 2,4 GHz Power output was not changed.");
                return MIWI0868_STACK_ERROR;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                spi_prev_ec = GetSPIErrorCounter();
                ok = MiMAC_MRF24J40_SetPower(power);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if (ok){
                    //Printf("    HALPower = ");
                    //PrintChar(power);
                    //Store the MAC level value (3 LS bits are neglected).
                    MIWI2400_PowerOutput = (power & 0xF8);
                    return NO_ERROR;
                }
                Printf("\r\nError: MiWi at 2,4 GHz Power output was not changed.");
                return MIWI2400_STACK_ERROR;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                // FALTAN COSAS AQU�
                return NO_ERROR;
            #endif
        case NONE:
            //NOP
            Printf("\r\nError: NONE of the Radio Interfaces were selected to set "
                    "its power output.");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Power Output of each Radio Interfaces must be set "
                   "one by one.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetTXPower(radioInterface ri, BYTE *storeItHere)
 * Input:       radioInterface ri - The selected radio interface to get its
 *                                  output power. NONE, ALL_MIWI and ALL are
 *                                  invalid here.
 *              BYTE *storeItHere - A byte where the power output will be stored
 *                                  if successful.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use this function to read the output power of a single radio
 *              interface.
 *              VERY IMPORTANT! Due to the different transceivers issues, the
 *              output power can only be adjusted to some fixed values. At HAL
 *              level, you can set the ouput power using any byte value. The
 *              value provided is truncated internally to one of the fixed
 *              values and stored by HAL. You read THE STORED VALUE, not the
 *              last byte value used for setting the output power.
 *              IMPORTANT! Keep in mind that power output increase from 0xFF to
 *              0x00. Remember also that dividing by two the power level DOES
 *              NOT IMPLY doubling the output power. The function is inherently
 *              not linear because of the steps, but also because the step is
 *              not constant for some transceivers:
 *              INFO: The output power can be different from the theorical
 *                    values because of antenna circuit misadaptation.
 ******************************************************************************/
BYTE GetTXPower(radioInterface ri, BYTE *storeItHere){
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                *storeItHere = MIWI0434_PowerOutput;
                return NO_ERROR;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                *storeItHere = MIWI0868_PowerOutput;
                return NO_ERROR;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                *storeItHere = MIWI2400_PowerOutput;
                return NO_ERROR;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                *storeItHere = WIFI2400_PowerOutput;
                return NO_ERROR;
            #endif
        case NONE:
            //NOP
            Printf("\r\nError: NONE of the Radio Interfaces were selected to get "
                    "its power output.");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Power Output of each Radio Interface must be read "
                   "one by one.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}


BYTE SwitchOnRI(radioInterface ri){
     switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
//                MRF49XA_1_PWR = 1;
                return NO_ERROR;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //MRF49XA_2_PWR = 1;
                return NO_ERROR;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                MRF24J40_PWR = 1;
                return NO_ERROR;
            #endif
        case NONE:
            //NOP
            Printf("\r\nError: NONE of the Radio Interfaces were selected to "
                    "switch its power on.");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
       //         MRF49XA_1_PWR = 1;
            #endif
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
             //   MRF49XA_2_PWR = 1;
            #endif
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                MRF24J40_PWR = 1;
            #endif
                return NO_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

BYTE SwitchOffRI(radioInterface ri){
        switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
         //       MRF49XA_1_PWR = 0;
                return NO_ERROR;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
            //    MRF49XA_2_PWR = 0;
                return NO_ERROR;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                MRF24J40_PWR = 0;
                return NO_ERROR;
            #endif
        case NONE:
            //NOP
            Printf("\r\nError: NONE of the Radio Interfaces were selected to "
                    "switch its power on.");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
        //        MRF49XA_1_PWR = 0;
            #endif
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
             //   MRF49XA_2_PWR = 0;
            #endif
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                MRF24J40_PWR = 0;
            #endif
                return NO_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

BYTE SwitchOnLed(BYTE led){
   switch(led){
       case 1:
           LED1 = 1;
           break;
       case 2:
           LED2 = 1;
           break;
       case 3:
           LED3 = 1;
           break;
       default:
           return INVALID_INPUT_ERROR;
           }

       return NO_ERROR;
}
BYTE SwitchOffLed(BYTE led){
    switch(led){
       case 1:
           LED1 = 0;
           break;
       case 2:
           LED2 = 0;
           break;
       case 3:
           LED3 = 0;
           break;
       default:
           return INVALID_INPUT_ERROR;
           }
       return NO_ERROR;
}

/*******************************************************************************
 * Function: PutTXData(radioInterface ri, BYTE data)
 * Input:    radioInterface ri - Radio Interface chosen (MIWI_0434, MIWI_0868,
 *                               MIWI_2400, ALL_MIWI, WIFI_2400, ALL)
 *           BYTE data         - Data payload to be written.
 * Output:   NO_ERROR if success; HAL error code if failure.
 * Overview: Writes a single byte in the transmission buffer of the selected
 *           radio interface. You don't have access to these internal buffers
 *           but you can "read" if there is enough free space by invoking the
 *           GetFreeTxBufSpace function.   .
 ******************************************************************************/
BYTE PutTXData(radioInterface ri, BYTE data){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI0434isON){
                    if (NodeStatus.MIWI0434_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI0434_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_434);
                                MIWI0434_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_434);
                        MIWI0434_datacount++;
                        if(MIWI0434_datacount >= MIWI0434_TX_BUF_SIZE){
                            NodeStatus.MIWI0434_TXbuf_isFull=TRUE;
                        }
                    }
                    else{
                        Printf("\r\nError: MiWi at 434 MHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 434 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI0868isON){
                    if (NodeStatus.MIWI0868_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI0868_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_868);
                                MIWI0868_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_868);
                        MIWI0868_datacount++;
                        if(MIWI0868_datacount >= MIWI0868_TX_BUF_SIZE)
                            NodeStatus.MIWI0868_TXbuf_isFull=TRUE;
                    }
                    else{
                        Printf("\r\nError: MiWi at 868 MHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 868 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI2400isON){
                    if (NodeStatus.MIWI2400_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI2400_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_2G4);
                                MIWI2400_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_2G4);
                        MIWI2400_datacount++;
                        if(MIWI2400_datacount >= MIWI2400_TX_BUF_SIZE)
                            NodeStatus.MIWI2400_TXbuf_isFull=TRUE;
                    }
                    else{
                        Printf("\r\nError: MiWi at 2,4 GHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 2,4 GHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.WIFI2400isON){
                    if (NodeStatus.WIFI2400_TXbuf_isFull == 0){
                        //Intentar escribir un dato en WIFI
                        if(UDPIsPutReady(skt) > 0){
                            UDPPut(data);
                            WIFI2400_datacount++;
                            if(WIFI2400_datacount >= WIFI2400_TX_BUF_SIZE)
                                NodeStatus.WIFI2400_TXbuf_isFull=TRUE;
                        }
                        else return WIFI_STACK_ERROR;
                    }
                    else{
                        Printf("\r\nError: WiFi TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: WiFi Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                if (NodeStatus.flags.bits.MIWI0434isON){
                    if (NodeStatus.MIWI0434_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI0434_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_434);
                                MIWI0434_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_434);
                        MIWI0434_datacount++;
                        if(MIWI0434_datacount >= MIWI0434_TX_BUF_SIZE)
                            NodeStatus.MIWI0434_TXbuf_isFull=TRUE;
                    }
                    else{
                        Printf("\r\nError: MiWi at 434 MHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 434 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            #ifdef MIWI_0868_RI
                if (NodeStatus.flags.bits.MIWI0868isON){
                    if (NodeStatus.MIWI0868_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI0868_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_868);
                                MIWI0868_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_868);
                        MIWI0868_datacount++;
                        if(MIWI0868_datacount >= MIWI0868_TX_BUF_SIZE)
                            NodeStatus.MIWI0868_TXbuf_isFull=TRUE;
                    }
                    else{
                        Printf("\r\nError: MiWi at 868 MHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 868 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            #ifdef MIWI_2400_RI
                if (NodeStatus.flags.bits.MIWI2400isON){
                    if (NodeStatus.MIWI2400_TXbuf_isFull == 0){
                        #if defined PROTOCOL_MIWI
                        if(MIWI2400_datacount == 0)
                            #if defined ENABLE_DUMMY_BYTE
                            {
                            //Write a dummy byte for the new packet. This is a
                            //fix to MiWi Stack, as the packets starting with 0
                            //are treated as stack packets instead of user data.
                                MiApp_WriteData(0xFF, ISM_2G4);
                                MIWI2400_datacount++;
                            }
                            #else
                            {
                                if(data == 0x00){
                                    return PAYLOAD_START_ERROR;
                                }
                            }
                            #endif
                        #endif
                        MiApp_WriteData(data, ISM_2G4);
                        MIWI2400_datacount++;
                        if(MIWI2400_datacount >= MIWI2400_TX_BUF_SIZE)
                            NodeStatus.MIWI2400_TXbuf_isFull=TRUE;
                    }
                    else{
                        Printf("\r\nError: MiWi at 2,4 GHz TX buffer is full");
                        return TX_BUFFER_FULL_ERROR;
                    }
                }
                else{
                    Printf("\r\nError: MiWi at 2,4 GHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
                    if (NodeStatus.flags.bits.WIFI2400isON){
                        if (NodeStatus.WIFI2400_TXbuf_isFull == 0){
                            //Intentar escribir un dato en WIFI
                            if(UDPIsPutReady(skt) > 0){
                                UDPPut(data);
                                WIFI2400_datacount++;
                                if(WIFI2400_datacount >= WIFI2400_TX_BUF_SIZE)
                                    NodeStatus.WIFI2400_TXbuf_isFull=TRUE;
                            }
                            else return WIFI_STACK_ERROR;
                        }
                        else{
                            Printf("\r\nError: WiFi TX buffer is full");
                            return TX_BUFFER_FULL_ERROR;
                        }
                    }
                    else{
                        Printf("\r\nError: WiFi Radio Interface is Asleep");
                        return ASLEEP_INTERFACE_ERROR;
                    }
                #endif
            }
            return NO_ERROR;
        case NONE:
            //NOP
            Printf("\r\nError: No data was written. NONE of the Radio Interfaces "
                   "were selected");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface. No data was written.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function: DiscardTXData(radioInterface ri)
 * Input:    radioInterface ri - Radio Interface chosen (MIWI_0434, MIWI_0868,
 *                               MIWI_2400, ALL_MIWI, WIFI_2400, ALL)
 * Output:   NO_ERROR if success; HAL error code if failure.
 * Overview: Discard all the user data stored in the selected radio interface TX
 *           buffer.   .
 ******************************************************************************/
BYTE DiscardTXData(radioInterface ri){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI0434isON){
                    MiApp_FlushTx(ISM_434);
                    MIWI0434_datacount = 0;
                    NodeStatus.MIWI0434_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 434 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI0868isON){
                    MiApp_FlushTx(ISM_868);
                    MIWI0868_datacount = 0;
                    NodeStatus.MIWI0868_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 868 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.MIWI2400isON){
                    MiApp_FlushTx(ISM_2G4);
                    MIWI2400_datacount = 0;
                    NodeStatus.MIWI2400_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 2,4 GHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.flags.bits.WIFI2400isON){
                    //Resetear el buffer o el puntero a la zona de datos.
                    WIFI2400_datacount = 0;
                    NodeStatus.WIFI2400_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: WiFi Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
                return NO_ERROR;
            #endif

        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                if (NodeStatus.flags.bits.MIWI0434isON){
                    MiApp_FlushTx(ISM_434);
                    MIWI0434_datacount = 0;
                    NodeStatus.MIWI0434_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 434 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            #ifdef MIWI_0868_RI
                if (NodeStatus.flags.bits.MIWI0868isON){
                    MiApp_FlushTx(ISM_868);
                    MIWI0868_datacount = 0;
                    NodeStatus.MIWI0868_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 868 MHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            #ifdef MIWI_2400_RI
                if (NodeStatus.flags.bits.MIWI2400isON){
                    MiApp_FlushTx(ISM_2G4);
                    MIWI2400_datacount = 0;
                    NodeStatus.MIWI2400_TXbuf_isFull = FALSE;
                }
                else{
                    Printf("\r\nError: MiWi at 2,4 GHz Radio Interface is Asleep");
                    return ASLEEP_INTERFACE_ERROR;
                }
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
                    if (NodeStatus.flags.bits.WIFI2400isON){
                       //Resetear el buffer o el puntero a la zona de datos.
                        WIFI2400_datacount = 0;
                        NodeStatus.WIFI2400_TXbuf_isFull = FALSE;
                    }
                    else{
                        Printf("\r\nError: WiFi Radio Interface is Asleep");
                        return ASLEEP_INTERFACE_ERROR;
                    }
                #endif
            }
            return NO_ERROR;
        case NONE:
            //NOP
            Printf("\r\nError: No data was written. NONE of the Radio Interfaces "
                   "were selected");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface. No data was written.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}
/*******************************************************************************
 * Function:    SendPckt(radioInterface ri, UINT16 devDestAddr)
 * Input:       radioInterface ri - Radio Interface chosen. NONE is not valid.
 *              addrMode AddrMode - What type of address is given. Options are:
 *                                  Broadcast: broadcast in chosen interface/s
 *                                  ShortMIWI: short (alternative) MiWi address.
 *                                  LongMIWI: long (permanent) MiWi address.
 *                                  //WIFI_IPv4: IPv4 address.
 *                                  //WIFI_MAC: WIFI MAC layer address
 *              BYTE *Address     - Pointer to address
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Once all the payload bytes have been written in the TX buffer,
 *              specify the destination address and trigger the transmission
 *              with SendPckt. If success, a radio packet will be transmitted
 *              and the TX buffer will be reset for the next transmission. Thus,
 *              retransmission is not allowed once the packet was sent.
 *              However, if an error occured, application may retry to send the
 *              packet as long as it is not transmitted. The TX buffer remains
 *              as when payload bytes were written.
 * Note:        Be careful not to invoke SendPacket instead of SendPckt, as it
 *              is a MiWi Stack Internal function. Compiler should notice it,
 *              anyway.
 ******************************************************************************/
BYTE SendPckt(radioInterface ri, BYTE AddrMode, BYTE *Address){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    BOOL isBroadcast = (AddrMode == BROADCAST_ADDRMODE)? TRUE: FALSE;
    BOOL isLongAddr = (AddrMode == LONG_MIWI_ADDRMODE)? TRUE: FALSE;
    //If ri is "ALL", address mode must be broadcast. Otherwise, node HAL can't
    //manage to send packets to WiFi and MiWi interfaces using the same address.
    if ((ri == ALL) && !isBroadcast){
        Printf("\r\nThe only way to send a packet through all radio interfaces is"
               "using broadcast mode.");
        return INVALID_ADDR_MODE_ERROR;
    }
    #if defined PROTOCOL_P2P
        if ((isBroadcast || isLongAddr) == FALSE){
            //Short address not valid for P2P
            return INVALID_ADDR_MODE_ERROR;
        }
    #endif

    //************************************************************************//
    //CLARA:
    //En esta primera parte que est� comentada, los dispositivos antes de mandar
    //  un mensaje comprueban su tabla de conexiones. S�lo si tiene al nodo
    //  receptor registrado se lo env�a.
    //
    //Simplemente se ha usado para hacer pruebas de manejo de tabla de conexiones
    //  y comprobar la informaci�n almacenada. En la parte inferior se ha
    //  mantenido el c�digo original, donde simplemente se env�a.
    //************************************************************************//
/*
#if defined PROTOCOL_P2P

    BOOL aux;
    BYTE i;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 434 MHz band

                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x01){
                            aux = SendMIWI(ISM_434, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 434 MHz transmission. Data discarded.");
                                MIWI0434_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 434 MHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif


        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 868 MHz band
                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x02){
                            aux = SendMIWI(ISM_868, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 868 MHz transmission. Data discarded.");
                                MIWI0868_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 868 MHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2.4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 2.4 GHz band
                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x04){
                            aux = SendMIWI(ISM_2G4, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 2.4 GHz transmission. Data discarded.");
                                MIWI2400_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 2.4 GHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xD0){                        //MiWi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Trigger the transmission!
                aux = SendWIFI();
                //Update statistics.

                return DiscardTXData(ri);   //Discard successfully sent pckt
                return aux;
            #endif
        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                //Try to send a MiWi packet with the stored data in 434 MHz band
                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x01){
                            aux = SendMIWI(ISM_434, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 434 MHz transmission. Data discarded.");
                                MIWI0434_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 434 MHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif

            #ifdef MIWI_0868_RI
                //Try to send a Miwi packet with the stored data in 868 MHz band
                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x02){
                            aux = SendMIWI(ISM_868, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 868 MHz transmission. Data discarded.");
                                MIWI0868_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 868 MHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif
            #ifdef MIWI_2400_RI
                //Try to send a MiWi packet with the stored data in 2,4 GHz band
                for (i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid && isSameAddress(Address, ConnectionTable[i].Address)){
                        if(ConnectionTable[i].MiWiInterfaces & 0x04){
                            aux = SendMIWI(ISM_2G4, isBroadcast, Address, isLongAddr);
                            if (aux == NO_ERROR){
                                //Printf("\r\nSuccessful 2.4 GHz transmission. Data discarded.");
                                MIWI2400_sentPckts++;   //Update statistics.
                                return DiscardTXData(ri);   //Discard successfully sent pckt
                            }
                            else{
                                //Printf("\r\nFailed 2.4 GHz transmission. Data not discarded.");
                                return aux;
                            }
                        }
                        else{
                            Printf("\r\nNo existe conexi�n establecida a trav�s de dicha interfaz");
                            return UNAVAILABLE_INTERFACE_ERROR;
                        }
                    }
                }
                Printf("\r\nNodo no conocido a mi alrededor");
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif

            #ifdef WIFI_2400_RI
                //SendWIFI();
                aux = DiscardTXData(WIFI_2400);   //Discard sent packet
                if(aux != NO_ERROR){
                    return aux;
                }
            #endif
            return NO_ERROR;

        case NONE:
            //NOP
            Printf("\r\nError: No packets were sent. NONE of the Radio Interfaces"
                   " were selected");
            return INVALID_INTERFACE_ERROR;

        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
*/
//FOR MIWI_PROTOCOL WE KEEP IT JUST LIKE IT WAS BEFORE
// we try to send it and then in MiWi.c he will find the way to route it

//#else
    BOOL aux;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 434 MHz band
                aux = SendMIWI(ISM_434, isBroadcast, Address, isLongAddr);
                if (aux == NO_ERROR){
                    //Printf("\r\nSuccessful 434 MHz transmission. Data discarded.");
                    MIWI0434_sentPckts++;   //Update statistics.

                    return DiscardTXData(ri);   //Discard successfully sent pckt
                }
                else{
                    //Printf("\r\nFailed 434 MHz transmission. Data not discarded.");
                    return aux;
                }
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 868 MHz band
                aux = SendMIWI(ISM_868, isBroadcast, Address, isLongAddr);
                if(aux == NO_ERROR){
                    //Printf("\r\nSuccessful 868 MHz transmission. Data discarded.");
                    MIWI0868_sentPckts++;   //Update statistics.

                    return DiscardTXData(ri);   //Discard successfully sent pckt
                }
                else{
                    Printf("\r\nFailed 868 MHz transmission. Data not discarded.");
                    return aux;
                }
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xE0){                 //Wifi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Try to send a MiWi packet with the stored data in 2,4 GHz band
                aux = SendMIWI(ISM_2G4, isBroadcast, Address, isLongAddr);
                if(aux == NO_ERROR){
                    //Printf("\r\nSuccessful 2,4 GHz transmission. Data discarded.");
                    MIWI2400_sentPckts++;   //Update statistics.

                    return DiscardTXData(ri);   //Discard successfully sent pckt
                }
                else{
                    //Printf("\r\nFailed 2,4 GHz transmission. Data not discarded.");
                    return aux;
                }
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(AddrMode & 0xD0){                        //MiWi modes filter
                    return INVALID_ADDR_MODE_ERROR;
                }
                //Trigger the transmission!
                aux = SendWIFI();
                //Update statistics.

                return DiscardTXData(ri);   //Discard successfully sent pckt

                return aux;
            #endif
        case ALL_MIWI:
        case ALL:
            #ifdef MIWI_0434_RI
                //Try to send a MiWi packet with the stored data in 434 MHz band
                aux = SendMIWI(ISM_434, isBroadcast, Address, isLongAddr);
                if(aux == NO_ERROR){
                    MIWI0434_sentPckts++;   //Update statistics.

                    aux = DiscardTXData(MIWI_0434);   //Discard sent packet
                    if(aux != NO_ERROR){
                        return aux;
                    }
                }
                else{
                    Printf("\r\nFailed 434 MHz transmission.");
                    return aux;
                }
            #endif
            #ifdef MIWI_0868_RI
                //Try to send a Miwi packet with the stored data in 868 MHz band
                aux = SendMIWI(ISM_868, isBroadcast, Address, isLongAddr);
                if(aux == NO_ERROR){
                    MIWI0868_sentPckts++;   //Update statistics.

                    aux = DiscardTXData(MIWI_0868);   //Discard sent packet
                    if(aux != NO_ERROR){
                        return aux;
                    }
                }
                else{
                    Printf("\r\nFailed 868 MHz transmission.");
                    return aux;
                }
            #endif
            #ifdef MIWI_2400_RI
                //Try to send a MiWi packet with the stored data in 2,4 GHz band
                aux = SendMIWI(ISM_2G4, isBroadcast, Address, isLongAddr);
                if (aux == NO_ERROR){
                    MIWI2400_sentPckts++;   //Update statistics.

                    aux = DiscardTXData(MIWI_2400);   //Discard sent packet
                    if(aux != NO_ERROR){
                        return aux;
                    }
                }
                else{
                    Printf("\r\nFailed 2,4 GHz transmission.");
                    return aux;
                }
            #endif
            #ifdef WIFI_2400_RI
                //SendWIFI();
                aux = DiscardTXData(WIFI_2400);   //Discard sent packet
                if(aux != NO_ERROR){
                    return aux;
                }
            #endif
            return NO_ERROR;

        case NONE:
            //NOP
            Printf("\r\nError: No packets were sent. NONE of the Radio Interfaces"
                   " were selected");
            return INVALID_INTERFACE_ERROR;

        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
//#endif
}

/*******************************************************************************
 * Function:    WhichRIHasData()
 * Input:       None.
 * Output:      A byte reporting which interfaces have data pending to be read
 *              in case of success. A HAL error code otherwise.
 *              Check the output using radio interfaces bit masks.
 * Overview:    The application may use this function to guess which interfaces
 *              have received data before invoking GetRXData function. Once the
 *              application notices there are pending bytes to be read, it is
 *              highly recommended to process the whole packet, as the node
 *              would be able to discard it then.
 ******************************************************************************/
BYTE WhichRIHasData(){
    BYTE interfaces = 0x00;                     //NONE
    #if defined MIWI_0434_RI
        if (GetPayloadToRead(MIWI_0434) > 0)
            interfaces |= MIWI_0434_RI_MASK;    //MiWi at 434 MHz bit is set.
    #endif
    #if defined MIWI_0868_RI
        if (GetPayloadToRead(MIWI_0868) > 0)
            interfaces |= MIWI_0868_RI_MASK;    //MiWi at 868 MHz bit is set.
    #endif
    #if defined MIWI_2400_RI
        if (GetPayloadToRead(MIWI_2400) > 0)
            interfaces |= MIWI_2400_RI_MASK;    //MiWi at 2,4 GHz bit is set.
    #endif
    #if defined WIFI_2400_RI
        if (GetPayloadToRead(WIFI_2400) > 0)
            interfaces |= WIFI_2400_RI_MASK;    //WiFi at 2,4 GHz bit is set.
    #endif
    if(interfaces > 15)     //Due to masks definition, this should not happen
        return HAL_INTERNAL_ERROR;
    else
        return interfaces;
}

/*******************************************************************************
 * Function:    GetRXData(radioInterface ri, BYTE *storeItHere)
 * Input:       radioInterface ri - Radio Interface chosen.
 *                                  NONE, ALL_MIWI and ALL are invalid options.
 *              BYTE *storeItHere - The byte address where data will be written.
 * Output:      NO_ERROR if success (valid data has been read); HAL error code
 *              if failure. Requested data will be stored in storeItHere byte.
 * Overview:    Get one "NEW" payload byte of the packet received from the radio
 *              interface selected. The data is stored in the given address and
 *              once it is read, it cannot be read again. If the last pending
 *              byte is read, the packet is discarded for receiving new packets.
 ******************************************************************************/
BYTE GetRXData(radioInterface ri, BYTE *storeItHere){
    BYTE index;
    switch (ri){
        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected");
            return INVALID_INTERFACE_ERROR;
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.MIWI0434_RXbuf_isEmpty){
                    Printf("\r\nError: No MiWi at 434 MHz data available!");
                    return RX_BUFFER_EMPTY_ERROR;
                }
                else{   //Packet received...
                    if (MIWI0434_payloadToRead > 0){  //...and available data.
                        index = MIWI0434_rxMessage.PayloadSize - MIWI0434_payloadToRead;
                        *storeItHere = MIWI0434_rxMessage.Payload[index];    //Write data

                        if(--MIWI0434_payloadToRead == 0){
                            //Decrease counter and evaluate if it was the last
                            //byte to be read. If so, discard the packet (free
                            //the stack buffer for receiving new packets) and
                            //toggle the RX flag.

                            MIWI0434_procPckts++;           //Node statistics.
                            MiApp_DiscardMessage(ISM_434); //Discard MIWI packet
                            NodeStatus.MIWI0434_RXbuf_isEmpty = TRUE;

                        }
                        //If there are pending bytes, packet can't be discarded.
                        return NO_ERROR;
                    }
                    else{   //...and no data.
                        //This should not happen. If all data have been read the
                        //packet should have been discarded and the RX buffer
                        //empty flag should be TRUE.
                        return HAL_INTERNAL_ERROR;
                    }
                }
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.MIWI0868_RXbuf_isEmpty){
                    Printf("\r\nError: No MiWi at 868 MHz data available!");
                    return RX_BUFFER_EMPTY_ERROR;
                }
                else{   //Packet received...
                    if (MIWI0868_payloadToRead > 0){  //...and available data.
                        index = MIWI0868_rxMessage.PayloadSize - MIWI0868_payloadToRead;
                        *storeItHere = MIWI0868_rxMessage.Payload[index];    //Write data
                        //Printf("\r\nMiWi at 868 MHz: byte read");
                        if(--MIWI0868_payloadToRead == 0){
                            //Decrease counter and evaluate if it was the last
                            //byte to be read. If so, discard the packet (free
                            //the stack buffer for receiving new packets) and
                            //toggle the RX flag.

                            MIWI0868_procPckts++;           //Node statistics.
                            MiApp_DiscardMessage(ISM_868); //Discard MIWI packet
                            NodeStatus.MIWI0868_RXbuf_isEmpty = TRUE;

                        }
                        //If there are pending bytes, packet can't be discarded.
                        return NO_ERROR;
                    }
                    else{   //...and no data.
                        //This should not happen. If all data have been read the
                        //packet should have been discarded and the RX buffer
                        //empty flag should be TRUE.
                        return HAL_INTERNAL_ERROR;
                    }
                }
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.MIWI2400_RXbuf_isEmpty){
                    Printf("\r\nError: No MiWi at 2,4 GHz data available!");
                    return RX_BUFFER_EMPTY_ERROR;
                }
                else{   //Packet received...
                    if (MIWI2400_payloadToRead > 0){  //...and available data.
                        index = MIWI2400_rxMessage.PayloadSize - MIWI2400_payloadToRead;
                        *storeItHere = MIWI2400_rxMessage.Payload[index];    //Write data
                        //Printf("\r\nMiWi at 2,4 GHz: byte read");
                        if(--MIWI2400_payloadToRead == 0){
                            //Decrease counter and evaluate if it was the last
                            //byte to be read. If so, discard the packet (free
                            //the stack buffer for receiving new packets) and
                            //toggle the RX flag.

                            MIWI2400_procPckts++;           //Node statistics.
                            MiApp_DiscardMessage(ISM_2G4); //Discard MIWI packet
                            NodeStatus.MIWI2400_RXbuf_isEmpty = TRUE;

                        }
                        //If there are pending bytes, packet can't be discarded.
                        return NO_ERROR;
                    }
                    else{   //...and no data.
                        //This should not happen. If all data have been read the
                        //packet should have been discarded and the RX buffer
                        //empty flag should be TRUE.
                        return HAL_INTERNAL_ERROR;
                    }
                }
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if (NodeStatus.WIFI2400_RXbuf_isEmpty){
                    Printf("\r\nError: No WiFi data available!");
                    return RX_BUFFER_EMPTY_ERROR;
                }
                else{   //Packet received...
                    if (WIFI2400_payloadToRead > 0){  //...and available data.
                        if(UDPIsGetReady(skt) >0){
                            UDPGet(storeItHere);    //Write data
                            Printf("\r\nWiFi byte read");
                            if(WIFI2400_payloadToRead-- == 0){
                                //Decrease counter and evaluate if it was the last
                                //byte to be read. If so, discard the packet (free
                                //the stack for receiving new packets) and toggle
                                //the RX flag.
                                WIFI2400_procPckts++;  //Node statistics.
                                UDPDiscard();           //Discard WiFi packet
                                NodeStatus.WIFI2400_RXbuf_isEmpty = TRUE;

                            }
                            //If there are pending bytes, packet can't be discarded.
                            return NO_ERROR;
                        }
                        else{
                            //This should not happen. UDP socket has no data but
                            //HAL is unaware of this situation. Possible causes
                            //are a bad payloadToRead initialization or socket
                            //premature (?) closing.
                            return HAL_INTERNAL_ERROR;
                        }
                    }
                    else{   //...and no data.
                        //This should not happen. If all data have been read the
                        //packet should have been discarded and the RX buffer
                        //empty flag should be TRUE.
                        return HAL_INTERNAL_ERROR;
                    }
                }
            #endif

        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Get received data from a single interface.");
            return INVALID_INTERFACE_ERROR;
        default:
            //NOP
            Printf("\r\nError: Unknown radio interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    GetRXSourceAddr(radioInterface ri, BYTE *storeItFromHere)
 * Input:       radioInterface ri - Radio Interface chosen.
 *                                  NONE, ALL_MIWI and ALL are invalid options.
 *              BYTE *storeItHere -
 * Output:
 * Overview:
 ******************************************************************************/
BYTE GetRXSourceAddr(radioInterface ri, BYTE *storeItFromHere){
    BYTE i;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(MIWI0434_payloadToRead > 0){
                    if(MIWI0434_rxMessage.flags.bits.srcPrsnt){
                        if(MIWI0434_rxMessage.flags.bits.altSrcAddr){
                            memcpy(storeItFromHere, MIWI0434_rxMessage.SourceAddress, 2);
                            return SHORT_MIWI_ADDRMODE;
                        }else{
                            memcpy(storeItFromHere, MIWI0434_rxMessage.SourceAddress, MY_ADDRESS_LENGTH);
                            return LONG_MIWI_ADDRMODE;
                        }
                    }else{
                        return UNAVAILABLE_RX_ADDR_ERROR;
                    }
                }
                else{
                    return UNAVAILABLE_RX_PCKT_ERROR;
                }
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(MIWI0868_payloadToRead > 0){
                    if(MIWI0868_rxMessage.flags.bits.srcPrsnt){
                        if(MIWI0868_rxMessage.flags.bits.altSrcAddr){
                            memcpy(storeItFromHere, MIWI0868_rxMessage.SourceAddress, 2);
                            return SHORT_MIWI_ADDRMODE;
                        }else{
                            memcpy(storeItFromHere, MIWI0868_rxMessage.SourceAddress, MY_ADDRESS_LENGTH);
                            return LONG_MIWI_ADDRMODE;
                        }
                    }else{
                        return UNAVAILABLE_RX_ADDR_ERROR;
                    }
                }
                else{
                    return UNAVAILABLE_RX_PCKT_ERROR;
                }
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(MIWI2400_payloadToRead > 0){
                    if(MIWI2400_rxMessage.flags.bits.srcPrsnt){
                        if(MIWI2400_rxMessage.flags.bits.altSrcAddr){
                            memcpy(storeItFromHere, MIWI2400_rxMessage.SourceAddress, 2);
                            return SHORT_MIWI_ADDRMODE;
                        }else{
                            memcpy(storeItFromHere, MIWI2400_rxMessage.SourceAddress, MY_ADDRESS_LENGTH);
                            return LONG_MIWI_ADDRMODE;
                        }
                    }else{
                        return UNAVAILABLE_RX_ADDR_ERROR;
                    }
                }
                else{
                    return UNAVAILABLE_RX_PCKT_ERROR;
                }
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //Faltan cosas!
                return UNAVAILABLE_RX_ADDR_ERROR;
            #endif
        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected.");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: RX Source Address must be read for a single radio "
                   "interface.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    SetSecurityLevel(BYTE SecLevel)
 * Input:       BYTE SecLevel - The desired security level. Options are:
 *                  * 0x00 - Security features are disabled
 *                  * 0x01 - Packets will be ciphered in the MAC-PHY layer.
 * Output:      NO_ERROR if success. HAL error code otherwise.
 * Overview:    Use this function to enable or disable the security features in
 *              the node.
 ******************************************************************************/
BYTE SetSecurityLevel(BYTE SecLevel){
    switch(SecLevel){
        case 0x00:
            //Level 0. No security - Security flag disabled.
            NodeStatus.flags.bits.SecurityEN = 0;
            return NO_ERROR;
        case 0x01:
            //Level 1. Packets will be ciphered - Security flag enabled.
            NodeStatus.flags.bits.SecurityEN = 1;
            return NO_ERROR;
        //case...  FUTURE IMPLEMENTATION OF NEW SECURITY LEVELS
        default:
            Printf("\r\nInvalid security level.");
            return INVALID_SECLEVEL_ERROR;
    }
}

/*******************************************************************************
 * Function:    SetChannel(radioInterface ri, BYTE channel)
 * Input:       radioInterface ri - Selects a single radio interface available.
 *                                  NONE, ALL_MIWI and ALL options are disabled.
 *              BYTE channel      - The operating channel desired. It has to be
 *                                  within the valid range.
 * Output:      NO_ERROR code if success. Otherwise, a HAL error code.
 * Overview:    This function is used to change the operating channel of a radio
 *              interface available.
 ******************************************************************************/
BYTE SetChannel(radioInterface ri, BYTE channel){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    BOOL ok;
    UINT8 spi_prev_ec;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI0434NumChannels){
                    spi_prev_ec = GetSPIErrorCounter();
                    ok = MiApp_SetChannel(channel, ISM_434);  //Set MIWI channel
                    if(GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    if (ok){         //Success
                        NodeStatus.MIWI0434_OpChannel= channel;
                        return NO_ERROR;
                    }
                    return MIWI0434_STACK_ERROR;
                }
                Printf("\r\nError: Invalid channel for MiWi at 434 MHz.");
                return INVALID_CHANNEL_ERROR;
            #endif

        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI0868NumChannels){
                    spi_prev_ec = GetSPIErrorCounter();
                    ok = MiApp_SetChannel(channel, ISM_868);  //Set MIWI channel
                    if(GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    if (ok){            //Success
                        NodeStatus.MIWI0868_OpChannel= channel;
                        return NO_ERROR;
                    }
                    return MIWI0868_STACK_ERROR;
                }
                Printf("\r\nError: Invalid channel for MiWi at 868 MHz.");
                return INVALID_CHANNEL_ERROR;
            #endif

        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < MIWI2400NumChannels){
                    spi_prev_ec = GetSPIErrorCounter();
                    ok = MiApp_SetChannel(channel+MIWI2400ConfChannelOffset, ISM_2G4);
                    if(GetSPIErrorCounter() != spi_prev_ec){
                        ResetSPIErrorCounter();
                        return SPI_ERROR;
                    }
                    if (ok){            //Success
                        NodeStatus.MIWI2400_OpChannel= channel;
                        return NO_ERROR;
                    }
                    return MIWI2400_STACK_ERROR;
                }
                Printf("\r\nError: Invalid channel for MiWi at 2,4 GHz.");
                return INVALID_CHANNEL_ERROR;
            #endif

        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(channel < WIFI2400NumChannels){
                    //AQU� FALTAN COSAS //Set WIFI channel
                    NodeStatus.WIFI2400_OpChannel= channel;
                    return NO_ERROR;
                }
                else{
                    Printf("\r\nError: Invalid WiFi channel.");
                    return INVALID_CHANNEL_ERROR;
                }
            #endif

        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected to set "
                   "its channel");
            return INVALID_INTERFACE_ERROR;
        case ALL_MIWI:
        case ALL:
            Printf("\r\nError: Operating Channels of Radio Interfaces must be set"
                   " one by one.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    DoChannelScanning(radioInterface ri, BYTE *storeInfoHere)
 * Input:       radioInterface ri   - The selected interface to do scanning.
 *                                    NONE is not a valid option.
 *              BYTE *storeInfoHere - A byte where the RSSI value or the optimal
 *                                    radio interface will be stored.
 * Output:      The optimal (i.e. the least noisy) channel of the selected radio
 *              interface. In addition, the RSSI value will be stored in the
 *              storeInfoHere byte.
 *              IMPORTANT! If "ALL" were selected, the RSSI value of the optimal
 *              channel will not be written. Instead, the node will report which
 *              interface has the optimal channel using the bit masks available.
 *              Application should read the StoreInfoHere byte for identifying
 *              the channel among ALL the channels available in ALL interfaces.
 *              If the RSSI value is desired, the application may invoke the
 *              GetScanResults function.
 * Overview:    Invoke this function for doing channel scanning in the selected
 *              radio interface. Carrier Sense scan mode is not supported by
 *              MiWi stack so Energy Detection scan mode is used. In case of
 *              success (output is not an error code)this function implies:
 *              - Obtaining the optimal channel of a single radio interface as a
 *                returned value.
 *              - Storing the RSSI noise value or the radio interface bit mask
 *                in the given address - storeInfoHere byte.
 *              - Refreshing the RSSI values stored in the node status struct.
 ******************************************************************************/
BYTE DoChannelScanning(radioInterface ri, BYTE *storeInfoHere){
    if (NodeStatus.flags.bits.NodeAsleep){
        Printf("\r\nError: Trying to write a data byte but node is asleep.");
        return ASLEEP_NODE_ERROR;
    }
    BYTE opCh, tempCh, tempNoise, best_ri;
    UINT8 spi_prev_ec;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                spi_prev_ec = GetSPIErrorCounter();
                opCh = MiApp_NoiseDetection(0xFFFFFFFF, 5, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_434);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(opCh == 0xFF){
                    return MIWI0434_STACK_ERROR;
                }
                return opCh - MIWI0434ConfChannelOffset;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                spi_prev_ec = GetSPIErrorCounter();
                opCh = MiApp_NoiseDetection(0xFFFFFFFF, 5, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_868);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(opCh == 0xFF){
                    return MIWI0868_STACK_ERROR;
                }
                return opCh - MIWI0868ConfChannelOffset;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                spi_prev_ec = GetSPIErrorCounter();
                opCh = MiApp_NoiseDetection(0xFFFFFFFF, 5, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_2G4);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(opCh == 0xFF){
                    return MIWI2400_STACK_ERROR;
                }
                return opCh - MIWI2400ConfChannelOffset;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //faltan cosas!
                return UNKNOWN_ERROR;
            #endif
        case ALL_MIWI:
        case ALL:
            tempNoise = 0xFF;
            #ifdef MIWI_0434_RI
                spi_prev_ec = GetSPIErrorCounter();
                tempCh = MiApp_NoiseDetection(0xFFFFFFFF, 10, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_434);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(tempCh == 0xFF){
                    return MIWI0434_STACK_ERROR;
                }
                if(*storeInfoHere < tempNoise){
                    tempNoise = *storeInfoHere;
                    opCh = tempCh - MIWI0434ConfChannelOffset;
                    best_ri = MIWI_0434_RI_MASK;
                }
            #endif
            #ifdef MIWI_0868_RI
                spi_prev_ec = GetSPIErrorCounter();
                tempCh = MiApp_NoiseDetection(0xFFFFFFFF, 10, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_868);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(tempCh == 0xFF){
                    return MIWI0868_STACK_ERROR;
                }
                if(*storeInfoHere < tempNoise){
                    tempNoise = *storeInfoHere;
                    opCh = tempCh - MIWI0868ConfChannelOffset;
                    best_ri = MIWI_0868_RI_MASK;
                }
            #endif
            #ifdef MIWI_2400_RI
                spi_prev_ec = GetSPIErrorCounter();
                tempCh = MiApp_NoiseDetection(0xFFFFFFFF, 10, NOISE_DETECT_ENERGY,\
                                            storeInfoHere, ISM_2G4);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(tempCh == 0xFF){
                    return MIWI2400_STACK_ERROR;
                }
                if(*storeInfoHere < tempNoise){
                    tempNoise = *storeInfoHere;
                    opCh = tempCh - MIWI2400ConfChannelOffset;
                    best_ri = MIWI_2400_RI_MASK;
                }
            #endif
            if(ri == ALL){
                #ifdef WIFI_2400_RI
                    //faltan cosas!
                #endif
            }
            *storeInfoHere = best_ri;
            return opCh;
        case NONE:
            //NOP
            Printf("\r\nError: NONE of Radio Interfaces were selected to do the "
                   "channel scanning.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
}

/*******************************************************************************
 * Function:    SaveConnTable(BYTE *storeItFromHere)
 * Input:       BYTE *storeItFromHere - A pointer to the base memory position
 *                                      where the first data byte will be stored
 *                                      (the following data will be saved in the
 *                                      following positions).
 * Output:      Returns an non-negative integer which indicates how many valid
 *              connections have been saved, unless an error occurs. As usually,
 *              error codes are used to report the cause.
 * Overview:    This function saves the valid connections stored in the MiWi
 *              Connection Table from the memory position given as a parameter
 *              on. Connection Table is shared for all MiWi interfaces available
 *              in the node and contains information about peers such as address
 *              or connectivity.
 * Remarks:     This function disables interruptions for a while in order to
 *              access to frequently used stacks' data in a safe way.
 *              This function has special memory requirements. The maximum bytes
 *              that will be stored depends on the protocol and the number of
 *              valid connections. Memory needed for saving the table is then up
 *              to (CONNECTION_SIZE * MIWI_CONN_ENTRY_SIZE) bytes.
 ******************************************************************************/
BYTE SaveConnTable(BYTE *storeItFromHere){
    BYTE i;
    BYTE connStored = 0;        //Connections saved
    unsigned int status;
    status = INTDisableInterrupts();    //Disable Interrupts.
    //Data not modified by ISRs or stacks maintenance or transceivers.

    for (i=0; i<CONNECTION_SIZE; i++){
        //Check all connection entries.
        if(ConnectionTable[i].status.bits.isValid){
            //Copy only the valid connection entries.
            memcpy(storeItFromHere, &ConnectionTable, MIWI_CONN_ENTRY_SIZE);
            //Increment pointer for avoid overlapping.
            storeItFromHere += MIWI_CONN_ENTRY_SIZE;
            connStored++;
        }
    }
    status = INTEnableInterrupts();     //Enable interrupts again.
    Printf("\r\nValid connections stored: ");
    PrintDec(connStored);
    Printf("    Total bytes: ");
    PrintDec((connStored*MIWI_CONN_ENTRY_SIZE)/100);    //Thousands & Hundreds
    PrintDec((connStored*MIWI_CONN_ENTRY_SIZE)%100);    //Tens & Units
    return connStored;
}

/*******************************************************************************
 * Function:    RestoreConnTable(BYTE *takeItFromHere, BYTE numConn)
 * Input:       BYTE *takeItFromHere - A pointer to the base memory position
 *                                     where the valid connections were stored.
 *              BYTE numConn         - Number of connections to be restored.
 * Output:      Returns NO_ERROR or HAL error code.
 * Overview:    This function is complementary to SaveConnTable and may be used
 *              to restore the valid connections in the connection table.
 *              Connection Table is shared for all MiWi interfaces available
 *              in the node and contains information about peers such as address
 *              or connectivity.
 * Remarks:     This function disables interruptions for a while in order to
 *              access to frequently used stacks' data in a safe way.
 *              Once the connections are restored, the remaining connection
 *              slots are set to invalid.
 *              BE VERY AWARE OF restoring invalid, modified or conflicting
 *              connections with this node's variables or with other peers'
 *              variables within the network. This may cause the network's
 *              features malfunctioning.
 ******************************************************************************/
BYTE RestoreConnTable(BYTE *takeItFromHere, BYTE numConn){
    BYTE i;
    unsigned int status;
    status = INTDisableInterrupts();
    //Data not modified by ISRs of stacks maintenance or transceivers.
    if(numConn > CONNECTION_SIZE){
        return CONNTABLE_EXCEEDED_ERROR;
    }

    memcpy(&ConnectionTable, takeItFromHere, numConn*MIWI_CONN_ENTRY_SIZE);

    //IMPORTANT: "Erases" the rest of the connection table.
    for(i=numConn; i<CONNECTION_SIZE; i++){
        ConnectionTable[i].status.bits.isValid = 0;
    }
    status = INTEnableInterrupts(); //Enable interrupts again.
    Printf("\r\nValid connections restored: ");
    PrintDec(numConn);
    return NO_ERROR;
}

#if (defined PROTOCOL_MIWI) && (defined NWK_ROLE_COORDINATOR)
    #ifdef MIWI_0434_RI
        extern Routing0434Table;
        extern Router0434Failures;
        extern known0434Coordinators;
    #endif
    #ifdef MIWI_0868_RI
        extern Routing0868Table;
        extern Router0868Failures;
        extern known0868Coordinators;
    #endif
    #ifdef MIWI_2400_RI
        extern Routing2400Table;
        extern Router2400Failures;
        extern known2400Coordinators;
    #endif

    /***************************************************************************
    * Function: SaveRoutingTable(radioInterface ri, BYTE *storeItFromHere)
    * Input:    radioInterface ri     - The MiWi interface whose routing table
    *                                   will be saved.
    *           BYTE *storeItFromHere - A pointer to the base memory position
    *                                   where the data will be stored.
    * Output:   Returns NO_ERROR or HAL error code.
    * Overview: This function saves the routing table of a MiWi interface from
    *           the position given as a parameter on. Routing Table contains
    *           information about network's topology and possible hops to reach
    *           a node within the network.
    * Remarks:  This function disables interruptions for a while in order to
    *           access to frequently used stacks' data in a safe way.
    *           This function has special memory requirements. The number of
    *           bytes that will be stored per MiWi interface are 17 (8 + 8 + 1),
    *           as there are up to eight coordinators in the MiWi Protocol. The
    *           info stored is:
    *           - next hop to reach every coordinator
    *           - routing failures through every coordinator
    *           - a byte regarding to the known coordinators (bit-wise).
    *           Thus, 17 x 3 = 51 bytes max.
    ***************************************************************************/
    BYTE SaveRoutingTable(radioInterface ri, BYTE *storeItFromHere){
        unsigned int status;
        switch (ri){
            case MIWI_0434:
                #ifndef MIWI_0434_RI
                    Printf("\r\nError: MiWi at 434 MHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    status = INTDisableInterrupts();
                    memcpy(storeItFromHere, &Routing0434Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router0434Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known0434Coordinators, 1);
                    break;
                #endif
            case MIWI_0868:
                #ifndef MIWI_0868_RI
                    Printf("\r\nError: MiWi at 868 MHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    status = INTDisableInterrupts();
                    memcpy(storeItFromHere, &Routing0868Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router0868Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known0868Coordinators, 1);
                    break;
                #endif
            case MIWI_2400:
                #ifndef MIWI_2400_RI
                    Printf("\r\nError: MiWi at 2,4 GHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    status = INTDisableInterrupts();
                    memcpy(storeItFromHere, &Routing2400Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router2400Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known2400Coordinators, 1);
                    break;
                #endif

            case ALL_MIWI:
                status = INTDisableInterrupts();
                #ifdef MIWI_0434_RI
                    memcpy(storeItFromHere, &Routing0434Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router0434Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known0434Coordinators, 1);
                    storeItFromHere++;
                #endif
                #ifdef MIWI_0868_RI
                    memcpy(storeItFromHere, &Routing0868Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router0868Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known0868Coordinators, 1);
                    storeItFromHere++;
                #endif
                #ifdef MIWI_2400_RI
                    memcpy(storeItFromHere, &Routing2400Table, 8);
                    storeItFromHere += 8;
                    memcpy(storeItFromHere, &Router2400Failures, 8);
                    storeItFromHere +=8;
                    memcpy(storeItFromHere, &known2400Coordinators, 1);
                    storeItFromHere++;
                #endif
                break;
            case WIFI_2400:
            case ALL:
            case NONE:
                Printf("\r\nError: You must select a single MiWi radio interface.");
                return INVALID_INTERFACE_ERROR;
            default:
                Printf("\r\nError: Unknown Radio Interface.");
                return UNKNOWN_INTERFACE_ERROR;
        }
        INTEnableInterrupts();
        return NO_ERROR;
    }

     /**************************************************************************
     * Function:    RestoreRoutingTable(radioInterface ri, BYTE *takeItFromHere)
     * Input:       radioInterface ri    - The MiWi interface whose routing
     *                                     table will be restored.
     *              BYTE *takeItFromHere - A pointer to the base memory position
     *                                     where the valid data were stored.
     * Output:      Returns NO_ERROR or HAL error code.
     * Overview:    This function is complementary to SaveRoutingTable and may
     *              be used to restore the routing information of a coordinator.
     *              Routing Table contains information about network's topology
     *              and possible hops to reach a node within the network.
     * Remarks:     This function disables interruptions for a while in order to
     *              access to frequently used stacks' data in a safe way.
     *              BE VERY AWARE OF restoring invalid, modified or conflicting
     *              information with this node's variables or with other peers'
     *              variables within the network. This may cause the network's
     *              features malfunctioning.
     **************************************************************************/
    BYTE RestoreRoutingTable(radioInterface ri, BYTE *takeItFromHere){
        switch (ri){
            case MIWI_0434:
                #ifndef MIWI_0434_RI
                    Printf("\r\nError: MiWi at 434 MHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    memcpy(&Routing0434Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router0434Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known0434Coordinators, takeItFromHere, 1);
                    return NO_ERROR;
                #endif
            case MIWI_0868:
                #ifndef MIWI_0868_RI
                    Printf("\r\nError: MiWi at 868 MHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    memcpy(&Routing0868Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router0868Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known0868Coordinators, takeItFromHere, 1);
                    return NO_ERROR;
                #endif
            case MIWI_2400:
                #ifndef MIWI_2400_RI
                    Printf("\r\nError: MiWi at 2,4 GHz is not available");
                    return UNAVAILABLE_INTERFACE_ERROR;
                #else
                    memcpy(&Routing2400Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router2400Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known2400Coordinators, takeItFromHere, 1);
                    return NO_ERROR;
                #endif
            case ALL_MIWI:
                #ifdef MIWI_0434_RI
                    memcpy(&Routing0434Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router0434Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known0434Coordinators, takeItFromHere, 1);
                    takeItFromHere++;
                #endif
                #ifdef MIWI_0868_RI
                    memcpy(&Routing0868Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router0868Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known0868Coordinators, takeItFromHere, 1);
                    takeItFromHere++;
                #endif
                #ifdef MIWI_2400_RI
                    memcpy(&Routing2400Table, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&Router2400Failures, takeItFromHere, 8);
                    takeItFromHere +=8;
                    memcpy(&known2400Coordinators, takeItFromHere, 1);
                #endif
                break;
            case ALL:
            case WIFI_2400:
            case NONE:
                Printf("\r\nError: You must select a MiWi radio interface.");
                return INVALID_INTERFACE_ERROR;
            default:
                Printf("\r\nError: Unknown Radio Interface.");
                return UNKNOWN_INTERFACE_ERROR;
        }
    }
#endif

////////////////////////////////////////////////////////////////////////////////
/************ DEBUG FUNCTIONS - AVAILABLE FOR APPLICATION CODE TOO ************/
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
 * Function:    void Print32Dec(unsigned int toPrint)
 * Input:       toPrint - a 32 bits unsigned int to be printed in decimal.
 * Output:      None
 * Overview:    LSI-CWSN Stack - Printing 32 bits uints for debugging.
 ******************************************************************************/
void Print32Dec(unsigned int toPrint){
    unsigned int temp1, temp2;
    temp2 = (toPrint / 100000000);
    temp1 = temp2 * 100000000;
    PrintDec((BYTE)temp2);
    temp2 = (toPrint - temp1) / 1000000;
    temp1 += temp2 * 1000000;
    PrintDec((BYTE)temp2);
    temp2 = (toPrint - temp1) / 10000;
    temp1 += temp2*10000;
    PrintDec((BYTE)temp2);
    temp2 = (toPrint - temp1) / 100;
    temp1 += temp2 * 100;
    PrintDec((BYTE) temp2);
    temp2 = (toPrint - temp1);
    PrintDec((BYTE) temp2);
}

/*******************************************************************************
 * Function:    DumpRXPckt(radioInterface ri)
 * Input:       radioInterface ri - Selected radio interface. ALL_MIWI, ALL,
 *                                  and NONE are invalid options.
 * Output:      Returns NO_ERROR or HAL error code.
 * Overview:    Prints out the received packet from the selected radio interface.
 ******************************************************************************/
BYTE DumpRXPckt(radioInterface ri){
    BYTE i;
    RECEIVED_MESSAGE *rxPckt;
    switch (ri){
        case MIWI_0434:
            #ifndef MIWI_0434_RI
                Printf("\r\nError: MiWi at 434 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0434_RXbuf_isEmpty){
                    return RX_BUFFER_EMPTY_ERROR;
                }
                rxPckt = &MIWI0434_rxMessage;
                break;
            #endif
        case MIWI_0868:
            #ifndef MIWI_0868_RI
                Printf("\r\nError: MiWi at 868 MHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI0868_RXbuf_isEmpty){
                    return RX_BUFFER_EMPTY_ERROR;
                }
                rxPckt = &MIWI0868_rxMessage;
                break;
            #endif
        case MIWI_2400:
            #ifndef MIWI_2400_RI
                Printf("\r\nError: MiWi at 2,4 GHz is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                if(NodeStatus.MIWI2400_RXbuf_isEmpty){
                    return RX_BUFFER_EMPTY_ERROR;
                }
                rxPckt = &MIWI2400_rxMessage;
                break;
            #endif
        case WIFI_2400:
            #ifndef WIFI_2400_RI
                Printf("\r\nError: WiFi is not available");
                return UNAVAILABLE_INTERFACE_ERROR;
            #else
                //Faltan cosas!
                return NO_ERROR;
            #endif
        case ALL_MIWI:
        case ALL:
        case NONE:
            //NOP
            Printf("\r\nError: Invalid radio interface for DumpRXPckt function.");
            return INVALID_INTERFACE_ERROR;
        default:
            Printf("\r\nError: Unknown Radio Interface.");
            return UNKNOWN_INTERFACE_ERROR;
    }
    Printf("\r\n\n\n\rVolcado por pantalla del paquete radio recibido.\n\r");
    Printf("\r\nFlags: ");
    PrintChar(rxPckt->flags.Val);
    Printf("\r\nRSSI: ");
    PrintChar(rxPckt->PacketRSSI);
    Printf("\r\nLQI: ");
    PrintChar(rxPckt->PacketLQI);
    Printf("\r\nSourcePANID: ");
    PrintChar(rxPckt->SourcePANID.v[1]);
    PrintChar(rxPckt->SourcePANID.v[0]);
    if(rxPckt->flags.bits.srcPrsnt)
        Printf("\r\nSource Address IS present. Value: ");
    else
        Printf("\r\nSource Address IS NOT present. Value: ");
    if(rxPckt->flags.bits.altSrcAddr){
        PrintChar(rxPckt->SourceAddress[1]);
        PrintChar(rxPckt->SourceAddress[0]);
    }
    else{
        for (i=0; i<MY_ADDRESS_LENGTH; i++){
            PrintChar(rxPckt->SourceAddress[MY_ADDRESS_LENGTH-1-i]);
        }
    }
    Printf("\r\nPayload: ");
    PrintDec(rxPckt->PayloadSize);
    Printf(" bytes: ");
    for (i=0; i<rxPckt->PayloadSize; i++){
        ConsolePut(rxPckt->Payload[i]);
    }
}

//ADDITIONAL defined debug macros in NodeHAL.h

////////////////////////////////////////////////////////////////////////////////
/******* SPECIAL FUNCTIONS - OPTIONALLY AVAILABLE FOR APPLICATION CODE  *******/
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Function:    AllStacksTasks()
 * Input:       None
 * Output:      None
 * Overview:    This function must be called frequently to do maintenance tasks
 *              of the stacks available, either by the application code or by
 *              the node itself. If the node does the maintenance, a timer with
 *              a very low interruption priority is enabled for doing the tasks.
 *              If this option is disabled, the application is in charge of
 *              calling this function as frequently as possible to ensure the
 *              right behaviour of the available stacks.
 *              Second option is only recommended for those applications which
 *              demand doing their tasks without being interrupted by the
 *              maintenance tasks.
 ******************************************************************************/
#if defined NODE_DOES_MAINTENANCE_TASKS
    static void AllStacksTasks()
#elif defined APP_DOES_MANTEINANCE_TASKS
    void AllStacksTasks()
#endif
{
    BYTE MiWiWithUserData = 0;
    MiWiWithUserData = MiApp_MessageAvailable(ALL_ISM);
    //coreTMRvals[1] = ReadCoreTimer();
    #ifdef MIWI_0434_RI
        if (MiWiWithUserData & MIWI_0434_RI_MASK){
            //There is a packet...
            if (NodeStatus.MIWI0434_RXbuf_isEmpty){
                //If RXbuf empty flag = 1 means: NEW PACKET TO BE PROCESSED.
                MIWI0434_payloadToRead = MIWI0434_rxMessage.PayloadSize;

                //Printf("\r\nNew MiWi at 434 MHz packet ready to be processed.");
                NodeStatus.MIWI0434_RXbuf_isEmpty = FALSE;      //Activate Flag!

                #if defined PROTOCOL_MIWI && defined ENABLE_DUMMY_BYTE
                    // Get the dummy byte at the first payload position. This is
                    // a fix to MiWi Stack, as the packets starting with 0 are
                    // treated as stack packets instead of user data.
                    GetRXData(MIWI_0434, &dummy);
                    if(dummy != 0xFF){
                        Printf("\r\nDummy byte unexpected value: ");
                        PrintChar(dummy);
                    }
                #endif
            }
            //If not empty means: STILL PROCESSING THE AVAILABLE PACKET. NOP
        }
        //No packets from MIWI radio interface...
    #endif
    //coreTMRvals[2] = ReadCoreTimer();
    #ifdef MIWI_0868_RI
        if (MiWiWithUserData & MIWI_0868_RI_MASK){
            //There is a packet...
            if (NodeStatus.MIWI0868_RXbuf_isEmpty){
                //If RXbuf empty flag = 1 means: NEW PACKET TO BE PROCESSED.
                MIWI0868_payloadToRead = MIWI0868_rxMessage.PayloadSize;

                //Printf("\r\nNew MiWi at 868 MHz packet ready to be processed.");
                NodeStatus.MIWI0868_RXbuf_isEmpty = FALSE;      //Activate Flag!

                #if defined PROTOCOL_MIWI && defined ENABLE_DUMMY_BYTE
                    // Get the dummy byte at the first payload position. This is
                    // a fix to MiWi Stack, as the packets starting with 0 are
                    // treated as stack packets instead of user data.
                    GetRXData(MIWI_0868, &dummy);
                    if(dummy != 0xFF){
                        Printf("\r\nDummy byte unexpected value: ");
                        PrintChar(dummy);
                    }
                #endif
            }
            //If not empty means: STILL PROCESSING THE AVAILABLE PACKET. NOP
        }
        //No packets from MIWI radio interface...
    #endif
    //coreTMRvals[3] = ReadCoreTimer();
    #ifdef MIWI_2400_RI
        if (MiWiWithUserData & MIWI_2400_RI_MASK){
            //There is a packet...
            if (NodeStatus.MIWI2400_RXbuf_isEmpty){
                //If RXbuf empty flag = 1 means: NEW PACKET TO BE PROCESSED.
                MIWI2400_payloadToRead = MIWI2400_rxMessage.PayloadSize;

                //Printf("\r\nNew MiWi at 2.4 GHz packet ready to be processed.");
                NodeStatus.MIWI2400_RXbuf_isEmpty = FALSE;      //Activate Flag!

                #if defined PROTOCOL_MIWI && defined ENABLE_DUMMY_BYTE
                    // Get the dummy byte at the first payload position. This is
                    // a fix to MiWi Stack, as the packets starting with 0 are
                    // treated as stack packets instead of user data.
                    GetRXData(MIWI_2400, &dummy);
                    if(dummy != 0xFF){
                        Printf("\r\nDummy byte unexpected value: ");
                        PrintChar(dummy);
                    }
                #endif
            }
            //If not empty means: STILL PROCESSING THE AVAILABLE PACKET. NOP
        }
        //No packets from MIWI radio interface...
    #endif
    //coreTMRvals[4] = ReadCoreTimer();

    #ifdef WIFI_2400_RI
        if (WIFIPcktAvailable){
            //There is a packet...
            if (NodeStatus.WIFI2400_RXbuf_isEmpty){
                //If RXbuf empty flag = 1 means: NEW PACKET TO BE PROCESSED.
                //WIFI_payloadToRead = XXXXXX;     //Data available
                NodeStatus.WIFI2400_RXbuf_isEmpty = FALSE;      //Activate Flag!
                Printf("\r\nNew WiFi packet ready to be processed.");
            }
            else{
                //If not empty means: STILL PROCESSING THE AVAILABLE PACKET. NOP
            }
        }
        //No packets from WIFI radio interface...
    #endif
}

////////////////////////////////////////////////////////////////////////////////
/********** INTERNAL FUNCTIONS - NOT AVAILABLE FOR APPLICATION CODE! **********/
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Function: InitVariables()
 * Input:    None
 * Output:   None
 * Overview: Node variables initialization function: status, node variables...
 ******************************************************************************/
static void InitVariables(){
    BYTE i;

    NodeStatus.flags.statusFlags = 0x00;
    #if defined MIWI_0434_RI
        NodeStatus.flags.bits.MIWI0434isON = 1;
        NodeStatus.MIWI0434_OpChannel = MIWI0434DfltChannel;
        NodeStatus.MIWI0434_TXbuf_isFull = FALSE;
        NodeStatus.MIWI0434_RXbuf_isEmpty = TRUE;
        for (i=0; i<MIWI0434NumChannels; i++){
            NodeStatus.scanMIWI0434 [i]= 0x00;
        }
        MIWI0434_PowerOutput = 0;
        MIWI0434_payloadToRead = 0;
        MIWI0434_procPckts = 0;
        MIWI0434_datacount = 0;

    #endif
    #if defined MIWI_0868_RI
        NodeStatus.flags.bits.MIWI0868isON = 1;
        NodeStatus.MIWI0868_OpChannel = MIWI0868DfltChannel;
        NodeStatus.MIWI0868_TXbuf_isFull = FALSE;
        NodeStatus.MIWI0868_RXbuf_isEmpty = TRUE;
        for (i=0; i<MIWI0868NumChannels; i++){
            NodeStatus.scanMIWI0868 [i] = 0x00;
        }
        MIWI0868_PowerOutput = 0;
        MIWI0868_payloadToRead = 0;
        MIWI0868_procPckts = 0;
        MIWI0868_datacount = 0;

    #endif
    #if defined MIWI_2400_RI
        NodeStatus.flags.bits.MIWI2400isON = 1;
        NodeStatus.MIWI2400_OpChannel = MIWI2400DfltChannel;
        NodeStatus.MIWI2400_TXbuf_isFull = FALSE;
        NodeStatus.MIWI2400_RXbuf_isEmpty = TRUE;
        for (i=0; i<MIWI2400NumChannels; i++){
            NodeStatus.scanMIWI2400 [i] = 0x00;
        }
        MIWI2400_PowerOutput = 0;   //Dflt: max. See InitMRF24J40(), MRF24J40.c
        MIWI2400_payloadToRead = 0;
        MIWI2400_procPckts = 0;
        MIWI2400_datacount = 0;

    #endif
    #if defined WIFI_2400_RI
        NodeStatus.flags.bits.WIFI2400isON = 1;
        NodeStatus.WIFI2400_OpChannel = WIFI2400DfltChannel;
        NodeStatus.WIFI2400_TXbuf_isFull = FALSE;
        NodeStatus.WIFI2400_RXbuf_isEmpty = TRUE;
        for (i=0; i<WIFI2400NumChannels; i++){
            NodeStatus.scanWIFI2400 [i][j] = 0x00;
        }
        WIFI2400_datacount = 0;
        WIFI2400_payloadToRead = 0;
        WIFI2400_procPckts = 0;
    #endif

  //  for (i=0; i< 11; i++){
   //     coreTMRvals[i] = 0;
   // }
}

/*******************************************************************************
 * Function: HALIntervalSleep()
 * Input:    None
 * Output:   None
 * Overview: This function is called when the node is ready to sleep for a time.
 *           Depending on configuration options, either WDT module or TIMER1 is
 *           used to count events and wake up the node if none of the enabled
 *           interrupt sources have done it by the timeout is reached.
 *           Interruptions are disabled and re-enabled in SleepNode function.
 *           Switch off as many modules (not being used during sleep mode) as
 *           you can before entering sleep mode for reducing energy consumption
 *           as much as possible.
 ******************************************************************************/
static void TimedPICSleep(){
    UINT32 Last_SlpEvCnt;
    //PIC TO SLEEP: 1)Enable T1/WDT;  2)SLEEP;  3)WAKE SRC;  4)SLEEP AGAIN?
    while (SleepEventCounter > 0){
        Last_SlpEvCnt = SleepEventCounter;
        #if defined WAKE_FROM_SLEEP_SOSC_T1
            T1CONSET = 0x8000;      //Enable T1
        #elif defined WAKE_FROM_SLEEP_WDT
            WDTCONSET = 0x8000;     //Enable WDT.
        #endif
        SYSKEY = 0x0;               //Force OSCCON lock
        SYSKEY = 0xAA996655;        //Unlock it for writing, step 1
        SYSKEY = 0x556699AA;        //Unlock it for writing, step 2
        OSCCONSET = 0x0010;   //Next "wait" instruction will enter sleep mode.
        asm volatile("wait");   //Enter sleep mode!
        //EXECUTION AFTER SLEEP WILL RESUME HERE. (SOMETHING WAKES UP THE NODE)

        #if defined WAKE_FROM_SLEEP_SOSC_T1
            T1CONCLR = 0x8000;      //Disable T1
            if(Last_SlpEvCnt == SleepEventCounter){
                //Timer didn't wake up the node so it must have been another
                //enabled interruption source => Break.
                Printf("\r\nISR");
                break;
            }
            Printf("\r\nT1");
            //Else: The interruption was timer1 and SleepEventCounter has been
            //updated for checking if the node must keep on sleeping. If so,
            //timer will resume in the next iteration.
        #elif defined WAKE_FROM_SLEEP_WDT
            if((WDTCON & 0x8000) == 0){
                WDTCONCLR = 0x8000;
                //Disable WDT should be done by ISR but let's play it safe...
                Printf("\r\nISR");
                break;
            }
            else if(RCON & 0x0010){
                //WDT generated an NMI during sleep mode and woke up the node.
                WDTCONCLR = 0x8000;     //Disable WDT
                RCONCLR = 0x0010;       //Clear WDT Timeout reset event.
                Printf("\r\nWDT");
                SleepEventCounter--;   //Decrease count
                WDTCONSET = 0x0001;     //Reset WDT count
            }
        #endif
    }
    return;
}

//MIWI INTERNAL FUNCTIONS///////////////////////////////////////////////////////
#if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Function:    void InitMIWI()
 * Input:       None
 * Output:      None
 * Overview:    Initialize MIWI Stack
 ******************************************************************************/
static BYTE InitMIWI(){
    #if defined(PROTOCOL_P2P)
        Printf("\r\nStarting MiWi(TM) P2P LSI-CWSN Stack...");
    #endif
    #if defined(PROTOCOL_MIWI)
        Printf("\r\nStarting MiWi(TM) LSI-CWSN Stack ...");
    #endif
    //#if defined (PROTOCOL_MIWI_PRO)
        //Printf("\r\nStarting MiWiPro(TM) LSI-CWSN Stack ...");
        //Miwi pro stack has not been implemented in this node yet.
    //#endif

    BYTE i,j,k;  //Auxiliar variables
    BYTE myChannel, ChannelOffset;
    miwi_band mb, next_mb;
    BYTE Miwi_Conn_Index;
    mb = 0;

#if defined FORCE_CONN_WITH_1 || defined FORCE_CONN_WITH_2 || defined FORCE_CONN_WITH_3

    BYTE LongAddress[8];
    BYTE ShortAddress[2];
    LongAddress[0] = 0x00;
    LongAddress[1] = 0x11;
    LongAddress[2] = 0x22;
    LongAddress[3] = 0x33;
    LongAddress[4] = 0x44;
    LongAddress[5] = 0x55;
    LongAddress[6] = 0x66;
    ShortAddress[0] = 0x00;


#endif

#ifdef FORCE_CONN_WITH_1
    LongAddress[7] = 0x11;
    ShortAddress[1] = 0x00;
#elif defined  FORCE_CONN_WITH_2
    LongAddress[7] = 0x22;
    ShortAddress[1] = 0x01;
#elif defined  FORCE_CONN_WITH_3
    LongAddress[7] = 0x33;
    ShortAddress[1] = 0x02;
#endif



    BYTE auxiliar;

    UINT8 spi_prev_ec = GetSPIErrorCounter();
    /**************************************************************************/
    // Function MiApp_ProtocolInit intialize the protocol stack.
    // The return value is a boolean to indicate the status of the operation.
    // The only parameter indicates if Network Freezer should be invoked.
    //      When Network Freezer feature is invoked, all previous network
    //      configurations will be restored to the states before the reset or
    //      power cycle.
    /**************************************************************************/
    #if defined (NETWORK_FREEZER_ENABLE)
    {
        MiApp_ProtocolInit(TRUE);
        if (GetSPIErrorCounter() != spi_prev_ec){
            ResetSPIErrorCounter();
            Printf("\r\nSPI errors occured during MiWi initialization.");
            return SPI_ERROR;
        }
        Printf("\r\nNetwork Freezer enabled. No hand-shake process.\r\n");
        DumpConnection(0xFF);
    }
    #else
    {
        MiApp_ProtocolInit(FALSE);      // Init all MiWi interfaces.
        if (GetSPIErrorCounter() != spi_prev_ec){
            ResetSPIErrorCounter();
            Printf("\r\nSPI errors occured during protocol MiWi initialization.");
            return SPI_ERROR;
        }

    SINGLE_RI_TASK:

        switch(mb){
            case NO_ISM:
                #if defined MIWI_0434_RI
                    next_mb = ISM_434;
                    ChannelOffset = MIWI0434ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 434 MHz...\r");
                #elif defined MIWI_0868_RI
                    next_mb = ISM_868;
                    ChannelOffset = MIWI0868ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 868 MHz...\r");
                #elif defined MIWI_2400_RI
                    next_mb = ISM_2G4;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 2,4 GHz...\r");
                #else
                    #error "A Miwi Interface must be defined"
                #endif
                break;
            case ISM_434:
                #if defined MIWI_0868_RI
                    next_mb = ISM_868;
                    ChannelOffset = MIWI0868ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 868 MHz...\r");
                #elif defined MIWI_2400_RI
                    next_mb = ISM_2G4;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 2,4 GHz...\r");
                #else
                    next_mb = NO_ISM;
                #endif
                break;
            case ISM_868:
                #if defined MIWI_2400_RI
                    next_mb = ISM_2G4;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    Printf("\r\n\nInitialising MiWi at 2,4 GHz...\r");
                #else
                    next_mb = NO_ISM;
                #endif
                break;
            case ISM_2G4:
                next_mb = NO_ISM;
                break;
            default:
                Printf("\r\nMUY MAL ROLLO\r");
                return HAL_INTERNAL_ERROR;
        }
        if (next_mb == NO_ISM){
            goto INIT_DONE;
        }
        mb = next_mb;

        #ifdef ENABLE_ACTIVE_SCAN
            Printf("\r\nStarting Active Scan...");
            /**************************************************************/
            // Function MiApp_SearchConnection will return the number of
            // existing connections in all channels. It will help to decide
            // which channel to operate on and which connection to add.
            // The return value is the number of connections. The connection
            // data are stored in global variable ActiveScanResults. Maximum
            // active scan result is defined as ACTIVE_SCAN_RESULT_SIZE
            // The first parameter is the scan duration, which has the same
            // definition in Energy Scan.
            //      10 is roughly 1 second; 9 is 0,5 seconds; 11, 2 seconds.
            //      Maximum scan duration is 14, or roughly 16 seconds.
            // The second parameter is the channel map. Bit 0 of the double
            // word parameter represents channel 0.
            //      For the 2.4GHz frequency band, all possible channels are
            //      channel 11 to channel 26.
            // As the result, the bit map is 0x07FFF800. Stack will filter
            // out all invalid channels, so the application only needs to
            // pay attention to the channels that are not preferred.
            /**************************************************************/
            myChannel = 0xFF;
            spi_prev_ec = GetSPIErrorCounter();
            i = MiApp_SearchConnection(9, 0xFFFFFFFF, mb);
            if (GetSPIErrorCounter() != spi_prev_ec){
                ResetSPIErrorCounter();
                Printf("\r\nSPI errors occured during MiWi initialization.");
                return SPI_ERROR;
            }

            if (i > 0){ //Networks found
                // now print out the scan result.
                Printf("\r\nActive Scan Results: \r\n");
                for(j = 0; j < i; j++){
                    Printf("Channel: ");
                    PrintDec(ActiveScanResults[j].Channel - ChannelOffset);
                    Printf("   RSSI: ");
                    PrintChar(ActiveScanResults[j].RSSIValue);
                    Printf(" DIR: ");
                    for (k=0; k< MY_ADDRESS_LENGTH; k++){
                        PrintChar(ActiveScanResults[j].Address[k]);
                    }
                    Printf("   PANID: ");
                    PrintChar(ActiveScanResults[j].PANID.v[1]);
                    PrintChar(ActiveScanResults[j].PANID.v[0]);
                    Printf("\r\n");
                    myChannel = ActiveScanResults[j].Channel;
                }
            }
        #endif

        /**********************************************************************/
        // Function MiApp_ConnectionMode sets the connection mode for the
        // protocol stack. Possible connection modes are:
        //  - ENABLE_ALL_CONN       accept all connection request
        //  - ENABLE_PREV_CONN      accept only known device to connect
        //  - ENABL_ACTIVE_SCAN_RSP do not accept connection request, but allow
        //                          response to active scan
        //  - DISABLE_ALL_CONN      disable all connection request, including
        //                          active scan request
        /**********************************************************************/
        MiApp_ConnectionMode(ENABLE_ALL_CONN, mb);

        if (i > 0){

            /******************************************************************/
            // Function MiApp_EstablishConnection try to establish a new
            // connection with peer device.
            // The first parameter is the index to the active scan result, which
            //      is acquired by discovery process (active scan). If the value
            //      of the index is 0xFF, try to establish a connection with any
            //      peer.
            // The second parameter is the mode to establish connection, either
            //      direct or indirect. Direct mode means connection within the
            //      radio range; Indirect mode means connection may or may not
            //      in the radio range.
            /******************************************************************/
            spi_prev_ec = GetSPIErrorCounter();


            //******************************************************************/
            // CLARA:
            //Para facilitar las pruebas y el despliegue de la red, se ha
            //  forzado que los nodos se conecten al mismo padre en todas las
            //  interfaces. De esta forma en 434 se conectara al primer nodo que
            //  encuentre, mientras que en 868 escogera a ese mismo padre. De no
            //  tener conectada la interfaz de 434 buscara un nuevo disp al que
            //  asociarse.
            //
            //Para modificarlo simplemente modificar o quitar las comprobaciones
            //  de la tabla de conexiones.
            /******************************************************************/

        #if defined PROTOCOL_MIWI

                if (mb == ISM_434){
                    #if defined MIWI_0434_RI
                                Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                                if (GetSPIErrorCounter() != spi_prev_ec){
                                    ResetSPIErrorCounter();
                                    Printf("\r\nSPI errors occured during MiWi initialization.");
                                    return SPI_ERROR;
                                }
                                if (Miwi_Conn_Index == 0xFF){
                                    Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                }
                                //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }
                else if (mb == ISM_868){
                    #if defined MIWI_0868_RI
                    //try to find the father in the scanResults and join it

                    for (j=0; j< i ; j++){
                        for (k=0; k<CONNECTION_SIZE; k++){
                            if ((ConnectionTable[k].status.bits.isValid) && isSameAddress(ActiveScanResults[i].Address,ConnectionTable[k].Address) && (ConnectionTable[k].MiWiInterfaces & 0x01)){

                            Printf("\r\n NODE FOUND");
                            Miwi_Conn_Index = MiApp_EstablishConnection(j, CONN_MODE_DIRECT, mb);
                                if (GetSPIErrorCounter() != spi_prev_ec){
                                    ResetSPIErrorCounter();
                                    Printf("\r\nSPI errors occured during MiWi initialization.");
                                    return SPI_ERROR;
                                }
                                if (Miwi_Conn_Index == 0xFF){
                                    Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                    break;
                                }
                                else {
                                    //Printf("\r\n\n JOIN SUCCESS!! \n\n");
                                    break;
                                }
                            }
                        }
                    }
                    Printf("\r\n NEW CONNECTION");
                    //at this point he is not connected no any other node
                    Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            Printf("\r\nSPI errors occured during MiWi initialization.");
                            return SPI_ERROR;
                        }
                        if (Miwi_Conn_Index == 0xFF){
                            Printf("\r\n\nError: JOIN FAIL!! \n\n");
                        }
                        //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }
                else if (mb == ISM_2G4){
                    #if defined MIWI_2400_RI
                    //try to find the father in the scanResults and join it

                    for (j=0; j< i ; j++){
                        for (k=0; k<CONNECTION_SIZE; k++){
                            if ((ConnectionTable[k].status.bits.isValid)){
                                if ((ActiveScanResults[j].Address[0] == ConnectionTable[k].MIWI0434AltAddress.v[0]) && (ActiveScanResults[j].Address[1] == ConnectionTable[k].MIWI0434AltAddress.v[1])){
                                    if ((ConnectionTable[k].MiWiInterfaces & 0x01) || (ConnectionTable[k].MiWiInterfaces & 0x02)){

                                        Printf("\r\n NODE FOUND\n\n");
                                        Miwi_Conn_Index = MiApp_EstablishConnection(j, CONN_MODE_DIRECT, mb);
                                        if (GetSPIErrorCounter() != spi_prev_ec){
                                                ResetSPIErrorCounter();
                                                Printf("\r\nSPI errors occured during MiWi initialization.");
                                                return SPI_ERROR;
                                        }
                                        if (Miwi_Conn_Index == 0xFF){
                                                Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                                goto COMMON;
                                        }
                                        else {
                                                //Printf("\r\n\n JOIN SUCCESS!! \n\n");
                                                goto COMMON;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Printf("\r\n NEW CONNECTION\n\n");
                    //at this point he is not connected no any other node
                    Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            Printf("\r\nSPI errors occured during MiWi initialization.");
                            return SPI_ERROR;
                        }
                        if (Miwi_Conn_Index == 0xFF){
                            Printf("\r\n\nError: JOIN FAIL!! \n\n");
                        }
                        //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }

        #elif defined PROTOCOL_P2P && !defined ENABLE_SLEEP
                if (i > 0){ //Networks found
                    for (j=0; j<i; j++){
                        Miwi_Conn_Index = MiApp_EstablishConnection(j, CONN_MODE_DIRECT, mb);

                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            Printf("\r\nSPI errors occured during MiWi initialization.");
                            return SPI_ERROR;
                        }
                        if (Miwi_Conn_Index == 0xFF){
                            Printf("\r\n\nError: JOIN FAIL!! \n\n");
                        }
                        //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    }
                }
        #else //for a enable sleep and p2p
                if (mb == ISM_434){
                    #if defined MIWI_0434_RI

                                Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                                if (GetSPIErrorCounter() != spi_prev_ec){
                                    ResetSPIErrorCounter();
                                    Printf("\r\nSPI errors occured during MiWi initialization.");
                                    return SPI_ERROR;
                                }
                                if (Miwi_Conn_Index == 0xFF){
                                    Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                }
                                //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }
                else if (mb == ISM_868){
                    #if defined MIWI_0868_RI
                    //try to find the father in the scanResults and join it

                    for (j=0; j< i ; j++){
                        for (k=0; k<CONNECTION_SIZE; k++){
                            if ((ConnectionTable[k].status.bits.isValid) && isSameAddress(ActiveScanResults[i].Address,ConnectionTable[k].Address) && (ConnectionTable[k].MiWiInterfaces & 0x01)){

                            Printf("\r\n NODE FOUND");
                            Miwi_Conn_Index = MiApp_EstablishConnection(j, CONN_MODE_DIRECT, mb);
                                if (GetSPIErrorCounter() != spi_prev_ec){
                                    ResetSPIErrorCounter();
                                    Printf("\r\nSPI errors occured during MiWi initialization.");
                                    return SPI_ERROR;
                                }
                                if (Miwi_Conn_Index == 0xFF){
                                    Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                    break;
                                }
                                else {
                                    //Printf("\r\n\n JOIN SUCCESS!! \n\n");
                                    break;
                                }
                            }
                        }
                    }
                    Printf("\r\n NEW CONNECTION");
                    //at this point he is not connected no any other node
                    Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            Printf("\r\nSPI errors occured during MiWi initialization.");
                            return SPI_ERROR;
                        }
                        if (Miwi_Conn_Index == 0xFF){
                            Printf("\r\n\nError: JOIN FAIL!! \n\n");
                        }
                        //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }
                else if (mb == ISM_2G4){
                    #if defined MIWI_2400_RI
                    //try to find the father in the scanResults and join it

                    for (j=0; j< i ; j++){
                        for (k=0; k<CONNECTION_SIZE; k++){
                            if ((ConnectionTable[k].status.bits.isValid) && isSameAddress(ActiveScanResults[i].Address,ConnectionTable[k].Address) && (ConnectionTable[k].MiWiInterfaces & 0x01)){

                                        Printf("\r\n NODE FOUND\n\n");
                                        Miwi_Conn_Index = MiApp_EstablishConnection(j, CONN_MODE_DIRECT, mb);
                                        if (GetSPIErrorCounter() != spi_prev_ec){
                                                ResetSPIErrorCounter();
                                                Printf("\r\nSPI errors occured during MiWi initialization.");
                                                return SPI_ERROR;
                                        }
                                        if (Miwi_Conn_Index == 0xFF){
                                                Printf("\r\n\nError: JOIN FAIL!! \n\n");
                                                goto COMMON;
                                        }
                                        else {
                                                //Printf("\r\n\n JOIN SUCCESS!! \n\n");
                                                goto COMMON;
                                        }
                            }
                        }
                    }
                    Printf("\r\n NEW CONNECTION\n\n");
                    //at this point he is not connected no any other node
                    Miwi_Conn_Index = MiApp_EstablishConnection(0, CONN_MODE_DIRECT, mb);
                        if (GetSPIErrorCounter() != spi_prev_ec){
                            ResetSPIErrorCounter();
                            Printf("\r\nSPI errors occured during MiWi initialization.");
                            return SPI_ERROR;
                        }
                        if (Miwi_Conn_Index == 0xFF){
                            Printf("\r\n\nError: JOIN FAIL!! \n\n");
                        }
                        //else Printf("\r\n\n JOIN SUCCESS!! \n\n");
                    #endif
                }



            #endif
        }
        else{
            Printf("\r\nNo PANs were found. Starting a network... ");

            /******************************************************************/
            // Function MiApp_StartConnection tries to establish a valid
            // connection before returning the index of connection table for the
            // partner device.
            // The first parameter is the mode of start connection. There are
            // two valid connection modes:
            //  - START_CONN_DIRECT     start the connection on current channel.
            //  - START_CONN_ENERGY_SCN perform an energy scan first, before
            //                          starting the connection on the channel
            //                          with least noise.
            //  - START_CONN_CS_SCN     perform a carrier sense scan first,
            //                          before starting the connection on the
            //                          channel with least carrier sense noise.
            // The second parameter is the scan duration, which has the same
            //     definition in Energy Scan. 10 is roughly 1 second. 9 is a
            //     half second and 11 is 2 seconds. Maximum scan duration is 14,
            //     or roughly 16 seconds.
            // The third parameter is the channel map. Bit 0 of the double word
            //     parameter represents channel 0. For the 2.4GHz frequency band
            //     all possible channels are channel 11 to channel 26. As the
            //     result, the bit map is 0x07FFF800. Stack will filter out all
            //     invalid channels, so the application only needs to pay
            //     attention to the channels that are not preferred.
            /******************************************************************/

            #if defined ENABLE_ED_SCAN &&  !(defined ENABLE_SLEEP)
                spi_prev_ec = GetSPIErrorCounter();
                MiApp_StartConnection(START_CONN_ENERGY_SCN, 9, 0xFFFFFFFF, mb);
                if (GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    Printf("\r\nSPI errors occured during MiWi initialization.");
                    return SPI_ERROR;
                }
            #else
            Printf("\r\n Sorry, RFD no puede ser PANC");
            #endif
        }

        /* Update Node Status **************************************************
         * MiWi Channels in NodeHAL are defined from 0 to (MIWINumChannels -1)
         * for simplicity, which correspond with a range of transceiver channels
         * between 0 and 31 depending on the transceiver used.
         *
         * After updated, the next MiWi interface is initialised. There is an
         * implicit order of initialization, due to the code here and above. (!)
         **********************************************************************/
COMMON:
        #if defined MIWI_0434_RI
            if (mb == ISM_434) {
                NodeStatus.MIWI0434_OpChannel= MIWI0434_currentChannel - \
                                               MIWI0434ConfChannelOffset;
            }
        #endif
        #if defined MIWI_0868_RI
            if (mb == ISM_868) {
                NodeStatus.MIWI0868_OpChannel= MIWI0868_currentChannel - \
                                               MIWI0868ConfChannelOffset;
            }
        #endif
        #if defined MIWI_2400_RI
            if (mb == ISM_2G4) {
                NodeStatus.MIWI2400_OpChannel = MIWI2400_currentChannel - \
                                                MIWI2400ConfChannelOffset;
            }
        #endif

        //SWDelay(100);      //Wait for connection responses
        goto SINGLE_RI_TASK;
    }
    #endif
INIT_DONE:
    Printf("\r\nAll MiWi interfaces have been initialised.\r\nMiWi connection "
           "table is showed:");
    DumpConnection(0xFF);
    return NO_ERROR;
}

static BYTE SendMIWI(miwi_band mb, BOOL isBroadcast, BYTE *Address, BOOL isLongAddr){
    BOOL SecEn = (NodeStatus.flags.bits.SecurityEN) ? TRUE : FALSE;
    BOOL aux;
    UINT8 spi_prev_ec;
    if (isBroadcast){
        spi_prev_ec = GetSPIErrorCounter();
        aux = MiApp_BroadcastPacket(SecEn, mb);
        if(GetSPIErrorCounter() != spi_prev_ec){
            ResetSPIErrorCounter();
            return SPI_ERROR;
        }
        if (aux) {return NO_ERROR;} else {return BROADCAST_TX_ERROR;}
    }
    else{
        if(isLongAddr){
            //Printf("\r\nUnicast MiWi packet - long (EUI or permanent) address mode.");
            spi_prev_ec = GetSPIErrorCounter();
            aux = MiApp_UnicastAddress(Address, TRUE, SecEn, mb);
            if(GetSPIErrorCounter() != spi_prev_ec){
                ResetSPIErrorCounter();
                return SPI_ERROR;
            }//Else: no SPI errors ocurred in the unicast process.

            if (aux) {return NO_ERROR;} else {return UNICAST_MIWILONGADDR_ERROR;}
        }
        else{
            #if defined PROTOCOL_P2P
                return;
            #else
                //Printf("\r\nUnicast MiWi packet - short (or alternative) address mode.");
                spi_prev_ec = GetSPIErrorCounter();
                aux = MiApp_UnicastAddress(Address, FALSE, SecEn, mb);
                if(GetSPIErrorCounter() != spi_prev_ec){
                    ResetSPIErrorCounter();
                    return SPI_ERROR;
                }
                if(aux) {return NO_ERROR;} else {return UNICAST_MIWISHORTADDR_ERROR;}
            #endif
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
#endif
//END OF MIWI INTERNAL FUNCTIONS////////////////////////////////////////////////

//WIFI INTERNAL FUNCTIONS///////////////////////////////////////////////////////
#ifdef WIFI_2400_RI
////////////////////////////////////////////////////////////////////////////////
#endif

//INTERRUPTION SERVICE ROUTINES ////////////////////////////////////////////////
#ifdef __PIC32MX__
////////////////////////////////////////////////////////////////////////////////
    void __ISR(_TIMER_1_VECTOR, ipl1AUTO) _TMR1_Interrupt_ISR(void){
      //  coreTMRvals[0] = ReadCoreTimer();
        WDTCONCLR = 0x8000;         //Disable WDT, just in case...
        mT1ClearIntFlag();
        #if defined WAKE_FROM_SLEEP_SOSC_T1
            if (SleepEventCounter > 0){
                //Exiting sleep mode...
                SleepEventCounter--;        //Decrease sleep counter
            }else
        #endif
        {
            #if defined NODE_DOES_MAINTENANCE_TASKS
                AllStacksTasks();
            #endif
            }
        BYTE i;
//        for(i = 0; i< 5; i++){
//            Print32Dec(coreTMRvals[i]);
//            Printf("\r\n");
//        }
//        Printf("\r\nCom: ");
//        Print32Dec(coreTMRvals[1] - coreTMRvals[0]);
//        //Print32Dec(coreTMRvals[1]+ coreTMRvals[4]+coreTMRvals[7]+coreTMRvals[10]-coreTMRvals[0]-coreTMRvals[3]-coreTMRvals[6]-coreTMRvals[9]);
//        Printf("\r\n434: ");
//        Print32Dec(coreTMRvals[2]-coreTMRvals[1]);
//        //Print32Dec(coreTMRvals[2]+ coreTMRvals[5]+coreTMRvals[8]-coreTMRvals[1]-coreTMRvals[4]-coreTMRvals[7]);
//        Printf("\r\n868: ");
//        Print32Dec(coreTMRvals[3]-coreTMRvals[2]);
//        //Print32Dec(coreTMRvals[3]+ coreTMRvals[6]+coreTMRvals[9]-coreTMRvals[2]-coreTMRvals[5]-coreTMRvals[6]);
//        ConsolePut('\r');
//        Printf("\r\n2G4: ");
//        Print32Dec(coreTMRvals[4]-coreTMRvals[3]);
//        //Print32Dec(coreTMRvals[3]+ coreTMRvals[6]+coreTMRvals[9]-coreTMRvals[2]-coreTMRvals[5]-coreTMRvals[6]);
//        ConsolePut('\r');

//        SWDelay(7000);
    }

 /*   void __ISR(_CHANGE_NOTICE_VECTOR, ipl6AUTO) _CN_Interrupt_ISR(void){
        WDTCONCLR = 0x8000;         //Disable WDT, just in case.
        unsigned int readValue;
        readValue = ReadBUTTONS();  //Read PORT to clear mismatch condition
        IFS1CLR = 0x00000001;       //Clear the CN interrupt flag status bit
        if(((readValue & BUTTON_1_PORT_MASK)==0) && (NodeStatus.flags.bits.NodeAsleep)){
            IEC1CLR = 0x00000001;   //Disable the CN Interrupt..
        }
    }
  */
////////////////////////////////////////////////////////////////////////////////
#endif
//END OF INTERRUPTION SERVICE ROUTINES /////////////////////////////////////////

int main(void){
    /*RESET EVENTS HANDLERS ***************************************************/
//    if(RCON & 0x0003){
//
//    }
//    else if (RCON & 0x0200){
//        //Configuration Mismatch Reset Handler.
//    }
//    //The following events try to display a message, as UART may be initialised.
//    else if(RCON & 0x0002){
//        //Brown-out Reset Handler
//        Printf("\r\n\nBrown-out Reset Event. Check batteries (if included).\r\r");
//    }
//    else if(RCON & 0x0010){
//        //Watchdog Timeout Reset Handler.
//        Printf("\r\n\nWatchdog Timeout Reset Event during run mode. Every ISR"
//                "must disbale WDT as a safety policy.\r\r");
//    }
//    else if (RCON & 0040){
//        //Software Reset Handler.
//        Printf("\r\n\nSoftware Reset Event.\r\r");
//    }
//    else
    if (RCON & 0x0080){
        //Master Clear Reset handler.
        if(RCON & 0x0008){
            RCONCLR = 0x0080;
            //If MCLR button is pressed during sleep mode: node exits sleep mode
            //(return from interrupt).
            SleepEventCounter = 0;
            asm volatile("eret");
        }
    }

    /*RESET EVENTS HANDLERS END ***********************************************/
    mainApp();
}
