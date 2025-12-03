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

// Memory addresses
#define EEPROM_START_ADDR 0x7000

#define ADDR_PMON       (EEPROM_START_ADDR + 0) // 0x7000
#define ADDR_TALA       (EEPROM_START_ADDR + 1) // 0x7001
#define ADDR_TINA       (EEPROM_START_ADDR + 2) // 0x7002
#define ADDR_ALAF       (EEPROM_START_ADDR + 3) // 0x7003
#define ADDR_ALAH       (EEPROM_START_ADDR + 4) // 0x7004
#define ADDR_ALAM       (EEPROM_START_ADDR + 5) // 0x7005
#define ADDR_ALAS       (EEPROM_START_ADDR + 6) // 0x7006
#define ADDR_ALAT       (EEPROM_START_ADDR + 7) // 0x7007
#define ADDR_ALAL       (EEPROM_START_ADDR + 8) // 0x7008
#define ADDR_CLKH       (EEPROM_START_ADDR + 9) // 0x7009
#define ADDR_CLKM       (EEPROM_START_ADDR + 10) // 0x7010

#define PMON 5

void set_eeprom_default();

void set_eeprom(uint16_t address, uint8_t value);
uint8_t read_eeprom(uint16_t address);

unsigned char readTC74 (void);

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

void t1_isr();

extern volatile uint8_t update_lcd;
extern volatile uint8_t time_paused;
extern volatile uint8_t trigger_sensors;
extern volatile Time systime;
extern volatile Btn S1, S2;

#endif
