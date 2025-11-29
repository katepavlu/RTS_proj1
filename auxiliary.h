#ifndef __AUXILIARY_H
#define __AUXILIARY_H

#include "LCD/lcd.h"
#include "I2C/i2c.h"
#include <xc.h>

typedef enum {normal, config, records}SystemState;

unsigned char readTC74 (void);



// Button settings and structure
#define BTN_PRESSED 0
#define BTN_RELEASED 1
#define BTN_DEBOUNCE 60 // maybe increase this value if the debouncer is still very jittery? 
typedef struct{
    uint8_t timer;
    uint8_t event;
    uint8_t processed;
}Btn;

void btn_update(Btn *btn, uint8_t pin);

#endif
