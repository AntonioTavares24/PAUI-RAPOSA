

#ifndef EEG_HPP
#define EEG_HPP

//#include "bitalino.h"
#include "fftw3.h"
#include "OGLGraph.h"
//#include <List>
//#include "discpp.h"	// DISLIN Library for static graphs
#include <ostream>
#include <iostream>
#include <mutex>
#include "stdafx.h"

class EEG
{
private:
	OGLGraph* graphObj;
	static EEG* _instance;
	bool applyBandpassFilter = true;

	std::list<float> _listSignals, _signalForDisplay;

	float channelGainsEEG = 100;
	float lowpassCutoffFrequency = 5, highpassCutoffFrequency = 40, previousSignal = 0, previousFilteredSignal_lp = 0, previousFilteredSignal_hp = 0;
	float  m_pi = 3.14159265358979f;
	int nbrOfInputSamples = 3001;
	int nbrOfOutputSamples = ceil(nbrOfInputSamples / 2.0);
	float displayThreshold = 0;

public:
	// EEG Data 
	float delta = 0.0, theta = 0.0, alpha = 0.0, beta = 0.0, gamma = 0.0, engagement = 0.0;
	std::mutex eegMutex;

	static EEG* Instance();
	void processEEGSignal(BITalino::VFrame);
	void processEEGSignal(float frame[]);
	int calculatePSD();
	//void draw(double[], double[], double, int, double, double, double[][2] = {}, int = 0);
	//void testPSD();
};

#endif