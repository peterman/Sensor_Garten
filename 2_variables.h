const char* const PROGMEM NTP_SERVER[] = {"de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
const char* const PROGMEM DAY_NAMES[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const char* filename = "/settings.json";

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

Config config;

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
 
// Data point
Point sensor("sensor_status");

struct CronJobs {
  int c1 = 10;      // Interval 10 sek
  int c2 = 30;      // Interval 30 sek
  int c3 = 300;     // Interval 5 min
  int c4 = 3600;    // Interval 1h
  long t1, t2, t3, t4;
};

CronJobs cronjobs;
