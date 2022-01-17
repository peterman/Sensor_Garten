


// ----------------------------------------------------
// Saves the configuration to a file
void saveConfiguration(const char *filename, const Config &config) {
  SPIFFS.remove(filename);
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println("faile to create");
    return;
  }

  StaticJsonDocument<512> doc;

  // Set the values in the document
  doc["ssid"]       = config.ssid;
  doc["wlankey"]    = config.wlankey;
  doc["hostName"]   = config.hostName;
  doc["http_user"]  = config.http_user;
  doc["http_pw"]    = config.http_pw;
  doc["www_user"]   = config.www_user;
  doc["www_pw"]     = config.www_pw;
  doc["port"]       = config.port;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  // Close the file
  file.close();
}

// ---------------- Loads the configuration settingsfile
void loadConfiguration(const char *filename, Config &config) {
  File file; 
  if (!SPIFFS.open(filename, "r" )) {
    saveConfiguration(filename, config);
    Serial.println("no file");
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  config.port = doc["port"] | 2731;
  strlcpy(config.hostName,                  // <- destination
          doc["hostname"] | "example.com",  // <- source
          sizeof(config.hostName));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}
