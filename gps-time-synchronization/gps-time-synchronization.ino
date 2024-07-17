#include <GNSS.h>
#include <RTC.h>
#include <LowPower.h>

static SpGnss Gnss;

#define JST_IN_SECONDS  (9 * 60 * 60);
#define TIME_WAIT_GPS_TIME_SYNC 180 //Time to wait for time synchronization from GPS (seconds)

void initRTC(){

  Serial.println("begin");
  RTC.begin();

  RtcTime start = RTC.getTime();
  Serial.println(start.year());
  if(start.year() > 2000) return;
  Serial.println("wake up");
  Gnss.begin();
  Gnss.select(GPS);
  Gnss.start(COLD_START);
  Serial.println("Gnss start...");

  int timeSyncFlag = 0;
  int timeOverFlag = 0;

  while((timeSyncFlag == 0) || (timeOverFlag == 0)){
    if (!Gnss.waitUpdate(-1)) return;
    SpNavData NavData;
    Gnss.getNavData(&NavData);
    Serial.println("numSat: " + String(NavData.numSatellites));
    if(NavData.time.year > 2000) {
      /* Create RTC time */
      RtcTime gtime(NavData.time.year, NavData.time.month, NavData.time.day
              , NavData.time.hour, NavData.time.minute, NavData.time.sec
              , NavData.time.usec * 1000 /* RtcTime requires nsec */);
      printf("%04d\n", gtime.year());
      gtime += JST_IN_SECONDS; /* convert UTC to JST */
      RTC.setTime(gtime);

      RtcTime now = RTC.getTime();
      printf("%04d/%02d/%02d %02d:%02d:%02d\n"
          , now.year(), now.month(), now.day()
          , now.hour(), now.minute(), now.second());
      rtcflag = 1;
      break;
    }

    RtcTime end = RTC.getTime();
    int diff = end - start;
    if (abs(diff) >= TIME_WAIT_GPS_TIME_SYNC ) {
      timeOverFlag = 1;
    }
    sleep(1);
  };
}

void setup() {
  Serial.begin(115200);
  Serial.println("wake up");
  // RtcTime now = RTC.getTime();
  // printf("%04d/%02d/%02d %02d:%02d:%02d\n"
  //         , now.year(), now.month(), now.day()
  //         , now.hour(), now.minute(), now.second());
  // Serial.println("init");
  initRTC();
}

void loop() {

  RtcTime now = RTC.getTime();
  printf("%04d/%02d/%02d %02d:%02d:%02d\n"
          , now.year(), now.month(), now.day()
          , now.hour(), now.minute(), now.second());

  Serial.println("sleep");

  LowPower.deepSleep(60);
}
