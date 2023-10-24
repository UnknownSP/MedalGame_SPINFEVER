/*
 * File:   LEDBlink.h
 * Author: seimi
 *
 * Created on 2016/02/13, 13:38
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
#include "ECCP.h"

#define MODE_CHANGE_WAIT_COUNT 60000

typedef enum{
  STANDBY = 0,
  IN_GAME = 1,
} Game_mode;

#define RIFT_IN RA3
#define RIFT_UPPER RA4
#define RIFT_LOWER RA5
#define LOUNCH_RAIL_S RA0 //基板のI2Cから遠い方
#define OUT_1_S RA1 //発射機構に近い方
#define OUT_2_S RA2
#define LED_1 LATC1
#define LED_2 LATC0
#define LED_3 LATC5
#define LOUNCH LATB5 //基板のI2Cから遠い方
#define BALL_SET LATB3

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */
