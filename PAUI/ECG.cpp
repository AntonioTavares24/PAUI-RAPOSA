#include "ECG.hpp"

ECG* ECG::_instance = 0;

ECG* ECG::Instance()
{
	if (_instance == 0)
	{
		_instance = new ECG();
	}
	return _instance;
	
}

// HR, RRI, Delta Mean, and Delta Standard deviation calculation from Bitalion
void ECG::processECGSignal(BITalino::VFrame frame)
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	for (int i = 0; i < frame.size(); i++)
	{
		float signal = (float)frame[i].analog[ECG_INDEX];
		signal = (signal - 50) * scale / channelGainsECG;
		float F = signal;

		if (applyBandpassFilter)
			F = applyBandPassFilter(signal, i);

		processingBlock(F, signal, i);

		//printf("HR: %0.1f RRI: %0.1f SDNN: %0.3f RMSSD: %0.3f LF: %0.4f HF: %0.4f SVB: %0.1f\n", _heartRate, _RRI*1000.0, _SDNN, _RMSSD, _LF, _HF, _LF / _HF);
		#if DISPLAY_GRAPH == 1
			//graphObj->update((float)F, (float)m_peakThreshold, _rriFound, (float)_heartRate, (float)_RRI, (float)_SDNN, (float)_RMSSD, (float)_LF, (float)_HF, (float)(_LF / _HF));
		#endif	
	}
	//finalizingThings();
}

// HR, RRI, Delta Mean, and Delta Standard deviation calculation from File
void ECG::processECGSignal(float frame[])
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	int j = 20;
	for (int i = 0; i < frameLength; i++)
	{
		float signal = frame[i];

		//graphObj->update((float)signal);	// Display graph

		signal = (signal - 50) * scale / channelGainsECG;
		//signal = (signal * (3.3 / 1024) - (3.3 / 2)) / 1100;
		float F = signal;
		
		if (applyBandpassFilter)
			F = applyBandPassFilter(signal, i);

		processingBlock(F, signal, i);

		//printf("HR: %0.1f RRI: %0.1f SDNN: %0.3f RMSSD: %0.3f LF: %0.4f HF: %0.4f SVB: %0.1f\n", _heartRate, _RRI*1000.0, _SDNN, _RMSSD, _LF, _HF, _LF/_HF);
		#if DISPLAY_GRAPH == 1
			if ((int)_data2.size() >= 1000)
			{
				_data2.pop_front();
				_thresh.pop_front();
				_rriFoundList.pop_front();
				_hrList.pop_front();
				_rriList.pop_front();
				_sdnnList.pop_front();
				_rmssdList.pop_front();
				_lfList.pop_front();
				_hfList.pop_front();
				_svbList.pop_front();

				_data2.push_back(F);
				_thresh.push_back(m_peakThreshold);
				_rriFoundList.push_back(_rriFound);
				_hrList.push_back(_heartRate);
				_rriList.push_back(_RRI);
				_sdnnList.push_back(_SDNN);
				_rmssdList.push_back(_RMSSD);
				_lfList.push_back(_LF);
				_hfList.push_back(_HF);
				_svbList.push_back((float)(_LF / _HF));
			}
			else
			{
				_data2.push_back(F);
				_thresh.push_back(m_peakThreshold);
				_rriFoundList.push_back(_rriFound);
				_hrList.push_back(_heartRate);
				_rriList.push_back(_RRI);
				_sdnnList.push_back(_SDNN);
				_rmssdList.push_back(_RMSSD);
				_lfList.push_back(_LF);
				_hfList.push_back(_HF);
				_svbList.push_back((float)(_LF / _HF));
			}
			if (i == j)
			{
				graphObj->drawECG(_data2, _thresh, _rriFoundList, _hrList, _rriList, _sdnnList, _rmssdList, _lfList, _hfList, _svbList);
				j = j + 20;
			}
				//graphObj->update((float)F, (float)m_peakThreshold, _rriFound, (float)_heartRate, (float)_RRI, (float)_SDNN, (float)_RMSSD, (float)_LF, (float)_HF, (float)(_LF / _HF));
		#endif	
	}
	//finalizingThings();
}

