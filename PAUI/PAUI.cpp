// PAUI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vld.h>

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <map>

#include "BoostTools.h"
#include "MainTools.h"
#include "SDLTools.h"

#include <windows.h>   // Standard Header For Most Programs
#include <math.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <iterator>
#include <cmath>
#include <conio.h>
#include <string>

#include <thread>	// for multi threading
#include <mutex>
#include <chrono>

#include "EYE.h"
#include "MinimalStatusNotifications.h"
#include "WriteToFile.hpp"
#include "ECG.hpp"
#include "EEG.hpp"
#include "EMG.hpp"
#include "EDA.hpp"
#include "CAM.hpp";
#include "DataBaseClass.hpp"




using namespace std;

OGLGraph* myGraph;
ECG* ecgObj;
EEG* eegObj;
EMG* emgObj;
EDA* edaObj;
DTF* dtfObj;
CAM* camObj;
DBC* dbcObj;
FDC* fdcObj;

ifstream myfile;
BITalino dev;

static int frameCounter = 0;
//static float emgRatio = 96.0;
int gwECG, gwEMG, gwEEG, gwEDA;
char ftwb[100];

bool errorDisplayed = false;	// Stop displaying end of file error after displaying once
bool doProcessing	= true;

float ecgfromFile[frameLength];
float emgfromFile[frameLength];
float eegfromFile[frameLength];
float edafromFile[frameLength];
string camfromFile;

SDL_Window* gWindow;
SDL_Surface* gScreenSurface;
SDL_Renderer* gRenderer;

LTexture gTexture;
LTexture cameraFrontImage;
LTexture cameraRearImage;
LTexture sensorTemp;
LTexture sensorHum;
LTexture classRedButton;
LTexture classGreenButton;

LButton gButtons[TOTAL_BUTTONS];
LButton gToggleClassButton;

SDL_Rect gOriginalSprites[17];
SDL_Rect gDetailedSprites[9];
SDL_Rect gCamSimulationSprites[6];
SDL_Rect gSensorSymbolSprites[2];

HWND hwnd;

std::atomic<int> emotionalState = 0;
std::atomic<int> currentTab = 0;
std::atomic<bool> isInterfaceReady = false;

