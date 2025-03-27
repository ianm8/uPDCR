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

#ifndef DSP_H
#define DSP_H

#include "filter.h"

namespace DSP
{
  volatile static float agc_peak = 0.0f;

  static void __not_in_flash_func(mute)(void)
  {
    // set AGC to high value so that audio is temporarily muted
    static const float mute_value = 8192.0f;
    agc_peak = mute_value;
  }

  static const int16_t __not_in_flash_func(agc)(const float in)
  {
    // limit gain to max of 40 (32db)
    static const float max_gain = 40.0f;
    // about 10dB per second
    static const float k = 0.99996f;

    const float magnitude = fabsf(in);
    if (magnitude > agc_peak)
    {
      agc_peak = magnitude;
    }
    else
    {
      agc_peak *= k;
    }

    // trap issues with low values
    if (agc_peak<1.0f) return (int16_t)(in * max_gain);

    // set maximum gain possible for 12 bit DAC
    const float m = 2047.0f / agc_peak;
    return (int16_t)(in * fminf(m, max_gain));
  }

  static const uint32_t __not_in_flash_func(map)(const uint32_t x,const uint32_t in_min, const uint32_t in_max,const uint32_t out_min, const float out_max)
  {
    // unsigned map
    if (x<in_min)
    {
      return out_min;
    }
    if (x>in_max)
    {
      return out_max;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  static const uint32_t __not_in_flash_func(smeter)(void)
  {
    // assume S9 = 86 in 14 bits (35mv PP)
    volatile static uint32_t s = 0;
    volatile static uint32_t agc_update = 0;
    //static const float S0_sig = 30.0f; // not bad
    //static const float S0_sig = 5.0f; // too low, LED on all the time with antenna noise
    //static const float S0_sig = 10.0f; // too low, LED on all the time with antenna noise
    static const float S0_sig = 20.0f;
    static const float S9_sig = 86.0f;
    static const uint32_t sig_min = (uint32_t)(log10f(S0_sig) * 1024.0f);
    static const uint32_t sig_max = (uint32_t)(log10f(S9_sig) * 1024.0f);
    static const uint32_t led_min = 0ul;
    static const uint32_t led_max = 255ul;
    const uint32_t now = millis();
    if (now>agc_update)
    {
      agc_update = now + 20ul;
      s = 0;
      const float peak = agc_peak;
      if (peak>1.0f)
      {
        const uint32_t log_peak = (uint32_t)(log10f(peak) * 1024.0f);
        s = map(log_peak,sig_min,sig_max,led_min,led_max);
      }
    }
    return s;
  }

  static const int16_t __not_in_flash_func(process_ssb)(const float in_i,const float in_q)
  {
    const float ii = FILTER::dc1(in_i);
    const float qq = FILTER::dc2(in_q);

    // phase shift IQ +/- 45
    const float p45 = FILTER::ap1(ii);
    const float n45 = FILTER::ap2(qq);

    // reject image
    const float ssb = p45 - n45;

    // LPF
    const float audio = FILTER::lpf_2400(ssb);

    // AGC returns 12 bit value
    return agc(audio * 8192.0f);
  }

  static const int16_t __not_in_flash_func(process_cw)(const float in_i,const float in_q)
  {
    const float ii = FILTER::dc1(in_i);
    const float qq = FILTER::dc2(in_q);

    // phase shift IQ +/- 45
    const float p45 = FILTER::ap1(ii);
    const float n45 = FILTER::ap2(qq);

    // reject image
    const float ssb = p45 - n45;

    // BPF for CW
    const float audio = FILTER::bpf_700(ssb);

    // AGC returns 12 bit value
    return agc(audio * 8192.0f);
  }
}

#endif