/* This source file is part of the ATMEL QTouch Library Release 4.4 */
/******************************************************************************
 *
 * \file
 *
 * \brief  This file contains the BitBangSPI Configuration settings.
 *
 * - Compiler:           IAR EWAVR and GNU GCC for AVR32
 * - Supported devices:  AT32UC3A0/A1 Series, AT32UC3B0/B1 Series 
 *                       AND AT32UC3C0/C1 Series 
 * - Userguide:          QTouch Library User Guide - doc8207.pdf.
 * - Support email:      touch@atmel.com
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *   Copyright (c) 2010-2012, Atmel Corporation All rights reserved.
 *  
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 ******************************************************************************/


/*============================ INCLUDES ======================================*/
#ifdef _DEBUG_INTERFACE_
#include <compiler.h>
#include "BitBangSPI_Master.h"
#include "QDebugTransport.h"

#if defined(__ICCAVR32__)
 #include "intrinsics.h"
#endif


#define PORTA (((volatile avr32_gpio_port_t *) &AVR32_GPIO) + 0)
#define PORTB (((volatile avr32_gpio_port_t *) &AVR32_GPIO) + 1)
#define PORTC (((volatile avr32_gpio_port_t *) &AVR32_GPIO) + 2)

#if (UC3A0 || UC3A1 || UC3B)
#define PORTX (((volatile avr32_gpio_port_t *) &AVR32_GPIO) + 3)
#elif UC3C
#define PORTD (((volatile avr32_gpio_port_t *) &AVR32_GPIO) + 3)
#else
# error Unsupported chip type
#endif




/*============================ IMPLEMENTATION ================================*/

/*============================================================================
Name    :   BitBangSPI_Master_Init
------------------------------------------------------------------------------
Purpose :   Initialize BitBangSPI Interface
Input   :   n/a
Output  :   n/a
Notes   :	Called from QDebug_Init in QDebug.c
============================================================================*/
void BitBangSPI_Master_Init (void)
{
    REG( PORT, SPI_BB_SS )->oders =  ((1<<SS_BB) );
    REG( PORT, SPI_BB_MOSI )->oders =  (( 1 << MOSI_BB ) );
    REG( PORT, SPI_BB_SCK )->oders =  ( ( 1 << SCK_BB ));

    REG( PORT, SPI_BB_MISO )->oderc =  ( 1 << MISO_BB );

    REG( PORT, SPI_BB_SS )->ovrc =  ((1<<SS_BB) );
    REG( PORT, SPI_BB_MOSI )->ovrc =  (( 1 << MOSI_BB ) );
    REG( PORT, SPI_BB_SCK )->ovrc =  ( ( 1 << SCK_BB ));

    REG( PORT, SPI_BB_MISO )->ovrs =  ( 1 << MISO_BB );
}

/*============================================================================
Name    :   BitBangSPI_Send_Byte
------------------------------------------------------------------------------
Purpose :   Send and Read one byte using BitBangSPI Interface
Input   :   Data to send to slave
Output  :   Data read from slave
Notes   :	Called from BitBangSPI_Send_Message in this file
============================================================================*/
uint8_t BitBangSPI_Send_Byte(uint8_t c)
{
    unsigned bit;
    for (bit = 0; bit < 8; bit++) {
        /* write MOSI on trailing edge of previous clock */
        if (c & 0x80)
            REG( PORT, SPI_BB_MOSI )->ovrs =  ( 1 << MOSI_BB );
        else
            REG( PORT, SPI_BB_MOSI )->ovrc =  ( 1 << MOSI_BB );

        c <<= 1;

        /* half a clock cycle before leading/rising edge */
        DELAY_1US();

        REG( PORT, SPI_BB_SCK )->ovrs = ( 1 << SCK_BB );

        /* half a clock cycle before trailing/falling edge */
        DELAY_1US();

        /* read MISO on trailing edge */
        c |= ((REG( PORT, SPI_BB_MISO )->pvr >> MISO_BB) & 0x01);
        REG( PORT, SPI_BB_SCK )->ovrc = ( 1 << SCK_BB );
    }

    REG( PORT, SPI_BB_MOSI )->ovrc =  ( 1 << MOSI_BB );

    DELAY_75US();


    return c;
}


/*============================================================================
Name    :   BitBangSPI_Send_Message
------------------------------------------------------------------------------
Purpose :   Send and Read one frame using BitBangSPI Interface
Input   :   n/a
Output  :   n/a
Notes   :	Called from Send_Message in QDebugTransport.c
============================================================================*/
void BitBangSPI_Send_Message(void)
{
    unsigned int i;
    uint8_t FrameInProgress;

    // Send our message upstream
    for (i=0; i <= TX_index; i++)
    {
        FrameInProgress = RxHandler(BitBangSPI_Send_Byte(TX_Buffer[i]));
    }

    // Do we need to receive even more bytes?
    while (FrameInProgress)
        FrameInProgress = RxHandler(BitBangSPI_Send_Byte(0));

}
#endif 
