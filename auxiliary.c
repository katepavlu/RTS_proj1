#include "auxiliary.h"
#include "mcc_generated_files/mcc.h"

void set_default_params(Params* params){
    params->pmon = PMON;
    params->tala = TALA;
    params->tina = TINA;
    params->alaf = ALAF;
    
    params->altime.hours =  ALAH;
    params->altime.minutes =  ALAM;
    params->altime.seconds =  ALAS;
    
    params->alat = ALAT;
    params->alal = ALAL;
    
    params->systime.hours = CLKH;
    params->systime.minutes = CLKM;
    params->systime.seconds = 0;
    
    params->maxtemp.time.hours = 0;
    params->maxtemp.time.minutes = 0;
    params->maxtemp.time.seconds = 0;
    params->maxtemp.temp = 0;
    params->maxtemp.light = 0;
    
    params->mintemp.time.hours = 0;
    params->mintemp.time.minutes = 0;
    params->mintemp.time.seconds = 0;
    params->mintemp.temp = 50; // this means that the minima will be update immediately
    params->mintemp.light = 0;
    
    params->maxlight.time.hours = 0;
    params->maxlight.time.minutes = 0;
    params->maxlight.time.seconds = 0;
    params->maxlight.temp = 0;
    params->maxlight.light = 0;
    
    params->minlight.time.hours = 0;
    params->minlight.time.minutes = 0;
    params->minlight.time.seconds = 0;
    params->minlight.temp = 0;
    params->minlight.light = 3;    
}

void read_from_eeprom(Params* params){
    uint8_t stored_sum, calc_sum;
    uint8_t *p = (uint8_t*) params;
    // read the parameter struct as a byte array
    
    // write the bytes
    // we do not need to care about padding or endianness as long as the read and write
    // are done in the same order
    for(size_t i = 0; i < sizeof(Params); i++)
        *(p + i) = DATAEE_ReadByte(EEPROM_START_ADDR + i);
    
    stored_sum = DATAEE_ReadByte(ADDR_CHECKSUM);
    calc_sum = calculate_checksum(params);
    
    if (stored_sum != calc_sum){
        set_default_params(params);
        save_to_eeprom(params);
    }

}

void copy_params(Params* source, Params* target) {
    uint8_t *ps = (uint8_t*) source;
    uint8_t *pt = (uint8_t*) target;
    
    for(size_t i = 0; i < sizeof(Params); i++)
        *(pt + i) = *(ps + i);
}

// compare two parameter structs, BUT skip the current seconds
// returns 0 if they are the same
uint8_t compare_params(Params* eeprom_params, Params* current_params) {
    return 
            eeprom_params->pmon != current_params->pmon ||
            eeprom_params->tala != current_params->tala ||
            eeprom_params->tina != current_params->tina ||
            eeprom_params->alaf != current_params->alaf ||
            
            eeprom_params->altime.hours != current_params->altime.hours ||
            eeprom_params->altime.minutes != current_params->altime.minutes ||
            eeprom_params->altime.seconds != current_params->altime.seconds ||
            
            eeprom_params->alat != current_params->alat ||
            eeprom_params->alal != current_params->alal ||
            
            eeprom_params->systime.hours != current_params->systime.hours ||
            eeprom_params->systime.minutes != current_params->systime.minutes ||
          //eeprom_params->altime.seconds != current_params->altime.seconds ||
            
            eeprom_params->maxtemp.time.hours != current_params->maxtemp.time.hours ||
            eeprom_params->maxtemp.time.minutes != current_params->maxtemp.time.minutes ||
            eeprom_params->maxtemp.time.seconds != current_params->maxtemp.time.seconds ||
            eeprom_params->maxtemp.temp != current_params->maxtemp.temp ||
            eeprom_params->maxtemp.light != current_params->maxtemp.light ||
            
            eeprom_params->mintemp.time.hours != current_params->mintemp.time.hours ||
            eeprom_params->mintemp.time.minutes != current_params->mintemp.time.minutes ||
            eeprom_params->mintemp.time.seconds != current_params->mintemp.time.seconds ||
            eeprom_params->mintemp.temp != current_params->mintemp.temp ||
            eeprom_params->mintemp.light != current_params->mintemp.light ||
            
            eeprom_params->maxlight.time.hours != current_params->maxlight.time.hours ||
            eeprom_params->maxlight.time.minutes != current_params->maxlight.time.minutes ||
            eeprom_params->maxlight.time.seconds != current_params->maxlight.time.seconds ||
            eeprom_params->maxlight.temp != current_params->maxlight.temp ||
            eeprom_params->maxlight.light != current_params->maxlight.light ||
            
            eeprom_params->minlight.time.hours != current_params->minlight.time.hours ||
            eeprom_params->minlight.time.minutes != current_params->minlight.time.minutes ||
            eeprom_params->minlight.time.seconds != current_params->minlight.time.seconds ||
            eeprom_params->minlight.temp != current_params->minlight.temp ||
            eeprom_params->minlight.light != current_params->minlight.light
            ;
}

// the checksum is just the sum of the bytes in the struct
uint8_t calculate_checksum(Params* params){
    uint8_t *p = (uint8_t*) params;
    uint8_t sum = 0;
    for(size_t i = 0; i < sizeof(Params); i++)
        sum += *(p + i);
    return sum;
}

void save_to_eeprom(Params* params){
    uint8_t *p = (uint8_t*) params;

    for(size_t i = 0; i < sizeof(Params); i++)
        DATAEE_WriteByte(EEPROM_START_ADDR + i, *(p + i));

    uint8_t new_sum=calculate_checksum(params);
    DATAEE_WriteByte(ADDR_CHECKSUM, new_sum);
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
    btn->hist = ( btn->hist << 1 ) | pin;
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

void update_record(Record* rec, Time tim, uint8_t temp, uint8_t light) {
    rec->time.hours = tim.hours;
    rec->time.minutes = tim.minutes;
    rec->time.seconds = tim.seconds;
    rec->temp = temp;
    rec->light = light;
}

// periodic timer isr (5ms)
void t1_isr() {
    static uint8_t xmilis;
    static uint8_t seconds_monitor;
    xmilis++;
    if (xmilis > 200) {
        if (!time_paused) {
            current_params.systime.seconds++;
            update_lcd = 1;
        }
        seconds_monitor++;
        xmilis = 0;
        D5_Toggle();
    }
    
    if (seconds_monitor > EEPROM_params.pmon) {
        seconds_monitor = 0;
        trigger_sensors = 1;
    }
    
    if (current_params.systime.seconds >= 60) {
        current_params.systime.seconds = 0;
        current_params.systime.minutes++;
        update_lcd = 1;
    }
    if (current_params.systime.minutes >= 60) {
        current_params.systime.minutes = 0;
        current_params.systime.hours = (current_params.systime.hours + 1) % 24;
        update_lcd = 1;
    }
    
    if (current_params.alaf &&
            current_params.systime.hours == current_params.altime.hours &&
            current_params.systime.minutes == current_params.altime.minutes &&
            current_params.systime.seconds == current_params.altime.seconds) 
        time_alarm_trigd = 1;
    
    btn_update(&S1, SW1_GetValue());
    btn_update(&S2, SW2_GetValue());
}
