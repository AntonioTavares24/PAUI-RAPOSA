

#ifndef ECG_hpp
#define ECG_hpp

#include "fftw3.h"
#include "OGLGraph.h"
#include "stdafx.h"
#include <mutex>

class ECG
{
private:
	static ECG* _instance;
	OGLGraph* graphObj;
	int samplingFrequency = 1000;
	bool m_peakFlag = false, applyBandpassFilter = false;

	float channelGainsECG = 100;
	float lowpassCutoffFrequency = 10, highpassCutoffFrequency = 40, previousSignal = 0, previousFilteredSignal_lp = 0, previousFilteredSignal_hp = 0;
	float m_maximum, m_peakThreshold = 0, m_peakTimer = 0, m_heartRates[100], currentHR = 0, m_pi = 3.14159265358979f, scale = 3;

	int frameLengthCounter = 0;
	const int arrayLengthRRI = 16, frequencyRRILimit = 23;
	
	int _rriFound = 0;

	std::list<float> arrayRRIForTimeDomain, arrayRRIForFrequencyDomain;
	std::list<float> _data2, _thresh, _rriFoundList, _hrList, _rriList, _sdnnList, _rmssdList, _lfList, _hfList, _svbList;
	float _preRRI = 0, _RRI = 0.0, _mean = 0.0, _deltaMean = 0.0, _deltaSD = 0.0; //-- delta items are used for calculations on 2 successive RRI
	float displayThreshold = 0;

public:
	// ECG Data
	float  _RMSSD = 0.0, _heartRate = 0.0, _SDNN = 0.0, _LF = 0.0, _HF = 0.0, _VLF = 0.0;
	std::mutex ecgMutex;

	static ECG* Instance();
	void processECGSignal(BITalino::VFrame);
	void processECGSignal(float frame[]);
	inline void ECGTimeDomain();
	inline void ECGFrequencyDomain();
	inline void processingBlock(float F, float signal, int i);
	inline void finalizingThings();
	inline float applyBandPassFilter(float signal, int i);
};

#endif