#include <OneWire.h>
#include <DHT.h>

// Sketch to run on Arduino (Uno)
// monitoring environment in Stacken Computer Club server room
// using one or more Dallas DS18[S]20 temperature sensors,
// a DHT22 Temp+Humidity sensor
// and a Light Dependent Resistor to detect lights on/off

//Outputs serial lines in these formats:
// 0 <sp> Comment-text
// -n <sp> Error text for error n
// n <sp> data for type n:
//  1 sensor-addr(8HEX) temp(float w decimal point)
//  2 0 light-level(int 0..1023, close to 0 in darkness)
//  3 0 temp(float) humidity(???)

OneWire  ds(12);  // on pin 12 - RJ45 Twisted Pair connector
DHT  dht(8, DHT22);  // on pin 8 - coax cable
//LDR is connected from +3.3 to Analog 0, with a resistor to ground.

void setup(void) {
  Serial.begin(9600);
  Serial.print("0 StackEnvMon V2.0 stellanl@stacken.kth.se\n");
  dht.begin();
}


//Print a hardware address
void pa(byte *addr){
  for(byte i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
  }
  Serial.print(" ");
}


void loop(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  
  if ( !ds.search(addr)) {
      //We have read all the temp sensors
      Serial.print("0 No more addresses.\n");
      ds.reset_search();
      delay(250);
      //Read and send light level
      int light = analogRead(A0);
      Serial.print("2 0 ");
      Serial.print(light,DEC);
      Serial.print("\n");
      
      // Wait a few seconds extra for DHT22 in case we had no DS sensors.
      delay(2000);
    
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius
      float t = dht.readTemperature();
      
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("-4 Failed to read from DHT sensor!");
        return;
      }
      Serial.print("3 0 ");
      Serial.print(t);
      Serial.print(" ");
      Serial.print(h);
      Serial.print("\n");
      
      return;
  }
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("-1 Search CRC is not valid!\n");
      return;
  }

  //address ok

  if ( addr[0] != 0x10) {//todo: support for DS18B22
      Serial.print("-2 ");
      pa(addr);
      Serial.print("Device is not a DS18S20 family device.\n");
      return;
  }

  //sensor ok, get data
  ds.reset();
  ds.select(addr);
//  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  ds.write(0x44);         // start conversion, without parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
//    Serial.print(data[i], HEX);//DEBUG
//    Serial.print(" ");
  }
  //check CRC
  if ( OneWire::crc8( data, 8) != data[8]) {
      Serial.print("-3 ");
      pa(addr);
      Serial.print("Data CRC is not valid!\n");
      return;
  }

  //decode and send temperature
  float t = ((data[0] & 0xfe) + 256*data[1])/2.0
           - 0.25 + (16-data[6])/16.0;
  Serial.print("1 ");//this is temp
  pa(addr);
  Serial.print(t);
  //end of the line
  Serial.print("\n");
}

