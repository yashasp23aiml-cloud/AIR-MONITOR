/*
 * Major Project: ESP32 Air Monitoring & Automatic Control System
 *
 * Copyright (c) 2026 
* Project Team:
 * Yashas P
 * Yashas Kaustav
 * Tarun V
 * Vishwajeet
 * All rights reserved.
 *
 * This source code is part of a college major project.
 * Unauthorized copying, modification, or redistribution
 * of this code is not permitted without permission.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Your code starts here...
// ==================================================
// WIFI
// ==================================================

const char* WIFI_SSID = "YOUR WIFI NAME";
const char* WIFI_PASSWORD = "PASSWARD";

const char* SERVER_URL =
  "http://YOUR IP:3000/api/sensor-data";


// ==================================================
// PIN DEFINITIONS
// ==================================================

#define DHTPIN 4
#define DHTTYPE DHT22

#define RELAY_PIN 26
#define LED_PIN 2


// ==================================================
// DHT SENSOR
// ==================================================

DHT dht(DHTPIN, DHTTYPE);


// ==================================================
// FAN STATE
// ==================================================

int currentFanState = 0;


// ==================================================
// AQI VARIABLES
// ==================================================

int currentAQI = 50;
int sendCycleCount = 0;


// ==================================================
// GENERATE NEW AQI
// ==================================================

void generateNewAQI() {

  currentAQI = random(45, 66);

  Serial.print("New AQI generated: ");
  Serial.println(currentAQI);
}


// ==================================================
// CONNECT TO WIFI
// ==================================================

void connectWiFi() {

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi connected!");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}


// ==================================================
// SEND DATA TO SERVER
// ==================================================

void postReadings(
  float temperature,
  float humidity,
  int aqi,
  int fanState
) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi disconnected!");
    
    connectWiFi();

    return;
  }


  HTTPClient http;

  http.begin(SERVER_URL);

  http.addHeader(
    "Content-Type",
    "application/json"
  );


  // Create JSON document
  StaticJsonDocument<200> doc;

  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["gasValue"] = aqi;
  doc["fanState"] = fanState;


  // Convert JSON to String
  String jsonString;

  serializeJson(
    doc,
    jsonString
  );


  Serial.print("Sending: ");
  Serial.println(jsonString);


  // Send POST request
  int httpResponseCode =
    http.POST(jsonString);


  if (httpResponseCode > 0) {

    Serial.print("Server Response: ");
    Serial.println(httpResponseCode);

  } else {

    Serial.print("Error sending data: ");
    Serial.println(httpResponseCode);
  }


  http.end();
}


// ==================================================
// CONTROL FAN
// RELAY = ACTIVE LOW
//
// LOW  = ON
// HIGH = OFF
// ==================================================

void setFan(bool on) {

  if (on) {

    digitalWrite(
      RELAY_PIN,
      LOW
    );

  } else {

    digitalWrite(
      RELAY_PIN,
      HIGH
    );
  }
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // Seed random generator
  randomSeed(
    analogRead(0)
  );


  // Configure pins
  pinMode(
    RELAY_PIN,
    OUTPUT
  );

  pinMode(
    LED_PIN,
    OUTPUT
  );


  // Fan OFF at startup
  digitalWrite(
    RELAY_PIN,
    HIGH
  );


  // LED OFF
  digitalWrite(
    LED_PIN,
    LOW
  );


  // Start DHT22
  dht.begin();


  // Generate first AQI
  generateNewAQI();


  // Connect WiFi
  connectWiFi();


  Serial.println();
  Serial.println("=================================");
  Serial.println("      SMART CLASSROOM SYSTEM");
  Serial.println("=================================");

  Serial.println(
    "Fan ON  : Temperature >= 30 C"
  );

  Serial.println(
    "Fan OFF : Temperature < 30 C"
  );

  Serial.println(
    "AQI     : Random 45-65"
  );

  Serial.println(
    "AQI     : Changes every 6 sends"
  );

  Serial.println(
    "Relay   : GPIO26"
  );

  Serial.println("=================================");
}


// ==================================================
// MAIN LOOP
// ==================================================

void loop() {


  // ==================================================
  // READ DHT22
  // ==================================================

  float temperature =
    dht.readTemperature();

  float humidity =
    dht.readHumidity();


  // Check sensor
  if (
    isnan(temperature) ||
    isnan(humidity)
  ) {

    Serial.println(
      "ERROR: Failed to read DHT22!"
    );


    // Safety: fan OFF
    setFan(false);

    currentFanState = 0;


    delay(2000);

    return;
  }


  // ==================================================
  // FAN CONTROL
  // ==================================================

  if (temperature >= 30.0) {

    setFan(true);

    currentFanState = 1;

  } else {

    setFan(false);

    currentFanState = 0;
  }


  // ==================================================
  // SEND DATA
  // ==================================================

  digitalWrite(
    LED_PIN,
    HIGH
  );


  delay(50);


  postReadings(
    temperature,
    humidity,
    currentAQI,
    currentFanState
  );


  digitalWrite(
    LED_PIN,
    LOW
  );


  // ==================================================
  // PRINT DATA
  // ==================================================

  Serial.println();

  Serial.println(
    "---------- SENSOR DATA ----------"
  );


  Serial.print(
    "Temperature : "
  );

  Serial.print(
    temperature
  );

  Serial.println(
    " C"
  );


  Serial.print(
    "Humidity    : "
  );

  Serial.print(
    humidity
  );

  Serial.println(
    " %"
  );


  Serial.print(
    "AQI         : "
  );

  Serial.println(
    currentAQI
  );


  Serial.print(
    "Fan         : "
  );


  if (currentFanState == 1) {

    Serial.println("ON");

  } else {

    Serial.println("OFF");
  }


  Serial.print(
    "Send Cycle  : "
  );

  Serial.print(
    sendCycleCount + 1
  );

  Serial.println(
    " / 6"
  );


  Serial.println(
    "---------------------------------"
  );


  // ==================================================
  // INCREASE SEND COUNTER
  // ==================================================

  sendCycleCount++;


  // ==================================================
  // CHANGE AQI AFTER 6 SENDS
  // ==================================================

  if (sendCycleCount >= 6) {

    sendCycleCount = 0;

    generateNewAQI();
  }


  // ==================================================
  // WAIT 1 SECOND
  // ==================================================

  delay(1000);
}
