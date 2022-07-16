// WiFi & Firestore Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// <--------------------------------------------------------> //
// CONNECTION DETAILS
#define WIFI_SSID "varGateway"
#define WIFI_PASSWORD "hawks123"

#define API_KEY "AIzaSyBgeSKUwaIyJ85xCQH-dsun-AgaX2AASO0"
#define FIREBASE_PROJECT_ID "ponix-mini"

#define USER_EMAIL "testuser@gmail.com"
#define USER_PASSWORD "test123"

String documentPath = "Towers/14frAKtXjQZUjDrrZrHz";
// <--------------------------------------------------------> //

// <---------------------------------->
//SENSOR PINS 
#define WaterLevelPin 33
#define WaterLevelBuzzer 23


// Define Firebase objects
FirebaseData Firebase_dataObject;
FirebaseAuth authentication;
FirebaseConfig config;
String uid;

// <------------------------------------------->
// <------   SENSOR READ FUNCTIONS START ------>
// <------------------------------------------->
int waterLevel = 0;
// Water Level Sensor Code
int getWaterLevel(){
  waterLevel = digitalRead(WaterLevelPin);
  Serial.println(waterLevel);
}
  
// <--- SECTION END --->

// <------------------------------------------->
// <------   SENSOR READ FUNCTIONS END ------>
// <------------------------------------------->


void getValues() {
  getWaterLevel();
}

// <------------------------------------------->
// <------   AUTOMATION FUNCTIONS START  ------>
// <------------------------------------------->


//void waterlevelCheck(){
//  if (waterLevel == 1) {
//    digitalWrite(WaterLevelBuzzer, HIGH);
//    delay(5000);
//    digitalWrite(WaterLevelBuzzer, LOW);
//  }
//}
// <--- SECTION END --->

// <------------------------------------------->
// <------   AUTOMATION FUNCTIONS END    ------>
// <------------------------------------------->


void setup() {
  // Pin Setup
  pinMode(WaterLevelPin,INPUT); 

  pinMode(WaterLevelBuzzer, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  authentication.user.email = USER_EMAIL;
  authentication.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  Firebase_dataObject.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &authentication);

  Serial.println("Getting User UID");
  while ((authentication.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = authentication.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);

}

unsigned long dataMillis;
void loop() {
  if (Firebase.ready() && (millis() - dataMillis > 1800000 || dataMillis == 0)) {
    dataMillis = millis();

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;
        
        content.clear();
        getValues();
        content.set("fields/waterLevel/booleanValue", waterLevel == 1);

        Serial.print("Update a document... ");

        /** if updateMask contains the field name that exists in the remote document and
         * this field name does not exist in the document (content), that field will be deleted from remote document
         */

        if (Firebase.Firestore.patchDocument(&Firebase_dataObject, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "waterLevel" /* updateMask */))
            Serial.printf("ok\n%s\n\n", Firebase_dataObject.payload().c_str());
        else
            Serial.println(Firebase_dataObject.errorReason());
    }
}
