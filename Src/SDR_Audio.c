/*
 * SDR_Audio.c
 *
 *  Created on: May 10, 2016
 *      Author: user
 */

#include <stdlib.h>
#include "SDR_Audio.h"
#include "stm32746g_discovery_audio.h"
#include "wm8994.h"
#include "stm32746g_discovery_lcd.h"
#include "arm_math.h"
#include "Process_DSP.h"
#include "main.h" // for decode_flag
#include "button.h"
#include "Sine_table.h"

q15_t FIR_I_In[BUFFERSIZE / 4];
q15_t FIR_Q_In[BUFFERSIZE / 4];
q15_t FIR_I_Out[BUFFERSIZE / 4];
q15_t FIR_Q_Out[BUFFERSIZE / 4];

q15_t FT8_Data[2048 / 2];

// DSP_Flag encoding: 0=idle, 1=second half ready, 2=first half ready
// Main loop uses (DSP_Flag & 1) to derive offset: 1->BUFFERSIZE/2, 0->0
volatile int DSP_Flag;

#define PI2 6.2831853071795864765
#define KCONV 10430.37835 // 		4096*16/PI2

// static const double LO_Freq = 10000;
static const int Sample_Frequency = 32000;

static q15_t USB_Out[BUFFERSIZE / 4];
static q15_t LSB_Out[BUFFERSIZE / 4];
static q15_t in_buff[BUFFERSIZE];
static q15_t out_buff[BUFFERSIZE];

void start_audio_I2C(void)
{
	AUDIO_IO_Init();
}

void config_line_in(void)
{
	/* Configure WM8994 for SDR line-in operation (I/Q inputs)
	 * These settings override the standard BSP line-in configuration
	 * to provide proper differential input routing and 0dB gain path
	 * suitable for SDR applications.
	 */

	/* Register 0x28: Input Mixer 3 - Configure differential input routing
	 * IN1LN_TO_IN1L = 1 (bit 0): Connect IN1L negative pin to left input
	 * IN1LP_TO_VMID = 0 (bit 1): Connect IN1L positive pin to VMID (ground ref)
	 * IN1RN_TO_IN1R = 1 (bit 4): Connect IN1R negative pin to right input
	 * IN1RP_TO_VMID = 0 (bit 5): Connect IN1R positive pin to VMID (ground ref)
	 * This creates pseudo-differential inputs for I and Q channels */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x28, 0x0011);
	AUDIO_IO_Delay(2);

	/* Register 0x29: Input Mixer 4 - Connect IN1L to left input mixer
	 * IN1L_TO_MIXINL = 1 (bit 5): Enable connection
	 * IN1L_MIXINL_VOL = 0dB: Unity gain (no preamp boost)
	 * CRITICAL: Without this, codec uses mic preamp path with ~30dB gain */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x29, 0x0020);
	AUDIO_IO_Delay(2);

	/* Register 0x2A: Input Mixer 5 - Connect IN1R to right input mixer
	 * IN1R_TO_MIXINR = 1 (bit 5): Enable connection
	 * IN1R_MIXINR_VOL = 0dB: Unity gain (no preamp boost)
	 * CRITICAL: Without this, codec uses mic preamp path with ~30dB gain */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x2A, 0x0020);
	AUDIO_IO_Delay(2);

	/* Register 0x18: Left Line Input 1&2 Volume
	 * IN1L_VU = 1 (bit 7): Volume update bit (synchronize L/R updates)
	 * IN1L_VOL = 0x0B: Volume setting (~+1.5dB) */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x18, 0x008B);
	AUDIO_IO_Delay(2);

	/* Register 0x1A: Right Line Input 1&2 Volume
	 * IN1R_VU = 1 (bit 8): Volume update bit
	 * IN1R_VOL = 0x0B: Volume setting (~+1.5dB) */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x1A, 0x018B);
	AUDIO_IO_Delay(2);

	/* Register 0x400: Left AIF1 ADC1 Volume
	 * 0x00C0 = 0dB digital gain (no boost) */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x400, 0x00C0);
	AUDIO_IO_Delay(2);

	/* Register 0x401: Right AIF1 ADC1 Volume
	 * 0x01C0 = 0dB digital gain with VU bit */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x401, 0x01C0);
	AUDIO_IO_Delay(2);

	/* Register 0x404: Left AIF1 ADC2 Volume (for Q channel)
	 * 0x00C0 = 0dB digital gain */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x404, 0x00C0);
	AUDIO_IO_Delay(2);

	/* Register 0x405: Right AIF1 ADC2 Volume (for Q channel)
	 * 0x01C0 = 0dB digital gain with VU bit */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x405, 0x01C0);
	AUDIO_IO_Delay(2);

	/* Register 0x411: AIF1 ADC2 High Pass Filters
	 * Enable HPF on both left and right ADC2 channels
	 * Voice mode fc=127Hz at 8kHz (but at 32kHz → ~508Hz)
	 * This removes DC offset from SDR mixer outputs */
	AUDIO_IO_Write(AUDIO_I2C_ADDRESS, 0x411, 0x3800);
	AUDIO_IO_Delay(2);
}

