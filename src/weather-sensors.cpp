/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/scottperkins/Documents/weather-station/weather-sensors/src/weather-sensors.ino"
#include "MQTT.h"

void callback(char *topic, byte *payload, unsigned int length);
void setup();
void loop();
void calculate_windspeed();
void wind_speed_ISR();
#line 3 "/Users/scottperkins/Documents/weather-station/weather-sensors/src/weather-sensors.ino"
SerialLogHandler logHandler;
MQTT client("192.168.3.2", 1883, callback);
// MQTT client("" 1883, callback);

// This is called when a message is received. However, we do not use this feature in
// this project so it will be left empty
void callback(char *topic, byte *payload, unsigned int length)
{
}

int WIND_SPEED_PIN = D3;
int WIND_DIR_PIN = 12;

int numberOfWindTicks = 0;
volatile long lastWindIRQ = 0;
float currentWindSpeed = 0;

Timer timer(1000, calculate_windspeed);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.info("This is from the logger");
  pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
  attachInterrupt(WIND_SPEED_PIN, wind_speed_ISR, RISING);
  // Connect to the server and call ourselves "photonDev"
  client.connect("weather-station");

  timer.start();
}

void loop()
{
  delay(5000);
  Log.info(String(currentWindSpeed));
  char buf[1024];
  JSONBufferWriter writer(buf, sizeof(buf));
  writer.beginObject();
  writer.name("windSpeed").value(currentWindSpeed);
  writer.endObject();

  if (client.isConnected())
  {
    Log.info(buf);
    Log.info("Sending weather data");
    client.publish("weather-station", buf);
  }
  client.loop();
}

void calculate_windspeed()
{
  //Calculate the wind speed based on the number of ticks
  currentWindSpeed = numberOfWindTicks *= 1.492;
  //Reset the wind tick counter
  numberOfWindTicks = 0;
}

void wind_speed_ISR()
{
  if (millis() - lastWindIRQ > 10)
  {
    lastWindIRQ = 0;
    numberOfWindTicks++;
  }
}