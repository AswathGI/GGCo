#include<ESP8266WiFi.h>
#include "Adafruit_MQTT_Client.h"

//#define ldr A0
#define wifi "UWE-Guest"


#define server "io.adafruit.com"

#define port 1883
#define username "TheGreenerGreensCo"
#define key "aio_mbfi15dstb6CrcJFEnObYQRPTbXp"

WiFiClient esp;
Adafruit_MQTT_Client mqtt(&esp, server, port, username,key);
Adafruit_MQTT_Publish feed1 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/room-temp");
Adafruit_MQTT_Publish feed2 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/room-humidity");
Adafruit_MQTT_Publish feed3 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/watertest");

//#############################
//     STH
#include <Arduino.h>

#include <SHT1x-ESP.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  10
#define clockPin 13

// default to 5.0v boards, e.g. Arduino UNO
SHT1x sht1x(dataPin, clockPin);
//##############################





void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("Adafruit MQTT demo");
  Serial.print("Connecting to ");
  Serial.println(wifi);

  WiFi.begin(wifi);
  
  //Water sensor
  pinMode(D1, INPUT);

  while (WiFi.status()!= WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    
  }


Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
Serial.print("Connecting to MQTT");


while(mqtt.connect())
{
  Serial.print(",");
  
}

}


void loop()
{
  if(mqtt.connected());
  {


    //==========================
    //=      Sensor Data       =
    //==========================
   //from WaterLevel
    int sensor1 = digitalRead(D1);
    int waterlevel = 0;

  
    float temp_c;
    float temp_f;
    float humidity;

    // Read values from the sensor
    temp_c = sht1x.readTemperatureC();
    temp_f = sht1x.readTemperatureF();
    humidity = sht1x.readHumidity();
    //temp_f = 100;
    //humidity = 56;
    //temp_c = 12;

    // Print the values to the serial port
    Serial.print("Temperature: ");
    Serial.print(temp_c, DEC);
    Serial.print("C / ");
    Serial.print(temp_f, DEC);
    Serial.print("F. Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Water Level code --  
   if (sensor1 == HIGH ) 
    {
      waterlevel = 1;
      Serial.println(waterlevel);
    } 

   else
    {
     waterlevel = 0;
     Serial.println(waterlevel);    
   }
    // -- x -- WaterLevel


    if (feed1.publish(temp_c))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
      

      if (feed2.publish(humidity))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
      
    if (feed3.publish(waterlevel))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
   }
   delay(10000); 
}
