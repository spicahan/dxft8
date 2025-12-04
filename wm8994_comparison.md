# WM8994 Input Configuration Comparison

## Key Differences for Line-In (SDR I/Q) Configuration

### Register 0x01 (Power Management 1)
**dxft8:** `0x0013` - Enables Microphone Bias 1 Generator + VMID
**mcu-ft8:** Not set for line-in (only for microphones)
**Effect:** Microphone bias is NOT needed for line-in inputs and can introduce noise/distortion

### Register 0x18 (Left Line Input 1&2 Volume)
**dxft8:** `0x008B` 
- Bit 7: IN1L_VU (Volume Update) = 1
- Bits 4-0: Volume = 0x0B (~+1.5dB with some DSP gain structure)
**mcu-ft8:** `0x000B`
- No volume update bit set
- Same base volume
**Effect:** Volume update synchronization

### Register 0x1A (Right Line Input 1&2 Volume)
**dxft8:** `0x018B` 
- Bit 8: IN1R_VU (Volume Update) = 1
- Bit 7: IN1R_ZC (Zero Cross) = 0
- Bits 4-0: Volume = 0x0B
**mcu-ft8:** `0x000B`
**Effect:** Similar to 0x18

### Register 0x28 (Input Mixer 3)
**dxft8:** `0x0011`
- IN1LN_TO_IN1L = 1 (negative input to left)
- IN1LP_TO_VMID = 0 (positive to VMID)  
- IN1RN_TO_IN1R = 1 (negative input to right)
- IN1RP_TO_VMID = 0 (positive to VMID)
**mcu-ft8:** NOT SET
**Effect:** **CRITICAL - Without this, inputs are not properly routed!**

### Register 0x29 (Input Mixer 4 - Left)
**dxft8:** `0x0020`
- IN1L_TO_MIXINL = 1 (connect IN1L to left mixer)
- IN1L_MIXINL_VOL = 0dB
**mcu-ft8:** NOT SET
**Effect:** **CRITICAL - Input not connected to ADC path!**

### Register 0x2A (Input Mixer 5 - Right) 
**dxft8:** `0x0020`
- IN1R_TO_MIXINR = 1 (connect IN1R to right mixer)
- IN1R_MIXINR_VOL = 0dB
**mcu-ft8:** NOT SET  
**Effect:** **CRITICAL - Input not connected to ADC path!**

### Register 0x04 (Power Management 4)
**dxft8:** `0x0F33`
- Enables AIF1ADC1L, AIF1ADC1R, AIF1ADC2L, AIF1ADC2R
- Enables ADCL, ADCR, AIF2ADCL, AIF2ADCR
**mcu-ft8:** Standard microphone config
**Effect:** Dual ADC timeslot configuration for I/Q

### Registers 0x400, 0x401, 0x404, 0x405 (AIF1 ADC1/ADC2 Volumes)
**dxft8:** All set to 0dB (`0x00C0`, `0x01C0`)
**mcu-ft8:** NOT SET (defaults may have gain)
**Effect:** Explicit 0dB digital gain vs. potentially boosted defaults

### Register 0x411 (AIF1 ADC2 High Pass Filters)
**dxft8:** `0x3800`
- HPF enabled for both L/R channels
- Voice mode fc=127Hz at 8kHz (but running at 32kHz so ~508Hz)
**mcu-ft8:** `0x1800` (if digital mic) or NOT SET
**Effect:** High-pass filter for DC blocking

## Summary of Effects

The dxft8 customizations create a **differential line-in configuration** suitable for SDR I/Q:

1. **No Microphone Bias** - Line inputs don't need bias voltage (would add noise)
2. **Proper Input Routing (0x28)** - Connects differential inputs: IN1LN/IN1LP and IN1RN/IN1RP as differential pairs
3. **Mixer Connections (0x29, 0x2A)** - Actually connects the inputs to the ADC path with 0dB gain
4. **Dual Timeslot ADC** - Uses both ADC1 and ADC2 timeslots for I and Q channels
5. **Explicit 0dB Gain** - Prevents the default +30dB microphone preamp gain
6. **HPF for DC Blocking** - Removes DC offset from SDR signals

**The current mcu-ft8 version is missing registers 0x28, 0x29, 0x2A which are CRITICAL for connecting the line inputs to the ADC path. This is why you're getting too much gain - the default microphone input configuration has ~30dB of preamp gain.**
