/*
	Header file for OGLGraph class
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
#ifndef OGLGRAPH_H
#define OGLGRAPH_H

#include <time.h>
#include <List>
#include "glut.h"
#include "bitalino.h"

class OGLGraph
{
public:
	static OGLGraph* Instance();	
	~OGLGraph() {}
	void setup(int width, int height, int offsetX, int offsetY, int scaleX, int scaleY, int channels, int cacheSize, int ecgId, int emgId, int eegId, int edaId);
	void update( double frame);
	void update(BITalino::VFrame, int);
	void update(float point, float thresh, int rriFound, float HR, float RRI, float SDNN, float RMSSD, float LF, float HF, float SVB);
	void updateEMG(float oEMG, float pEMG, float mean, float pEMG150);
	void draw();

	void drawEMG(std::list<float> _oEMGList, std::list<float> _pEMGList, std::list<float> _meanEMGList, std::list<float> _pEMG150List);
	void drawECG(std::list<float> _data2, std::list<float> _thresh, std::list<float> _rriFoundList, std::list<float> _hrList, 
		std::list<float> _rriList, std::list<float> _sdnnList, std::list<float> _rmssdList, std::list<float> _lfList, 
		std::list<float> _hfList, std::list<float> _svbList);
	void OGLGraph::drawEEG(std::list<float> _eegSignal);
	void OGLGraph::drawEDA(std::list<float> _listSignal, std::list<float> _sclList);
protected:
//	OGLGraph();

private:
	/*inline void drawECG();*/
	static OGLGraph* _instance;
	int _ecgId, _emgId, _eegId, _edaId;

	OGLGraph() {}
	int _cacheSize;
	int _pWidth, _pHeight;	// in percentages of existing window sizes
	int _width, _height;
	int _channels;
	int _scaleX, _scaleY;
	int _offsetX, _offsetY;
	std::list<float> _data;
	//std::list<float> _data2, _thresh, _rriFoundList, _hrList, _rriList, _sdnnList, _rmssdList, _lfList, _hfList, _svbList;
	std::list<float> _movingAverage;
	std::list<float> _threshold;
	/*std::list<float>::const_iterator _iterData;*/
};

#endif