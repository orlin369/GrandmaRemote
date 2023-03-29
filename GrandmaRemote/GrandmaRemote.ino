/*

MIT License

Copyright (c) [2019] [Orlin Dimitrov]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma region Headers

#include <WiFi.h>

#include <WiFiMulti.h>

#include <HTTPClient.h>

#include "ApplicationConfiguration.h"

#include <DefaultCredentials.h>

/* Debug serial port. */
#include "DebugPort.h"

#pragma endregion

#pragma region Variables

esp_sleep_wakeup_cause_t WakeupReason_g;

/**
 * @brief Boot time counts.
 * 
 */
RTC_DATA_ATTR int BootCount_g = 0;

/**
 * @brief Interrupt mask.
 * 
 */
uint64_t InterruptMask_g;

/**
 * @brief Wakeup state.
 * 
 */
uint64_t WakeupStatus_g;

/**
 * @brief Button state.
 * 
 */
uint8_t ButtonState_g;

/**
 * @brief Battery voltage.
 * 
 */
double BatteryVoltage_g;

/**
 * @brief Wi-Fi multi client.
 * 
 */
WiFiMulti WiFiMultiClient_g;

/**
 * @brief HTTP Client.
 * 
 */
HTTPClient HTTPClient_g;

/**
 * @brief Current time.
 * 
 */
unsigned long CurrentTime_g = 0;

/**
 * @brief Last time.
 * 
 */
unsigned long LastTime_g = 0;

/**
 * @brief URL to the endpoint server.
 * 
 */
String URL_g = "";

#pragma endregion

#pragma region Functions

/** @brief Printout in the debug console flash state.
 *  @return Void.
 */
void show_device_properties() {
#ifdef SHOW_FUNC_NAMES
	DEBUGLOG("\r\n");
	DEBUGLOG(__PRETTY_FUNCTION__);
	DEBUGLOG("\r\n");
#endif // SHOW_FUNC_NAMES

#if defined(ESP8266)
// ESP8266
	DEBUGLOG("Flash chip size: %u\r\n", ESP.getFlashChipRealSize());
#endif

	DEBUGLOG("Sketch size: %u\r\n", ESP.getSketchSize());
	DEBUGLOG("Free flash space: %u\r\n", ESP.getFreeSketchSpace());
	DEBUGLOG("Free heap: %d\r\n", ESP.getFreeHeap());
	DEBUGLOG("Firmware version: %d\r\n", ESP_FW_VERSION);
	DEBUGLOG("SDK version: %s\r\n", ESP.getSdkVersion());
	DEBUGLOG("MAC address: %s\r\n", WiFi.macAddress().c_str());
	DEBUGLOG("\r\n");
}

/**
 * @brief Read battery voltage.
 * 
 * @return double 
 */
double bat_voltage()
{
  // double vin = map(analogRead(PIN_BATT), 0.0f, 4095.0f, 0, 3.3F);
  double voltage = map((analogRead(PIN_BATT)*2.0), 0.0f, 4095.0f, 0, 3.3F); // analogRead(PIN_BATT) ; //(analogRead(PIN_BATT) / DIV_R2) / (DIV_R1 + DIV_R2);
  return voltage;
}

/**
 * @brief Function that prints the reason by which ESP32 has been awaken from sleep.
 * 
 */
