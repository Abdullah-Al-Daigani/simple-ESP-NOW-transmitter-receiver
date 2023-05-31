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

char KeyboardCheck(void);         // function declaration, the function will be written at the end of the file
void DisplayLoop(String message); // function declaration, the function will be written at the end of the file
void sendESPNOW(String message);  // function declaration, the function will be written at the end of the file

int colomn[4] = {32, 33, 25, 26}; // the colomn pin numbers
int row[4] = {27, 14, 12, 13};    // the row pin numbers
char buttons[4][4] = {            //
    {'1', '2', '3', 'A'},         //
    {'4', '5', '6', 'B'},         // the button values
    {'7', '8', '9', 'C'},         //
    {'*', '0', '#', 'D'}};        //

char lastChar = 'X';  // dummy variable to store the last numpad input, X is used as null
String message = "";  // String used to buffer the message before sending
int message_size = 0; // the size of the message, it is used in the Delete condition

#define macAddress "A0:B7:65:49:7B:9C"                             // this device's mac address
uint8_t broadcastAddress[] = {0xA0, 0xB7, 0x65, 0x69, 0xA8, 0x4C}; // the receiver's mac address
esp_now_peer_info_t peerInfo;                                      // object constructor for ESPNOW

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) // function called when sending data
{
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status == ESP_NOW_SEND_SUCCESS)
  {
    DisplayLoop("Delivery\nSuccess");
    delay(1000);
    display.clearDisplay();
  }
  else
  {
    DisplayLoop("Delivery Fail");
  }
}

void setup()
{
  delay(100);         // a delay to make sure the generation is complete before doing anything else
  Serial.begin(9600); // start the serial interface to report back through the USB for debugging
  while (!Serial)     // wait until serial is active before continuing
    ;
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // wait until the display object is active before continuing
    ;
  WiFi.mode(WIFI_STA);                            // configure the esp to eifi station mode
  if (esp_now_init() != ESP_OK)                   // initiate espnow
  {                                               //
    Serial.println("Error initializing ESP-NOW"); //
    return;                                       //
  }                                               //
  esp_now_register_send_cb(OnDataSent);           // set the callback function

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(LED_BUILTIN, OUTPUT);         // set embedded LED pin to output pin
  for (int i = 0; i < 4; i++)           //
  {                                     //
    pinMode(row[i], OUTPUT);            // set output pins
    pinMode(colomn[i], INPUT_PULLDOWN); // set input pins
  }

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
  digitalWrite(LED_BUILTIN, 0);
  char charInput = KeyboardCheck();

  if (charInput == 'C')
  {
    message = "";
    message_size = 0;
    charInput = 'X';
    DisplayLoop(message);
  }
  else if (charInput == 'A')
  {
    sendESPNOW(message);
    message = "";
    message_size = 0;
    charInput = 'X';
  }
  else if (charInput == 'D')
  {
    message.remove(message_size);
    message_size -= 1;
    DisplayLoop(message);
    charInput = 'X';
  }
  else if (charInput != 'X')
  {
    message += charInput;
    message_size += 1;
    DisplayLoop(message);
    lastChar = charInput;
    digitalWrite(LED_BUILTIN, 1);
  }
  delay(180);
}

char KeyboardCheck(void)
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(row[i], HIGH);
    for (int j = 0; j < 4; j++)
    {

      if (digitalRead(colomn[j]) == HIGH)
      {
        digitalWrite(row[i], LOW);
        return buttons[i][j];
      }
    }
    digitalWrite(row[i], LOW);
  }
  return 'X';
}

void DisplayLoop(String message)
{
  Serial.println(message);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(message);
  display.display();
}

void sendESPNOW(String message)
{
  uint8_t *buffer = (uint8_t *)message.c_str();
  size_t sizeBuff = sizeof(buffer) * message.length();
  esp_err_t result = esp_now_send(broadcastAddress, buffer, sizeBuff);
  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
}
