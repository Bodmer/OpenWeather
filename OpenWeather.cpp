// Client library for the OpenWeather data-point server
// https://openweathermap.org/

// Created by Bodmer 09/04/2020
// Updated by Bodmer 08/01/2021
// Updated by Bodmer 15/02/2023 to support free forecast API

// See license.txt in root folder of library
// Insecure mode added by ADAMSIN12

#if defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)
  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    #include <WiFi.h>
  #else
    #include <WiFiNINA.h>
  #endif
#else
  #ifdef ESP8266
    #include <ESP8266WiFi.h>
  #else
    #include <WiFi.h>
  #endif
  #include <WiFiClientSecure.h>
#endif


#include "OpenWeather.h"


/***************************************************************************************
** Function name:           getForecast (using onecall API)
** Description:             Setup the weather forecast request
***************************************************************************************/
// The structures etc are created by the sketch and passed to this function.
// Pass a nullptr for current, hourly or daily pointers to exclude in response.
// ESP8266: Setting secure to false will invoke an insecure connection with AXTLS
//          for the connection, when set true BearSSL will be used.
// ESP32:   Secure parameter has no affect.
bool OW_Weather::getForecast(OW_current *current, OW_hourly *hourly, OW_daily *daily,
                             String api_key, String latitude, String longitude,
                             String units, String language, bool secure) {

  data_set = "";
  hourly_index = 0;
  daily_index = 0;
  Secure = secure;
  oneCall = true;

  // Local copies of structure pointers, the structures are filled during parsing
  this->current  = current;
  this->hourly   = hourly;
  this->daily    = daily;

  // Exclude some info by passing fn a NULL pointer to reduce memory needed
  String exclude = ",alerts";
  if (!current)  exclude += ",current";
  if (!hourly)   exclude += ",hourly";
  if (!daily)    exclude += ",daily";

  // One call API now subscription
  String url = "https://api.openweathermap.org/data/2.5/onecall?lat=" + latitude + "&lon=" + longitude + "&exclude=minutely" + exclude + "&units=" + units + "&lang=" + language + "&appid=" + api_key;

  // Send GET request and feed the parser
  bool result = parseRequest(url);

  // Null out pointers to prevent crashes
  this->current  = nullptr;
  this->hourly   = nullptr;
  this->daily    = nullptr;

  return result;
}

/***************************************************************************************
** Function name:           getForecast (using forecast API)
** Description:             Setup the weather forecast request
***************************************************************************************/
bool OW_Weather::getForecast(OW_forecast *forecast, String api_key, 
                             String latitude, String longitude,
                             String units, String language, bool secure)
{
  data_set = "";
  forecast_index = 0;
  Secure = secure;
  oneCall = false;

  // Local copies of structure pointers, the structures are filled during parsing
  this->forecast  = forecast;

  // 5 day forecast every 3 hours from request time
  String url = "https://api.openweathermap.org/data/2.5/forecast?lat=" + latitude + "&lon=" + longitude + "&units=" + units + "&lang=" + language + "&appid=" + api_key;

  // Send GET request and feed the parser
  bool result = parseRequest(url);

  // Null out pointers to prevent crashes
  this->forecast  = nullptr;

  return result;
}
/***************************************************************************************
** Function name:           partialDataSet
** Description:             Set requested data set to partial (true) or full (false)
***************************************************************************************/
void OW_Weather::partialDataSet(bool partialSet) {
  
  this->partialSet = partialSet;
}

#ifdef ESP32 // Decide if ESP32 or ESP8266 parseRequest available

