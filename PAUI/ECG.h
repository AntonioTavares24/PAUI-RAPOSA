
#include "bitalino.h"
#include "fftw3.h"
#include <List>


class ECG
{
private:
	static ECG* _instance;
	int samplingFrequency = 1000;
	bool m_peakFlag = false, applyBandpassFilter = true;

	double channelGainsECG = 100;
	double lowpassCutoffFrequency = 5, highpassCutoffFrequency = 40, previousSignal = 0, previousFilteredSignal_lp = 0, previousFilteredSignal_hp = 0;
	double m_maximum, m_peakThreshold = 0, m_peakTimer = 0, m_heartRates[100], currentHR = 0, m_pi = 3.14159265358979f, scale = 3, heartRate = 0;

	int rriInEachFrameArray = 0, arrayLengthRRI = 16, frameLength = 15000, frameLengthCounter = 0;
	double _RRI = 0, _RMSSD = 0;
	double _mean = 0, _SDNN = 0, _deltaMean = 0, _deltaSD = 0; //-- delta items are used for calculations on 2 successive RRI

	std::list<double> arrayRRIForTimeDomain, arrayRRIForFrequencyDomain;
	std::list<int> arrayRRIinEachFrameArray;
	double _preRRI = 0;

	// Frequency domain (PSD) variables
	float   timeIntervall = 1.0;     // in seconds
	int     nbrOfSamples = 1000;      //  number of Samples per time intervall, so the unit is S/s
	double  timeStep = timeIntervall / nbrOfSamples; // in seconds
	//float   frequency = 5;          // frequency in Hz

	int nbrOfInputSamples = 16;
	int n = 100; // Length of heartrate array
	//int nbrOfOutputSamples;
	
	// In the case of pure real input data,
	// the output values of the positive frequencies and the negative frequencies
	// are conjugated complex values.
	// This means, that there no need for calculating both.
	// If you have the complex values for the positive frequencitput
	double *in; // = (double*)fftw_malloc(sizeof(double) * nbrOfes,
	// you can calculate the values of the negative frequencies just by
	// changing the sign of the value's imaginary part
	// So the number of complex output values ( amplitudes of frequency components)
	// are the half of the number of the real input values ( amplitutes in time domain):
	//int nbrOfOutputSamples = ceil(nbrOfInputSamples / 2.0);

	// Create a plan for a 1D DFT with real input and complex ouInputSamples);
	fftw_complex *out; // = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nbrOfOutputSamples);

	fftw_plan p;

public:
	float displayThreshold = 0;
	static ECG* Instance();
	void processECGSignal(BITalino::VFrame, int);
	void processECGSignal(double frame[]);
	inline void ECGTimeDomain();
	inline void ECGFrequencyDomain();
	inline void processingBlock(double, double, int);
	inline void finalizingThings();
	inline double applyBandPassFilter(double, int);
};