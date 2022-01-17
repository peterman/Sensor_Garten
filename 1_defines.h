#define INFLUXDB_URL "http://iot.pfeiffer-privat.de:8086"
#define INFLUXDB_TOKEN "-hQ6CjVU1oiMTToKTETU9-P3rVDPti2W0J2-Xds9UexzxrC-BElJkMYDCQXfEI8cWSLii1POBqxJJMoiKgsmFw=="
#define INFLUXDB_ORG "MyServer"
#define INFLUXDB_BUCKET "Sensor Garten"
#define DEVICE "ESP8266"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Create a sensor object
Adafruit_BME280 bme2;         // BME280 connect to I2C (Addresse = 0x76)
Adafruit_BME680 bme6;         // BME680 connect to I2C (Addresse = 0x77)
