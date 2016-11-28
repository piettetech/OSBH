
/**************************************

OSBH Project
January 2015

 *************************************/

#include "application.h"
#include "audio.h"

#include "kiss_fftr.h"
#include <stdarg.h>
#include <math.h>

kiss_fftr_cfg fft_cfg;
kiss_fft_scalar *fft_in;
kiss_fft_cpx *fft_out;
int frequency;

void FFTinit() {
	/**************************************************************************/
       // initialize audio frequency analysis arrays
/**************************************************************************/
	fft_cfg = kiss_fftr_alloc(FFT_SIZE, FALSE, NULL, NULL);
    fft_in = (kiss_fft_scalar*)malloc(FFT_SIZE * sizeof(kiss_fft_scalar));
    fft_out = (kiss_fft_cpx*)malloc(FFT_SIZE / 2 * sizeof(kiss_fft_cpx) + 1);
/**************************************************************************/
	
}

float windowfunction(unsigned int n) { 
	// define windowfunction to prevent leakage of the signal
	// Hamming-Window
	return 0.53836 - 0.46164 * cos(2.0 * M_PI * n / double(FFT_SIZE - 1));	
	} 

void updateFFT() {
    //kiss_fft_scalar pt;
	
	int timespend = 0;
	int timestamp = millis();
    for(int i=0; i < FFT_SIZE; i++) {
		
        fft_in[i] = (windowfunction(i)*(((float)analogRead(MICROPHONE))-2048)/16.f); // decrease the signal input number so that u can process it with fft
        /*float anIN = ((float)analogRead(MICROPHONE))/32.f;
        fft_in[i] = windowfunction(i)*anIN;
        Serial.print(anIN);
        Serial.print(" \t");
        Serial.print(windowfunction(i));
        Serial.print(" \t");
        Serial.println(fft_in[i]);
        //testing with artificial sinus signal */
        //fft_in[i] = sin(2*3.14*5/FFT_SIZE*i)+sin(2*3.14*0.45*i);
        
        delayMicroseconds(SAMPLEDELAY);  // Define the sample rate: 280us are about 3500Hz samplerate, 
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
    
    kiss_fftr(fft_cfg, fft_in, fft_out);
}

void printfrequencies() {
	updateFFT();
	/*
	for(int i=0; i < FFT_SIZE/2+2; i++) {
		float_t magn = fft_out[i].i * fft_out[i].i + fft_out[i].r * fft_out[i].r; // amplitude of the signal is the magnitude of the complex number
		if( magn > 1000)  {	// realtively low filter for surrounding noise - important constant for analysis
		Serial.print("number ");
		Serial.print(i);
        Serial.print(" frequ: ");
        Serial.print(i*frequency/FFT_SIZE); // calculate the frequency bin
        Serial.print(",magn: ");
        Serial.println(magn);
		}
		
    }*/
}