//----ECG Processing blocks----//
inline float ECG::applyBandPassFilter(float signal, int i)
{
	float F = signal;
	float alpha_hp = samplingFrequency / (2 * m_pi * highpassCutoffFrequency + samplingFrequency);
	F = i == 0 ? 0 : alpha_hp * (previousFilteredSignal_hp + signal - previousSignal);
	previousFilteredSignal_hp = F;

	float alpha_lp = 2 * m_pi * lowpassCutoffFrequency / (2 * m_pi * lowpassCutoffFrequency + samplingFrequency);
	F = i == 0 ? 0 : alpha_lp * F + (1 - alpha_lp) * previousFilteredSignal_lp;
	previousFilteredSignal_lp = F;

	return F;
}

inline void ECG::processingBlock(float F, float signal, int i)
{
	bool m_peakFlag = false;
	if (F > m_maximum)
		m_maximum = F;

	m_peakThreshold = m_maximum * 0.8f;	// 0.7f o

	if (F < m_peakThreshold && m_peakTimer > 1.5f)	//1.5f n
		m_maximum -= 0.01f;	// 0.01f n		

	if (F > m_peakThreshold * 1.1f && !m_peakFlag && m_peakTimer > 0.240)	// Previously 1.1f 1.3
	{
		_rriFound = 10;

		ecgMutex.lock();
		_heartRate = 60 / m_peakTimer;
		ecgMutex.unlock();
		
		_RRI = m_peakTimer;

		_deltaMean = (_preRRI + _RRI) / 2;
		float preSD = (_preRRI - _mean) * (_preRRI - _mean);
		_deltaSD = std::sqrt(((_preRRI - _mean) * (_preRRI - _mean)) + ((_RRI - _mean) * (_RRI - _mean)));

		//-------------------------//
		arrayRRIForFrequencyDomain.push_back(m_peakTimer);
		arrayRRIForTimeDomain.push_back(m_peakTimer);

		if (arrayRRIForTimeDomain.size() == arrayLengthRRI)
		{
			ECGTimeDomain();
			arrayRRIForTimeDomain.pop_front();
		}
		if (arrayRRIForFrequencyDomain.size() == frequencyRRILimit)
		{
			ECGFrequencyDomain();
			std::list<float>::const_iterator it = arrayRRIForFrequencyDomain.begin();
			frameLengthCounter -= *it * 1000;
			arrayRRIForFrequencyDomain.pop_front();
		}
		//-------------------------//

		ecgMutex.lock();
		if (_heartRate > 40 && _heartRate < 130)
		{
			for (int w = frameLength - 1; w > 0; w--)
				m_heartRates[w] = m_heartRates[w - 1];

			m_heartRates[0] = _heartRate;
		}
		ecgMutex.unlock();

		m_peakTimer = 0;
		m_peakFlag = true;
	}
	else
		_rriFound = 0;
	if (F < m_peakThreshold * 0.9f && m_peakFlag)
		m_peakFlag = false;


	m_peakTimer += 1.0f / (float)samplingFrequency;
	previousSignal = i == 0 ? 0 : signal;

	frameLengthCounter++;
}

inline void ECG::finalizingThings()
{	
	float sum = 0;
	for (int w = 0; w < frameLength; w++)
		sum += m_heartRates[w];

	currentHR = sum / frameLength;

	//---Update secCounter with 1 second of data, and keep it fixed when reached to dataLengthInSec value
	/*if (frameLengthCounter == frameLengthFD)
	{
		ECGFrequencyDomain();
		std::list<int>::const_iterator it;
		it = arrayRRIinEachFrameArray.begin();
		int rriToRemove = *it;
		
		for (int k =0; k < rriToRemove; k++)
			arrayRRIForFrequencyDomain.pop_front();

		arrayRRIinEachFrameArray.pop_front();
		frameLengthCounter -= frameLength;
	}*/
}
//-----------------------------//

