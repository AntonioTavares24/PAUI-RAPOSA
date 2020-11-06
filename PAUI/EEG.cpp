#include "EEG.hpp"

EEG* EEG::_instance = 0;

EEG* EEG::Instance()
{
	if (_instance == 0)
	{
		_instance = new EEG();
	}
	return _instance;
}

// HR, RRI, Delta Mean, and Delta Standard deviation calculation from Bitalion
void EEG::processEEGSignal(BITalino::VFrame frame)
{

	for (int i = 0; i < frame.size(); i++)
	{
		float signal = (float)frame[i].analog[EEG_INDEX];
		float F = signal;

		/*if (applyBandpassFilter)
		{
			float alpha_hp = samplingFrequency / (2 * m_pi * highpassCutoffFrequency + samplingFrequency);
			F = i == 0 ? 0 : alpha_hp * (previousFilteredSignal_hp + signal - previousSignal);
			previousFilteredSignal_hp = F;

			float alpha_lp = 2 * m_pi * lowpassCutoffFrequency / (2 * m_pi * lowpassCutoffFrequency + samplingFrequency);
			F = i == 0 ? 0 : alpha_lp * F + (1 - alpha_lp) * previousFilteredSignal_lp;
			previousFilteredSignal_lp = F;
		}
		previousSignal = i == 0 ? 0 : signal;	*/

		_listSignals.push_back(F);

		if (nbrOfInputSamples == _listSignals.size())
		{
			calculatePSD();
			for (int i = 0; i < frameLength; i++)
				_listSignals.pop_front();
		}
	}
}

// HR, RRI, Delta Mean, and Delta Standard deviation calculation from File
void EEG::processEEGSignal(float frame[])
{
	#if DISPLAY_GRAPH == 1
		graphObj = OGLGraph::Instance();
	#endif
	float x[100];
	float y[100];
	float yMin = 999.9, yMax = -999.9;

	for (int i = 0; i < frameLength; i++)
	{
		float signal = frame[i];
		float F = signal;
		/*
				if (applyBandpassFilter)
				{
					float alpha_hp = samplingFrequency / (2 * m_pi * highpassCutoffFrequency + samplingFrequency);
					F = i == 0 ? 0 : alpha_hp * (previousFilteredSignal_hp + signal - previousSignal);
					previousFilteredSignal_hp = F;

					float alpha_lp = 2 * m_pi * lowpassCutoffFrequency / (2 * m_pi * lowpassCutoffFrequency + samplingFrequency);
					F = i == 0 ? 0 : alpha_lp * F + (1 - alpha_lp) * previousFilteredSignal_lp;
					previousFilteredSignal_lp = F;
				}
				previousSignal = i == 0 ? 0 : signal;*/

		_listSignals.push_back(F);

		/*
				x[i] = i;
				y[i] = F;
				if (F < yMin)
					yMin = F;
				else if (F > yMax)
					yMax = F;*/
		//std::cout << "listSignals: " << _listSignals.size() << std::endl;
		if (nbrOfInputSamples == _listSignals.size())
		{
			calculatePSD();
			for (int i = 0; i < frameLength; i++)
				_listSignals.pop_front();
		}

		#if DISPLAY_GRAPH == 1		
			if (_signalForDisplay.size() >= 1000)
			{
				_signalForDisplay.pop_front();
				_signalForDisplay.push_back(F);
			}
			else
				_signalForDisplay.push_back(F);

			graphObj->drawEEG(_signalForDisplay);
		#endif
	}
	

	//draw(x, y, 100, 100, yMin, yMax);
}

