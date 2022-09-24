#include <OneWire.h>
#include <LowPower.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <DallasTemperature.h>

const int rh_server_address = 1;
const int rh_client_address = 2;

RH_RF95<HardwareSerial> driver(Serial1);
RHReliableDatagram manager(driver, rh_client_address);
OneWire one_wire1(4);
OneWire one_wire2(5);
DallasTemperature sensor_water(&one_wire1);
DallasTemperature sensor_ambient(&one_wire2);
const int radio_power_sw = 6;
float water_temp = 0;
float ambient_temp = 0;
uint8_t buf[8];

const int sleep_time = 60*10;
const int sleep_cycles = sleep_time/8;
const int tx_time = 50;

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
    sensor_water.begin();
    sensor_ambient.begin();
    sensor_water.setResolution(9);
    sensor_ambient.setResolution(9);
    sensor_water.setWaitForConversion(false);
    sensor_ambient.setWaitForConversion(false);
}

void loop() {
    sensor_water.requestTemperatures();
    sensor_ambient.requestTemperatures();
    delay(100); // wait for 9 bit temperature conversion
    water_temp = sensor_water.getTempCByIndex(0);
    ambient_temp = sensor_ambient.getTempCByIndex(0);
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
        // init failed
        delay(100); //wait to retry
    }
    driver.setTxPower(23, false);
    if (!manager.sendtoWait(buf, sizeof(buf), rh_server_address)) {
        // sendtowait failed
    }
    Serial1.flush();
    delay(1);
    digitalWrite(radio_power_sw,LOW);
    RXLED1;
    delay(tx_time);
    RXLED0;
    for(int sleep_counter=0;sleep_counter < sleep_cycles;sleep_counter++){
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}

