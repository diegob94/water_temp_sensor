
void setup() {
    TXLED0;
    RXLED0;
}

void loop() {
    RXLED1;
    TXLED0;
    delay(1000);
    RXLED0;
    TXLED1;
    delay(1000);
}

