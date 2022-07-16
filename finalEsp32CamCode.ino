/*********
  Rui Santos
  Complete instructions at: https://RandomNerdTutorials.com/esp32-cam-save-picture-firebase-storage/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Based on the example provided by the ESP Firebase Client Library
*********/

#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
#include "addons/RTDBHelper.h"

//Replace with your network credentials
const char* ssid = "varGateway";
const char* password = "hawks123";

// Insert Firebase project API Key
#define API_KEY "AIzaSyBgeSKUwaIyJ85xCQH-dsun-AgaX2AASO0"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "testuser@gmail.com"
#define USER_PASSWORD "test123"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "ponix-mini.appspot.com"

#define FIREBASE_PROJECT_ID "ponix-mini"

// Photo File Name to save in SPIFFS
String FILE_PHOTO = "/data/photo.jpg";
String path = "/Towers/ImageSnapshots/14frAKtXjQZUjDrrZrHz/";

String documentPath = "Towers/14frAKtXjQZUjDrrZrHz";


// Timestamp Libraries
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate; // Variables to save date and time

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly
  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    SPIFFS.remove(FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initSPIFFS(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
    SPIFFS.remove(FILE_PHOTO);
  }
}

void initCamera(){
 // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

String uid;

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  configF.token_status_callback = tokenStatusCallback;
  configF.max_token_generation_retry = 5;
  Firebase.begin(&configF, &auth);

  
   // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-18000);
}

unsigned long dataMillis;
void loop() {
  if (Firebase.ready() && (millis() - dataMillis > 25000 || dataMillis == 0)) {
    dataMillis = millis();
    
    capturePhotoSaveSpiffs();
    
    timeClient.forceUpdate();
    String timeStamp = timeClient.getFormattedDate();
    Serial.println(timeStamp);
  
    String REAL_NAME = path + timeStamp + ".jpg";
    delay(1);
    if (Firebase.ready()){
      Serial.print("Uploading picture... ");
  
      //MIME type should be valid to avoid the download problem.
      //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
      if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, REAL_NAME /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
        Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
      }
      else{
        Serial.println(fbdo.errorReason());
      }
    }
    // CAMERA END
    
    
    Serial.print("Commit a document (append array)... ");
    //The dyamic array of write object fb_esp_firestore_document_write_t.
    std::vector<struct fb_esp_firestore_document_write_t> writes;

    //A write object that will be written to the document.
    struct fb_esp_firestore_document_write_t transform_write;

    //Set the write object write operation type.
    transform_write.type = fb_esp_firestore_document_write_type_transform;

    //Set the document path of document to write (transform)
    transform_write.document_transform.transform_document_path = documentPath;

    //Set a transformation of a field of the document.
    struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

    //Set field path to write.
    field_transforms.fieldPath = "cameraSnapshots";

    //Set the transformation type.
    field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

    //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson content;

    content.set("values/[0]/stringValue", timeStamp);

    //Set the transformation content.
    field_transforms.transform_content = content.raw();

    //Add a field transformation object to a write object.
    transform_write.document_transform.field_transforms.push_back(field_transforms);

    //Add a write object to a write array.
    writes.push_back(transform_write);

    if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "", writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
      Serial.println(fbdo.errorReason());
  }
}
