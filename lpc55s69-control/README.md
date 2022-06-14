## Smart Access - LPC55S69 controller ##



## Tools ##

MCU-Link

USB2UART dongle



## SCR ##


### Helix DNA ###
This Helix DNA MP3 decode source flow the license
 Copyright (c) 1995-2004 RealNetworks, Inc. All Rights Reserved.  
        
 The contents of this directory, and (except where otherwise
 indicated) the directories included within this directory, are
 subject to the current version of the RealNetworks Public Source
 License (the "RPSL") available at RPSL.txt in this directory, unless
 you have licensed the directory under the current version of the
 RealNetworks Community Source License (the "RCSL") available at
 RCSL.txt in this directory, in which case the RCSL will apply. You
 may also obtain the license terms directly from RealNetworks.  You
 may not use the files in this directory except in compliance with the
 RPSL or, if you have a valid RCSL with RealNetworks applicable to
 this directory, the RCSL.  Please see the applicable RPSL or RCSL for
 the rights, obligations and limitations governing use of the contents
 of the directory.

 This directory is part of the Helix DNA Technology. RealNetworks is
 the developer of the Original Code and owns the copyrights in the
 portions it created.

 This directory, and the directories included with this directory, are
 distributed and made available on an 'AS IS' basis, WITHOUT WARRANTY
 OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY
 DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 QUIET ENJOYMENT OR NON-INFRINGEMENT.

 Technology Compatibility Kit Test Suite(s) Location:  
    http://www.helixcommunity.org/content/tck  



### MULTIBUTTON ###

[MultiButton/LICENSE at master · 0x1abin/MultiButton · GitHub](https://github.com/0x1abin/MultiButton/blob/master/LICENSE)

![multibutton license](pictures\LICENSE_MULTIBUTTON.png)



## How to Update MP3 bin file to LPC55

1. download lpc55s69 2nd bootloader and application to the board

2. connected J8(uart txd/rxd/gnd) with usb2uart dongle

3. open uart terminal 

   ![multibutton license](pictures\lpc55_auido_update.jpg)

4. run python script in folder "smartaccess_lpc55s69_audio_update" by "python py_generate_eng.py"

5. if serial port not correct, please change serial port number here, in python script line 71 

   ser.port = "COM10"

6. if everything goes well, this will update the bin file 30 times or more, usually 10-15mins.

