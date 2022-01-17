#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BME680.h>
#include <InfluxDbClient.h>
#include <time.h>
#include "1_defines.h"
#include "2_variables.h"
#include "3_settings.h"
#include "4_functions.h"
#include "5_sensor.h"
#include "6_crontab.h"

// ---------------------------- SKETCH BEGIN ---------------------

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(100);

  initBME();
  initFS();
  initWiFi();
  
  

  // Add tags
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  
  
  
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    int n = WiFi.scanComplete();
    if(n == -2){
      WiFi.scanNetworks(true);
    } else if(n){
      for (int i = 0; i < n; ++i){
        if(i) json += ",";
        json += "{";
        json += "\"rssi\":"+String(WiFi.RSSI(i));
        json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
        json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
        json += ",\"channel\":"+String(WiFi.channel(i));
        json += ",\"secure\":"+String(WiFi.encryptionType(i));
        json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
        json += "}";
      }
      WiFi.scanDelete();
      if(WiFi.scanComplete() == -2){
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
    request->send(200, "application/json", json);
    json = String();
  });

  
        
//  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);
//  
// Serve a file with no cache so every tile It's downloaded
  server.serveStatic("/settings.json", SPIFFS, "/settings.json", "no-cache, no-store, must-revalidate");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  server.serveStatic("/", SPIFFS, "/");
  
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  
  server.addHandler(&events);
  server.addHandler(new SPIFFSEditor(config.http_user,config.http_pw));
  // Start server
  server.begin();
}

void loop(){
  do_crontabs();
  
//  if (millis() > ntpTO + ntpTM ) {
//    ntpTM = millis();
//    leseMesswerte();
//
//    // Clear fields for reusing the point. Tags will remain untouched
//    sensor.clearFields();
//
//    // Store measured value into point
//    // Report RSSI of currently connected network
//    sensor.addField("rssi", WiFi.RSSI());
//    sensor.addField("temp", String(luftTemp));
//    sensor.addField("taup", String(luftDew));
//    sensor.addField("humi", String(luftFeuchte));
//    sensor.addField("pres", String(redLuftDruck));
//
//    // Print what are we exactly writing
//    Serial.print("Writing: ");
//    Serial.println(sensor.toLineProtocol());
//
////    // Check WiFi connection and reconnect if needed
////    if (wifiMulti.run() != WL_CONNECTED) {
////      Serial.println("Wifi connection lost");
////    }
//
//    // Write point
//    if (!client.writePoint(sensor)) {
//      Serial.print("InfluxDB write failed: ");
//      Serial.println(client.getLastErrorMessage());
//     }
//
//    }
}
