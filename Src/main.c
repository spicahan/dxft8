/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "dma2d.h"
#include "fatfs.h"
#include "i2c.h"
#include "ltdc.h"
#include "sai.h"
#include "sdmmc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifndef HOST_HAL_MOCK
#include "stm32f7xx_hal_rcc.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
#include "arm_math.h"

#include "SDR_Audio.h"
#include "Process_DSP.h"
#include "Codec_Gains.h"
#include "button.h"

#include "DS3231.h"

#include "SiLabs.h"

#include "options.h"
#endif

#include "autoseq_engine.h"
#include "constants.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "log_file.h"
#include "Display.h"
#include "qso_display.h"
#include "traffic_manager.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t start_time, ft8_time;

int QSO_xmit = 0;
int target_slot;
int target_freq;
int slot_state = 0;
int decode_flag = 0;
// Used for skipping the TX slot
int was_txing = 0;
bool clr_pressed = false;
bool free_text = false;
bool tx_pressed = false;

// Autoseq TX text buffer
static char autoseq_txbuf[MAX_MSG_LEN];
// Autoseq QSO states text
static char autoseq_state_strs[MAX_QUEUE_SIZE][MAX_LINE_LEN];
// Autoseq ctx queue log text
static char autoseq_queue_strs[MAX_QUEUE_SIZE][53];
static int master_decoded = 0;
static bool worked_qsos_in_display = false;
// Used for display RX and TX after returning from Tune
static bool tune_pressed = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Helper function for updating TX region display
void tx_display_update()
{
	if (Tune_On || worked_qsos_in_display) {
		return;
	}
	if (xmit_flag) {
		display_txing_message(autoseq_txbuf);
	} else {
		display_queued_message(autoseq_txbuf);
	}
	autoseq_get_qso_states(autoseq_state_strs);
	display_qso_state(autoseq_state_strs);
}

