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

#pragma endregion

#pragma region Variables

esp_sleep_wakeup_cause_t WakeupReason_g;

/**
 * @brief Boot time counts.
 * 
 */
RTC_DATA_ATTR int BootCount_g = 0;

/**
 * @brief Interupt mask.
 * 
 */
uint64_t InteruptMask_g;

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
 * @brief Wi-Fi multy client.
 * 
 */
WiFiMulti WiFiMultyClient_g;

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

/**
 * @brief Read battery voltage.
 * 
 * @return double 
 */
double bat_voltage()
{
  double vin = map(analogRead(PIN_BATT), 0.0f, 4095.0f, 0, 3.3F);
  double voltage = (vin / DIV_R2) / (DIV_R1 + DIV_R2);
  return voltage;
}

/**
 * @brief Function that prints the reason by which ESP32 has been awaken from sleep.
 * 
 */
void print_wakeup_reason()
{
  switch(WakeupReason_g)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

/**
 * @brief Update loop.
 * 
 */
void update_loop()
{
  // 
  static int WiFiMultyClientStateL = 0;

  // Update WiFi client.
  WiFiMultyClientStateL = WiFiMultyClient_g.run();

  // Wait for WiFi connection.
  if (WiFiMultyClientStateL == WL_CONNECTED)
  {
    // CAll the server.
    HTTPClient_g.begin(URL_g); //HTTP
    Serial.println("[HTTP] begin...");
        
    // Start connection and send HTTP header.
    int HTTPCodeL = HTTPClient_g.GET();
    Serial.print("[HTTP] GET...\n");
    
    // HTTPCodeL will be negative on error.
    if(HTTPCodeL > 0)
    {
      // HTTP header has been send and Server response header has been handled.
      Serial.printf("[HTTP] GET... code: %d\n", HTTPCodeL);
      
      // File found at server.
      if(HTTPCodeL == HTTP_CODE_OK)
      {
        String payload = HTTPClient_g.getString();
        Serial.println(payload);
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", HTTPClient_g.errorToString(HTTPCodeL).c_str());
    }
    
    // End HTTP connection.
    HTTPClient_g.end();
    
    // Inform the user that the process of calling is enup succesfully and it is time to sleep!
    digitalWrite(PIN_LED, LOW);

    // Bye bye ...
    Serial.println("Going to deep sleep.");
    
    //Go to sleep now
    esp_deep_sleep_start();
  }
  else
  {
    Serial.printf("[WiFi] State: %s\n", WiFiMultyClientStateL);
  }
}

#pragma endregion

void setup()
{
  // Get wakeup reason.
  WakeupReason_g = esp_sleep_get_wakeup_cause();

  // Init UART.
  Serial.begin(115200);

  // Read the state of the pushbutton value.
  ButtonState_g = 0;
  WakeupStatus_g = esp_sleep_get_ext1_wakeup_status();
  ButtonState_g += (((1 << PIN_INPUT_1) & WakeupStatus_g) == (1 << PIN_INPUT_1)) * 1;
  ButtonState_g += (((1 << PIN_INPUT_2) & WakeupStatus_g) == (1 << PIN_INPUT_2)) * 2;
  ButtonState_g += (((1 << PIN_INPUT_3) & WakeupStatus_g) == (1 << PIN_INPUT_3)) * 4;
  ButtonState_g += (((1 << PIN_INPUT_4) & WakeupStatus_g) == (1 << PIN_INPUT_4)) * 8;
  Serial.println("[GPIO] ButtonState_g: " + String(ButtonState_g));

  // Prepare interupt mask.
  InteruptMask_g = 0;
  InteruptMask_g |= (1 << PIN_INPUT_1);
  InteruptMask_g |= (1 << PIN_INPUT_2);
  InteruptMask_g |= (1 << PIN_INPUT_3);
  InteruptMask_g |= (1 << PIN_INPUT_4);

  //Configure EXT1 wake up source for HIGH logic level.
  esp_sleep_enable_ext1_wakeup(InteruptMask_g, ESP_EXT1_WAKEUP_ANY_HIGH);

  // User LED.
  pinMode(PIN_LED, OUTPUT);
  // Inform the user that the proces of calling has began.
  digitalWrite(PIN_LED, HIGH);

  // Read battery voltage!
  BatteryVoltage_g = bat_voltage();
  Serial.println("[BAT] BatteryVoltage_g: " + String(BatteryVoltage_g));

  //Increment boot number and print it every reboot
  ++BootCount_g;
  Serial.println("[SYS] Boot number: " + String(BootCount_g));

  //
  URL_g = String(END_POINT)
    + "?btn=" + String(ButtonState_g)
    + "&bat=" + String(BatteryVoltage_g)
    + "&wkr=" + String(WakeupReason_g)
    + "&bootc=" + String(BootCount_g);
  Serial.println(URL_g);

  // Connect 
  WiFiMultyClient_g.addAP(DEFAULT_SSID , DEFAULT_PASS);
  WiFiMultyClient_g.addAP(MOBILE_SSID, MOBILE_PASS);

  // Print the wakeup reason for ESP32
  print_wakeup_reason();

  //Go to sleep now
  // Serial.println("Going to sleep ...");

  // 
  // esp_deep_sleep_start();
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
