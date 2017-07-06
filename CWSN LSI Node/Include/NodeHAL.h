/*******************************************************************************
 * File:   NodeHAL.h
 * Author: Juan Domingo Rebollo - Laboratorio de Sistemas Integrados (LSI) - UPM
 *
 * File Description: Node Hardware Abstraction Level
 * Change History:
 * Rev   Date         Description
 ******************************************************************************/
/* INCLUDES *******************************************************************/
#ifndef NodeHAL_H
#define NodeHAL_H

#include "GenericTypeDefs.h"    //Microchip - Type Definitions
#include "HardwareProfile.h"    //Microchip - Template modified for our design

//Chosen Protocol Configuration Header.
#if defined MIWI_0434_RI || defined MIWI_0868_RI || defined MIWI_2400_RI
    #if defined (PROTOCOL_P2P)
        #include "WirelessProtocols/P2P/P2P.h"
    #endif
    #if defined (PROTOCOL_MIWI)
        #include "WirelessProtocols/MiWi/MiWi.h"
    #endif
    #if defined (PROTOCOL_MIWIPRO)
        #include "WirelessProtocols/MiWiPRO/ConfigMiWiPRO.h"
    #endif
#endif

/* END of INCLUDES ************************************************************/

/* DEFINITIONS ****************************************************************/
//---------------------------- HAL ERROR CODES -------------------------------//
#define NO_ERROR                    0x00
//ALL ERROR CODES HAVE A FIX '1' IN THE MSB: max. 128 error codes are possible
  //RADIO INTERFACES ERRORS
#define UNKNOWN_INTERFACE_ERROR     0x80 //param ri is not a defined value.
#define INVALID_INTERFACE_ERROR     0x81 //param ri is invalid in a function.
#define UNAVAILABLE_INTERFACE_ERROR 0x82 //param ri is not implemented/used.
#define ASLEEP_INTERFACE_ERROR      0x83 //param ri is asleep and can't be used.
  //POWER MANAGEMENT ERRORS
#define TRANSCEIVER_PM_ERROR        0x90 //TRX fails to go to sleep or wake up
#define DATA_REQUEST_TX_ERROR       0x91 //TX of Data Request command failed
#define DATA_REQUEST_RX_ERROR       0x92 //RX of any response to DataReq failed.
#define AWAKE_IS_MANDATORY_ERROR    0x9E //Node config avoids ri to sleep.
#define INVALID_POWER_MODE_ERROR    0x9F //TRX doesn't recognise input pwr mode.
  //RADIOCOMMUNICATION: TX/RX DATA ERRORS
#define TX_BUFFER_FULL_ERROR        0xA0 //Full TX buffer. Data can't be written
#define RX_BUFFER_EMPTY_ERROR       0xA1 //Empty RX buffer. Data can't be read
#define PAYLOAD_START_ERROR         0xA2 //Payload starts with a reserved value.
#define BROADCAST_TX_ERROR          0xB0 //Broadcast TX error.
#define UNICAST_MIWILONGADDR_ERROR  0xB1 //Unicast TX error using MiWi longAddr
#define UNICAST_MIWISHORTADDR_ERROR 0xB2 //Unicast TX error using MiWi shortAddr
#define UNICAST_WIFIIPv4ADDR_ERROR  0xB3 //Not implemented yet
#define UNICAST_WIFIMACADDR_ERROR   0xB4 //Not implemented yet
#define UNAVAILABLE_RX_ADDR_ERROR   0xBD //Packet's SourceAddress is not present
#define UNAVAILABLE_RX_PCKT_ERROR   0xBE //No RX packet available.
#define INVALID_ADDR_MODE_ERROR     0xBF //addrMode invalid or incoherent for TX
  //RADIO FREQUENCY
#define INVALID_CHANNEL_ERROR       0xC0 //Invalid channel usage was attempted.
#define UNAVAILABLE_RSSI_ERROR      0xC1 //RSSI(Received Signal Strength Indication) from a RX packet can't be read.
#define UNAVAILABLE_LQI_ERROR       0xC2 //LQI(Link Quality Indication) from a RX packet can't be read.
#define INVALID_SECLEVEL_ERROR      0xCF //Invalid security level
  //SAVING/RESTORING NODE STATUS
