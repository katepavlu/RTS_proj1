/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.6
        Device            :  PIC16F18875
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */

#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"
#include "LCD/lcd.h"
#include "stdio.h"
#include "auxiliary.h"

/*
                         Main application
 */

volatile Btn S1 = {0};
volatile Btn S2 = {0};

volatile Time systime;
uint8_t altemp;
uint8_t allight;
volatile uint8_t time_paused;

volatile uint8_t trigger_sensors;

volatile uint8_t temp_alarm_trigd;
volatile uint8_t time_alarm_trigd;
volatile uint8_t light_alarm_trigd;

volatile uint8_t update_lcd = 1;

Record rec_maxtemp;
Record rec_mintemp;
Record rec_maxlight;
Record rec_minlight;

volatile Parameters params;


// cursor positions
uint8_t pos[RESET + 1][SET_SECONDS + 1][ACTIVE + 1] ={
    [TIME_SET][SET_HOURS][INACTIVE]    = 0x01,
    [TIME_SET][SET_MINUTES][INACTIVE]  = 0x04,
    [TIME_SET][SET_SECONDS][INACTIVE]  = 0x07,
    
    [TIME_ALARM][SET_HOURS][INACTIVE]  = 0x0a,
    
    [TIME_ALARM][SET_HOURS][ACTIVE]    = 0x01,
    [TIME_ALARM][SET_MINUTES][ACTIVE]  = 0x04,
    [TIME_ALARM][SET_SECONDS][ACTIVE]  = 0x07,
    
    [TEMP_ALARM][SET_HOURS][INACTIVE]  = 0x0b,
    [TEMP_ALARM][SET_HOURS][ACTIVE]    = 0x41,
    
    [LIGHT_ALARM][SET_HOURS][INACTIVE] = 0x0c,
    [LIGHT_ALARM][SET_HOURS][ACTIVE]   = 0x4f,
    
    [ALARMS][SET_HOURS][INACTIVE]      = 0x0e,
    [RESET][SET_HOURS][INACTIVE]       = 0x0f    
};

