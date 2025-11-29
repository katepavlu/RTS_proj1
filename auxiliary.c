#include "auxiliary.h"

unsigned char readTC74(void) {
    unsigned char value;
    do {
        IdleI2C();
        StartI2C();
        IdleI2C();

        WriteI2C(0x9a | 0x00);
        IdleI2C();
        WriteI2C(0x01);
        IdleI2C();
        RestartI2C();
        IdleI2C();
        WriteI2C(0x9a | 0x01);
        IdleI2C();
        value = ReadI2C();
        IdleI2C();
        NotAckI2C();
        IdleI2C();
        StopI2C();
    } while (!(value & 0x40));

    IdleI2C();
    StartI2C();
    IdleI2C();
    WriteI2C(0x9a | 0x00);
    IdleI2C();
    WriteI2C(0x00);
    IdleI2C();
    RestartI2C();
    IdleI2C();
    WriteI2C(0x9a | 0x01);
    IdleI2C();
    value = ReadI2C();
    IdleI2C();
    NotAckI2C();
    IdleI2C();
    StopI2C();

    return value;
}

void btn_update(Btn *btn, uint8_t pin) {
    if (pin == BTN_PRESSED) {
        if (btn->timer < 20) btn->timer++;

        if (btn->timer >= BTN_DEBOUNCE && btn->processed == 0) {
            btn->event = 1;
            btn->processed = 1;
        }
    } else {
        btn->timer = 0;
        btn->processed = 0;
    }
}