#include "auxiliary.h"
#include "mcc_generated_files/mcc.h"

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
    // https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/
    if (
            (btn->hist & 0b11000111)
            == 
            ((0b11000000 * BTN_RELEASED) | (0b00000111 * BTN_PRESSED))) {
        btn->event = 1;
        btn->hist = 0b11111111 * BTN_PRESSED;
    }
}

uint8_t handle_btn(volatile Btn *btn) {
    uint8_t ret = 0;
    if(btn->event > 0) {
        ret = 1;
        update_lcd = 1;
    }
    btn->event = 0;
    return ret;
}

// make sure the buffer buf fits 17 characters or more
void print_record(char* buf, Record* rec) {
    sprintf(buf, 
            "%02d:%02d:%02d  %02dC L%d",
            rec->time.hours,
            rec->time.minutes,
            rec->time.seconds,
            rec->temp,
            rec->light);
}

// periodic timer isr (5ms)
void t1_isr() {
    static uint8_t xmilis;
    static uint8_t seconds_monitor;
    xmilis++;
    if (xmilis >= 200) {
        if (!time_paused) {
            systime.seconds++;
            update_lcd = 1;
        }
        seconds_monitor++;
        xmilis = 0;
        D5_Toggle();
    }
    
    if (seconds_monitor > PMON) {
        seconds_monitor = 0;
        trigger_sensors = 1;
    }
    
    if (systime.seconds >= 60) {
        systime.seconds = 0;
        systime.minutes++;
        update_lcd = 1;
    }
    if (systime.minutes >= 60) {
        systime.minutes = 0;
        systime.hours = (systime.hours + 1) % 24;
        update_lcd = 1;
    }

    
    btn_update(&S1, SW1_GetValue());
    btn_update(&S2, SW2_GetValue());
}