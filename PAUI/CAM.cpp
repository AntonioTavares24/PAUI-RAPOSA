#include "CAM.hpp"


CAM* CAM::_instance = 0;

CAM* CAM::Instance()
{
	if (_instance == 0)
	{
		_instance = new CAM();
	}
	return _instance;

}

/// <summary>
/// Project for demoing the Windows SDK CameraDetector class (grabbing and processing frames from the camera).
/// </summary>
int CAM::processCAM(int argsc, char ** argsv, bool displayCam, char* fileToWriteCAM, const std::chrono::steady_clock::time_point startClock)
{
	// Initializing File writer ----------------
 	dtfObj = new DTF();
	string data = "HourMinSec;#TimeStamp(in microSec);Age;Anger;Contempt;Disgust;Engagement;Fear;Joy;Sadness;Surprise;Valence;Attention;Browfurrow;BrowRaise;Cheekraise;Chinraise;Dimpler;Eyeclosure;Eyewiden;InnerBrowraise;"
				"Jawdrop;Lidtighten;Lipcornerdepressor;Lippress;Lippucker;Lipstretch;Lipsuck;Mouthopen;Nosewrinkle;Smile;Smirk;Upperlipraise;Pitch;Roll;Yaw;Id\n";
	
	time_t now = time(0);	
	tm *ltm = localtime(&now);
	char ftwc[100];
	sprintf(ftwc, "%s%d.%d.%d-%d.%d.%d.txt", fileToWriteCAM, ltm->tm_mday, ltm->tm_mon + 1, ltm->tm_year + 1900, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	dtfObj->writeData(data, ftwc);
	// --------------------------

	namespace po = boost::program_options; // abbreviate namespace

	std::cerr << "Hit ESCAPE key to exit app.." << endl;
	shared_ptr<FrameDetector> frameDetector;


	try {

		std::cerr.precision(precision);
		std::cout.precision(precision);
		po::options_description description("Project for demoing the Affdex SDK CameraDetector class (grabbing and processing frames from the camera).");
		description.add_options()
			("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
			("data,d", po::wvalue< affdex::path >(&DATA_FOLDER)->default_value(affdex::path(L"C:\\Program Files (x86)\\Affectiva\\AffdexSDK\\data"), std::string("data")), "Path to the data folder")
#else //  _WIN32
			("data,d", po::value< affdex::path >(&DATA_FOLDER)->default_value(affdex::path("data"), std::string("data")), "Path to the data folder")
#endif // _WIN32
			("resolution,r", po::value< std::vector<int> >(&resolution)->default_value(DEFAULT_RESOLUTION, "640 480")->multitoken(), "Resolution in pixels (2-values): width height")
			("pfps", po::value< int >(&process_framerate)->default_value(30), "Processing framerate.")
			("cfps", po::value< int >(&camera_framerate)->default_value(30), "Camera capture framerate.")
			("bufferLen", po::value< int >(&buffer_length)->default_value(30), "process buffer size.")
			("cid", po::value< int >(&camera_id)->default_value(0), "Camera ID.")
			("faceMode", po::value< int >(&faceDetectorMode)->default_value((int)FaceDetectorMode::LARGE_FACES), "Face detector mode (large faces vs small faces).")
			("numFaces", po::value< unsigned int >(&nFaces)->default_value(1), "Number of faces to be tracked.")
			("draw", po::value< bool >(&draw_display)->default_value(true), "Draw metrics on screen.")
			;
		po::variables_map args;
		//draw_display = displayCam;

		try
		{
			po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
			if (args["help"].as<bool>())
			{
				std::cout << description << std::endl;
				return 0;
			}
			po::notify(args);
		}
		catch (po::error& e)
		{
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << "For help, use the -h option." << std::endl << std::endl;
			return 1;
		}

		if (!boost::filesystem::exists(DATA_FOLDER))
		{
			std::cerr << "Folder doesn't exist: " << std::string(DATA_FOLDER.begin(), DATA_FOLDER.end()) << std::endl << std::endl;;
			std::cerr << "Try specifying the folder through the command line" << std::endl;
			std::cerr << description << std::endl;
			return 1;
		}
		if (resolution.size() != 2)
		{
			std::cerr << "Only two numbers must be specified for resolution." << std::endl;
			return 1;
		}
		else if (resolution[0] <= 0 || resolution[1] <= 0)
		{
			std::cerr << "Resolutions must be positive number." << std::endl;
			return 1;
		}

		std::ofstream csvFileStream;

		std::cerr << "Initializing Affdex FrameDetector" << endl;
		shared_ptr<FaceListener> faceListenPtr(new AFaceListener());
		shared_ptr<PlottingImageListener> listenPtr(new PlottingImageListener(csvFileStream, draw_display));    // Instanciate the ImageListener class
		shared_ptr<StatusListener> videoListenPtr(new StatusListener());
		frameDetector = make_shared<FrameDetector>(buffer_length, process_framerate, nFaces, (affdex::FaceDetectorMode) faceDetectorMode);        // Init the FrameDetector Class

																																				  //Initialize detectors
		frameDetector->setDetectAllEmotions(true);
		frameDetector->setDetectAllExpressions(true);
		frameDetector->setDetectAllEmojis(true);
		frameDetector->setDetectAllAppearances(true);
		frameDetector->setClassifierPath(DATA_FOLDER);
		frameDetector->setImageListener(listenPtr.get());
		frameDetector->setFaceListener(faceListenPtr.get());
		frameDetector->setProcessStatusListener(videoListenPtr.get());

		cv::VideoCapture webcam(camera_id);    //Connect to the first webcam
		webcam.set(CV_CAP_PROP_FPS, camera_framerate);    //Set webcam framerate.
		webcam.set(CV_CAP_PROP_FRAME_WIDTH, resolution[0]);
		webcam.set(CV_CAP_PROP_FRAME_HEIGHT, resolution[1]);
		std::cerr << "Setting the webcam frame rate to: " << camera_framerate << std::endl;
		auto start_time = std::chrono::system_clock::now();
		if (!webcam.isOpened())
		{
			std::cerr << "Error opening webcam!" << std::endl;
			return 1;
		}

		std::cout << "Max num of faces set to: " << frameDetector->getMaxNumberFaces() << std::endl;
		std::string mode;
		switch (frameDetector->getFaceDetectorMode())
		{
		case FaceDetectorMode::LARGE_FACES:
			mode = "LARGE_FACES";
			break;
		case FaceDetectorMode::SMALL_FACES:
			mode = "SMALL_FACES";
			break;
		default:
			break;
		}
		std::cout << "Face detector mode set to: " << mode << std::endl;

		//Start the frame detector thread.
		frameDetector->start();

		do {
			cv::Mat img;
			
			if (!webcam.read(img))    //Capture an image from the camera
			{
				std::cerr << "Failed to read frame from webcam! " << std::endl;
				break;
			}

			//Calculate the Image timestamp and the capture frame rate;
			const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
			const double seconds = milliseconds.count() / 1000.f;

			// Create a frame
			Frame f(img.size().width, img.size().height, img.data, Frame::COLOR_FORMAT::BGR, seconds);
			capture_fps = 1.0f / (seconds - last_timestamp);
			last_timestamp = seconds;
			frameDetector->process(f);  //Pass the frame to detector

			// For each frame processed
			if (listenPtr->getDataSize() > 0)
			{
				std::pair<Frame, std::map<FaceId, Face> > dataPoint = listenPtr->getData();
				Frame frame = dataPoint.first;
				std::map<FaceId, Face> faces = dataPoint.second;

				_frame = frame;
				camMutex.lock();
				_faces = faces;
				camMutex.unlock();
				/*string faceEmo = std::to_string(faces[0].appearance.age) + "; " + std::to_string(faces[0].emotions.anger)
					+ "; " + std::to_string(faces[0].emotions.contempt) + "; " + std::to_string(faces[0].emotions.disgust) + "; " + std::to_string(faces[0].emotions.engagement)
					+ ";" + std::to_string(faces[0].emotions.fear) + ";" + std::to_string(faces[0].emotions.joy) + ";" + std::to_string(faces[0].emotions.sadness) + ";" + std::to_string(faces[0].emotions.surprise) + ";" + std::to_string(faces[0].emotions.valence)
					+ ";" + std::to_string(faces[0].expressions.attention) + ";" + std::to_string(faces[0].expressions.browFurrow) + ";" + std::to_string(faces[0].expressions.browRaise) + ";" + std::to_string(faces[0].expressions.cheekRaise)
					+ ";" + std::to_string(faces[0].expressions.chinRaise) + ";" + std::to_string(faces[0].expressions.dimpler) + ";" + std::to_string(faces[0].expressions.eyeClosure) + ";" + std::to_string(faces[0].expressions.eyeWiden)
					+ ";" + std::to_string(faces[0].expressions.innerBrowRaise) + ";" + std::to_string(faces[0].expressions.jawDrop) + ";" + std::to_string(faces[0].expressions.lidTighten) + ";" + std::to_string(faces[0].expressions.lipCornerDepressor)
					+ ";" + std::to_string(faces[0].expressions.lipPress) + ";" + std::to_string(faces[0].expressions.lipPucker) + ";" + std::to_string(faces[0].expressions.lipStretch) + ";" + std::to_string(faces[0].expressions.lipSuck)
					+ ";" + std::to_string(faces[0].expressions.mouthOpen) + ";" + std::to_string(faces[0].expressions.noseWrinkle) + ";" + std::to_string(faces[0].expressions.smile) + ";" + std::to_string(faces[0].expressions.smirk)
					+ ";" + std::to_string(faces[0].expressions.upperLipRaise) + ";" + std::to_string(faces[0].measurements.orientation.pitch) + ";" + std::to_string(faces[0].measurements.orientation.roll)
					+ ";" + std::to_string(faces[0].measurements.orientation.yaw) + ";" + std::to_string(faces[0].id) + "\n";

				cout << faceEmo << endl;*/
				// Draw metrics to the GUI
				if (draw_display)
				{
					//listenPtr->draw(faces, frame);
					for (auto & face_id_pair : faces)
					{
						affdex::Face f = face_id_pair.second;
						std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();	

						time_t now = time(0);
						tm *ltm = localtime(&now);
						string timestamp = to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);

						string date2 = timestamp + "; " + to_string(std::chrono::duration_cast<std::chrono::milliseconds>(current - startClock).count()) + "; " + std::to_string(f.appearance.age) + "; " + std::to_string(f.emotions.anger) 
							+ "; " + std::to_string(f.emotions.contempt) + "; " + std::to_string(f.emotions.disgust) + "; " + std::to_string(f.emotions.engagement)
							+ ";" + std::to_string(f.emotions.fear) + ";" + std::to_string(f.emotions.joy) + ";" + std::to_string(f.emotions.sadness) + ";" + std::to_string(f.emotions.surprise) + ";" + std::to_string(f.emotions.valence)
							+ ";" + std::to_string(f.expressions.attention) + ";" + std::to_string(f.expressions.browFurrow) + ";" + std::to_string(f.expressions.browRaise) + ";" + std::to_string(f.expressions.cheekRaise)
							+ ";" + std::to_string(f.expressions.chinRaise) + ";" + std::to_string(f.expressions.dimpler) + ";" + std::to_string(f.expressions.eyeClosure) + ";" + std::to_string(f.expressions.eyeWiden)
							+ ";" + std::to_string(f.expressions.innerBrowRaise) + ";" + std::to_string(f.expressions.jawDrop) + ";" + std::to_string(f.expressions.lidTighten) + ";" + std::to_string(f.expressions.lipCornerDepressor)
							+ ";" + std::to_string(f.expressions.lipPress) + ";" + std::to_string(f.expressions.lipPucker) + ";" + std::to_string(f.expressions.lipStretch) + ";" + std::to_string(f.expressions.lipSuck)
							+ ";" + std::to_string(f.expressions.mouthOpen) + ";" + std::to_string(f.expressions.noseWrinkle) + ";" + std::to_string(f.expressions.smile) + ";" + std::to_string(f.expressions.smirk)
							+ ";" + std::to_string(f.expressions.upperLipRaise) + ";" + std::to_string(f.measurements.orientation.pitch) + ";" + std::to_string(f.measurements.orientation.roll)
							+ ";" + std::to_string(f.measurements.orientation.yaw) + ";" + std::to_string(f.id) + "\n";

						dtfObj->writeData(date2, ftwc);
						
						break;	// Currently writing data for only one face
					}
				}

				/*std::cerr << "timestamp: " << frame.getTimestamp()
					<< " cfps: " << listenPtr->getCaptureFrameRate()
					<< " pfps: " << listenPtr->getProcessingFrameRate()
					<< " faces: " << faces.size() << endl;*/

				//Output metrics to the file
				//listenPtr->outputToFile(faces, frame.getTimestamp());
			}


		}

#ifdef _WIN32
		while (!GetAsyncKeyState(VK_ESCAPE) && videoListenPtr->isRunning());
#else //  _WIN32
		while (videoListenPtr->isRunning());//(cv::waitKey(20) != -1);
#endif
		std::cerr << "Stopping FrameDetector Thread" << endl;
		frameDetector->stop();    //Stop frame detector thread
	}
	catch (AffdexException ex)
	{
		std::cerr << "Encountered an AffdexException " << ex.what();
		return 1;
	}
	catch (std::runtime_error err)
	{
		std::cerr << "Encountered a runtime error " << err.what();
		return 1;
	}
	catch (std::exception ex)
	{
		std::cerr << "Encountered an exception " << ex.what();
		return 1;
	}
	catch (...)
	{
		std::cerr << "Encountered an unhandled exception ";
		return 1;
	}

	dtfObj->fin();

	return 0;
}
