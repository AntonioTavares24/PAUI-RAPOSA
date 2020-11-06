
#ifndef EDA_HPP
#define EDA_HPP

#include "OGLGraph.h"
#include "stdafx.h"
#include <mutex>

class EDA
{
private:
	OGLGraph* graphObj;
	static EDA* _instance;
	std::list<float> _listSignal;
	std::list<float> _sclList;
	std::list<float>::const_iterator _iterUniversal;

	bool firstTime = true;


	int frameCounter		= 0; 
	int samplingFrequency	= 1000;
	
	float sum				= 0;
	float movingAverage		= 0;
	float m_peakTimer		= 0;
	float displayThreshold	= 0;

public:
	float _SD = 0.0, _SCL = 0.0;
	float _SCR = 0.0;
	std::mutex edaMutex;

	static EDA* Instance();
	void processEDASignal(BITalino::VFrame);
	void processEDASignal(float frame[]);
};

#endif