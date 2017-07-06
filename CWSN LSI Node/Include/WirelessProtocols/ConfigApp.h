 /*******************************************************************************
 * Software License Agreement                   ¡¡TEMPLATE FOR OUR APPLICATION!!
 *
 * Copyright 2007-2010 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital signal
 * controller and used with a Microchip radio frequency transceiver, which are
 * integrated into your product or third party product (pursuant to the terms in
 * the accompanying license agreement).
 *
 * You should refer to the license agreement accompanying this Software for
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
 * CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
 * OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 * INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
 * SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 ******************************************************************************/
#ifndef __CONFIG_APP_H_
#define __CONFIG_APP_H_


#include "HardwareConfig.h"
////////////////////////////////////////////////////////////////////////////////
/*LSI-CWSN - IMPORTANT:
* The following code defines the platform as well as the hardware configuration.
* A MiWi template has been modified in order to admit more than one transceiver,
* including 2 MRF49XA which must operate in different frequency bands.*/
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------//
//------------- CONFIGURATION FOR LSI-CWSN NodeTest APP. CODE ----------------//
//----------------------------------------------------------------------------//
//Select only one of these...
//#define NODE_1      //TX role in App. code - node 1. Sets EUI_testA address.
#define NODE_2      //RX role in App. code - node 2. Sets EUI_testB address.
//#define NODE_3      //RX role in App. code - node 3. Sets EUI_testC address.
//#define NODE_4      //RX role in App. code - node 4. Sets EUI_testD address.
//#define NODE_5
//#define NODE_NOP    //Dummy role - It only joins the network at the beginning
                    //and does stacks maintenance tasks. Sets EUI_testC address.
    //#define NOP_JOINS_AND_LEAVES_LOOP     //Provisional, debugging!
    #define NOP_DUMPS_CONN_TABLE


//----------------------------------------------------------------------------//
//------ CONFIGURATION OF MIWI STACK AND MIWI TRANSCEIVERS FOR THE NODE ------//


    //------------------------------------------------------------------------//
    // Definition of Protocol Stack. ONLY ONE PROTOCOL STACK CAN BE CHOSEN
    //------------------------------------------------------------------------//
        /**********************************************************************/
        // PROTOCOL_P2P enables the application to use MiWi P2P stack. This
        // definition cannot be defined with PROTOCOL_MIWI.
        /**********************************************************************/
        //#define PROTOCOL_P2P

        /**********************************************************************/
        // PROTOCOL_MIWI enables the application to use MiWi mesh networking
        // stack. This definition cannot be defined with PROTOCOL_P2P or
        // PROTOCOL_MIWI_PRO
        /**********************************************************************/
        #define PROTOCOL_MIWI

        /**********************************************************************/
        // PROTOCOL_MIWI_PRO enables the application to use MiWi PRO networking
        // stack. This definition cannot be defined with PROTOCOL_P2P or
        // PROTOCOL_MIWI
        /**********************************************************************/
        //#define PROTOCOL_MIWI_PRO   //NOT ADAPTED YET FOR MCHP LSI-CWSN STACK!

            /******************************************************************/
            // NWK_ROLE_END_DEVICE is not valid if PROTOCOL_P2P is defined. It
            // specifies that the node has the capability to be an end device.
            // This definition cannot be defined with NWK_ROLE_COORDINATOR.
            /******************************************************************/
            //#define NWK_ROLE_END_DEVICE

            /******************************************************************/
            // NWK_ROLE_COORDINATOR is not valid if PROTOCOL_P2P is defined. It
            // specifies that the node has the capability to be a coordinator.
            // This definition cannot be defined with NWK_ROLE_END_DEVICE.
            /******************************************************************/
            #define NWK_ROLE_COORDINATOR

    //--------------------------------------------------------------------------
    // PAN IDentifier and Node's MiWi Permanent Address
    //--------------------------------------------------------------------------
        /**********************************************************************/
        // MY_PAN_ID defines the PAN identifier
        /**********************************************************************/       

