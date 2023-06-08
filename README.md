# RP2040 Nano Connect, ESP8266 and ESP32 OpenWeather client

Arduino client library for https://openweathermap.org/

Collects current weather plus daily forecasts.

Requires the JSON parse library here:
https://github.com/Bodmer/JSON_Decoder

The OpenWeather_Forecast_Test example sketch sends collected data to the Serial port for API test. It does not not require a TFT screen and works with the Raspberry Pico W, RP2040 Nano Connect, ESP32 and ESP8266 processor boards. This example provides access to the weather data via a ser of variables, so could be adapted for use in weather related projects.

The TFT_eSPI_OpenWeather_LittleFS example works with the RP2040 Pico W, RP2040 Nano Connect, ESP32 and ESP8266. It uses LittleFS and displays the weather data on a TFT screen. This example uses the TFT_eSPI library.

The above examples will work with a free subscription to the OpenWeather service. The examples in the Onecall folder however require a subscription account (See OpenWeatherMap website for details).

The Raspberry Pico W and RP2040 Nano Connect must be used with Earle Philhower's board package:
https://github.com/earlephilhower/arduino-pico

These examples use anti-aliased fonts and newly created icons:

![Weather isons](https://i.imgur.com/luK7Vcj.jpg)

Latest screen grabs:

![TFT screenshot 1](https://i.imgur.com/ORovwNY.png)

