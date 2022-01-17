
// Json Variable to Hold Sensor Readings
DynamicJsonDocument readings(140);

// Init BME280
void initBME(){
  bme2.begin(0x76);
  bme6.begin();
//  if (!bme2.begin(0x76)) {
//    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
//    while (1);
//  }
//  if (!bme6.begin()) {
//    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
//    while (1);
//  }
// weather monitoring
//    Serial.println(F("-- Weather Station Scenario --"));
//    Serial.println(F("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling,"));
//    Serial.println(F("filter off"));
  bme2.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
                      
// suggested rate is 1/60Hz (1m)
// Set up oversampling and filter initialization
  bme6.setTemperatureOversampling(BME680_OS_8X);
  bme6.setHumidityOversampling(BME680_OS_2X);
  bme6.setPressureOversampling(BME680_OS_4X);
  bme6.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme6.setGasHeater(320, 150); // 320*C for 150 ms  
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  bme2.takeForcedMeasurement(); // has no effect in normal mode
  bme6.performReading();
  readings.clear();
  readings["S1_temp"] = String(bme2.readTemperature());
  readings["S1_humi"] = String(bme2.readHumidity());
  readings["S1_pres"] = String(bme2.readPressure()/100.0F);
  readings["S2_temp"] = String(bme6.temperature);
  readings["S2_humi"] = String(bme6.humidity);
  readings["S2_pres"] = String(bme6.pressure/100.0F);
  
  String jsonString = "";
  serializeJson(readings, jsonString);
  return jsonString;
}
