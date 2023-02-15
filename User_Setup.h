
// Configuration settings for OpenWeather library


// These parameters set the data point count stored in program memory (not the datapoint
// count sent by the server). So they determine the memory used during collection
// of the data points.

#define MAX_HOURS 6     // Maximum "hourly" forecast period, can be up 1 to 48
                        // Hourly forecast not used by TFT_eSPI_OpenWeather example

#define MAX_DAYS 5      // Maximum "daily" forecast periods can be 1 to 8 (Today + 7 days = 8 maximum)
                        // TFT_eSPI_OpenWeather example requires this to be >= 5 (today + 4 forecast days)

//#define SHOW_HEADER   // Debug only - for checking response header via serial message
//#define SHOW_JSON     // Debug only - simple serial output formatting of whole JSON message
//#define SHOW_CALLBACK // Debug only to show the decode tree
#define OW_STATUS_ON    // Debug only - turn on/off progress and status messages


// ###############################################################################
// DO NOT tinker below, this is configuration checking that helps stop crashes:
// ###############################################################################

#ifdef OW_STATUS_ON
  #define OW_STATUS_PRINTF(C) Serial.print(F(C))
  #define OW_STATUS_PRINT(V) Serial.print(V)
#else
  #define OW_STATUS_PRINTF(C)
  #define OW_STATUS_PRINT(X)
#endif

// Check and correct bad setting
#if (MAX_HOURS > 48) || (MAX_HOURS < 1)
  #undef  MAX_HOURS
  #define MAX_HOURS 48 // Ignore compiler warning!
#endif

// Check and correct bad setting
#if (MAX_DAYS > 8) || (MAX_DAYS < 1)
  #undef  MAX_DAYS
  #define MAX_DAYS 8  // Ignore compiler warning!
#endif

#define MAX_3HRS (MAX_DAYS * 8)