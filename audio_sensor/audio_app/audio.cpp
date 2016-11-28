
/**************************************

OSBH Project
January 2015

 *************************************/

#include "application.h"
#include <stdarg.h>
#include <math.h>

#include "kiss_fftr.h"

#define WAIT_FOR_KEYPRESS // for Debugging via serial communication -> see setup()

#define MICROPHONE 10
#define GAIN_CONTROL 11 // not used yet

#define FFT_SIZE 400
kiss_fftr_cfg fft_cfg;
kiss_fft_scalar *fft_in;
kiss_fft_cpx *fft_out;
int frequency;


void updateFFT() {
    //kiss_fft_scalar pt;
	
	int timespend = 0;
	int timestamp = millis();
    for(int i=0; i < FFT_SIZE; i++) {
        fft_in[i] = ((float)analogRead(MICROPHONE))/256.f; // decrease the signal input number so that u can process it with fft
        
        //testing with artificial sinus signal
        //fft_in[i] = sin(2*3.14*5/FFT_SIZE*i)+sin(2*3.14*0.45*i);
        
        delayMicroseconds(1000);  // Define the sample rate: 280us are about 3500Hz samplerate, 
									// frequency analysis works up to the half of this frequency
    }
    timespend = millis() - timestamp;
    frequency = FFT_SIZE*1000/timespend;
    Serial.print(FFT_SIZE);
    Serial.print("analog reads at ");
    Serial.print(frequency);
    Serial.print(" Hz needed");
    Serial.print(timespend);
    Serial.println(" ms");
    
    kiss_fftr(fft_cfg, fft_in, fft_out); //perform the fft with only real numbers, no imaginary parts - speeding up the process
}

void setup() {
	
	fft_cfg = kiss_fftr_alloc(FFT_SIZE, FALSE, NULL, NULL);
    fft_in = (kiss_fft_scalar*)malloc(FFT_SIZE * sizeof(kiss_fft_scalar));
    fft_out = (kiss_fft_cpx*)malloc(FFT_SIZE / 2 * sizeof(kiss_fft_cpx) + 1);
    
    char check = 'o';

    Serial.begin(9600);
#if defined(WAIT_FOR_KEYPRESS) // this is included for debugging mode. Entering a letter is not the usual way, but otherwise wouldn't work for all linux serial communication
    while(check != 'a') {
	delay(500);
	Serial.println(" enter <a> to begin"); // Entering a letter is not the usual way, but otherwise wouldn't work for all linux serial communications
	check = Serial.read();
	Spark.process();  // keep the spark happy
	delay(500);
    }
#endif


}



void loop() {

    updateFFT();
	
    for(int i=0; i < FFT_SIZE/2+2; i++) {
		float_t magn = fft_out[i].i * fft_out[i].i + fft_out[i].r * fft_out[i].r; // amplitude of the signal is the magnitude of the complex number
		if( magn > 100)  {	// realtively low filter for surrounding noise - important constant for analysis
		Serial.print("number ");
		Serial.print(i);
        Serial.print(" frequ: ");
        Serial.print(i*frequency/FFT_SIZE); // calculate the frequency bin
        Serial.print(",magn: ");
        Serial.println(magn);
		}
    }
    Serial.println();
    delay(5000);
}