#define CONNTABLE_EXCEEDED_ERROR    0xD0 //Connection table size exceeded.
  //GENERAL STACKS OR NODE ERRORS
#define MIWI0434_STACK_ERROR        0xF0 //MiWi at 434 MHz internal stack error.
#define MIWI0868_STACK_ERROR        0xF1 //MiWi at 868 MHz internal stack error.
#define MIWI2400_STACK_ERROR        0xF2 //MiWi at 2,4 GHz internal stack error.
#define WIFI_STACK_ERROR            0xF3 //WiFi at 2,4 GHz internal stack error.
#define UART_ERROR                  0xF4 //UART module detected an error.
#define USB_ERROR                   0xF5 //Not implemented yet.
#define SPI_ERROR                   0xF6 //A SPI module detected an error.
#define I2C_ERROR                   0xF7 //Not implemented yet.
#define INVALID_INPUT_ERROR         0xFA //General invalid input error.
#define MAX_SLPTIME_EXCEEDED_ERROR  0xFB //SleepTime demanded exceeds the limit.
#define MIN_SLPTIME_REQUIRED_ERROR  0xFC //SleepTime doesn't reach the minimum.
#define ASLEEP_NODE_ERROR           0xFD //Invalid operation in sleep mode.
#define HAL_INTERNAL_ERROR          0xFE //HAL unexpected situations.
#define UNKNOWN_ERROR               0xFF //Fatal error.

//------ BIT MASKS, ADDRESS MODES AND OTHER INPUT/OUTPUT HAL DEFINITIONS -----//
#define ERROR_BIT_MASK      0x80

#define MIWI_0434_RI_MASK   0x01
#define MIWI_0868_RI_MASK   0x02
#define MIWI_2400_RI_MASK   0x04
#define WIFI_2400_RI_MASK   0x08

#define BROADCAST_ADDRMODE      0x00
#define LONG_MIWI_ADDRMODE      0x10
#define SHORT_MIWI_ADDRMODE     0x11
#define WIFI_IPv4_ADDRMODE      0x20
#define WIFI_MAC_ADDRMODE       0x21

//---------------------------- DevNAT definitions ----------------------------//
#define DEV_ADDR_TABLE_SIZE     16      //Max. Device Address Table Size is 255
#define BROADCAST_DEV_ADDR      0xFFFF  //Broadcast Addr (no translation needed)

//----------------------- Radio Interfaces definitions -----------------------//

#if defined MIWI_0434_RI
    //Number of channels depends on data rate chosen. See Transceivers.h
    #if defined MRF49XA_1_IN_434
        #define MIWI0434NumChannels     MRF49XA_1_CHANNEL_NUM
    #elif defined MRF49XA_2_IN_434
        #define MIWI0434NumChannels     MRF49XA_2_CHANNEL_NUM
    #endif
    #define MIWI0434ConfChannelOffset   0
    #define MIWI0434DfltChannel 0   //Choose one from 0 to MIWI0434NumChannels-1
#endif

#if defined MIWI_0868_RI
    //Number of channels depends on data rate chosen. See Transceivers.h
    #if defined MRF49XA_1_IN_868
        #define MIWI0868NumChannels     MRF49XA_1_CHANNEL_NUM
    #elif defined MRF49XA_2_IN_868
        #define MIWI0868NumChannels     MRF49XA_2_CHANNEL_NUM
    #endif
    #define MIWI0868ConfChannelOffset   0
    #define MIWI0868DfltChannel 0   //Choose one from 0 to MIWI0868NumChannels-1
#endif

#if defined MIWI_2400_RI
    //MiWi at 2G4 Interface number of channels and Configuration Channel Offset
    //are defined according to MRF24J40MA Transceiver Module and Spectrum Band.
    #define MIWI2400NumChannels         16  //Number of channels
    #define MIWI2400ConfChannelOffset   11  //Transceiver range from 11 to 26.
    #define MIWI2400DfltChannel 0   //Choose one from 0 to MIWI2400NumChannels-1
#endif

#if defined WIFI_2400_RI
    #define WIFI2400NumChannels 20    //WiFi Interface number of channels
    #define WIFI2400DfltChannel 10    //Choose one from 0 to WIFINumChannel-1
    #define WIFI_SCAN_SIZE      3     //Bytes stored per channel during scanning
    #define MyUDPPort           777   //LSI
