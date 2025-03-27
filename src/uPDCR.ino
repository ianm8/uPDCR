/*
 * uPDCR - Direct Conversion Receiver mk III
 *
 * Copyright (C) 2025 Ian Mitchell VK7IAN
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
 *
 * Version 0.9 2025-03-01
 * Version 1.0 2025-03-26
 *
 * Build:
 *  Board: Seeed XIAO RP2350
 *  Flash 2MB (no FS)
 *  CPU Speed: 240Mhz
 *  Optimize: -O2
 *  USB Stack: No USB
 */

#include <Wire.h>
#include "si5351.h"
#include "Rotary.h"
#include "filter.h"
#include "dsp.h"
#include "vfa.h"
#include "quad5351.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

// make sure si5351 library has been modified for 80m
#if SI5351_PLL_VCO_MIN != 440000000
#err set SI5351_PLL_VCO_MIN to 440000000 in si5351.h
#endif

#define PIN_SIG_I  A0   // GPIO 26
#define PIN_SIG_Q  A1   // GPIO 27
#define PIN_ENCA   D2   // GPIO 28
#define PIN_ENCB   D3   // GPIO 5
#define PIN_SDA    D4   // GPIO 6
#define PIN_SCL    D5   // GPIO 7
#define PIN_ENCBUT D6   // GPIO 0
#define PIN_VOL    D7   // GPIO 1
#define PIN_AUDH   D8   // GPIO 2
#define PIN_1LED   D9   // GPIO 4
#define PIN_AUDL   D10  // GPIO 3
#define SIG_MUX    0u   // A0, GPIO 26

#define DEFAULT_VOLUME    120u
#define DEFAULT_FREQUENCY 7100000ul
#define DEFAULT_STEP      1000ul
#define DEFAULT_BAND      BAND_40M
#define DEFAULT_MODE      MODE_LSB
#define DEFAULT_ZERO      0u
#define DEFAULT_AUTO_MODE true
#define VOLUME_STEP       5u
#define LONG_PRESS_TIME   1000u
#define MIN_FREQUENCY     3500000ul
#define MAX_FREQUENCY     15000000ul
#define MIN_VOL           80ul
#define MAX_VOL           255ul
#define TCXO_FREQ         25000000ul
#define VFA_DELAY         2000ul
#define TEST_5351         0
#define DEBUG_LED         0

enum band_t
{
  BAND_80M,
  BAND_40M,
  BAND_20M,
  NUM_BANDS
};

enum radio_mode_t
{
  MODE_LSB,
  MODE_USB,
  MODE_CWL,
  MODE_CWU
};

volatile static struct
{
  uint32_t frequency;
  uint32_t divisor;
  uint32_t tuning_step;
  uint32_t volume;
  radio_mode_t mode;
  bool auto_mode;
  band_t band;
  uint32_t band_frequency[NUM_BANDS];
}
radio =
{
  DEFAULT_FREQUENCY,
  DEFAULT_ZERO,
  DEFAULT_STEP,
  DEFAULT_VOLUME,
  DEFAULT_MODE,
  DEFAULT_AUTO_MODE,
  DEFAULT_BAND,
  {
    3600000ul,
    7100000ul,
    14100000ul
  }
};

Si5351 si5351;
Rotary r = Rotary(PIN_ENCA,PIN_ENCB);

volatile static int32_t dac_h = 0;
volatile static int32_t dac_l = 0;
volatile static uint32_t audio_pwm = 0;
volatile static float adc_value_i = 0;
volatile static float adc_value_q = 0;
volatile static bool adc_value_ready = false;
volatile static bool setup_complete = false;