#define MY_PAN_ID       0x1234


        /**********************************************************************/
        // MY_ADDRESS_LENGTH defines the size of wireless node permanent address
        // in bytes. This definition is not valid for IEEE 802.15.4 compliant RF
        // transceivers.
        // IMPORTANT: It could be redefined in this file!
        /**********************************************************************/
        #define MY_ADDRESS_LENGTH       8
            //MY_ADDRESS_LENGTH VALIDATION.
            #if MY_ADDRESS_LENGTH > 8
                #error "Maximum address length is 8"
            #endif
            #if MY_ADDRESS_LENGTH < 2
                #error "Minimum address length is 2"
            #endif

            #if defined(MRF24J40)
                #define IEEE_802_15_4
                #undef MY_ADDRESS_LENGTH
                #define MY_ADDRESS_LENGTH 8
            #endif

        /**********************************************************************/
        // EUI_x defines the xth byte of permanent address for the wireless node
        // This is the Node Permanent MiWi Address. It must be unique worldwide.
        // Check assignment policies.           (64 bits according to 802.15.4)
        // JUAN: Permanent addr is unique and is shared for all MiWi interfaces.
        /**********************************************************************/
        #if defined NODE_1
            #define EUI_7 0x11      //CWSN-LSI, EUI_testA
        #elif defined NODE_2
            #define EUI_7 0x22      //CWSN-LSI, EUI_testB
        #elif defined NODE_3
            #define EUI_7 0x33      //CWSN-LSI, EUI_testC
        #elif defined NODE_4
            #define EUI_7 0x44      //CWSN-LSI, EUI_testC
        #elif defined NODE_5
            #define EUI_7 0x55
        #elif defined NODE_NOP
            #define EUI_7 0xFF      //CWSN-LSI, EUI_testC
        #else
            #define EUI_7 0x77      //Introduce here the real assigned address
        #endif
            #define EUI_6 0x66      //Introduce here the real assigned address
            #define EUI_5 0x55      //Introduce here the real assigned address
            #define EUI_4 0x44      //Introduce here the real assigned address
            #define EUI_3 0x33      //Introduce here the real assigned address
            #define EUI_2 0x22      //Introduce here the real assigned address
            #define EUI_1 0x11      //Introduce here the real assigned address
            #define EUI_0 0x00      //Introduce here the real assigned address

    //--------------------------------------------------------------------------
    // P2P Connection Table Size and Additional Node ID Size
    //--------------------------------------------------------------------------
        /**********************************************************************/
        // ADDITIONAL_NODE_ID_SIZE defines the size of additional payload will
        // be attached to the P2P Connection Request. Additional payload is the
        // information that the devices what to share with their peers on the
        // P2P connection. The additional payload will be defined by the
        // application and defined in NodeHAL.c
        /**********************************************************************/
        #define ADDITIONAL_NODE_ID_SIZE 0  //LSI - For including MyDeviceAddress

        /**********************************************************************/
        // P2P_CONNECTION_SIZE defines the maximum P2P connections that this
        // device allowes at the same time.
        /***********************************************************************
         * Juan: MiWi stack uses a byte to refer to a connection index, but also 
         * reserves 0xFF for invalid index. Therefore, 255 would be the maximum 
         * size permitted. However, in order to report errors, this value is 
         * limited to 128.
         * It has to be pointed out that connection table has a deep impact on 
         * memory usage. In order to keep the memory requirements low, use a 
         * small size for connection table. Also, stacks' tasks will last for 
         * more time if more connections are managed.
         * Recommended values go from 8 to 32, depending on application's needs 
         * and the network topology.
         **********************************************************************/
        #define CONNECTION_SIZE     5
            #if CONNECTION_SIZE > 128   //Juan: limited by NodeHAL error codes.
                #undef CONNECTION_SIZE
                #define CONNECTION_SIZE 128
            #endif

    //--------------------------------------------------------------------------
    // TX_BUFFER_SIZE and RX_BUFFER_SIZE
    //--------------------------------------------------------------------------
        /**********************************************************************/
        // TX/RX buffer sizes are defined for each transceiver. This variables
        // define the maximum size of application payload (in bytes) which is to
        // be transmitted / received.
        // MiWi Stack original variables were: TX_BUFFER_SIZE, RX_BUFFER_SIZE
        /**********************************************************************/
        //Transceivers buffer size
        #if defined MRF24J40
            #define MRF24J40_TX_BUF_SIZE 87
            #define MRF24J40_RX_BUF_SIZE 87
            //Max size with MIC_SIZE 4, ADDR_SIZE 8:   P2P:<=90  MIWI: SIZE<=79
            //BUFFER SIZES VALIDATION.
                #if (MRF24J40_RX_BUF_SIZE > 127) || (MRF24J40_RX_BUF_SIZE < 10)
                    #error "MRF24J40_RX_BUF_SIZE must be between 10 and 127."
                #endif
                #if (MRF24J40_TX_BUF_SIZE > 127) || (MRF24J40_TX_BUF_SIZE < 10)
                    #error "MRF24J40_TX_BUF_SIZE must be between 10 and 127."
                #endif
        #endif
        #if defined(MRF49XA_1) || defined(MRF49XA_2)
            #define MRF49XA_TX_BUF_SIZE 93
            #define MRF49XA_RX_BUF_SIZE 93
            //Max size with MIC_SIZE 4, ADDR_SIZE 4:   P2P:<105  MIWI: SIZE<
            //Max size with MIC_SIZE 4, ADDR_SIZE 8:   P2P:<97   MIWI: SIZE<
            //BUFFER SIZES VALIDATION.
                #if (MRF49XA_RX_BUF_SIZE > 127) || (MRF49XA_RX_BUF_SIZE < 10)
                    #error "MRF49XA_RX_BUF_SIZE must be between 10 and 127."
                #endif
                #if (MRF49XA_TX_BUF_SIZE > 127) || (MRF49XA_TX_BUF_SIZE < 10)
                    #error "MRF49XA_TX_BUF_SIZE must be between 10 and 127."
                #endif
        #endif
        #if defined MRF89XA
            #define MRF89XA_TX_BUF_SIZE 30
            #define MRF89XA_RX_BUF_SIZE 30
            //BUFFER SIZES VALIDATION.
                #if (MRF89XA_RX_BUF_SIZE > 127) || (MRF89XA_RX_BUF_SIZE < 10)
                    #error "MRF89XA_RX_BUF_SIZE must be between 10 and 127."
                #endif
                #if (MRF89XA_TX_BUF_SIZE > 127) || (MRF89XA_TX_BUF_SIZE < 10)
                    #error "MRF89XA_TX_BUF_SIZE must be between 10 and 127."
                #endif
        #endif

        //MiWi "original" stack buffer size. Now only used for INDIRECT PACKET
        //in P2P Protocol. The size must be equal or greater than any single
        //transceiver's buffer size.
        #if MRF49XA_TX_BUF_SIZE > MRF24J40_TX_BUF_SIZE
            #define TX_BUFFER_SIZE MRF49XA_TX_BUF_SIZE
        #else
            #define TX_BUFFER_SIZE MRF24J40_TX_BUF_SIZE
        #endif
        #if MRF49XA_RX_BUF_SIZE > MRF24J40_RX_BUF_SIZE
            #define RX_BUFFER_SIZE MRF49XA_RX_BUF_SIZE
        #else
            #define RX_BUFFER_SIZE MRF24J40_RX_BUF_SIZE
        #endif

