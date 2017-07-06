/*******************************************************************************
* FileName:	Transceivers.h                ¡¡¡MODIFIED FROM ORIGINAL STACK!!!
* Dependencies:
* Processor:
* Compiler:     Microchip C18 v3.04 or higher
*               Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher
* Company:	Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice
*
* Copyright ? 2007-2010 Microchip Technology Inc.  All rights reserved.
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
* SOFTWARE AND DOCUMENTATION ARE PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
* MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
* CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
* OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
* INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
* SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
* (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
********************************************************************************
* File Description:
*
* Microchip MiWi Stack variation for including several Microchip RF transceivers
* simultaneously in order to use different frequency bands.
* This particular file contains common definitions for all MiWi transceivers and
* specific definitions for each of the MiWi transceivers considered in this
* design.
*
* MiWi protocols and transceivers are defined in "ConfigApp.h" file.
* MiWi transceivers' configuration options (most of them) are defined in 
* "ConfigTransceivers.h" file.
*
* Original MiWi Stack:      Rev 4.1   6/3/2011.
* Author:  Juan Domingo Rebollo - LSI - Laboratorio de Sistemas Intergados - UPM
*******************************************************************************/
#if !defined __TRANSCEIVERS_H
    #define __TRANSCEIVERS_H

#include "Transceivers/ConfigTransceivers.h"
#include "GenericTypeDefs.h"
#include "Common/TimeDelay.h"
#include "Compiler.h"
#include "WirelessProtocols/SymbolTime.h"
#include "Transceivers/MCHP_MAC.h"

// PROTOCOL CHOSEN. All MiWi transceivers will use the same protocol stack.
#if defined(PROTOCOL_P2P)
    #include "WirelessProtocols/P2P/P2P.h"
#elif defined(PROTOCOL_MIWI)
    #include "WirelessProtocols/MiWi/MiWi.h"
#elif defined (PROTOCOL_MIWI_PRO)
    #include "WirelessProtocols/MiWiPRO/MiWiPRO.h"
#endif

#if defined(PROTOCOL_P2P)
    #define PROTOCOL_HEADER_SIZE 0
#elif defined(PROTOCOL_MIWI)
    #define PROTOCOL_HEADER_SIZE 11      //MIWI_HEADER_LEN
#elif defined(PROTOCOL_MIWI_PRO)
    #define PROTOCOL_HEADER_SIZE MIWI_PRO_HEADER_LEN        //11?
#endif

