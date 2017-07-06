/* 
 * File:   AutoConfByPlatformSelect.h
 * Author: agus
 *
 * Created on July 16, 2013, 1:08 PM
 */

#ifndef HW_VALIDATIONS_H
#define	HW_VALIDATIONS_H

#include "HardwareConfig.h"

////////////////////////////////////////////////////////////////////////////////
/* VALIDATIONS ****************************************************************/
////////////////////////////////////////////////////////////////////////////////

#if !defined(MRF24J40) && !defined(MRF49XA_1) && !defined(MRF49XA_2) && \
    !defined(MRF89XA)
#error "One transceiver must be defined for the wireless application"
#endif

//Communication thru SPI modules.
#if defined MRF49XA_1_IN_SPI1  && (defined MRF49XA_2_IN_SPI1 || \
    defined MRF89XA_IN_SPI1    || defined MRF24J40_IN_SPI1   || \
    defined MRF24WB0M_IN_SPI1) || defined MRF49XA_2_IN_SPI1  && \
    (defined MRF89XA_IN_SPI1   || defined MRF24J40_IN_SPI1   || \
    defined MRF24WB0M_IN_SPI1) || defined MRF89XA_IN_SPI1    && \
    (defined MRF24J40_IN_SPI1  || defined MRF24WB0M_IN_SPI1)
#error "Select only one transceiver using SPI1."
#endif
#if defined MRF49XA_1_IN_SPI2  && (defined MRF49XA_2_IN_SPI2 || \
    defined MRF89XA_IN_SPI2    || defined MRF24J40_IN_SPI2   || \
    defined MRF24WB0M_IN_SPI2) || defined MRF49XA_2_IN_SPI2  && \
    (defined MRF89XA_IN_SPI2   || defined MRF24J40_IN_SPI2   || \
    defined MRF24WB0M_IN_SPI2) || defined MRF89XA_IN_SPI2    && \
    (defined MRF24J40_IN_SPI2  || defined MRF24WB0M_IN_SPI2)
#error "Select only one transceiver using SPI2."
#endif
#if defined MRF49XA_1_IN_SPI3  && (defined MRF49XA_2_IN_SPI3 || \
    defined MRF89XA_IN_SPI3    || defined MRF24J40_IN_SPI3   || \
    defined MRF24WB0M_IN_SPI3) || defined MRF49XA_2_IN_SPI3  && \
    (defined MRF89XA_IN_SPI3   || defined MRF24J40_IN_SPI3   || \
    defined MRF24WB0M_IN_SPI3) || defined MRF89XA_IN_SPI3    && \
    (defined MRF24J40_IN_SPI3  || defined MRF24WB0M_IN_SPI3)
#error "Select only one transceiver using SPI3."
#endif
#if defined MRF49XA_1_IN_SPI4  && (defined MRF49XA_2_IN_SPI4 || \
    defined MRF89XA_IN_SPI4    || defined MRF24J40_IN_SPI4   || \
    defined MRF24WB0M_IN_SPI4) || defined MRF49XA_2_IN_SPI4  && \
    (defined MRF89XA_IN_SPI4   || defined MRF24J40_IN_SPI4   || \
    defined MRF24WB0M_IN_SPI4) || defined MRF89XA_IN_SPI4    && \
    (defined MRF24J40_IN_SPI4  || defined MRF24WB0M_IN_SPI4)
#error "Select only one transceiver using SPI4."
#endif
//----------------------------------------------------------------------------//
#if defined MRF49XA_1 && !defined MRF49XA_1_IN_SPI1 && \
    !defined MRF49XA_1_IN_SPI2 && !defined MRF49XA_1_IN_SPI3 && \
    !defined MRF49XA_1_IN_SPI4
#error "Select one SPI module for MRF49XA_1 transceiver."
#endif
#if defined MRF49XA_2 && !defined MRF49XA_2_IN_SPI1 && \
    !defined MRF49XA_2_IN_SPI2 && !defined MRF49XA_2_IN_SPI3 && \
    !defined MRF49XA_2_IN_SPI4
#error "Select one SPI module for MRF49XA_2 transceiver."
#endif
#if defined MRF89XA && !defined MRF89XA_IN_SPI1 && \
    !defined MRF89XA_IN_SPI2 && !defined MRF89XA_IN_SPI3 && \
    !defined MRF89XA_IN_SPI4
