/*
	Implementation of the OGLGraph class functions
	Copyright (C) 2006 Gabriyel Wong (gabriyel@gmail.com)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "OGLGraph.h"

OGLGraph* OGLGraph::_instance = 0;

OGLGraph* OGLGraph::Instance()
{
	if (_instance == 0) 
	{
		_instance = new OGLGraph();
    }
    return _instance;
}

void OGLGraph::setup( int width, int height, int offsetX, int offsetY, 
				 int scaleX, int scaleY, int channels, int cacheSize, int ecgId, int emgId, int eegId, int edaId )
{
	_offsetX = offsetX;
	_offsetY = offsetY;
	_width = width;
	_height = height;
	_scaleX = scaleX;
	_scaleY = scaleY;
	_channels = channels;	// ToDo
	_cacheSize = cacheSize;
	_ecgId = ecgId;
	_emgId = emgId;
	_eegId = eegId;
	_edaId = edaId;
}

//--While reading from file line by line
void OGLGraph::update(double frames)
{
	if (_data.size() > (unsigned int)(_cacheSize))
	{
		_data.pop_front();
		_data.push_back(frames);

		//_movingAverage.pop_front();
		////_threshold.pop_front();

		//std::list<float>::const_iterator it;

		//// Computes moving average
		//float subTotal = 0.f;
		////_threshold.push_back(threshold);

		//for (it = _data.begin(); it != _data.end(); it++)
		//{
		//	subTotal += *it;
		//}
		//float maTemp = (float)(subTotal / (_data.size()));
		//_movingAverage.push_back(maTemp);

	}
	else
	{
		_data.push_back(frames);
		//_threshold.push_back(threshold);

		//// Computes moving average
		//std::list<float>::const_iterator it;
		//float subTotal = 0.f;
		//for (it = _data.begin(); it != _data.end(); it++)
		//{
		//	subTotal += *it;
		//}
		//float maTemp = (float)(subTotal / (_data.size()));
		//_movingAverage.push_back(maTemp);
	}
}

//--While reading several frames from bitalino from Bitalino
void OGLGraph::update(BITalino::VFrame f, int index)
{
	for (int i = 0; i < f.size(); i += 1)		// Only printing frames 10 point from 100 frames to keep things faster
	{
		if (_data.size() > (unsigned int)(_cacheSize))
		{
			_data.pop_front();
			_movingAverage.pop_front();

			std::list<float>::const_iterator it;

			// Computes moving average
			float subTotal = 0.f;
			_data.push_back((float)f[i].analog[index]);
			for (it = _data.begin(); it != _data.end(); it++)
			{
				subTotal += *it;
			}
			float maTemp = (float)(subTotal / (_data.size()));
			_movingAverage.push_back(maTemp);

		}
		else
		{
			_data.push_back((float)f[i].analog[index]);

			// Computes moving average
			std::list<float>::const_iterator it;
			float subTotal = 0.f;
			for (it = _data.begin(); it != _data.end(); it++)
			{
				subTotal += *it;
			}
			float maTemp = (float)(subTotal / (_data.size()));
			_movingAverage.push_back(maTemp);
		}
	}
}

//void OGLGraph::update(float point, float thresh, int rriFound, float HR, float RRI, float SDNN, float RMSSD, float LF, float HF, float SVB)
//{
//	glutSetWindow(_ecgId);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	// This is a dummy function. Replace with custom input/data
//	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
//	if (_data2.size() > (unsigned int)(_cacheSize))
//	{
//		_data2.pop_front();
//		//_thresh.pop_front();
//		//_rriFoundList.pop_front();
//		//_hrList.pop_front();
//		////_rriList.pop_front();
//		//_sdnnList.pop_front();
//		//_rmssdList.pop_front();
//		//_lfList.pop_front();
//		//_hfList.pop_front();
//		//_svbList.pop_front();
//
//		_data2.push_back(point);
//		//_thresh.push_back(thresh);
//		//_rriFoundList.push_back(rriFound);
//		//_hrList.push_back(HR);
//		////_rriList.push_back(RRI);
//		//_sdnnList.push_back(SDNN);
//		//_rmssdList.push_back(RMSSD);
//		//_lfList.push_back(LF);
//		//_hfList.push_back(HF);
//		//_svbList.push_back(SVB);
//	}
//	else
//	{
//		_data2.push_back(point);
//		//_thresh.push_back(thresh);
//		//_rriFoundList.push_back(rriFound);
//		//_hrList.push_back(HR);
//		////_rriList.push_back(RRI);
//		//_sdnnList.push_back(SDNN);
//		//_rmssdList.push_back(RMSSD);
//		//_lfList.push_back(LF);
//		//_hfList.push_back(HF);
//		//_svbList.push_back(SVB);
//	}
//	//draw();
//	draw2();
//	glutSwapBuffers();
//}

//void OGLGraph::updateEMG(float oEMG, float pEMG, float mean, float pEMG150)
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
//
//	if (_oEMGList.size() > (unsigned int)(_cacheSize))
//	{
//		_oEMGList.pop_front();
//		_pEMGList.pop_front();
//		_meanEMGList.pop_front();
//		_pEMG150List.pop_front();
//
//		_oEMGList.push_back(oEMG);
//		_pEMGList.push_back(pEMG);
//		_meanEMGList.push_back(mean);
//		_pEMG150List.push_back(pEMG150);
//	}
//	else
//	{
//		_oEMGList.push_back(oEMG);
//		_pEMGList.push_back(pEMG);
//		_meanEMGList.push_back(mean);
//		_pEMG150List.push_back(pEMG150);
//	}
//	drawEMG();
//	glutSwapBuffers();
//}

void OGLGraph::draw()
{
	int cntX = 0;
	int cntY = 0;
	// Set up the display
	glMatrixMode(GL_PROJECTION);  //We'll talk about this one more as we go 
	glLoadIdentity(); 
	glOrtho(0, _width, 0, _height, 0, 1.0); 

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

	// Draw the axes - OpenGL screen coordinates start from bottom-left (0,0)
	glDisable (GL_LINE_STIPPLE);
	glBegin( GL_LINES );
	glColor3ub( 255, 255, 255 );	// Green
	// Draw x-axis
	glVertex3f( _offsetX, _offsetY, 0 );
	glVertex3f( _width-_offsetX, _offsetY, 0 );
	// Draw y-axis
	glVertex3f( _offsetX, _offsetY, 0 );
	glVertex3f( _offsetX, 1000, 0 );
	glEnd();


	std::list<float>::const_iterator _it;

	// Draw the data points
	glBegin( GL_LINE_STRIP );
	glColor3ub( 0, 255, 0 );	// Green
	for (_it =_data.begin(); _it != _data.end(); _it++ )
	{
		//double scaled = (*_iterData);// +200.0) / 2;
		glVertex3f(10 + (cntX*_scaleX), (*_it), 0); // (10 + (*_iterData))*_scaleY, 0 );
		cntX++;
	}
	glEnd();

	//// Draw the moving average values
	//glEnable (GL_LINE_STIPPLE);
	//glLineStipple (3, 0xAAAA);
	//glBegin( GL_LINE_STRIP );
	//glColor3ub( 255, 0, 0);	// Red
	//// Draw moving average graph
	//for (_iterMA=_movingAverage.begin(); _iterMA != _movingAverage.end(); _iterMA++)
	//{
	//	//double scaled = (*_iterMA);// +200) / 2;
	//	glVertex3f(10 + (cntY*_scaleX), (*_iterMA), 0);// (10 + (*_iterMA))*_scaleY, 0);
	//	cntY++;
	//}
	//glEnd();

	//// Draw the Threshold values from ECG.cpp
	//cntY = 0;
	//glEnable(GL_LINE_STIPPLE);
	//glLineStipple(3, 0xAAAA);
	//glBegin(GL_LINE_STRIP);
	//glColor3ub(255, 0, 255);	// Blue
	//// Draw Threshold graph
	//for (_iterThreshold = _data2.begin(); _iterThreshold != _data2.end(); _iterThreshold++)
	//{
	//	glVertex3f(10 + (cntY*_scaleX), (10 + (*_iterThreshold))*_scaleY, 0);
	//	cntY++;
	//}
	//glEnd();
}

void OGLGraph::drawECG(std::list<float> _data2, std::list<float> _thresh, std::list<float> _rriFoundList, std::list<float> _hrList,
	std::list<float> _rriList, std::list<float> _sdnnList, std::list<float> _rmssdList, std::list<float> _lfList,
	std::list<float> _hfList, std::list<float> _svbList)
{
	glutSetWindow(_ecgId);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// This is a dummy function. Replace with custom input/data
		float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	glutSetWindow(_ecgId);
	int cntX = 0;
	int cntY = 0;
	// Set up the display
	glMatrixMode(GL_PROJECTION);  //We'll talk about this one more as we go 
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, 0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw the axes - OpenGL screen coordinates start from bottom-left (0,0)
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glColor3ub(255, 255, 255);	// Green
								// Draw x-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_width - _offsetX, _offsetY, 0);
	// Draw y-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_offsetX, _height, 0);
	glEnd();

	std::list<float>::const_iterator _it;

	// Draw the data points
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 255, 0);	// Green
	for (_it = _data2.begin(); _it != _data2.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 6), 0);
		cntX++;
	}
	glEnd();

	cntX = 0;
	// Draw the moving average values
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(3, 0xAAAA);
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 0, 0);	// Red
	for (_it = _thresh.begin(); _it != _thresh.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 6), 0);// (10 + (*_iterMA))*_scaleY, 0);
		cntX++;
	}
	glEnd();

	// Draw the RRI Peaks from ECG.cpp
	cntX = 0;
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 0, 255);	// Blue
	for (_it = _rriFoundList.begin(); _it != _rriFoundList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 10), 0);
		cntX++;
	}
	glEnd();

#if DISPLAY_GRAPH_FULL == 1
	// Draw the HR from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 153, 255);	// Light Purple
	for (_it = _hrList.begin(); _it != _hrList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 2), 0);
		cntX++;
	}
	glEnd();

	//// Draw the RRI from ECG.cpp
	//cntX = 0;
	//glBegin(GL_LINE_STRIP);
	//glColor3ub(102, 0, 102);	// dark Purple
	//for (_iterThreshold = _rriList.begin(); _iterThreshold != _rriList.end(); _iterThreshold++)
	//{
	//	glVertex3f(10 + (cntX*_scaleX), (*_iterThreshold * 500), 0);
	//	cntX++;
	//}
	//glEnd();

	// Draw the SDNN from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(76, 153, 0);	// Dark Green
	for (_it = _sdnnList.begin(); _it != _sdnnList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 9000), 0);
		cntX++;
	}
	glEnd();

	// Draw the RMSSD from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(229, 255, 204);	// Light Green
	for (_it = _rmssdList.begin(); _it != _rmssdList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 20000), 0);
		cntX++;
	}
	glEnd();

	// Draw the LF from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(128, 128, 128);	// Mid Gray
	for (_it = _lfList.begin(); _it != _lfList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 450000), 0);
		cntX++;
	}
	glEnd();

	// Draw the HF from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 255, 255);	// mid Sky Blue
	for (_it = _hfList.begin(); _it != _hfList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 3000000), 0);
		cntX++;
	}
	glEnd();

	// Draw the Sympathovagal Balance from ECG.cpp
	cntX = 0;
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 0, 127);	// Mid Pink
	for (_it = _svbList.begin(); _it != _svbList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 100), 0);
		cntX++;
	}
	glEnd();

#endif


	glutSwapBuffers();
}

void OGLGraph::drawEMG(std::list<float> _oEMGList, std::list<float> _pEMGList, std::list<float> _meanEMGList, std::list<float> _pEMG150List)
{
	glutSetWindow(_emgId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	int cntX = 0;
	int cntY = 0;
	// Set up the display
	glMatrixMode(GL_PROJECTION);  //We'll talk about this one more as we go 
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, 0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw the axes - OpenGL screen coordinates start from bottom-left (0,0)
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glColor3ub(255, 255, 255);	// Green
								// Draw x-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_width - _offsetX, _offsetY, 0);
	// Draw y-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_offsetX, _height, 0);
	glEnd();

	std::list<float>::const_iterator _it;

	// Draw Original EMG data points
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 255, 0);	// Green
	for (_it = _oEMGList.begin(); _it != _oEMGList.end(); _it++)
	{
		//double scaled = (*_iterData);// +200.0) / 2;
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1), 0); // (10 + (*_iterData))*_scaleY, 0 );
		cntX++;
	}
	glEnd();

#if DISPLAY_GRAPH_FULL == 1
	cntX = 0;
	// Draw processed EMG data values
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 0, 0);	// Red
	for (_it = _pEMGList.begin(); _it != _pEMGList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1.5), 0);// (10 + (*_iterMA))*_scaleY, 0);
		cntX++;
	}
	glEnd();

	cntX = 0;
	// Draw EMG Threshold data values
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 0, 255);	// Red
	for (_it = _meanEMGList.begin(); _it != _meanEMGList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1.5), 0);// (10 + (*_iterMA))*_scaleY, 0);
		cntX++;
	}
	glEnd();

	cntX = 0;
	// Draw processed EMG150 data values
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 255, 0);	// Yellow
	for (_it = _pEMG150List.begin(); _it != _pEMG150List.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1.5) - 10, 0);// (10 + (*_iterMA))*_scaleY, 0);
		cntX++;
	}
	glEnd();

#endif

	glutSwapBuffers();
}

void OGLGraph::drawEEG(std::list<float> _eegSignal)
{
	glutSetWindow(_eegId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	int cntX = 0;
	int cntY = 0;
	// Set up the display
	glMatrixMode(GL_PROJECTION);  //We'll talk about this one more as we go 
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, 0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw the axes - OpenGL screen coordinates start from bottom-left (0,0)
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glColor3ub(255, 255, 255);	// Green
								// Draw x-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_width - _offsetX, _offsetY, 0);
	// Draw y-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_offsetX, _height, 0);
	glEnd();

	std::list<float>::const_iterator _it;

	// Draw Original EMG data points
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 255, 0);	// Green
	for (_it = _eegSignal.begin(); _it != _eegSignal.end(); _it++)
	{
		//double scaled = (*_iterData);// +200.0) / 2;
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1), 0); // (10 + (*_iterData))*_scaleY, 0 );
		cntX++;
	}
	glEnd();

	glutSwapBuffers();
}

void OGLGraph::drawEDA(std::list<float> _listSignal, std::list<float> _sclList)
{
	glutSetWindow(_edaId);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)800 / (float)600, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	int cntX = 0;
	int cntY = 0;
	// Set up the display
	glMatrixMode(GL_PROJECTION);  //We'll talk about this one more as we go 
	glLoadIdentity();
	glOrtho(0, _width, 0, _height, 0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw the axes - OpenGL screen coordinates start from bottom-left (0,0)
	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glColor3ub(255, 255, 255);	// Green
								// Draw x-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_width - _offsetX, _offsetY, 0);
	// Draw y-axis
	glVertex3f(_offsetX, _offsetY, 0);
	glVertex3f(_offsetX, _height, 0);
	glEnd();

	std::list<float>::const_iterator _it;

	// Draw Original EMG data points
	glBegin(GL_LINE_STRIP);
	glColor3ub(0, 255, 0);	// Green
	for (_it = _listSignal.begin(); _it != _listSignal.end(); _it++)
	{
		//double scaled = (*_iterData);// +200.0) / 2;
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1), 0); // (10 + (*_iterData))*_scaleY, 0 );
		cntX++;
	}
	glEnd();

	cntX = 0;
	// Draw processed EMG data values
	glBegin(GL_LINE_STRIP);
	glColor3ub(255, 0, 0);	// Red
	for (_it = _sclList.begin(); _it != _sclList.end(); _it++)
	{
		glVertex3f(10 + (cntX*_scaleX), (*_it * 1), 0);// (10 + (*_iterMA))*_scaleY, 0);
		cntX++;
	}
	glEnd();

	glutSwapBuffers();

}