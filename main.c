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

volatile uint8_t seconds;
volatile uint8_t minutes;
volatile uint8_t hours;

volatile uint8_t sensors_read;

volatile uint8_t temp_alarm_trigd;
volatile uint8_t time_alarm_trigd;
volatile uint8_t light_alarm_trigd;


#define PMON 5

void t1_isr() {
    static uint8_t xmilis;
    static uint8_t seconds_monitor;
    xmilis++;
    if (xmilis >= 100) {
        seconds++;
        seconds_monitor++;
        xmilis = 0;
        D5_Toggle();
    }
    
    if (seconds_monitor > PMON) {
        sensors_read = 1;
    }
    
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
    }
    if (minutes >= 60) {
        minutes = 0;
        hours = (hours + 1) % 24;
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
    

    seconds = 0;
    minutes = 0;
    hours = 0;
    sensors_read = 0;
    SystemState state = normal;
    
    
    uint8_t alarms_enabled = 0;

    D5_SetHigh();

    while (1) {

        // Periodic reading of temperature sensor
        if (sensors_read == 1) {
            temperature = readTC74();
            sensors_read = 0;
            // max min stuff goes here later?
        }
        

        // Handle the buttons depending on the state of the system
        if (S1.event) {
            S1.event = 0;
            switch (state) {
                case normal:
                    state = config;
                    break;
                case config:
                    state = normal;
                    break;
                case records:
                    break;
            }
        }


        // Print the screen depending on the state of the system
        switch (state) {
            case normal:
                
                LCDcmd(0x80);       //first line, first column
                while (LCDbusy());
                sprintf(buf,"%02d:%02d:%02d  %c%c%c %c%c",
                        hours, minutes, seconds,
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
                
            case config:
                LCDcmd(0x80);       //first line, first column
                while (LCDbusy());
                LCDstr("CONFIG MODE");
                break;
            case records:
                break;
        }
        
    }
}
/**
 End of File
 */