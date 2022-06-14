## Introduction
This repository contains all the source code and binary for [Smart Access Platform Solution](https://www.nxp.com/design/designs/smart-access-platform-solution:SMART-ACCESS-PLATFORM). The platform intergrates various connecitivity and authentication options to showcase the NXP smart technologies for access solution.

### Main Control
The LPC55S69 is an ARM Cortex M33 based micro controller, running at a frequency of up to 150 MHz, supports Floating Point Unit (FPU) , includes up to 320 KB on-chip SRAM and up to 640 KB on-chip flash, provides up to 9 flexible communication interfaces (each of FlexComm 0-7 could be configured to be a USART, SPI, I2C, or I2S interface, FlexComm 8 is dedicated for High-Speed SPI), which is very good at to be a main control.

The folder 'lpc55s69-control' contains all the source code running on top of LPC55S69.
The flash address is 0x00000000 for bootloader and 0x00008000 for application.

### Android APK
The Android application would be used to manipulate the system remotely, such as 'add user', 'delete user', 'update user', etc.
The folder 'smartlockmanager' contains all the source code to implement the APK.

### Face Recognition
Smart Access Platform Solution leverages [SLN-VIZN3D-IOT](https://www.nxp.com/design/designs/nxp-edgeready-mcu-based-solution-for-3d-face-recognition:VIZN3D) to provide the 3D Face Recognition functionality.

###### Bootloader
The foloder 'vizn3d_bootloader' contains all the source code to boot 'SLN-VIZN3D-IOT kit' for application update.
The flash address is 0x30000000.


###### Application
The foloder 'sln_smart_lock' contains all the source code to implement face recognition.
The flash address is 0x30200000.

### Capacitive Touch Panel
The folder 'k16-touch' contains the binary running on KL16 for PinPad functionality.
The flash address is 0x00000000.

### Matter
The folder 'kw-matter' contains the binary running on K32W for Matter functionality based on Thread.
The flash address is 0x00000000.

### BLE & UWB
The folder 'qn9090_firmware' contains the binary running on QN9090 for BLE and UWB functionality.
The flash address is 0x00000000.
