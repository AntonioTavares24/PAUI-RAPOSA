#include "EDA.hpp"

EDA* EDA::_instance = 0;

EDA* EDA::Instance()
{
	if (_instance == 0)
	{
		_instance = new EDA();
	}
	return _instance;
}

void EDA::processEDASignal(BITalino::VFrame frame)
{
	int n = 100; // Length of frame
	float sum = 0;

	for (int i = 0; i < frame.size(); i++)
	{
		float signal = (float)frame[i].analog[EDA_INDEX];

		if (firstTime)
		{
			sum += signal;
			frameCounter++;
			movingAverage = sum / frameCounter;
			_sclList.push_back(movingAverage);
			_listSignal.push_back(signal);
		}
		else
		{
			_iterUniversal = _listSignal.begin();
			sum -= *_iterUniversal;
			sum += signal;
			movingAverage = sum / n;	

			_sclList.pop_front();
			_listSignal.pop_front();

			_sclList.push_back(movingAverage);
			_listSignal.push_back(signal);
		}

		
		
		if (signal > movingAverage * 1.5f)
		{
			edaMutex.lock();
			_SCR = signal;
			_SCL = movingAverage;
			_SD  = _SCR - _SCL;
			edaMutex.unlock();

			m_peakTimer = 0;
		}
		m_peakTimer += 1.0f / (float)samplingFrequency;
	}
	firstTime = false;
	//printf("HR: %f\t dMean: %f\t dSD: %f\n", heartRate, _deltaMean, _deltaSD);
}

// HR, RRI, Delta Mean, and Delta Standard deviation calculation from File
void EDA::processEDASignal(float frame[])
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	
	int j = 20;
	for (int i = 0; i < frameLength; i++)
	{
		float signal = frame[i];
		if (_sclList.size() < 1000)
		{
			sum += signal;
			frameCounter++;
			movingAverage = sum / frameCounter;
			_sclList.push_back(movingAverage);
			_listSignal.push_back(signal);
		}
		else
		{
			_iterUniversal = _listSignal.begin();
			sum -= *_iterUniversal;
			sum += signal;
			movingAverage = sum / frameCounter;

			_sclList.pop_front();
			_listSignal.pop_front();

			_sclList.push_back(movingAverage);
			_listSignal.push_back(signal);
		}

		if (signal > movingAverage * 1.012f)
		{
			_SCR = signal;
			_SCL = movingAverage;
			_SD  = _SCR - _SCL;

			m_peakTimer = 0;
		}
		m_peakTimer += 1.0f / (float)samplingFrequency;

		#if DISPLAY_GRAPH == 1
			if (i == j)
			{
				graphObj->drawEDA(_listSignal, _sclList);
				j = j + 20;
			}
		#endif
	}
	firstTime = false;

	//printf("HR: %f\t dMean: %f\t dSD: %f\n", heartRate, _deltaMean, _deltaSD);
}