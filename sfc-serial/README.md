To compile this code, load "sfc-serial.ino" into the Arduino IDE.

This code is designed specifically for the Raspberry Pi RP2040 microcontroller.

This code also assumes you're using the following board library:
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

Go to File > Preferences, and click the button next to "Additional Board Manager URLs",
and then add the URL listed above to the list. After that, go to Tools > Board > Board Manager,
then search for and install "Raspberry Pi Pico/RP2040". You will then need to go to Tools > Board > and then select the specific RP2040 board that you're using. In my case, I've tested this code
with the "Waveshare RP2040 Zero" board. It will probably work on other boards too, but I
offer no promises or guarantees.