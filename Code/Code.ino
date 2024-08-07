// ###########################################################################################################################################
// #
// # Code for the "AWSW FANS N LIGHTS" project: https://github.com/AWSW-de/FansNLights
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/FansNLights?tab=GPL-3.0-1-ov-file#readme
// # Creative Commons Attribution-NonCommercial 3.0 Unported License http://creativecommons.org/licenses/by-nc/3.0
// # Important: NonCommercial — You may not use the material for commercial purposes !
// #
// ###########################################################################################################################################



// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project.
// #
// # IMPORTANT: ! Use the tested versions and named links to download the libraries. Avoid the Arduino IDE internal download manager!
// #
// # Library name:              Publisher:           Tested version:    Download link:
// # #############              ##########           ###############    ##############
// # - Adafruit NeoPixel        Adafruit             (V1.12.3)          https://github.com/adafruit/Adafruit_NeoPixel
// # - Adafruit BME280          Adafruit             (V2.2.4)           https://github.com/adafruit/Adafruit_BME280_Library
// # - Adafruit BusIO           Adafruit             (V1.16.1)          https://github.com/adafruit/Adafruit_BusIO
// # - Adafruit Unified Sensor  Adafruit             (V1.1.14)          https://github.com/adafruit/Adafruit_Sensor
// # - AsyncTCP                 me-no-dev            (V1.1.4)           https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer        me-no-dev            (V1.2.4)           https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                    s00500:              (V2.2.3)           https://github.com/s00500/ESPUI/archive/refs/tags/2.2.3.zip (!!! MAKE SURE NOT TO UPDATE TO >=2.2.4, ESP32 will permanetelly reboot !!!)
// # - ArduinoJson              bblanchon            (V7.1.0)           https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS                 lorol                (V1.0.6)           https://github.com/lorol/LITTLEFS
// # - Universal TelegramBot    witnessmenow         (V1.3.0)           https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
// # - LedControl               Everhard Fahle       (V1.0.6)           http://wayoda.github.io/LedControl/
// # - ezButtons                ArduinoGetStarted    (V1.0.4)           https://arduinogetstarted.com/tutorials/arduino-button-library
// #
// ###########################################################################################################################################
#include <SPI.h>                   // SPI
#include <Wire.h>                  // I2C
#include <Adafruit_NeoPixel.h>     // Used to drive the NeoPixel LEDs
#include <Adafruit_BME280.h>       // Used for the BME280 sensor
#include <Adafruit_Sensor.h>       // Used for the BME280 sensor
#include <WiFi.h>                  // Used to connect the ESP32 to your WiFi
#include <WiFiClientSecure.h>      // Telegram support
#include <UniversalTelegramBot.h>  // Telegram support
#include <ArduinoJson.h>           // Telegram support
#include <LedController.hpp>       // LED Display
#include <ezButton.h>              // Hardware buttons
#include <WebServer.h>             // ESP32 OTA update function
#include <Update.h>                // ESP32 OTA update function
#include <AsyncTCP.h>              // Used for the internal web server
#include <ESPAsyncWebServer.h>     // Used for the internal web server
#include <DNSServer.h>             // Used for the internal web server
#include <ESPUI.h>                 // Used for the internal web server
#include <Preferences.h>           // Used to save the configuration to the ESP32 flash



// ###########################################################################################################################################
// # Hardware settings:
// ###########################################################################################################################################
// LEDs:
#define LEDPIN 32     // Pin connected to the NeoPixel LEDs
#define NUMPIXELS 30  // How many NeoPixels are attached to the Arduino
// Fans:
#define FAN1_PIN 16     // Fan 1 control pin
#define SIGNAL1_PIN 17  // Fan 1 RPM signal pin
#define FAN2_PIN 18     // Fan 2 control pin
#define SIGNAL2_PIN 19  // Fan 2 RPM signal pin
// Display:
#define DIN 15  // pin 15 is connected to the DataIn
#define CS 13   // pin 13 is connected to LOAD/ChipSelect
#define CLK 14  // pin 14 is connected to the CLK
// Buttons:
#define BUTTON_PIN_1 25  // The ESP32 pin GPIO25 connected to the button 1
#define BUTTON_PIN_2 26  // The ESP32 pin GPIO26 connected to the button 2



// ###################################################################################################################################################################################################################################
// # !!! DO NOT PERFORM CHANGES FROM HERE !!! DO NOT PERFORM CHANGES FROM HERE !!! DO NOT PERFORM CHANGES FROM HERE !!! DO NOT PERFORM CHANGES FROM HERE !!! DO NOT PERFORM CHANGES FROM HERE !!! DO NOT PERFORM CHANGES FROM HERE !!!
// ###################################################################################################################################################################################################################################



// ###########################################################################################################################################
// # Code vesion:
// ###########################################################################################################################################
String CodeVersion = "1.0.0";


// ###########################################################################################################################################
// # Variables and init settings used in the code:
// ###########################################################################################################################################
// General:
Preferences preferences;  // ESP32 flash storage
bool updatedevice = true;
bool updatenow = false;
bool changedvalues = false;
bool WiFIsetup = false;
// Buttons:
ezButton button1(BUTTON_PIN_1);  // create ezButton object for button 1
ezButton button2(BUTTON_PIN_2);  // create ezButton object for button 2
// Telegram:
String BOTtoken = "XXXXXXXXXX:YYYYYYYYYYYYYYY-ZZZZZZZZZZZZZZZZZZZZ";
String CHAT_ID = "1234512345";
String chat_id = CHAT_ID;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
unsigned long bot_lasttime;           // last time messages' scan has been done
const unsigned long BOT_MTBS = 1000;  // mean time between scan messages --> 5s delay because of GUI stability issues at 1s
// BME280:
Adafruit_BME280 bme;
float temp, hum;
// LEDs:
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);  // Init LEDs
int redVal, greenVal, blueVal;
uint16_t color_LEDs;
// Network / Telegram / PrusaConnect:
int intensityLEDs, intensity7Seg, useshowip, usePrusaConnnect, useTelegram, setLEDonStart, checkLEDs, checkFANs, updatemode, useCorFtemp, MIN_TEMP, MAX_TEMP, MIN_FAN_SPEED, logoutput;
String printer1, printer2, TelegramName;
// Fans:
int SpeedForAllFans;
int fanerror = 0;
// Display:
LedController<1, 4> lc;
int iTempInt1, iTempInt2;
// Web server:
AsyncWebServer server(80);  // Web server for config
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer otaserver(8080);  // Web OTA ESP32 update server on the port 8080


