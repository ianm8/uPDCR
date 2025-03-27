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

#ifndef FILTER_H
#define FILTER_H

#define FIR_LENGTH 256
#define __mac_tap(_h) acc += (_h)*x[i++]
#define __mac_zap(_h) i++
#define MA_FILTER_LENGTH 32u
#define MA_LOG2_LENGTH 5u
#define MA_FILTER_MASK (MA_FILTER_LENGTH-1u)

namespace FILTER
{
  static const float __not_in_flash_func(ma4fi)(const int16_t s)
  {
    // I channel
    // 4th order 16 pole moving average
    // (notch at 250,000/16 = 15625Hz)
    static float delay1[MA_FILTER_LENGTH] = { 0 };
    static float delay2[MA_FILTER_LENGTH] = { 0 };
    static float delay3[MA_FILTER_LENGTH] = { 0 };
    static float delay4[MA_FILTER_LENGTH] = { 0 };
    static float sum1 = 0;
    static float sum2 = 0;
    static float sum3 = 0;
    static float sum4 = 0;
    static uint8_t p = 0;
    const float v1 = (float)(s - 2048) / 2048.0f;
    sum1 = sum1 - delay1[p] + v1;
    delay1[p] = v1;
    const float v2 = sum1 / (float)MA_FILTER_LENGTH;
    sum2 = sum2 - delay2[p] + v2;
    delay2[p] = v2;
    const float v3 = sum2 / (float)MA_FILTER_LENGTH;
    sum3 = sum3 - delay3[p] + v3;
    delay3[p] = v3;
    const float v4 = sum3 / (float)MA_FILTER_LENGTH;
    sum4 = sum4 - delay4[p] + v4;
    delay4[p] = v4;
    p++;
    p &= MA_FILTER_MASK;
    return sum4 / (float)MA_FILTER_LENGTH;
  }

  static const float __not_in_flash_func(ma4fq)(const int16_t s)
  {
    // Q channel
    // 4th order 16 pole moving average
    // (notch at 250,000/16 = 15625Hz)
    static float delay1[MA_FILTER_LENGTH] = { 0 };
    static float delay2[MA_FILTER_LENGTH] = { 0 };
    static float delay3[MA_FILTER_LENGTH] = { 0 };
    static float delay4[MA_FILTER_LENGTH] = { 0 };
    static float sum1 = 0;
    static float sum2 = 0;
    static float sum3 = 0;
    static float sum4 = 0;
    static uint8_t p = 0;
    const float v1 = (float)(s - 2048) / 2048.0f;
    sum1 = sum1 - delay1[p] + v1;
    delay1[p] = v1;
    const float v2 = sum1 / (float)MA_FILTER_LENGTH;
    sum2 = sum2 - delay2[p] + v2;
    delay2[p] = v2;
    const float v3 = sum2 / (float)MA_FILTER_LENGTH;
    sum3 = sum3 - delay3[p] + v3;
    delay3[p] = v3;
    const float v4 = sum3 / (float)MA_FILTER_LENGTH;
    sum4 = sum4 - delay4[p] + v4;
    delay4[p] = v4;
    p++;
    p &= MA_FILTER_MASK;
    return sum4 / (float)MA_FILTER_LENGTH;
  }

  static float __not_in_flash_func(dc1)(const float in)
  {
    // single pole IIR high-pass filter
    //static const float k = 0.004f; // <100Hz
    //static const float k = 0.01f; // ~100Hz
    //static const float k = 0.1f; // ~300Hz
    static const float k = 0.05f; // ~200Hz
    static float s = 0;
    static float x1 = 0;
    static float y1 = 0;
    s -= x1;
    x1 = in;
    s += x1 - y1 * k;
    return (y1 = s);
  }

  static float __not_in_flash_func(dc2)(const float in)
  {
    // single pole IIR high-pass filter
    //static const float k = 0.004f; // <100Hz
    //static const float k = 0.01f; // ~100Hz
    //static const float k = 0.1f; // ~300Hz
    static const float k = 0.05f; // ~200Hz
    static float s = 0;
    static float x1 = 0;
    static float y1 = 0;
    s -= x1;
    x1 = in;
    s += x1 - y1 * k;
    return (y1 = s);
  }