void main(void) {
    uint8_t temperature;
    uint8_t light_level;
    char buf[17];
    update_record(&rec_maxtemp, systime, 0, 0);
    update_record(&rec_mintemp, systime, 50, 0);
    update_record(&rec_maxlight, systime, 0, 0);
    update_record(&rec_minlight, systime, 0, 3);

    // initialize the device
    SYSTEM_Initialize();
    system_init_params();

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();
    TMR1_SetInterruptHandler(t1_isr);
    // TMR2_SetInterruptHandler(t2_isr); maybe later on we have a 2nd timer for dealing with buttons?

    OpenI2C();
    //I2C_SCL = 1;
    //I2C_SDA = 1;
    //WPUC3 = 1;
    //WPUC4 = 1;
    LCDinit();    
    // this now should be done in system_init_parms(), line 111
    //systime.hours = 0; systime.minutes = 0; systime.seconds = 0;
    altemp = 0;
    allight = 0;
    trigger_sensors = 1;
    time_paused = 0;
    State state = {};
    
    uint8_t alarms_enabled = 0;
    time_alarm_trigd = 0;
    temp_alarm_trigd = 0;
    light_alarm_trigd = 0;
    update_lcd = 1;

    D5_SetHigh();

    while (1) {

        // Periodic reading of temperature sensor
        if (trigger_sensors == 1) {
            temperature = readTC74();
            
            ADCC_StartConversion(ANALOG_IN);
            while(!ADCC_IsConversionDone());
            light_level = (uint8_t)((3 * ADCC_GetSingleConversion(ANALOG_IN))/1023 );
            
            if (temperature >= rec_maxtemp.temp) 
                update_record(&rec_maxtemp, systime, temperature, light_level);
            if (temperature <= rec_mintemp.temp)
                update_record(&rec_mintemp, systime, temperature, light_level);
            if (light_level >= rec_maxlight.light )
                update_record(&rec_maxlight, systime, temperature, light_level);
            if (light_level <= rec_minlight.light)
                update_record(&rec_minlight, systime, temperature, light_level);
            
            trigger_sensors = 0;
            update_lcd = 1;
            // max min stuff goes here later?
        }
        
        if(update_lcd) {
            update_lcd = 0;
                // Print the screen depending on the state of the system
            switch (state.sys) {
                case NORMAL:

                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    sprintf(buf,"%02d:%02d:%02d  %c%c%c %c%c",
                            systime.hours, systime.minutes, systime.seconds,
                            temp_alarm_trigd?'C':' ',
                            time_alarm_trigd?'T':' ',
                            light_alarm_trigd?'L':' ',
                            alarms_enabled?'A':' ',
                            ' ');
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);       // second line, first column
                    sprintf(buf, "%02d C         L %d",temperature, light_level);
                    while (LCDbusy());
                    LCDstr(buf);

                    break;

                case CONFIG:
                    
                    
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    sprintf(buf,"%02d:%02d:%02d  %c%c%c %c%c",
                            (state.cfg!=TIME_ALARM)? systime.hours : params.alarm.hours,
                            (state.cfg!=TIME_ALARM)? systime.minutes: params.alarm.minutes,
                            (state.cfg!=TIME_ALARM)? systime.seconds: params.alarm.seconds,
                            'C',
                            'T',
                            'L',
                            alarms_enabled?'A':'a',
                            'R');
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);       // second line, first column
                    sprintf(buf, "%02d C         L %d",altemp, allight);
                    while (LCDbusy());
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(pos[state.cfg][state.tim][state.act] | 0x80); // sets the cursor over editable field
                    break;

                case RECORDS1:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    print_record(buf, &rec_maxtemp);
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);
                    while (LCDbusy());
                    print_record(buf, &rec_mintemp);
                    LCDstr(buf);
                    break;

                case RECORDS2:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    print_record(buf, &rec_maxlight);
                    LCDstr(buf);
                    LCDcmd(0xc0);
                    while (LCDbusy());
                    print_record(buf, &rec_minlight);
                    LCDstr(buf);
                    break;
            }

        }
        
        
        if(S1.event | S2.event) {
            // Handle the buttons depending on the state of the system
            switch (state.sys) {
                case NORMAL:
                    // if there are alarms, S1 clears them
                    if(time_alarm_trigd || temp_alarm_trigd || light_alarm_trigd){
                       if ( handle_btn(&S1) ) {
                           time_alarm_trigd = 0;
                           temp_alarm_trigd = 0;
                           light_alarm_trigd = 0;
                       }
                    }
                    else {
                        state.sys = handle_btn(&S1)? CONFIG: state.sys;
                        // otherwise S1 advances to cfg state                    
                    }
                    // on sw2  enter records
                    state.sys = handle_btn(&S2)? RECORDS1: state.sys;

                    break;

                case CONFIG:
                    switch (state.cfg) {
                        case TIME_SET:
                            time_paused = 1;
                            switch (state.tim) {
                                case SET_HOURS:
                                    //pos = ;// cursor position
                                    state.tim = handle_btn(&S1)? SET_MINUTES: state.tim;
                                    systime.hours = (systime.hours + handle_btn(&S2)) % 24;
                                    break;
                                case SET_MINUTES:
                                    //pos = 0x04;
                                    state.tim = handle_btn(&S1)? SET_SECONDS: state.tim;
                                    systime.minutes = (systime.minutes + handle_btn(&S2)) % 60;
                                    break;
                                case SET_SECONDS:
                                    //pos = 0x07;
                                    if( handle_btn(&S1) ) {
                                        state.tim = SET_HOURS;
                                        state.cfg = TIME_ALARM;  
                                        time_paused = 0;
                                    }
                                    systime.seconds = (systime.seconds + handle_btn(&S2)) % 60;
                                    break;
                            }
                            break;
                        case TIME_ALARM:
                            if(state.act == INACTIVE) {
                                //pos = 0x0a;
                                state.cfg = handle_btn(&S1)? TEMP_ALARM: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                switch (state.tim) {
                                    case SET_HOURS:
                                        //pos = 0x01;
                                        state.tim = handle_btn(&S1)? SET_MINUTES: state.tim;
                                        params.alarm.hours = (params.alarm.hours + handle_btn(&S2)) % 24;
                                        break;
                                    case SET_MINUTES:
                                        //pos = 0x04;
                                        state.tim = handle_btn(&S1)? SET_SECONDS: state.tim;
                                        params.alarm.minutes = (params.alarm.minutes + handle_btn(&S2)) % 60;
                                        break;
                                    case SET_SECONDS:
                                        //pos = 0x07;
                                        if( handle_btn(&S1) ) {
                                            state.tim = SET_HOURS;
                                            state.act = INACTIVE;                                            
                                        }
                                        params.alarm.seconds = (params.alarm.seconds + handle_btn(&S2)) % 60;
                                        break;
                                }
                            }
                            break;
                        case TEMP_ALARM:
                            if(state.act == INACTIVE) {
                                //pos = 0x0b;
                                state.cfg = handle_btn(&S1)? LIGHT_ALARM: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                //pos = 0x41;
                                state.act = handle_btn(&S1)? INACTIVE: state.act;
                                altemp = (altemp + handle_btn(&S2)) % 51;
                            }
                            break;
                        case LIGHT_ALARM:
                            if(state.act == INACTIVE) {
                                //pos = 0x0c;
                                state.cfg = handle_btn(&S1)? ALARMS: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                //pos = 0x4f;
                                state.act = handle_btn(&S1)? INACTIVE: state.act;
                                allight = (allight + handle_btn(&S2)) % 4;
                            }
                            break;
                        case ALARMS:
                            //pos = 0x0e;
                            state.cfg = handle_btn(&S1)? RESET: state.cfg;
                            if(handle_btn(&S2))
                                alarms_enabled = !alarms_enabled;
                            break;
                        case RESET:
                            //pos = 0x0f;
                            if( handle_btn(&S1) ) {
                                state.cfg = TIME_SET;
                                state.sys = NORMAL;
                            }
                            if(handle_btn(&S2)) {
                                update_record(&rec_maxtemp, systime, 0, 0);
                                update_record(&rec_mintemp, systime, 50, 0);
                                update_record(&rec_maxlight, systime, 0, 0);
                                update_record(&rec_minlight, systime, 0, 3);
                            }

                            break;
                    };
                    break;

                case RECORDS1:
                    handle_btn(&S1); // we do this just to clear S1 state
                    state.sys = handle_btn(&S2)? RECORDS2: state.sys;
                    break;

                case RECORDS2:
                    handle_btn(&S1); // we do this just to clear S1 state
                    state.sys = handle_btn(&S2)? NORMAL: state.sys;
                    break;
            }
            
        }
        
        asm("SLEEP");
        
    }
}
/**
 End of File
 */