
#ifndef CAM_HPP
#define CAM_HPP

#include <iostream>
#include <memory>
#include <chrono>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "Frame.h"
#include "Face.h"
#include "FrameDetector.h"
#include "AffdexException.h"

#include "AFaceListener.hpp"
#include "PlottingImageListener.hpp"
#include "StatusListener.hpp"


#include "WriteToFile.hpp"

#include <mutex>

using namespace std;
using namespace affdex;

class CAM
{
private:
	static CAM* _instance;
	DTF* dtfObj;
	string fileToWriteCAM = "D:\\PAUI-(M-ITI)\\Sample-Signals\\CAM1.txt";

	const std::vector<int> DEFAULT_RESOLUTION{ 640, 480 };
	affdex::path DATA_FOLDER;

	std::vector<int> resolution;
	int process_framerate = 30;
	int camera_framerate = 15;
	int buffer_length = 2;
	int camera_id = 0;
	unsigned int nFaces = 1;
	bool draw_display = true;
	int faceDetectorMode = (int)FaceDetectorMode::LARGE_FACES;

	float last_timestamp = -1.0f;
	float capture_fps = -1.0f;

	const int precision = 2;
	
public:
	Frame _frame;
	std::map<FaceId, Face> _faces;
	std::mutex camMutex;

	static CAM* Instance();
	int processCAM(int argsc, char ** argsv, bool displayCam, char* fileToWriteCAM, const std::chrono::steady_clock::time_point startClock);
	
};


#endif // !CAM_HPP