#endif


#define MIWI_CONN_ENTRY_SIZE sizeof(CONNECTION_ENTRY)   //Different for P2P/MiWi
/* END of DEFINITIONS *********************************************************/

/* DATA TYPES AND STRUCTURES **************************************************/
typedef enum{
    NONE, MIWI_0434, MIWI_0868, MIWI_2400, ALL_MIWI, WIFI_2400, ALL
}radioInterface;

typedef struct{
    //FLAGS STRUCT. Those related to not defined Radio Interfaces are negligible
    //         |---- GENERAL STATUS NIBBLE ----|--- RADIO INTERFACES NIBBLE ---|
    //Bits:     7       6       5       4       3       2       1       0
    //Flags:    SecEN   Asleep  NotUsed NotUsed WiFi2G4 MiWi2G4 MiWi868 MiWi434

    union{
        BYTE statusFlags;
        struct{
            #ifdef MIWI_0434_RI                 //Bit 0 - MIWI at 434 MHz
                BYTE MIWI0434isON   : 1;
            #else
                BYTE                : 1;
            #endif
            #ifdef MIWI_0868_RI                 //Bit 1 - MIWI at 868 MHz
                BYTE MIWI0868isON   : 1;
            #else
                BYTE                : 1;
            #endif
            #ifdef MIWI_2400_RI                 //Bit 2 - MIWI at 2,4 GHz
                BYTE MIWI2400isON   : 1;
            #else
                BYTE                : 1;
            #endif
            #ifdef WIFI_2400_RI                 //Bit 3 - WIFI at 2,4 GHz
                BYTE WIFI2400isON   : 1;
            #else
                BYTE                : 1;
            #endif
            BYTE                    : 2;        //Bits 4 & 5 - Not Used
            BYTE NodeAsleep         : 1;        //Bit 6 - Node Sleep Mode flag
            BYTE SecurityEN         : 1;        //Bit 7 - Security Enable flag
        }bits;
    }flags;

    //RADIO INTERFACES STATUS:
    //Operating channel, Full TX buffer flag, Empty RX buffer flag, scan results
    #if defined MIWI_0434_RI
        BYTE MIWI0434_OpChannel;
        BYTE scanMIWI0434 [MIWI0434NumChannels];
        BOOL MIWI0434_TXbuf_isFull;
        BOOL MIWI0434_RXbuf_isEmpty;
    #endif

    #if defined MIWI_0868_RI
        BYTE MIWI0868_OpChannel;
        BYTE scanMIWI0868 [MIWI0868NumChannels];
        BOOL MIWI0868_TXbuf_isFull;
        BOOL MIWI0868_RXbuf_isEmpty;
    #endif

    #if defined MIWI_2400_RI
        BYTE MIWI2400_OpChannel;
        BYTE scanMIWI2400 [MIWI2400NumChannels];
        BOOL MIWI2400_TXbuf_isFull;
        BOOL MIWI2400_RXbuf_isEmpty;
    #endif

    #if defined WIFI_2400_RI
        BYTE WIFI2400_OpChannel;
        BYTE scanWIFI2400 [WIFI2400NumChannels];
        BOOL WIFI2400_TXbuf_isFull;
        BOOL WIFI2400_RXbuf_isEmpty;
    #endif
} nodeStatus;

////Define a header structure for validating the AppConfig data structure in EEPROM/Flash
//typedef struct {
//    unsigned short wConfigurationLength;
//        // Number of bytes saved in EEPROM/Flash (sizeof(APP_CONFIG))
//    unsigned short wOriginalChecksum;
//        // Checksum of the original AppConfig defaults as loaded from ROM (to
//        // detect when to wipe the EEPROM/Flash record of AppConfig due to a
//        // stack change, such as when switching from Ethernet to Wi-Fi)
//    unsigned short wCurrentChecksum;
//        // Checksum of the current EEPROM/Flash data. This protects against
//        // using corrupt values if power failure occurs while writing them and
//        // helps detect coding errors in which some other task writes to the
//        // EEPROM in the AppConfig area.
//} NVM_VALIDATION_STRUCT;
/* END of DATA TYPES AND STRUCTURES *******************************************/

