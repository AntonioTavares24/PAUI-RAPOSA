
#include "bitalino.h"
#include "fftw3.h"
#include <List>
#include "discpp.h"
#include <ostream>
#include <iostream>
//#include "WriteToFile.h"

class EEG
{
private:
	static EEG* _instance;
	int samplingFrequency = 500;
	bool applyBandpassFilter = true;

	std::list<float> _listSignals;

	float channelGainsEEG = 100;
	float lowpassCutoffFrequency = 5, highpassCutoffFrequency = 40, previousSignal = 0, previousFilteredSignal_lp = 0, previousFilteredSignal_hp = 0;
	float  m_pi = 3.14159265358979f;
	int nbrOfInputSamples = 3000;
	int n = 100; // Length of heartrate array

	// In the case of pure real input data,
	// the output values of the positive frequencies and the negative frequencies
	// are conjugated complex values.
	// This means, that there no need for calculating both.
	// If you have the complex values for the positive frequencies,
	// you can calculate the values of the negative frequencies just by
	// changing the sign of the value's imaginary part
	// So the number of complex output values ( amplitudes of frequency components)
	// are the half of the number of the real input values ( amplitutes in time domain):
	int nbrOfOutputSamples = ceil(nbrOfInputSamples / 2.0);

	// Create a plan for a 1D DFT with real input and complex output
	//double *in;// = (double*)fftw_malloc(sizeof(double) * nbrOfInputSamples);
	fftw_complex *out;// = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nbrOfOutputSamples);
	fftw_plan p;

public:
	float displayThreshold = 0;
	static EEG* Instance();
	void processEEGSignal(BITalino::VFrame, int);
	void processEEGSignal(float frame[]);
	int calculatePSD(double* in);
	void draw(double[], double[], double, int, double, double, double[][2] = {}, int = 0);
	void testPSD();
};