#error "Select one SPI module for MRF89XA transceiver."
#endif
#if defined MRF24J40 && !defined MRF24J40_IN_SPI1 && \
    !defined MRF24J40_IN_SPI2 && !defined MRF24J40_IN_SPI3 && \
    !defined MRF24J40_IN_SPI4
#error "Select one SPI module for MRF24J40 transceiver."
#endif
#if defined MRF24WB0M && !defined MRF24WB0M_IN_SPI1 && \
    !defined MRF24WB0M_IN_SPI2 && !defined MRF24WB0M_IN_SPI3 && \
    !defined MRF24WB0M_IN_SPI4
#error "Select one SPI module for MRF24WB0M transceiver."
#endif
//----------------------------------------------------------------------------//
#if defined MRF49XA_1_IN_SPI1  && (defined MRF49XA_1_IN_SPI2 || \
    defined MRF49XA_1_IN_SPI3  || defined MRF49XA_1_IN_SPI4) || \
    defined MRF49XA_1_IN_SPI2  && (defined MRF49XA_1_IN_SPI3 || \
    defined MRF49XA_1_IN_SPI4) || (defined MRF49XA_1_IN_SPI3 && \
    defined MRF49XA_1_IN_SPI4)
#error "Select only one SPI module for MRF49XA_1 transceiver"
#endif
#if defined MRF49XA_2_IN_SPI1  && (defined MRF49XA_2_IN_SPI2 || \
    defined MRF49XA_2_IN_SPI3  || defined MRF49XA_2_IN_SPI4) || \
    defined MRF49XA_2_IN_SPI2  && (defined MRF49XA_2_IN_SPI3 || \
    defined MRF49XA_2_IN_SPI4) || (defined MRF49XA_2_IN_SPI3 && \
    defined MRF49XA_2_IN_SPI4)
#error "Select only one SPI module for MRF49XA_2 transceiver"
#endif
#if defined MRF89XA_IN_SPI1  && (defined MRF89XA_IN_SPI2 || \
    defined MRF89XA_IN_SPI3  || defined MRF89XA_IN_SPI4) || \
    defined MRF89XA_IN_SPI2  && (defined MRF89XA_IN_SPI3 || \
    defined MRF89XA_IN_SPI4) || (defined MRF89XA_IN_SPI3 && \
    defined MRF89XA_IN_SPI4)
#error "Select only one SPI module for MRF89XA transceiver"
#endif
#if defined MRF24J40_IN_SPI1  && (defined MRF24J40_IN_SPI2 || \
    defined MRF24J40_IN_SPI3  || defined MRF24J40_IN_SPI4) || \
    defined MRF24J40_IN_SPI2  && (defined MRF24J40_IN_SPI3 || \
    defined MRF24J40_IN_SPI4) || (defined MRF24J40_IN_SPI3 && \
    defined MRF24J40_IN_SPI4)
#error "Select only one SPI module for MRF24J40 transceiver"
#endif
#if defined MRF24WB0M_IN_SPI1  && (defined MRF24WB0M_IN_SPI2 || \
    defined MRF24WB0M_IN_SPI3  || defined MRF24WB0M_IN_SPI4) || \
    defined MRF24WB0M_IN_SPI2  && (defined MRF24WB0M_IN_SPI3 || \
    defined MRF24WB0M_IN_SPI4) || (defined MRF24WB0M_IN_SPI3 && \
    defined MRF24WB0M_IN_SPI4)
#error "Select only one SPI module for MRF24WB0M transceiver"
#endif
//----------------------------------------------------------------------------//
#if defined MRF49XA_1_USES_INT0  && (defined MRF49XA_2_USES_INT0 || \
    defined MRF89XA_USES_INT0    || defined MRF24J40_USES_INT0   || \
    defined MRF24WB0M_USES_INT0) || defined MRF49XA_2_USES_INT0  && \
    (defined MRF89XA_USES_INT0   || defined MRF24J40_USES_INT0   || \
    defined MRF24WB0M_USES_INT0) || defined MRF89XA_USES_INT0    && \
    (defined MRF24J40_USES_INT0  || defined MRF24WB0M_USES_INT0) || \
    (defined MRF24J40_USES_INT0  && defined MRF24WB0M_USES_INT0)