/* CONSTANTS VALIDATIONS ******************************************************/
#if defined MIWI_0434_RI
    #if MIWI0434NumChannels > 0xFE
        #error "MiWi at 434 MHz number of channels can't exceed 255."
    #endif
#endif
#if defined MIWI_0868_RI
    #if MIWI0868NumChannels > 0xFE
        #error "MiWi at 868 MHz number of channels can't exceed 255."
    #endif
#endif
#if defined MIWI_2400_RI
    #if MIWI2400NumChannels > 0xFE
        #error "MiWi at 868 MHz number of channels can't exceed 255."
    #endif
#endif
#if defined WIFI_2400_RI
    #if WIFI2400NumChannels > 0xFE
        #error "WiFi at 2,4 GHz number of channels can't exceed 255."
    #endif
#endif
/* END of CONSTANTS VALIDATIONS ***********************************************/

////////////////////////////////////////////////////////////////////////////////
/*********************** HAL FUNCTION PROTOTYPES ******************************/
////////////////////////////////////////////////////////////////////////////////
//Initialization
BYTE InitNode();

//Status
BYTE GetStatusFlags();
BYTE GetOpChannel(radioInterface ri);
BYTE GetPayloadToRead(radioInterface ri);
BYTE GetFreeTXBufSpace(radioInterface ri);

//Power Management
BYTE SleepNode(radioInterface forceWakeUp, UINT32 slpTime_ms);
BYTE SleepRadioInterface(radioInterface ri);
//BYTE WakeUpNode(radioInterface ri);
BYTE WakeUpRadioInterface(radioInterface ri);
BYTE SetTXPower(radioInterface ri, BYTE powerOutput);
BYTE GetTXPower(radioInterface ri, BYTE *storeItHere);
BYTE SwitchOnRI(radioInterface ri);
BYTE SwitchOffRI(radioInterface ri);

BYTE SwitchOnLed(BYTE led);
BYTE SwitchOffLed(BYTE led);

//Send & Receive - Radio Communication300
BYTE PutTXData(radioInterface ri, BYTE data);
BYTE DiscardTXData(radioInterface ri);
BYTE SendPckt(radioInterface ri, BYTE AddrMode, BYTE *Address);
BYTE WhichRIHasData();
BYTE GetRXData(radioInterface ri, BYTE *storeItHere);
BYTE GetRXSourceAddr(radioInterface ri, BYTE *storeItFromHere);

//Security
BYTE SetSecurityLevel(BYTE SecLevel);

//RadioFrequency
BYTE SetChannel(radioInterface ri, BYTE channel);
BYTE DoChannelScanning(radioInterface ri, BYTE *storeInfoHere);
BYTE GetRSSI(radioInterface ri, BYTE *storeItHere);
BYTE GetLQI(radioInterface ri, BYTE *storeItHere);

//Node's variables saving:
BYTE SaveConnTable(BYTE *storeItFromHere);
BYTE RestoreConnTable(BYTE *takeItFromHere, BYTE numConn);
#if defined PROTOCOL_MIWI && defined NWK_ROLE_COORDINATOR
    BYTE SaveRoutingTable(radioInterface ri, BYTE *storeItFromHere);
    BYTE RestoreRoutingTable(radioInterface ri, BYTE *takeItFromHere);
#endif

//Stacks Maintenance
#if defined APP_DOES_MANTEINANCE_TASKS
    void AllStacksTasks();
#endif

////////////////////////////////////////////////////////////////////////////////
/******************** Helpers, Console and Debug functions ********************/
////////////////////////////////////////////////////////////////////////////////

#define SWDelay(ms)             DelayMs(ms)
BYTE DumpRXPckt(radioInterface ri);

UINT16 GetSentPckts(radioInterface ri);
UINT16 GetProcPckts(radioInterface ri);
void Print32Dec(unsigned int toPrint);
//ENABLE_DUMP MUST BE ENABLED for the following functions. See ConfigApp.h
#if defined ENABLE_DUMP
    #define DumpConnTable()         DumpConnection(0xFF)
    #define DumpSingleConn(index)   DumpConnection(index)
#endif

#endif
