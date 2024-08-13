# steam-hardware-authenticator
Generate Steam Guard tokens with an esp8266 driven SSD1306 OLED.

![alt text](https://raw.githubusercontent.com/ran-sama/steam_hardware_authenticator/master/images/flavours.png)
![alt text](https://raw.githubusercontent.com/ran-sama/steam_hardware_authenticator/master/images/on_battery.jpg)
![alt text](https://raw.githubusercontent.com/ran-sama/steam_hardware_authenticator/master/images/ds3231_wire_setup.jpg)
![alt text](https://raw.githubusercontent.com/ran-sama/steam_hardware_authenticator/master/images/setup_behind_screen.jpg)

## Dependencies
```
SHA1-HMAC for esp8266: https://github.com/bbx10/Cryptosuite
U8g2_Arduino (2.20.7 or newer): https://github.com/olikraus/U8g2_Arduino
Only if using the RTC version: https://github.com/adafruit/RTClib
```

## Extract your Steam Guard key
```
adb devices
adb root
adb pull /data/data/com.valvesoftware.android.steam.community/shared_prefs/steam.uuid.xml
adb pull /data/data/com.valvesoftware.android.steam.community/files/SteamGuard-*
adb kill-server
```

## Getting it running on your end
Convert the BASE32 to HEX:
```
ran@gensokyo:~ $ python3
Python 3.5.3 (default, Jan 19 2017, 14:11:04)
[GCC 6.3.0 20170124] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import base64
>>> base64.b32decode('E743CHKDEA44APEGLSDIUT2N4OY5NXTQ').hex()
'27f9b11d432039c03c865c868a4f4de3b1d6de70'
>>>
```

Edit the following char array in the source file with your key, example below:
```
char key[] = { 0x27, 0xf9, 0xb1, 0x1d, 0x43, 0x20, 0x39, 0xc0, 0x3c, 0x86, 0x5c, 0x86, 0x8a, 0x4f, 0x4d, 0xe3, 0xb1, 0xd6, 0xde, 0x70 };
```

Please change the NTP server to a reliable one in your own country:
```
WiFi.hostByName("ntp.nict.jp", timeServerIP);
```

Make sure you can connect to your Wifi:
```
char ssid[] = "mySSID";
char pass[] = "myWifiPass";
```
Edit these if you use different pins or lack a reset pin (use U8X8_PIN_NONE in the constructor):
```
#define OLED_SDA  2
#define OLED_SCL 14
#define OLED_RST  4
```

## Future Authenticator?
A new branch using different OLED drivers and Brzo I2C, faster, better and improved time code!
```
Dependencies for future branch are:
https://github.com/bbx10/Cryptosuite
https://github.com/pasko-zh/brzo_i2c
https://github.com/ThingPulse/esp8266-oled-ssd1306
```

![alt text](https://raw.githubusercontent.com/ran-sama/steam_hardware_authenticator/master/images/future_auth.png)

## Notes on compiling for the ESP32
Don't do this if you use the ESP8266. If compiling doesn't work, put comment slashes in front of these two lines inside sha1.cpp and sha256.cpp:
```
#include <avr/io.h>
#include <avr/pgmspace.h>
```

## FAQ
```
Q: What hardware do I need?
A: Easiest for you is the "Wemos TTGO ESP8266 0.91 Inch OLED For Arduino Nodemcu".
But any other esp8266 NodeMCU and SSD1306 OLED screen should be fine.

Q: Is that your key?
A: Why yes it is, but the account has no games. I never used it for anything, but coding this project.

Q: Is this safe?
A: It can be stolen like any other device with your tokens. Best is to keep it at home.

Q: Which version should I choose and can I use other RTCs than the DS3231?
A: If you really must carry the device with you in places without wifi, pick the RTC version.
The code can be edited for most RTCs to work, yet the DS3231 merely costs ~$1.83 (free shipping)
and offers a TCXO (temperature compensated crystal oscillator) making it more accurate than a DS1307.

Q: What can't I do with it and anything I should know?
A: You cannot authorize trades with it, but think of it as a security feature.
Also the font I use prints the Q similar to an O or a 0, but there is only the Q by design in tokens.
Feel free to explore the U8glib V2 library, it comes with an amazing amount of nice looking fonts.

Q: Won't the millis() roll over at one point?
A: They will, but it is safe for at least 13 days of continued operation.
You can always push the reboot button.

Please see: https://github.com/esp8266/Arduino/issues/3600#issuecomment-332174397
```

## License
Licensed under the WTFPL license.
