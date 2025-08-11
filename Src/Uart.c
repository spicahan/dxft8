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
/* UART handler declaration */
UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;

static void Error_Handler(void);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize UART peripheral
  * @param  None
  * @retval None
  */
void uart_init(void)
{
  /* Configure UART */
  UartHandle.Instance        = USARTx;

  UartHandle.Init.BaudRate   = 38400;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  // For KX2/KX3
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXINVERT_INIT |
                                           UART_ADVFEATURE_TXINVERT_INIT;
  UartHandle.AdvancedInit.RxPinLevelInvert  = UART_ADVFEATURE_RXINV_ENABLE;
  UartHandle.AdvancedInit.TxPinLevelInvert  = UART_ADVFEATURE_TXINV_ENABLE;
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Ensure UART is in READY state since we don't want to receive data */
  UartHandle.State = HAL_UART_STATE_READY;
  
  /* Enable RXNE interrupt to immediately discard incoming data */
  __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
}
  

/**
  * @brief  Transmit data via UART
  * @param  data: Null-terminated string to transmit
  * @retval None
  */
void uart_tx(const char *data)
{
  uint16_t len = strnlen(data, 16);
  
  if(HAL_UART_Transmit_IT(&UartHandle, (uint8_t*)data, len) != HAL_OK)
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

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  (void)UartHandle;
  /* Set transmission flag: trasfer complete*/
  UartReady = SET;

  
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  (void)UartHandle;
  /* Set transmission flag: trasfer complete*/
  UartReady = SET;

  
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
// Debug counter to track unexpected RX handling
volatile uint32_t unexpectedRxCount = 0;

void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
    // Debug: Capture all relevant debugging info
    // Error codes: PE=0x01, NE=0x02, FE=0x04, ORE=0x08, DMA=0x10
    volatile uint32_t errorCode = UartHandle->ErrorCode;
    volatile uint32_t uartState = UartHandle->State;
    volatile uint32_t rxXferCount = UartHandle->RxXferCount;
    volatile uint32_t isrFlags = UartHandle->Instance->ISR;
    volatile uint32_t cr1Reg = UartHandle->Instance->CR1;
    volatile uint32_t cr3Reg = UartHandle->Instance->CR3;
    volatile uint32_t rdrData = UartHandle->Instance->RDR;
    volatile uint32_t debugUnexpectedRx = unexpectedRxCount;
    
    // Set breakpoint here to examine all variables
    (void)errorCode;
    (void)uartState; 
    (void)rxXferCount;
    (void)isrFlags;
    (void)cr1Reg;
    (void)cr3Reg;
    (void)rdrData;
    (void)debugUnexpectedRx;
    
    Error_Handler();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while(1){

  }
    
}

// Copied from stm32f7xx_hal_msp.c
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  (void)huart;
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();


  /* Enable USARTx clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
    
  /*##-3- Configure the NVIC for UART ########################################*/
  /* NVIC for USART */
  HAL_NVIC_SetPriority(USARTx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  (void)huart;
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
  
  /*##-3- Disable the NVIC for UART ##########################################*/
  HAL_NVIC_DisableIRQ(USARTx_IRQn);
}