// ###########################################################################################################################################
// # Setup function on startup:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);                               // Init the serial port for debugging
  pinMode(FAN1_PIN, OUTPUT);                          // Fan 1 control pin
  pinMode(SIGNAL1_PIN, INPUT);                        // Fan 1 RPM signal pin
  pinMode(FAN2_PIN, OUTPUT);                          // Fan 2 control pin
  pinMode(SIGNAL2_PIN, INPUT);                        // Fan 2 RPM signal pin
  preferences.begin("fansnlights", false);            // Init ESP32 flash
  getFlashValues();                                   // Read settings from flash
  strip.begin();                                      // Init the LEDs
  strip.show();                                       // Init the LEDs --> Set them all to OFF
  strip.setBrightness(intensityLEDs);                 // Set LED brightness
  delay(3000);                                        // Wait 3 seconds for the init of the serial port
  for (int i = 1; i <= 10; i++) Serial.println(" ");  // Generate some empty lines for better reading of the start sequence
  Serial.println("#######################################################");
  Serial.println("# Starting " + TelegramName + " - Version: " + CodeVersion);
  Serial.println("#######################################################");

  // Buttons:
  button1.setDebounceTime(100);  // set debounce time to 100 milliseconds
  button2.setDebounceTime(100);  // set debounce time to 100 milliseconds

  // Init display:
  lc = LedController<1, 4>(DIN, CLK, CS);
  lc.setIntensity(intensity7Seg);  // Set the brightness
  lc.clearMatrix();                // and clear the display

  // Display 1:
  lc.setChar(0, 7, ' ', false);
  lc.setChar(0, 6, ' ', false);
  lc.setChar(0, 5, 'A', false);
  lc.setChar(0, 4, 'W', false);
  lc.setChar(0, 3, 'S', false);
  lc.setChar(0, 2, 'W', false);
  lc.setChar(0, 1, ' ', false);
  lc.setChar(0, 0, ' ', false);
  // Display 2:
  lc.setChar(1, 7, ' ', false);
  lc.setChar(1, 6, 'F', false);
  lc.setChar(1, 5, 'A', false);
  lc.setChar(1, 4, 'N', false);
  lc.setChar(1, 3, 'S', false);
  lc.setChar(1, 2, ' ', false);
  lc.setChar(1, 1, 'N', false);
  lc.setChar(1, 0, ' ', false);
  // Display 3:
  lc.setChar(2, 7, ' ', false);
  lc.setChar(2, 6, 'L', false);
  lc.setChar(2, 5, 'I', false);
  lc.setChar(2, 4, 'G', false);
  lc.setChar(2, 3, 'H', false);
  lc.setChar(2, 2, 'T', false);
  lc.setChar(2, 1, 'S', false);
  lc.setChar(2, 0, ' ', false);
  // Display 3:
  lc.setChar(3, 7, ' ', false);
  lc.setChar(3, 6, ' ', false);
  lc.setChar(3, 5, ' ', false);
  lc.setDigit(3, 4, getValue(CodeVersion, '.', 0).toInt(), true);   // Show code verssion number
  lc.setDigit(3, 3, getValue(CodeVersion, '.', 1).toInt(), true);   // Show code verssion number
  lc.setDigit(3, 2, getValue(CodeVersion, '.', 2).toInt(), false);  // Show code verssion number
  lc.setChar(3, 1, ' ', false);
  lc.setChar(3, 0, ' ', false);
  delay(5000);
  lc.clearMatrix();

  // LED Display Test:
  if (checkLEDs == 1) {
    for (int i = 0; i <= 7; i++) {
      lc.setDigit(0, i, i, false);
      lc.setDigit(1, i, i, false);
      lc.setDigit(2, i, i, false);
      lc.setDigit(3, i, i, false);
      delay(250);
    }
    lc.clearMatrix();
    delay(250);
    for (int i = 7; i >= 0; i--) {
      lc.setDigit(0, i, 8, true);
      lc.setDigit(1, i, 8, true);
      lc.setDigit(2, i, 8, true);
      lc.setDigit(3, i, 8, true);
      delay(250);
    }
    lc.clearMatrix();
    delay(250);
  }

  // Startup:
  if (checkLEDs == 1) testLEDs();          // LED test
  checkBME280();                           // BME280 init
  if (checkFANs == 1) checkFANsOnStart();  // Check fans on startup
  if (fanerror == 0) {
    WIFI_SETUP();  // WiFi setup
    // Telegram:
    if (useTelegram == 1) {                                 // Use Telegram support and send initial message after startup
      secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
      delay(3000);
      Serial.println("Send initial Telegram message: " + TelegramName + " - Version: " + CodeVersion + " startup finished");
      bot.sendMessage(CHAT_ID.c_str(), TelegramName + " - Version: " + CodeVersion + " startup finished " + "\xF0\x9F\x98\x8A", "");
      Serial.println("###############################################");
    }

    // Set LEDs to ON or OFF on startup:
    if (setLEDonStart == 1) {
      setLEDs(redVal, greenVal, blueVal, 0, 0, 1);  // Set the LEDs to ON and the default LED color
    } else {
      setLEDs(0, 0, 0, 0, 0, 1);  // OFF
    }
  }
}


// ###########################################################################################################################################
// # Loop function during runtime:
// ###########################################################################################################################################
void loop() {
  if (WiFIsetup == true) {  // Check for WIFI setup
    // Buttons:
    // ########
    button1.loop();  // MUST call the loop() function first
    button2.loop();  // MUST call the loop() function first
    // Get button state after debounce
    int button1_state = button1.getState();  // the state after debounce
    int button2_state = button2.getState();  // the state after debounce
    // Button 1 actions:
    if (button1.isPressed()) {
      if (logoutput == 1) Serial.println("The button 1 'ON' is pressed");
      setLEDs(redVal, greenVal, blueVal, 0, 0, 1);  // ON
    }
    if (logoutput == 1) {
      if (button1.isReleased()) Serial.println("The button 1 'ON' is released");
    }
    // Button 2 actions:
    if (button2.isPressed()) {
      if (logoutput == 1) Serial.println("The button 2 'OFF' is pressed");
      setLEDs(0, 0, 0, 0, 0, 1);  // OFF
    }
    if (logoutput == 1) {
      if (button2.isReleased()) Serial.println("The button 2 'OFF' is released");
    }

    // Other tasks:
    // ############
    if (fanerror == 0) {  // Just start to work if fan test was OK
      if (millis() - bot_lasttime > BOT_MTBS) {

        if (updatedevice == true) {                     // Allow display updates (normal usage)
          if (changedvalues == true) setFlashValues();  // Write settings to flash

          // Temperature and humidity:
          getTempHum();  // Get the temperature and humidity

          // Display data:
          writeOn7Segment();  // Show data on the display

          // Control fans:
          setANDgetFanSpeeds();  // Set and get fan speed

          // Web server:
          dnsServer.processNextRequest();  // Update the web server

          // Telegram:
          if (useTelegram == 1) {  // Telegram chat action command usage
            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            while (numNewMessages) {
              handleNewMessages(numNewMessages);
              numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            }
          }
        }

        if (updatemode == 1) otaserver.handleClient();  // ESP32 OTA update

        bot_lasttime = millis();
      }
    }
  }
}


// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);

  // Section General:
  // ################
  ESPUI.separator("General:");

  // Welcome label:
  ESPUI.label("AWSW Fans N Lights configuration", ControlColor::None, "Welcome to the AWSW Fans N Lights configuration. Here you can adjust your settings to your personal needs.");

  // Version:
  ESPUI.label("Software version", ControlColor::None, CodeVersion);


  // Section LED settings:
  // #####################
  ESPUI.separator("LED settings:");

  // Cabin LED color selector:
  char hex_color[7] = { 0 };
  sprintf(hex_color, "#%02X%02X%02X", redVal, greenVal, blueVal);
  color_LEDs = ESPUI.text("LED color", colCallLEDs, ControlColor::Dark, hex_color);
  ESPUI.setInputType(color_LEDs, "color");

  // Intensity LEDs slider selector:
  ESPUI.slider("Brightness of the cabin LEDs", &sliderBrightnessLEDs, ControlColor::Dark, intensityLEDs, 0, 255);

  // Turn LEDs on startup ON or OFF:
  ESPUI.switcher("Turn cabin LEDs ON on startup", &switchSetLEDonStart, ControlColor::Dark, setLEDonStart);

  // Turn LED on:
  ESPUI.button("Turn LEDs ON", &buttonLEDsOn, ControlColor::Dark, "Turn LEDs ON");

  // Turn LED off:
  ESPUI.button("Turn LEDs OFF", &buttonLEDsOff, ControlColor::Dark, "Turn LEDs OFF");



  // Section 7-segment displays:
  // ###########################
  ESPUI.separator("7-segment displays:");

  // Intensity 7-segment displays slider selector:
  ESPUI.slider("Brightness of the 7-segment displays", &sliderBrightness7Seg, ControlColor::Dark, intensity7Seg, 0, 8);

  // Use °C or °F values on display:
  int uiCorF = ESPUI.switcher("Use °F values on display (Work in progress)", &switchUseCorFtemp, ControlColor::Dark, useCorFtemp);
  ESPUI.setEnabled(uiCorF, false);



  // Section Fans:
  // #############
  ESPUI.separator("Fans:");

  // Minimum fan speed:
  ESPUI.slider("Fan minimum speed in %", &call_FanMin_select, ControlColor::Dark, MIN_FAN_SPEED, 5, 35);

  // Minimum fan temp:
  ESPUI.slider("Fan minimum temperature in °C for minimum speed", &call_TempMin_select, ControlColor::Dark, MIN_TEMP, 20, 60);

  // Maximum fan temp:
  ESPUI.slider("Fan maximum temperature in °C for maximum speed", &call_TempMax_select, ControlColor::Dark, MAX_TEMP, 20, 60);



  // Section Startup:
  // ################
  ESPUI.separator("Startup:");

  // Show IP-address on startup:
  ESPUI.switcher("Show IP-address on startup", &switchShowIP, ControlColor::Dark, useshowip);

  // Show LED test on startup:
  ESPUI.switcher("Use LED test on startup", &switchTestLEDs, ControlColor::Dark, checkLEDs);

  // Check fans on startup:
  ESPUI.switcher("Check fans on startup", &switchcheckFANs, ControlColor::Dark, checkFANs);



  // Section Prusa Connect:
  // ######################
  ESPUI.separator("Prusa Connect: (Work in progress)");

  // Use Prusa Connect:
  int uiPrusaConnect = ESPUI.switcher("Use Prusa Connect (change requires restart)", &switchUsePrusaConnnect, ControlColor::Dark, usePrusaConnnect);
  ESPUI.setEnabled(uiPrusaConnect, false);

  // Printer 1 name:
  int textP1 = ESPUI.text("Name of the printer 1 on Prusa Connect", callbackTextP1, ControlColor::Dark, String(printer1));
  ESPUI.addControl(ControlType::Max, "", "32", ControlColor::None, textP1);

  // Printer 2 name:
  int textP2 = ESPUI.text("Name of the printer 2 on Prusa Connect", callbackTextP2, ControlColor::Dark, String(printer2));
  ESPUI.addControl(ControlType::Max, "", "32", ControlColor::None, textP2);



  // Section Telegram:
  // #################
  ESPUI.separator("Telegram: (Work in progress)");

  // Use Telegram:
  int uiTelegram = ESPUI.switcher("Use Telegram (change requires restart)", &switchUseTelegram, ControlColor::Dark, useTelegram);
  ESPUI.setEnabled(uiTelegram, false);

  // Telegram name:
  int textTelegramName = ESPUI.text("Name of the device on Telegram", callbackTextTelegram, ControlColor::Dark, String(TelegramName));
  ESPUI.addControl(ControlType::Max, "", "32", ControlColor::None, textTelegramName);



  // Section WiFi:
  // #############
  ESPUI.separator("WiFi:");

  // WiFi SSID:
  ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

  // WiFi signal strength:
  ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

  // Hostname:
  ESPUI.label("Hostname in your router", ControlColor::Dark, "<a href='http://" + String(WiFi.getHostname()) + "' target='_blank' style='color:#ffffff;'>" + String(WiFi.getHostname()) + "</a>");

  // WiFi MAC-address:
  ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());

  // WiFi ip-address:
  ESPUI.label("IP-address", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + "' target='_blank' style='color:#ffffff;'>" + IpAddress2String(WiFi.localIP()) + "</a>");

  // WiFi DNS address:
  ESPUI.label("DNS address", ControlColor::Dark, IpAddress2String(WiFi.dnsIP()));

  // WiFi Gateway address:
  ESPUI.label("Gateway address", ControlColor::Dark, IpAddress2String(WiFi.gatewayIP()));



  // Section Update:
  // ###############
  ESPUI.separator("Update:");

  // Use the update function or not:
  ESPUI.switcher("Use the OTA update function (change requires restart)", &switchUpdateMode, ControlColor::Dark, updatemode);

  // Update URL:
  ESPUI.label("Update URLs", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":8080</a> or <a href='http://" + String(WiFi.getHostname()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + String(WiFi.getHostname()) + ":8080</a>");

  // AWSW software GitHub repository:
  ESPUI.label("Download newer software updates here", ControlColor::Dark, "<a href='https://github.com/AWSW-de/FansNLights' target='_blank' style='color:#ffffff;'>https://github.com/AWSW-de/FansNLights</a>");



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)3);

  // Reset ALL settings:
  ESPUI.button("Reset ALL settings", &buttonReset, ControlColor::Dark, "Reset ALL settings", (void*)4);

  // Show log output in Arduino IDE Serial Monitor:
  ESPUI.switcher("Show log output in Arduino IDE Serial Monitor", &switchLogoutput, ControlColor::Dark, logoutput);



  // Section License:
  // ####################
  ESPUI.separator("License information:");

  // License information:
  ESPUI.label("License information", ControlColor::Dark, "NonCommercial — You may not use the project for commercial purposes! © 2024 Copyright by <a href='https://github.com/AWSW-de/FansNLights' target='_blank' style='color:#ffffff;'>AWSW</a>");


  // Deploy the page:
  ESPUI.begin("AWSW Fans N Lights");
}


// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  redVal = preferences.getUInt("redVal", 255);
  greenVal = preferences.getUInt("greenVal", 255);
  blueVal = preferences.getUInt("blueVal", 255);
  setLEDonStart = preferences.getUInt("setLEDonStart", 0);
  checkLEDs = preferences.getUInt("checkLEDs", 0);
  useCorFtemp = preferences.getUInt("useCorFtemp", 0);
  updatemode = preferences.getUInt("updatemode", 1);
  checkFANs = preferences.getUInt("checkFANs", 0);
  intensityLEDs = preferences.getUInt("intensityLEDs", 32);
  intensity7Seg = preferences.getUInt("intensity7Seg", 4);
  useshowip = preferences.getUInt("useshowip", 1);
  usePrusaConnnect = preferences.getUInt("usePrusaConnect", 0);
  printer1 = preferences.getString("printer1", "Prusa MK3.5 - 2019");
  printer2 = preferences.getString("printer2", "Prusa MK3.5 - 2021");
  useTelegram = preferences.getUInt("useTelegram", 0);
  TelegramName = preferences.getString("TelegramName", "AWSW FANS N LIGHTS");
  MIN_FAN_SPEED = preferences.getUInt("MIN_FAN_SPEED", 20);
  MIN_TEMP = preferences.getUInt("MIN_TEMP", 25);
  MAX_TEMP = preferences.getUInt("MAX_TEMP", 30);
  BOTtoken = preferences.getString("iBotToken", "");
  CHAT_ID = preferences.getString("iChatID", "");
  bot.updateToken(String(BOTtoken));
  // useWiFiReCon = preferences.getUInt("useWiFiReCon", 1);
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  changedvalues = false;
  preferences.putUInt("redVal", redVal);
  preferences.putUInt("greenVal", greenVal);
  preferences.putUInt("blueVal", blueVal);
  preferences.putUInt("setLEDonStart", setLEDonStart);
  preferences.putUInt("checkLEDs", checkLEDs);
  preferences.putUInt("useCorFtemp", useCorFtemp);
  preferences.putUInt("updatemode", updatemode);
  preferences.putUInt("checkFANs", checkFANs);
  preferences.putUInt("intensityLEDs", intensityLEDs);
  preferences.putUInt("intensity7Seg", intensity7Seg);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("usePrusaConnect", usePrusaConnnect);
  preferences.putString("printer1", printer1);
  preferences.putString("printer2", printer2);
  preferences.putUInt("useTelegram", useTelegram);
  preferences.putString("TelegramName", TelegramName);
  preferences.putUInt("MIN_FAN_SPEED", MIN_FAN_SPEED);
  preferences.putUInt("MIN_TEMP", MIN_TEMP);
  preferences.putUInt("MAX_TEMP", MAX_TEMP);
  // preferences.putUInt("useWiFiReCon", useWiFiReCon);
}


