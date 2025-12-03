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
#define MINLUM  0x00
#define MAXLUM  0x05
#define MINTEMP 0x0A
#define MAXTEMP 0x14
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
void t2_isr()
{
    TMR2_StopTimer();
    PWM_Output_D4_Disable();
}


void main(void) {
    uint8_t temperature;
    uint8_t light_level = 2;
    char buf[17];

    int minLum = 1023;
    int maxLum = 0;
    int minTemp = 255;
    int maxTemp = 0;
    int lumlvl;
    int lumAlarm = 0;
    uint8_t tstmp[3];
    uint8_t newLum;
    adc_result_t lum;
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
    TMR2_SetInterruptHandler(t2_isr); //maybe later on we have a 2nd timer for dealing with buttons?
    ADCC_Initialize();
    ADCC_DisableContinuousConversion();
    
    
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
            lum = ADCC_GetSingleConversion(POT);

            sensors_read = 0;
            
            if(lum < 255){
                lumlvl = 0;      
                PWM_Output_D4_Enable();
                TMR2_StartTimer();
                PWM6_LoadDutyValue(1);
                D2_SetHigh();
            }
            else if (lum > 255 && lum < 510){
                lumlvl = 1;
                PWM_Output_D4_Enable();
                TMR2_StartTimer();
                PWM6_LoadDutyValue(1);
                D2_SetHigh();
            }
            else if (lum > 510 && lum < 765){
                lumlvl = 2;
                D2_SetLow();
            }
            else{
                lumlvl = 3;
                D2_SetLow();
            }
            
            if(temperature > 20){
                PWM_Output_D4_Enable();
                TMR2_StartTimer();
                PWM6_LoadDutyValue(1);
                D3_SetHigh();
            } else {
                D3_SetLow();
            }

            newLum = (lum * 255) / 1023;

            if(lum < minLum){
                minLum = lum;
                DATAEE_WriteByte(MINLUM,   hours);
                DATAEE_WriteByte(MINLUM+1, minutes);
                DATAEE_WriteByte(MINLUM+2, seconds);
                DATAEE_WriteByte(MINLUM+3, newLum);
                DATAEE_WriteByte(MINLUM+4, temperature);
            }
            if(lum > maxLum){
                maxLum = lum;
                DATAEE_WriteByte(MAXLUM,   hours);
                DATAEE_WriteByte(MAXLUM+1, minutes);
                DATAEE_WriteByte(MAXLUM+2, seconds);
                DATAEE_WriteByte(MAXLUM+3, newLum);
                DATAEE_WriteByte(MAXLUM+4, temperature);
            }

            if(temperature < minTemp){
                minTemp = temp;
                DATAEE_WriteByte(MINTEMP,   hours);
                DATAEE_WriteByte(MINTEMP+1, minutes);
                DATAEE_WriteByte(MINTEMP+2, seconds);
                DATAEE_WriteByte(MINTEMP+3, temperature);
            }
            if(temperature > maxTemp){
                maxTemp = temp;
                DATAEE_WriteByte(MAXTEMP,   hours);
                DATAEE_WriteByte(MAXTEMP+1, minutes);
                DATAEE_WriteByte(MAXTEMP+2, seconds);
                DATAEE_WriteByte(MAXTEMP+3, temperature);
            }
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
