#include <RHReliableDatagram.h>
#include <RH_RF95.h>

const int rh_server_address = 1;
const int rh_client_address = 2;

RH_RF95<HardwareSerial> driver(Serial1);
RHReliableDatagram manager(driver, rh_client_address);

void setup() {
    TXLED0;
    RXLED0;
    if (!manager.init()) {
        Serial.println("RH init failed");
        TXLED1;
    }
}

int8_t water_temp = 0;
int8_t ambient_temp = 0;

void loop() {
    Serial1.println("wtf");
    RXLED1;
    delay(1000);
    Serial1.println("wtf");
    RXLED0;
    delay(1000);
    if (!manager.sendtoWait((ambient_temp<<8)|water_temp, 2, rh_server_address)) {
        Serial.println("sendtoWait failed");
    }
    water_temp++;
    ambient_temp++;
}

