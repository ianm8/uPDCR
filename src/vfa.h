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

#ifndef VFA_H
#define VFA_H

// Voice Frequency Announce
#define VFA_TESTS 0

namespace VFA
{
  #include "vfadata.h"

  #define WORD_POINT 16u
  #define WORD_MEGAHERTZ 17u

  const static uint32_t data_size = sizeof(data);

  static const struct
  {
    const uint32_t begin;
    const uint32_t end;
  }
  word[] =
  {
    {356ul,31294ul},     // zero
    {40669ul,65669ul},   // one
    {68794ul,93794ul},   // two
    {106294ul,131294ul}, // three
    {140669ul,165669ul}, // four
    {162544ul,193794ul}, // five
    {193794ul,225044ul}, // six
    {225044ul,253169ul}, // seven
    {250044ul,275044ul}, // eight
    {275044ul,300044ul}, // nine
    {300044ul,328169ul}, // ten
    {328169ul,356294ul}, // eleven
    {353169ul,384419ul}, // twelve
    {387544ul,410981ul}, // thirteen
    {412544ul,428169ul}, // fourteen
    {429731ul,453169ul}, // fifteen
    {456294ul,473481ul}, // point
    {473481ul,509419ul}  // megahertz
  };

  // n.nnn MHz
  volatile static uint32_t speak[6] = {0};
  volatile static uint32_t p_word = 0;
  volatile static uint32_t p_sample = 0;
  volatile static bool active = false;

  static void __not_in_flash_func(setFreq)(const uint32_t frequency)
  {
    if (active)
    {
      return;
    }
    const uint32_t MHz = frequency / 1000000ul;
    const uint32_t KHz = (frequency - MHz*1000000ul) / 1000;
    const uint32_t dig3 = KHz / 100ul;
    const uint32_t dig2 = (KHz - dig3*100ul) / 10ul;
    const uint32_t dig1 = KHz - dig3*100ul - dig2*10ul;
    speak[0] = MHz;
    speak[1] = WORD_POINT;
    speak[2] = dig3;
    speak[3] = dig2;
    speak[4] = dig1;
    speak[5] = WORD_MEGAHERTZ;
    p_word = 0;
    p_sample = word[speak[p_word]].begin;
    active = true;
  }

  static const int16_t __not_in_flash_func(announce)(void)
  {
    if (!active)
    {
      return 0;
    }
    if (p_sample>=data_size)
    {
      active = false;
      return 0;
    }
    const int16_t sample = (int16_t)data[p_sample++] - 128;
    if (p_sample>word[speak[p_word]].end)
    {
      p_word++;
      if (p_word>=6)
      {
        active = false;
      }
      else
      {
        p_sample = word[speak[p_word]].begin;
      }
    }
    return sample<<3;
  }

#if defined VFA_TESTS && VFA_TESTS==1
  static void __not_in_flash_func(init_test_1)(const uint32_t start = 0)
  {
    if (active)
    {
      return;
    }
    p_sample = start;
    p_word = 0;
    active = true;
  }

  static const int16_t __not_in_flash_func(run_test_1)(void)
  {
    // all data
    if (!active)
    {
      return 0;
    }
    if (p_sample>=data_size)
    {
      active = false;
      return 0;
    }
    const int16_t sample = (int16_t)data[p_sample++] - 128;
    return sample<<2;
  }

  static void __not_in_flash_func(init_test_2)(void)
  {
    if (active)
    {
      return;
    }
    p_word = 0;
    p_sample = word[p_word].begin;
    active = true;
  }

  static const int16_t __not_in_flash_func(run_test_2)(void)
  {
    // all words
    if (!active)
    {
      return 0;
    }
    if (p_word>WORD_MEGAHERTZ || p_sample>=data_size)
    {
      active = false;
      return 0;
    }
    const int16_t sample = (int16_t)data[p_sample++] - 128;
    if (p_sample>word[p_word].end)
    {
      p_word++;
      if (p_word>WORD_MEGAHERTZ)
      {
        active = false;
      }
      else
      {
        p_sample = word[p_word].begin;
      }
    }
    return sample<<2;
  }

  static void __not_in_flash_func(init_test_word)(const uint32_t the_word)
  {
    if (active)
    {
      return;
    }
    if (the_word>WORD_MEGAHERTZ)
    {
      active = false;
      return;
    }
    p_word = the_word;
    p_sample = word[p_word].begin;
    active = true;
  }

  static const int16_t __not_in_flash_func(run_test_word)(void)
  {
    // all words
    if (!active)
    {
      return 0;
    }
    if (p_word>WORD_MEGAHERTZ || p_sample>=data_size)
    {
      active = false;
      return 0;
    }
    const int16_t sample = (int16_t)data[p_sample++] - 128;
    if (p_sample>word[p_word].end)
    {
      active = false;
    }
    return sample<<2;
  }
#endif
}

#endif