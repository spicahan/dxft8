
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#ifdef HOST_HAL_MOCK
#include "host_mocks.h"
#else
#include "stm32746g_discovery.h"
#include "stm32f7xx_hal.h"
#endif
void _debug(const char *txt);
void tx_display_update();

// Original APIs for EXT_I2C_*
void CAMERA_IO_Init(void);
uint8_t CAMERA_IO_Read(uint8_t Addr, uint8_t Reg);
void CAMERA_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);


#define NoOp  __NOP()

#define MAX_QUEUE_SIZE 9

extern uint32_t start_time, ft8_time;

extern int QSO_xmit;

extern int slot_state;
extern int target_slot;
extern int target_freq;
extern int decode_flag;
extern int was_txing;
extern bool clr_pressed;
extern bool tx_pressed;
extern bool free_text;

extern const char* test_data_file;

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define SDMMC_CK_Pin GPIO_PIN_12
#define SDMMC_CK_GPIO_Port GPIOC
#define SDMMC_D0_Pin GPIO_PIN_2
#define SDMMC_D0_GPIO_Port GPIOD
#define SDMMC_D2_Pin GPIO_PIN_10
#define SDMMC_D2_GPIO_Port GPIOC
#define SDMMC_D3_Pin GPIO_PIN_11
#define SDMMC_D3_GPIO_Port GPIOC

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
