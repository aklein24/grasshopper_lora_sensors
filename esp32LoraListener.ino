/* 

LoRa listener with OLED status update
The code works w/ HELTEC WiFi Lora 32 V2 ESP32 device.
I purchased the HELTEC device on Amazon, if you follow the instructions for that device they point you toward a git repository
that is basically a wrapper for many ESP32 libraries. I found the HELTEC libraries to be cumbersome and difficult to use.
Save yourself the trouble and use all the generic drivers. The only hiccup was identifying all the correct pinouts.
See below for the OLED pinouts and the LoRa pinouts.

This example will work with pretty much any Arduino device with the following conditions:

1) You need a microcontroller that has SPI and I2C pins available.
2) You need a SemTech LoRa device of some kind, the RMF_95 is a common one.
3) You'll need to change the pin #defines below to match your device pinout.
4) you need to setup your Arduino IDE correctly and install the libraries used by this program. Start here: https://www.arduino.cc/en/Guide/HomePage
4) or you can buy a cheap HELTEC WiFi Lora 32 V2 ESP32 device from amazon or alibaba and flash this code.



This is all in the public domain. Use at your own risk etc etc. Incorporates lots of code from other open source examples.

Created by Andrew Klein

*/

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h> //uses Sandeep Mistry's very simple LoRa library: https://github.com/sandeepmistry/arduino-LoRa

// Libraries for OLED Display. Uses the well-supported and simple Adafruit_GFX / SSD1306 libraries. Make sure you're using the latest version.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// defines the SPI pins used by the LoRa transceiver module, all pins specific to the HELTEC WiFi Lora 32 V2
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6 // I wondered what the "E6" meant and then remembered that it's 915 x 10E6 or 915,000,000 = 915MHz. If you don't know mega then check your SI units.

//OLED pins. This implementation uses I2C because the LoRa transceiver uses the SPI pins. With a large LCD screen this would be sloooowwww.
#define OLED_SDA 4 // I2C data
#define OLED_SCL 15 // I2C clock
#define OLED_RST 16 // I2C reset pin
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

int count = 1;

void setup() { 
  
  //reset OLED display via software so it starts up correctly.
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL); // sets the I2C pins up to the custom pins on this board, must do this if your pins aren't standard.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();
  
  //initialize Serial Monitor
  Serial.begin(115200);

  Serial.println("LoRa Receiver Test");
  
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.enableCrc();

  Serial.println("LoRa initialized OK!");
  display.setCursor(0,10);
  display.println("LoRa initialized OK!");
  display.display();  
  delay(5000);
}

void loop() {
  // check to see if there's a message available.
  // get the pack length if there's a packet.
  int packetSize = LoRa.parsePacket(); 
  
  uint8_t radiopacket[packetSize+1]; // make a buffer for the data
  

  if (packetSize > 0) {
    Serial.print("Packet received, size: ");
    Serial.println(packetSize);
    Serial.println("Received data:");
    int i = 0;
    while (LoRa.available() && i < packetSize) {
        radiopacket[i] = LoRa.read();
        i++;
    }

    radiopacket[packetSize] = 0; // put a null character at the end of the string
    Serial.println((char*)radiopacket); // print to serial
    // send a response to the device
    //LoRa.enableInvertIQ(); // we will invertIQ for gateway transmissions to keep sensors from receiving sensors messages.
    LoRa.beginPacket();
    LoRa.print("OK");
    LoRa.endPacket();
    //LoRa.disableInvertIQ();

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rssi);

    // Display information
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("LORA RECEIVER");
    display.setCursor(0,20);
    display.print("Rec'd packet#:");
    display.print(count++);
    display.setCursor(0,30);
    display.print((char*)radiopacket);
    display.setCursor(0,40);
    display.print("RSSI:");
    display.setCursor(30,40);
    display.print(rssi);
    display.display();
  }
  delay(1); // need a short delay to prevent lockups?
}
