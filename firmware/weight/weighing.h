/****************************************************************************
*   Electronic scale using HX711 as the ADC with built-in high-gain         *
*   OpAmp and display on a local OLED display.                              *
****************************************************************************/
#ifndef __weighing__
#define __weighing__

#define stackMax 12

// Pin out defs
extern int ADSK ;
extern int ADDO ;
//int TARE = D4;		// Tare pin not implemented yet

// init defaults for scale. Set via cal() function or manually by evaluating sensor data
extern int offset ;
extern int scale ;
extern int refWeight ;

unsigned long readADOutput();

void resetAD();

void pushWeighStack(unsigned long val);

int avgWeighStack();

int ADRead();

int ADReadAssured();

void tare();

int cal(String cmd);

#endif