//----------------------------------------------------------------------------//
//-------------------------------- MRF24J40 ----------------------------------//
//----------------------------------------------------------------------------//
#if defined(MRF24J40)
    #define IEEE_802_15_4

    #if !defined(_ZMRF24J40_H_) && defined(MRF24J40)
        #define _ZMRF24J40_H_

        #if defined(ENABLE_SECURITY)
            #if MRF24J40_SECURITY_LEVEL == 0x01
                #define MIC_SIZE 0
            #elif (MRF24J40_SECURITY_LEVEL == 0x02) || (MRF24J40_SECURITY_LEVEL == 0x05)
                #define MIC_SIZE 16
            #elif (MRF24J40_SECURITY_LEVEL == 0x03) || (MRF24J40_SECURITY_LEVEL == 0x06)
                #define MIC_SIZE 8
            #elif (MRF24J40_SECURITY_LEVEL == 0x04) || (MRF24J40_SECURITY_LEVEL == 0x07)
                #define MIC_SIZE 4
            #endif

            #define MRF24J40_RX_PCKT_SIZE (MRF24J40_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+ \
                    MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+MIC_SIZE+17)
            #if (MRF24J40_RX_PCKT_SIZE > 127) && !defined(__18CXX)
                #warning "Maximum application payload MRF24J40_RX_PCKT_SIZE is "\
                         "exceeded. Default: <=90 for P2P; <=79 for MIWI"
            #endif
        #else
            #define MRF24J40_RX_PCKT_SIZE (MRF24J40_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+ \
                    MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+12)
            #if (MRF24J40_RX_PCKT_SIZE > 127) && !defined(__18CXX)
                #warning "Maximum application payload MRF24J40_RX_PCKT_SIZE is "\
                         "exceeded. Default: <=95 for P2P; <=84 for MIWI"
            #endif
        #endif

        #if MRF24J40_RX_PCKT_SIZE > 127
            #undef MRF24J40_RX_PCKT_SIZE
            #define MRF24J40_RX_PCKT_SIZE 127
        #endif

        //long address registers
        #define RFCTRL0             (0x200)
        #define RFCTRL1             (0x201)
        #define RFCTRL2             (0x202)
        #define RFCTRL3             (0x203)
        #define RFCTRL4             (0x204)
        #define RFCTRL5             (0x205)
        #define RFCTRL6             (0x206)
        #define RFCTRL7             (0x207)
        #define RFCTRL8             (0x208)
        #define CAL1                (0x209)
        #define CAL2                (0x20a)
        #define CAL3                (0x20b)
        #define SFCNTRH             (0x20c)
        #define SFCNTRM             (0x20d)
        #define SFCNTRL             (0x20e)
        #define RFSTATE             (0x20f)
                #define RF_RSSI             (0x210)
                //Juan: NAME MODIFIED (added prefix "RF_") to avoid conflict
                //with MRF49XA_STATUS. Anyway, this definition is not used...
        #define CLKIRQCR            (0x211)
        #define SRCADRMODE          (0x212)
        #define SRCADDR0            (0x213)
        #define SRCADDR1            (0x214)
        #define SRCADDR2            (0x215)
        #define SRCADDR3            (0x216)
        #define SRCADDR4            (0x217)
        #define SRCADDR5            (0x218)
        #define SRCADDR6            (0x219)
        #define SRCADDR7            (0x21a)
        #define RXFRAMESTATE        (0x21b)
        #define SECSTATUS           (0x21c)
        #define STCCMP              (0x21d)
        #define HLEN                (0x21e)
        #define FLEN                (0x21f)
        #define SCLKDIV             (0x220)
        //#define reserved          (0x221)
        #define WAKETIMEL           (0x222)
        #define WAKETIMEH           (0x223)
        #define TXREMCNTL           (0x224)
        #define TXREMCNTH           (0x225)
        #define TXMAINCNTL          (0x226)
        #define TXMAINCNTM          (0x227)
        #define TXMAINCNTH0         (0x228)
        #define TXMAINCNTH1         (0x229)
        #define RFMANUALCTRLEN      (0x22a)
        #define RFMANUALCTRL        (0x22b)
        #define RFRXCTRL            RFMANUALCTRL
        #define TxDACMANUALCTRL     (0x22c)
        #define RFMANUALCTRL2       (0x22d)
        #define TESTRSSI            (0x22e)
        #define TESTMODE            (0x22f)

        #define NORMAL_TX_FIFO      (0x000)
        #define BEACON_TX_FIFO      (0x080)
        #define GTS1_TX_FIFO        (0x100)
        #define GTS2_TX_FIFO        (0x180)

        #define RX_FIFO             (0x300)

        #define SECURITY_FIFO       (0x280)

        //short address registers for reading
        #define READ_RXMCR          (0x00)
        #define READ_PANIDL         (0x02)
        #define READ_PANIDH         (0x04)
        #define READ_SADRL          (0x06)
        #define READ_SADRH          (0x08)
        #define READ_EADR0          (0x0A)
        #define READ_EADR1          (0x0C)
        #define READ_EADR2          (0x0E)
        #define READ_EADR3          (0x10)
        #define READ_EADR4          (0x12)
        #define READ_EADR5          (0x14)
        #define READ_EADR6          (0x16)
        #define READ_EADR7          (0x18)
        #define READ_RXFLUSH        (0x1a)
        #define READ_TXSTATE0       (0x1c)
        #define READ_TXSTATE1       (0x1e)
        #define READ_ORDER          (0x20)
        #define READ_TXMCR          (0x22)
        #define READ_ACKTMOUT       (0x24)
        #define READ_SLALLOC        (0x26)
        #define READ_SYMTICKL       (0x28)
        #define READ_SYMTICKH       (0x2A)
        #define READ_PAONTIME       (0x2C)
        #define READ_PAONSETUP      (0x2E)
        #define READ_FFOEN          (0x30)
        #define READ_CSMACR         (0x32)
        #define READ_TXBCNTRIG      (0x34)
        #define READ_TXNMTRIG       (0x36)
        #define READ_TXG1TRIG       (0x38)
        #define READ_TXG2TRIG       (0x3A)
        #define READ_ESLOTG23       (0x3C)
        #define READ_ESLOTG45       (0x3E)
        #define READ_ESLOTG67       (0x40)
        #define READ_TXPEND         (0x42)
        #define READ_TXBCNINTL      (0x44)
        #define READ_FRMOFFSET      (0x46)
        #define READ_TXSR           (0x48)
        #define READ_TXLERR         (0x4A)
        #define READ_GATE_CLK       (0x4C)
        #define READ_TXOFFSET       (0x4E)
        #define READ_HSYMTMR0       (0x50)
        #define READ_HSYMTMR1       (0x52)
        #define READ_SOFTRST        (0x54)
        #define READ_BISTCR         (0x56)
        #define READ_SECCR0         (0x58)
        #define READ_SECCR1         (0x5A)
        #define READ_TXPEMISP       (0x5C)
        #define READ_SECISR         (0x5E)
        #define READ_RXSR           (0x60)
        #define READ_ISRSTS         (0x62)
        #define READ_INTMSK         (0x64)
        #define READ_GPIO           (0x66)
        #define READ_GPIODIR        (0x68)
        #define READ_SLPACK         (0x6A)
        #define READ_RFCTL          (0x6C)
        #define READ_SECCR2         (0x6E)
        #define READ_BBREG0         (0x70)
        #define READ_BBREG1         (0x72)
        #define READ_BBREG2         (0x74)
        #define READ_BBREG3         (0x76)
        #define READ_BBREG4         (0x78)
        #define READ_BBREG5         (0x7A)
        #define READ_BBREG6         (0x7C)
        #define READ_RSSITHCCA      (0x7E)

        //short address registers for writing
        #define WRITE_RXMCR         (0x01)
        #define WRITE_PANIDL        (0x03)
        #define WRITE_PANIDH        (0x05)
        #define WRITE_SADRL         (0x07)
        #define WRITE_SADRH         (0x09)
        #define WRITE_EADR0         (0x0B)
        #define WRITE_EADR1         (0x0D)
        #define WRITE_EADR2         (0x0F)
        #define WRITE_EADR3         (0x11)
        #define WRITE_EADR4         (0x13)
        #define WRITE_EADR5         (0x15)
        #define WRITE_EADR6         (0x17)
        #define WRITE_EADR7         (0x19)
        #define WRITE_RXFLUSH       (0x1B)
        #define WRITE_TXSTATE0      (0x1D)
        #define WRITE_TXSTATE1      (0x1F)
        #define WRITE_ORDER         (0x21)
        #define WRITE_TXMCR         (0x23)
        #define WRITE_ACKTMOUT      (0x25)
        #define WRITE_SLALLOC       (0x27)
        #define WRITE_SYMTICKL      (0x29)
        #define WRITE_SYMTICKH      (0x2B)
        #define WRITE_PAONTIME      (0x2D)
        #define WRITE_PAONSETUP     (0x2F)
        #define WRITE_FFOEN         (0x31)
        #define WRITE_CSMACR        (0x33)
        #define WRITE_TXBCNTRIG     (0x35)
        #define WRITE_TXNMTRIG      (0x37)
        #define WRITE_TXG1TRIG      (0x39)
        #define WRITE_TXG2TRIG      (0x3B)
        #define WRITE_ESLOTG23      (0x3D)
        #define WRITE_ESLOTG45      (0x3F)
        #define WRITE_ESLOTG67      (0x41)
        #define WRITE_TXPEND        (0x43)
        #define WRITE_TXBCNINTL     (0x45)
        #define WRITE_FRMOFFSET     (0x47)
        #define WRITE_TXSR          (0x49)
        #define WRITE_TXLERR        (0x4B)
        #define WRITE_GATE_CLK      (0x4D)
        #define WRITE_TXOFFSET      (0x4F)
        #define WRITE_HSYMTMR0      (0x51)
        #define WRITE_HSYMTMR1      (0x53)
        #define WRITE_SOFTRST       (0x55)
        #define WRITE_BISTCR        (0x57)
        #define WRITE_SECCR0        (0x59)
        #define WRITE_SECCR1        (0x5B)
        #define WRITE_TXPEMISP      (0x5D)
        #define WRITE_SECISR        (0x5F)
        #define WRITE_RXSR          (0x61)
        #define WRITE_ISRSTS        (0x63)
        #define WRITE_INTMSK        (0x65)
        #define WRITE_GPIO          (0x67)
        #define WRITE_GPIODIR       (0x69)
        #define WRITE_SLPACK        (0x6B)
        #define WRITE_RFCTL         (0x6D)
        #define WRITE_SECCR2        (0x6F)
        #define WRITE_BBREG0        (0x71)
        #define WRITE_BBREG1        (0x73)
        #define WRITE_BBREG2        (0x75)
        #define WRITE_BBREG3        (0x77)
        #define WRITE_BBREG4        (0x79)
        #define WRITE_BBREG5        (0x7B)
        #define WRITE_BBREG6        (0x7D)
        #define WRITE_RSSITHCCA     (0x7F)

        #define CHANNEL_11 0x00
        #define CHANNEL_12 0x10
        #define CHANNEL_13 0x20
        #define CHANNEL_14 0x30
        #define CHANNEL_15 0x40
        #define CHANNEL_16 0x50
        #define CHANNEL_17 0x60
        #define CHANNEL_18 0x70
        #define CHANNEL_19 0x80
        #define CHANNEL_20 0x90
        #define CHANNEL_21 0xa0
        #define CHANNEL_22 0xb0
        #define CHANNEL_23 0xc0
        #define CHANNEL_24 0xd0
        #define CHANNEL_25 0xe0
        #define CHANNEL_26 0xf0

        #if defined(ENABLE_PA_LNA) && (defined(MRF24J40MB) || defined(MRF24J40MC))
            #define MRF24J40_FULL_CHANNEL_MAP        0x03FFF800
        #else
            #define MRF24J40_FULL_CHANNEL_MAP        0x07FFF800
        #endif

        // MRF24J40 TYPE DEFINITIONS AND VARIABLES ---------------------------//
        typedef union {
            BYTE Val;               // value of interrupts
            struct {
                BYTE RF_TXIF :1;    // transmission finish interrupt
                BYTE :2;
                BYTE RF_RXIF :1;    // receiving a packet interrupt
                BYTE SECIF :1;      // receiving a secured packet interrupt
                BYTE :4;
            }bits;                  // bit map of interrupts
        } MRF24J40_IFREG;

        typedef union {
            BYTE Val;
            struct {
                BYTE        TX_BUSY             : 1;
                BYTE        TX_PENDING_ACK      : 1;
                BYTE        TX_FAIL             : 1;
                BYTE        RX_SECURITY         : 1;
                BYTE        RX_IGNORE_SECURITY  : 1;
                BYTE        RX_BUFFERED         : 1;
            }bits;
        } MRF24J40_STATUS;
    #endif
