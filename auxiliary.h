#ifndef __AUXILIARY_H
#define __AUXILIARY_H

#include "LCD/lcd.h"
#include "I2C/i2c.h"
#include <xc.h>

typedef enum {normal, config, records}SystemState;
typedef enum {time, time_alarm, temp_alarm, light_alarm, alarms, reset}ConfigSelection;
typedef enum {sel_hours, sel_minutes, sel_seconds}TimeSelection;
typedef enum {regular, activated}Activation;

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