/***************************************************************************************
** Function name:           parseRequest (for ESP32)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool OW_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  OW_STATUS_PRINTF("\n\nThe connection to server is secure (https). Certificate not checked.\n");
  WiFiClientSecure client;
  client.setInsecure(); // Certificate not checked

  const char*  host = "api.openweathermap.org";
  port = 443;

  if (!client.connect(host, port))
  {
    OW_STATUS_PRINTF("Connection failed.\n");
    return false;
  }

  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

#ifdef SHOW_JSON
  int ccount = 0;
#endif
  // Send GET request
  Serial.println();
  OW_STATUS_PRINT("Sending GET request to "); OW_STATUS_PRINT(host); OW_STATUS_PRINT(" port "); OW_STATUS_PRINT(port); OW_STATUS_PRINTF("\n");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OW_STATUS_PRINTF("Header end found\n");
      break;
    }

#ifdef SHOW_HEADER
    Serial.println(line);
#endif

    if ((millis() - timeout) > 5000UL)
    {
      OW_STATUS_PRINTF ("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }

  OW_STATUS_PRINTF("\nParsing JSON\n");

  // Parse the JSON data, available() includes yields
  while ( client.available() > 0 || client.connected())
  {
    while(client.available() > 0)
    {
      c = client.read();
      parser.parse(c);
#ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.println(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
#endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OW_STATUS_PRINTF("Client timeout during JSON parse\n");
      parser.reset();
      client.stop();
      return false;
    }
    yield();
  }

  OW_STATUS_PRINTF("\nDone in "); OW_STATUS_PRINT(millis()-dt); OW_STATUS_PRINTF(" ms\n");
  Serial.println();

  parser.reset();

  client.stop();
  
  // A message has been parsed, but the data-point correctness is unknown
  return parseOK;
}

#else // ESP8266 or Arduino RP2040 Nano Connect version

/***************************************************************************************
** Function name:           parseRequest (for ESP8266)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool OW_Weather::parseRequest(String url) {
  if (Secure) return parseRequestSecure(&url);
  else return parseRequestInsecure(&url);
}

bool OW_Weather::parseRequestSecure(String* url) {

  uint32_t dt = millis();

  const char*  host = "api.openweathermap.org";

  #if (defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)) && !defined(ARDUINO_RASPBERRY_PI_PICO_W)
  WiFiSSLClient client;
  #else
  // Must use namespace:: to select BearSSL
  BearSSL::WiFiClientSecure client;
  client.setInsecure(); // Certificate not checked
  #endif
  port = 443;

  if (!client.connect(host, port))
  {
    OW_STATUS_PRINTF("Connection failed.\n");
    return false;
  }
  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

  #ifdef SHOW_JSON
  int ccount = 0;
  #endif

  #ifdef ESP8266
  OW_STATUS_PRINTF("\nThe connection to server is using BearSSL in insecure mode (certificates not checked).\n");
  #endif

  // Send GET request
  Serial.println();
  OW_STATUS_PRINTF("Sending GET request to api.openweathermap.org...\n");
  Serial.println();
  client.print(String("GET ") + *url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  Serial.println();

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.available() || client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OW_STATUS_PRINTF("Header end found\n");
      break;
    }

    OW_STATUS_PRINT(line); OW_STATUS_PRINTF("\n");

    if ((millis() - timeout) > 5000UL)
    {
      OW_STATUS_PRINTF ("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }


  // Parse the JSON data, available() includes yields
  while (client.available() || client.connected())
  {
    while (client.available())
    {
      c = client.read();
      parser.parse(c);
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OW_STATUS_PRINTF ("JSON client timeout\n");
      parser.reset();
      client.stop();
      return false;
    }
  }

  Serial.println();
  OW_STATUS_PRINTF("\nDone in "); OW_STATUS_PRINT(millis()-dt); OW_STATUS_PRINTF(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed without error but the data-point correctness is unknown
  return parseOK;
}

bool OW_Weather::parseRequestInsecure(String* url) {

  uint32_t dt = millis();

  const char*  host = "api.openweathermap.org";

  // AXTLS used (insecure)
  WiFiClient client;
  port = 80;
 
  if (!client.connect(host, port))
  {
    OW_STATUS_PRINTF("Connection failed.\n");
    return false;
  }
  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

  #ifdef SHOW_JSON
  int ccount = 0;
  #endif

  OW_STATUS_PRINTF("\nThe connection to server is INSECURE (using AXTLS).\n");

  // Send GET request
  OW_STATUS_PRINTF("Sending GET request to api.openweathermap.org...\n");
  client.print(String("GET ") + *url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.available() || client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OW_STATUS_PRINTF("Header end found\n");
      break;
    }

    OW_STATUS_PRINT(line); OW_STATUS_PRINTF("\n");

    if ((millis() - timeout) > 5000UL)
    {
      OW_STATUS_PRINTF("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }


  // Parse the JSON data, available() includes yields
  while (client.available() || client.connected())
  {
    while (client.available())
    {
      c = client.read();
      parser.parse(c);
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OW_STATUS_PRINTF("JSON client timeout\n");
      parser.reset();
      client.stop();
      return false;
    }
  }

  OW_STATUS_PRINTF("\nDone in "); OW_STATUS_PRINT(millis()-dt); OW_STATUS_PRINTF(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed without error but the data-point correctness is unknown
  return parseOK;
}

 #endif // ESP32 or ESP8266 parseRequest


/***************************************************************************************
** Function name:           key etc
** Description:             These functions are called while parsing the JSON message
***************************************************************************************/
void OW_Weather::key(const char *key) {

  currentKey = key;

#ifdef SHOW_CALLBACK
  Serial.println("\n>>> Key >>>" + (String)key);
#endif
}

