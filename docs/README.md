# In this directory
Schematic and pics of the uPDCR.

# How to update the announce audio
I used Audacity to record and export the announce audio. In general, we need a single file with the numbers from 0 to 15 and the words "point" and "megahertz". The file should then be exported as raw, 8 bit, unsigned, and at a sample rate of 31250 Hz. Total file size should not exceed 2 MB.

1. set up Audacity to record from desktop audio: https://support.audacityteam.org/basics/recording-desktop-audio (if there is an export function then you don't need to do this just export the audio and open in Audacity)
2. use an online text-to-speech with the following text (I used https://www.naturalreaders.com/online/):
```
0

1

2

3

4

5

6

7

8

9

10

11

12

13

14

15

point

megahertz
```

3. Export or record the audio in Audacity

4. Open the recording in Audactiy

5. Use the export function (File -> Export Audio) with the following options:
* Format: Other uncompressed files
* Channels: Mono
* Sample Rate: 31250
* Header: RAW
* Encoding: Unsigned 8-bit PCM
* Export Range: Entire Project

6. Now use an online tool to convert to a C file structure. I used https://notisrac.github.io/FileToCArray/ with the following options:
* Code format: Hex (0x00)
* static: (checked)
* const: (checked)
* unsigned: (not checked)
* Data type: uint8_t
* PROGMEM: (not checked)

6. Copy the code into the vfadata.h file

7. Using Audacity, update the start and end values in the vfa.h file using the following formulae:

* start = start-time * 31250
* end = end-time * 31250

where start-time and end-time are in seconds, eg, if the word "fifteen" starts at 17.0 and ends at 18.0 then "start" will be at 17.0 * 31250 = 531250 and "end" will be at 18.0 * 31250 = 562500