// STACK CONFIGURATION OPTIONS -----------------------------------------------//

    /**************************************************************************/
    // ENABLE_DUMMY_BYTE (LSI CWSN Node only!) It was detected that MiWi payload
    // can't start with 0x00. Otherwise, payload data will be treated as a stack
    // report. To fix this issue, there are two possibilities:
    // 1) return a NodeHAL error code and raise an error message (disabled) or
    // 2) Write always a dummy byte as the first payload byte and remove it when
    // receiving a packet with user data (enabled).
    /**************************************************************************/
    #if defined PROTOCOL_MIWI
        //#define ENABLE_DUMMY_BYTE             //Enable to write a dummy byte.
    #endif

    /**************************************************************************/
    // ENABLE_NETWORK_FREEZER enables the network freezer feature, which stores
    // critical network information into non-volatile memory, so that the
    // protocol stack can recover from power loss gracefully. The network infor
    // can be saved in data EPROM of MCU, external EEPROM or programming space,
    // if enhanced flash is used in MCU. Network freezer feature needs
    // definition of NVM kind to be used, specified in HardwareProfile.h
    /**************************************************************************/
    //#define ENABLE_NETWORK_FREEZER        //Not adapted for LSI CWSN stack!!!!
        #if defined(ENABLE_NETWORK_FREEZER)
            #define ENABLE_NVM
            //#define ENABLE_NVM_MAC
        #endif

    /**************************************************************************/
    // HARDWARE_SPI enables the hardware SPI implementation on MCU silicon. If
    // HARDWARE_SPI is not defined, digital I/O pins will be used to bit-bang
    // the RF transceiver
    /**************************************************************************/
    #define HARDWARE_SPI

    /**************************************************************************/
    // ENABLE_SECURITY will enable the device to encrypt and decrypt information
    // transferred
    /**************************************************************************/
    //#define ENABLE_SECURITY

    /**************************************************************************/
    // ENABLE_INDIRECT_MESSAGE will enable the device to store the packets for 
    // the sleeping devices temporily until they wake up and ask for the 
    // messages
    /**************************************************************************/
    #define ENABLE_INDIRECT_MESSAGE

    /**************************************************************************/
    // ENABLE_BROADCAST_TO_SLEEP_DEVICE will enable the device to save broadcast
    // messages for sleeping devices until they wake up and ask for them.
    /**************************************************************************/
    //#define ENABLE_BROADCAST_TO_SLEEP_DEVICE        //#define ENABLE_BROADCAST
    //Juan, LSI-CWSN MiWi stack: redundant options Broadcast and Broadcast to
    //sleep device have been merged into the second one.

    /**************************************************************************/
    // ENABLE_PA_LNA enable the external power amplifier and low noise amplifier
    // on the RF board to achieve longer radio communication range. To enable 
    // PA/LNA on RF board without power amplifier and low noise amplifier may be
    // harmful to the transceiver.
    /**************************************************************************/
    //#define ENABLE_PA_LNA

    /**************************************************************************/
    // ENABLE_HAND_SHAKE enables the protocol stack to hand-shake before 
    // communicating with each other. Without a handshake process, RF 
    // transceivers can only broadcast, or hardcoded the destination address to
    // perform unicast.
    /**************************************************************************/
    #define ENABLE_HAND_SHAKE

    /**************************************************************************/
    // TARGET_SMALL will remove the support of inter PAN communication and other
    // minor features to save programming space
    /**************************************************************************/
    //#define TARGET_SMALL

    /**************************************************************************/
    // ENABLE_ED_SCAN will enable the device to do an energy detection scan to
    // find out the channel with least noise and operate on that channel
    /**************************************************************************/
    #define ENABLE_ED_SCAN