void setup()
{
  // LED as active low
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
#if defined DEBUG_LED && DEBUG_LED==1
  for (int i=0;i<5;i++)
  {
    delay(250);
    digitalWrite(LED_BUILTIN,LOW);
    delay(50);
    digitalWrite(LED_BUILTIN,HIGH);
  }
  delay(5000);
#endif
  analogWriteResolution(256);
  analogWriteFreq(500000ul);
  pinMode(PIN_ENCBUT,INPUT_PULLUP);
  pinMode(PIN_ENCA,INPUT_PULLUP);
  pinMode(PIN_ENCB,INPUT_PULLUP);
  analogWrite(PIN_1LED,0);
  analogWrite(PIN_VOL,0);

  // set up audio out PWM
  gpio_set_function(PIN_AUDH,GPIO_FUNC_PWM);
  gpio_set_function(PIN_AUDL,GPIO_FUNC_PWM);
  audio_pwm = pwm_gpio_to_slice_num(PIN_AUDH);
  pwm_set_wrap(audio_pwm,63u); // 150,000,000 / 64 = 2,343,750
  pwm_set_both_levels(audio_pwm,31u,0u);
  pwm_set_enabled(audio_pwm,true);

  // set up MS5351M
  Wire1.setSDA(PIN_SDA);
  Wire1.setSCL(PIN_SCL);
  const bool si5351_found = si5351.init(SI5351_CRYSTAL_LOAD_0PF,TCXO_FREQ,0);
  if (!si5351_found)
  {
    for (;;)
    {
      digitalWrite(LED_BUILTIN,LOW);
      delay(25);
      digitalWrite(LED_BUILTIN,HIGH);
      delay(250);
    }
  }
  si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK1,SI5351_DRIVE_8MA);
  radio.divisor = get_divisor(radio.frequency);
  const uint64_t f = radio.frequency*SI5351_FREQ_MULT;
  const uint64_t p = radio.frequency*radio.divisor*SI5351_FREQ_MULT;
  si5351.set_freq_manual(f,p,SI5351_CLK0);
  si5351.set_freq_manual(f,p,SI5351_CLK1);
  si5351.set_phase(SI5351_CLK0,0);
  si5351.set_phase(SI5351_CLK1,radio.divisor);
  si5351.pll_reset(SI5351_PLLA);

#if defined TEST_5351 && TEST_5351==1
  for (;;)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
#endif

  // if button pressed at startup
  // then turn off auto mode
  if (digitalRead(PIN_ENCBUT)==LOW)
  {
    radio.auto_mode = false;
    delay(50);
    while (digitalRead(PIN_ENCBUT)==LOW)
    {
      delay(50);
    }
    delay(50);
  }
  r.begin();
  init_adc();
  analogWrite(PIN_VOL,radio.volume);
  setup_complete = true;
}

void setup1()
{
  // wait for setup() to complete
  while (!setup_complete)
  {
    // setup() not done yet
    tight_loop_contents();
  }
#if defined DEBUG_LED && DEBUG_LED==1
  pinMode(LED_BUILTIN,OUTPUT);
  for (int i=0;i<5;i++)
  {
    digitalWrite(LED_BUILTIN,LOW);
    delay(50);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
  }
#endif
}

void __not_in_flash_func(adc_interrupt_handler)(void)
{
  // note, FIFO depth of 8 caused random
  // channel swapping causing the radio
  // to switch between LSB and USB
  // this is probably due to buffer overflow
  // in the FIFO caused by background interrupts
  volatile static uint32_t counter = 0;
  volatile static float adc_raw_i = 0;
  volatile static float adc_raw_q = 0;
  if (adc_fifo_get_level()<4u)
  {
    return;
  }
  volatile const uint16_t i0 = adc_fifo_get();
  volatile const uint16_t q0 = adc_fifo_get();
  volatile const uint16_t i1 = adc_fifo_get();
  volatile const uint16_t q1 = adc_fifo_get();
  adc_raw_i += FILTER::ma4fi(i0);
  adc_raw_q += FILTER::ma4fq(q0);
  adc_raw_i += FILTER::ma4fi(i1);
  adc_raw_q += FILTER::ma4fq(q1);
  if (counter==4)
  {
    pwm_set_both_levels(audio_pwm,dac_h,dac_l);
    // 8 times oversampling per channel
    adc_value_i = adc_raw_i / 8.0f;
    adc_value_q = adc_raw_q / 8.0f;
    adc_value_ready = true;
    adc_raw_i = 0;
    adc_raw_q = 0;
    counter = 0;
  }
  counter++;
}

