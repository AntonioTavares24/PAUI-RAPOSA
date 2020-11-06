#ifndef SDLTOOLS_H
#define SDLTOOLS_H

#include "stdafx.h"
#include <string>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_syswm.h>

using std::cout;
using std::endl;

//Screen Constants
const int SDL_SCREEN_WIDTH = 1030;
const int SDL_SCREEN_HEIGHT = 768;

//Button Constants
const int OPERATION_WIDTH = 60;
const int SETUP_WIDTH = 40;
const int SETUPSEN_WIDTH = 82;
const int DEBUG_WIDTH = 44;
const int BUTTON_HEIGHT = 22;
const int TOGGLE_CLASS_WIDTH = 98;
const int TOGGLE_CLASS_HEIGHT = 25;

//SDL Classes
class LTexture {
public:
	LTexture();
	~LTexture();
	bool loadFromMemory(char* bitmapfinal, int size);
	bool loadFromFile(std::string path, bool setColorKey);
	void freeTexture();
	void render(int x, int y, SDL_Rect* clip = NULL, float scale = 1);
	void renderResized(SDL_Rect* renderQuad);
	void renderTo(int x, int y, SDL_Rect* clip = NULL, SDL_Rect* destQuad = NULL);
	void setBlendMode(SDL_BlendMode blending);
	void setAlpha(Uint8 alpha);

	bool lockTexture();
	bool unlockTexture();
	void* getPixels();
	int getPitch();
	int getWidth();
	int getHeight();

	bool togglePower = false;
	bool toggleSensor = false;

private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;

	void* mPixels;
	int mPitch;
};

class LButton {
public:
	LButton();
	void setPosition(int x, int y, int width, int height);
	bool handleEvent(SDL_Event* e);

private:
	SDL_Point mPosition;
	int mWidth;
	int mHeight;
};

//SDL Global Handles
extern SDL_Window* gWindow;
extern SDL_Surface* gScreenSurface;
extern SDL_Renderer* gRenderer;

//SDL Global Texture and Buttons
extern LButton gButtons[TOTAL_BUTTONS];
extern LButton gToggleClassButton;
extern SDL_Rect gOriginalSprites[17];
extern SDL_Rect gDetailedSprites[9];
extern SDL_Rect gCamSimulationSprites[6];
extern SDL_Rect gSensorSymbolSprites[2];

extern LTexture gTexture;
extern LTexture cameraFrontImage;
extern LTexture cameraRearImage;
extern LTexture sensorTemp;
extern LTexture sensorHum;
extern LTexture classRedButton;
extern LTexture classGreenButton;

//SDL Functions
bool initSDL();
bool loadScreenshot(char* bitmapfinal, int size);
bool loadScreenshotFromFile(char* path);
bool loadFileImage(LTexture& fileImage, std::string path, bool setColorKey = false);
bool loadMedia();
void renderOriginalInterface(bool classification);
void renderVehicleInterface(bool classification);
void renderGameInterface(bool classification);
void renderInterface(int emotionalState, bool classification, int tabNumber);
void closeSDL();

#endif