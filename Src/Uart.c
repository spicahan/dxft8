/**
  ******************************************************************************
  * @file    Uart.c
  * @brief   UART module implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "Uart.h"

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef UartHandle;
static __IO ITStatus UartReady = RESET;

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize UART peripheral
  * @param  None
  * @retval None
  */
void uart_init(void)
{
  /* Configure UART */
  UartHandle.Instance = USARTx;
  UartHandle.Init.BaudRate = 38400;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits = UART_STOPBITS_1;
  UartHandle.Init.Parity = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode = UART_MODE_TX;
  
  /* KX2/KX3 specific configuration */
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT;
  UartHandle.AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
  
  if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    while(1); /* Error */
  }

  if (HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    while(1); /* Error */
  }
}

/**
  * @brief  Transmit data via UART
  * @param  data: Null-terminated string to transmit
  * @retval None
  */
void uart_tx(const char *data)
{
  uint16_t len = strlen(data);
  
  /* Reset transmission flag */
  UartReady = RESET;
  
  /* Start DMA transmission */
  if (HAL_UART_Transmit_DMA(&UartHandle, (uint8_t*)data, len) != HAL_OK)
  {
    while(1); /* Error */
  }
  
  /* Wait for transmission to complete */
  // while (UartReady != SET)
  // {
  //   /* Wait */
  // }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of DMA Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  (void)UartHandle;
  /* Set transmission flag: trasfer complete*/
  UartReady = SET;
}