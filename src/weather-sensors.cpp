/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/scottperkins/Documents/weather-station/weather-sensors/src/weather-sensors.ino"
#include "MQTT.h"
#include "SparkFun_Photon_Weather_Shield_Library.h"

void callback(char *topic, byte *payload, unsigned int length);
void setup();
void loop();
void calculate_windspeed();
void calculate_rainfall();
void wind_speed_ISR();
void rain_fall_ISR();
int get_wind_direction();
#line 4 "/Users/scottperkins/Documents/weather-station/weather-sensors/src/weather-sensors.ino"
SerialLogHandler logHandler;
MQTT client("192.168.3.2", 1883, callback);

// This is called when a message is received. However, we do not use this feature in
// this project so it will be left empty
void callback(char *topic, byte *payload, unsigned int length)
{
}

int WIND_SPEED_PIN = D3;
int WIND_DIR_PIN = A0;
int RAIN_PIN = D2;

int numberOfWindTicks = 0;
int numberOfRainTicks = 0;

volatile long lastWindIRQ = 0;
volatile long lastRainIRQ = 0;

float currentWindSpeed = 0;
float currentRainFall = 0;

int winddir = 0; // [0-360 instantaneous wind direction]
float humidity = 0;
float tempf = 0;
float pascals = 0;
float baroTemp = 0;

char buf[1024];
Timer timer(1000, calculate_windspeed);
Timer rain_timer(60000, calculate_rainfall);
Weather sensor;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Log.info("System version: %s", System.version().c_str());
  //Initialize the I2C sensors and ping them
  sensor.begin();            //This will print out which devices it has detected
  sensor.setModeBarometer(); //Set to Barometer Mode
  //baro.setModeAltimeter();//Set to altimeter Mode

  //These are additional MPL3115A2 functions the MUST be called for the sensor to work.
  sensor.setOversampleRate(7); // Set Oversample rate

  sensor.enableEventFlags(); //Necessary register calls to enble temp, baro ansd alt

  pinMode(RAIN_PIN, INPUT_PULLUP);
  attachInterrupt(RAIN_PIN, rain_fall_ISR, RISING);

  pinMode(WIND_SPEED_PIN, INPUT_PULLUP);
  attachInterrupt(WIND_SPEED_PIN, wind_speed_ISR, RISING);
  // Connect to the server and call ourselves "photonDev"
  client.connect("weather-station");

  timer.start();
}

void loop()
{
  humidity = sensor.getRH();
  tempf = sensor.getTempF();
  baroTemp = sensor.readBaroTempF();

  //Measure Pressure from the MPL3115A2
  pascals = sensor.readPressure();
  float hpa = pascals / 100;
  int wind_direction = get_wind_direction();

  memset(buf, 0, sizeof(buf));
  JSONBufferWriter writer(buf, sizeof(buf) - 1);
  writer.beginObject();
  writer.name("windSpeed").value(currentWindSpeed);
  writer.name("windDir").value(wind_direction);
  writer.name("tempF").value(tempf);
  writer.name("humidity").value(humidity);
  writer.name("pressure").value(hpa);
  writer.name("baroTemp").value(baroTemp);
  writer.name("rainFall").value(currentRainFall);
  writer.endObject();

  if (client.isConnected())
  {
    client.publish("weather-station", buf);
  }
  client.loop();
  delay(5000);
}

void calculate_windspeed()
{
  //Calculate the wind speed based on the number of ticks
  currentWindSpeed = numberOfWindTicks *= 1.492;
  //Reset the wind tick counter
  numberOfWindTicks = 0;
}

void calculate_rainfall()
{
  Log.info("Calculating Rainfall");
  currentRainFall = 0.011 * numberOfRainTicks;
  numberOfRainTicks = 0;
}

void wind_speed_ISR()
{
  if (millis() - lastWindIRQ > 10)
  {
    lastWindIRQ = millis();
    numberOfWindTicks++;
  }
}

void rain_fall_ISR()
{
  if (millis() - lastRainIRQ > 10)
  {
    Log.info("Rainfall interupt");
    lastRainIRQ = millis();
    numberOfRainTicks++;
  }
}

int get_wind_direction()
{
  unsigned int adc;

  adc = analogRead(WIND_DIR_PIN); // get the current reading from the sensor

  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
  // Note that these are not in compass degree order! See Weather Meters datasheet for more information.

  if (adc < 1500)
    return (113);
  if (adc < 1570)
    return (68);
  if (adc < 1600)
    return (90);
  if (adc < 1730)
    return (158);
  if (adc < 1940)
    return (135);
  if (adc < 2100)
    return (203);
  if (adc < 2300)
    return (180);
  if (adc < 2700)
    return (23);
  if (adc < 2900)
    return (45);
  if (adc < 3200)
    return (248);
  if (adc < 3300)
    return (225);
  if (adc < 3500)
    return (338);
  if (adc < 3630)
    return (0);
  if (adc < 3750)
    return (293);
  if (adc < 3900)
    return (315);
  if (adc < 4096)
    return (270);
  return (-1); // error, disconnected?
}