void OW_Weather::startDocument() {

  currentParent = currentKey =   currentSet = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;
  arrayLevel = 0;
  parseOK = true;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start document >>>");
#endif
}

void OW_Weather::endDocument() {

  currentParent = currentKey = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;
  arrayLevel = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End document <<<");
#endif
}

void OW_Weather::startObject() {

  if (arrayIndex == 0 && objectLevel == 1) currentParent = currentKey;
  currentSet = currentKey;
  objectLevel++;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start object level:" + (String) objectLevel + " array level:" + (String) arrayLevel + " array index:" + (String) arrayIndex +" >>>");
#endif
}

void OW_Weather::endObject() {

  if (arrayLevel == 0) currentParent = "";
  if (arrayLevel == 1  && objectLevel == 2) arrayIndex++;
  objectLevel--;
  

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End object <<<");
#endif
}

void OW_Weather::startArray() {

  arrayLevel++;
  valuePath = currentParent + "/" + currentKey; // aka = current Object, e.g. "daily:data"

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start array " + valuePath + "/" + (String) arrayLevel + "/" + (String) arrayIndex +" >>>");
#endif
}

void OW_Weather::endArray() {
  if (arrayLevel > 0) arrayLevel--;
  if (arrayLevel == 0) arrayIndex = 0;
  valuePath = "";

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End array <<<");
#endif
}

void OW_Weather::whitespace(char c) {
  c = c; // Avoid warning
}

void OW_Weather::error( const char *message ) {
  Serial.print("\nParse error message: ");
  Serial.print(message);
  parseOK = false;
}

/***************************************************************************************
** Function name:           value (full or partial data set)
** Description:             Stores the parsed data in the structures for sketch access
***************************************************************************************/
void OW_Weather::value(const char *val)
{
  if (oneCall) {
    if (!partialSet) fullDataSet(val);
    else partialDataSet(val);
  }
  else {
    forecastDataSet(val);
  }
}