void print_wakeup_reason()
{
  Serial.print("[SYS] ");
  switch(WakeupReason_g)
  {
    case 1  : DEBUGLOG("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : DEBUGLOG("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : DEBUGLOG("Wakeup caused by timer"); break;
    case 4  : DEBUGLOG("Wakeup caused by touch"); break;
    case 5  : DEBUGLOG("Wakeup caused by ULP program"); break;
    default : DEBUGLOG("Wakeup was not caused by deep sleep"); break;
  }
  DEBUGLOG("\r\n");
}

/**
 * @brief Update loop.
 * 
 */
void update_loop()
{
  // 
  static int WiFiMultiClientStateL = 0;

  // Update WiFi client.
  WiFiMultiClientStateL = WiFiMultiClient_g.run();

  // Wait for WiFi connection.
  if (WiFiMultiClientStateL == WL_CONNECTED)
  {
    // CAll the server.
    HTTPClient_g.begin(URL_g); //HTTP
    DEBUGLOG("[HTTP] Begin\r\n");
        
    // Start connection and send HTTP header.
    int HTTPCodeL = HTTPClient_g.GET();
    DEBUGLOG("[HTTP] Get\n");
    
    // HTTPCodeL will be negative on error.
    if(HTTPCodeL > 0)
    {
      // HTTP header has been send and Server response header has been handled.
      DEBUGLOG("[HTTP] Code: %d\n", HTTPCodeL);
      
      // File found at server.
      if(HTTPCodeL == HTTP_CODE_OK)
      {
        String payload = HTTPClient_g.getString();
        DEBUGLOG("%S", payload.c_str());
      }
    }
    else
    {
      DEBUGLOG("[HTTP] Code: %s\n", HTTPClient_g.errorToString(HTTPCodeL).c_str());
    }
    
    // End HTTP connection.
    HTTPClient_g.end();
    
    // Inform the user that the process of calling is end up successfully and it is time to sleep!
    digitalWrite(PIN_LED, LOW);

    // Bye bye ...
    DEBUGLOG("[SYS] Going to deep sleep.\r\n");
    
    //Go to sleep now
    esp_deep_sleep_start();
  }
  else
  {
    DEBUGLOG("[WiFi] State: %d\r\n", WiFiMultiClientStateL);
  }
}

#pragma endregion

void setup()
{
  // Setup debug port module.
	setup_debug_port();

  show_device_properties();

  // Get wakeup reason.
  WakeupReason_g = esp_sleep_get_wakeup_cause();

  // Print the wakeup reason for ESP32.
  print_wakeup_reason();

  // Read the state of the pushbutton value.
  ButtonState_g = 0;
  WakeupStatus_g = esp_sleep_get_ext1_wakeup_status();
  ButtonState_g += (((1 << PIN_INPUT_1) & WakeupStatus_g) == (1 << PIN_INPUT_1)) * 1;
  ButtonState_g += (((1 << PIN_INPUT_2) & WakeupStatus_g) == (1 << PIN_INPUT_2)) * 2;
  ButtonState_g += (((1 << PIN_INPUT_3) & WakeupStatus_g) == (1 << PIN_INPUT_3)) * 4;
  ButtonState_g += (((1 << PIN_INPUT_4) & WakeupStatus_g) == (1 << PIN_INPUT_4)) * 8;
  DEBUGLOG("[GPIO] ButtonState_g: %d\r\n", ButtonState_g);

  // Prepare interrupt mask.
  InterruptMask_g = 0;
  InterruptMask_g |= (1 << PIN_INPUT_1);
  InterruptMask_g |= (1 << PIN_INPUT_2);
  InterruptMask_g |= (1 << PIN_INPUT_3);
  InterruptMask_g |= (1 << PIN_INPUT_4);

  //Configure EXT1 wake up source for HIGH logic level.
  esp_sleep_enable_ext1_wakeup(InterruptMask_g, ESP_EXT1_WAKEUP_ANY_HIGH);

  // User LED.
  pinMode(PIN_LED, OUTPUT);
  // Inform the user that the process of calling has began.
  digitalWrite(PIN_LED, HIGH);

  // Read battery voltage!
  BatteryVoltage_g = bat_voltage();
  DEBUGLOG("[BAT] BatteryVoltage_g: %d\r\n", BatteryVoltage_g);

  //Increment boot number and print it every reboot
  ++BootCount_g;
  DEBUGLOG("[SYS] Boot number: %d\r\n", BootCount_g);

  //
  URL_g = String(END_POINT)
    + "?btn=" + String(ButtonState_g)
    + "&bat=" + String(BatteryVoltage_g)
    + "&wkr=" + String(WakeupReason_g)
    + "&bootc=" + String(BootCount_g);
  DEBUGLOG("[HTTP] URL_g: %s\r\n", URL_g.c_str());

  // Connect 
  WiFiMultiClient_g.addAP(DEFAULT_SSID , DEFAULT_PASS);
  WiFiMultiClient_g.addAP(MOBILE_SSID, MOBILE_PASS);
}

void loop()
{
  // Update time.
  CurrentTime_g = millis();

  // Check the time.
  if (CurrentTime_g - LastTime_g >= UPDATE_RATE)
  {
    // Update last time.
    LastTime_g = CurrentTime_g;

    // Call update loop.
    update_loop();
  }
}
