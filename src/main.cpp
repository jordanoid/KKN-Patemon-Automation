#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS1302.h>

#define DHT_PIN 5 // DHT22 sensor pin
#define DHT_TYPE DHT22 // Specify the DHT sensor type (DHT11, DHT22, or DHT21)
DHT dht(DHT_PIN, DHT_TYPE);

ThreeWire myWire(2, 15, 4); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

LiquidCrystal_I2C lcd(0x27, 20, 4);

int relayPin[7] = {14, 12, 13, 27, 26, 25, 33}; // Relay pins for each soil sensor, including the new relay
int soilPin[4] = {A0, A3, A6, A7};
const int desiredMoistureLevel = 50; // Adjust the desired moisture level as needed

int soilReadings[4]; // Array to store soil moisture readings

int readSoilMoisture(int index) {
   // Read the analog value from the soil moisture sensor
   int sensorValue = analogRead(soilPin[index]); // Connect the sensor to an appropriate pin
   // Convert the sensor value to a moisture percentage
   int moisturePercentage = map(sensorValue, 0, 4095, 100, 0); // Reverse mapping
   return moisturePercentage;
}

String padding(int numSpaces) { 
   String spaceString;
   for (int i = 0; i < numSpaces; i++) {
    spaceString += ' '; // Concatenate space characters
   } 
   return spaceString;
}

void setup() {
   Serial.begin(115200);
   for(int i = 0; i < 7; i++){
      pinMode(relayPin[i], OUTPUT);
      digitalWrite(relayPin[i], LOW);
      delay(250);
      digitalWrite(relayPin[i], HIGH);
      delay(250);
   }
   for(int i = 0; i < 3; i++){
      pinMode(soilPin[i], INPUT);
   }

   Rtc.Begin();

   // RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
   // Rtc.SetDateTime(compiled);  
   dht.begin();

   lcd.init();
   lcd.backlight();
   lcd.clear();
}

void loop() {
   // lcd.clear();
   
   RtcDateTime now = Rtc.GetDateTime();

   // Display Date and Time
   lcd.setCursor(0, 0);
   String formattedDateTime = String(now.Year()) + "-" +
                              String(now.Month()) + "-" +
                              String(now.Day()) + " " +
                              String(now.Hour()) + ":" +
                              String(now.Minute()) + ":" +
                              String(now.Second());
   lcd.println(formattedDateTime);

   // lcd.print("Time: ");
   // if(now.Hour()<10) lcd.print("0");
   // lcd.print(now.Hour(), DEC);
   // lcd.print(':');
   // if(now.Minute()<10) lcd.print("0");
   // lcd.print(now.Minute(), DEC);
   // lcd.print(':');
   // if(now.Second()<10) lcd.print("0");
   // lcd.print(now.Second(), DEC);
   // lcd.print("   ");

   // Read and Display DHT readings on the LCD
   float temperature = dht.readTemperature();
   float humidity = dht.readHumidity();
   lcd.setCursor(0, 1);
   lcd.print("T: ");
   lcd.print(temperature);
   lcd.print("C ");
   lcd.setCursor(10, 1);
   lcd.print(" H: ");
   lcd.print(humidity);
   lcd.print("% ");

   // Read and Display Soil Sensor  h
   for (int i = 0; i < 3; i++) soilReadings[i] = readSoilMoisture(i);
   lcd.setCursor(0, 2);
   lcd.print("SOIL : ");
   char soil_buff[14];
   sprintf(soil_buff, "%d%% %d%% %d%%", soilReadings[0], soilReadings[1], soilReadings[2]);
   String soil = String(soil_buff) + padding(13-strlen(soil_buff));
   lcd.print(soil);

   // for (int i = 0; i < 3; i++) {
   //    soilReadings[i] = readSoilMoisture(i);
   //    lcd.print(soilReadings[i]);
   //    lcd.print("%");
   //    if (i < 3) {
   //       lcd.print(" ");
   //    }
   // }
   // lcd.print("");

   // Check soil moisture and control relays
   for (int i = 0; i < 3; i++) {
      if (soilReadings[i] <= 30) {
         // Soil is too dry, activate the corresponding relay
         digitalWrite(relayPin[i], LOW);
         digitalWrite(relayPin[3], LOW);
         digitalWrite(relayPin[4], LOW);
         digitalWrite(relayPin[5], LOW);
         digitalWrite(relayPin[6], LOW);
      } else if (soilReadings[i] > desiredMoistureLevel){
         // Soil is moist enough, deactivate the corresponding relay
         digitalWrite(relayPin[i], HIGH);
      }
   }

   if(soilReadings[0] > desiredMoistureLevel && soilReadings[1] > desiredMoistureLevel && soilReadings[2] > desiredMoistureLevel){
      digitalWrite(relayPin[3], HIGH);
      digitalWrite(relayPin[4], HIGH);
      digitalWrite(relayPin[5], HIGH);
      digitalWrite(relayPin[6], HIGH);
   }

   // Serial.print(now.Hour());
   // Serial.print(":");
   // Serial.print(now.Minute());
   // Serial.print(":");
   // Serial.println(now.Second());

   // Control the new relay based on whether any other relay is ON
   // Other tasks or sensor readings can be added here
   // ...
   delay(1000);
}