#error "Select only one transceiver using external interrupt 1."
#endif
#if defined MRF49XA_1_USES_INT1  && (defined MRF49XA_2_USES_INT1 || \
    defined MRF89XA_USES_INT1    || defined MRF24J40_USES_INT1   || \
    defined MRF24WB0M_USES_INT1) || defined MRF49XA_2_USES_INT1  && \
    (defined MRF89XA_USES_INT1   || defined MRF24J40_USES_INT1   || \
    defined MRF24WB0M_USES_INT1) || defined MRF89XA_USES_INT1    && \
    (defined MRF24J40_USES_INT1  || defined MRF24WB0M_USES_INT1) || \
    (defined MRF24J40_USES_INT1  && defined MRF24WB0M_USES_INT1)
#error "Select only one transceiver using external interrupt 1."
#endif
#if defined MRF49XA_1_USES_INT2  && (defined MRF49XA_2_USES_INT2 || \
    defined MRF89XA_USES_INT2    || defined MRF24J40_USES_INT2   || \
    defined MRF24WB0M_USES_INT2) || defined MRF49XA_2_USES_INT2  && \
    (defined MRF89XA_USES_INT2   || defined MRF24J40_USES_INT2   || \
    defined MRF24WB0M_USES_INT2) || defined MRF89XA_USES_INT2    && \
    (defined MRF24J40_USES_INT2  || defined MRF24WB0M_USES_INT2) || \
    (defined MRF24J40_USES_INT2  && defined MRF24WB0M_USES_INT2)
#error "Select only one transceiver using external interrupt 2."
#endif
#if defined MRF49XA_1_USES_INT3  && (defined MRF49XA_2_USES_INT3 || \
    defined MRF89XA_USES_INT3    || defined MRF24J40_USES_INT3   || \
    defined MRF24WB0M_USES_INT3) || defined MRF49XA_2_USES_INT3  && \
    (defined MRF89XA_USES_INT3   || defined MRF24J40_USES_INT3   || \
    defined MRF24WB0M_USES_INT3) || defined MRF89XA_USES_INT3    && \
    (defined MRF24J40_USES_INT3  || defined MRF24WB0M_USES_INT3) || \
    (defined MRF24J40_USES_INT3  && defined MRF24WB0M_USES_INT3)
#error "Select only one transceiver using external interrupt 3."
#endif
#if defined MRF49XA_1_USES_INT4  && (defined MRF49XA_2_USES_INT4 || \
    defined MRF89XA_USES_INT4    || defined MRF24J40_USES_INT4   || \
    defined MRF24WB0M_USES_INT4) || defined MRF49XA_2_USES_INT4  && \
    (defined MRF89XA_USES_INT4   || defined MRF24J40_USES_INT4   || \
    defined MRF24WB0M_USES_INT4) || defined MRF89XA_USES_INT4    && \
    (defined MRF24J40_USES_INT4  || defined MRF24WB0M_USES_INT4) || \
    (defined MRF24J40_USES_INT4  && defined MRF24WB0M_USES_INT4)
#error "Select only one transceiver using external interrupt 4."
#endif
//----------------------------------------------------------------------------//
#if  defined MRF49XA_1           && !defined MRF49XA_1_USES_INT1 && \
    !defined MRF49XA_1_USES_INT2 && !defined MRF49XA_1_USES_INT3 && \
    !defined MRF49XA_1_USES_INT4
#error "Select one external interrupt for MRF49XA_1 transceiver"
#endif
#if  defined MRF49XA_2           && !defined MRF49XA_2_USES_INT1 && \
    !defined MRF49XA_2_USES_INT2 && !defined MRF49XA_2_USES_INT3 && \
    !defined MRF49XA_2_USES_INT4
#error "Select one external interrupt for MRF49XA_2 transceiver"
#endif
#if  defined MRF89XA           && !defined MRF89XA_USES_INT1 && \
    !defined MRF89XA_USES_INT2 && !defined MRF89XA_USES_INT3 && \
    !defined MRF89XA_USES_INT4
#error "Select one external interrupt for MRF89XA transceiver"
#endif
#if  defined MRF24J40           && !defined MRF24J40_USES_INT1 && \
    !defined MRF24J40_USES_INT2 && !defined MRF24J40_USES_INT3 && \
    !defined MRF24J40_USES_INT4
#error "Select one external interrupt for MRF24J40 transceiver"
#endif
#if  defined MRF24WB0M           && !defined MRF24WB0M_USES_INT1 && \
    !defined MRF24WB0M_USES_INT2 && !defined MRF24WB0M_USES_INT3 && \
    !defined MRF24WB0M_USES_INT4
