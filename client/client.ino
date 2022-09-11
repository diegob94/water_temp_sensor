#include <RHReliableDatagram.h>
#include <RH_RF95.h>

const int rh_server_address = 1;
const int rh_client_address = 2;

RH_RF95<HardwareSerial> driver(Serial1);
RHReliableDatagram manager(driver, rh_client_address);
int8_t water_temp = 0;
int8_t ambient_temp = 0;
uint8_t buf[2];

const int period = 10000;
const int led_time = 1000;

void setup() {
    TXLED0;
    RXLED0;
    if (manager.init()) {
        TXLED1;
    } else {
        Serial.println("RH init failed");
    }
}

void loop() {
    buf[0] = water_temp;
    buf[1] = ambient_temp;
    if (!manager.sendtoWait(buf, sizeof(buf), rh_server_address)) {
        TXLED0;
        Serial.println("sendtoWait failed");
    }
    water_temp--;
    ambient_temp++;
    RXLED1;
    delay(led_time);
    RXLED0;
    delay(period - led_time);
}