// ###################################################################### //
// ################# Initialization fuction for objects ################# //
// ###################################################################### //
inline void initialize()
{
	dbcObj = DBC::Instance();	// Object of DatabaseClass

	#if PROCESS_ECG == 1
		ecgObj = ECG::Instance();	// Object of ECG
	#endif
	#if PROCESS_EEG == 1
		eegObj = EEG::Instance();	// Object of EEG
	#endif
	#if PROCESS_EMG == 1
		emgObj = EMG::Instance();	// Object of EMG
	#endif
	#if PROCESS_EDA == 1
		edaObj = EDA::Instance();	// Object of EMG
	#endif
	#if CAMERA == 1
		camObj = CAM::Instance();	// Object of CAM
	#endif	
	#if TOBII == 1
		fdcObj = FDC::Instance();	// Object of FixationDataClass
	#endif // TOBII == 1
	#if RECORD_BITALINO == 1
		dtfObj = new DTF();
		string str = "#TimeStamp(in miliSec);analog0;analog1;analog2;analog3;analog4;analog5;digital0;digital1;digital2;digital3;Stress;Workload;State\n";

		time_t now = time(0);
		tm *ltm = localtime(&now);
		sprintf(ftwb, "%s%d.%d.%d-%d.%d.%d.txt", fileToWriteBitalino, ltm->tm_mday, ltm->tm_mon + 1, ltm->tm_year + 1900, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
		dtfObj->writeData(str, ftwb);
	#endif
}

// ###################################################################### //
// ################# Reading data from file ############################# //
// ###################################################################### //
template<typename Out>
inline void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

int lineRead = 0;
int averageCam = 0;
float camData[33];
mutex mtx;

inline std::string readFile()
{
	while (frameCounter < frameLength)
	{
		string line;
		std::vector<std::string> elems;

		if (myfile.is_open())
		{
			if (getline(myfile, line))
			{
				lineRead++;
				if (line.substr(0, 1) == "#")
					continue;
				//printf("------------------------------------------------------------------------------ %d \n", lineRead);

				split(line, splitWith, std::back_inserter(elems));

				#if PROCESS_ECG == 1
					ecgfromFile[frameCounter] = stof(elems[ECG_INDEX + 1]);
				#endif //  PROCESS_ECG == 1
				#if  PROCESS_EMG == 1
					emgfromFile[frameCounter] = stof(elems[EMG_INDEX + 1]);
				#endif //  PROCESS_EMG == 1
				#if PROCESS_EEG == 1
					eegfromFile[frameCounter] = stof(elems[EEG_INDEX + 1]);
				#endif //  PROCESS_EEG == 1
				#if PROCESS_EDA == 1
					edafromFile[frameCounter] = stof(elems[EDA_INDEX + 1]);
				#endif //  PROCESS_EDA == 1

				#if CAM_FROM_FILE == 1
					//mtx.lock();
					camfromFile = "";
					for (int i = 0; i < 33; i++)
					{
						camData[i] = averageCam == 0 ? stof(elems[i + 14]) : (camData[i] + stof(elems[i + 14]));
						camfromFile += to_string(camData[i] / averageCam) + ";";
					}
					//mtx.unlock();					
					averageCam++;

				#endif // CAM_FROM_FILE == 1

				frameCounter++;
			}
			else
				return "Finish";
		}
		else return "Error";
	}
	return "Batch Read";
}

// ###################################################################### //
// ################# Reading data from bitalion ######################### //
// ###################################################################### //
inline BITalino::VFrame GetBitalinoFrames()
{
	std::string line;
	BITalino::VFrame frames(frameLength); // initialize the frames vector with 100 frames 
	dev.read(frames);
	return frames;
}

// ###################################################################### //
// ########## Insertion fuction to fill main Data base struct ########### //
// ###################################################################### //
void insertData()
{
	#if CLASSIFY_EMOTION == 1
		dbcObj->dense1Weights = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense1_weights.csv");
		dbcObj->dense1Bias = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense1_bias.csv");
		dbcObj->dense2Weights = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense2_weights.csv");
		dbcObj->dense2Bias = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense2_bias.csv");
		dbcObj->dense3Weights = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense3_weights.csv");
		dbcObj->dense3Bias = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_dense3_bias.csv");
		dbcObj->softmaxWeights = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_softmax_weights.csv");
		dbcObj->softmaxBias = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_softmax_bias.csv");
		
		dbcObj->mean = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_mean.csv");
		dbcObj->std = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_std.csv");
		dbcObj->pca = csv2matrix("D:\\PAUI-(M-ITI)\\Network Weights\\User4_pca.csv");

		dbcObj->scores[0] = 0;
		dbcObj->scores[1] = 0;
		dbcObj->scores[2] = 0;
	#endif

	while (doProcessing)
	{
		//mtx.lock();
		//ECG
		#if PROCESS_ECG == 1
			dbcObj->addECG(*ecgObj);
		#endif
		// EMG
		#if  PROCESS_EMG == 1
			dbcObj->addEMG(*emgObj);
			//emgRatio += 96;
			//if (!emgObj->peakStart)
			//{
			//	emgObj->numOfPeaks = 0;
			//	/*emgObj->numOfPlains = 0;
			//	emgObj->totalPlainTime = 0;
			//	emgObj->totalPeakTime = 0;
			//	emgObj->maxPeakMagnitude = 0;
			//	emgObj->currentpeakMag = 0;*/
			//	emgRatio = 96;
			//}
		#endif	
		// EEG
		#if PROCESS_EEG == 1
			dbcObj->addEEG(*eegObj);
		#endif
		// EEG
		#if PROCESS_EDA == 1
			dbcObj->addEDA(*edaObj);
		#endif
		// EYE
		#if TOBII == 1
			dbcObj->addEYE(*fdcObj);
		#endif 
		// CAM
		#if CAMERA == 1
			dbcObj->addCAM(*camObj);
		#endif 
		#if CAM_FROM_FILE == 1
			if (averageCam >= 3000)
			{
				dbcObj->addCAM(camfromFile);
				averageCam = 0;
			}	
		#endif

		dbcObj->insertDBM(lineRead);

		#if CLASSIFY_EMOTION == 1
			emotionalState = dbcObj->emotionalState;
		#endif

		std::this_thread::sleep_for(std::chrono::microseconds(processingFreq));	// Sleep for 500 micro Sec to run this thread at 2000Hz
		//mtx.unlock();
		
	}
}

// ###################################################################### //
// ################# Thread functions ################################### //
// ###################################################################### //
inline void pECG()
{
#if PROCESS_ECG == 1
	ecgObj->processECGSignal(ecgfromFile);
#endif
}

inline void pEMG()
{
#if  PROCESS_EMG == 1
	emgObj->processEMGSignal(emgfromFile);
#endif //  PROCESS_EMG == 1
}

inline void pEEG()
{
#if PROCESS_EEG == 1
	eegObj->processEEGSignal(eegfromFile);
#endif //  PROCESS_EEG == 1
}

inline void pEDA()
{
	#if PROCESS_EDA == 1
		edaObj->processEDASignal(edafromFile);
	#endif //  PROCESS_EDA == 1
}

inline void pCAM(int argc, char** args)
{
	camObj->processCAM(argc, args, displayCam, fileToWriteCAM, startClock);
}

void pBitalino(void)   // Bitalino processing block
{	

	#if DISPLAY_GRAPH == 1
	#else
		while (doProcessing)
		{
	#endif
	
	#if FROM_FILE == 1	// When data is from File
		string line = "";
		line = readFile();
		if (line == "Error")
		{
			printf("Error reading file \n");
			return;
		}
		else if (line == "Finish")
		{
			errorDisplayed = errorDisplayed ? true : printf("Data from file finished! \n");
			errorDisplayed = true;
			return;
		}

		#if PROCESS_ECG == 1
			//ecgObj->processECGSignal(ecgfromFile);
			std::thread ecgThread(pECG);
		#endif //  PROCESS_ECG == 1
		#if  PROCESS_EMG == 1
			//emgObj->processEMGSignal(emgfromFile);
			std::thread emgThread(pEMG);
		#endif //  PROCESS_EMG == 1
		#if PROCESS_EEG == 1
			//eegObj->processEEGSignal(eegfromFile);
			std::thread eegThread(pEEG);
		#endif //  PROCESS_EEG == 1
		#if PROCESS_EDA == 1
			//edaObj->processEDASignal(edafromFile);
			std::thread edaThread(pEDA);
		#endif //  PROCESS_EDA == 1
			
		frameCounter = 0;

		#if PROCESS_ECG == 1
			ecgThread.join();
		#endif //  PROCESS_ECG == 1
		#if  PROCESS_EMG == 1
			emgThread.join();
		#endif //  PROCESS_EMG == 1
		#if PROCESS_EEG == 1
			eegThread.join();
		#endif //  PROCESS_EEG == 1
		#if PROCESS_EDA == 1
			edaThread.join();
		#endif //  PROCESS_EDA == 1

		//insertData();

	#else	// Else when data is from Bitalino
			BITalino::VFrame f = GetBitalinoFrames();

		//#############Commenting the procesing of Bialino objects for now to collect data at faster rate for initial testing. 
		//############Will do the processing at offline mode########################
		#if	PROCESS_EMG == 1
			emgObj->processEMGSignal(f);
		#endif //  PROCESS_EMG == 1
		#if PROCESS_ECG == 1
			ecgObj->processECGSignal(f);
		#endif // PROCESS_ECG == 1
		#if PROCESS_EEG == 1
			eegObj->processEEGSignal(f);
		#endif // PROCESS_EEG == 1
		#if PROCESS_EDA == 1
			edaObj->processEDASignal(f);
		#endif // PROCESS_EDA == 1

		#if RECORD_BITALINO == 1
			for (int i = 0; i < f.size(); i++)
			{
				std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
				string str = to_string(std::chrono::duration_cast<std::chrono::milliseconds>(current - startClock).count()) + ";" + to_string(f[i].analog[0]) 
					+ ";" + to_string(f[i].analog[1]) + ";" + to_string(f[i].analog[2]) + ";" + to_string(f[i].analog[3]) + ";" + to_string(f[i].analog[4]) 
					+ ";" + to_string(f[i].analog[5]) + ";" + to_string(f[i].digital[0]) + ";" + to_string(f[i].digital[1]) + ";" + to_string(f[i].digital[2]) 
					+ ";" + to_string(f[i].digital[3]) + ";" + to_string(stressedStatus) + ";" + to_string(workloadStatus) + ";" + to_string(currentState) + "\n";
					
				dtfObj->writeData(str, ftwb);
			}
			/*if (stressedStatus == 1 || workloadStatus == 1)
			{
				stressedStatus = 0;
				workloadStatus = 0;
			}*/
		#endif		
	#endif

	#if DISPLAY_GRAPH == 1
	#else	
		}
	#endif
}

void pHandleSikuliRequest(std::string sikuliScript, int tabNumber) {
	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	system((callurl + sikuliScript).c_str());
	printTimestamp(timestamp, "SIKULI THREAD");
	currentTab = tabNumber;

	return;
}

void pInterface(void) {
	using std::cout;
	using std::endl;
	using std::cerr;
	using boost::asio::ip::tcp;

	cout << "===============================================" << endl;
	cout << "PAUI - Physiologically Attentive User Interface" << endl;
	cout << "===============================================" << par;

	system("sikuliconfig.bat");

	if (!initSDL()) {
		cout << "Shutting down..." << endl;
		return;
	}

	if (!loadMedia()) {
		cout << "Error while loading textures." << par;
		closeSDL();
		return;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(gWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	SDL_Event e;
	bool quit = false;
	
	bool toggleClassification = true;
	bool stressChangesApplied = false;
	bool restChangesApplied = true;

	cout << "Preparing the system API..." << endl;

	boost::asio::io_service ioService;
	boost::system::error_code errCode;
	tcp::acceptor clientAcceptor(ioService, tcp::endpoint(tcp::v4(), PORT));
	std::shared_ptr<tcp::socket> clientSocket(new tcp::socket(ioService));

	cout << "Socket ready to accept a connection.\n" << par;

	clientAcceptor.accept(*clientSocket);
	boost::asio::ip::tcp::no_delay optionNoDelay(true);
	clientSocket->set_option(optionNoDelay);

	cout << "Connection established." << endl;
	cout << "Client IP: " << clientSocket->remote_endpoint().address().to_string() << par;

	char bitmapSizeStr[bmSizeStrMax];
	unsigned long int bitmapSize;

	//Receive image size
	readMessage(clientSocket, bitmapSizeStr, bmSizeStrMax, errCode);
	bitmapSize = std::stoi(bitmapSizeStr);

	char* bitmapArray = new char[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bitmapSize];
	char* bitmapDataPtr = bitmapArray + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	unsigned long int bitmapTotalSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bitmapSize;

	//Receive bitmap file and info headers
	readMessage(clientSocket, bitmapArray, sizeof(BITMAPFILEHEADER), errCode);
	readMessage(clientSocket, bitmapArray + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER), errCode);
	
	int cycleNr = 0;
	std::chrono::steady_clock::time_point timestamp;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	isInterfaceReady = true;

	while (!quit && !GetAsyncKeyState(VK_ESCAPE)) {
		try {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
				
				for (int i = 0; i < TOTAL_BUTTONS; i++) {
					if (gButtons[i].handleEvent(&e)) {
						std::thread sikuliTabChangeThread(pHandleSikuliRequest, buttonURL[i], i);
						sikuliTabChangeThread.detach();
						break;
					}
				}

				if (gToggleClassButton.handleEvent(&e)) {
					toggleClassification = !toggleClassification;
				}

				if (e.type == SDL_KEYDOWN) {
					switch (e.key.keysym.sym) {
					case SDLK_UP:
						if (emotionalState < 2) {
							emotionalState++;
							cout << "Emotional State: " << emotionalState << endl;
						}

						break;

					case SDLK_DOWN:
						if (emotionalState > 0) {
							emotionalState--;
							cout << "Emotional State: " << emotionalState << endl;
						}

						if (emotionalState == 0) {
							stressChangesApplied = false;
						}

						break;
					}
				}
			}

			if ((emotionalState == 1 || emotionalState == 2) && toggleClassification && !stressChangesApplied) {
				std::thread sikuliStressThread(pHandleSikuliRequest, stressChanges, 0);
				sikuliStressThread.detach();
				stressChangesApplied = true;
				restChangesApplied = false;
			}
			else if ((emotionalState == 0 || !toggleClassification) && !restChangesApplied) {
				std::thread sikuliStressThread(pHandleSikuliRequest, restChanges, 0);
				sikuliStressThread.detach();
				restChangesApplied = true;
				stressChangesApplied = false;
			}

			readMessage(clientSocket, bitmapDataPtr, bitmapSize, errCode);

			if (!loadScreenshot(bitmapArray, bitmapTotalSize)) {
				throw lightException("Error while loading image.\n");
			}
			else {
				renderInterface(emotionalState, toggleClassification, currentTab);
			}

			cycleNr++;
		}
		catch (heavyException& e) {
			cerr << "Heavy exception: " << e.what() << endl;
			break;
		}
		catch (lightException& e) {
			cerr << "Light exception: " << e.what() << endl;
			continue;
		}
		catch (std::exception& e) {
			cerr << "Standard exception: " << e.what() << endl;
			break;
		}
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
	cout << "Cycles/s = " << cycleNr / duration << endl;

	closeSDL();
	system((callurl + stopSikuli).c_str());

	delete[] bitmapArray;
	clientSocket->shutdown(tcp::socket::shutdown_both);
	clientSocket->close();

	cout << "Connection closed. Shutting down...\n" << endl;
}

// ##################################################################### //
// #### Idle, Init, Keyboard, and Reshape functions for glut graph ##### //
// ##################################################################### //
void reshapeECG(int w, int h)   // Create The Reshape Function (the viewport)
{
	glutSetWindow(gwECG);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void reshapeEMG(int w, int h)   // Create The Reshape Function (the viewport)
{
	glutSetWindow(gwEMG);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void reshapeEEG(int w, int h)   // Create The Reshape Function (the viewport)
{
	glutSetWindow(gwEEG);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void reshapeEDA(int w, int h)   // Create The Reshape Function (the viewport)
{
	glutSetWindow(gwEDA);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

void idle(void)
{
	glutPostRedisplay();
}

void init(GLvoid)     // Create Some Everyday Functions
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);				// Black Background
														//glClearDepth(1.0f);								// Depth Buffer Setup
	myGraph = OGLGraph::Instance();
	myGraph->setup(4050, 2000, 10, 10, 4, 6, 1, 1000, gwECG, gwEMG, gwEEG, gwEDA);
}

bool keypressed(void)
{
	return (_kbhit() != 0);
}

void keyboard(unsigned char key, int x, int y)  // Create Keyboard Function
{
	switch (key)
	{
	case 27:        // When Escape Is Pressed...
		#if FROM_FILE == 1
		#else
			dev.stop();
		#endif
		#if RECORD_BITALINO == 1
			dtfObj->fin();
		#endif
		
		doProcessing = false;
		exit(0);   // Exit The Program
		break;        // Ready For Next Case
	case 83:
		stressedStatus = 1;
		break;
	case 87:
		workloadStatus = 1;
		break;
	default:        // Now Wrap It Up
		break;
	}
}

// ################################################################### //
// ################# Main functions ################################## //
// ################################################################### //

int main(int argc, char* args[])   // Create Main Function For Bringing It All Together
{

	initialize();

	
	// ------------------------------------------------------------------ //
	// Starting All four threads for Camera, Tobii, Bitalino and Interface//
	// ------------------------------------------------------------------ //
	
	#if INTERFACE_ON == 1
		std::thread interfaceThread(pInterface);
		while (!isInterfaceReady) {}
	#endif

	#if CAMERA == 1
		std::thread camThread(pCAM, argc, args);
	#endif

	#if TOBII == 1

		getWindow();
		printf("\n\nWidth: %f,\t Height: %f\n", SCREEN_WIDTH, SCREEN_HEIGHT);

		TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
		TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
		TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;
		BOOL success;

		// initialize and enable the context that is our link to the EyeX Engine.
		success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
		success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
		success &= InitializeGlobalInteractorSnapshot(hContext);
		success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged2, NULL) == TX_RESULT_OK;
		success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
		success &= txEnableConnection(hContext) == TX_RESULT_OK;

		// let the events flow until a key is pressed.
		if (success) {
			printf("Initialization was successful.\n");
		}
		else {
			printf("Initialization failed.\n");
		}

	#endif

	std::thread processingThread(insertData);

	#if BITALINO == 1
		try
		{
			#if FROM_FILE == 1
				myfile.open(fileToReadBitalino);
			#else
				puts("Connecting to device...");
				dev.open("20:16:07:18:15:45");  // device MAC address (Windows and Linux)
				puts("Connected to device. Press Enter to exit.");

				std::string ver = dev.version();    // get device version string
				printf("BITalino version: %s\n", ver.c_str());

				dev.battery(10);  // set battery threshold (optional)
				dev.start(1000, { 0, 1, 2, 3, 4, 5 });   // start acquisition of all channels at 1000 Hz
			#endif
		}
		catch (std::exception& e)
		{
			printf("ERROR!  %s", e.what());
			exit(0);
		}

		#if DISPLAY_GRAPH == 1
			glutInit(&argc, args); // Erm Just Write It =)	
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Display Mode
			glutInitWindowSize(900, 500); // If glutFullScreen wasn't called this is the window size 500, 250
	
			#if PROCESS_ECG == 1
				gwECG = glutCreateWindow("ECG Graph");
				init();
				glutDisplayFunc(pBitalino);
				glutReshapeFunc(reshapeECG);
				glutKeyboardFunc(keyboard);
			#endif
			#if PROCESS_EMG == 1
				gwEMG = glutCreateWindow("EMG Graph"); // Window Title (args[0] for current directory as title)
				glutPositionWindow(50, 50);
				init();
				glutDisplayFunc(pBitalino);  // Matching Earlier Functions To Their Counterparts
				glutReshapeFunc(reshapeEMG);
				glutKeyboardFunc(keyboard);
			#endif		
			#if PROCESS_EEG == 1
				gwEEG = glutCreateWindow("EEG Graph"); // Window Title (args[0] for current directory as title)
				glutPositionWindow(100, 100);
				init();
				glutDisplayFunc(pBitalino);  // Matching Earlier Functions To Their Counterparts
				glutReshapeFunc(reshapeEEG);
				glutKeyboardFunc(keyboard);
			#endif
			#if PROCESS_EDA == 1
				gwEDA = glutCreateWindow("EDA Graph"); // Window Title (args[0] for current directory as title)
				glutPositionWindow(150, 150);
				init();
				glutDisplayFunc(pBitalino);  // Matching Earlier Functions To Their Counterparts
				glutReshapeFunc(reshapeEDA);
				glutKeyboardFunc(keyboard);
			#endif
	
			glutIdleFunc(idle);
			glutMainLoop();          // Initialize The Main Loop
		#else
			std::thread bitalinoThread(pBitalino);
		#endif
	#endif

NotEscape:printf("Press Esc key to exit.");
	char keyPressed = getch();
	if (keyPressed == 's')
	{
		stressedStatus = 1;
		workloadStatus = 0;
		printf("Stressed Activated!");
		goto NotEscape;
	}
	else if (keyPressed == 'w')
	{
		workloadStatus = 1;	
		stressedStatus = 0;
		printf("Workload Activated!");
		goto NotEscape;
	}
	else if (keyPressed == 'b')
	{
		workloadStatus = 1;
		stressedStatus = 1;
		printf("Both Activated!");
		goto NotEscape;
	}
	else if (keyPressed == 'd')
	{
		workloadStatus = 0;
		stressedStatus = 0;
		printf("Both Deactivated!");
		goto NotEscape;
	}
	else if (keyPressed == 'r')
	{
		currentState = -1;
		printf("In Resting Phase!");
		goto NotEscape;
	}
	else if (keyPressed == 't')
	{
		currentState = 1;
		printf("In Testing Phase!");
		goto NotEscape;
	}
	else if (keyPressed == 'i')
	{
		currentState = 0;
		printf("In Idle Phase!");
		goto NotEscape;
	}
	//else if ((int)keyPressed == 27)
	else if (keyPressed == 'm' || keyPressed == 'M')
		doProcessing = false;

	#if DISPLAY_GRAPH == 1
	#else
		#if BITALINO == 1
			bitalinoThread.join();
		#endif
	#endif

	#if CAMERA == 1
		camThread.join();
	#endif

	// -------------------------------------------------------------------- //
	// Closing Tobii and waiting for Camera thread to finish processing //
	// -------------------------------------------------------------------- //
	#if TOBII == 1

		printf("Exiting.\n");

		// disable and delete the context.
		txDisableConnection(hContext);
		txReleaseObject(&g_hGlobalInteractorSnapshot);
		success = txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE) == TX_RESULT_OK;
		success &= txReleaseContext(&hContext) == TX_RESULT_OK;
		success &= txUninitializeEyeX() == TX_RESULT_OK;
		if (!success) {
			printf("EyeX could not be shut down cleanly. Did you remember to release all handles?\n");
		}
	#endif	
		
	processingThread.join();

	#if INTERFACE_ON == 1
		interfaceThread.join();
	#endif

	getchar();

	return 0;
}