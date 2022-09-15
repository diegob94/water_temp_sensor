#include <OneWire.h>
#include <LowPower.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

const int rh_server_address = 1;
const int rh_client_address = 2;

RH_RF95<HardwareSerial> driver(Serial1);
RHReliableDatagram manager(driver, rh_client_address);
OneWire one_wire1(4);
OneWire one_wire2(5);
const int radio_power_sw = 6;
float water_temp = 0;
float ambient_temp = 0;
uint8_t buf[8];
int sleep_counter = 0;

const int sleep_time = 1000*60*10;
const int tx_time = 50;

bool getTemp(OneWire ds, float* temp){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return false;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return false;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return false;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  *temp = TemperatureSum;
  return true;
}

uint8_t float_get_byte(float x, int idx){
    union my_union {
        float float_variable;
        char bytes_array[4];
    };
    union my_union u;
    u.float_variable = x;
    return u.bytes_array[idx];
}

void setup() {
    pinMode(radio_power_sw,OUTPUT);
    digitalWrite(radio_power_sw,LOW);
    TXLED0;
    RXLED0;
}

void loop() {
    getTemp(one_wire1, &water_temp);
    getTemp(one_wire2, &ambient_temp);
    buf[0] = float_get_byte(water_temp,0);
    buf[1] = float_get_byte(water_temp,1);
    buf[2] = float_get_byte(water_temp,2);
    buf[3] = float_get_byte(water_temp,3);
    buf[4] = float_get_byte(ambient_temp,0);
    buf[5] = float_get_byte(ambient_temp,1);
    buf[6] = float_get_byte(ambient_temp,2);
    buf[7] = float_get_byte(ambient_temp,3);
    digitalWrite(radio_power_sw,HIGH);
    delay(10); // wait radio power on
    while(!manager.init()) {
        Serial.println("RH init failed");
        delay(100); //wait to retry
    }
    if (!manager.sendtoWait(buf, sizeof(buf), rh_server_address)) {
        TXLED0;
        Serial.println("sendtoWait failed");
    }
    digitalWrite(radio_power_sw,LOW);
    RXLED1;
    delay(tx_time);
    RXLED0;
    for(sleep_counter=0;sleep_counter < (sleep_time/(8*1000));sleep_counter++){
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}

