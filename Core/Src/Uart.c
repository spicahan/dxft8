/**
  ******************************************************************************
  * @file    Uart.c
  * @brief   UART module implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "Uart.h"
#include "main.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart6;

/* Private variables ---------------------------------------------------------*/
__IO ITStatus UartReady = RESET;

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize UART for KX2/KX3 communication
  * @note   UART peripheral is already configured by MX_USART6_UART_Init()
  *         This function just enables the RXNE interrupt to discard incoming data
  * @param  None
  * @retval None
  */
void uart_init(void)
{
  /* Enable USART6 interrupt */
  HAL_NVIC_SetPriority(USART6_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART6_IRQn);

  /* Enable RXNE interrupt to immediately discard incoming data */
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);
}


/**
  * @brief  Transmit data via UART
  * @param  data: Null-terminated string to transmit
  * @retval None
  */
void uart_tx(const char *data)
{
  uint16_t len = strnlen(data, 16);

  if(HAL_UART_Transmit_IT(&huart6, (uint8_t*)data, len) != HAL_OK)
  {
    Error_Handler();
  }

  /* Wait for transmission to complete */
  while (UartReady != SET)
  {
    /* Wait */
  }
  /* Reset transmission flag */
  UartReady = RESET;
}

/* Callback functions --------------------------------------------------------*/

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART6)
  {
    /* Set transmission flag: transfer complete */
    UartReady = SET;
  }
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART6)
  {
    /* Set transmission flag: transfer complete */
    UartReady = SET;
  }
}

/**
  * @brief  UART error callbacks
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  (void)huart;
  /* Silently handle errors - don't call Error_Handler to avoid hanging */
}