  static const float __not_in_flash_func(ap1)(const float s)
  {
    // all pass 84 @ 31250
    // all pass 607 @ 31250
    // all pass 2539 @ 31250
    static const float k1 = 0.98325f;
    static const float k2 = 0.88497f;
    static const float k3 = 0.59331f;
    static float x1 = 0.0f;
    static float y1 = 0.0f;
    static float x2 = 0.0f;
    static float y2 = 0.0f;
    static float x3 = 0.0f;
    static float y3 = 0.0f;
    y1 = (k1 * (s + y1)) - x1;
    x1 = s;
    y2 = (k2 * (y1 + y2)) - x2;
    x2 = y1;
    y3 = (k3 * (y2 + y3)) - x3;
    x3 = y2;
    return y3;
  }

  static const float __not_in_flash_func(ap2)(const float s)
  {
    // all pass 8628 @ 31250
    // all pass 1200 @ 31250
    // all pass 287 @ 31250
    static const float k1 = 0.07102f;
    static const float k2 = 0.78470f;
    static const float k3 = 0.94391f;
    static float x1 = 0.0f;
    static float y1 = 0.0f;
    static float x2 = 0.0f;
    static float y2 = 0.0f;
    static float x3 = 0.0f;
    static float y3 = 0.0f;
    y1 = (k1 * (s + y1)) - x1;
    x1 = s;
    y2 = (k2 * (y1 + y2)) - x2;
    x2 = y1;
    y3 = (k3 * (y2 + y3)) - x3;
    x3 = y2;
    return y3;
  }