/***************************************************************************************
** Function name:           fullDataSet
** Description:             Collects full data set
***************************************************************************************/
void OW_Weather::fullDataSet(const char *val) {

   String value = val;

  // Start of JSON
  if (currentParent == "") {
    if (currentKey == "lat") lat = value.toFloat();
    if (currentKey == "lon") lon = value.toFloat();
    if (currentKey == "timezone") timezone = value;
  }

  // Current forecast - no array index - short path
  if (currentParent == "current") {
    data_set = "current";
    if (currentKey == "dt") current->dt = (uint32_t)value.toInt();
    else
    if (currentKey == "sunrise") current->sunrise = (uint32_t)value.toInt();
    else
    if (currentKey == "sunset") current->sunset = (uint32_t)value.toInt();
    else
    if (currentKey == "temp") current->temp = value.toFloat();
    else
    if (currentKey == "feels_like") current->feels_like = value.toFloat();
    else
    if (currentKey == "pressure") current->pressure = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = value.toInt();
    else
    if (currentKey == "dew_point") current->dew_point = value.toFloat();
    else
    if (currentKey == "uvi") current->uvi = value.toFloat();
    else
    if (currentKey == "clouds") current->clouds = value.toInt();
    else
    if (currentKey == "visibility") current->visibility = value.toInt();
    else
    if (currentKey == "wind_speed") current->wind_speed = value.toFloat();
    else
    if (currentKey == "wind_gust") current->wind_gust = value.toFloat();
    else
    if (currentKey == "wind_deg") current->wind_deg = (uint16_t)value.toInt();
    else
    if (currentKey == "rain") current->rain = value.toFloat();
    else
    if (currentKey == "snow") current->snow = value.toFloat();
    else

    if (currentKey == "id") current->id = value.toInt();
    else
    if (currentKey == "main") current->main = value;
    else
    if (currentKey == "description") current->description = value;
    else
    if (currentKey == "icon") current->icon = value;

    return;
  }

  // Hourly forecast
  if (currentParent == "hourly") {
    data_set = "hourly";
    
    if (arrayIndex >= MAX_HOURS) return;
    
    if (currentKey == "dt") hourly->dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "temp") hourly->temp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "feels_like") hourly->feels_like[arrayIndex] = value.toFloat();
    else
    if (currentKey == "pressure") hourly->pressure[arrayIndex] = value.toFloat();
    else
    if (currentKey == "humidity") hourly->humidity[arrayIndex] = value.toInt();
    else
    if (currentKey == "dew_point") hourly->dew_point[arrayIndex] = value.toFloat();
    else
    if (currentKey == "clouds") hourly->clouds[arrayIndex] = value.toInt();
    else
    if (currentKey == "wind_speed") hourly->wind_speed[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_gust") hourly->wind_gust[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_deg") hourly->wind_deg[arrayIndex] = (uint16_t)value.toInt();
    else
    if (currentKey == "rain") hourly->rain[arrayIndex] = value.toFloat();
    else
    if (currentKey == "snow") hourly->snow[arrayIndex] = value.toFloat();
    else

    if (currentKey == "id") hourly->id[arrayIndex] = value.toInt();
    else
    if (currentKey == "main") hourly->main[arrayIndex] = value;
    else
    if (currentKey == "description") hourly->description[arrayIndex] = value;
    else
    if (currentKey == "icon") hourly->icon[arrayIndex] = value;
    else
    if (currentKey == "pop") hourly->pop[arrayIndex] = value.toFloat();
    else
    if (currentKey == "1h") hourly->rain1h[arrayIndex] = value.toFloat();

    return;
  }


  // Daily forecast
  if (currentParent == "daily") {
    data_set = "daily";
    
    if (arrayIndex >= MAX_DAYS) return;
    
    if (currentKey == "dt") daily->dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "sunrise") daily->sunrise[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "sunset") daily->sunset[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "moonrise") daily->moonrise[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "moonset") daily->moonset[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "pressure") daily->pressure[arrayIndex] = value.toFloat();
    else
    if (currentKey == "humidity") daily->humidity[arrayIndex] = value.toInt();
    else
    if (currentKey == "dew_point") daily->dew_point[arrayIndex] = value.toFloat();
    else
    if (currentKey == "clouds") daily->clouds[arrayIndex] = value.toInt();
    else
    if (currentKey == "wind_speed") daily->wind_speed[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_gust") daily->wind_gust[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_deg") daily->wind_deg[arrayIndex] = (uint16_t)value.toInt();
    else
    if (currentKey == "rain") daily->rain[arrayIndex] = value.toFloat();
    else
    if (currentKey == "snow") daily->snow[arrayIndex] = value.toFloat();
    else

    if (currentKey == "id") daily->id[arrayIndex] = value.toInt();
    else
    if (currentKey == "main") daily->main[arrayIndex] = value;
    else
    if (currentKey == "description") daily->description[arrayIndex] = value;
    else
    if (currentKey == "icon") daily->icon[arrayIndex] = value;
    else
    if (currentKey == "pop") daily->pop[arrayIndex] = value.toFloat();

    if (currentSet == "temp") {
      if (currentKey == "morn") daily->temp_morn[arrayIndex] = value.toFloat();
      else
      if (currentKey == "day") daily->temp_day[arrayIndex] = value.toFloat();
      else
      if (currentKey == "eve") daily->temp_eve[arrayIndex] = value.toFloat();
      else
      if (currentKey == "night") daily->temp_night[arrayIndex] = value.toFloat();
      else
      if (currentKey == "min") daily->temp_min[arrayIndex] = value.toFloat();
      else
      if (currentKey == "max") daily->temp_max[arrayIndex] = value.toFloat();
    }

    if (currentSet == "feels_like") {
      if (currentKey == "morn") daily->feels_like_morn[arrayIndex] = value.toFloat();
      else
      if (currentKey == "day") daily->feels_like_day[arrayIndex] = value.toFloat();
      else
      if (currentKey == "eve") daily->feels_like_eve[arrayIndex] = value.toFloat();
      else
      if (currentKey == "night") daily->feels_like_night[arrayIndex] = value.toFloat();
    }

    return;
  }

}

