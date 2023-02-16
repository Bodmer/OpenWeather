// Sketch for ESP32, ESP8266, RP2040 Pico W, RP2040 Nano Connect
// it will run on a "bare" board ans reports via Serial messages.

// It fetches the Weather Forecast from OpenWeather and is
// an example from the library here:
// https://github.com/Bodmer/OpenWeather

// Sign up for a key and read API configuration info here:
// https://openweathermap.org/

// You can change the "User_Setup.h" file inside the OpenWeather
// to shows the data stream from the server

// Choose library to load
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#else // ESP32, Pico W, RP2040 Nano Connect
#include <WiFi.h>
#endif

#include <JSON_Decoder.h>

#include <OpenWeather.h>

// Just using this library for unix time conversion
#include <Time.h>

// =====================================================
// ========= User configured stuff starts here =========
// Further configuration settings can be found in the
// OpenWeather library "User_Setup.h" file

#define TIME_OFFSET 1UL * 3600UL // UTC + 0 hour

// Change to suit your WiFi router
#define WIFI_SSID     "Your_SSID"
#define WIFI_PASSWORD "Your_password"

// OpenWeather API Details, replace x's with your API key
String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Obtain this from your OpenWeather account

// Set both your longitude and latitude to at least 4 decimal places
String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "86.9250"; // 180.000 to -180.000 negative for West

String units = "metric";  // or "imperial"
String language = "en";   // See notes tab

// =========  User configured stuff ends here  =========
// =====================================================

OW_Weather ow; // Weather forecast library instance

void setup() {
  Serial.begin(250000); // Fast to stop it holding up the stream

  Serial.printf("\n\nConnecting to %s\n", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected\n");
}

void loop() {

  printForecast();
  // We can make 1000 requests a day
  delay(5 * 60 * 1000); // Every 5 minutes = 288 requests per day
}

/***************************************************************************************
**                          Send weather info to serial port
***************************************************************************************/
void printForecast()
{
  // Create the structures that hold the retrieved weather
  OW_forecast  *forecast = new OW_forecast;

  Serial.print("\nRequesting weather information from OpenWeather... ");

  ow.getForecast(forecast, api_key, latitude, longitude, units, language);

  Serial.println("Weather from OpenWeather\n");

  Serial.print("city_name           : "); Serial.println(forecast->city_name);
  Serial.print("sunrise             : "); Serial.println(strTime(forecast->sunrise));
  Serial.print("sunset              : "); Serial.println(strTime(forecast->sunset));
  Serial.print("Latitude            : "); Serial.println(ow.lat);
  Serial.print("Longitude           : "); Serial.println(ow.lon);
  Serial.print("Timezone            : "); Serial.println(forecast->timezone);
  Serial.println();

  if (forecast)
  {
    Serial.println("###############  Forecast weather  ###############\n");
    for (int i = 0; i < (MAX_DAYS * 8); i++)
    {
      Serial.print("3 hourly forecast   "); if (i < 10) Serial.print(" "); Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : "); Serial.print(strTime(forecast->dt[i]));

      Serial.print("temp             : "); Serial.println(forecast->temp[i]);
      Serial.print("temp.min         : "); Serial.println(forecast->temp_min[i]);
      Serial.print("temp.max         : "); Serial.println(forecast->temp_max[i]);

      Serial.print("pressure         : "); Serial.println(forecast->pressure[i]);
      Serial.print("sea_level        : "); Serial.println(forecast->sea_level[i]);
      Serial.print("grnd_level       : "); Serial.println(forecast->grnd_level[i]);
      Serial.print("humidity         : "); Serial.println(forecast->humidity[i]);

      Serial.print("clouds           : "); Serial.println(forecast->clouds_all[i]);
      Serial.print("wind_speed       : "); Serial.println(forecast->wind_speed[i]);
      Serial.print("wind_deg         : "); Serial.println(forecast->wind_deg[i]);
      Serial.print("wind_gust        : "); Serial.println(forecast->wind_gust[i]);

      Serial.print("visibility       : "); Serial.println(forecast->visibility[i]);
      Serial.print("pop              : "); Serial.println(forecast->pop[i]);
      Serial.println();

      Serial.print("dt_txt           : "); Serial.println(forecast->dt_txt[i]);
      Serial.print("id               : "); Serial.println(forecast->id[i]);
      Serial.print("main             : "); Serial.println(forecast->main[i]);
      Serial.print("description      : "); Serial.println(forecast->description[i]);
      Serial.print("icon             : "); Serial.println(forecast->icon[i]);

      Serial.println();
    }
  }
  // Delete to free up space and prevent fragmentation as strings change in length
  delete forecast;
}

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}
