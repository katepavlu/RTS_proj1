#ifndef __AUXILIARY_H
#define __AUXILIARY_H

#include "LCD/lcd.h"
#include "I2C/i2c.h"
#include <xc.h>

typedef enum {NORMAL, CONFIG, RECORDS1, RECORDS2}SystemState;
typedef enum {TIME_SET, TIME_ALARM, TEMP_ALARM, LIGHT_ALARM, ALARMS, RESET}ConfigSelection;
typedef enum {SET_HOURS, SET_MINUTES, SET_SECONDS}TimeSelection;
typedef enum {INACTIVE, ACTIVE}Activation;

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
}Time;

typedef struct {
    SystemState sys;
    ConfigSelection cfg;
    TimeSelection tim;
    Activation act;
}State;

typedef struct {
    Time time;
    uint8_t temp;
    uint8_t light;
}Record;


// cached values of eeprom parameters
typedef struct {
    uint8_t pmon;
    uint8_t tala;
    uint8_t tina;
    uint8_t alaf;
    Time altime;
    uint8_t alat;
    uint8_t alal;
    Time systime;
    Record maxtemp;
    Record mintemp;
    Record maxlight;
    Record minlight;
} Params;

typedef struct {
    uint8_t unhandled;
    uint8_t triggered;
} Alarm;

// EEPROM Memory addresses
#define EEPROM_START_ADDR 0x7000

#define ADDR_CHECKSUM   (EEPROM_START_ADDR + sizeof(Params)) // 0x7031

// Default Values

#define PMON 5 // monitoring period
#define TALA 3 // alarm duration
#define TINA 10// inactivity timeout
#define ALAF 0 // alarm flag
#define ALAH 12// alarm clock hours
#define ALAM 0 // alarm clock minutes
#define ALAS 0 // alarm clock seconds
#define ALAT 20// temperature threshold
#define ALAL 2 // light threshold
#define CLKH 0 // clock hours
#define CLKM 0 // clock minutes

void set_default_params(Params* params);
void read_from_eeprom(Params* params);
uint8_t calculate_checksum(Params* params);
void save_to_eeprom(Params* params);
uint8_t compare_params(Params* eeprom_params, Params* current_params);
void copy_params(Params* source, Params* target);

unsigned char readTC74 (void);

extern volatile Params EEPROM_params;

// Button settings and structure
#define BTN_PRESSED 0
#define BTN_RELEASED 1
typedef struct{
    uint8_t hist;
    uint8_t event;
}Btn;

void btn_update(volatile Btn *btn, uint8_t pin);
uint8_t handle_btn(volatile Btn *btn);

void print_record(char* buf, Record* rec);
void update_record(Record* rec, Time tim, uint8_t temp, uint8_t light);

void t1_isr();

extern volatile uint8_t update_lcd;
extern volatile uint8_t time_paused;
extern volatile uint8_t trigger_sensors;
extern volatile Params current_params;
extern volatile Btn S1, S2;

extern volatile Alarm light_alarm;
extern volatile Alarm time_alarm;
extern volatile Alarm temp_alarm;

uint8_t alarm_handler(Alarm* alarm);

#endif
