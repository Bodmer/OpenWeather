# RP2040 Nano Connect, ESP8266 and ESP32 OpenWeather client

Arduino client library for https://openweathermap.org/

Collects current weather plus daily forecasts.

Requires the JSON parse library here:
https://github.com/Bodmer/JSON_Decoder

The OpenWeather_Test example sketch sends collected data to the Serial port for API test. It does not not require a TFT screen and works with Raspberry Pico W, RP2040 Nano Connect, ESP32 and ESP8266.

The TFT_eSPI_OpenWeather_LittleFS example works with the RP2040 Nano Connect, ESP32 and ESP8266 and uses LittleFS, it displays the weather data on a TFT screen.

The TFT_eSPI_Weather example works with the ESP8266 and ESP32 only and uses SPIFFS, it displays the weather data on a TFT screen.

The Raspberry Pico W and RP2040 Nano Connect must be used with Earle Philhower's board package:
https://github.com/earlephilhower/arduino-pico

These examples use anti-aliased fonts and newly created icons:

![Weather isons](https://i.imgur.com/luK7Vcj.jpg)

Latest screen grabs:

![TFT screenshot 1](https://i.imgur.com/ORovwNY.png)

