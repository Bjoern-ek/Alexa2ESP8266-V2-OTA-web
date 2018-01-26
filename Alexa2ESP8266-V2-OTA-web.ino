#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <functional>
#include "Switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include "Credentials.h"

#define SERIAL_BAUDRATE 115200

// prototypes
boolean connectWifi();

//on/off callbacks 
bool GartenLichtOn();
bool GartenLichtOff();
bool AussenLichtOn();
bool AussenLichtOff();

const int relayS1Pin = D4;  // GPIO04 blue LED
const int relayS2Pin = D8;  // GPIO13

Switch *s1 = NULL;
Switch *s2 = NULL;



// Change this before you flash


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
const char* host = "OTA-LEDS";

// OTA
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
// OTA

boolean wifiConnected = false;

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *garten = NULL;
Switch *aussen = NULL;

bool isGartenLichtOn = false;
bool isAussenLichtOn = false;

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);

  pinMode(relayS1Pin, OUTPUT);
  pinMode(relayS2Pin, OUTPUT);

   
   
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();
    
    // Define your switches here. Max 10
    // Format: Alexa invocation name, local port no, on callback, off callback
    garten = new Switch("Garten Licht", 80, GartenLichtOn, GartenLichtOff);
    aussen = new Switch("Aussen Licht", 81, AussenLichtOn, AussenLichtOff);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*garten);
    upnpBroadcastResponder.addDevice(*aussen);
  }

//--------------- O T A ----------------
    MDNS.begin(host);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);   
//------------------------------------------

  
}
 
void loop()
{
  httpServer.handleClient();
	if(wifiConnected){
    upnpBroadcastResponder.serverLoop();
      
    aussen->serverLoop();
    garten->serverLoop();
	 }
}

bool GartenLichtOn() {
    Serial.println("Switch 1 turn on ...");
    digitalWrite(relayS1Pin, HIGH); // turn on relay with voltage HIGH
    isGartenLichtOn = true;    
    return isGartenLichtOn;
}

bool GartenLichtOff() {
    Serial.println("Switch 1 turn off ...");
    digitalWrite(relayS1Pin, LOW); // turn off relay with voltage HIGH
    isGartenLichtOn = false;
    return isGartenLichtOn;
}

bool AussenLichtOn() {
    Serial.println("Switch 2 turn on ...");
    digitalWrite(relayS2Pin, HIGH); // turn on relay with voltage HIGH
    isAussenLichtOn = true;
    return isAussenLichtOn;
}

bool AussenLichtOff() {
  Serial.println("Switch 2 turn off ...");
  digitalWrite(relayS2Pin, LOW); // turn off relay with voltage HIGH
  isAussenLichtOn = false;
  return isAussenLichtOn;
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}
