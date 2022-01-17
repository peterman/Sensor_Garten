extern "C" uint8_t sntp_getreachability(uint8_t);

bool getNtpServer(bool reply = false) {
  uint32_t timeout {millis()};
  configTime(TZ_INFO, NTP_SERVER[0], NTP_SERVER[1], NTP_SERVER[2]);   // Zeitzone einstellen https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
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


// Initialize SPIFFS
void initFS() {
  if (!SPIFFS.begin()) {
    Serial.println(F("An error has occurred while mounting SPIFFS"));
  }
  Serial.println(F("SPIFFS mounted successfully"));
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
    luftTemp = bme2.readTemperature();
    luftDruck = bme2.readPressure() / 100.0F;
    redLuftDruck = (luftDruck/pow(1-(hoehe/44330.0),5.255));
    luftFeuchte = bme2.readHumidity();
    luftDew = 243.04 * (log(luftFeuchte/100.0) + ((17.625 * luftTemp)/(243.04 + luftTemp)))/(17.625 - log(luftFeuchte/100.0) - ((17.625 * luftTemp)/(243.04 + luftTemp)));
  }