//#define SIMPLE_EXAMPLE
// Simple example disables the following features...
#ifndef SIMPLE_EXAMPLE
    /**************************************************************************/
    // ENABLE_ACTIVE_SCAN will enable the device to do an active scan to detect
    // current existing connection.
    /**************************************************************************/
    #define ENABLE_ACTIVE_SCAN

    /**************************************************************************/
    // ENABLE_SLEEP will enable the device to go to sleep and wake up from the
    // sleep
    /**************************************************************************/
    //#define ENABLE_SLEEP

    #if defined ENABLE_SLEEP & defined PROTOCOL_MIWI & defined NWK_ROLE_COORDINATOR
        #error "Coordinators can't be sleeping devices"
    #endif
    /**************************************************************************/
    // RFD_WAKEUP_INTERVAL defines the wake up interval for RFDs in second.
    // This definition is for the FFD devices to calculated various timeout. 
    // RFD depends on the setting of the watchdog timer to wake up, thus this 
    // definition is not used.
    /**************************************************************************/
    #define RFD_WAKEUP_INTERVAL     8

    /**************************************************************************/
    // ENABLE_FREQUENCY_AGILITY will enable the device to change operating
    // channel to bypass the sudden change of noise
    /**************************************************************************/
    //#define ENABLE_FREQUENCY_AGILITY
#endif

//----------------------------------------------------------------------------//
//------------------------- CONSTANTS VALIDATION -----------------------------//
//----------------------------------------------------------------------------//

#if !defined(PROTOCOL_P2P) && !defined(PROTOCOL_MIWI) && \
    !defined(PROTOCOL_MIWI_PRO)
    #error  "One Microchip proprietary protocol must be defined for the "
            "wireless application."
#endif
#if (defined(PROTOCOL_P2P)  &&  defined(PROTOCOL_MIWI))     || \
    (defined(PROTOCOL_P2P)  &&  defined(PROTOCOL_MIWI_PRO)) || \
    (defined(PROTOCOL_MIWI) &&  defined(PROTOCOL_MIWI_PRO))
    #error  "Define only one Microchip proprietary protocol."
#endif

#if defined(ENABLE_ACTIVE_SCAN) && defined(TARGET_SMALL)
    #error  Target_Small and Enable_Active_Scan cannot be defined together
#endif

#if defined(ENABLE_INDIRECT_MESSAGE) && !defined(RFD_WAKEUP_INTERVAL)
    #error "RFD Wakeup Interval must be defined if indirect message is enabled"
#endif

#if (NETWORK_TABLE_SIZE > 0xFE)
    #error NETWORK TABLE SIZE too large. Must be < 0xFF.
#endif

//EOF
#endif