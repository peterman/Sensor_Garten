#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <InfluxDbClient.h>
#include <time.h>
#include "sensor.h"

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
struct tm tm;         // http://www.cplusplus.com/reference/ctime/tm/

// Replace with your network credentials
const char* ssid = "devnet-34";
const char* password = "testerwlan";
const char* http_username = "admin";
const char* http_password = "admin";

float luftTemp, luftDruck, redLuftDruck, luftFeuchte, luftDew;
String akttime;
String temp_str;
char temp[50];
bool timeok;
int hoehe = 235; //Messort 215 m ueber dem Meer
const char *filename = "/settings.json"; 
// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

const uint32_t SYNC_INTERVAL = 24;              // NTP Sync Interval in Stunden


const char* const PROGMEM NTP_SERVER[] = {"de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
const char* const PROGMEM DAY_NAMES[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

Config config;
// ---------------------------- SKETCH BEGIN ---------------------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Data point
Point sensor("sensor_status");


// Initialize LittleFS
void initFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

extern "C" uint8_t sntp_getreachability(uint8_t);

bool getNtpServer(bool reply = false) {
  uint32_t timeout {millis()};
  configTime("CET-1CEST,M3.5.0/02,M10.5.0/03", NTP_SERVER[0], NTP_SERVER[1], NTP_SERVER[2]);   // Zeitzone einstellen https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  do {
    delay(25);
    if (millis() - timeout >= 1e3) {
      Serial.printf("Warten auf NTP-Antwort %02ld sec\n", (millis() - timeout) / 1000);
      delay(975);
    }
    sntp_getreachability(0) ? reply = true : sntp_getreachability(1) ? reply = true : sntp_getreachability(2) ? reply = true : false;
  } while (millis() - timeout <= 16e3 && !reply);
  return reply;
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(5000);
  }
  Serial.println("\nVerbunden mit: " + WiFi.SSID());
  Serial.println("IP Addresse: " + WiFi.localIP().toString());
  bool timeSync = getNtpServer();
  Serial.printf("NTP Synchronisation %s!\n", timeSync ? "erfolgreich" : "fehlgeschlagen");
}




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
        
//  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);
//  
//  // Serve a file with no cache so every tile It's downloaded
//  server.serveStatic("/settings.json", SPIFFS, "/settings.json", "no-cache, no-store, must-revalidate");

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
  if ((millis() - lastTime) > timerDelay) {
      // Send Events to the client with the Sensor Readings Every 30 seconds
      events.send("ping",NULL,millis());
      events.send(getSensorReadings().c_str(),"new_readings" ,millis());
      lastTime = millis();
    }
  char buff[20];                                                   // je nach Format von "strftime" eventuell anpassen
  static time_t lastmin {0};
  time_t now = time(&now);
  localtime_r(&now, &tm);
  if (tm.tm_min != lastmin) {
    lastmin = tm.tm_min;
    strftime (buff, sizeof(buff), "%d.%m.%Y %T", &tm);             // http://www.cplusplus.com/reference/ctime/strftime/
    if (!(time(&now) % (SYNC_INTERVAL * 3600))) {
      getNtpServer(true);
    }
    // Verwendungsbeispiele
    Serial.printf("\nLokalzeit:  %s\n", buff);                     // Ausgabe der Kalenderzeit
                                              
    Serial.println(DAY_NAMES[tm.tm_wday]);                         // aktueller Wochentag
    
    Serial.println(tm.tm_isdst ? "Sommerzeit" : "Normalzeit");
  }  
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
