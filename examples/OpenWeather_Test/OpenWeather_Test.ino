// Sketch for ESP32 to fetch the Weather Forecast from OpenWeather
// an example from the library here:
// https://github.com/Bodmer/OpenWeather

// Sign up for a key and read API configuration info here:
// https://openweathermap.org/

// You can change the number of hours and days for the forecast in the
// "User_Setup.h" file inside the OpenWeather library folder.
// By default this is 6 hours (can be up to 48) and 5 days
// (can be up to 8 days = today plus 7 days)

// Choose library to load
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
#else // ESP32
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

  Serial.printf("Connecting to %s\n", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected\n");
}

void loop() {

  printCurrentWeather();

  // We can make 1000 requests a day
  delay(5 * 60 * 1000); // Every 5 minutes = 288 requests per day
}

/***************************************************************************************
**                          Send weather info to serial port
***************************************************************************************/
void printCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  OW_current *current = new OW_current;
  OW_hourly *hourly = new OW_hourly;
  OW_daily  *daily = new OW_daily;

  //time_t time;

  Serial.print("\nRequesting weather information from OpenWeather... ");

  ow.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);

  Serial.println("Weather from Open Weather\n");

  // We can use the timezone to set the offset eventually...
  // Serial.print("Timezone            : "); Serial.println(current->timezone);
  
  Serial.println("############### Current weather ###############\n");
  Serial.print("dt (time)        : "); Serial.print(strTime(current->dt));
  Serial.print("sunrise          : "); Serial.print(strTime(current->sunrise));
  Serial.print("sunset           : "); Serial.print(strTime(current->sunset));
  Serial.print("temp             : "); Serial.println(current->temp);
  Serial.print("feels_like       : "); Serial.println(current->feels_like);
  Serial.print("pressure         : "); Serial.println(current->pressure);
  Serial.print("humidity         : "); Serial.println(current->humidity);
  Serial.print("dew_point        : "); Serial.println(current->dew_point);
  Serial.print("uvi              : "); Serial.println(current->uvi);
  Serial.print("clouds           : "); Serial.println(current->clouds);
  Serial.print("visibility       : "); Serial.println(current->visibility);
  Serial.print("wind_speed       : "); Serial.println(current->wind_speed);
  Serial.print("wind_gust        : "); Serial.println(current->wind_gust);
  Serial.print("wind_deg         : "); Serial.println(current->wind_deg);
  Serial.print("rain             : "); Serial.println(current->rain);
  Serial.print("snow             : "); Serial.println(current->snow);
  Serial.println();
  Serial.print("id               : "); Serial.println(current->id);
  Serial.print("main             : "); Serial.println(current->main);
  Serial.print("description      : "); Serial.println(current->description);
  Serial.print("icon             : "); Serial.println(current->icon); //Serial.println(getMeteoconIcon(current->icon));

  Serial.println();

  Serial.println("############### Hourly weather  ###############\n");
  for (int i = 0; i<MAX_HOURS; i++)
  {
    Serial.print("Hourly summary  "); if (i<10) Serial.print(" "); Serial.print(i);
    Serial.println();
    Serial.print("dt (time)        : "); Serial.print(strTime(hourly->dt[i]));
    Serial.print("temp             : "); Serial.println(hourly->temp[i]);
    Serial.print("feels_like       : "); Serial.println(hourly->feels_like[i]);
    Serial.print("pressure         : "); Serial.println(hourly->pressure[i]);
    Serial.print("humidity         : "); Serial.println(hourly->humidity[i]);
    Serial.print("dew_point        : "); Serial.println(hourly->dew_point[i]);
    Serial.print("clouds           : "); Serial.println(hourly->clouds[i]);
    Serial.print("wind_speed       : "); Serial.println(hourly->wind_speed[i]);
    Serial.print("wind_gust        : "); Serial.println(hourly->wind_gust[i]);
    Serial.print("wind_deg         : "); Serial.println(hourly->wind_deg[i]);
    Serial.print("rain             : "); Serial.println(hourly->rain[i]);
    Serial.print("snow             : "); Serial.println(hourly->snow[i]);
    Serial.println();
    Serial.print("id               : "); Serial.println(hourly->id[i]);
    Serial.print("main             : "); Serial.println(hourly->main[i]);
    Serial.print("description      : "); Serial.println(hourly->description[i]);
    Serial.print("icon             : "); Serial.println(hourly->icon[i]);

    Serial.println();
  }

  Serial.println("###############  Daily weather  ###############\n");
  for (int i = 0; i<MAX_DAYS; i++)
  {
    Serial.print("Daily summary   "); if (i<10) Serial.print(" "); Serial.print(i);
    Serial.println();
    Serial.print("dt (time)        : "); Serial.print(strTime(daily->dt[i]));
    Serial.print("sunrise          : "); Serial.print(strTime(daily->sunrise[i]));
    Serial.print("sunset           : "); Serial.print(strTime(daily->sunset[i]));

    Serial.print("temp.morn        : "); Serial.println(daily->temp_morn[i]);
    Serial.print("temp.day         : "); Serial.println(daily->temp_day[i]);
    Serial.print("temp.eve         : "); Serial.println(daily->temp_eve[i]);
    Serial.print("temp.night       : "); Serial.println(daily->temp_night[i]);
    Serial.print("temp.min         : "); Serial.println(daily->temp_min[i]);
    Serial.print("temp.max         : "); Serial.println(daily->temp_max[i]);
    
    Serial.print("feels_like.morn  : "); Serial.println(daily->feels_like_morn[i]);
    Serial.print("feels_like.day   : "); Serial.println(daily->feels_like_day[i]);
    Serial.print("feels_like.eve   : "); Serial.println(daily->feels_like_eve[i]);
    Serial.print("feels_like.night : "); Serial.println(daily->feels_like_night[i]);

    Serial.print("pressure         : "); Serial.println(daily->pressure[i]);
    Serial.print("humidity         : "); Serial.println(daily->humidity[i]);
    Serial.print("dew_point        : "); Serial.println(daily->dew_point[i]);
    Serial.print("uvi              : "); Serial.println(daily->uvi[i]);
    Serial.print("clouds           : "); Serial.println(daily->clouds[i]);
    Serial.print("visibility       : "); Serial.println(daily->visibility[i]);
    Serial.print("wind_speed       : "); Serial.println(daily->wind_speed[i]);
    Serial.print("wind_gust        : "); Serial.println(daily->wind_gust[i]);
    Serial.print("wind_deg         : "); Serial.println(daily->wind_deg[i]);
    Serial.print("rain             : "); Serial.println(daily->rain[i]);
    Serial.print("snow             : "); Serial.println(daily->snow[i]);
    Serial.println();
    Serial.print("id               : "); Serial.println(daily->id[i]);
    Serial.print("main             : "); Serial.println(daily->main[i]);
    Serial.print("description      : "); Serial.println(daily->description[i]);
    Serial.print("icon             : "); Serial.println(daily->icon[i]);

    Serial.println();
  }

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete hourly;
  delete daily;
}

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}

/***************************************************************************************
**                          Get the icon file name from the index number
***************************************************************************************/
const char* getMeteoconIcon(uint8_t index)
{
  if (index > MAX_ICON_INDEX) index = 0;
  return ow.iconName(index);
}
