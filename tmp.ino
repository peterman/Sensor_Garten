//server->on("/wifi/list", [&] () {
//    //scan for wifi networks
//    int n = WiFi.scanNetworks();
//
//    //build array of indices
//    int ix[n];
//    for (int i = 0; i < n; i++) ix[i] = i;
//
//    //sort by signal strength
//    for (int i = 0; i < n; i++) for (int j = 1; j < n - i; j++) if (WiFi.RSSI(ix[j]) > WiFi.RSSI(ix[j - 1])) std::swap(ix[j], ix[j - 1]);
//    //remove duplicates
//    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (WiFi.SSID(ix[i]).equals(WiFi.SSID(ix[j])) && WiFi.encryptionType(ix[i]) == WiFi.encryptionType(ix[j])) ix[j] = -1;
//
//    //build plain text string of wifi info
//    //format [signal%]:[encrypted 0 or 1]:SSID
//    String s = "";
//    s.reserve(2050);
//    for (int i = 0; i < n && s.length() < 2000; i++) { //check s.length to limit memory usage
//      if (ix[i] != -1) {
//#if defined(ESP8266)
//        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + ","
//             + ((WiFi.encryptionType(ix[i]) == ENC_TYPE_NONE) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
//#elif defined(ESP32)
//        s += String(i ? "\n" : "") + ((constrain(WiFi.RSSI(ix[i]), -100, -50) + 100) * 2) + ","
//             + ((WiFi.encryptionType(ix[i]) == WIFI_AUTH_OPEN) ? 0 : 1) + "," + WiFi.SSID(ix[i]);
//#endif
//      }
//    }