// Reference from http://stackoverflow.com/a/24739037
// fftw linking instructions from http://stackoverflow.com/a/42137837
int EEG::calculatePSD()
{
	float* x;
	float* y;
	x = new float[nbrOfOutputSamples];
	y = new float[nbrOfOutputSamples];
	float yMin = DBL_MAX, yMax = DBL_MIN;

	double *in = (double*)fftw_malloc(sizeof(double) * nbrOfInputSamples);
	fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nbrOfOutputSamples);

	int idx = 0;

	std::copy(_listSignals.begin(), _listSignals.end(), in); //ver sizeof disto

	fftw_plan p = fftw_plan_dft_r2c_1d(nbrOfInputSamples, in, out, FFTW_ESTIMATE);

	// Perform the FFT
	fftw_execute(p);

	float realVal, imagVal, powVal, absVal, frequency;
	float frequencyDomainStep = ((float)samplingFrequency / (float)nbrOfInputSamples);
	float bands[6][2];

	eegMutex.lock();
	delta = 0; theta = 0; alpha = 0; beta = 0; gamma = 0;
	eegMutex.unlock();

	int framesCount = 0;
	for (idx = 0; idx < nbrOfOutputSamples; idx++)
	{
		//realVal = out[idx][0] / nbrOfInputSamples; // Ideed nbrOfInputSamples is correct!
		//imagVal = out[idx][1] / nbrOfInputSamples; // Ideed nbrOfInputSamples is correct!
		//powVal = 2 * (realVal*realVal + imagVal*imagVal);
		realVal = out[idx][0] * (2. / nbrOfInputSamples); // Ideed nbrOfInputSamples is correct!
		imagVal = out[idx][1] * (2. / nbrOfInputSamples); // Ideed nbrOfInputSamples is correct!
		powVal = (realVal*realVal + imagVal*imagVal);
		//powVal = 10. / log(10.) * log(powVal + 1e-6);   // dB

		absVal = sqrt(powVal / 2);
		frequency = idx * frequencyDomainStep;

		if (idx == 0)
			powVal /= 2;

		eegMutex.lock();

		if (frequency >= 0.5 && frequency <= 3.5)
			delta += powVal;

		else if (frequency >= 3.5 && frequency <= 8)
			theta += powVal;

		else if (frequency >= 8 && frequency <= 13)
			alpha += powVal;

		else if (frequency >= 13 && frequency <= 30)
			beta += powVal;

		else if (frequency >= 30 && frequency <= 45)
			gamma += powVal;

		eegMutex.unlock();

		if (frequency <= 61.0)
		{
			framesCount = idx;
			x[idx] = (float)frequency;
			y[idx] = powVal;
			if (powVal < yMin)
				yMin = powVal;
			else if (powVal > yMax)
				yMax = powVal;
		}
	}

	eegMutex.lock();
	int bandSize = 6;
	bands[0][0] = 'd';
	bands[0][1] = delta;
	bands[1][0] = 't';
	bands[1][1] = theta;
	bands[2][0] = 'a';
	bands[2][1] = alpha;
	bands[3][0] = 'b';
	bands[3][1] = beta;
	bands[4][0] = 'g';
	bands[4][1] = gamma;
	bands[5][0] = 'E';		// Engagement Level
	bands[5][1] = beta / (alpha + theta);

	engagement = beta / (alpha + theta);
	eegMutex.unlock();

	//printf("D: %0.4lf\t T: %0.4lf\t A: %0.4lf B: %0.4lf\t G: %0.4lf\t O: %0.4lf\n\n", delta, theta, alpha, beta, gamma, otherF);
	// Clean up

	//draw(x, y, x[framesCount - 1], framesCount, yMin, yMax, bands, bandSize);

	FFTW3_H::fftw_destroy_plan(p);
	fftw_free(in);
	FFTW3_H::fftw_free(out);

	//changed
	delete x;
	delete y;

	return 0;
}