void init_adc(void)
{
  adc_init();
  adc_gpio_init(PIN_SIG_I);
  adc_gpio_init(PIN_SIG_Q);
  adc_select_input(SIG_MUX);
  adc_set_round_robin(0b00000011);
  adc_fifo_setup(true, false, 4, false, false);
  adc_fifo_drain();
  adc_irq_set_enabled(true);
  irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_interrupt_handler);
  irq_set_priority(ADC_IRQ_FIFO, PICO_HIGHEST_IRQ_PRIORITY);
  irq_set_enabled(ADC_IRQ_FIFO, true);
  adc_run(true);
}

void __not_in_flash_func(loop)(void)
{
  // run DSP on core 0
  if (adc_value_ready)
  {
    adc_value_ready = false;
    int32_t rx_value = 0;
////
    switch (radio.mode)
    {
      case MODE_LSB: rx_value = (int32_t)DSP::process_ssb(adc_value_i,adc_value_q); break;
      case MODE_USB: rx_value = (int32_t)DSP::process_ssb(adc_value_q,adc_value_i); break;
      case MODE_CWL: rx_value = (int32_t)DSP::process_cw(adc_value_i,adc_value_q);  break;
      case MODE_CWU: rx_value = (int32_t)DSP::process_cw(adc_value_q,adc_value_i);  break;
    }
    if (VFA::active)
    {
      rx_value = (rx_value>>4) + VFA::announce();
    }
    const int32_t dac_audio = constrain(rx_value,-2048l,+2047l)+2048l;
    dac_h = dac_audio >> 6;
    dac_l = dac_audio & 0x3f;
  }
}

