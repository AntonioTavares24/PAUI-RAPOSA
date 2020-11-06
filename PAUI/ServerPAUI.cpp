#include "stdafx.h"
#include "BoostTools.h"
#include "MainTools.h"
#include "SDLTools.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

using std::cout;
using std::endl;
using std::cerr;
using boost::asio::ip::tcp;

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

SDL_Rect gOriginalSprites[12];
SDL_Rect gDetailedSprites[9];
SDL_Rect gCamSimulationSprites[6];
SDL_Rect gSensorSymbolSprites[2];

std::atomic<int> currentTab = 0;

void pHandleSikuliRequest(std::string sikuliScript, int tabNumber) {
	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	system((callurl + sikuliScript).c_str());
	printTimestamp(timestamp, "SIKULI THREAD");
	currentTab = tabNumber;

	return;
}

//TODO: FAZER HANDLING DE CTRL C
//TODO: CRIAR CLASSE PARA NETWORKING
//TODO: CONDENSAR CODIGO DA MAIN
//TODO: JUNTAR INTERFACE AO CLASSIFICADOR
//TODO: MUDAR NOMES DE VARIAVEIS E PARAMETROS DE FUNÇOES RENDER E BITMAP SIZES, ETC.
//TODO: FAZER SIKULI SCRIPT PARA REVERTER ALTERAÇOES PARA REST STATE

//Interface Ideas
//Criar simbolos para temperatura, humidade, etc. que pisquem conforme valor e que só apareçam quando o valor esta na zona amarela (podem aparecer em linha estilo debuffs)

int main(int argc, char* args[]) {

	cout << "===============================================" << endl;
	cout << "PAUI - Physiologically Attentive User Interface" << endl;
	cout << "===============================================" << par;

	system("sikuliconfig.bat");

	if (!initSDL()) {
		cout << "Shutting down..." << endl;
		return 0;
	}

	if (!loadMedia()) {
		cout << "Error while loading textures." << par;
		closeSDL();
		return 0;
	}

	SDL_Event e;
	bool quit = false;
	int emotionalState = 0;
	bool toggleClassification = true;
	bool togglePower = true;
	bool toggleSensors = true;
	bool toggleReverse = false;
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
	
	while (!quit) {
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

					case SDLK_p:
						togglePower = !togglePower;
						cout << "Power: " << togglePower << endl;
						break;

					case SDLK_s:
						toggleSensors = !toggleSensors;
						cout << "Sensors: " << toggleSensors << endl;
						break;

					case SDLK_r:
						toggleReverse = !toggleReverse;
						cout << "Reverse: " << toggleReverse << endl;
						break;
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
			}

			readMessage(clientSocket, bitmapDataPtr, bitmapSize, errCode);

			if (!loadScreenshot(bitmapArray, bitmapTotalSize)) {
				throw lightException("Error while loading image.\n");
			}
			else {
				renderInterface(emotionalState, toggleClassification, togglePower, toggleSensors, toggleReverse, currentTab);
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

	/*
	while (!quit) {
		try {
			cout << ">> NEW CYCLE" << endl;
			std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}

				for (int i = 0; i < TOTAL_BUTTONS; i++) {
					if (gButtons[i].handleEvent(&e)) {
						std::thread sikuliRequestThread(pHandleSikuliRequest, callurl + buttonURL[i]);
						sikuliRequestThread.detach();
						cout << "---> CLICK DETECTED <---" << endl;
						break;
					}
				}

				if (gToggleClassButton.handleEvent(&e)) {
					toggleClassification = !toggleClassification;
					cout << "---> CLICK DETECTED <---" << endl;
				}

				if (e.type == SDL_KEYDOWN) {
					cout << "---> KEY PRESS DETECTED <---" << endl;
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

						break;
					}
				}
			}

			timestamp = printTimestamp(timestamp, "EVENT LOOP");

			readMessage(clientSocket, bitmapDataPtr, bitmapSize, errCode);

			timestamp = printTimestamp(timestamp, "RECV SCREENSHOT");

			if (!loadScreenshot(bitmapArray, bitmapTotalSize)) {
				throw lightException("Error while loading image.\n");
			}
			else {
				timestamp = printTimestamp(timestamp, "LOAD SCREENSHOT");

				renderInterface(emotionalState, toggleClassification);

				timestamp = printTimestamp(timestamp, "RENDERING");
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
	}*/
	
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
	cout << "Cycles/s = " << cycleNr / duration << endl;

	closeSDL();
	system((callurl + stopSikuli).c_str());

	delete[] bitmapArray;
	clientSocket->shutdown(tcp::socket::shutdown_both);
	clientSocket->close();

	cout << "Connection closed. Shutting down...\n" << endl;

	return 0;
}