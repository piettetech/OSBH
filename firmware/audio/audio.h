
/**************************************

OSBH Project
January 2015

 *************************************/
#ifndef __audio__
#define __audio__

#include "application.h"
#include <stdarg.h>
#include <math.h>

#include "kiss_fftr.h"

extern int  MICROPHONE  ; // A1 on spark Core
extern int  FFT_SIZE  ; //FFT Bucket Size (32,64,128,256 - higher means more frequency resolution)
extern int  SAMPLEDELAY ; //Delay for sampling in microseconds f = 1/t*10^6


void FFTinit();

float windowfunction(unsigned int n);

void updateFFT();

void printfrequencies();

#endif
