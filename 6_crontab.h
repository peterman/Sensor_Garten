// -----------------------------------------------------

void cron1() { 
  // 
}
void cron2() { 
  // Send Events to the client with the Sensor Readings Every 30 seconds
      events.send("ping",NULL,millis());
      events.send(getSensorReadings().c_str(),"new_readings" ,millis());
}
void cron3() { 
  //
}
void cron4() {
  //
  Serial.println(F("get new Time from Server"));
  getNtpServer(true);
}

// ------------------------------------------------------
void do_crontabs(){
  long tmp = millis();
  if ((cronjobs.t1+(cronjobs.c1*1000)) <= tmp) {
    cron1();
    cronjobs.t1=millis();
  }
  if ((cronjobs.t2+(cronjobs.c2*1000)) <= tmp) {
    cron2();
    cronjobs.t2=millis();
  }
  if ((cronjobs.t3+(cronjobs.c3*1000)) <= tmp) {
    cron3();
    cronjobs.t3=millis();
  }
  if ((cronjobs.t4+(cronjobs.c4*1000)) <= tmp) {
    cron4();
    cronjobs.t4=millis();
  }
}
