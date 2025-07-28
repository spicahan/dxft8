/**
  ******************************************************************************
  * @file    Uart.h
  * @brief   UART module header
  ******************************************************************************
  */

#ifndef __UART_H
#define __UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* UART configuration defines -----------------------------------------------*/
#define USARTx                           USART6
#define USARTx_CLK_ENABLE()              __USART6_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA2_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOC_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART6_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART6_RELEASE_RESET()

/* UART GPIO pins */
#define USARTx_TX_PIN                    GPIO_PIN_6
#define USARTx_TX_GPIO_PORT              GPIOC
#define USARTx_TX_AF                     GPIO_AF8_USART6

/* UART DMA configuration */
#define USARTx_TX_DMA_STREAM             DMA2_Stream6
#define USARTx_TX_DMA_CHANNEL            DMA_CHANNEL_5
#define USARTx_DMA_TX_IRQn               DMA2_Stream6_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA2_Stream6_IRQHandler

/* Exported functions --------------------------------------------------------*/
void uart_init(void);
void uart_tx(const char *data);

#endif /* __UART_H */