#error "Select one external interrupt for MRF24J40 transceiver"
#endif
//----------------------------------------------------------------------------//
#if defined MRF49XA_1_USES_INT0  && (defined MRF49XA_1_USES_INT1 || \
    defined MRF49XA_1_USES_INT2  || defined MRF49XA_1_USES_INT3  || \
    defined MRF49XA_1_USES_INT4) || defined MRF49XA_1_USES_INT1  && \
    (defined MRF49XA_1_USES_INT2 || defined MRF49XA_1_USES_INT3  || \
    defined MRF49XA_1_USES_INT4) || defined MRF49XA_1_USES_INT2  && \
    (defined MRF49XA_1_USES_INT3 || defined MRF49XA_1_USES_INT4) || \
    (defined MRF49XA_1_USES_INT3 && defined MRF49XA_1_USES_INT4)
#error "Select only one external interrupt for MRF49XA_1 transceiver"
#endif
#if defined MRF49XA_2_USES_INT0  && (defined MRF49XA_2_USES_INT1 || \
    defined MRF49XA_2_USES_INT2  || defined MRF49XA_2_USES_INT3  || \
    defined MRF49XA_2_USES_INT4) || defined MRF49XA_2_USES_INT1  && \
    (defined MRF49XA_2_USES_INT2 || defined MRF49XA_2_USES_INT3  || \
    defined MRF49XA_2_USES_INT4) || defined MRF49XA_2_USES_INT2  && \
    (defined MRF49XA_2_USES_INT3 || defined MRF49XA_2_USES_INT4) || \
    (defined MRF49XA_2_USES_INT3 && defined MRF49XA_2_USES_INT4)
#error "Select only one external interrupt for MRF49XA_2 transceiver"
#endif
#if defined MRF89XA_USES_INT0  && (defined MRF89XA_USES_INT1 || \
    defined MRF89XA_USES_INT2  || defined MRF89XA_USES_INT3  || \
    defined MRF89XA_USES_INT4) || defined MRF89XA_USES_INT1  && \
    (defined MRF89XA_USES_INT2 || defined MRF89XA_USES_INT3  || \
    defined MRF89XA_USES_INT4) || defined MRF89XA_USES_INT2  && \
    (defined MRF89XA_USES_INT3 || defined MRF89XA_USES_INT4) || \
    (defined MRF89XA_USES_INT3 && defined MRF89XA_USES_INT4)
#error "Select only one external interrupt for MRF89XA transceiver"
#endif
#if defined MRF24J40_USES_INT0  && (defined MRF24J40_USES_INT1 || \
    defined MRF24J40_USES_INT2  || defined MRF24J40_USES_INT3  || \
    defined MRF24J40_USES_INT4) || defined MRF24J40_USES_INT1  && \
    (defined MRF24J40_USES_INT2 || defined MRF24J40_USES_INT3  || \
    defined MRF24J40_USES_INT4) || defined MRF24J40_USES_INT2  && \
    (defined MRF24J40_USES_INT3 || defined MRF24J40_USES_INT4) || \
    (defined MRF24J40_USES_INT3 && defined MRF24J40_USES_INT4)
#error "Select only one external interrupt for MRF24J40 transceiver"
#endif
#if defined MMRF24WB0M_USES_INT0  && (defined MRF24WB0M_USES_INT1 || \
    defined MRF24WB0M_USES_INT2  || defined MRF24WB0M_USES_INT3  || \
    defined MRF24WB0M_USES_INT4) || defined MRF24WB0M_USES_INT1  && \
    (defined MMRF24WB0M_USES_INT2 || defined MRF24WB0M_USES_INT3  || \
    defined MRF24WB0M_USES_INT4) || defined MRF24WB0M_USES_INT2  && \
    (defined MRF24WB0M_USES_INT3 || defined MRF24WB0M_USES_INT4) || \
    (defined MRF24WB0M_USES_INT3 && defined MRF24WB0M_USES_INT4)
#error "Select only one external interrupt for MRF24WB0M transceiver"
#endif

