#include "auxiliary.h"

void set_eeprom_default(){
    /* From the prof example we should be able to just uncomment this and it should work for the default values
    DATAEE_WriteByte(ADDR_PMON, 5);
    DATAEE_WriteByte(ADDR_TALA, 3);
    DATAEE_WriteByte(ADDR_TINA, 10);
    DATAEE_WriteByte(ADDR_ALAF, 0);
    DATAEE_WriteByte(ADDR_ALAH, 12);
    DATAEE_WriteByte(ADDR_ALAM, 0);
    DATAEE_WriteByte(ADDR_ALAS, 0);
    DATAEE_WriteByte(ADDR_ALAT, 20);
    DATAEE_WriteByte(ADDR_ALAL, 2);
    DATAEE_WriteByte(ADDR_CLKH, 0);
    DATAEE_WriteByte(ADDR_CLKM, 0);
     */
}

void write_eeprom(uint16_t address, uint8_t value){
    //DATAEE_WriteByte(ADDR_CLKM, value);
}

uint8_t read_eeprom(uint16_t address){
    //return DATAEE_ReadByte(address)
}


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

void btn_update(volatile Btn *btn, uint8_t pin) {
    btn->hist = (btn->hist<<1) | pin;
    
    if (btn->hist == ((0b10000000 * BTN_RELEASED) | (0b01111111 * BTN_PRESSED))) {
        btn->event = 1;
    }
}

extern volatile uint8_t update_lcd;

uint8_t scan_btn(volatile Btn *btn) {
    uint8_t ret = 0;
    if(btn->event > 0) {
        ret = 1;
        update_lcd = 2;
    }
    btn->event = 0;
    return ret;
}