//void EEG::testPSD()
//{
//	int outSam = ceil(255 / 2.0);
//	float* x;
//	float* y;
//	x = new float[outSam];
//	y = new float[outSam];
//
//	int N = 256;
//	float Fs = 200;//sampling frequency
//	float  T = 1 / Fs;//sample time 
//	float f = 20;//frequency
//
//	float t[255];//time vector 
//	float signal[255];
//	float yMin = FLT_MAX, yMax = FLT_MIN;
//	
//	float realVal, imagVal, powVal, absVal, frequency;
//	float frequencyDomainStep = ((float)Fs / (float)255);
//	int framesCount = 0;
//
//	for (int i = 0; i < N - 1; i++)
//	{
//		t[i] = i*T;
//		signal[i] = 0.7 *sin(2 * m_pi*f*t[i]);// generate sine waveform
//	}
//
//	fftw_complex *out1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outSam);
//	p = fftw_plan_dft_r2c_1d(255, signal, out1, FFTW_ESTIMATE);
//	// Perform the FFT
//	fftw_execute(p);
//
//	for (int i = 0; i < outSam; i++)
//	{
//		//realVal = out1[i][0] / nbrOfInputSamples; // Ideed nbrOfInputSamples is correct!
//		//imagVal = out1[i][1] / nbrOfInputSamples; // Ideed nbrOfInputSamples is correct!
//		//powVal = 2 * (realVal*realVal + imagVal*imagVal);
//		realVal = out1[i][0] * (2. / 255); // Ideed nbrOfInputSamples is correct!
//		imagVal = out1[i][1] * (2. / 255); // Ideed nbrOfInputSamples is correct!
//		powVal = (realVal*realVal + imagVal*imagVal);
//		//powVal = 10. / log(10.) * log(powVal + 1e-6);   // dB
//
//		absVal = sqrt(powVal / 2);
//		frequency = i * frequencyDomainStep;
//
//		if (frequency <= 101.0)
//		{
//			framesCount = i;
//			x[i] = (float)frequency;
//			y[i] = powVal;
//			if (powVal < yMin)
//				yMin = powVal;
//			if (powVal > yMax)
//				yMax = powVal;
//		}
//	}
//
//	FFTW3_H::fftw_destroy_plan(p);
//	//fftw_free(in);
//	FFTW3_H::fftw_free(out);
//
//	//draw(x, y, x[framesCount - 1], framesCount, yMin, yMax);
//
//}

//void EEG::draw(float X[], float Y[], float maxX, int framesCount, float yMin, float yMax, float bands[][2], int bandSize)
//{
//	yMin -= 5.2;
//	yMax += 0.2;
//	int i, ic;
//	float step, x;
//	Dislin g;
//
//	step = maxX / (framesCount - 1);
//	
//	g.metafl("cons");
//	g.scrmod("revers");
//	g.disini();
//	g.pagera();
//	g.complx();
//	g.axspos(450, 1600);
//	g.axslen(2300, 900);
//
//	g.name("---- frequency ---->", "x");
//	g.name("---- PSD ---->", "y");
//
//	g.labdig(-1, "x");
//	g.ticks(10, "x");
//	g.ticks(10, "y");
//
//	g.titlin("PSD CURVE",2);
//	char* title2 = new char;
//	sprintf(title2, " ");
//
//	for (int i = 0; i < bandSize; i++)
//		sprintf(title2, "%s %c: %0.4lf", title2, (char)bands[i][0], bands[i][1]);
//	g.titlin(title2, 3);
//
//	ic = g.intrgb(0.95, 0.95, 0.95);
//	g.axsbgd(ic);
//
//	//g.graf(0.0, 5, 0.0, 0.2, yMin, yMax, yMin, ((yMax - yMin) / 10.0));
//	g.graf(0.0, maxX, 0.0, maxX / 5.0, yMin, yMax, yMin, ((yMax - yMin) / 10.0));
//	g.setrgb(0.7, 0.7, 0.7);
//	g.grid(1, 1);
//
//	g.color("fore");
//	g.height(30);
//	g.title();
//
//	g.color("red");
//	g.curve(X, Y, n);
//	g.disfin();
//
//}