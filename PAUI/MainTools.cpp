#include "MainTools.h"

//Light Exception Methods
lightException::lightException(const std::string& str) {
	message = str;
}

const char* lightException::what() const throw() {
	return message.c_str();
}

//Heavy Exception Methods
heavyException::heavyException(const std::string& str) {
	message = str;
}

const char* heavyException::what() const throw() {
	return message.c_str();
}

//Old GUI Handling Class Methods
InterfaceScreen::InterfaceScreen(HWND windowHandle) {
	interfaceHwnd = windowHandle;
	initialize();
}

InterfaceScreen::~InterfaceScreen() {
	delete[] lpbitmap;
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(interfaceHwnd, hdcWindow);
}

void InterfaceScreen::initialize() {
	hdcMemDC = NULL;
	hbmScreen = NULL;

	hdcWindow = GetDC(interfaceHwnd);
	hdcMemDC = CreateCompatibleDC(hdcWindow);

	GetClientRect(interfaceHwnd, &rcClient);
	width = rcClient.right - rcClient.left;
	height = rcClient.bottom - rcClient.top;

	hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
	SelectObject(hdcMemDC, hbmScreen);
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
	lpbitmap = new char[dwBmpSize];

	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfType = 0x4D42;
}

void InterfaceScreen::takeScreenshot() {
	BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
	GetDIBits(hdcWindow, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)& bi, DIB_RGB_COLORS);
}

char* InterfaceScreen::getFileHeader() {
	return (char*)& bmfHeader;
}

char* InterfaceScreen::getInfoHeader() {
	return (char*)& bi;
}

char* InterfaceScreen::getBitmapArray() {
	return lpbitmap;
}

unsigned long int InterfaceScreen::getBitmapSize() {
	return dwBmpSize;
}

//Miscellaneous
std::ostream& par(std::ostream& out) {
	return out << "\n\n" << std::flush;
}

std::chrono::steady_clock::time_point printTimestamp(std::chrono::steady_clock::time_point start, const char* moment) {
	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	cout << "[Timestamp] " << moment << ":" << std::chrono::duration_cast<std::chrono::microseconds>(timestamp - start).count() << endl;
	return std::chrono::steady_clock::now();
}