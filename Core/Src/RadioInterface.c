/*
 * traffic_manager.c
 *
 *  Created on: Feb 29, 2020
 *      Author: user
 */

#include <stdbool.h>
#include <Display.h>
#include "button.h"
// #include "gen_ft8.h"
// #include "decode_ft8.h"
// #include "SDR_Audio.h"
#include "RadioInterface.h"
#include "Process_DSP.h"
#include "Codec_Gains.h"
#include "Uart.h"
#include "main.h"

const int ADC_DVC_Gain = 180;
const int ADC_DVC_Off = 90;

#define FT8_TONE_SPACING 625

static uint64_t F_Long;

static void set_freq(uint64_t freq);
static void transmit();
static void receive();

void radio_init(void)
{
	// UART is initialized by CubeMX (MX_USART6_UART_Init)
	// Nothing additional to initialize for KX3 mode
}

void setup_to_transmit_on_next_DSP_Flag(void)
{
	ft8_xmit_counter = 0;
	xmit_sequence();
	ft8_transmit_sequence();
	xmit_flag = 1;
}

void terminate_QSO(void)
{
	receive();
	ft8_receive_sequence();
	receive_sequence();
	xmit_flag = 0;
	ft8_xmit_delay = 0;
}

void ft8_transmit_sequence(void)
{
	Set_ADC_DVC(ADC_DVC_Off);
	// HAL_Delay(10);
	set_Xmit_Freq();
	transmit();
	HAL_Delay(10);
}

void ft8_receive_sequence(void)
{
	receive();
	// HAL_Delay(10);
	set_Rcvr_Freq();
	Set_ADC_DVC(ADC_DVC_Gain);
}

void tune_On_sequence(void)
{
	Set_ADC_DVC(ADC_DVC_Off);
	HAL_Delay(10);
	set_Xmit_Freq();
	HAL_Delay(10);
	transmit();
}

void tune_Off_sequence(void)
{
	ft8_receive_sequence();
}

void set_Xmit_Freq(void)
{
	F_Long = start_freq * 1000ULL + (uint16_t)NCO_Frequency;
	set_freq(F_Long);
}

void set_FT8_Tone(uint8_t ft8_tone)
{
	static const uint8_t fsk_freq[8] = {0, 6, 13, 19, 25, 31, 38, 44};
	char cat_cmd[6];
	snprintf(cat_cmd, sizeof(cat_cmd), "FO%.02u;", fsk_freq[ft8_tone]);
	uart_tx(cat_cmd);
}

void set_Rcvr_Freq(void)
{
	uint64_t F_Receive = start_freq * 1000ULL;
	set_freq(F_Receive);
}

// Radio implementation
static void transmit()
{
	if (xmit_flag) {
		return;
	}
	uart_tx("MD2;HK1;");
	xmit_flag = 1;
}

static void receive()
{
	if (!xmit_flag) {
		return;
	}
	uart_tx("HK0;FO99;MD2;");
	xmit_flag = 0;
}

static void set_freq(uint64_t freq)
{
	char cat_cmd[11];
	// KH1 supports only 10Hz resolution
	snprintf(cat_cmd, sizeof(cat_cmd), "FA%.07lu;", (uint32_t)freq / 10);
	uart_tx(cat_cmd);
}