  static const float __not_in_flash_func(lpf_2400)(const float sample)
  {
    // 31250
    // 2400 Hz
    // att: 60dB
    // 255 taps
    static float x[FIR_LENGTH] = { 0.0f };
    static uint8_t sample_index = 0;
    uint8_t i = sample_index;
    x[sample_index--] = sample;
    float acc = 0;
    __mac_tap(-0.000051f);
    __mac_tap(-0.000052f);
    __mac_tap(-0.000039f);
    __mac_tap(-0.000011f);
    __mac_tap(0.000027f);
    __mac_tap(0.000067f);
    __mac_tap(0.000097f);
    __mac_tap(0.000109f);
    __mac_tap(0.000094f);
    __mac_tap(0.000051f);
    __mac_tap(-0.000013f);
    __mac_tap(-0.000086f);
    __mac_tap(-0.00015f);
    __mac_tap(-0.000186f);
    __mac_tap(-0.000181f);
    __mac_tap(-0.000129f);
    __mac_tap(-0.000036f);
    __mac_tap(0.00008f);
    __mac_tap(0.000194f);
    __mac_tap(0.000276f);
    __mac_tap(0.000301f);
    __mac_tap(0.000254f);
    __mac_tap(0.000137f);
    __mac_tap(-0.00003f);
    __mac_tap(-0.000211f);
    __mac_tap(-0.000364f);
    __mac_tap(-0.000446f);
    __mac_tap(-0.000428f);
    __mac_tap(-0.000302f);
    __mac_tap(-0.000087f);
    __mac_tap(0.000174f);
    __mac_tap(0.000424f);
    __mac_tap(0.000598f);
    __mac_tap(0.000646f);
    __mac_tap(0.000541f);
    __mac_tap(0.000292f);
    __mac_tap(-0.000054f);
    __mac_tap(-0.000424f);
    __mac_tap(-0.000729f);
    __mac_tap(-0.000889f);
    __mac_tap(-0.000849f);
    __mac_tap(-0.000599f);
    __mac_tap(-0.00018f);
    __mac_tap(0.000325f);
    __mac_tap(0.000799f);
    __mac_tap(0.001126f);
    __mac_tap(0.001212f);
    __mac_tap(0.001013f);
    __mac_tap(0.000552f);
    __mac_tap(-0.000085f);
    __mac_tap(-0.000758f);
    __mac_tap(-0.00131f);
    __mac_tap(-0.001595f);
    __mac_tap(-0.001522f);
    __mac_tap(-0.001077f);
    __mac_tap(-0.000335f);
    __mac_tap(0.000551f);
    __mac_tap(0.001379f);
    __mac_tap(0.001947f);
    __mac_tap(0.002095f);
    __mac_tap(0.001754f);
    __mac_tap(0.000964f);
    __mac_tap(-0.00012f);
    __mac_tap(-0.001264f);
    __mac_tap(-0.002198f);
    __mac_tap(-0.002681f);
    __mac_tap(-0.002561f);
    __mac_tap(-0.001821f);
    __mac_tap(-0.000587f);
    __mac_tap(0.000884f);
    __mac_tap(0.002259f);
    __mac_tap(0.003202f);
    __mac_tap(0.003455f);
    __mac_tap(0.002902f);
    __mac_tap(0.001615f);
    __mac_tap(-0.000157f);
    __mac_tap(-0.002029f);
    __mac_tap(-0.00356f);
    __mac_tap(-0.004361f);
    __mac_tap(-0.004184f);
    __mac_tap(-0.002996f);
    __mac_tap(-0.001001f);
    __mac_tap(0.001386f);
    __mac_tap(0.003629f);
    __mac_tap(0.005181f);
    __mac_tap(0.005619f);
    __mac_tap(0.004751f);
    __mac_tap(0.002682f);
    __mac_tap(-0.000192f);
    __mac_tap(-0.003251f);
    __mac_tap(-0.005781f);
    __mac_tap(-0.007138f);
    __mac_tap(-0.006902f);
    __mac_tap(-0.004996f);
    __mac_tap(-0.001735f);
    __mac_tap(0.00222f);
    __mac_tap(0.005989f);
    __mac_tap(0.008657f);
    __mac_tap(0.00949f);
    __mac_tap(0.008124f);
    __mac_tap(0.004679f);
    __mac_tap(-0.000221f);
    __mac_tap(-0.005549f);
    __mac_tap(-0.010079f);
    __mac_tap(-0.012649f);
    __mac_tap(-0.012438f);
    __mac_tap(-0.009194f);
    __mac_tap(-0.003348f);
    __mac_tap(0.00401f);
    __mac_tap(0.011309f);
    __mac_tap(0.0168f);
    __mac_tap(0.018929f);
    __mac_tap(0.016713f);
    __mac_tap(0.010029f);
    __mac_tap(-0.00024f);
    __mac_tap(-0.01226f);
    __mac_tap(-0.023497f);
    __mac_tap(-0.031128f);
    __mac_tap(-0.032559f);
    __mac_tap(-0.025935f);
    __mac_tap(-0.010561f);
    __mac_tap(0.012861f);
    __mac_tap(0.042231f);
    __mac_tap(0.074304f);
    __mac_tap(0.105149f);
    __mac_tap(0.130759f);
    __mac_tap(0.147685f);
    __mac_tap(0.1536f);
    __mac_tap(0.147685f);
    __mac_tap(0.130759f);
    __mac_tap(0.105149f);
    __mac_tap(0.074304f);
    __mac_tap(0.042231f);
    __mac_tap(0.012861f);
    __mac_tap(-0.010561f);
    __mac_tap(-0.025935f);
    __mac_tap(-0.032559f);
    __mac_tap(-0.031128f);
    __mac_tap(-0.023497f);
    __mac_tap(-0.01226f);
    __mac_tap(-0.00024f);
    __mac_tap(0.010029f);
    __mac_tap(0.016713f);
    __mac_tap(0.018929f);
    __mac_tap(0.0168f);
    __mac_tap(0.011309f);
    __mac_tap(0.00401f);
    __mac_tap(-0.003348f);
    __mac_tap(-0.009194f);
    __mac_tap(-0.012438f);
    __mac_tap(-0.012649f);
    __mac_tap(-0.010079f);
    __mac_tap(-0.005549f);
    __mac_tap(-0.000221f);
    __mac_tap(0.004679f);
    __mac_tap(0.008124f);
    __mac_tap(0.00949f);
    __mac_tap(0.008657f);
    __mac_tap(0.005989f);
    __mac_tap(0.00222f);
    __mac_tap(-0.001735f);
    __mac_tap(-0.004996f);
    __mac_tap(-0.006902f);
    __mac_tap(-0.007138f);
    __mac_tap(-0.005781f);
    __mac_tap(-0.003251f);
    __mac_tap(-0.000192f);
    __mac_tap(0.002682f);
    __mac_tap(0.004751f);
    __mac_tap(0.005619f);
    __mac_tap(0.005181f);
    __mac_tap(0.003629f);
    __mac_tap(0.001386f);
    __mac_tap(-0.001001f);
    __mac_tap(-0.002996f);
    __mac_tap(-0.004184f);
    __mac_tap(-0.004361f);
    __mac_tap(-0.00356f);
    __mac_tap(-0.002029f);
    __mac_tap(-0.000157f);
    __mac_tap(0.001615f);
    __mac_tap(0.002902f);
    __mac_tap(0.003455f);
    __mac_tap(0.003202f);
    __mac_tap(0.002259f);
    __mac_tap(0.000884f);
    __mac_tap(-0.000587f);
    __mac_tap(-0.001821f);
    __mac_tap(-0.002561f);
    __mac_tap(-0.002681f);
    __mac_tap(-0.002198f);
    __mac_tap(-0.001264f);
    __mac_tap(-0.00012f);
    __mac_tap(0.000964f);
    __mac_tap(0.001754f);
    __mac_tap(0.002095f);
    __mac_tap(0.001947f);
    __mac_tap(0.001379f);
    __mac_tap(0.000551f);
    __mac_tap(-0.000335f);
    __mac_tap(-0.001077f);
    __mac_tap(-0.001522f);
    __mac_tap(-0.001595f);
    __mac_tap(-0.00131f);
    __mac_tap(-0.000758f);
    __mac_tap(-0.000085f);
    __mac_tap(0.000552f);
    __mac_tap(0.001013f);
    __mac_tap(0.001212f);
    __mac_tap(0.001126f);
    __mac_tap(0.000799f);
    __mac_tap(0.000325f);
    __mac_tap(-0.00018f);
    __mac_tap(-0.000599f);
    __mac_tap(-0.000849f);
    __mac_tap(-0.000889f);
    __mac_tap(-0.000729f);
    __mac_tap(-0.000424f);
    __mac_tap(-0.000054f);
    __mac_tap(0.000292f);
    __mac_tap(0.000541f);
    __mac_tap(0.000646f);
    __mac_tap(0.000598f);
    __mac_tap(0.000424f);
    __mac_tap(0.000174f);
    __mac_tap(-0.000087f);
    __mac_tap(-0.000302f);
    __mac_tap(-0.000428f);
    __mac_tap(-0.000446f);
    __mac_tap(-0.000364f);
    __mac_tap(-0.000211f);
    __mac_tap(-0.00003f);
    __mac_tap(0.000137f);
    __mac_tap(0.000254f);
    __mac_tap(0.000301f);
    __mac_tap(0.000276f);
    __mac_tap(0.000194f);
    __mac_tap(0.00008f);
    __mac_tap(-0.000036f);
    __mac_tap(-0.000129f);
    __mac_tap(-0.000181f);
    __mac_tap(-0.000186f);
    __mac_tap(-0.00015f);
    __mac_tap(-0.000086f);
    __mac_tap(-0.000013f);
    __mac_tap(0.000051f);
    __mac_tap(0.000094f);
    __mac_tap(0.000109f);
    __mac_tap(0.000097f);
    __mac_tap(0.000067f);
    __mac_tap(0.000027f);
    __mac_tap(-0.000011f);
    __mac_tap(-0.000039f);
    __mac_tap(-0.000052f);
    __mac_tap(-0.000051f);
    return acc;
  }

