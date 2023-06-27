#include<ESP8266WiFi.h>
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_MCP3008.h>

//==========================
//=      Sensor Setup      =
//==========================

//EC
#include "DFRobot_EC10.h"
#include <EEPROM.h>

//DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 // DS18B20 on arduino pin2 corresponds to D4 on physical board "D4 pin on the ndoemcu Module"
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

//Ph code items--
#define Offset1 0.00            //deviation compensate
#define samplingInterval1 20
#define printInterval1 800
#define ArrayLenth1  40    //times of collection
int pHArray[ArrayLenth1];   //Store the average value of the sensor feedback
int pHArrayIndex=0;
// -- x --

//DFRobot
float voltage,ecValue,phValue, temperature = 0;
DFRobot_EC10 ec;
//temp
float temp;
float Fahrenheit=0;



Adafruit_MCP3008 adc;

int waterlevel = 0;

// Sensor Setup //
// -- x --

//==========================
//=          WiFi          =
//==========================

#define wifi "UWE-Guest"

//#define password "d57dnamzaqgab"
#define server "io.adafruit.com"

#define port 1883
#define username "TheGreenerGreensCo"
#define key "aio_mbfi15dstb6CrcJFEnObYQRPTbXp"


WiFiClient esp;
Adafruit_MQTT_Client mqtt(&esp, server, port, username,key);
Adafruit_MQTT_Publish feed1 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/waterlevel");
Adafruit_MQTT_Publish feed2 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/ph");
Adafruit_MQTT_Publish feed3 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/ec");
Adafruit_MQTT_Publish feed4 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/temp");
Adafruit_MQTT_Publish feed5 = Adafruit_MQTT_Publish(&mqtt, username"/feeds/volt");


//==========================
//=         SETUP          =
//==========================

void setup()
{
  // Sensor 
  Serial.begin(115200);
  while(!Serial);
  Serial.println("MCP3008 simple test.");
  DS18B20.begin();
  adc.begin();
  ec.begin();

  //configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(D1, INPUT);
  //pinMode(D2, INPUT);

  
  Serial.begin(115200);
  delay(10);
  Serial.println("Adafruit MQTT demo");
  Serial.print("Connecting to ");
  Serial.println(wifi);

  WiFi.begin(wifi);

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
  //int sensor2 = digitalRead(D2);
  int waterlevel = 0;

  //from ADC
  int EC_sensor = adc.readADC(6);
  int pH_sensor = adc.readADC(7);

 /* 
  int raw = adc.readADC(1);
  Serial.print(raw);
  Serial.println("\t");
  Serial.println(voltage3(raw));
  */

// -- Temp Code Start
  DS18B20.requestTemperatures(); 
  temp = DS18B20.getTempCByIndex(0); // Celcius
  Fahrenheit = DS18B20.toFahrenheit(temp); // Fahrenheit
  //Serial.println(temp);
  //Serial.println(Fahrenheit);
// -- x -- Temp

temperature = temp;
 
// -- EC code start
    static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U)  //time interval: 1s
    {
      timepoint = millis();
      voltage = EC_sensor/1024.0*5000;   // read the voltage

      //temperature = readTemperature();          // read your temperature sensor to execute temperature compensation
      ecValue =  ec.readEC(voltage,temperature);  // convert voltage to EC with temperature compensation

      Serial.print("temperature:");
      Serial.print(temperature,1);
      Serial.print("^C  EC:");
      Serial.print(ecValue,2);
      Serial.println("ms/cm");

    }
    ec.calibration(voltage,temperature);    // calibration process by Serail CMD
// -- x -- EC 


// -- pH code --
  static unsigned long samplingTime2 = millis();
  static unsigned long printTime2 = millis();
  static float pHValue,voltage2;
  if(millis()-samplingTime2 > samplingInterval1)
  {
      pHArray[pHArrayIndex++]=pH_sensor;
      if(pHArrayIndex==ArrayLenth1)pHArrayIndex=0;
      voltage2 = avergearray(pHArray, ArrayLenth1)*5.0/1024;
      pHValue = 3.5*voltage2 + Offset1;
      samplingTime2=millis();
  }
  if(millis() - printTime2 > printInterval1)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
        Serial.print("Voltage:");
        Serial.print(voltage2,2);
        Serial.print("    pH value: ");
        Serial.println(pHValue,2);
        printTime2=millis();
  }
 // -- x -- pH

// Water Level code --  
  if (sensor1 == HIGH ) 
  {
    waterlevel = 100;
    Serial.println(waterlevel);
  } 

  else
  {
    waterlevel = 10;
    Serial.println(waterlevel);    
  }
// -- x -- WaterLevel
    

    if (feed1.publish(waterlevel))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }

      if (feed2.publish(pHValue))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }

    if (feed3.publish(ecValue))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
      
    if (feed4.publish(temperature))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
      
    if (feed5.publish(voltage2))
      {
        Serial.println("Success");
       }
    else
      {
        Serial.println("Fail!");
      }
    
   }
   delay(60000); 
}


//pH code --
double avergearray(int* arr, int number)
{
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
// -- x -- pH
