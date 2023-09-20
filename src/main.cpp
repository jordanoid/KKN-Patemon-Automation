#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS1302.h>

#define DHT_PIN 5 // DHT22 sensor pin
#define DHT_TYPE DHT22 // Specify the DHT sensor type (DHT11, DHT22, or DHT21)
DHT dht(DHT_PIN, DHT_TYPE);

ThreeWire myWire(15, 2, 4); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int relayPin = 16;
const int flowRateLPH = 300; // Flow rate in liters per hour
const int desiredVolumeMLPerPlant = 500; // Desired volume
const int plantRows = 16;
const int plantPerRows = 20;
float wateringDurationSeconds;
int soilPin = A0;

int readSoilMoisture() {
   // Read the analog value from the soil moisture sensor
   int sensorValue = analogRead(soilPin); // Connect the sensor to an appropriate pin
   // Convert the sensor value to a moisture percentage
   // int moisturePercentage = ( 100 - ( (sensorValue/4095) * 100 ) );
   int moisturePercentage = map(sensorValue, 0, 4095, 100, 0);
   return moisturePercentage;
}

bool shouldWaterPlant(int nowHour, int nowMinute, int nowSecond) {
   // Calculate the time intervals for watering (5 times in 24 hours)
   int intervals[] = {1, 8, 11, 16, 21}; // Adjust as needed

   // Check if the current time falls within one of the intervals
   for (int i = 0; i < 5; i++) {
      if (nowHour == intervals[i]  && nowSecond + (nowMinute * 60) <= wateringDurationSeconds) {
         return true;
      }
   }
   return false;
}

void timedWaterPlant(float durationSeconds, int nowSecond, int nowMinute) {

   if(nowSecond + nowMinute*60 < durationSeconds){
      // lcd.setCursor(0, 2);
      // lcd.print("WaterPlant");
      digitalWrite(relayPin, HIGH);
      lcd.setCursor(0, 3);
      lcd.print("Time Left :");
      lcd.print(durationSeconds - (nowSecond + nowMinute*60));
   }
   else if(nowSecond + nowMinute*60 > durationSeconds){
      // lcd.setCursor(0, 2);
      // lcd.print("StopWaterPlant");
      digitalWrite(relayPin, LOW);
   }

}

void soilWaterPlant(int soilRead) {

   if(soilRead <= 50){
      digitalWrite(relayPin, HIGH);
      lcd.setCursor(0, 3);
      lcd.print("WaterPlant");
   }
   else if(soilRead > 50){
      digitalWrite(relayPin, LOW);
   }

}

float calculateWateringDuration(float flowRateLPH, float desiredVolumeMLPerPlant) {

   // Convert flow rate to milliliters per second
   float perPlantFlowRateLPH = flowRateLPH / (plantRows*plantPerRows);

   // Calculate the watering duration in seconds
   float durationSeconds = desiredVolumeMLPerPlant / (perPlantFlowRateLPH * 1000) * 3600;

   return durationSeconds;
}

void setup() {
   Serial.begin(115200);
   pinMode(relayPin, OUTPUT);
   pinMode(soilPin, INPUT);

   Rtc.Begin();

   RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
   Rtc.SetDateTime(compiled);
   dht.begin();

   lcd.init();
   lcd.backlight();
   lcd.clear();
}

void loop() {
   wateringDurationSeconds = calculateWateringDuration(flowRateLPH, desiredVolumeMLPerPlant);
   lcd.clear();
   Serial.println("Begin");
   RtcDateTime now = Rtc.GetDateTime();

   // Read DHT22 sensor data
   float temperature = dht.readTemperature();
   float humidity = dht.readHumidity();

   lcd.setCursor(0, 0);
   lcd.print("Time: ");
   if(now.Hour()<10) lcd.print("0");
   lcd.print(now.Hour(), DEC);
   lcd.print(':');
   if(now.Minute()<10) lcd.print("0");
   lcd.print(now.Minute(), DEC);
   lcd.print(':');
   if(now.Second()<10) lcd.print("0");
   lcd.print(now.Second(), DEC);

   // Display DHT readings on the LCD
   lcd.setCursor(0, 1);
   lcd.print("T: ");
   lcd.print(temperature);
   lcd.print("C");
   lcd.print(" H: ");
   lcd.print(humidity);
   lcd.print("%");

   lcd.setCursor(0, 2);
   lcd.print("Soil:");
   lcd.print(readSoilMoisture());


   int nowHour = now.Hour();
   int nowSecond = now.Second();
   int nowMinute = now.Minute();
   if (shouldWaterPlant(nowHour, nowMinute, nowSecond)) {
      // if (readSoilMoisture() < threshold) {
      //   waterPlant(wateringDurationSeconds);
      // }
      timedWaterPlant(wateringDurationSeconds, now.Second(), now.Minute());
   }else{
      if(readSoilMoisture() <= 30){
         soilWaterPlant(readSoilMoisture());
      } // Delay for 1 second between readings
   }
   // Other tasks or sensor readings can be added here
   // ...
   delay(1000);
}
// Read soil moisture level (optional)
