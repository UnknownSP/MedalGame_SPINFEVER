/*
 * File:   main.h
 * Author: Izutsu
 *
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>

#define _XTAL_FREQ 32000000

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = ON       // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#include <pic16f1938.h>
#include "I2C.h"

typedef enum{
  STANDBY = 0,
  IN_GAME = 1,
} Game_mode;

typedef enum{
  LED = 0,
  OUT = 1,
} Output_t;

typedef enum{
  S_1 = 0,
  S_2 = 1,
} Input_t;

void output(Output_t type, int num, int output);
int input(Input_t type, int num);

#define B1_S_1 RA1
#define B1_S_2 RA0
#define B1_OUT LATB5
#define B1_LED LATC5
#define B2_S_1 RA3
#define B2_S_2 RA2
#define B2_OUT LATB4
#define B2_LED LATC6
#define B3_S_1 RA5
#define B3_S_2 RA4
#define B3_OUT LATB3
#define B3_LED LATC7
#define B4_S_1 RA6
#define B4_S_2 RA7
#define B4_OUT LATB2
#define B4_LED LATB0

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */
