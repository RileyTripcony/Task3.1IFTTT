#include <WiFiNINA.h>
#include "Secret.h"
#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;

WiFiClient client;

char HOST_NAME[] = "maker.ifttt.com";
String SUFFICIENT_LIGHT_PATH = "/trigger/sunlight_received/with/key/cuXtFESE6lJYSaWqt3OMMW";
String INSUFFICIENT_LIGHT_PATH = "/trigger/insufficient_light/with/key/cuXtFESE6lJYSaWqt3OMMW";
String queryString = "?value1=57&value2=25";

bool emailSent = false;

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 10000; // 10 seconds

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();

  lightMeter.begin();

  //Initialize WiFi connection
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}

void loop() {
  unsigned long currentTime = millis();

  //Print light value every 10 seconds
  if (currentTime - lastPrintTime >= printInterval) {
    printLightValue();
    lastPrintTime = currentTime;
  }

  float lux = lightMeter.readLightLevel();

  if (lux >= 200 && !emailSent) {
    sendEmail(SUFFICIENT_LIGHT_PATH);
    emailSent = true; // Set flag to true indicating email has been sent
  }

  // If lux drops below 200 and email has been sent previously
  // send webhook for insufficient light and reset emailSent flag
  if (lux < 200 && emailSent) {
    sendEmail(INSUFFICIENT_LIGHT_PATH);
    emailSent = false; // Reset flag
  }
  delay(100); 
}

void printLightValue() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
}

void sendEmail(String path) {
  // connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    Serial.println("Connected to server");
    // send HTTP request
    client.print("GET ");
    client.print(path);
    client.print(queryString);
    client.println(" HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header

    while (client.connected()) {
      if (client.available()) {
        //Read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    //The server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("Disconnected");
  }
  else {
    //If not connected:
    Serial.println("Connection to server failed");
  }
}


