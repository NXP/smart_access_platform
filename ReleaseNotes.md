# Smart Access Platform Solution Release Notes

## V1.2.0

**What's New**
1. Add audio prompt for face registration and recognition cases
2. Entend the UWB unlock & lock boundary from 90 ~ 110 to 80 ~ 120 cm for more robustness
3. Lock the door if UWB ranging signal lost longer than 3 seconds for safety
4. Align the LPC55S69 Application project settings between debug and release versions
5. Add fingerprint enrollment progress indication into APK
6. Keep BLE connection still alive after smart phone sleep
7. Optimize LPC factory reset function

**Known Limitation**
1. UWB application only support Xiaomi Mix 4 cell phone, Sunway and Finder V3 Tag
2. Face registration and recognition audio prompt volume is low compared to others
3. NFC application need to support UUID read out, not all smart phones allow that

## V1.1.0

**What's New**
1. Upgrade UWB SDK to improve the ranging performance
2. Standalone QN9090 firmware to support both wireless UART and UWB controlee functions

## V1.0.0

**What's New**
1. Initial release to showcase the various smart access options