#endif

//----------------------------------------------------------------------------//
//-------------------------- MRF49XA_1 AND MRF49XA_2 -------------------------//
//----------------------------------------------------------------------------//
#if defined(MRF49XA_1) || defined (MRF49XA_2)
    #define SOFTWARE_CRC

    //The following definitions are common to both MRFXA_1 and MRF49XA_2. The
    //definitions are used during all MRF49XA transceivers configuration.

    #define PMCREG                  0x8201
    #define FIFORSTREG              0xCA81
    #define AFCCREG                 0xC4B7
    #define BBFCREG                 0xC2AC

    #if defined(ENABLE_SECURITY)
        #define MRF49XA_TX_PCKT_SIZE (MRF49XA_TX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                BLOCK_SIZE+MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+10)
        #define MRF49XA_RX_PCKT_SIZE (MRF49XA_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+BLOCK_SIZE+10)
    #else
        #define MRF49XA_TX_PCKT_SIZE (MRF49XA_TX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+5)
        #define MRF49XA_RX_PCKT_SIZE (MRF49XA_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+5)
    #endif

    #if MRF49XA_RX_PCKT_SIZE > 126
        #warning "MRF49XA_RX_PCKT_SIZE may be defined too big"
        #undef MRF49XA_RX_PCKT_SIZE
        #define MRF49XA_RX_PCKT_SIZE 126
    #endif

    #if MRF49XA_TX_PCKT_SIZE > 126
        #warning "MRF49XA_TX_PCKT_SIZE may be defined too big"
        #undef MRF49XA_TX_PCKT_SIZE
        #define MRF49XA_TX_PCKT_SIZE 126
    #endif

    typedef struct{
        union{  
            BYTE Val;
            struct{
                BYTE Valid    :1;
                BYTE RSSI     :1;   //"RSSI": Conflict with a MRF24J40 SFR define
                BYTE DQD      :1;
            }bits;
        }flags;
        BYTE Payload [MRF49XA_RX_PCKT_SIZE];
        BYTE PayloadLen;
    }MRF49XA_RX_PACKET;

    typedef struct {
        BOOL        Valid;
        BYTE        Seq;
        WORD        CRC;
        MIWI_TICK   startTick;
    }MRF49XA_ACK_INFO;

    typedef union {
        WORD    Val;
        BYTE    v[2];
        struct {
            BYTE    RSSI_ATS    :1;
            BYTE    FFEM        :1;
            BYTE    LBD         :1;
            BYTE    EXT         :1;
            BYTE    WKUP        :1;
            BYTE    RGUR_FFOV   :1;
            BYTE    POR         :1;
            BYTE    RG_FF_IT    :1;
            BYTE    filler      :5;
            BYTE    ATGL        :1;
            BYTE    CRL         :1;
            BYTE    DQD         :1;
        } bits;
    }MRF49XA_STATUS;

#endif

//----------------------------------------------------------------------------//
//--------------------------------- MRF49XA_1 --------------------------------//
//----------------------------------------------------------------------------//

#if defined MRF49XA_1
    
    #if defined MRF49XA_1_IN_434
        #define MRF49XA_1_FREQ_BAND         0x0010    // 434MHz
        #if defined(MRF49XA_1_RATE_1200)
            #define MRF49XA_1_DRVSREG       0xC6A3    // 1200bps
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START          1332
            #define MRF49XA_1_FREQ_STEP           132
            #define MRF49XA_1_CHANNEL_NUM         5
            #define MRF49XA_1_FULL_CHANNEL_MAP    0x0000001F
        #elif defined(MRF49XA_1_RATE_9600)
            #define MRF49XA_1_DRVSREG       0xC623              // 9600bps
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              1332
            #define MRF49XA_1_FREQ_STEP               132
            #define MRF49XA_1_CHANNEL_NUM             5
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x0000001F
        #elif defined(MRF49XA_1_RATE_19200)
            #define MRF49XA_1_DRVSREG       0xC611              // 19200bps
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              1412
            #define MRF49XA_1_FREQ_STEP               154
            #define MRF49XA_1_CHANNEL_NUM             4
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x0000000F
        #elif defined(MRF49XA_1_RATE_38400)
            #define MRF49XA_1_DRVSREG       0xC608              // 38400
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)48 + 2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              1388
            #define MRF49XA_1_FREQ_STEP               180
            #define MRF49XA_1_CHANNEL_NUM             4
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x0000000F
        #elif defined(MRF49XA_1_RATE_57600)
            #define MRF49XA_1_DRVSREG       0xC605
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)67+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              1444
            #define MRF49XA_1_FREQ_STEP               256
            #define MRF49XA_1_CHANNEL_NUM             3
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x00000007
        #elif defined(MRF49XA_1_RATE_115200)
            #define MRF49XA_1_DRVSREG       0xC602
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              1564
            #define MRF49XA_1_FREQ_STEP               345
            #define MRF49XA_1_CHANNEL_NUM             2
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x00000003
        #else
            #error "No valid data rate defined"
        #endif
    #elif defined(MRF49XA_1_IN_868)
        #define MRF49XA_1_FREQ_BAND               0x0020              // 868MHz
        #if defined(MRF49XA_1_RATE_1200)
            #define MRF49XA_1_DRVSREG       0xC6A3
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              710
            #define MRF49XA_1_FREQ_STEP               90
            #define MRF49XA_1_CHANNEL_NUM             15
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x00007FFF
        #elif defined(MRF49XA_1_RATE_9600)
            #define MRF49XA_1_DRVSREG       0xC623
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              710
            #define MRF49XA_1_FREQ_STEP               90
            #define MRF49XA_1_CHANNEL_NUM             15
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x00007FFF
        #elif defined(MRF49XA_1_RATE_19200)
            #define MRF49XA_1_DRVSREG       0xC611
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              740
            #define MRF49XA_1_FREQ_STEP               110
            #define MRF49XA_1_CHANNEL_NUM             12
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x00000FFF
        #elif defined(MRF49XA_1_RATE_38400)
            #define MRF49XA_1_DRVSREG       0xC608
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)48+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              724
            #define MRF49XA_1_FREQ_STEP               128
            #define MRF49XA_1_CHANNEL_NUM             11
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x000007FF
        #elif defined(MRF49XA_1_RATE_57600)
            #define MRF49XA_1_DRVSREG       0xC605
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)67+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              750
            #define MRF49XA_1_FREQ_STEP               157
            #define MRF49XA_1_CHANNEL_NUM             9
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x000001FF
        #elif defined(MRF49XA_1_RATE_115200)
            #define MRF49XA_1_DRVSREG       0xC602
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              800
            #define MRF49XA_1_FREQ_STEP               200
            #define MRF49XA_1_CHANNEL_NUM             7
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x0000007F
        #else
            #error "No valid data rate defined"
        #endif
    #elif defined(MRF49XA_1_IN_915)
        #define MRF49XA_1_FREQ_BAND               0x0030              // 915MHz
        #if defined(MRF49XA_1_RATE_1200)
            #define MRF49XA_1_DRVSREG       0xC6A3
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              320
            #define MRF49XA_1_FREQ_STEP               105
            #define MRF49XA_1_CHANNEL_NUM             32
            #define MRF49XA_1_FULL_CHANNEL_MAP        0xFFFFFFFF
       #elif defined(MRF49XA_1_RATE__9600)
            #define MRF49XA_1_DRVSREG       0xC623
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              320
            #define MRF49XA_1_FREQ_STEP               105
            #define MRF49XA_1_CHANNEL_NUM             32
            #define MRF49XA_1_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_1_RATE__19200)
            #define MRF49XA_1_DRVSREG       0xC611
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              320
            #define MRF49XA_1_FREQ_STEP               105
            #define MRF49XA_1_CHANNEL_NUM             32
            #define MRF49XA_1_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_1_RATE__38400)
            #define MRF49XA_1_DRVSREG       0xC608
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)48+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              320
            #define MRF49XA_1_FREQ_STEP               105
            #define MRF49XA_1_CHANNEL_NUM             32
            #define MRF49XA_1_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_1_RATE__57600)
            #define MRF49XA_1_DRVSREG       0xC605
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)68+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              320
            #define MRF49XA_1_FREQ_STEP               105
            #define MRF49XA_1_CHANNEL_NUM             32
            #define MRF49XA_1_FULL_CHANNEL_MAP        0xFFFFFFFF
       #elif defined(MRF49XA_1_RATE_115200)
            #define MRF49XA_1_DRVSREG       0xC602
            #define MRF49XA_1_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_1_RF_DEV        (((MRF49XA_1_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)) : \
                            (MRF49XA_1_RAW_RF_DEV-(MRF49XA_1_RAW_RF_DEV % 15)+15))
            #define MRF49XA_1_TXCREG    (0x9800 | (((WORD)MRF49XA_1_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_1_FREQ_START              333
            #define MRF49XA_1_FREQ_STEP               134
            #define MRF49XA_1_CHANNEL_NUM             26
            #define MRF49XA_1_FULL_CHANNEL_MAP        0x03FFFFFF
       #else
            #error "No valid data rate defined"
       #endif
    #else
        #error "No valid frequency band defined"
    #endif

    #define MRF49XA_1_RXCREG   (0x9400 | MRF49XA_LNA_GAIN | MRF49XA_RSSI_THRESHOLD)
    #define MRF49XA_1_GENCREG  (0x8000 | MRF49XA_1_FREQ_BAND | XTAL_LD_CAP)
    
    extern volatile MRF49XA_STATUS   MRF49XA_1_Status;
