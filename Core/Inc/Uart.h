/**
  ******************************************************************************
  * @file    Uart.h
  * @brief   UART module header for KX2/KX3 communication
  ******************************************************************************
 */

#ifndef __UART_H
#define __UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported functions --------------------------------------------------------*/
void uart_tx(const char *data);

#endif /* __UART_H */
