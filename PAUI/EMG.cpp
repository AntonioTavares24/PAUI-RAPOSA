#include "EMG.hpp"

EMG* EMG::_instance = 0;

EMG* EMG::Instance()
{
	if (_instance == 0)
	{
		_instance = new EMG();
	}
	return _instance;
	
}

void EMG::processEMGSignal(void)
{
}
void EMG::processEMGSignal(BITalino::VFrame frame)
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	for (int i = 2; i < frame.size() - 2; i += 1)
	{

		_signalArray[0] = (float)frame[i - 2].analog[EMG_INDEX];
		_signalArray[1] = (float)frame[i - 1].analog[EMG_INDEX];
		_signalArray[2] = (float)frame[i].analog[EMG_INDEX];
		_signalArray[3] = (float)frame[i + 1].analog[EMG_INDEX];
		_signalArray[4] = (float)frame[i + 2].analog[EMG_INDEX];

		processingBlock();

		#if DISPLAY_GRAPH == 1
				graphObj->drawEMG(_listSignal, _pEMGList, _meanEMGList, _pEMG150List);
		#endif
	}
	firstTime = false;	
}

void EMG::processEMGSignal(float frame[])
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	int j = 20;
	for (int i = 2; i < frameLength - 2; i += 1)
	{
		_signalArray[0] = (float)frame[i - 2];
		_signalArray[1] = (float)frame[i - 1];
		_signalArray[2] = (float)frame[i];
		_signalArray[3] = (float)frame[i + 1];
		_signalArray[4] = (float)frame[i + 2]; 

		processingBlock();

		#if DISPLAY_GRAPH == 1
			if (i == j)
			{
					graphObj->drawEMG(_listSignal, _pEMGList, _meanEMGList, _pEMG150List);
				j = j + 20;
			}
		#endif

	}
	firstTime = false;
}

inline void EMG::processingBlock()
{
	float pSignal = 0.0, oSignal = 0.0;

	if ((int)_listSignal.size() <= 1000)	// Run for first batch of data points
	{
		frameCounter++;
		oSignal		= _signalArray[2];
		_sum		+= _signalArray[2];
		_mean		= _sum / frameCounter;

		_listSignal.push_back(_signalArray[2]);

		_signalArray[0] = (_signalArray[0] < _mean ? (_mean + (_mean - _signalArray[0])) : _signalArray[0]);
		_signalArray[1] = (_signalArray[1] < _mean ? (_mean + (_mean - _signalArray[1])) : _signalArray[1]);
		_signalArray[2] = (_signalArray[2] < _mean ? (_mean + (_mean - _signalArray[2])) : _signalArray[2]);
		_signalArray[3] = (_signalArray[3] < _mean ? (_mean + (_mean - _signalArray[3])) : _signalArray[3]);
		_signalArray[4] = (_signalArray[4] < _mean ? (_mean + (_mean - _signalArray[4])) : _signalArray[4]);

		pSignal = (_signalArray[0] + _signalArray[1] + _signalArray[2] + _signalArray[3] + _signalArray[4]) / 5;
		
		_pEMGList.push_back(pSignal);
		_meanEMGList.push_back(_mean);
	}
	else
	{
		oSignal	= _signalArray[2];
		_it		= _listSignal.begin();
		_sum	-= *_it;
		_sum	+= _signalArray[2];
		_mean	= _sum / frameCounter;

		_listSignal.pop_front();
		_listSignal.push_back(_signalArray[2]);

		_signalArray[0] = (_signalArray[0] < _mean ? (_mean + (_mean - _signalArray[0])) : _signalArray[0]);
		_signalArray[1] = (_signalArray[1] < _mean ? (_mean + (_mean - _signalArray[1])) : _signalArray[1]);
		_signalArray[2] = (_signalArray[2] < _mean ? (_mean + (_mean - _signalArray[2])) : _signalArray[2]);
		_signalArray[3] = (_signalArray[3] < _mean ? (_mean + (_mean - _signalArray[3])) : _signalArray[3]);
		_signalArray[4] = (_signalArray[4] < _mean ? (_mean + (_mean - _signalArray[4])) : _signalArray[4]);

		pSignal = (_signalArray[0] + _signalArray[1] + _signalArray[2] + _signalArray[3] + _signalArray[4]) / 5;

		_pEMGList.pop_front();
		_meanEMGList.pop_front();
		_pEMG150List.pop_front();

		_pEMGList.push_back(pSignal);
		_meanEMGList.push_back(_mean);
	}

	float pSignal150	= _mean;
	/*int widthThresh		= 10;
	int noPeakThresh	= 130;
	int longPeakThresh	= 150;*/
	int pakStartThresh	= _mean + 20;
	int peakEndThresh	= _mean + 10;

	if (pSignal > pakStartThresh && !peakStart)		// When peak starts
	{				
		pSignal150			+= 100;	// Just for visualization
		peakStart			= true;
		maxPeakMagnitude	= maxPeakMagnitude < pSignal ? pSignal : maxPeakMagnitude;
		currentpeakMag		= pSignal;
		totalPeakTime++;	// Start peakTime with start of peak
		emgMutex.lock();
		numOfPeaks++;		// Increment peak counter
		emgMutex.unlock();
	}
	else if (pSignal <= peakEndThresh && peakStart)	// When peak ends
	{
		peakStart		= false;
		currentpeakMag	= currentpeakMag < pSignal ? pSignal : currentpeakMag;
		numOfPlains++;		// Increment Plain counter
	}
	else if (peakStart)		// In between peak start and end
	{
		pSignal150 += 100;	// Just for visualization
		totalPeakTime++;	//  increment peakTime when inside a peak
	}
	else
		totalPlainTime++; // Increment Plain time when not in peak
	

	_pEMG150List.push_back(pSignal150);
}