static void update_synchronization(void)
{
	uint32_t current_time = HAL_GetTick();
	ft8_time = current_time - start_time;

	// Update slot and reset RX
	int current_slot = ft8_time / 15000 % 2;
	if (current_slot != slot_state)
	{
		// toggle the slot state
#ifdef HOST_HAL_MOCK
		printf("\n----------------------------------------\n");
		printf("slot state %d -> %d\n", slot_state, slot_state ^ 1);
#endif
		slot_state ^= 1;
		if (was_txing) {
			autoseq_tick();
		}
		was_txing = 0;

		ft8_flag = 1;
		FT_8_counter = 0;
		ft8_marker = 1;

		tx_display_update();
	}

	// Check if TX is intended
	if (QSO_xmit && target_slot == slot_state && FT_8_counter < 29)
	{
		setup_to_transmit_on_next_DSP_Flag(); // TODO: move to main.c
		QSO_xmit = 0;
		was_txing = 1;
		// Partial TX, set the TX counter based on current ft8_time
		ft8_xmit_counter = (ft8_time % 15000) / 160; // 160ms per symbol
		// Log the TX
		char log_str[128];
		make_Real_Time();
		make_Real_Date();
		// Log the ctx queue
		autoseq_log_ctx_queue(autoseq_queue_strs);
		for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
			const char *cur_line = autoseq_queue_strs[i];
			if (cur_line[0] == '\0') {
				break;
			}
			Write_RxTxLog_Data(cur_line);
		}
		snprintf(log_str, sizeof(log_str), "T [%s %s][%s] %s",
				 log_rtc_date_string,
				 log_rtc_time_string,
				 sBand_Data[BandIndex].display,
				 autoseq_txbuf);
		Write_RxTxLog_Data(log_str);
		tx_display_update();
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DMA2D_Init();
  MX_USART6_UART_Init();
  MX_SAI2_Init();
  MX_LTDC_Init();
  MX_FMC_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_TIM6_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */
  start_audio_I2C();

  PTT_Out_Init();

  Init_BoardVersionInput();
  Check_Board_Version();
  DeInit_BoardVersionInput();

//   HID_InitApplication(); // really sets up LCD Display, leftover from example
  HAL_Delay(10);
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  initalize_constants();

  init_DSP();

  SD_Initialize();
  Read_Station_File();
  setup_display();

  Options_Initialize();

  CAMERA_IO_Init();
  HAL_Delay(10);
  DS3231_init();
  display_Real_Date(0, 240);

  start_Si5351();

  Set_Cursor_Frequency();
  show_variable(400, 25, (int)NCO_Frequency);
  show_short(667, 255, AGC_Gain);
  start_duplex();
  HAL_Delay(10);
  set_codec_input_gain();
  HAL_Delay(10);
  receive_sequence();
  HAL_Delay(10);
  Set_Headphone_Gain(30);
  Init_Log_File();
  FT8_Sync();
  HAL_Delay(10);

  autoseq_init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (DSP_Flag)
	{
		I2S2_RX_ProcessBuffer(buff_offset);

		if (xmit_flag)
		{
			// Start sending FT8 messages about 0.1 to 0.5 seconds into the time slot
			// to match the observed behavior.
			if (ft8_xmit_delay >= 28)
			{
				if (!Arm_Tune)
				{
					if ((ft8_xmit_counter < 79) && (frame_counter == 2))
					{
						set_FT8_Tone(tones[ft8_xmit_counter]);
						ft8_xmit_counter++;
					}

					if (ft8_xmit_counter == 79)
					{
						xmit_flag = 0;
						ft8_receive_sequence();
						receive_sequence();
						ft8_xmit_delay = 0;
					}
				}
			}
			else
			{
				if (++ft8_xmit_delay == 16)
					output_enable(SI5351_CLK0, 1);
			}
		}

		// Called at 25Hz, need to be efficient
		display_RealTime(100, 240);

		// falling edge detection - tune mode exited
		if (!Tune_On && tune_pressed)
		{
			// Need to display RX and TX again
			display_messages(new_decoded, master_decoded);
			tx_display_update();
		}
		tune_pressed = Tune_On;

		DSP_Flag = 0;
	}

	if (decode_flag && !xmit_flag)
	{
		master_decoded = ft8_decode();
		if (!Tune_On)
		{
			display_messages(new_decoded, master_decoded);
		}
		// Write all the decoded messages to RxTxLog
		make_Real_Time();
		make_Real_Date();
		for (int i = 0; i < master_decoded; ++i)
		{
			char log_str[64];
			snprintf(log_str, sizeof(log_str), "%c [%s %s][%s] %s %s %s %2i %d",
					 was_txing ? 'O' : 'R',
					 log_rtc_date_string,
					 log_rtc_time_string,
					 sBand_Data[BandIndex].display,
					 new_decoded[i].call_to,
					 new_decoded[i].call_from,
					 new_decoded[i].locator,
					 new_decoded[i].snr,
					 new_decoded[i].freq_hz);
			Write_RxTxLog_Data(log_str);
		}
		if (!was_txing)
		{
			autoseq_on_decodes(new_decoded, master_decoded);
			if (autoseq_get_next_tx(autoseq_txbuf))
			{
				queue_custom_text(autoseq_txbuf);
				QSO_xmit = 1;
			}
			else if (Beacon_On)
			{
				autoseq_start_cq();
				autoseq_get_next_tx(autoseq_txbuf);
				queue_custom_text(autoseq_txbuf);
				QSO_xmit = 1;
				target_slot = slot_state ^ 1;
			}
			tx_display_update();
		}

		decode_flag = 0;
	} // end of servicing FT_Decode

	// Check if touch happened
	// Note: In HOST_HAL_MOCK mode, touch events are simulated via JSON test data

	Process_Touch();

	if (clr_pressed)
	{
		terminate_QSO();
		QSO_xmit = 0;
		was_txing = 0;
		autoseq_init();
		autoseq_txbuf[0] = '\0';
		tx_display_update();
		clr_pressed = false;
	}

	if (tx_pressed)
	{
		worked_qsos_in_display = display_worked_qsos();
		tx_pressed = false;
		tx_display_update();
	}

	if (!Tune_On && FT8_Touch_Flag && FT_8_TouchIndex < master_decoded)
	{
		process_selected_Station(master_decoded, FT_8_TouchIndex);
		autoseq_on_touch(&new_decoded[FT_8_TouchIndex]);
		if (autoseq_get_next_tx(autoseq_txbuf))
		{
			queue_custom_text(autoseq_txbuf);
			QSO_xmit = 1;
		}
		tx_display_update();
		FT8_Touch_Flag = 0;
	}

	update_synchronization();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SAI2;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 50;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
