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

int relayPin[5] = {13, 12, 14, 27, 26}; // Relay pins for each soil sensor, including the new relay
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

void setup() {
   Serial.begin(115200);
   for(int i = 0; i < 4; i++){
       pinMode(relayPin[i], OUTPUT);
       digitalWrite(relayPin[i], LOW);
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
   lcd.setCursor(10, 1);
   lcd.print(" H: ");
   lcd.print(humidity);
   lcd.print("%");

   lcd.setCursor(0, 3);
   lcd.print("SOIL : ");
   for (int i = 0; i < 3; i++) {
      soilReadings[i] = readSoilMoisture(i);
      lcd.print(soilReadings[i]);
      if (i < 3) {
         lcd.print(" ");
      }
   }

   // Check soil moisture and control relays
   for (int i = 0; i < 3; i++) {
      if (soilReadings[i] <= 30) {
         // Soil is too dry, activate the corresponding relay
         digitalWrite(relayPin[i], HIGH);
         digitalWrite(relayPin[3], HIGH);
      } else if (soilReadings[i] > desiredMoistureLevel){
         // Soil is moist enough, deactivate the corresponding relay
         digitalWrite(relayPin[i], LOW);
      }
   }

   if(soilReadings[0] > desiredMoistureLevel && soilReadings[1] > desiredMoistureLevel && soilReadings[2] > desiredMoistureLevel){
      digitalWrite(relayPin[3], LOW);
   }  

   // Control the new relay based on whether any other relay is ON
   // Other tasks or sensor readings can be added here
   // ...
   delay(1000);
}