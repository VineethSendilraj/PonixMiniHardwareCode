// WiFi & Firestore Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// <--------------------------------------------------------> //
// CONNECTION DETAILS
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

#define API_KEY "AIzaSyDcycUvZM2q_ODoXstbE6WQ_tknr2JcR3w"
#define FIREBASE_PROJECT_ID "hydropicture-123"

#define USER_EMAIL "testuser@gmail.com"
#define USER_PASSWORD "test123"
// <--------------------------------------------------------> //

// <---------------------------------->
//SENSOR PINS 
#define waterLevelPin 35 
// <---------------------------------->

// Define Firebase objects
FirebaseData Firebase_dataObject;
FirebaseAuth authentication;
FirebaseConfig config;

String uid;

// <------------------------------------------->
// <------   SENSOR READ FUNCTIONS START ------>
// <------------------------------------------->

bool waterLevel = false;

// Water Level Sensor Code
bool getWaterLevel() {
  waterLevel = (analogRead(waterLevelPin));
  return (waterLevel == 1);
}
// <--- SECTION END --->

// ---- ADD OTHER SENSORS HERE ----

// <------------------------------------------->
// <------   SENSOR READ FUNCTIONS END ------>
// <------------------------------------------->

//Get all sensor reads in 1 function
void getValues() {
  getWaterLevel();

}


void setup() {
  // Pin Setup
  pinMode(waterLevelPin ,INPUT); 
  
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

void loop(){
    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;

        String documentPath = "Towers/PvdHXjtv2TlMA3I2tiOD";
        
        content.clear();
        content.set("fields/waterLevel/booleanValue", waterLevel);

        Serial.print("Update a document... ");

        if (Firebase.Firestore.patchDocument(&Firebase_dataObject, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "count,status" /* updateMask */))
            Serial.printf("ok\n%s\n\n", Firebase_dataObject.payload().c_str());
        else
            Serial.println(Firebase_dataObject.errorReason());
    }
}




//void loop() {
//  if (Firebase.ready() && (millis() - dataMillis > 100000 || dataMillis == 0)) { //HOW OFTEN IT CHECKS
//    dataMillis = millis();
//
//    Serial.print("Commit a document (append array)... ");
//    //The dyamic array of write object fb_esp_firestore_document_write_t.
//    std::vector<struct fb_esp_firestore_document_write_t> writes;
//
//    //A write object that will be written to the document.
//    struct fb_esp_firestore_document_write_t transform_write;
//
//    //Set the write object write operation type.
//    transform_write.type = fb_esp_firestore_document_write_type_transform;
//
//    //Set the document path of document to write (transform)
//    transform_write.document_transform.transform_document_path = "Towers/PvdHXjtv2TlMA3I2tiOD";
//
//    //Set a transformation of a field of the document.
//    struct fb_esp_firestore_document_write_field_transforms_t field_transforms;
//
//    //Set field path to write.
//    field_transforms.fieldPath = "waterLevel";
//
//    //Set the transformation type.
//    field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;
//
//    //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
//    FirebaseJson content;
//
//    getValues();
//    content.set("fields/waterLevel/booleanValue", waterLevel);
//
//    //Set the transformation content.
//    field_transforms.transform_content = content.raw();
//
//    //Add a field transformation object to a write object.
//    transform_write.document_transform.field_transforms.push_back(field_transforms);
//
//    //Add a write object to a write array.
//    writes.push_back(transform_write);
//
//    if (Firebase.Firestore.commitDocument(&Firebase_dataObject, FIREBASE_PROJECT_ID, "", writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
//      Serial.printf("ok\n%s\n\n", Firebase_dataObject.payload().c_str());
//    else
//      Serial.println(Firebase_dataObject.errorReason());
//  }
//  
//}
