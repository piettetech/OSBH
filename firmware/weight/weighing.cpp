/****************************************************************************
*   Electronic scale using HX711 as the ADC with built-in high-gain         *
*   OpAmp and display on a local OLED display.                              *
****************************************************************************/
#include "application.h"
#include "weighing.h"

extern char* itoa(int a, char* buffer, unsigned char radix);



// Common variables
unsigned long weighStack[stackMax];
unsigned long rawWeight = 0;
int read = 0;
bool isCalMode = false;
bool weightUpdated;
int stackIndex = 0;


/****************************************************************************
*****************************************************************************
*************************  Weighing functions  ******************************
*****************************************************************************
****************************************************************************/


/****************************************************************
*
*   resetADOutput()
*   Physically read the output from the ADC.
*   Details see the datasheet.
*
*****************************************************************/

unsigned long readADOutput()
{
    noInterrupts();
    unsigned long Count;
    digitalWrite(ADSK, LOW); //Start measuring (PD_SCK LOW)
    Count=0;
    while(digitalRead(ADDO)); //Wait until ADC is ready
    for (int i=0;i<24;i++)
    {
        digitalWrite(ADSK, HIGH); //PD_SCK HIGH (Start) 
        Count=Count<<1; //Shift Count left at falling edge
        digitalWrite(ADSK, LOW); //PD_SCK LOW
        if(digitalRead(ADDO)) Count++;
    }
    digitalWrite(ADSK, HIGH); 
    delayMicroseconds(5);
    Count = Count^0x800000; //This is the 25th pulses, gain set to 128
    digitalWrite(ADSK, LOW);
    interrupts();

    return(Count);
}


/****************************************************************
*
*   resetAD()
*   Long pulse will cause the ADC to reset
*   Details see the datasheet.
*
*****************************************************************/

void resetAD()
{
    digitalWrite(ADSK, HIGH); 
    delayMicroseconds(80);
    digitalWrite(ADSK, LOW); 
}

void pushWeighStack(unsigned long val)
{
    weighStack[stackIndex++] = val;
    if (stackIndex >= stackMax) stackIndex = 0;
}

/****************************************************************
*
*   avgWeighStack()
*   Calculate the average weight from the values stored in
*   the stack.
*
*****************************************************************/

int avgWeighStack()
{
    int sum = 0;
    int i = 0;
    for (i = 0; i < stackMax; i++) {
        sum += weighStack[i];
    }
    return (sum / i);
}

/****************************************************************
*
*   ADRead()
*   Read the average value from ADC and adjust the reading by
*   the necessary offset, then scale it to the correct weight
*
*****************************************************************/

int ADRead()
{
    char dp[32];
    weightUpdated = false;
    //pushWeighStack(readADOutput());
    //int avg = avgWeighStack();
    int output = readADOutput();
    int weight = (output-offset) * scale/1000;
    return weight; //weight
}

/****************************************************************
*
*   ADReadAssured()
*   Read the average weight by trying to read from the average
*   weight at least 4 times and upto 100 times or when value
*   read become stable.
*
*****************************************************************/

int ADReadAssured()
{
    int lastRead = 0;
    int delta = 0;
    int i = 0;
    lastRead = ADRead();
    while ((i < 4) || ((i < 100) && abs(ADRead() - lastRead) > 0)) {
        lastRead = ADRead();
        i++;
    }
    return lastRead;
}

void tare()
{
    offset = avgWeighStack();
}

int cal(String cmd)
{
  // Calibrate the scale using the value supplied in cmd
  // Login below dictates the calibre must be heavier than 50g

    // First, prevent scale update
    isCalMode = true;
    
    char dp[32];
    
    int v = cmd.toInt();
    if (v != 0) refWeight = v;
    
    // then set zero (tare)
    while (ADRead() > 5) { // weight not zero, something still on it?
		Serial.print(ADRead());
		Serial.print(" grams ");
        Serial.println("Remove things on top");
        delay(500);
    }
    tare();
    Serial.print("Put gramms on top:   ");
    Serial.println(refWeight);
    unsigned long oldReading = ADReadAssured();
    while (oldReading < 50) { // we need more than 50g for calibration
        delay(500);
        oldReading = ADReadAssured();
    }
    scale = refWeight / oldReading * scale;
    Serial.print("Scale:" );
    Serial.println(scale);
    isCalMode = false;
    
    return 1;
}