/***************************************************************************************
** Function name:           forecastDataSet
** Description:             Collects full data set
***************************************************************************************/
void OW_Weather::forecastDataSet(const char *val) {

   String value = val;

  // Start of JSON
  if (currentParent == "") {
    if (currentKey == "timezone") forecast->timezone = value.toInt();
    else
    if (currentKey == "sunrise") forecast->sunrise = (uint32_t)value.toInt();
    else
    if (currentKey == "sunset") forecast->sunset = (uint32_t)value.toInt();

    return;
  }

  // Loacation
  if (currentParent == "city") {
    if (currentKey == "name") forecast->city_name = value;
    else
    if (currentKey == "lat") lat = value.toFloat();
    else
    if (currentKey == "lon") lon = value.toFloat();

    return;
  }

  // 3 hourly forecasts
  if (currentParent == "list") {
    data_set = "list";
    if (arrayIndex >= MAX_3HRS) return;

    if (currentKey == "dt") forecast->dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "temp") forecast->temp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "temp_min") forecast->temp_min[arrayIndex] = value.toFloat();
    else
    if (currentKey == "temp_max") forecast->temp_max[arrayIndex] = value.toFloat();
    else
    if (currentKey == "feels_like") forecast->feels_like[arrayIndex] = value.toFloat();
    else
    if (currentKey == "pressure") forecast->pressure[arrayIndex] = value.toFloat();
    else
    if (currentKey == "sea_level") forecast->sea_level[arrayIndex] = value.toFloat();
    else
    if (currentKey == "grnd_level") forecast->grnd_level[arrayIndex] = value.toFloat();
    else
    if (currentKey == "humidity") forecast->humidity[arrayIndex] = value.toInt();
    else
    if (currentKey == "id") forecast->id[arrayIndex] = value.toInt();
    else
    if (currentKey == "main") forecast->main[arrayIndex] = value;
    else
    if (currentKey == "description") forecast->description[arrayIndex] = value;
    else
    if (currentKey == "icon") forecast->icon[arrayIndex] = value;
    else
    if (currentKey == "all") forecast->clouds_all[arrayIndex] = (uint8_t)value.toInt();
    else
    if (currentKey == "speed") forecast->wind_speed[arrayIndex] = value.toFloat();
    else
    if (currentKey == "deg") forecast->wind_deg[arrayIndex] = (uint16_t)value.toInt();
    else
    if (currentKey == "gust") forecast->wind_gust[arrayIndex] = value.toFloat();
    else
    if (currentKey == "visibility") forecast->visibility[arrayIndex] = value.toInt();
    else
    if (currentKey == "pop") forecast->pop[arrayIndex] = value.toFloat();
    else
    if (currentKey == "dt_txt") forecast->dt_txt[arrayIndex] = value;

    return;
  }

}

