#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>

#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include <ArduinoJson.h>

#include <AsyncElegantOTA.h>
#include <elegantWebpage.h>
#include <Hash.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <InfluxDbClient.h>

#define INFLUXDB_URL "http://iot.pfeiffer-privat.de:8086"
#define INFLUXDB_TOKEN "-hQ6CjVU1oiMTToKTETU9-P3rVDPti2W0J2-Xds9UexzxrC-BElJkMYDCQXfEI8cWSLii1POBqxJJMoiKgsmFw=="
#define INFLUXDB_ORG "MyServer"
#define INFLUXDB_BUCKET "Sensor Garten"
#define DEVICE "ESP8266"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"


//----------------------------------------------------------------
struct Config {
  char ssid[20]       = "Sensor";
  char wlankey[20]    = "sensorgarten";
  char mqtt_broker[30]= "ppfeiffer.home-webserver.de";
  //char mqtt_broker[30]= "192.168.10.8";
  char mqtt_topic[20] = "/Garten/Sensor1/";
  char hostName[20]   = "esp-async";
  char http_user[10]  = "admin";
  char http_pw[10]    = "admin";
  char www_user[10]   = "admin";
  char www_pw[10]     = "admin";
  
  int port;
  int mqtt_port   = 11883;
  
};

float luftTemp, luftDruck, redLuftDruck, luftFeuchte, luftDew;
String akttime;
String temp_str;
char temp[50];
bool timeok;
int hoehe = 235; //Messort 215 m ueber dem Meer
Adafruit_BME280 bme;

const char *filename = "/settings.json"; 
Config config;
// ---------------------------- SKETCH BEGIN ---------------------
AsyncWebServer server(80);
WiFiClient espClient;

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
// Data point
Point sensor("sensor_status");

char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Sonnabend"};

long ntpTM = 0;
long ntpTO = 30000;


#include "webserver.h"
#include "settings.h"



void leseMesswerte() {
    luftTemp = bme.readTemperature();
    luftDruck = bme.readPressure() / 100.0F;
    redLuftDruck = (luftDruck/pow(1-(hoehe/44330.0),5.255));
    luftFeuchte = bme.readHumidity();
    luftDew = 243.04 * (log(luftFeuchte/100.0) + ((17.625 * luftTemp)/(243.04 + luftTemp)))/(17.625 - log(luftFeuchte/100.0) - ((17.625 * luftTemp)/(243.04 + luftTemp)));
  }

    
void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  SPIFFS.begin();
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  //loadConfiguration(filename, config);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config.hostName);
  WiFi.begin(config.ssid, config.wlankey);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(config.ssid, config.wlankey);
  }

  //Send OTA events to the browser
  
  // Init Sensor
  bme.begin(0x76);

  MDNS.addService("http","tcp",80);

  // Add tags
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  
  
  server.addHandler(new SPIFFSEditor(config.http_user,config.http_pw));
  
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

  server.on("/mess", HTTP_GET, [](AsyncWebServerRequest *request) {
        leseMesswerte();
        String mess = "LuftTemperatur: " + String(luftTemp) + " °C \n";
        mess +=       "Luftdruck:      " + String(luftDruck) + " hPa \n";
        mess +=       "red.Luftdruck : " + String(redLuftDruck) + " hPa \n";
        mess +=       "Luftfeuchte:    " + String(luftFeuchte) + " % \n";
        mess +=       "Taupunkt:       " + String(luftDew) + " °C \n\n";
        request->send(200, "text/plain", mess);
        mess = String();
        });  
        
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);
  
  // Serve a file with no cache so every tile It's downloaded
  server.serveStatic("/settings.json", SPIFFS, "/settings.json", "no-cache, no-store, must-revalidate");

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  
  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);    
  server.begin();
}

void loop(){
  AsyncElegantOTA.loop();
  if (millis() > ntpTO + ntpTM ) {
    ntpTM = millis();
    leseMesswerte();

    // Clear fields for reusing the point. Tags will remain untouched
    sensor.clearFields();

    // Store measured value into point
    // Report RSSI of currently connected network
    sensor.addField("rssi", WiFi.RSSI());
    sensor.addField("temp", String(luftTemp));
    sensor.addField("taup", String(luftDew));
    sensor.addField("humi", String(luftFeuchte));
    sensor.addField("pres", String(redLuftDruck));

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());

//    // Check WiFi connection and reconnect if needed
//    if (wifiMulti.run() != WL_CONNECTED) {
//      Serial.println("Wifi connection lost");
//    }

    // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
     }

    }
}
