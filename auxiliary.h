#ifndef __AUXILIARY_H
#define __AUXILIARY_H

#include "LCD/lcd.h"
#include "I2C/i2c.h"
#include <xc.h>

typedef enum {normal, config, records}SystemState;
typedef enum {time, time_alarm, temp_alarm, light_alarm, alarms, reset}ConfigSelection;
typedef enum {sel_hours, sel_minutes, sel_seconds}TimeSelection;
typedef enum {regular, activated}Activation;

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

#define MAXLUM        (EEPROM_START_ADDR + 11) //0X7011
#define MINLUM        (EEPROM_START_ADDR + 12) //0X7012
#define MAXTEMP        (EEPROM_START_ADDR + 13) //0X7013
#define MINTEMP       (EEPROM_START_ADDR + 14) //0X7014

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

void btn_update(Btn *btn, uint8_t pin);

#endif