// ###########################################################################################################################################
// # GUI: Fan minimum speed:
// ###########################################################################################################################################
void call_FanMin_select(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  MIN_FAN_SPEED = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Fan minimum temp:
// ###########################################################################################################################################
void call_TempMin_select(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  MIN_TEMP = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Fan naximmum temp:
// ###########################################################################################################################################
void call_TempMax_select(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  MAX_TEMP = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Printer 1 name:
// ###########################################################################################################################################
void callbackTextP1(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  printer1 = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Printer 2 name:
// ###########################################################################################################################################
void callbackTextP2(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  printer2 = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Project name on Telegram:
// ###########################################################################################################################################
void callbackTextTelegram(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  TelegramName = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Color change for background color:
// ###########################################################################################################################################
#define SLIDER_UPDATE_BACK 150  // Wait at least 100 ms before allowing another slider update --> Bug fix for color slider crashing ESP32
void colCallLEDs(Control* sender, int type) {
  static unsigned long last_slider_update = 0;  // Track the time of the last slider update
  if ((type == 10) && (millis() - last_slider_update >= SLIDER_UPDATE_BACK)) {
    getRGBforLEDs(sender->value);
    last_slider_update = millis();
  }
  return;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - BACKGROUND:
// ###########################################################################################################################################
void getRGBforLEDs(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal = red;
  greenVal = green;
  blueVal = blue;
  changedvalues = true;
  updatedevice = true;
  setLEDs(redVal, greenVal, blueVal, 0, 0, 1);
  strip.show();
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity:
// ###########################################################################################################################################
void sliderBrightnessLEDs(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensityLEDs = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
  strip.setBrightness(intensityLEDs);  // Set LED brightness
  setLEDs(redVal, greenVal, blueVal, 0, 0, 1);
  strip.show();
}


// ###########################################################################################################################################
// # GUI: Turn LEDs on:
// ###########################################################################################################################################
void buttonLEDsOn(Control* sender, int type) {
  updatedevice = false;
  delay(250);
  changedvalues = true;
  updatedevice = true;
  strip.setBrightness(intensityLEDs);  // Set LED brightness
  setLEDs(redVal, greenVal, blueVal, 0, 0, 1);
  strip.show();
}


// ###########################################################################################################################################
// # GUI: Turn LEDs off:
// ###########################################################################################################################################
void buttonLEDsOff(Control* sender, int type) {
  updatedevice = false;
  delay(250);
  changedvalues = true;
  updatedevice = true;
  strip.setBrightness(intensityLEDs);  // Set LED brightness
  setLEDs(0, 0, 0, 0, 0, 1);
  strip.show();
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity:
// ###########################################################################################################################################
void sliderBrightness7Seg(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity7Seg = sender->value.toInt();
  changedvalues = true;
  lc.setIntensity(intensity7Seg);  // Set the brightness
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - helper function:
// ###########################################################################################################################################
int hexcolorToInt(char upper, char lower) {
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  return uVal + lVal;
}


// ###########################################################################################################################################
// # GUI: Show Test LEDs on startup switch:
// ###########################################################################################################################################
void switchTestLEDs(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      checkLEDs = 1;
      break;
    case S_INACTIVE:
      checkLEDs = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use Telegram switch:
// ###########################################################################################################################################
void switchUseTelegram(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useTelegram = 1;
      break;
    case S_INACTIVE:
      useTelegram = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use Prusa Connect switch:
// ###########################################################################################################################################
void switchUsePrusaConnnect(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usePrusaConnnect = 1;
      break;
    case S_INACTIVE:
      usePrusaConnnect = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Show log output in Arduino IDE Serial Monitort switch:
// ###########################################################################################################################################
void switchLogoutput(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      logoutput = 1;
      break;
    case S_INACTIVE:
      logoutput = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use °C or °F values on display switch:
// ###########################################################################################################################################
void switchUseCorFtemp(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useCorFtemp = 1;  // °F
      break;
    case S_INACTIVE:
      useCorFtemp = 0;  // °C
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Set LEDs on startup ON or OFF switch:
// ###########################################################################################################################################
void switchSetLEDonStart(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      setLEDonStart = 1;
      break;
    case S_INACTIVE:
      setLEDonStart = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Set LEDs on startup ON or OFF switch:
// ###########################################################################################################################################
void switchUpdateMode(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      updatemode = 1;
      break;
    case S_INACTIVE:
      updatemode = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;

  // TOD: RESTART
}


// ###########################################################################################################################################
// # GUI: Check fans on startup switch:
// ###########################################################################################################################################
void switchcheckFANs(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      checkFANs = 1;
      break;
    case S_INACTIVE:
      checkFANs = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Restart:
// ###########################################################################################################################################
void buttonRestart(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(100);
  lc.clearMatrix();
  for (int i = 0; i <= 3; i++) {
    lc.setChar(i, 7, 'R', false);
    lc.setChar(i, 6, 'E', false);
    lc.setChar(i, 5, 'S', false);
    lc.setChar(i, 4, 'T', false);
    lc.setChar(i, 3, 'A', false);
    lc.setChar(i, 2, 'R', false);
    lc.setChar(i, 1, 'T', false);
    lc.setChar(i, 0, ' ', false);
  }
  delay(3000);
  setFlashValues();  // Write settings to flash
  preferences.end();
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Reset ALL the settings:
// ###########################################################################################################################################
void buttonReset(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(100);
  Serial.println("Status: SETTINGS RESET REQUEST EXECUTED");
  lc.clearMatrix();
  for (int i = 0; i <= 3; i++) {
    lc.setChar(i, 7, 'R', false);
    lc.setChar(i, 6, 'E', false);
    lc.setChar(i, 5, 'S', false);
    lc.setChar(i, 4, 'E', false);
    lc.setChar(i, 3, 'T', false);
    lc.setChar(i, 2, ' ', false);
    lc.setChar(i, 1, ' ', false);
    lc.setChar(i, 0, ' ', false);
  }
  delay(3000);
  preferences.clear();
  delay(100);
  preferences.end();
  Serial.println("####################################################################################################");
  Serial.println("# SETTING WERE SET TO DEFAULT... RESTARTING NOW... PLEASE CONFIGURE AGAIN...");
  Serial.println("####################################################################################################");
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Show IP-ADdress switch:
// ###########################################################################################################################################
void switchShowIP(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useshowip = 1;
      break;
    case S_INACTIVE:
      useshowip = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Telegram chat and massage handling:
// ###########################################################################################################################################
void handleNewMessages(int numNewMessages) {
  // Serial.println("Telegram handle new messages: " + String(numNewMessages));

  // Icons: https://apps.timwhitlock.info/emoji/tables/unicode

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println("Telegram message received: " + text);

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    // React to Prusa Connect messages to your 3D printers:
    if (usePrusaConnnect == 1) {
      // Printer 1:
      if (text.indexOf(printer1) != -1) {
        Serial.println("Message received from " + printer1);
        if (text.indexOf("finished") != -1) {
          Serial.println("Message received from " + printer1 + " - Print job has finished. \xE2\x9C\x85");
          for (int i = 1; i <= 10; i++) {
            setLEDs(0, 255, 0, 250, 0, 1);  // Green
            setLEDs(0, 0, 0, 250, 0, 1);    // Off
          }
          setLEDs(0, 255, 0, 250, 0, 1);  // Green
        }
        if (text.indexOf("attention") != -1) {
          Serial.println("Message received from " + printer1 + " - Printer needs manually attention!. \xE2\x80\xBC");
          for (int i = 1; i <= 10; i++) {
            setLEDs(255, 0, 0, 250, 0, 1);  // Red
            setLEDs(0, 0, 0, 250, 0, 1);    // Off
          }
          setLEDs(255, 0, 0, 250, 0, 1);  // Red
        }
      }

      // Printer 2:
      if (text.indexOf(printer2) != -1) {
        Serial.println("Message received from " + printer2);
        if (text.indexOf("finished") != -1) {
          Serial.println("Message received from " + printer2 + " - Print job has finished. \xE2\x9C\x85");
          for (int i = 1; i <= 10; i++) {
            setLEDs(0, 255, 0, 250, 0, 1);  // Green
            setLEDs(0, 0, 0, 250, 0, 1);    // Off
          }
          setLEDs(0, 255, 0, 250, 0, 1);  // Green
        }
        if (text.indexOf("attention") != -1) {
          Serial.println("Message received from " + printer2 + " - Printer needs manually attention!. \xE2\x80\xBC");
          for (int i = 1; i <= 10; i++) {
            setLEDs(255, 0, 0, 250, 0, 1);  // Red
            setLEDs(0, 0, 0, 250, 0, 1);    // Off
          }
          setLEDs(255, 0, 0, 250, 0, 1);  // Red
        }
      }
    }

    //##################################################################
    // Send respond messages to commands:
    //##################################################################
    if (text == "/LEDsON") {
      bot.sendMessage(chat_id, TelegramName + ": LEDs set to ON");
      setLEDs(redVal, greenVal, blueVal, 0, 0, 1);  // Default color
    }

    if (text == "/LEDsOFF") {
      bot.sendMessage(chat_id, TelegramName + ": LEDs set to OFF");
      setLEDs(0, 0, 0, 1000, 0, 1);  // Off
    }

    if (text == "/test") {
      bot.sendMessage(chat_id, TelegramName + " Telegram bot received test message: " + text);
    }

    if (text == "/start") {
      String welcome = "Welcome to " + TelegramName + " Telegram bot, " + from_name + ". " + "\xF0\x9F\x98\x8A \n";
      welcome += "Use /LEDsON, /LEDsOFF to control your " + TelegramName + " device\n\n";
      bot.sendMessage(chat_id, welcome);
    }
  }
}


// ###########################################################################################################################################
// # Set and get fan speed function:
// ###########################################################################################################################################
int actualFan1SpeedRpm, actualFan2SpeedRpm;
void setANDgetFanSpeeds() {
  // Calculate the required fan speed:
  if (temp < MIN_TEMP) {
    SpeedForAllFans = 0;
  } else if (temp > MAX_TEMP) {
    SpeedForAllFans = 100;
  } else {
    SpeedForAllFans = (100 - MIN_FAN_SPEED) * (temp - MIN_TEMP) / (MAX_TEMP - MIN_TEMP) + MIN_FAN_SPEED;
  }

  // Set the fan speed:
  setFanSpeedPercent(SpeedForAllFans, FAN1_PIN);
  setFanSpeedPercent(SpeedForAllFans, FAN2_PIN);
  // Get the fan speed:
  actualFan1SpeedRpm = getFanSpeedRpm(SIGNAL1_PIN);
  actualFan2SpeedRpm = getFanSpeedRpm(SIGNAL2_PIN);

  // Serial print output:
  if (logoutput == 1) {
    Serial.println("Setting fan 1 speed to " + String(SpeedForAllFans) + " %");
    Serial.println("Fan 1 speed is " + String(actualFan1SpeedRpm) + " RPM");
    Serial.println("Setting fan 2 speed to " + String(SpeedForAllFans) + " %");
    Serial.println("Fan 2 speed is " + String(actualFan2SpeedRpm) + " RPM");
    Serial.println("###############################");
  }
}


// ###########################################################################################################################################
// # Set LED light function:
// ###########################################################################################################################################
// # Value 1: int value for R in RGB: 0-255
// # Value 2: int value for G in RGB: 0-255
// # Value 3: int value for B in RGB: 0-255
// # Value 4: int value for a delay in ms after all LEDs were set: 0-10000 (0-10 seconds)
// # Value 5: int value for a delay in ms after a single LED was set: 0-50 (0-50 milliseconds)
// # Value 6: int value for the direction the LEDs should be set: 1 = counting up; 0 = counting down
// ###########################################################################################################################################
void setLEDs(int colorR, int colorG, int colorB, int delayAllLED, int delayOneLED, int directionUPorDOWN) {
  if (directionUPorDOWN == 1) {
    // Count LEDs up
    for (int i = 0; i <= NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(colorR, colorG, colorB));
      strip.show();
      delay(delayOneLED);
    }
  } else {
    // Count LEDs down
    for (int i = NUMPIXELS; i >= 0; i--) {
      strip.setPixelColor(i, strip.Color(colorR, colorG, colorB));
      strip.show();
      delay(delayOneLED);
    }
  }
  delay(delayAllLED);
  strip.show();
}

// ###########################################################################################################################################
// # LED test function during startup:
// ###########################################################################################################################################
void testLEDs() {
  Serial.println("LED startup test");
  lc.clearMatrix();
  for (int i = 0; i <= 3; i++) {
    lc.setChar(i, 7, 'L', false);
    lc.setChar(i, 6, 'E', false);
    lc.setChar(i, 5, 'D', false);
    lc.setChar(i, 4, ' ', false);
    lc.setChar(i, 3, 'T', false);
    lc.setChar(i, 2, 'E', false);
    lc.setChar(i, 1, 'S', false);
    lc.setChar(i, 0, 'T', false);
  }
  setLEDs(0, 0, 0, 500, 25, 1);    // LEDs off
  setLEDs(255, 0, 0, 500, 0, 1);   // Red
  setLEDs(0, 0, 0, 500, 0, 1);     // LEDs off
  setLEDs(0, 255, 0, 500, 0, 0);   // Green
  setLEDs(0, 0, 0, 500, 0, 1);     // LEDs off
  setLEDs(0, 0, 255, 500, 0, 1);   // Blue
  setLEDs(0, 0, 0, 500, 0, 1);     // LEDs off
  setLEDs(255, 0, 0, 500, 25, 1);  // Red
  setLEDs(0, 255, 0, 500, 25, 0);  // Green
  setLEDs(0, 0, 255, 500, 25, 1);  // Blue
  // setLEDs(0, 255, 255, 500, 25, 0);  // Cyan
  // setLEDs(0, 0, 0, 500, 25, 1);      // LEDs off
  // setLEDs(0, 255, 255, 500, 0, 0);   // Cyan
  // setLEDs(0, 0, 0, 500, 0, 1);       // LEDs off
  // setLEDs(0, 255, 255, 500, 0, 0);   // Cyan
  // setLEDs(0, 0, 0, 500, 0, 1);       // LEDs off
  // setLEDs(0, 255, 255, 500, 0, 0);   // Cyan
  // setLEDs(0, 0, 0, 500, 0, 1);       // LEDs off
}


// ###########################################################################################################################################
// # Check the BME280 connection on startup:
// ###########################################################################################################################################
int restartBME = 1;
int restartBMEMAX = 30;
void checkBME280() {
  Serial.println("Searching the BME280 sensor");
  // lc.clearMatrix();
  // for (int i = 0; i <= 3; i++) {
  //   lc.setChar(i, 7, 'T', false);
  //   lc.setChar(i, 6, 'E', false);
  //   lc.setChar(i, 5, 'S', false);
  //   lc.setChar(i, 4, 'T', false);
  //   lc.setChar(i, 3, ' ', false);
  //   lc.setChar(i, 2, 'B', false);
  //   lc.setChar(i, 1, 'M', false);
  //   lc.setChar(i, 0, 'E', false);
  // }
  // delay(1000);
  bool status = bme.begin(0x76);
  if (!status) {
    while (1) {
      Serial.println("Could not find the BME280 sensor. Check your wiring setup! Check " + String(restartBME) + " of " + String(restartBMEMAX) + " before automatic restart!");
      setLEDs(255, 0, 0, 500, 25, 1);  // Red
      setLEDs(0, 0, 255, 500, 25, 0);  // Blue
      // Display 1:
      lc.setChar(0, 7, ' ', false);
      lc.setChar(0, 6, 'E', false);
      lc.setChar(0, 5, 'R', false);
      lc.setChar(0, 4, 'R', false);
      lc.setChar(0, 3, 'O', false);
      lc.setChar(0, 2, 'R', false);
      lc.setChar(0, 1, ' ', false);
      lc.setChar(0, 0, ' ', false);
      // Display 2:
      lc.setChar(1, 7, 'B', false);
      lc.setChar(1, 6, 'M', false);
      lc.setChar(1, 5, 'E', false);
      lc.setDigit(1, 4, 2, false);
      lc.setDigit(1, 3, 8, false);
      lc.setDigit(1, 2, 0, false);
      lc.setChar(1, 1, ' ', false);
      lc.setChar(1, 0, 'N', false);
      // Display 3:
      lc.setChar(2, 7, 'O', false);
      lc.setChar(2, 6, 'T', false);
      lc.setChar(2, 5, ' ', false);
      lc.setChar(2, 4, 'F', false);
      lc.setChar(2, 3, 'O', false);
      lc.setChar(2, 2, 'U', false);
      lc.setChar(2, 1, 'N', false);
      lc.setChar(2, 0, 'D', false);
      // Display 4:
      lc.setChar(3, 7, ' ', false);
      lc.setChar(3, 6, 'R', false);
      lc.setChar(3, 5, 'E', false);
      lc.setChar(3, 4, 'S', false);
      lc.setChar(3, 3, 'T', false);
      lc.setChar(3, 2, 'A', false);
      lc.setChar(3, 1, 'R', false);
      lc.setChar(3, 0, 'T', false);
      delay(1000);
      lc.clearMatrix();
      restartBME = restartBME + 1;
      if (restartBME >= restartBMEMAX + 1) ESP.restart();
    }
  }
  Serial.println("Found the BME280 sensor");
  Serial.println("###############################################");
}


// ###########################################################################################################################################
// # Check functionalty of the fans:
// ###########################################################################################################################################
void checkFANsOnStart() {
  lc.clearMatrix();
  // Display 2:
  lc.setChar(1, 7, 'F', false);
  lc.setChar(1, 6, 'A', false);
  lc.setChar(1, 5, 'N', false);
  lc.setChar(1, 4, ' ', false);
  lc.setChar(1, 3, 'T', false);
  lc.setChar(1, 2, 'E', false);
  lc.setChar(1, 1, 'S', false);
  lc.setChar(1, 0, 'T', false);

  // CHECK FAN1:
  // Set the fan speed:
  setFanSpeedPercent(100, FAN1_PIN);
  // Wait to let the fan speed up:
  delay(5000);
  // Get the fan speed:
  int actualFan1SpeedRpm = getFanSpeedRpm(SIGNAL1_PIN);
  if (actualFan1SpeedRpm == 0) {
    Serial.println("Fan 1 error");
    fanerror = 1;
    // Display 1:
    lc.setChar(0, 7, 'F', false);
    lc.setChar(0, 6, 'A', false);
    lc.setChar(0, 5, 'N', false);
    lc.setChar(0, 4, ' ', false);
    lc.setChar(0, 3, 'F', false);
    lc.setChar(0, 2, 'A', false);
    lc.setChar(0, 1, 'I', false);
    lc.setChar(0, 0, 'L', false);
    // Display 3:
    lc.setChar(2, 7, 'E', false);
    lc.setChar(2, 6, 'R', false);
    lc.setChar(2, 5, 'R', false);
    lc.setChar(2, 4, 'O', false);
    lc.setChar(2, 3, 'R', false);
    lc.setChar(2, 2, ' ', false);
    lc.setChar(2, 1, ' ', false);
    lc.setChar(2, 0, ' ', false);
    delay(1000);
  } else {
    Serial.println("Fan 1 OK");
    // Display 1:
    lc.setChar(0, 7, 'F', false);
    lc.setChar(0, 6, 'A', false);
    lc.setChar(0, 5, 'N', false);
    lc.setChar(0, 4, ' ', false);
    lc.setChar(0, 3, '1', false);
    lc.setChar(0, 2, ' ', false);
    lc.setChar(0, 1, 'O', false);
    lc.setChar(0, 0, 'K', false);
    // Display 3:
    lc.setChar(2, 7, 'T', false);
    lc.setChar(2, 6, 'E', false);
    lc.setChar(2, 5, 'S', false);
    lc.setChar(2, 4, 'T', false);
    lc.setChar(2, 3, ' ', false);
    lc.setChar(2, 2, 'O', false);
    lc.setChar(2, 1, 'K', false);
    lc.setChar(2, 0, ' ', false);
    delay(1000);
  }
  setFanSpeedPercent(0, FAN1_PIN);
  delay(5000);
  // #############
  // CHECK FAN2:
  setFanSpeedPercent(100, FAN2_PIN);
  // Wait to let the fan speed up:
  delay(5000);
  // Get the fan speed:
  int actualFan2SpeedRpm = getFanSpeedRpm(SIGNAL2_PIN);
  if (actualFan2SpeedRpm == 0) {
    Serial.println("Fan 2 error");
    fanerror = 1;
    // Display 3:
    lc.setChar(2, 7, 'E', false);
    lc.setChar(2, 6, 'R', false);
    lc.setChar(2, 5, 'R', false);
    lc.setChar(2, 4, 'O', false);
    lc.setChar(2, 3, 'R', false);
    lc.setChar(2, 2, ' ', false);
    lc.setChar(2, 1, ' ', false);
    lc.setChar(2, 0, ' ', false);
    // Display 4:
    lc.setChar(3, 7, 'F', false);
    lc.setChar(3, 6, 'A', false);
    lc.setChar(3, 5, 'N', false);
    lc.setChar(3, 4, ' ', false);
    lc.setChar(3, 3, 'F', false);
    lc.setChar(3, 2, 'A', false);
    lc.setChar(3, 1, 'I', false);
    lc.setChar(3, 0, 'L', false);
    delay(1000);
  } else {
    Serial.println("Fan 2 OK");
    if (fanerror == 0) {
      // Display 3:
      lc.setChar(2, 7, 'T', false);
      lc.setChar(2, 6, 'E', false);
      lc.setChar(2, 5, 'S', false);
      lc.setChar(2, 4, 'T', false);
      lc.setChar(2, 3, ' ', false);
      lc.setChar(2, 2, 'O', false);
      lc.setChar(2, 1, 'K', false);
      lc.setChar(2, 0, ' ', false);
    }
    // Display 4:
    lc.setChar(3, 7, 'F', false);
    lc.setChar(3, 6, 'A', false);
    lc.setChar(3, 5, 'N', false);
    lc.setChar(3, 4, ' ', false);
    lc.setChar(3, 3, '2', false);
    lc.setChar(3, 2, ' ', false);
    lc.setChar(3, 1, 'O', false);
    lc.setChar(3, 0, 'K', false);
    delay(1000);
  }
  setFanSpeedPercent(0, FAN2_PIN);

  // Get results and set LEDs:
  if (fanerror == 0) {
    setLEDs(0, 255, 0, 0, 0, 1);  // Green
  } else {
    setLEDs(255, 0, 0, 0, 0, 1);  // Red
  }
  delay(10000);
}


// ###########################################################################################################################################
// # Get fan speed function:
// ###########################################################################################################################################
int getFanSpeedRpm(int fanpin) {
  int highTime = pulseIn(fanpin, HIGH);
  int lowTime = pulseIn(fanpin, LOW);
  int period = highTime + lowTime;
  if (period == 0) {
    return 0;
  }
  float freq = 1000000.0 / (float)period;
  return (freq * 60.0) / 2.0;  // 2 cycles per revolution
}


// ###########################################################################################################################################
// # Set fan speed function:
// ###########################################################################################################################################
void setFanSpeedPercent(int p, int fanpin) {
  int value = (p / 100.0) * 255;
  analogWrite(fanpin, value);
}


// ###########################################################################################################################################
// # Get temperature and humidity function:
// ###########################################################################################################################################
void getTempHum() {
  // Get the teperature:
  temp = bme.readTemperature();
  if (useCorFtemp == 0) {
    // Use temperature format in Celsius:
    if (logoutput == 1) Serial.println("Temperature is " + String(temp) + " °C");
  } else {
    // Convert temperature to Fahrenheit
    if (logoutput == 1) Serial.println("Temperature is " + String(1.8 * temp + 32) + " °F");
    temp = 1.8 * temp + 32;
  }
  // Get the humidity:
  hum = bme.readHumidity();
  if (logoutput == 1) Serial.println("Humidity is " + String(hum) + " %");

  // Set data for the display:
  double iTemp, iHum;
  iTemp = double(temp);
  String dataTemp = String(iTemp);

  // Split value by . sign:
  String integralPartTemp = getValue(dataTemp, '.', 0);
  String decimalPartTemp = getValue(dataTemp, '.', 1);
  // Set value to in to be displayed:
  iTempInt1 = integralPartTemp.toInt();
  iTempInt2 = decimalPartTemp.toInt();
  // if (logoutput == 1) Serial.println(iTempInt1);
  // if (logoutput == 1) Serial.println(iTempInt2);
}


// ###########################################################################################################################################
// # Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Setup AWSW FANS N LIGHTS</title>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 15px;
        box-shadow: 0 9px #999;
      }
      button:hover {
        background-color: #3e8e41;
      }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script type="text/javascript">
      function disableButtonAndSubmit() {
        var btn = document.getElementById("submitButton");
        btn.disabled = true;
        setTimeout(function() {
          document.forms["myForm"].submit();
        }, 100);
      }
    </script>
  </head>
  <body>
    <form action="/start" name="myForm">
      <center>
        <h1>Welcome to the AWSW FANS N LIGHTS setup</h1>
        <p>Please add your local WiFi credentials and set your Telegram IDs on the next page</p>
        <p><button id="submitButton" type="submit" onclick="disableButtonAndSubmit()">Configure AWSW FANS N LIGHTS</button></p>
      </center>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>Setup AWSW FANS N LIGHTS</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script language="JavaScript">
      function updateSSIDInput() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect && ssidSelect.options.length > 0) {
          document.getElementById("mySSID").value = ssidSelect.options[ssidSelect.selectedIndex].value;
        }
      }
       function validateForm() {
        var errorParagraph = document.querySelector('.error');
        errorParagraph.style.display = 'none';
        errorParagraph.innerHTML = '';
        if (document.forms["myForm"]["mySSID"].value == "") {
          errorParagraph.innerHTML = "WiFi SSID must be set. ";
          errorParagraph.style.display = 'block';
          return false;
        }
        if (document.forms["myForm"]["myPW"].value == "") {
          errorParagraph.innerHTML = "WiFi password must be set. ";
          errorParagraph.style.display = 'block'; 
          return false;
        }
        return true;
      }
      function disableButtonAndSubmit() {
        if (validateForm()) {
          var btn = document.getElementById("submitButton");
          btn.innerText = 'Restarting AWSW FANS N LIGHTS...';
          btn.disabled = true;
          setTimeout(function() {
            document.forms["myForm"].submit();
          }, 1000);
        }
      }
      window.onload = function() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect) {
          ssidSelect.addEventListener('change', updateSSIDInput);
        }
      };
    </script>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      p.error { 
        color: #ff0000; 
        display: none;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 15px;
        box-shadow: 0 9px #999;
      }
      button:hover { background-color: #3e8e41 }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
  </head>
  <body>
    <form action="/get" name="myForm" onsubmit="return validateForm()">
      <h1>Initial AWSW FANS N LIGHTS setup:</h1>
      <!-- Select element will be dynamically added here -->
      <p>
        <label for="mySSID">Enter your WiFi SSID:</label><br />
        <input id="mySSID" name="mySSID" value="" />
      </p>
      <p>
        <label for="myPW">Enter your WiFi password:</label><br/>
        <input type="text" id="myPW" name="myPW" value="" />
      </p>
      <p>
       <br/><br/>
       <label for="myBotToken">Enter your Telegram bot token:</label><br/>
       <input type="text" id="myBotToken" name="myBotToken" value="XXXXXXXXXX:YYYYYYYYYYYYYYY-ZZZZZZZZZZZZZZZZZZZZ" style="width: 400px;" /><br/><br/>
       <label for="myChatID">Enter your Telegram chat id:</label><br/>
       <input type="text" id="myChatID" name="myChatID" value="1234512345" style="width: 200px;" /><br/><br/>
      </p>
      <p class="error">Errors will be displayed here!</p>
      <p>
        <button id="submitButton" onclick="disableButtonAndSubmit()">Save values</button>
      </p>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>Setup AWSW FANS N LIGHTS</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
    </style>
  </head>
  <body>
    <h1>Settings saved!</h1>
    <p>AWSW FANS N LIGHTS is now trying to connect to the selected WiFi.</p>
    <p>First the WiFi leds will be lit blue and change to green in case of a successful WiFi connection.</p>
    <p>If the connection fails the WiFi leds will flash red. Then please reconnect to the temporary access point again.</p>
    <p>Please close this page now, rejoin your selected WiFi and enjoy your AWSW FANS N LIGHTS. =)</p>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
String ScanWiFi() {
  String html = config_html;
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println("Scan WiFi networks - END");
  Serial.println(" ");
  if (n > 0) {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    String ssidList = "<p><label for=\"mySSISelect\">Found these networks:</label><br /><select id=\"mySSIDSelect\" name=\"mySSIDSelect\"><option value=\"\" disabled selected>Choose...</option>";
    for (int i = 0; i < n; ++i) {
      ssidList += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*");
    }
    ssidList += "</select></p>";
    html.replace("<!-- Select element will be dynamically added here -->", ssidList);
  } else {
    Serial.println("No WiFi networks found");
  }
  return html;
}


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const char* PARAM_INPUT_4 = "myBotToken";
const char* PARAM_INPUT_5 = "myChatID";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePotalSetup() {
  String htmlConfigContent = ScanWiFi();
  const char* temp_ssid = "AWSW FANS N LIGHTS";
  const char* temp_password = "";
  WiFi.softAP(temp_ssid, temp_password);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("#################################################################################################################################################################################");
  Serial.print("# Temporary WiFi access point initialized. Please connect to the WiFi access point now and set your local WiFi credentials. Access point name: ");
  Serial.println(temp_ssid);
  Serial.print("# In case your browser does not open the AWSW FANS N LIGHTS setup page automatically after connecting to the access point, please navigate to this URL manually: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("#################################################################################################################################################################################");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {

      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      if (logoutput == 1) Serial.println(inputMessage);
      preferences.putString("WIFIssid", inputMessage);  // Save entered WiFi SSID

      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      if (logoutput == 1) Serial.println(inputMessage);
      preferences.putString("WIFIpass", inputMessage);  // Save entered WiFi password

      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      if (logoutput == 1) Serial.println(inputMessage);
      preferences.putString("iBotToken", inputMessage);  // Save entered Telegram bot token

      inputMessage = request->getParam(PARAM_INPUT_5)->value();
      inputParam = PARAM_INPUT_5;
      if (logoutput == 1) Serial.println(inputMessage);
      preferences.putString("iChatID", inputMessage);  // Save entered Telegram chat id

      delay(250);
      preferences.end();
    } else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send_P(200, "text/html", saved_html);
    // ResetTextLEDs(strip.Color(0, 255, 0));
    delay(1000);
    ESP.restart();
  });

  server.on("/start", HTTP_GET, [htmlConfigContent](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", htmlConfigContent.c_str());
  });

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("onnotfound ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  Serial.println("AWSW FANS N LIGHTS Captive Portal web server started");
}


// ###########################################################################################################################################
// # Wifi setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_SETUP() {
  int maxWiFiconnctiontries = 30;
  Serial.println(" ");
  esp_log_level_set("wifi", ESP_LOG_WARN);  // Disable WiFi debug warnings
  String WIFIssid = preferences.getString("WIFIssid");
  bool WiFiConfigEmpty = false;
  if (WIFIssid == "") {
    WiFiConfigEmpty = true;
  }
  String WIFIpass = preferences.getString("WIFIpass");
  if (WIFIpass == "") {
    WiFiConfigEmpty = true;
  }
  if (WiFiConfigEmpty == true) {
    setLEDs(0, 255, 255, 0, 0, 1);  // Cyan
    Serial.println("Show SET WIFI...");
    for (int y = 0; y <= 3; y++) {
      lc.setChar(y, 7, 'S', false);
      lc.setChar(y, 6, 'E', false);
      lc.setChar(y, 5, 'T', false);
      lc.setChar(y, 4, ' ', false);
      lc.setChar(y, 3, 'W', false);
      lc.setChar(y, 2, 'I', false);
      lc.setChar(y, 1, 'F', false);
      lc.setChar(y, 0, 'I', false);
    }
    CaptivePotalSetup();
  } else {
    Serial.println("Try to connect to found WiFi configuration: ");
    WiFi.disconnect();
    int tryCount = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
    Serial.println("Connecting to WiFi " + String(WIFIssid));
    while (WiFi.status() != WL_CONNECTED) {
      setLEDs(0, 0, 255, 0, 0, 1);  // Blue
      // Display Process:
      for (int y = 7; y >= 0; y--) {
        lc.setChar(0, 7, ' ', false);
        lc.setChar(0, 6, ' ', false);
        lc.setChar(0, 5, 'W', false);
        lc.setChar(0, 4, 'I', false);
        lc.setChar(0, 3, 'F', false);
        lc.setChar(0, 2, 'I', false);
        lc.setChar(0, 1, ' ', false);
        lc.setChar(0, 0, ' ', false);
        delay(25);
      }
      for (int i = 7; i >= 0; i--) {
        lc.setChar(1, i, '-', false);
        lc.setChar(2, i, '-', false);
        lc.setChar(3, i, '-', false);
        delay(25);
      }
      delay(1000);
      lc.clearMatrix();
      delay(250);
      tryCount = tryCount + 1;
      Serial.print("Connection try #: ");
      Serial.print(tryCount);
      Serial.print(" of maximum ");
      Serial.println(maxWiFiconnctiontries);
      if (tryCount >= maxWiFiconnctiontries - 5) {
        setLEDs(255, 0, 0, 0, 0, 1);  // Red
        // Display Process:
        lc.clearMatrix();
        delay(1000);
        // Display 1:
        lc.setChar(0, 7, 'C', false);
        lc.setChar(0, 6, 'O', false);
        lc.setChar(0, 5, 'N', false);
        lc.setChar(0, 4, 'N', false);
        lc.setChar(0, 3, 'E', false);
        lc.setChar(0, 2, 'C', false);
        lc.setChar(0, 1, 'T', false);
        lc.setChar(0, 0, ' ', false);
        // Display 2:
        lc.setChar(1, 7, 'T', false);
        lc.setChar(1, 6, 'O', false);
        lc.setChar(1, 5, ' ', false);
        lc.setChar(1, 4, 'W', false);
        lc.setChar(1, 3, 'I', false);
        lc.setChar(1, 2, 'F', false);
        lc.setChar(1, 1, 'I', false);
        lc.setChar(1, 0, ' ', false);
        // Display 3:
        lc.setChar(2, 7, 'F', false);
        lc.setChar(2, 6, 'A', false);
        lc.setChar(2, 5, 'I', false);
        lc.setChar(2, 4, 'L', false);
        lc.setChar(2, 3, 'E', false);
        lc.setChar(2, 2, 'D', false);
        lc.setChar(2, 1, ' ', false);
        lc.setChar(2, 0, ' ', false);
        // Display 4:
        lc.setChar(3, 7, ' ', false);
        lc.setChar(3, 6, ' ', false);
        lc.setChar(3, 5, ' ', false);
        lc.setChar(3, 4, ' ', false);
        lc.setChar(3, 3, ' ', false);
        lc.setChar(3, 2, ' ', false);
        lc.setChar(3, 1, ' ', false);
        lc.setChar(3, 0, ' ', false);
        delay(1000);
        lc.clearMatrix();
        delay(1000);
      }
      if (tryCount == maxWiFiconnctiontries) {
        Serial.println("\n\nWIFI CONNECTION ERROR: If the connection still can not be established please check the WiFi settings or location of the device.\n\n");
        preferences.putString("WIFIssid", "");  // Reset entered WiFi ssid
        preferences.putString("WIFIpass", "");  // Reset entered WiFi password
        preferences.end();
        delay(250);
        Serial.println("WiFi settings deleted because in " + String(maxWiFiconnctiontries) + " tries the WiFi connection could not be established. Temporary AWSW FANS N LIGHTS access point will be started to reconfigure WiFi again.");
        ESP.restart();
      }
      delay(1000);
      setLEDs(0, 0, 0, 0, 0, 1);  // LEDs off
      delay(1000);
    }
    Serial.println(" ");
    WiFIsetup = true;
    Serial.print("Successfully connected now to WiFi SSID: ");
    Serial.println(WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("DNS: " + WiFi.dnsIP().toString());
    Serial.println("GW: " + WiFi.gatewayIP().toString());
    Serial.println("ESP32 hostname: " + String(WiFi.getHostname()));
    setLEDs(0, 255, 0, 0, 0, 1);  // Green
    // Display Process:
    lc.clearMatrix();
    delay(250);
    // Display 1:
    lc.setChar(0, 7, 'C', false);
    lc.setChar(0, 6, 'O', false);
    lc.setChar(0, 5, 'N', false);
    lc.setChar(0, 4, 'N', false);
    lc.setChar(0, 3, 'E', false);
    lc.setChar(0, 2, 'C', false);
    lc.setChar(0, 1, 'T', false);
    lc.setChar(0, 0, ' ', false);
    // Display 2:
    lc.setChar(1, 7, 'T', false);
    lc.setChar(1, 6, 'O', false);
    lc.setChar(1, 5, ' ', false);
    lc.setChar(1, 4, 'W', false);
    lc.setChar(1, 3, 'I', false);
    lc.setChar(1, 2, 'F', false);
    lc.setChar(1, 1, 'I', false);
    lc.setChar(1, 0, ' ', false);
    // Display 3:
    lc.setChar(2, 7, 'S', false);
    lc.setChar(2, 6, 'U', false);
    lc.setChar(2, 5, 'C', false);
    lc.setChar(2, 4, 'C', false);
    lc.setChar(2, 3, 'E', false);
    lc.setChar(2, 2, 'S', false);
    lc.setChar(2, 1, 'S', false);
    lc.setChar(2, 0, 'U', false);
    // Display 4:
    lc.setChar(3, 7, 'L', false);
    lc.setChar(3, 6, 'L', false);
    lc.setChar(3, 5, 'Y', false);
    lc.setChar(3, 4, ' ', false);
    lc.setChar(3, 3, 'D', false);
    lc.setChar(3, 2, 'O', false);
    lc.setChar(3, 1, 'N', false);
    lc.setChar(3, 0, 'E', false);
    // lc.clearMatrix();
    delay(1000);

    // Show IP-address on the display:
    if (useshowip == 1) ShowIPaddress();

    // Web update:
    if (updatemode == 1) setupOTAupate();  // ESP32 OTA update

    // Start config web server:
    setupWebInterface();  // Generate the configuration page

    Serial.println("######################################################################");
    Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
    Serial.println("# Web interface online at: http://" + String(WiFi.getHostname()));
    Serial.println("######################################################################");
    Serial.println("# AWSW FANS N LIGHTS startup finished...");
    Serial.println("######################################################################");
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    updatenow = true;  // Update the display 1x after startup
  }
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
  lc.clearMatrix();
  delay(250);

  // Display 1:
  lc.setChar(0, 7, 'I', false);
  lc.setChar(0, 6, 'P', false);
  lc.setChar(0, 5, '-', false);
  lc.setChar(0, 4, 'A', false);
  lc.setChar(0, 3, 'd', false);
  lc.setChar(0, 2, 'd', false);
  lc.setChar(0, 1, 'r', true);
  lc.setChar(0, 0, ' ', false);

  // Display 2:
  lc.setChar(1, 7, ' ', false);
  lc.setChar(1, 6, getDigit(WiFi.localIP()[0], 2), false);
  lc.setChar(1, 5, getDigit(WiFi.localIP()[0], 1), false);
  lc.setChar(1, 4, getDigit(WiFi.localIP()[0], 0), true);
  lc.setChar(1, 3, getDigit(WiFi.localIP()[1], 2), false);
  lc.setChar(1, 2, getDigit(WiFi.localIP()[1], 1), false);
  lc.setChar(1, 1, getDigit(WiFi.localIP()[1], 0), true);
  lc.setChar(1, 0, ' ', false);

  // Display 3:
  lc.setChar(2, 7, ' ', false);
  lc.setChar(2, 6, getDigit(WiFi.localIP()[2], 2), false);
  lc.setChar(2, 5, getDigit(WiFi.localIP()[2], 1), false);
  lc.setChar(2, 4, getDigit(WiFi.localIP()[2], 0), true);
  lc.setChar(2, 3, getDigit(WiFi.localIP()[3], 2), false);
  lc.setChar(2, 2, getDigit(WiFi.localIP()[3], 1), false);
  lc.setChar(2, 1, getDigit(WiFi.localIP()[3], 0), false);
  lc.setChar(2, 0, ' ', false);

  // Display 4:
  lc.setChar(3, 7, 'h', false);
  lc.setChar(3, 6, 't', false);
  lc.setChar(3, 5, 't', false);
  lc.setChar(3, 4, 'p', false);
  lc.setChar(3, 3, ' ', false);
  lc.setChar(3, 2, 'o', false);
  lc.setChar(3, 1, 'n', false);
  lc.setChar(3, 0, ' ', false);
  delay(3000);
  lc.clearMatrix();
  delay(250);
}


// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}


// ###########################################################################################################################################
// # Split temperature value by . char:
// ###########################################################################################################################################
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// ###########################################################################################################################################
// Write on the display:
// ###########################################################################################################################################
void writeOn7Segment() {
  // Fan Speeds and Percent:
  // #######################
  // Fan 1: >>> Display 1
  // ####################
  lc.setChar(0, 7, (((int)actualFan1SpeedRpm / 1000) % 10), false);
  lc.setChar(0, 6, ((actualFan1SpeedRpm / 100) % 10), false);
  lc.setChar(0, 5, ((actualFan1SpeedRpm / 10) % 10), false);
  lc.setChar(0, 4, ((actualFan1SpeedRpm / 1) % 10), false);
  lc.setChar(0, 3, ' ', false);
  lc.setChar(0, 2, ((SpeedForAllFans / 100) % 10), false);
  lc.setChar(0, 1, ((SpeedForAllFans / 10) % 10), false);
  lc.setChar(0, 0, ((SpeedForAllFans / 1) % 10), false);

  // Temperature & Humidity: >>> Display 2
  // #####################################
  // Temperature:
  if ((int)iTempInt1 > 99) lc.setChar(1, 7, (((int)iTempInt1 / 100) % 10), false);
  else lc.setChar(1, 7, ' ', false);
  if ((int)iTempInt1 > 9) lc.setChar(1, 6, (((int)iTempInt1 / 10) % 10), false);
  else lc.setChar(1, 6, ' ', false);
  lc.setChar(1, 5, ((int)iTempInt1 % 10), true);
  lc.setChar(1, 4, (((int)iTempInt2 / 10) % 10), false);
  lc.setChar(1, 3, (((int)iTempInt2) % 10), false);
  // Humidity:
  if ((int)hum > 99) lc.setChar(1, 2, (((int)hum / 100) % 10), false);
  else lc.setChar(1, 2, ' ', false);
  if ((int)hum > 9) lc.setChar(1, 1, (((int)hum / 10) % 10), false);
  else lc.setChar(1, 1, ' ', false);
  lc.setChar(1, 0, ((int)hum % 10), false);

  // Status: >>> Display 3
  // ####################
  if (SpeedForAllFans != 0) {
    lc.setChar(2, 7, 'C', false);
    lc.setChar(2, 6, 'O', false);
    lc.setChar(2, 5, 'O', false);
    lc.setChar(2, 4, 'L', false);
    lc.setChar(2, 3, 'I', false);
    lc.setChar(2, 2, 'N', false);
    lc.setChar(2, 1, 'G', false);
    lc.setChar(2, 0, ' ', false);
  } else {
    lc.setChar(2, 7, 'S', false);
    lc.setChar(2, 6, 'L', false);
    lc.setChar(2, 5, 'E', false);
    lc.setChar(2, 4, 'E', false);
    lc.setChar(2, 3, 'P', false);
    lc.setChar(2, 2, 'I', false);
    lc.setChar(2, 1, 'N', false);
    lc.setChar(2, 0, 'G', false);
  }

  // Fan 2: >>> Display 4
  // ####################
  lc.setChar(3, 7, (((int)actualFan2SpeedRpm / 1000) % 10), false);
  lc.setChar(3, 6, ((actualFan2SpeedRpm / 100) % 10), false);
  lc.setChar(3, 5, ((actualFan2SpeedRpm / 10) % 10), false);
  lc.setChar(3, 4, ((actualFan2SpeedRpm / 1) % 10), false);
  lc.setChar(3, 3, ' ', false);
  lc.setChar(3, 2, ((SpeedForAllFans / 100) % 10), false);
  lc.setChar(3, 1, ((SpeedForAllFans / 10) % 10), false);
  lc.setChar(3, 0, ((SpeedForAllFans / 1) % 10), false);
}


// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char otaserverIndex[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>AWSW FANS N LIGHTS</title></head>
      <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <body>
    <form method='POST' action='/update' enctype='multipart/form-data'>
      <center><b><h1>AWSW FANS N LIGHTS software update</h1></b>
      <h2>Please select the in the Arduino IDE > "Sketch" ><br/>"Export Compiled Binary (Alt+Ctrl+S)"<br/>to generate the required "Code.ino.bin" file.<br/><br/>
      Select the "Code.ino.bin" file with the "Search" button.<br/><br/>
      Use the "Update" button to start the update.<br/><br/>The device will restart automatically.</h2><br/>
      <input type='file' name='update'>       <input type='submit' value='Update'>
     </center></form></body>
  </html>
 )=====";


const char otaNOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>AWSW FANS N LIGHTS</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>AWSW FANS N LIGHTS software update</h1></b>
      <h2>ERROR: Software update FAILED !!!<br/><br/>The device will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


const char otaOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>AWSW FANS N LIGHTS</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>AWSW FANS N LIGHTS software update</h1></b>
      <h2>Software update done =)<br/><br/>The device will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


void setupOTAupate() {
  otaserver.on("/", HTTP_GET, []() {
    otaserver.sendHeader("Connection", "close");
    otaserver.send(200, "text/html", otaserverIndex);
  });

  otaserver.on(
    "/update", HTTP_POST, []() {
      otaserver.sendHeader("Connection", "close");
      if (Update.hasError()) {
        otaserver.send(200, "text/html", otaNOK);
        setLEDs(255, 0, 0, 500, 0, 1);  // Red
      } else {
        otaserver.send(200, "text/html", otaOK);
        setLEDs(0, 255, 0, 500, 0, 1);  // Green
      }
      delay(3000);
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = otaserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update failed unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  otaserver.begin();
}