#endif

//----------------------------------------------------------------------------//
//--------------------------------- MRF49XA_2 --------------------------------//
//----------------------------------------------------------------------------//
#if defined(MRF49XA_2)
    #if defined(MRF49XA_2_IN_434)
        #define MRF49XA_2_FREQ_BAND         0x0010    // 434MHz
        #if defined(MRF49XA_2_RATE_1200)
            #define MRF49XA_2_DRVSREG       0xC6A3    // 1200bps
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START          1332
            #define MRF49XA_2_FREQ_STEP           132
            #define MRF49XA_2_CHANNEL_NUM         5
            #define MRF49XA_2_FULL_CHANNEL_MAP    0x0000001F
        #elif defined(MRF49XA_2_RATE_9600)
            #define MRF49XA_2_DRVSREG       0xC623              // 9600bps
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              1332
            #define MRF49XA_2_FREQ_STEP               132
            #define MRF49XA_2_CHANNEL_NUM             5
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x0000001F
       #elif defined(MRF49XA_2_RATE_19200)
            #define MRF49XA_2_DRVSREG       0xC611              // 19200bps
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              1412
            #define MRF49XA_2_FREQ_STEP               154
            #define MRF49XA_2_CHANNEL_NUM             4
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x0000000F
       #elif defined(MRF49XA_2_RATE_38400)
            #define MRF49XA_2_DRVSREG       0xC608              // 38400
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)48+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              1388
            #define MRF49XA_2_FREQ_STEP               180
            #define MRF49XA_2_CHANNEL_NUM             4
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x0000000F
       #elif defined(MRF49XA_2_RATE_57600)
            #define MRF49XA_2_DRVSREG       0xC605
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)67+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              1444
            #define MRF49XA_2_FREQ_STEP               256
            #define MRF49XA_2_CHANNEL_NUM             3
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x00000007
       #elif defined(MRF49XA_2_RATE_115200)
            #define MRF49XA_2_DRVSREG       0xC602
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*434/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              1564
            #define MRF49XA_2_FREQ_STEP               345
            #define MRF49XA_2_CHANNEL_NUM             2
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x00000003
       #else
            #error "No valid data rate defined"
       #endif
    #elif defined(MRF49XA_2_IN_868)
        #define MRF49XA_2_FREQ_BAND               0x0020              // 868MHz
        #if defined(MRF49XA_2_RATE_1200)
            #define MRF49XA_2_DRVSREG       0xC6A3
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              710
            #define MRF49XA_2_FREQ_STEP               90
            #define MRF49XA_2_CHANNEL_NUM             15
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x00007FFF
        #elif defined(MRF49XA_2_RATE_9600)
            #define MRF49XA_2_DRVSREG       0xC623
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              710
            #define MRF49XA_2_FREQ_STEP               90
            #define MRF49XA_2_CHANNEL_NUM             15
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x00007FFF
        #elif defined(MRF49XA_2_RATE_19200)
            #define MRF49XA_2_DRVSREG       0xC611
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              740
            #define MRF49XA_2_FREQ_STEP               110
            #define MRF49XA_2_CHANNEL_NUM             12
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x00000FFF
        #elif defined(MRF49XA_2_RATE_38400)
            #define MRF49XA_2_DRVSREG       0xC608
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)48+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              724
            #define MRF49XA_2_FREQ_STEP               128
            #define MRF49XA_2_CHANNEL_NUM             11
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x000007FF
        #elif defined(MRF49XA_2_RATE_57600)
            #define MRF49XA_2_DRVSREG       0xC605
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)67+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              750
            #define MRF49XA_2_FREQ_STEP               157
            #define MRF49XA_2_CHANNEL_NUM             9
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x000001FF
        #elif defined(MRF49XA_2_RATE_115200)
            #define MRF49XA_2_DRVSREG       0xC602
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*868/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              800
            #define MRF49XA_2_FREQ_STEP               200
            #define MRF49XA_2_CHANNEL_NUM             7
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x0000007F
        #else
            #error "No valid data rate defined"
        #endif
    #elif defined(MRF49XA_2_IN_915)
        #define MRF49XA_2_FREQ_BAND               0x0030              // 915MHz
        #if defined(MRF49XA_2_RATE_1200)
            #define MRF49XA_2_DRVSREG       0xC6A3
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)11+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              320
            #define MRF49XA_2_FREQ_STEP               105
            #define MRF49XA_2_CHANNEL_NUM             32
            #define MRF49XA_2_FULL_CHANNEL_MAP        0xFFFFFFFF
       #elif defined(MRF49XA_2_RATE__9600)
            #define MRF49XA_2_DRVSREG       0xC623
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)19+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              320
            #define MRF49XA_2_FREQ_STEP               105
            #define MRF49XA_2_CHANNEL_NUM             32
            #define MRF49XA_2_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_2_RATE__19200)
            #define MRF49XA_2_DRVSREG       0xC611
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)29+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              320
            #define MRF49XA_2_FREQ_STEP               105
            #define MRF49XA_2_CHANNEL_NUM             32
            #define MRF49XA_2_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_2_RATE__38400)
            #define MRF49XA_2_DRVSREG       0xC608
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)48+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              320
            #define MRF49XA_2_FREQ_STEP               105
            #define MRF49XA_2_CHANNEL_NUM             32
            #define MRF49XA_2_FULL_CHANNEL_MAP        0xFFFFFFFF
        #elif defined(MRF49XA_2_RATE__57600)
            #define MRF49XA_2_DRVSREG       0xC605
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)68+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              320
            #define MRF49XA_2_FREQ_STEP               105
            #define MRF49XA_2_CHANNEL_NUM             32
            #define MRF49XA_2_FULL_CHANNEL_MAP        0xFFFFFFFF
       #elif defined(MRF49XA_2_RATE_115200)
            #define MRF49XA_2_DRVSREG       0xC602
            #define MRF49XA_2_RAW_RF_DEV    ((WORD)125+2*((WORD)CRYSTAL_PPM*915/1000))
            #define MRF49XA_2_RF_DEV        (((MRF49XA_2_RAW_RF_DEV % 15) < 8) ? \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)) : \
                            (MRF49XA_2_RAW_RF_DEV-(MRF49XA_2_RAW_RF_DEV % 15)+15))
            #define MRF49XA_2_TXCREG    (0x9800 | (((WORD)MRF49XA_2_RF_DEV/15 - 1)<<4) | MRF49XA_TX_POWER)
            #define MRF49XA_2_FREQ_START              333
            #define MRF49XA_2_FREQ_STEP               134
            #define MRF49XA_2_CHANNEL_NUM             26
            #define MRF49XA_2_FULL_CHANNEL_MAP        0x03FFFFFF
       #else
            #error "No valid data rate defined"
       #endif
    #else
        #error "No valid frequency band defined"
    #endif

    #define MRF49XA_2_RXCREG    (0x9400 | MRF49XA_LNA_GAIN | MRF49XA_RSSI_THRESHOLD)
    #define MRF49XA_2_GENCREG   (0x8000 | MRF49XA_2_FREQ_BAND | XTAL_LD_CAP)

    extern volatile MRF49XA_STATUS MRF49XA_2_Status;
