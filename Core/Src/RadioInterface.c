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

void setup_to_transmit_on_next_DSP_Flag(void)
{
	ft8_xmit_counter = 0;
	xmit_sequence();
	transmit();
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
	// uint64_t F_FT8 = (F_Long * 100ULL + (uint64_t)ft8_tone * FT8_TONE_SPACING) / 100ULL;
	uint64_t F_FT8 = F_Long + fsk_freq[ft8_tone];
	set_freq(F_FT8);
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
	uart_tx("MD3;AP1;SWH16;");
	xmit_flag = 1;
}

static void receive()
{
	if (!xmit_flag) {
		return;
	}
	uart_tx("SWH16;MD2;");
	xmit_flag = 0;
}

static void set_freq(uint64_t freq)
{
	char cat_cmd[15];
	snprintf(cat_cmd, sizeof(cat_cmd), "FA%.011lu;", (uint32_t)freq);
	uart_tx(cat_cmd);
}
