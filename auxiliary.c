#include "auxiliary.h"
#include "mcc_generated_files/mcc.h"

void write_eeprom(uint16_t address, uint8_t value){
    //DATAEE_WriteByte(ADDR_CLKM, value);
}

uint8_t read_eeprom(uint16_t address){
    //return DATAEE_ReadByte(address)
}

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

void set_parameters_default(){
    params.pmon = PMON;
    params.tala = TALA;
    params.tina = TINA;
    params.alaf = ALAF;
    params.alarm.hours =  ALAH;
    params.alarm.minutes =  ALAM;
    params.alarm.seconds =  ALAS;
    params.alat = ALAT;
    params.alal = ALAL;
    params.clkh = CLKH;
    params.clkm = CLKM;
}

void load_from_eeprom(){
    params.pmon = read_eeprom(ADDR_PMON);
    params.tala = read_eeprom(ADDR_TALA);
    params.tina = read_eeprom(ADDR_TINA);
    params.alaf = read_eeprom(ADDR_ALAF);
    params.alarm.hours = read_eeprom(ADDR_ALAH);
    params.alarm.minutes = read_eeprom(ADDR_ALAM);
    params.alarm.seconds = read_eeprom(ADDR_ALAS);
    params.alat = read_eeprom(ADDR_ALAT);
    params.alal = read_eeprom(ADDR_ALAL);
    params.clkh = read_eeprom(ADDR_CLKH);
    params.clkm = read_eeprom(ADDR_CLKM);
}

void save_to_eeprom(){
    set_eeprom(ADDR_PMON, params.pmon);
    set_eeprom(ADDR_TALA, params.tala);
    set_eeprom(ADDR_TINA, params.tina);
    set_eeprom(ADDR_ALAF, params.alaf);
    set_eeprom(ADDR_ALAH, params.alarm.hours);
    set_eeprom(ADDR_ALAM, params.alarm.minutes);
    set_eeprom(ADDR_ALAS, params.alarm.seconds);
    set_eeprom(ADDR_ALAT, params.alat);
    set_eeprom(ADDR_ALAL, params.alal);
    set_eeprom(ADDR_CLKH, params.clkh); 
    set_eeprom(ADDR_CLKM, params.clkm);
    
    uint8_t new_sum=calculate_checksum();
    set_eeprom(ADDR_CHECKSUM, new_sum);
}

uint8_t calculate_checksum(){
    uint8_t sum = 0;
    sum += params.pmon;
    sum += params.tala;
    sum += params.tina;
    sum += params.alaf;
    sum += params.alarm.hours;
    sum += params.alarm.minutes;
    sum += params.alarm.seconds;
    sum += params.alat;
    sum += params.alal;
    sum += params.clkh;
    sum += params.clkm;
    return sum;
}

void system_init_params(){
    uint8_t stored_sum, calc_sum;
    
    load_from_eeprom();
    
    stored_sum = read_eeprom(ADDR_CHECKSUM);
    
    calc_sum = calculate_checksum();
    
    if (stored_sum == calc_sum){
        // Again, might have complicated this part? we return the clock to the state it was when it was saved
        systime.hours = params.clkh;
        systime.minutes = params.clkm;
        systime.seconds = 0;
    }
    else{
        set_parameters_default();
        systime.hours = 0;
        systime.minutes = 0;
        systime.seconds = 0;
        save_to_eeprom();
    }
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