void clear_Output_Buffers(void)
{
	memset(out_buff, 0, BUFFERSIZE * sizeof(q15_t));
}

void start_duplex(void)
{
	// note, somehow there is a sneak path for setting the codec frequency see wmcodec for reference

	clear_Output_Buffers();
	BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_BOTH, Sample_Frequency, 16, 2);

	// Apply SDR-specific line-in customizations (differential I/Q, 0dB gain path)
	config_line_in();

	BSP_AUDIO_IN_Record((uint16_t *)&in_buff, BUFFERSIZE);
	BSP_AUDIO_OUT_Play((uint16_t *)&out_buff, 2 * BUFFERSIZE);
	NoOp;
}

void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
	// DSP_Flag=1: LSB=1 means second half ready (offset = BUFFERSIZE/2)
	DSP_Flag = 1;
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
	// DSP_Flag=2: LSB=0 means first half ready (offset = 0)
	DSP_Flag = 2;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
}

// static const float RMSConstant = 1.0 / 65485.0;
// static const long NCOphzinc = (long)(KCONV * (PI2 * LO_Freq / Sample_Frequency));

int frame_counter = 0;

void I2S2_RX_ProcessBuffer(uint16_t offset)
{
#if 0
	static long NCO_phz = 0;

	for (int i = 0; i < BUFFERSIZE / 4; i++)
	{
		NCO_phz += NCOphzinc;
		float temp = (q15_t)Sine_table[(NCO_phz >> 4) & 0xFFF] * RMSConstant;
		int index = i * 2 + offset;
		FIR_I_In[i] = (q15_t)(temp * in_buff[index]);
		FIR_Q_In[i] = (q15_t)(temp * in_buff[index + 1]);
	}

	Process_FIR_I_32K();
	Process_FIR_Q_32K();
#endif

	// Decimation
	uint8_t  decimator = 0;
	uint16_t ft8_pos = frame_counter * 256;
	for (int i = 0; i < BUFFERSIZE / 4; i++)
	{
		// USB_Out[i] = FIR_I_Out[i] - FIR_Q_Out[i];
		// LSB_Out[i] = FIR_I_Out[i] + FIR_Q_Out[i];
		USB_Out[i] = in_buff[offset + i * 2 + 0];
		LSB_Out[i] = in_buff[offset + i * 2 + 1];

		if (++decimator == 5) {
			decimator = 0;
			FT8_Data[ft8_pos++] = USB_Out[i];
		}

		int index = i * 2 + offset;
		out_buff[index] = USB_Out[i];
		out_buff[index + 1] = LSB_Out[i];
	}

	if (++frame_counter == 4)
	{
		process_FT8_FFT();
		frame_counter = 0;
	}
}