//UART validations.
#if defined DEBUG_UART1  && (defined DEBUG_UART2 || defined DEBUG_UART3  || \
    defined DEBUG_UART4  ||  defined DEBUG_UART5 || defined DEBUG_UART6) || \
    defined DEBUG_UART2  && (defined DEBUG_UART3 || defined DEBUG_UART4  || \
    defined DEBUG_UART5  || defined DEBUG_UART6) || defined DEBUG_UART3  && \
    (defined DEBUG_UART4 || defined DEBUG_UART5  || defined DEBUG_UART6) || \
    defined DEBUG_UART4  && (defined DEBUG_UART5 || defined DEBUG_UART6) || \
    (defined DEBUG_UART5 && defined DEBUG_UART6)
#error "Select only one UART module for debugging."
#endif

#if defined ENABLE_CONSOLE && !defined DEBUG_UART1 && !defined DEBUG_UART2 && \
    !defined DEBUG_UART3   && !defined DEBUG_UART4 && !defined DEBUG_UART5 && \
    !defined DEBUG_UART6
#error "Select one UART module for debugging."
#endif

#if (defined MRF49XA_1_IN_SPI2 || defined MRF49XA_2_IN_SPI2 || \
    defined MRF89XA_IN_SPI2    || defined MRF24J40_IN_SPI2  || \
    defined MRF24WB0M_IN_SPI2)
#if defined DEBUG_UART3
#error "UART3 and SPI2 modules are incompatible."
#elif defined DEBUG_UART6
#define UART6_RXONLY
#warning "UART6 and SPI2 share some pins. UART6 will be configured in "\
                 "RXonly mode, avoiding debuggin"
#endif
#endif
#if (defined MRF49XA_1_IN_SPI3 || defined MRF49XA_2_IN_SPI3 || \
    defined MRF89XA_IN_SPI3    || defined MRF24J40_IN_SPI3  || \
    defined MRF24WB0M_IN_SPI3)
#if defined DEBUG_UART1
#error "UART1 and SPI3 modules are incompatible."
#elif defined DEBUG_UART4
#warning "UART4 and SPI3 share some pins. Check SPI3 is configured in "\
                 "normal mode and UART4 in RX only mode."
#endif
#endif
#if defined MRF49XA_1_IN_SPI4 || defined MRF49XA_2_IN_SPI4 || \
    defined MRF24J40_IN_SPI4  || defined MRF89XA_IN_SPI4   || \
    defined MRF24WB0M_IN_SPI4
#if defined DEBUG_UART2
#error "UART2 and SPI4 modules are incompatible."
#elif defined DEBUG_UART5
#warning "UART5 and SPI4 share some pins. Check SPI4 is configured in "\
                 "normal mode and UART5 in RX only mode."
#endif
#endif

// Suitable transceivers for the radio interfaces.
// Select the appropriate frequency band for each transceiver according to
// your radio interfaces definition in "ConfigTransceivers.h" file.
#if defined MIWI_2400_RI && !defined MRF24J40
#error "MiWi at 2,4 GHz band requires MRF24J40 transceiver."
#endif
#if defined MIWI_0868_RI
/***************************************************************************
 * MRF89XA operates in either 868, 915 or 955 MHz band. For Europe, only 868
 * MHz can be selected so, if defined MRF89XA, 868 MHz band must be chosen.
 * If not, a MRF49XA transceiver must use 868 MHz band in order to use
 * MIWI_0868_RI
 **************************************************************************/
#if defined MRF89XA && !defined BAND_863
#error "MRF89XA must define BAND_863. See ConfigTransceivers.h file"
#elif (!defined MRF49XA_1_IN_868 && !defined MRF49XA_2_IN_868)
#error "If MRF49XA is chosen for MIWI_0868_RI, it must define MiWi "
"868 MHz band. See ConfigTransceivers.h file"
#endif
#endif
#if defined MIWI_0434_RI
#if (!defined MRF49XA_1_IN_434 && !defined MRF49XA_2_IN_434)
#error "One MRF49XA transceivers must define MiWi 434 MHz band. "
"See ConfigTransceivers.h file"
#endif
#endif
#if defined WIFI_2400_RI && !defined MRF24WB0M
#error "WiFi at 2,4 GHz band requires MRF24WB0M transceiver."
#endif

// Number of interfaces available
#if !defined(MIWI_2400_RI) && !defined(MIWI_0868_RI) && !defined(MIWI_0434_RI) \
    && !defined(WIFI_2400_RI)
#error "At least one Radio Interface must be defined."
#endif





#endif	/* AUTOCONFBYPLATFORMSELECT_H */