#endif

//----------------------------------------------------------------------------//
//-------------------------------- MRF89XA -----------------------------------//
//----------------------------------------------------------------------------//
//MICROCHIP LSI CWSN STACK NOT ADAPTED YET!!
#if defined(MRF89XA)

    //#define APPLICATION_CIRCUIT_950
    //Enable if Application circuit is built for supporting 950-960 MHz instead
    //of 860-870 but the band selection must still indicate 863 (BAND_863)

    // MRF89XA Operating modes -----------------------------------------------//
    #define RF_SLEEP        0x00
    #define RF_STANDBY      0x20
    #define RF_SYNTHESIZER  0x40
    #define RF_RECEIVER     0x60
    #define RF_TRANSMITTER  0x80

    #define CHIPMODE_SLEEPMODE 0x00	//000 [7:5]
    #define CHIPMODE_STBYMODE  0x20	//001 ;default
    #define CHIPMODE_FSMODE    0x40	//010
    #define CHIPMODE_RX        0x60	//011
    #define CHIPMODE_TX        0x80	//100

    #define BITRATE_200     0x00
    #define BITRATE_100     0x01
    #define BITRATE_66      0x02
    #define BITRATE_50      0x03
    #define BITRATE_40      0x04
    #define BITRATE_25      0x07
    #define BITRATE_20      0x09
    #define BITRATE_10      0x13
    #define BITRATE_5       0x27
    #define BITRATE_2       0x63

    #define FREQBAND_902    0x00    //902-915 00 [4:3]
    #define FREQBAND_915    0x08    //915-928 01 ;default
    #define FREQBAND_863    0x10    //950-960 or 863-870 10

    #define BW_25  0x00
    #define BW_50  0x01
    #define BW_75  0x02
    #define BW_100 0x03
    #define BW_125 0x04
    #define BW_150 0x05
    #define BW_175 0x06
    #define BW_200 0x07
    #define BW_225 0x08
    #define BW_250 0x09
    #define BW_275 0x0A
    #define BW_300 0x0B
    #define BW_325 0x0C
    #define BW_350 0x0D
    #define BW_375 0x0E
    #define BW_400 0x0F

    #define FREQ_DEV_33  0x0B
    #define FREQ_DEV_40  0x09
    #define FREQ_DEV_50  0x07
    #define FREQ_DEV_67  0x05
    #define FREQ_DEV_80  0x04
    #define FREQ_DEV_100 0x03
    #define FREQ_DEV_133 0x02
    #define FREQ_DEV_200 0x01

    #define VCO_TRIM_00 0x00  //[2:1] Vtune determined by tank inductor values
    #define VCO_TRIM_01 0x02
    #define VCO_TRIM_10 0x04
    #define VCO_TRIM_11 0x06

    #define FILCON_SET_65   0x00    //65 KHz
    #define FILCON_SET_82   0x10    //82 KHz
    #define FILCON_SET_109  0x20
    #define FILCON_SET_137  0x30
    #define FILCON_SET_157  0x40
    #define FILCON_SET_184  0x50
    #define FILCON_SET_211  0x60
    #define FILCON_SET_234  0x70
    #define FILCON_SET_262  0x80
    #define FILCON_SET_321  0x90
    #define FILCON_SET_378  0xA0
    #define FILCON_SET_414  0xB0
    #define FILCON_SET_458  0xC0
    #define FILCON_SET_514  0xD0
    #define FILCON_SET_676  0xE0
    #define FILCON_SET_987  0xF0

    #if defined(BAND_863)
        #define FREQ_BAND   FREQBAND_863    // 863-870 MHz or 950-960 MHz
        #if defined(APPLICATION_CIRCUIT_950)
            #define START_FREQUENCY 950000
        #else
            #define START_FREQUENCY 860000  //in KHz
        #endif
        #define R1CNT   125
        #define P1CNT	100
        #define S1CNT	20              //center frequency - 868MHz
    #elif defined(BAND_902)
        #define FREQ_BAND FREQBAND_902      // 902MHz
        #define	START_FREQUENCY 902000
        #define	R1CNT   119
        #define	P1CNT   99
        #define	S1CNT   25              //center freq - 903MHz
    #elif defined(BAND_915)
        #define FREQ_BAND   FREQBAND_915    // 915MHz
        #define	START_FREQUENCY 915000
        #define	R1CNT   119
        #define	P1CNT	100
        #define	S1CNT	50              //center frequency - 916MHz
    #else
        #error "No valid frequency band defined"
    #endif

    //Define the R, P, S sets for different channels
    #define	RVALUE  100

    #if defined(BAND_863)
        #if(!defined(DATA_RATE_100) && !defined(DATA_RATE_200))
            #define CHANNEL_SPACING  300 //Channel spacing 300KHz (except for 100 and 200kbps)
            #define CHANNEL_NUM      32
            #define FULL_CHANNEL_MAP 0xFFFFFFFF
        #else
            #define CHANNEL_SPACING  384  //To support 25 channels instead of 32
            #define CHANNEL_NUM      25
            #define FULL_CHANNEL_MAP 0x01FFFFFF
        #endif
    #else
        #define CHANNEL_SPACING     400         //Channel spacing 400KHz
        #define CHANNEL_NUM         32
        #define FULL_CHANNEL_MAP    0xFFFFFFFF
    #endif

    #define	FXTAL   12.8    	//Frequency of the crystal in MHz

    #define CHANNEL1_FREQ ((DWORD)(START_FREQUENCY)+ CHANNEL_SPACING)
    #define CHANNEL2_FREQ ((DWORD)(START_FREQUENCY)+ (2*CHANNEL_SPACING))
    #define CHANNEL3_FREQ ((DWORD)(START_FREQUENCY)+ (3*CHANNEL_SPACING))
    #define CHANNEL4_FREQ ((DWORD)(START_FREQUENCY)+ (4*CHANNEL_SPACING))
    #define CHANNEL5_FREQ ((DWORD)(START_FREQUENCY)+ (5*CHANNEL_SPACING))
    #define CHANNEL6_FREQ ((DWORD)(START_FREQUENCY)+ (6*CHANNEL_SPACING))
    #define CHANNEL7_FREQ ((DWORD)(START_FREQUENCY)+ (7*CHANNEL_SPACING))
    #define CHANNEL8_FREQ ((DWORD)(START_FREQUENCY)+ (8*CHANNEL_SPACING))
    #define CHANNEL9_FREQ ((DWORD)(START_FREQUENCY)+ (9*CHANNEL_SPACING))
    #define CHANNEL10_FREQ ((DWORD)(START_FREQUENCY)+ (10*CHANNEL_SPACING))
    #define CHANNEL11_FREQ ((DWORD)(START_FREQUENCY)+ (11*CHANNEL_SPACING))
    #define CHANNEL12_FREQ ((DWORD)(START_FREQUENCY)+ (12*CHANNEL_SPACING))
    #define CHANNEL13_FREQ ((DWORD)(START_FREQUENCY)+ (13*CHANNEL_SPACING))
    #define CHANNEL14_FREQ ((DWORD)(START_FREQUENCY)+ (14*CHANNEL_SPACING))
    #define CHANNEL15_FREQ ((DWORD)(START_FREQUENCY)+ (15*CHANNEL_SPACING))
    #define CHANNEL16_FREQ ((DWORD)(START_FREQUENCY)+ (16*CHANNEL_SPACING))
    #define CHANNEL17_FREQ ((DWORD)(START_FREQUENCY)+ (17*CHANNEL_SPACING))
    #define CHANNEL18_FREQ ((DWORD)(START_FREQUENCY)+ (18*CHANNEL_SPACING))
    #define CHANNEL19_FREQ ((DWORD)(START_FREQUENCY)+ (19*CHANNEL_SPACING))
    #define CHANNEL20_FREQ ((DWORD)(START_FREQUENCY)+ (20*CHANNEL_SPACING))
    #define CHANNEL21_FREQ ((DWORD)(START_FREQUENCY)+ (21*CHANNEL_SPACING))
    #define CHANNEL22_FREQ ((DWORD)(START_FREQUENCY)+ (22*CHANNEL_SPACING))
    #define CHANNEL23_FREQ ((DWORD)(START_FREQUENCY)+ (23*CHANNEL_SPACING))
    #define CHANNEL24_FREQ ((DWORD)(START_FREQUENCY)+ (24*CHANNEL_SPACING))
    #define CHANNEL25_FREQ ((DWORD)(START_FREQUENCY)+ (25*CHANNEL_SPACING))
    #define CHANNEL26_FREQ ((DWORD)(START_FREQUENCY)+ (26*CHANNEL_SPACING))
    #define CHANNEL27_FREQ ((DWORD)(START_FREQUENCY)+ (27*CHANNEL_SPACING))
    #define CHANNEL28_FREQ ((DWORD)(START_FREQUENCY)+ (28*CHANNEL_SPACING))
    #define CHANNEL29_FREQ ((DWORD)(START_FREQUENCY)+ (29*CHANNEL_SPACING))
    #define CHANNEL30_FREQ ((DWORD)(START_FREQUENCY)+ (30*CHANNEL_SPACING))
    #define CHANNEL31_FREQ ((DWORD)(START_FREQUENCY)+ (31*CHANNEL_SPACING))
    #define CHANNEL32_FREQ ((DWORD)(START_FREQUENCY)+ (32*CHANNEL_SPACING))

    #define CHANNEL1_COMPARE    (WORD)((DWORD)(CHANNEL1_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL2_COMPARE    (WORD)((DWORD)(CHANNEL2_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL3_COMPARE    (WORD)((DWORD)(CHANNEL3_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL4_COMPARE    (WORD)((DWORD)(CHANNEL4_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL5_COMPARE    (WORD)((DWORD)(CHANNEL5_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL6_COMPARE    (WORD)((DWORD)(CHANNEL6_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL7_COMPARE    (WORD)((DWORD)(CHANNEL7_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL8_COMPARE    (WORD)((DWORD)(CHANNEL8_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL9_COMPARE    (WORD)((DWORD)(CHANNEL9_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL10_COMPARE   (WORD)((DWORD)(CHANNEL10_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL11_COMPARE   (WORD)((DWORD)(CHANNEL11_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL12_COMPARE   (WORD)((DWORD)(CHANNEL12_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL13_COMPARE   (WORD)((DWORD)(CHANNEL13_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL14_COMPARE   (WORD)((DWORD)(CHANNEL14_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL15_COMPARE   (WORD)((DWORD)(CHANNEL15_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL16_COMPARE   (WORD)((DWORD)(CHANNEL16_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL17_COMPARE   (WORD)((DWORD)(CHANNEL17_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL18_COMPARE   (WORD)((DWORD)(CHANNEL18_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL19_COMPARE   (WORD)((DWORD)(CHANNEL19_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL20_COMPARE   (WORD)((DWORD)(CHANNEL20_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL21_COMPARE   (WORD)((DWORD)(CHANNEL21_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL22_COMPARE   (WORD)((DWORD)(CHANNEL22_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL23_COMPARE   (WORD)((DWORD)(CHANNEL23_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL24_COMPARE   (WORD)((DWORD)(CHANNEL24_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL25_COMPARE   (WORD)((DWORD)(CHANNEL25_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL26_COMPARE   (WORD)((DWORD)(CHANNEL26_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL27_COMPARE   (WORD)((DWORD)(CHANNEL27_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL28_COMPARE   (WORD)((DWORD)(CHANNEL28_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL29_COMPARE   (WORD)((DWORD)(CHANNEL29_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL30_COMPARE   (WORD)((DWORD)(CHANNEL30_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL31_COMPARE   (WORD)((DWORD)(CHANNEL31_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))
    #define CHANNEL32_COMPARE   (WORD)((DWORD)(CHANNEL32_FREQ * 8 * 101)/(DWORD)(9*FXTAL*1000))

    #define CHANNEL1_PVALUE     (BYTE)(((WORD)(CHANNEL1_COMPARE - 75)/76)+1)
    #define CHANNEL2_PVALUE     (BYTE)(((WORD)(CHANNEL2_COMPARE - 75)/76)+1)
    #define CHANNEL3_PVALUE     (BYTE)(((WORD)(CHANNEL3_COMPARE - 75)/76)+1)
    #define CHANNEL4_PVALUE     (BYTE)(((WORD)(CHANNEL4_COMPARE - 75)/76)+1)
    #define CHANNEL5_PVALUE     (BYTE)(((WORD)(CHANNEL5_COMPARE - 75)/76)+1)
    #define CHANNEL6_PVALUE     (BYTE)(((WORD)(CHANNEL6_COMPARE - 75)/76)+1)
    #define CHANNEL7_PVALUE     (BYTE)(((WORD)(CHANNEL7_COMPARE - 75)/76)+1)
    #define CHANNEL8_PVALUE     (BYTE)(((WORD)(CHANNEL8_COMPARE - 75)/76)+1)
    #define CHANNEL9_PVALUE     (BYTE)(((WORD)(CHANNEL9_COMPARE - 75)/76)+1)
    #define CHANNEL10_PVALUE    (BYTE)(((WORD)(CHANNEL10_COMPARE - 75)/76)+1)
    #define CHANNEL11_PVALUE    (BYTE)(((WORD)(CHANNEL11_COMPARE - 75)/76)+1)
    #define CHANNEL12_PVALUE    (BYTE)(((WORD)(CHANNEL12_COMPARE - 75)/76)+1)
    #define CHANNEL13_PVALUE    (BYTE)(((WORD)(CHANNEL13_COMPARE - 75)/76)+1)
    #define CHANNEL14_PVALUE    (BYTE)(((WORD)(CHANNEL14_COMPARE - 75)/76)+1)
    #define CHANNEL15_PVALUE    (BYTE)(((WORD)(CHANNEL15_COMPARE - 75)/76)+1)
    #define CHANNEL16_PVALUE    (BYTE)(((WORD)(CHANNEL16_COMPARE - 75)/76)+1)
    #define CHANNEL17_PVALUE    (BYTE)(((WORD)(CHANNEL17_COMPARE - 75)/76)+1)
    #define CHANNEL18_PVALUE    (BYTE)(((WORD)(CHANNEL18_COMPARE - 75)/76)+1)
    #define CHANNEL19_PVALUE    (BYTE)(((WORD)(CHANNEL19_COMPARE - 75)/76)+1)
    #define CHANNEL20_PVALUE    (BYTE)(((WORD)(CHANNEL20_COMPARE - 75)/76)+1)
    #define CHANNEL21_PVALUE    (BYTE)(((WORD)(CHANNEL21_COMPARE - 75)/76)+1)
    #define CHANNEL22_PVALUE    (BYTE)(((WORD)(CHANNEL22_COMPARE - 75)/76)+1)
    #define CHANNEL23_PVALUE    (BYTE)(((WORD)(CHANNEL23_COMPARE - 75)/76)+1)
    #define CHANNEL24_PVALUE    (BYTE)(((WORD)(CHANNEL24_COMPARE - 75)/76)+1)
    #define CHANNEL25_PVALUE    (BYTE)(((WORD)(CHANNEL25_COMPARE - 75)/76)+1)
    #define CHANNEL26_PVALUE    (BYTE)(((WORD)(CHANNEL26_COMPARE - 75)/76)+1)
    #define CHANNEL27_PVALUE    (BYTE)(((WORD)(CHANNEL27_COMPARE - 75)/76)+1)
    #define CHANNEL28_PVALUE    (BYTE)(((WORD)(CHANNEL28_COMPARE - 75)/76)+1)
    #define CHANNEL29_PVALUE    (BYTE)(((WORD)(CHANNEL29_COMPARE - 75)/76)+1)
    #define CHANNEL30_PVALUE    (BYTE)(((WORD)(CHANNEL30_COMPARE - 75)/76)+1)
    #define CHANNEL31_PVALUE    (BYTE)(((WORD)(CHANNEL31_COMPARE - 75)/76)+1)
    #define CHANNEL32_PVALUE    (BYTE)(((WORD)(CHANNEL32_COMPARE - 75)/76)+1)

    #define CHANNEL1_SVALUE     (BYTE)(((WORD)CHANNEL1_COMPARE - ((WORD)(75*(CHANNEL1_PVALUE+1)))))
    #define CHANNEL2_SVALUE     (BYTE)(((WORD)CHANNEL2_COMPARE - ((WORD)(75*(CHANNEL2_PVALUE+1)))))
    #define CHANNEL3_SVALUE     (BYTE)(((WORD)CHANNEL3_COMPARE - ((WORD)(75*(CHANNEL3_PVALUE+1)))))
    #define CHANNEL4_SVALUE     (BYTE)(((WORD)CHANNEL4_COMPARE - ((WORD)(75*(CHANNEL4_PVALUE+1)))))
    #define CHANNEL5_SVALUE     (BYTE)(((WORD)CHANNEL5_COMPARE - ((WORD)(75*(CHANNEL5_PVALUE+1)))))
    #define CHANNEL6_SVALUE     (BYTE)(((WORD)CHANNEL6_COMPARE - ((WORD)(75*(CHANNEL6_PVALUE+1)))))
    #define CHANNEL7_SVALUE     (BYTE)(((WORD)CHANNEL7_COMPARE - ((WORD)(75*(CHANNEL7_PVALUE+1)))))
    #define CHANNEL8_SVALUE     (BYTE)(((WORD)CHANNEL8_COMPARE - ((WORD)(75*(CHANNEL8_PVALUE+1)))))
    #define CHANNEL9_SVALUE     (BYTE)(((WORD)CHANNEL9_COMPARE - ((WORD)(75*(CHANNEL9_PVALUE+1)))))
    #define CHANNEL10_SVALUE    (BYTE)(((WORD)CHANNEL10_COMPARE - ((WORD)(75*(CHANNEL10_PVALUE+1)))))
    #define CHANNEL11_SVALUE    (BYTE)(((WORD)CHANNEL11_COMPARE - ((WORD)(75*(CHANNEL11_PVALUE+1)))))
    #define CHANNEL12_SVALUE    (BYTE)(((WORD)CHANNEL12_COMPARE - ((WORD)(75*(CHANNEL12_PVALUE+1)))))
    #define CHANNEL13_SVALUE    (BYTE)(((WORD)CHANNEL13_COMPARE - ((WORD)(75*(CHANNEL13_PVALUE+1)))))
    #define CHANNEL14_SVALUE    (BYTE)(((WORD)CHANNEL14_COMPARE - ((WORD)(75*(CHANNEL14_PVALUE+1)))))
    #define CHANNEL15_SVALUE    (BYTE)(((WORD)CHANNEL15_COMPARE - ((WORD)(75*(CHANNEL15_PVALUE+1)))))
    #define CHANNEL16_SVALUE    (BYTE)(((WORD)CHANNEL16_COMPARE - ((WORD)(75*(CHANNEL16_PVALUE+1)))))
    #define CHANNEL17_SVALUE    (BYTE)(((WORD)CHANNEL17_COMPARE - ((WORD)(75*(CHANNEL17_PVALUE+1)))))
    #define CHANNEL18_SVALUE    (BYTE)(((WORD)CHANNEL18_COMPARE - ((WORD)(75*(CHANNEL18_PVALUE+1)))))
    #define CHANNEL19_SVALUE    (BYTE)(((WORD)CHANNEL19_COMPARE - ((WORD)(75*(CHANNEL19_PVALUE+1)))))
    #define CHANNEL20_SVALUE    (BYTE)(((WORD)CHANNEL20_COMPARE - ((WORD)(75*(CHANNEL20_PVALUE+1)))))
    #define CHANNEL21_SVALUE    (BYTE)(((WORD)CHANNEL21_COMPARE - ((WORD)(75*(CHANNEL21_PVALUE+1)))))
    #define CHANNEL22_SVALUE    (BYTE)(((WORD)CHANNEL22_COMPARE - ((WORD)(75*(CHANNEL22_PVALUE+1)))))
    #define CHANNEL23_SVALUE    (BYTE)(((WORD)CHANNEL23_COMPARE - ((WORD)(75*(CHANNEL23_PVALUE+1)))))
    #define CHANNEL24_SVALUE    (BYTE)(((WORD)CHANNEL24_COMPARE - ((WORD)(75*(CHANNEL24_PVALUE+1)))))
    #define CHANNEL25_SVALUE    (BYTE)(((WORD)CHANNEL25_COMPARE - ((WORD)(75*(CHANNEL25_PVALUE+1)))))
    #define CHANNEL26_SVALUE    (BYTE)(((WORD)CHANNEL26_COMPARE - ((WORD)(75*(CHANNEL26_PVALUE+1)))))
    #define CHANNEL27_SVALUE    (BYTE)(((WORD)CHANNEL27_COMPARE - ((WORD)(75*(CHANNEL27_PVALUE+1)))))
    #define CHANNEL28_SVALUE    (BYTE)(((WORD)CHANNEL28_COMPARE - ((WORD)(75*(CHANNEL28_PVALUE+1)))))
    #define CHANNEL29_SVALUE    (BYTE)(((WORD)CHANNEL29_COMPARE - ((WORD)(75*(CHANNEL29_PVALUE+1)))))
    #define CHANNEL30_SVALUE    (BYTE)(((WORD)CHANNEL30_COMPARE - ((WORD)(75*(CHANNEL30_PVALUE+1)))))
    #define CHANNEL31_SVALUE    (BYTE)(((WORD)CHANNEL31_COMPARE - ((WORD)(75*(CHANNEL31_PVALUE+1)))))
    #define CHANNEL32_SVALUE    (BYTE)(((WORD)CHANNEL32_COMPARE - ((WORD)(75*(CHANNEL32_PVALUE+1)))))

    #if defined(DATA_RATE_2)
        #define DATARATE        BITRATE_2
        #define BANDWIDTH       BW_50
        #define FREQ_DEV        FREQ_DEV_33
        #define FILCON_SET      FILCON_SET_157
    #elif defined(DATA_RATE_5)
        #define DATARATE        BITRATE_5
        #define BANDWIDTH       BW_50
        #define FREQ_DEV        FREQ_DEV_33
        #define FILCON_SET      FILCON_SET_157
    #elif defined(DATA_RATE_10)
        #define DATARATE        BITRATE_10
        #define BANDWIDTH       BW_50
        #define FREQ_DEV        FREQ_DEV_33
        #define FILCON_SET      FILCON_SET_157
    #elif defined(DATA_RATE_20)
        #define DATARATE        BITRATE_20
        #define BANDWIDTH       BW_75
        #define FREQ_DEV        FREQ_DEV_40
        #define FILCON_SET      FILCON_SET_234
    #elif defined(DATA_RATE_40)
        #define DATARATE        BITRATE_40
        #define BANDWIDTH       BW_150
        #define FREQ_DEV        FREQ_DEV_80
        #define FILCON_SET      FILCON_SET_414
    #elif defined(DATA_RATE_50)
        #define DATARATE        BITRATE_50
        #define BANDWIDTH       BW_175
        #define FREQ_DEV        FREQ_DEV_100
        #define FILCON_SET      FILCON_SET_514
    #elif defined(DATA_RATE_66)
        #define DATARATE        BITRATE_66
        #define BANDWIDTH       BW_250
        #define FREQ_DEV        FREQ_DEV_133
        #define FILCON_SET      FILCON_SET_676
    #elif defined(DATA_RATE_100)
        #define DATARATE        BITRATE_100
        #define BANDWIDTH       BW_400
        #define FREQ_DEV        FREQ_DEV_200
        #define FILCON_SET      FILCON_SET_987
    #elif defined(DATA_RATE_200)
        #define DATARATE        BITRATE_200
        #define BANDWIDTH       BW_400
        #define FREQ_DEV        FREQ_DEV_200
        #define FILCON_SET      FILCON_SET_987
    #else
        #error "No valid data rate defined"
    #endif

    //default register settings
    #define GCONREG_SET     (CHIPMODE_STBYMODE | FREQ_BAND | VCO_TRIM_11)
    #define DMODREG_SET     (0x84 | LNA_GAIN)
    #define FLTHREG_SET     (0x0C)
    #define FIFOCREG_SET    (0xC1)  //FIFO size = 64 bytes and threshold limit for IRQ is 1
    #define PACONREG_SET    (0x38)
    #define FTXRXIREG_SET   (0xC8)
    #define FTPRIREG_SET    (0x0D)
    #define RSTHIREG_SET    (0x00)
    #define FILCONREG_SET   (FILCON_SET | BANDWIDTH)
    #define PFILCREG_SET    (0x38)
    #define SYNCREG_SET     (0x38)
    #define RESVREG_SET     (0x07)
    #define SYNCV31REG_SET  (0x69)
    #define SYNCV23REG_SET  (0x81)
    #define SYNCV15REG_SET  (0x7E)
    #define SYNCV07REG_SET  (0x96)
    #define TXPARAMREG_SET  (0xF0 | (TX_POWER<<1))
    #define CLKOUTREG_SET   (0x88)
    #define PLOADREG_SET    (0x40)
    #define NADDREG_SET     (0x00)
    #define PCONREG_SET     (0xE8)
    #define FCRCREG_SET     (0x00)

    //register description
    #define GCONREG     0x0000
    #define DMODREG     0x0200
    #define FDEVREG     0x0400
    #define BRREG       0x0600
    #define FLTHREG     0x0800
    #define FIFOCREG    0x0A00
    #define R1CNTREG    0x0C00
    #define P1CNTREG    0x0E00
    #define S1CNTREG    0x1000
    #define R2CNTREG    0x1200
    #define P2CNTREG    0x1400
    #define S2CNTREG    0x1600
    #define PACONREG    0x1800
    #define FTXRXIREG   0x1A00
    #define FTPRIREG    0x1C00
    #define RSTHIREG    0x1E00
    #define FILCONREG   0x2000
    #define PFILCREG    0x2200
    #define SYNCREG     0x2400
    #define RESVREG     0x2600
    #define RSTSREG     0x2800
    #define OOKCREG     0x2A00
    #define SYNCV31REG  0x2C00
    #define SYNCV23REG  0x2E00
    #define SYNCV15REG  0x3000
    #define SYNCV07REG  0x3200
    #define TXPARAMREG  0x3400
    #define CLKOUTREG   0x3600
    #define PLOADREG    0x3800
    #define NADDREG     0x3A00
    #define PCONREG     0x3C00
    #define FCRCREG     0x3E00

    #if defined(ENABLE_SECURITY)
        #define MRF89XA_TX_PCKT_SIZE (MRF89XA_TX_BUF_SIZE+PROTOCOL_HEADER_SIZE+ \
                BLOCK_SIZE+MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+9)
        #define MRF89XA_RX_PCKT_SIZE (MRF89XA_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+ \
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+BLOCK_SIZE+9)
    #else
        #define MRF89XA_TX_PCKT_SIZE (MRF89XA_TX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+4)
        #define MRF89XA_RX_PCKT_SIZE (MRF89XA_RX_BUF_SIZE+PROTOCOL_HEADER_SIZE+\
                MY_ADDRESS_LENGTH+MY_ADDRESS_LENGTH+4)
    #endif

    #if MRF89XA_RX_PCKT_SIZE > 64
        #warning "Maximum MRF89XA_RX_PCKT_SIZE should not be greater 64"
        #undef MRF89XA_RX_PCKT_SIZE
        #define MRF89XA_RX_PCKT_SIZE 64
    #endif

    #if MRF89XA_TX_PCKT_SIZE > 64
        #warning "Maximum MRF89XA_TX_PCKT_SIZE should not be greater 64"
        #undef MRF89XA_TX_PCKT_SIZE
        #define MRF89XA_TX_PCKT_SIZE 64
    #endif

    typedef struct{
        union{
            BYTE    Val;
            struct{
                BYTE    Valid       :1;
            }bits;
        }flags;
        BYTE        Payload[MRF89XA_RX_PCKT_SIZE];
        BYTE        PayloadLen;
    }MRF89XA_RX_PACKET;

    typedef struct{
        BOOL        Valid;
        BYTE        Seq;
        MIWI_TICK   startTick;
    }MRF89XA_ACK_INFO;

#endif


#endif