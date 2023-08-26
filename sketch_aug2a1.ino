#include <TBPubSubClient.h>

#include <Arduino.h>
#include <LiquidCrystal_I2C.h> // Include the LCD library
#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

#define FREQUENCY_HZ 104
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

//LCD DISPLAY
LiquidCrystal_I2C lcd(0x27,20,4);
void setup_wifi();
void reconnect();
const int moistureSensorPin = A0;
const int pHSensorPin = A1;
const int OpenAirReading = 700;   //calibration data 1
const int WaterReading = 280;     //calibration data 2
int MoistureLevel = 0;
int moisture = 0 ;
float pHValue=0;
float pH=0;
static char payload[256];

StaticJsonDocument<256> doc;

const char* ssid = "ssid";
const char* password = "password";
const char mqtt_server[] = "broker.mqtt-dashboard.com";
const char publishTopic[] = "sense/soil";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);


void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while( WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("."); 
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect(){


  while(!mqtt.connected()){

   byte mac[6];
    WiFi.macAddress(mac);
    String macAddress = "";
    for (int i = 0; i < 6; i++) {
      macAddress += String(mac[i], HEX);
      if (i < 5) macAddress += ":";
    }
  if (mqtt.connect(macAddress.c_str(), "", NULL)) {
      Serial.println("Connected to MQTT broker");
      digitalWrite(LED_BUILTIN, HIGH);

    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);

      delay(5000);


    }
  }
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Soil Moisture:");

  lcd.setCursor(0, 1);
  lcd.print("%");
  lcd.setCursor(15,0);
  lcd.print("pH:");
}


void loop() {


  if (!mqtt.connected())

  {
    reconnect();

  }
  mqtt.loop();


  static unsigned long last_interval_ms = 0;
  if (millis() > last_interval_ms + INTERVAL_MS)
  
  { last_interval_ms = millis();
    MoistureLevel = analogRead(A0); //update based on the analog Pin selected
    moisture = map(MoistureLevel, OpenAirReading, WaterReading, 0, 100);
    
    pHValue= analogRead(A1);
    pH = map(pHValue, 0, 1023, 0, 140) / 10.0;
    
    Serial.print(pH);
    //lcd display
    lcd.setCursor(14, 0);
    lcd.print(moisture);
    lcd.setCursor(3, 1);
    lcd.print(pH);
  

    doc["Soil-moisture"]= moisture;
    
    doc["Soil-pH"]=pH;
    serializeJsonPretty(doc, payload);

   

    mqtt.publish(publishTopic, payload);

    Serial.println(payload);

 

  }


}
