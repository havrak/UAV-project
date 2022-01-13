#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP32Time.h>

ESP32Time rtc;

struct SendDataStruct{
  uint32_t numberOfSatellites;
  double latitude;
  double longitude;
  int32_t  hdop;
    time_t time;

};


uint32_t numberOfSatellites;
double latitude;
double longitude;
int32_t  hdop ;

static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
//SoftwareSerial ss(RXPin, TXPin); // The serial connection to the GPS device


void setup() {
  // Join I2C bus as slave with address 8
    //Wire.begin(0x8);
    Serial.begin(115200);
    Serial2.begin(GPSBaud, SERIAL_8N1,16,17);
    
}

void sendDataToMaster() {
  SendDataStruct data;
  
  data.hdop = hdop;
  data.numberOfSatellites = numberOfSatellites;
  data.latitude = latitude;
  data.longitude = longitude;
  data.time = rtc.getEpoch();

  uint8_t dataToBeSend[sizeof(data)];
  memcpy(dataToBeSend, &data, sizeof(data));  
}

// Function that executes whenever data is received from master

unsigned long getTimeDiffrence(const unsigned long sTime){
  if(millis() < sTime){
    return (ULONG_MAX - sTime) + millis();
  }
  return millis() - sTime;
}


static void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (getTimeDiffrence(start) < ms);
}


void loop() {
  numberOfSatellites =  gps.satellites.value();
  latitude = gps.location.lat();
  longitude = gps.location.lng();
  hdop = gps.hdop.value() ;
  rtc.setTime(gps.time.hour(),gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());
  
  Serial.print("GPSINTERFACE | LOOP | numberOfSatellites is: "); Serial.println(numberOfSatellites);
  Serial.print("GPSINTERFACE | LOOP | hdop is:               "); Serial.println(hdop);
  Serial.print("GPSINTERFACE | LOOP | latitude is:           "); Serial.println(latitude);
  Serial.print("GPSINTERFACE | LOOP | longitude is:          "); Serial.println(longitude);
  Serial.print("GPSINTERFACE | LOOP | hour is:               "); Serial.println(gps.time.hour());
  Serial.println("");

  smartDelay(1000);

}
