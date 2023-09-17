#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

DS1302 rtc;

const int relayPin = 12;
const float flowRateLPH = 300.0; // Flow rate in liters per hour
const float desiredVolumeMLPerPlant = 500.0; // Desired volume
const int plantRows = 16;
const int plantPerRows = 20;
float wateringDurationSeconds;

void setup() {
   Serial.begin(115200);
   pinMode(relayPin, OUTPUT);

   rtc.begin();
   wateringDurationSeconds = calculateWateringDuration(flowRateLPH, desiredVolumeMLPerPlant);
  
   // Time init setup
   // rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop() {
   DateTime now = rtc.now();

   unsigned long secondsSinceMidnight = now.hour() * 3600 + now.minute() * 60 + now.second();

   if (shouldWaterPlant(secondsSinceMidnight)) {

      // if (readSoilMoisture() < threshold) {
      //   waterPlant(wateringDurationSeconds);
      // }
      waterPlant(wateringDurationSeconds);
   }

   // Other tasks or sensor readings can be added here
   // ...

   delay(1000); // Delay for 1 second between readings
}

bool shouldWaterPlant(unsigned long secondsSinceMidnight) {
   // Calculate the time intervals for watering (5 times in 24 hours)
   int intervals[] = {1, 6, 11, 16, 21}; // Adjust as needed

   // Check if the current time falls within one of the intervals
   for (int i = 0; i < 5; i++) {
      if (secondsSinceMidnight >= intervals[i] * 3600 &&
         secondsSinceMidnight < (intervals[i] + wateringDurationSeconds) * 3600) {
      return true;
      }
   }
   return false;
}

void waterPlant(float durationSeconds) {
   digitalWrite(relayPin, HIGH);
   Serial.print("Watering the plant for ");
   Serial.print(durationSeconds);
   Serial.println(" seconds...");
   delay(durationSeconds * 1000); // Convert seconds to milliseconds and run the pump
   digitalWrite(relayPin, LOW);
   Serial.println("Watering complete.");
}

float calculateWateringDuration(float flowRateLPH, float desiredVolumeMLPerPlant) {

   // Convert flow rate to milliliters per second
   float flowRateMLPS = flowRateLPH * (5/18);
   float perPlantFlowRateMLPS = flowRateMLPS / (plantRows*plantPerRows);

   // Calculate the watering duration in seconds
   float durationSeconds = (desiredVolumeMLPerPlant / perPlantFlowRateMLPS);

   return durationSeconds;
}

// Read soil moisture level (optional)
// int readSoilMoisture() {
//    // Read the analog value from the soil moisture sensor
//    int sensorValue = analogRead(A0); // Connect the sensor to an appropriate pin
//    // Convert the sensor value to a moisture percentage
//    int moisturePercentage = map(sensorValue, 0, 4095, 0, 100);
//    return moisturePercentage;
// }
