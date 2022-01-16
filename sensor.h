// Create a sensor object
Adafruit_BME280 bme;         // BME280 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)
// Json Variable to Hold Sensor Readings
DynamicJsonDocument readings(128);

// Init BME280
void initBME(){
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  // weather monitoring
    Serial.println(F("-- Weather Station Scenario --"));
    Serial.println(F("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling,"));
    Serial.println(F("filter off"));
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
                      
    // suggested rate is 1/60Hz (1m)
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  bme.takeForcedMeasurement(); // has no effect in normal mode
  Serial.println(String(bme.readTemperature()));
  readings["temperature"] = String(bme.readTemperature());
  readings["humidity"] =  String(bme.readHumidity());
  readings["pressure"] = String(bme.readPressure()/100.0F);
  String jsonString = "";
  serializeJson(readings, jsonString);
  Serial.println(jsonString); Serial.println(measureJson(readings)); Serial.println(readings.overflowed());
  readings.clear();
  return jsonString;
}
