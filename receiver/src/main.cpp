#include <Arduino.h>
#include <string>
using namespace std;
#include <esp_now.h>
#include <WiFi.h>

// include necessary libraries for the modules to work
#include <SPI.h>              // the protocol used to communicate to the OLED display
#include <Wire.h>             // required for the Adafruit_SSD1306
#include <Adafruit_GFX.h>     // also required for the Adafruit_SSD1306 library
#include <Adafruit_SSD1306.h> // the library used to control the OLED display

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin for the OLED display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // the display object used to control the OLED display

void DisplayLoop(String message); // function declaration, the function will be written at the end of the file


#define macAddress "A0:B7:65:69:A8:4C" // this device's mac address

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  char *buff = (char *)data;
  String buffStr = String(buff);
  Serial.println(buffStr);
  DisplayLoop(buffStr);
}

void setup()
{
  delay(100);         // a delay to make sure the generation is complete before doing anything else
  Serial.begin(9600); // start the serial interface to report back through the USB for debugging
  while (!Serial)     // wait until serial is active before continuing
    ;
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // wait until the display object is active before continuing
    ;
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  display.clearDisplay();      // clear the display
  display.setTextSize(2);      // set the font size to x2
  display.setTextColor(WHITE); // set font color to white
  display.setCursor(15, 30);   // position curser in x = 15 and y = 30
  display.print("working!");   // print working in the display
  display.display();           // send the previous commands to the display
  delay(600);                  // wait to make sure the display is displaying correctly
}

void loop()
{

}

void DisplayLoop(String message){
  Serial.println(message);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message);
  display.display();
}