// Calculating RMSSD, SDNN, and mean
void ECG::ECGTimeDomain()
{
	float squaredSumRRI = 0, sumRRI = 0, squaredSumOfSD = 0;

	std::list<float>::const_iterator it;

	bool firstRead = false;
	float previousRRI = 0;
	for (it = arrayRRIForTimeDomain.begin(); it != arrayRRIForTimeDomain.end(); it++)
	{
		if (!firstRead)
		{
			previousRRI = *it;
			firstRead = true;
			continue;
		}
		squaredSumRRI += ((previousRRI - *it) * (previousRRI - *it));
		sumRRI += *it;
		previousRRI = *it;
	}
	ecgMutex.lock();
	_RMSSD = std::sqrt((squaredSumRRI) / (arrayLengthRRI));
	ecgMutex.unlock();

	_mean = sumRRI / (arrayLengthRRI); //--Mean of RRI array

	for (it = arrayRRIForTimeDomain.begin(); it != arrayRRIForTimeDomain.end(); it++)
	{
		squaredSumOfSD += ((*it - _mean) * (*it - _mean));
	}

	ecgMutex.lock();
	_SDNN = std::sqrt(squaredSumOfSD / (arrayLengthRRI)); //-- SDNN Standard Deviation from Normal to Normal of RRI array
	ecgMutex.unlock();

	//printf("\nRMSSD: %f\t SDNN: %f\t Mean: %f\n", _RMSSD, _SDNN, _mean);
}

// Reference from http://stackoverflow.com/a/24739037
// fftw linking instructions from http://stackoverflow.com/a/42137837
void ECG::ECGFrequencyDomain()
{
	try
	{
		int idx = 0;
		ecgMutex.lock();
		_LF = 0; _HF = 0; _VLF = 0;
		ecgMutex.unlock();

		int inputSamples = arrayRRIForFrequencyDomain.size();
		int outputSamples = ceil(inputSamples / 2.0);

		double *in = (double*)fftw_malloc(sizeof(double) * inputSamples);
		fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outputSamples);

		std::copy(arrayRRIForFrequencyDomain.begin(), arrayRRIForFrequencyDomain.end(), in);

		int nbrOfInputSamples = (int)arrayRRIForFrequencyDomain.size();
		fftw_plan p = fftw_plan_dft_r2c_1d(nbrOfInputSamples, in, out, FFTW_ESTIMATE);

		// Perform the fft
		fftw_execute(p);

		float  realVal, imagVal, powVal, absVal;
		float frequency = 0;

		//It is sampling frequency (SF) / data points for FFT (DP)
		//In case of Bitalino SF = 1000 and DP = 15000, DP can vary depending upon the output frequency to measure
		float frequencyDomainFactor = ((float)samplingFrequency / (float)frameLengthCounter);

		for (idx = 0; idx < outputSamples; idx++)
		{
			realVal = out[idx][0] / nbrOfInputSamples; // Ideed nbrOfInputSamples is correct!
			imagVal = out[idx][1] / nbrOfInputSamples; // Ideed samplingFrequency is correct!
			powVal = 2 * (realVal*realVal + imagVal*imagVal);
			absVal = sqrt(powVal / 2);
			if (idx == 0) {
				powVal /= 2;
			}
			frequency = idx * frequencyDomainFactor;

			ecgMutex.lock();
			if (frequency >= 0.04 && frequency <= 0.15)
			{
				_LF += powVal;
			}
			else if (frequency >= 0.15 && frequency <= 0.4)
			{
				_HF += powVal;
			}
			else if (frequency >= 0.003 && frequency <= 0.04)
			{
				_VLF += powVal;
			}
			ecgMutex.unlock();
		}
		//printf("VLF: %lf\t LF: %lf\t HF: %lf\t SVB: %lf\n\n", _VLF, _LF, _HF, _LF / _HF);

		//// Clean up
		fftw_destroy_plan(p);
		fftw_free(in);
		fftw_free(out);
	}
	catch (std::exception ex)
	{
		printf("Error! \n %s", ex.what());
	}
}