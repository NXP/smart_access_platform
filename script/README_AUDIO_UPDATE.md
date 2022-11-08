## Smart Access - LPC55S69 controller ##



## How to update audio binary to LPC55S69 board ##

MCU-Link or USB2UART dongle

Preinstalled python enviroment

Preinstalled below libraries into python

pip install pyserial
pip install pydub



**1. Connect the J8 on LPC55S69 board with MCU-Link's UART port or USB2UART dongle, refer below picture, YELLO wires are the TXD/RXD and GND connection fly wires.**

   ![Update connect](pictures\SmartAccessAudioUpdateCable.png)



**2. Downloaded lpc55s69 bootloader and application firmware to LPC55S69 board**

**3. Then open uart Terminal(Tera term or sscom).  remember set the Send with CR+LF(\r\n)**

reminder: for TeraTerm you may refer below settings, the "Transmit" item.

![Update connect](pictures\TeratemSettingforAudio.png)



**Then reset the board and get below messages if board working normally **

![Update connect](pictures\SystemResetLogs.png)



**Input Command "AUDIO" in the terminal window**

then you will see system feedback messages "Pls close this terminal run python scripty and select right COM port%"

![Update connect](pictures\InputAudioCMD.png)

**Closed Serial Terminal Window and run python script in folder "smartaccess_lpc55s69_audio_update" by "python py_generate_eng.py"**

Reminder: make sure the COM port setting in py script is match with your Usb VCOM port, refer **line71**

![Update connect](pictures\PythonCOMport.png)



Type below command into windows SHELL

"python py_generate_eng.py"

![Update connect](pictures\AudioRunPyCmd.png)

**If running python script well, you will get below messages**

The "Cnt X TimeOut\r\n" means receive audio data and this process will be running 35 time or more, usually 10-15mins for each audio file.

![Update connect](pictures\SuccesfullConnection.png)



Once the last piece of audio file updated, the messages information as below picture. script will automatically exit .

![Update connect](pictures\AudioUpdateFinished.png)