/***************************************************************************************
** Function name:           partialDataSet
** Description:             Collects partial data set
***************************************************************************************/
void OW_Weather::partialDataSet(const char *val) {

   String value = val;

  // Current forecast - no array index - short path
  if (currentParent == "current") {
    data_set = "current";
    if (currentKey == "dt") current->dt = (uint32_t)value.toInt();
    else
    if (currentKey == "sunrise") current->sunrise = (uint32_t)value.toInt();
    else
    if (currentKey == "sunset") current->sunset = (uint32_t)value.toInt();
    else
    if (currentKey == "temp") current->temp = value.toFloat();
    //else
    //if (currentKey == "feels_like") current->feels_like = value.toFloat();
    else
    if (currentKey == "pressure") current->pressure = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = value.toInt();
    //else
    //if (currentKey == "dew_point") current->dew_point = value.toFloat();
    //else
    //if (currentKey == "uvi") current->uvi = value.toFloat();
    else
    if (currentKey == "clouds") current->clouds = value.toInt();
    //else
    //if (currentKey == "visibility") current->visibility = value.toInt();
    else
    if (currentKey == "wind_speed") current->wind_speed = value.toFloat();
    //else
    //if (currentKey == "wind_gust") current->wind_gust = value.toFloat();
    else
    if (currentKey == "wind_deg") current->wind_deg = (uint16_t)value.toInt();
    //else
    //if (currentKey == "rain") current->rain = value.toFloat();
    //else
    //if (currentKey == "snow") current->snow = value.toFloat();

    else
    if (currentKey == "id") current->id = value.toInt();
    else
    if (currentKey == "main") current->main = value;
    else
    if (currentKey == "description") current->description = value;
    //else
    //if (currentKey == "icon") current->icon = value;

    return;
  }

/*
  // Hourly forecast
  if (currentParent == "hourly") {
    data_set = "hourly";
    
    if (arrayIndex >= MAX_HOURS) return;
    
    if (currentKey == "dt") hourly->dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "temp") hourly->temp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "feels_like") hourly->feels_like[arrayIndex] = value.toFloat();
    else
    if (currentKey == "pressure") hourly->pressure[arrayIndex] = value.toFloat();
    else
    if (currentKey == "humidity") hourly->humidity[arrayIndex] = value.toInt();
    else
    if (currentKey == "dew_point") hourly->dew_point[arrayIndex] = value.toFloat();
    else
    if (currentKey == "clouds") hourly->clouds[arrayIndex] = value.toInt();
    else
    if (currentKey == "wind_speed") hourly->wind_speed[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_gust") hourly->wind_gust[arrayIndex] = value.toFloat();
    else
    if (currentKey == "wind_deg") hourly->wind_deg[arrayIndex] = (uint16_t)value.toInt();
    else
    if (currentKey == "rain") hourly->rain[arrayIndex] = value.toFloat();
    else
    if (currentKey == "snow") hourly->snow[arrayIndex] = value.toFloat();
    else

    if (currentKey == "id") hourly->id[arrayIndex] = value.toInt();
    else
    if (currentKey == "main") hourly->main[arrayIndex] = value;
    else
    if (currentKey == "description") hourly->description[arrayIndex] = value;
    else
    if (currentKey == "icon") hourly->icon[arrayIndex] = value;

    return;
  }
*/

  // Daily forecast
  if (currentParent == "daily") {
    data_set = "daily";
    
    if (arrayIndex >= MAX_DAYS) return;
    
    if (currentKey == "dt") daily->dt[arrayIndex] = (uint32_t)value.toInt();
    else
    //if (currentKey == "sunrise") daily->sunrise[arrayIndex] = (uint32_t)value.toInt();
    //else
    //if (currentKey == "sunset") daily->sunset[arrayIndex] = (uint32_t)value.toInt();
    //else
    //if (currentKey == "pressure") daily->pressure[arrayIndex] = value.toFloat();
    //else
    //if (currentKey == "humidity") daily->humidity[arrayIndex] = value.toInt();
    //else
    //if (currentKey == "dew_point") daily->dew_point[arrayIndex] = value.toFloat();
    //else
    //if (currentKey == "clouds") daily->clouds[arrayIndex] = value.toInt();
    //else
    //if (currentKey == "wind_speed") daily->wind_speed[arrayIndex] = value.toFloat();
    //else
    //if (currentKey == "wind_gust") daily->wind_gust[arrayIndex] = value.toFloat();
    //else
    //if (currentKey == "wind_deg") daily->wind_deg[arrayIndex] = (uint16_t)value.toInt();
    //else
    //if (currentKey == "rain") daily->rain[arrayIndex] = value.toFloat();
    //else
    //if (currentKey == "snow") daily->snow[arrayIndex] = value.toFloat();
    //else

    if (currentKey == "id") daily->id[arrayIndex] = value.toInt();
    //else
    //if (currentKey == "main") daily->main[arrayIndex] = value;
    //else
    //if (currentKey == "description") daily->description[arrayIndex] = value;
    //else
    //if (currentKey == "icon") daily->icon[arrayIndex] = value;

    if (currentSet == "temp") {
      //if (currentKey == "morn") daily->temp_morn[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "day") daily->temp_day[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "eve") daily->temp_eve[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "night") daily->temp_night[arrayIndex] = value.toFloat();
      //else
      if (currentKey == "min") daily->temp_min[arrayIndex] = value.toFloat();
      else
      if (currentKey == "max") daily->temp_max[arrayIndex] = value.toFloat();
    }

    //if (currentSet == "feels_like") {
      //if (currentKey == "morn") daily->feels_like_morn[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "day") daily->feels_like_day[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "eve") daily->feels_like_eve[arrayIndex] = value.toFloat();
      //else
      //if (currentKey == "night") daily->feels_like_night[arrayIndex] = value.toFloat();
    //}

    return;
  }

}