  static const float __not_in_flash_func(bpf_700)(const float sample)
  {
    // 31250
    // att: 60dB
    // Lo: 600
    // Hi: 800
    // 255 taps
    static float x[FIR_LENGTH] = { 0.0f };
    static uint8_t sample_index = 0;
    uint8_t i = sample_index;
    x[sample_index--] = sample;
    float acc = 0;
    __mac_tap(0.000032f);
    __mac_tap(0.000029f);
    __mac_tap(0.000024f);
    __mac_tap(0.000015f);
    __mac_tap(0.000003f);
    __mac_tap(-0.000013f);
    __mac_tap(-0.000032f);
    __mac_tap(-0.000056f);
    __mac_tap(-0.000084f);
    __mac_tap(-0.000116f);
    __mac_tap(-0.00015f);
    __mac_tap(-0.000187f);
    __mac_tap(-0.000225f);
    __mac_tap(-0.000264f);
    __mac_tap(-0.000301f);
    __mac_tap(-0.000336f);
    __mac_tap(-0.000367f);
    __mac_tap(-0.000391f);
    __mac_tap(-0.000408f);
    __mac_tap(-0.000414f);
    __mac_tap(-0.000409f);
    __mac_tap(-0.000391f);
    __mac_tap(-0.000358f);
    __mac_tap(-0.00031f);
    __mac_tap(-0.000244f);
    __mac_tap(-0.000162f);
    __mac_tap(-0.000062f);
    __mac_tap(0.000054f);
    __mac_tap(0.000185f);
    __mac_tap(0.000329f);
    __mac_tap(0.000485f);
    __mac_tap(0.000648f);
    __mac_tap(0.000816f);
    __mac_tap(0.000984f);
    __mac_tap(0.001148f);
    __mac_tap(0.001302f);
    __mac_tap(0.001441f);
    __mac_tap(0.00156f);
    __mac_tap(0.001654f);
    __mac_tap(0.001717f);
    __mac_tap(0.001744f);
    __mac_tap(0.001731f);
    __mac_tap(0.001675f);
    __mac_tap(0.001572f);
    __mac_tap(0.00142f);
    __mac_tap(0.001219f);
    __mac_tap(0.000969f);
    __mac_tap(0.000672f);
    __mac_tap(0.000331f);
    __mac_tap(-0.000049f);
    __mac_tap(-0.000463f);
    __mac_tap(-0.000902f);
    __mac_tap(-0.001358f);
    __mac_tap(-0.001822f);
    __mac_tap(-0.002282f);
    __mac_tap(-0.002727f);
    __mac_tap(-0.003146f);
    __mac_tap(-0.003526f);
    __mac_tap(-0.003855f);
    __mac_tap(-0.004122f);
    __mac_tap(-0.004315f);
    __mac_tap(-0.004426f);
    __mac_tap(-0.004446f);
    __mac_tap(-0.004368f);
    __mac_tap(-0.004187f);
    __mac_tap(-0.003901f);
    __mac_tap(-0.003511f);
    __mac_tap(-0.003019f);
    __mac_tap(-0.002429f);
    __mac_tap(-0.00175f);
    __mac_tap(-0.000992f);
    __mac_tap(-0.000168f);
    __mac_tap(0.000706f);
    __mac_tap(0.001615f);
    __mac_tap(0.002538f);
    __mac_tap(0.003456f);
    __mac_tap(0.004347f);
    __mac_tap(0.005191f);
    __mac_tap(0.005965f);
    __mac_tap(0.006649f);
    __mac_tap(0.007224f);
    __mac_tap(0.007672f);
    __mac_tap(0.007977f);
    __mac_tap(0.008126f);
    __mac_tap(0.008109f);
    __mac_tap(0.00792f);
    __mac_tap(0.007556f);
    __mac_tap(0.007018f);
    __mac_tap(0.006311f);
    __mac_tap(0.005446f);
    __mac_tap(0.004435f);
    __mac_tap(0.003295f);
    __mac_tap(0.002047f);
    __mac_tap(0.000715f);
    __mac_tap(-0.000676f);
    __mac_tap(-0.002096f);
    __mac_tap(-0.003515f);
    __mac_tap(-0.004902f);
    __mac_tap(-0.006227f);
    __mac_tap(-0.00746f);
    __mac_tap(-0.00857f);
    __mac_tap(-0.009531f);
    __mac_tap(-0.010319f);
    __mac_tap(-0.010912f);
    __mac_tap(-0.011294f);
    __mac_tap(-0.011452f);
    __mac_tap(-0.011378f);
    __mac_tap(-0.011068f);
    __mac_tap(-0.010526f);
    __mac_tap(-0.009759f);
    __mac_tap(-0.008779f);
    __mac_tap(-0.007604f);
    __mac_tap(-0.006257f);
    __mac_tap(-0.004763f);
    __mac_tap(-0.003153f);
    __mac_tap(-0.00146f);
    __mac_tap(0.000282f);
    __mac_tap(0.002035f);
    __mac_tap(0.003763f);
    __mac_tap(0.005429f);
    __mac_tap(0.006996f);
    __mac_tap(0.008432f);
    __mac_tap(0.009704f);
    __mac_tap(0.010785f);
    __mac_tap(0.011652f);
    __mac_tap(0.012285f);
    __mac_tap(0.012671f);
    __mac_tap(0.0128f);
    __mac_tap(0.012671f);
    __mac_tap(0.012285f);
    __mac_tap(0.011652f);
    __mac_tap(0.010785f);
    __mac_tap(0.009704f);
    __mac_tap(0.008432f);
    __mac_tap(0.006996f);
    __mac_tap(0.005429f);
    __mac_tap(0.003763f);
    __mac_tap(0.002035f);
    __mac_tap(0.000282f);
    __mac_tap(-0.00146f);
    __mac_tap(-0.003153f);
    __mac_tap(-0.004763f);
    __mac_tap(-0.006257f);
    __mac_tap(-0.007604f);
    __mac_tap(-0.008779f);
    __mac_tap(-0.009759f);
    __mac_tap(-0.010526f);
    __mac_tap(-0.011068f);
    __mac_tap(-0.011378f);
    __mac_tap(-0.011452f);
    __mac_tap(-0.011294f);
    __mac_tap(-0.010912f);
    __mac_tap(-0.010319f);
    __mac_tap(-0.009531f);
    __mac_tap(-0.00857f);
    __mac_tap(-0.00746f);
    __mac_tap(-0.006227f);
    __mac_tap(-0.004902f);
    __mac_tap(-0.003515f);
    __mac_tap(-0.002096f);
    __mac_tap(-0.000676f);
    __mac_tap(0.000715f);
    __mac_tap(0.002047f);
    __mac_tap(0.003295f);
    __mac_tap(0.004435f);
    __mac_tap(0.005446f);
    __mac_tap(0.006311f);
    __mac_tap(0.007018f);
    __mac_tap(0.007556f);
    __mac_tap(0.00792f);
    __mac_tap(0.008109f);
    __mac_tap(0.008126f);
    __mac_tap(0.007977f);
    __mac_tap(0.007672f);
    __mac_tap(0.007224f);
    __mac_tap(0.006649f);
    __mac_tap(0.005965f);
    __mac_tap(0.005191f);
    __mac_tap(0.004347f);
    __mac_tap(0.003456f);
    __mac_tap(0.002538f);
    __mac_tap(0.001615f);
    __mac_tap(0.000706f);
    __mac_tap(-0.000168f);
    __mac_tap(-0.000992f);
    __mac_tap(-0.00175f);
    __mac_tap(-0.002429f);
    __mac_tap(-0.003019f);
    __mac_tap(-0.003511f);
    __mac_tap(-0.003901f);
    __mac_tap(-0.004187f);
    __mac_tap(-0.004368f);
    __mac_tap(-0.004446f);
    __mac_tap(-0.004426f);
    __mac_tap(-0.004315f);
    __mac_tap(-0.004122f);
    __mac_tap(-0.003855f);
    __mac_tap(-0.003526f);
    __mac_tap(-0.003146f);
    __mac_tap(-0.002727f);
    __mac_tap(-0.002282f);
    __mac_tap(-0.001822f);
    __mac_tap(-0.001358f);
    __mac_tap(-0.000902f);
    __mac_tap(-0.000463f);
    __mac_tap(-0.000049f);
    __mac_tap(0.000331f);
    __mac_tap(0.000672f);
    __mac_tap(0.000969f);
    __mac_tap(0.001219f);
    __mac_tap(0.00142f);
    __mac_tap(0.001572f);
    __mac_tap(0.001675f);
    __mac_tap(0.001731f);
    __mac_tap(0.001744f);
    __mac_tap(0.001717f);
    __mac_tap(0.001654f);
    __mac_tap(0.00156f);
    __mac_tap(0.001441f);
    __mac_tap(0.001302f);
    __mac_tap(0.001148f);
    __mac_tap(0.000984f);
    __mac_tap(0.000816f);
    __mac_tap(0.000648f);
    __mac_tap(0.000485f);
    __mac_tap(0.000329f);
    __mac_tap(0.000185f);
    __mac_tap(0.000054f);
    __mac_tap(-0.000062f);
    __mac_tap(-0.000162f);
    __mac_tap(-0.000244f);
    __mac_tap(-0.00031f);
    __mac_tap(-0.000358f);
    __mac_tap(-0.000391f);
    __mac_tap(-0.000409f);
    __mac_tap(-0.000414f);
    __mac_tap(-0.000408f);
    __mac_tap(-0.000391f);
    __mac_tap(-0.000367f);
    __mac_tap(-0.000336f);
    __mac_tap(-0.000301f);
    __mac_tap(-0.000264f);
    __mac_tap(-0.000225f);
    __mac_tap(-0.000187f);
    __mac_tap(-0.00015f);
    __mac_tap(-0.000116f);
    __mac_tap(-0.000084f);
    __mac_tap(-0.000056f);
    __mac_tap(-0.000032f);
    __mac_tap(-0.000013f);
    __mac_tap(0.000003f);
    __mac_tap(0.000015f);
    __mac_tap(0.000024f);
    __mac_tap(0.000029f);
    __mac_tap(0.000032f);
    return acc;
  }
}

#endif