void __not_in_flash_func(loop1)(void)
{
  // remember the current frequency and button state
  static uint32_t current_frequency = 0;
  static uint32_t current_divisor = 0;
  static uint32_t button_start_time = 0;
  static uint32_t vfa_announce_time = 0;
  static uint32_t current_vfa_frequency = 0;
  static uint32_t new_vfa_frequency = 0;
  static bool vfa_announce = false;

  // update volume and LED smeter
  analogWrite(PIN_VOL,radio.volume);
  analogWrite(PIN_1LED,DSP::smeter());
    
  // what's the rotary encoder doing?
  const uint8_t rotary = r.process();

  volatile static enum
  {
    STATE_TUNING,
    STATE_BUTTON_PRESS,
    STATE_VOLUME,
    STATE_WAIT_RELEASE
  } state = STATE_TUNING;

  switch (state)
  {
    case STATE_TUNING:
    {
      // tuning
      switch (rotary)
      {
        case DIR_CW:
        {
          radio.frequency += radio.tuning_step - radio.frequency%radio.tuning_step;
          radio.frequency = constrain(radio.frequency,MIN_FREQUENCY,MAX_FREQUENCY);
          break;
        }
        case DIR_CCW:
        {
          const uint32_t modula = radio.frequency%radio.tuning_step;
          radio.frequency -= (modula==0)?radio.tuning_step:modula;
          radio.frequency = constrain(radio.frequency,MIN_FREQUENCY,MAX_FREQUENCY);
          break;
        }
      }
      if (digitalRead(PIN_ENCBUT)==LOW)
      {
        button_start_time = millis();
        state = STATE_BUTTON_PRESS;
      }
      break;
    }
    case STATE_BUTTON_PRESS:
    {
      const uint32_t press_time = millis()-button_start_time;
      if (press_time>LONG_PRESS_TIME)
      {
        // change the band and wait for button release
        band_t old_band = radio.band;
        switch (radio.band)
        {
          case BAND_80M: radio.band = BAND_40M; break;
          case BAND_40M: radio.band = BAND_20M; break;
          case BAND_20M: radio.band = BAND_80M; break;
        }
        radio.band_frequency[old_band] = radio.frequency;
        radio.frequency = radio.band_frequency[radio.band];
        state = STATE_WAIT_RELEASE;
        break;
      }
      switch (rotary)
      {
        case DIR_CW:  radio.volume += VOLUME_STEP; state = STATE_VOLUME; break;
        case DIR_CCW: radio.volume -= VOLUME_STEP; state = STATE_VOLUME; break;
      }
      if (digitalRead(PIN_ENCBUT)==HIGH)
      {
        // change step
        switch (radio.tuning_step)
        {
          case 1000: radio.tuning_step = 100;  break;
          case 100:  radio.tuning_step = 10;   break;
          case 10:   radio.tuning_step = 1000; break;
        }
        state = STATE_WAIT_RELEASE;
      }
      break;
    }
    case STATE_VOLUME:
    {
      switch (rotary)
      {
        case DIR_CW:  radio.volume += VOLUME_STEP; break;
        case DIR_CCW: radio.volume -= VOLUME_STEP; break;
      }
      radio.volume = constrain(radio.volume,MIN_VOL,MAX_VOL);
      if (digitalRead(PIN_ENCBUT)==HIGH)
      {
        state = STATE_WAIT_RELEASE;
      }
      break;
    }
    case STATE_WAIT_RELEASE:
    {
      if (digitalRead(PIN_ENCBUT)==HIGH)
      {
        // debounce
        button_start_time = 0;
        state = STATE_TUNING;
        delay(50);
      }
    }
  }

  // frequency changed?
  if (radio.frequency != current_frequency)
  {
    // update the frequency
    current_frequency = radio.frequency;
    new_vfa_frequency = radio.frequency / 1000ul;
    radio.divisor = get_divisor(current_frequency);
    const uint64_t f = current_frequency * SI5351_FREQ_MULT;
    const uint64_t p = current_frequency * radio.divisor * SI5351_FREQ_MULT;
    si5351.set_freq_manual(f,p,SI5351_CLK0);
    si5351.set_freq_manual(f,p,SI5351_CLK1);
    if (current_divisor != radio.divisor)
    {
      // if the divisor changes, reset the PLL
      current_divisor = radio.divisor;
      DSP::mute();
      si5351.set_phase(SI5351_CLK0,0);
      si5351.set_phase(SI5351_CLK1,radio.divisor);
      si5351.pll_reset(SI5351_PLLA);
    }

    // reset announce time for any change
    vfa_announce_time = millis() + VFA_DELAY;

    // update the mode
    if (radio.auto_mode)
    {
      volatile radio_mode_t new_mode = MODE_LSB;
      if (radio.frequency < 10000000ul)
      {
        if ((radio.frequency >= 3500000ul && radio.frequency <= 3560000ul) ||
          (radio.frequency >= 7000000ul && radio.frequency <= 7060000ul))
        {
          new_mode = MODE_CWL;
        }
      }
      else
      {
        new_mode = MODE_USB;
        if (radio.frequency >= 14000000ul && radio.frequency <= 14060000ul)
        {
          new_mode = MODE_CWU;
        }
      }
      if (radio.mode != new_mode)
      {
        // only update the mode if it has changed
        radio.mode = new_mode;
        if (new_mode==MODE_LSB || new_mode==MODE_USB)
        {
          // changed to SSB
          radio.tuning_step = 1000u;
        }
        else if (radio.tuning_step==1000u)
        {
          // changed to CW
          radio.tuning_step = 100u;
        }
      }
    }
    else
    {
      // just update SSB
      volatile radio_mode_t new_mode = MODE_USB;
      if (radio.frequency < 10000000ul)
      {
        new_mode = MODE_LSB;
      }
      if (radio.mode != new_mode)
      {
        // only update the mode if it has changed
        radio.mode = new_mode;
      }
    }
  }

  // VFA processing
  if (current_vfa_frequency != new_vfa_frequency)
  {
    current_vfa_frequency = new_vfa_frequency;
    vfa_announce = true;
  }

  // announce frequency
  if (vfa_announce)
  {
    if (millis() > vfa_announce_time)
    {
      vfa_announce = false;
      VFA::setFreq(radio.frequency);
    }
  }
}