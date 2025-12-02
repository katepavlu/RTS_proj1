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
Time altime;
uint8_t altemp;
uint8_t allight;
volatile uint8_t time_paused;

volatile uint8_t sensors_read;

volatile uint8_t temp_alarm_trigd;
volatile uint8_t time_alarm_trigd;
volatile uint8_t light_alarm_trigd;

volatile uint8_t update_lcd = 1;


#define PMON 5

void t1_isr() {
    static uint8_t xmilis;
    static uint8_t seconds_monitor;
    xmilis++;
    if (xmilis >= 100) {
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
        sensors_read = 1;
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

void main(void) {
    uint8_t temperature;
    uint8_t light_level = 2;
    char buf[17];

    // initialize the device
    SYSTEM_Initialize();

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
    temperature = readTC74();
    

    systime.hours = 0; systime.minutes = 0; systime.seconds = 0;
    altime.hours = 0; altime.minutes= 0; altime.seconds = 0;
    altemp = 0;
    allight = 0;
    sensors_read = 0;
    time_paused = 0;
    State state = {};
    uint8_t pos = 0;
    
    
    uint8_t alarms_enabled = 0;
    time_alarm_trigd = 0;
    temp_alarm_trigd = 0;
    light_alarm_trigd = 0;
    update_lcd = 1;

    D5_SetHigh();

    while (1) {

        // Periodic reading of temperature sensor
        if (sensors_read == 1) {
            temperature = readTC74();
            sensors_read = 0;
            update_lcd = 1;
            // max min stuff goes here later?
        }
        

        // Handle the buttons depending on the state of the system
        switch (state.sys) {
            case NORMAL:
                // if there are alarms, S1 clears them
                if(time_alarm_trigd || temp_alarm_trigd || light_alarm_trigd){
                   if ( scan_btn(&S1) ) {
                       time_alarm_trigd = 0;
                       temp_alarm_trigd = 0;
                       light_alarm_trigd = 0;
                   }
                }
                else {
                    state.sys = scan_btn(&S1)? CONFIG: state.sys;
                    // otherwise S1 advances to cfg state                    
                }
                // on sw2  enter records
                state.sys = scan_btn(&S2)? RECORDS1: state.sys;
                
                break;
                
            case CONFIG:
                switch (state.cfg) {
                    case TIME_SET:
                        time_paused = 1;
                        switch (state.tim) {
                            case SET_HOURS:
                                pos = 0x01;
                                state.tim = scan_btn(&S1)? SET_MINUTES: state.tim;
                                scan_btn(&S2);
                                break;
                            case SET_MINUTES:
                                pos = 0x04;
                                state.tim = scan_btn(&S1)? SET_SECONDS: state.tim;
                                scan_btn(&S2);
                                break;
                            case SET_SECONDS:
                                pos = 0x07;
                                if( scan_btn(&S1) ) {
                                    state.tim = SET_HOURS;
                                    state.cfg = TIME_ALARM;  
                                    time_paused = 0;
                                }
                                scan_btn(&S2);
                                break;
                        }
                        break;
                    case TIME_ALARM:
                        pos = 0x0a;
                        if(state.act == INACTIVE) {
                            state.cfg = scan_btn(&S1)? TEMP_ALARM: state.cfg;
                            state.act = scan_btn(&S2)? ACTIVE: state.act;
                        }
                        else {
                            switch (state.tim) {
                                case SET_HOURS:
                                    state.tim = scan_btn(&S1)? SET_MINUTES: state.tim;
                                    scan_btn(&S2);
                                    break;
                                case SET_MINUTES:
                                    state.tim = scan_btn(&S1)? SET_SECONDS: state.tim;
                                    scan_btn(&S2);
                                    break;
                                case SET_SECONDS:
                                    if( scan_btn(&S1) ) {
                                        state.tim = SET_HOURS;
                                        state.act = INACTIVE;                                            
                                    }
                                    scan_btn(&S2);
                                    break;
                            }
                        }
                        break;
                    case TEMP_ALARM:
                        pos = 0x0b;
                        if(state.act == INACTIVE) {
                            state.cfg = scan_btn(&S1)? LIGHT_ALARM: state.cfg;
                            state.act = scan_btn(&S2)? ACTIVE: state.act;
                        }
                        else {
                            state.act = scan_btn(&S1)? INACTIVE: state.act;
                            scan_btn(&S2);
                        }
                        break;
                    case LIGHT_ALARM:
                        pos = 0x0c;
                        if(state.act == INACTIVE) {
                            state.cfg = scan_btn(&S1)? ALARMS: state.cfg;
                            state.act = scan_btn(&S2)? ACTIVE: state.act;
                        }
                        else {
                            state.act = scan_btn(&S1)? INACTIVE: state.act;
                            scan_btn(&S2);
                        }
                        break;
                    case ALARMS:
                        pos = 0x0e;
                        state.cfg = scan_btn(&S1)? RESET: state.cfg;
                        scan_btn(&S2);
                        break;
                    case RESET:
                        pos = 0x0f;
                        if( scan_btn(&S1) ) {
                            state.cfg = TIME_SET;
                            state.sys = NORMAL;
                        }
                        scan_btn(&S2);
                        break;
                };
                break;
                
            case RECORDS1:
                scan_btn(&S1);
                state.sys = scan_btn(&S2)? RECORDS2: state.sys;
                break;
                
            case RECORDS2:
                scan_btn(&S1);
                state.sys = scan_btn(&S2)? NORMAL: state.sys;
                break;
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
                            (state.cfg!=TIME_ALARM)? systime.hours : altime.hours,
                            (state.cfg!=TIME_ALARM)? systime.minutes: altime.minutes,
                            (state.cfg!=TIME_ALARM)? systime.seconds: altime.seconds,
                            'C',
                            'T',
                            'L',
                            alarms_enabled?'A':' ',
                            'R');
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);       // second line, first column
                    sprintf(buf, "%02d C         L %d",altemp, allight);
                    while (LCDbusy());
                    LCDstr(buf);
                    while (LCDbusy());
                    LCDcmd(pos | 0x80);
                    /*
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    LCDstr("CONFIG MODE");
                    while (LCDbusy());
                    LCDcmd(0xc0);
                    sprintf(buf, "cfg %d tim %d ac %d", state.cfg, state.tim, state.act);
                    while (LCDbusy());
                    LCDstr(buf);*/
                    break;

                case RECORDS1:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    LCDstr("RECORD MODE 1");
                    break;

                case RECORDS2:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    LCDstr("RECORD MODE 2");
                    break;
            }

        }
        
    }
}
/**
 End of File
 */