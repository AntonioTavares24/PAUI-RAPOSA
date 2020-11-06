

#ifndef EMG_HPP
#define EMG_HPP

#include "OGLGraph.h"
#include "stdafx.h"
#include <mutex>


class EMG
{
private:
	static EMG* _instance;
	OGLGraph* graphObj;

	std::list<float> _listSignal;
	std::list<float> _oEMGList, _pEMGList, _meanEMGList, _pEMG150List;
	std::list<float>::const_iterator _it;

	float _mean			= 0.0;
	float _SD				= 0.0; 
	float _sum				= 0.0;

	bool firstTime			= true;
	
	int frameCounter		= 0;

	float _signalArray[5];
	float maxPeakMagnitude = 0.0, currentpeakMag = 0.0;
	bool peakStart = false;
	float displayThreshold = 0;
	int totalTime = 0, totalPeakTime = 0, numOfPlains = 0, totalPlainTime = 0;

public:
	// EMG Data 
	int numOfPeaks = 0;
	std::mutex emgMutex;

	static EMG* Instance();
	void processEMGSignal(BITalino::VFrame);
	void processEMGSignal(float frame[]);
	void processEMGSignal(void);
	inline void processingBlock();
};

#endif