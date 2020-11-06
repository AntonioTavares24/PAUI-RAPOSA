#ifndef MAINTOOLS_H
#define MAINTOOLS_H

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <chrono>
#include <Windows.h>

using std::cout;
using std::endl;

//Max Size of Bitmap Size String
const int bmSizeStrMax = 20;

//SikuliX URL's
//static std::string callurl = "callurl http://192.168.40.150:50001/";
static std::string callurl = "callurl http://localhost:50001/";
static std::string stopSikuli = "stop";
static std::string buttonURL[TOTAL_BUTTONS] = {"run/buttonOperation", 
										"run/buttonSetup"};
static std::string stressChanges = "run/stressChanges";
static std::string restChanges = "run/restChanges";

//Exception Classes
class lightException : public std::exception {
public:
	explicit lightException(const std::string& str);
	virtual const char* what() const throw();

private:
	std::string message;
};

class heavyException : public std::exception {
public:
	explicit heavyException(const std::string& str);
	virtual const char* what() const throw();

private:
	std::string message;
};

//Old GUI Handling Class
class InterfaceScreen {
public:
	InterfaceScreen(HWND windowHandle);
	~InterfaceScreen();
	void initialize();
	void takeScreenshot();
	char* getFileHeader();
	char* getInfoHeader();
	char* getBitmapArray();
	unsigned long int getBitmapSize();

private:
	HWND interfaceHwnd;

	HDC hdcWindow;
	HDC hdcMemDC;
	HBITMAP hbmScreen;
	BITMAP bmpScreen;
	RECT rcClient;

	int width;
	int height;
	DWORD dwBmpSize;

	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi;
	char* lpbitmap;
};

//Miscellaneous
std::ostream& par(std::ostream& out);

std::chrono::steady_clock::time_point printTimestamp(std::chrono::steady_clock::time_point start, const char* moment);

#endif