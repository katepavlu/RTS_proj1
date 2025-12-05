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

volatile Params EEPROM_params;
volatile Params current_params;

volatile uint8_t time_paused;
volatile uint8_t trigger_sensors;
volatile uint8_t update_lcd = 1;

volatile uint8_t light_alarm_trigd;
volatile uint8_t time_alarm_trigd;
volatile uint8_t temp_alarm_trigd;

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
    char display_buf[17];
    uint8_t timestamp;
    

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
    LCDinit();    

    trigger_sensors = 1;
    time_paused = 0;
    State state = {};
    read_from_eeprom((Params*)&EEPROM_params);
    read_from_eeprom((Params*)&current_params);
    
    time_alarm_trigd = 0;
    temp_alarm_trigd = 0;
    light_alarm_trigd = 0;
    update_lcd = 1;

    D5_SetHigh();
    D2_SetLow();
    D3_SetLow();

    while (1) {

        // Periodic reading of temperature sensor
        if (trigger_sensors == 1) {
            temperature = readTC74();
            
            ADCC_StartConversion(ANALOG_IN);
            while(!ADCC_IsConversionDone());
            light_level = (uint8_t)((3 * ADCC_GetSingleConversion(ANALOG_IN))/1023 );
            
            if (temperature > current_params.maxtemp.temp) 
                update_record((Record*)&current_params.maxtemp, current_params.systime, temperature, light_level);
            if (temperature < current_params.mintemp.temp)
                update_record((Record*)&current_params.mintemp, current_params.systime, temperature, light_level);
            if (light_level > current_params.maxlight.light )
                update_record((Record*)&current_params.maxlight, current_params.systime, temperature, light_level);
            if (light_level < current_params.minlight.light)
                update_record((Record*)&current_params.minlight, current_params.systime, temperature, light_level);
            
            if (light_level < current_params.alal) {
                D2_SetHigh();
                if (current_params.alaf) {
                    light_alarm_trigd = 1;
                }
            }
            else {
                D2_SetLow();
            }
            
            if (temperature > current_params.alat) {
                D3_SetHigh();
                if (current_params.alaf) {
                    temp_alarm_trigd = 1;
                }
            }
            else {
                D3_SetLow();
            }
            
            trigger_sensors = 0;
            update_lcd = 1;
        }
        
        if(update_lcd) {
            update_lcd = 0;
                // Print the screen depending on the state of the system
            switch (state.sys) {
                case NORMAL:

                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    sprintf(display_buf,"%02d:%02d:%02d  %c%c%c %c%c",
                            current_params.systime.hours, current_params.systime.minutes, current_params.systime.seconds,
                            temp_alarm_trigd?'C':' ',
                            time_alarm_trigd?'T':' ',
                            light_alarm_trigd?'L':' ',
                            current_params.alaf?'A':' ',
                            ' ');
                    LCDstr(display_buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);       // second line, first column
                    sprintf(display_buf, "%02d C         L %d",temperature, light_level);
                    while (LCDbusy());
                    LCDstr(display_buf);

                    break;

                case CONFIG:
                    

                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    sprintf(display_buf,"%02d:%02d:%02d  %c%c%c %c%c",
                            (state.cfg!=TIME_ALARM)? current_params.systime.hours : current_params.altime.hours,
                            (state.cfg!=TIME_ALARM)? current_params.systime.minutes: current_params.altime.minutes,
                            (state.cfg!=TIME_ALARM)? current_params.systime.seconds: current_params.altime.seconds,
                            'C',
                            'T',
                            'L',
                            current_params.alaf?'A':'a',
                            'R');
                    LCDstr(display_buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);       // second line, first column
                    sprintf(display_buf, "%02d C         L %d",current_params.alat, current_params.alal);
                    while (LCDbusy());
                    LCDstr(display_buf);
                    while (LCDbusy());
                    LCDcmd(pos[state.cfg][state.tim][state.act] | 0x80); // sets the cursor over editable field
                    break;

                case RECORDS1:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    print_record(display_buf, (Record*)&current_params.maxtemp);
                    LCDstr(display_buf);
                    while (LCDbusy());
                    LCDcmd(0xc0);
                    while (LCDbusy());
                    print_record(display_buf, (Record*)&current_params.mintemp);
                    LCDstr(display_buf);
                    break;

                case RECORDS2:
                    LCDcmd(0x80);       //first line, first column
                    while (LCDbusy());
                    print_record(display_buf, (Record*)&current_params.maxlight);
                    LCDstr(display_buf);
                    LCDcmd(0xc0);
                    while (LCDbusy());
                    print_record(display_buf, (Record*)&current_params.minlight);
                    LCDstr(display_buf);
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
                    if(handle_btn(&S2)){
                        state.sys = RECORDS1;
                        timestamp = current_params.systime.seconds;
                    }

                    break;

                case CONFIG:
                    switch (state.cfg) {
                        case TIME_SET:
                            time_paused = 1;
                            switch (state.tim) {
                                case SET_HOURS:
                                    state.tim = handle_btn(&S1)? SET_MINUTES: state.tim;
                                    current_params.systime.hours = (current_params.systime.hours + handle_btn(&S2)) % 24;
                                    break;
                                case SET_MINUTES:
                                    state.tim = handle_btn(&S1)? SET_SECONDS: state.tim;
                                    current_params.systime.minutes = (current_params.systime.minutes + handle_btn(&S2)) % 60;
                                    break;
                                case SET_SECONDS:
                                    if( handle_btn(&S1) ) {
                                        state.tim = SET_HOURS;
                                        state.cfg = TIME_ALARM;  
                                        time_paused = 0;
                                    }
                                    current_params.systime.seconds = (current_params.systime.seconds + handle_btn(&S2)) % 60;
                                    break;
                            }
                            break;
                        case TIME_ALARM:
                            if(state.act == INACTIVE) {
                                state.cfg = handle_btn(&S1)? TEMP_ALARM: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                switch (state.tim) {
                                    case SET_HOURS:
                                        state.tim = handle_btn(&S1)? SET_MINUTES: state.tim;
                                        current_params.altime.hours = (current_params.altime.hours + handle_btn(&S2)) % 24;
                                        break;
                                    case SET_MINUTES:
                                        state.tim = handle_btn(&S1)? SET_SECONDS: state.tim;
                                        current_params.altime.minutes = (current_params.altime.minutes + handle_btn(&S2)) % 60;
                                        break;
                                    case SET_SECONDS:
                                        if( handle_btn(&S1) ) {
                                            state.tim = SET_HOURS;
                                            state.act = INACTIVE;                                            
                                        }
                                        current_params.altime.seconds = (current_params.altime.seconds + handle_btn(&S2)) % 60;
                                        break;
                                }
                            }
                            break;
                        case TEMP_ALARM:
                            if(state.act == INACTIVE) {
                                state.cfg = handle_btn(&S1)? LIGHT_ALARM: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                state.act = handle_btn(&S1)? INACTIVE: state.act;
                                current_params.alat = (current_params.alat + handle_btn(&S2)) % 51;
                            }
                            break;
                        case LIGHT_ALARM:
                            if(state.act == INACTIVE) {
                                state.cfg = handle_btn(&S1)? ALARMS: state.cfg;
                                state.act = handle_btn(&S2)? ACTIVE: state.act;
                            }
                            else {
                                state.act = handle_btn(&S1)? INACTIVE: state.act;
                                current_params.alal = (current_params.alal + handle_btn(&S2)) % 4;
                            }
                            break;
                        case ALARMS:
                            state.cfg = handle_btn(&S1)? RESET: state.cfg;
                            if(handle_btn(&S2))
                                current_params.alaf = !current_params.alaf;
                            break;
                        case RESET:
                            if( handle_btn(&S1) ) {
                                state.cfg = TIME_SET;
                                state.sys = NORMAL;
                            }
                            if(handle_btn(&S2)) {
                                update_record((Record*)&current_params.maxtemp, current_params.systime, 0, 0);
                                update_record((Record*)&current_params.mintemp, current_params.systime, 50, 0);
                                update_record((Record*)&current_params.maxlight, current_params.systime, 0, 0);
                                update_record((Record*)&current_params.minlight, current_params.systime, 0, 3);
                            }

                            break;
                    };
                    break;

                case RECORDS1:
                    handle_btn(&S1); // we do this just to clear S1 state
                    if(handle_btn(&S2)){
                        state.sys = RECORDS2;
                        timestamp = current_params.systime.seconds;
                    }
                    break;

                case RECORDS2:
                    handle_btn(&S1); // we do this just to clear S1 state
                    state.sys = handle_btn(&S2)? NORMAL: state.sys;
                    break;
            }
            
        }
        
        // if we are on one record or 8s, return to normal state
        if ( ((timestamp + current_params.tina)%60 ) == current_params.systime.seconds )
            state.sys = NORMAL;
        
        if( compare_params((Params*)&EEPROM_params, (Params*)&current_params) ) {
            copy_params((Params*)&current_params, (Params*)&EEPROM_params);
            EEPROM_params.systime.seconds = 0;
            save_to_eeprom((Params*)&EEPROM_params);
        }
        
        asm("SLEEP"); // enter low power mode until next timer firing
        
    }
}
/**
 End of File
 */