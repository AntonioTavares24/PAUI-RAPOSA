
#include "bitalino.h"
#include <List>

class EDA
{
private:
	static EDA* _instance;
	std::list<float> _listSignal;
	std::list<float> _SCL;
	std::list<float>::const_iterator _iterUniversal;

	bool firstTime = true;

	double _SCR = 0, _SD = 0;

	int frameCounter = 0, samplingFrequency = 1000;
	float movingAverage = 0, m_peakTimer = 0;

public:
	float displayThreshold = 0;
	static EDA* Instance();
	void processEDASignal(BITalino::VFrame, int);
	void processEDASignal(float frame[]);
};