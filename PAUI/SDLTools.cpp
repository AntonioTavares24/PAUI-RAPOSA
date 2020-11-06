#include "SDLTools.h"

//Texture Class Methods
LTexture::LTexture() {
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture() {
	freeTexture();
}

/*bool LTexture::loadFromMemory(char* bitmapfinal, int size) {
	freeTexture();

	SDL_Texture* newTexture = NULL;
	SDL_Surface* optimizedSurface = NULL;
	SDL_RWops* rw = SDL_RWFromMem(bitmapfinal, size);
	SDL_Surface* loadedSurface = SDL_LoadBMP_RW(rw, 1);
	if (loadedSurface == NULL) {
		cout << "Image loading failed. SDL_image error: " << IMG_GetError() << endl;
	}
	else {
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL) {
			cout << "Surface conversion failed. SDL error: " << SDL_GetError() << endl;
		}
		else {
			newTexture = SDL_CreateTextureFromSurface(gRenderer, optimizedSurface);
			if (newTexture == NULL) {
				cout << "Texture creation failed. SDL error: " << SDL_GetError() << endl;
			}
			else {
				mWidth = loadedSurface->w;
				mHeight = loadedSurface->h;
			}
			SDL_FreeSurface(optimizedSurface);
		}

		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != NULL;
}*/

bool LTexture::loadFromMemory(char* bitmapfinal, int size) {
	freeTexture();

	SDL_Texture* newTexture = NULL;
	SDL_Surface* optimizedSurface = NULL;
	SDL_RWops* rw = SDL_RWFromMem(bitmapfinal, size);
	SDL_Surface* loadedSurface = SDL_LoadBMP_RW(rw, 1);

	if (loadedSurface == NULL) {
		cout << "Image loading failed. SDL_image error: " << IMG_GetError() << endl;
	}
	else {
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_GetWindowPixelFormat(gWindow), 0);
		if (formattedSurface == NULL) {
			printf("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
		}
		else {
			newTexture = SDL_CreateTexture(gRenderer, SDL_GetWindowPixelFormat(gWindow), SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
			if (newTexture == NULL) {
				printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
			}
			else {
				gTexture.setBlendMode(SDL_BLENDMODE_BLEND);
				SDL_LockTexture(newTexture, NULL, &mPixels, &mPitch);

				memcpy(mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

				SDL_UnlockTexture(newTexture);
				mPixels = NULL;

				mWidth = formattedSurface->w;
				mHeight = formattedSurface->h;
			}

			SDL_FreeSurface(formattedSurface);
		}

		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromFile(std::string path, bool setColorKey) {
	freeTexture();

	SDL_Texture* newTexture = NULL;
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL) {
		cout << "Image loading failed. SDL_image error: " << IMG_GetError() << endl;
	}
	else {
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL) {
			cout << "Surface conversion failed. SDL error: " << SDL_GetError() << endl;
		}
		else {
			if (setColorKey) {
				SDL_SetColorKey(optimizedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
			}

			newTexture = SDL_CreateTextureFromSurface(gRenderer, optimizedSurface);
			if (newTexture == NULL) {
				cout << "Texture creation failed. SDL error: " << SDL_GetError() << endl;
			}
			else {
				mWidth = loadedSurface->w;
				mHeight = loadedSurface->h;
			}
			SDL_FreeSurface(optimizedSurface);
		}

		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::freeTexture() {
	if (mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(int x, int y, SDL_Rect* clip, float scale) {
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	if (clip != NULL) {
		renderQuad.w = clip->w * scale;
		renderQuad.h = clip->h * scale;
	}

	SDL_RenderCopy(gRenderer, mTexture, clip, &renderQuad);
}

void LTexture::renderResized(SDL_Rect* renderQuad) {
	SDL_RenderCopy(gRenderer, mTexture, NULL, renderQuad);
}

void LTexture::renderTo(int x, int y, SDL_Rect* clip, SDL_Rect* destQuad) {
	SDL_RenderCopy(gRenderer, mTexture, clip, destQuad);
}

void LTexture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha) {
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

bool LTexture::lockTexture()
{
	bool success = true;

	if (mPixels != NULL)
	{
		printf("Texture is already locked!\n");
		success = false;
	}

	else
	{
		if (SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0)
		{
			printf("Unable to lock texture! %s\n", SDL_GetError());
			success = false;
		}
	}

	return success;
}

bool LTexture::unlockTexture()
{
	bool success = true;

	if (mPixels == NULL)
	{
		printf("Texture is not locked!\n");
		success = false;
	}

	else
	{
		SDL_UnlockTexture(mTexture);
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* LTexture::getPixels()
{
	return mPixels;
}

int LTexture::getPitch()
{
	return mPitch;
}

int LTexture::getWidth() {
	return mWidth;
}

int LTexture::getHeight() {
	return mHeight;
}

//Button Class Methods
LButton::LButton() {
	mPosition.x = 0;
	mPosition.y = 0;
	mWidth = 0;
	mHeight = 0;
}

void LButton::setPosition(int x, int y, int width, int height) {
	mPosition.x = x;
	mPosition.y = y;
	mWidth = width;
	mHeight = height;
}

bool LButton::handleEvent(SDL_Event* e) {
	if (e->type == SDL_MOUSEBUTTONDOWN) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		bool mouseIn = true;

		if (x < mPosition.x) {
			mouseIn = false;
		}
		else if (x > mPosition.x + mWidth) {
			mouseIn = false;
		}
		else if (y < mPosition.y) {
			mouseIn = false;
		}
		else if (y > mPosition.y + mHeight) {
			mouseIn = false;
		}

		return mouseIn;
	}

	return false;
}

//SDL Functions
bool initSDL() {
	bool success = true;
	cout << "Initializing SDL..." << endl;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cout << "SDL initialization failed. SDL error: " << SDL_GetError() << endl;;
		success = false;
	}
	else {
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
			cout << "WARNING: Linear texture filtering not enabled." << endl;
		}

		//gWindow = SDL_CreateWindow("PAUI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_SCREEN_WIDTH, SDL_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		gWindow = SDL_CreateWindow("PAUI", 540, 30, SDL_SCREEN_WIDTH, SDL_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			cout << "SDL window creation failed. SDL error: " << SDL_GetError() << endl;
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				cout << "SDL renderer creation failed. SDL error: " << SDL_GetError() << endl;
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 0x71, 0x6F, 0x64, 0xFF);
				
				int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					cout << "SDL_image initialization failed. SDL_image error: " << IMG_GetError() << endl;
					success = false;
				}
				else {
					gScreenSurface = SDL_GetWindowSurface(gWindow);

					//Button Positions
					gButtons[0].setPosition(0, 0, OPERATION_WIDTH, BUTTON_HEIGHT);													//Operation
					gButtons[1].setPosition(OPERATION_WIDTH + 1, 0, SETUP_WIDTH, BUTTON_HEIGHT);									//Setup
					gButtons[2].setPosition(OPERATION_WIDTH + SETUP_WIDTH + 1, 0, SETUPSEN_WIDTH, BUTTON_HEIGHT);					//Setup Sensors
					gButtons[3].setPosition(OPERATION_WIDTH + SETUP_WIDTH + SETUPSEN_WIDTH + 1, 0, DEBUG_WIDTH, BUTTON_HEIGHT);		//Debug		
					gToggleClassButton.setPosition(916, 33, TOGGLE_CLASS_WIDTH, TOGGLE_CLASS_HEIGHT);								//Toggle Classification
					
					//Original Sprites

					//Frame
					gOriginalSprites[0].x = 6;
					gOriginalSprites[0].y = 24;
					gOriginalSprites[0].w = 1018;
					gOriginalSprites[0].h = 736;

					//Camera Numbers
					gOriginalSprites[1].x = 445;
					gOriginalSprites[1].y = 23;
					gOriginalSprites[1].w = 121;
					gOriginalSprites[1].h = 33;

					//Pitch
					gOriginalSprites[2].x = 13;
					gOriginalSprites[2].y = 79;
					gOriginalSprites[2].w = 137;
					gOriginalSprites[2].h = 137;

					//Roll
					gOriginalSprites[3].x = 13;
					gOriginalSprites[3].y = 231;
					gOriginalSprites[3].w = 137;
					gOriginalSprites[3].h = 137;

					//Lights
					gOriginalSprites[4].x = 869;
					gOriginalSprites[4].y = 79;
					gOriginalSprites[4].w = 137;
					gOriginalSprites[4].h = 137;

					//Arm
					gOriginalSprites[5].x = 869;
					gOriginalSprites[5].y = 231;
					gOriginalSprites[5].w = 137;
					gOriginalSprites[5].h = 137;

					//Motors
					gOriginalSprites[6].x = 157;
					gOriginalSprites[6].y = 376;
					gOriginalSprites[6].w = 169;
					gOriginalSprites[6].h = 353;

					//Power
					gOriginalSprites[7].x = 13;
					gOriginalSprites[7].y = 376;
					gOriginalSprites[7].w = 137;
					gOriginalSprites[7].h = 232;

					//Docking
					gOriginalSprites[8].x = 13;
					gOriginalSprites[8].y = 616;
					gOriginalSprites[8].w = 137; 
					gOriginalSprites[8].h = 112;

					//Sensors
					gOriginalSprites[9].x = 709;
					gOriginalSprites[9].y = 376;
					gOriginalSprites[9].w = 297;
					gOriginalSprites[9].h = 352;

					//Camera Buttons
					gOriginalSprites[10].x = 341;
					gOriginalSprites[10].y = 679;
					gOriginalSprites[10].w = 353;
					gOriginalSprites[10].h = 41;

					//Camera Sliders
					gOriginalSprites[11].x = 162;
					gOriginalSprites[11].y = 87;
					gOriginalSprites[11].w = 697;
					gOriginalSprites[11].h = 15;

					//Front Camera Left
					gOriginalSprites[12].x = 161;
					gOriginalSprites[12].y = 102;
					gOriginalSprites[12].w = 347;
					gOriginalSprites[12].h = 269;

					//Front Camera Right
					gOriginalSprites[13].x = 513;
					gOriginalSprites[13].y = 102;
					gOriginalSprites[13].w = 347;
					gOriginalSprites[13].h = 269;

					//Rear Camera
					gOriginalSprites[14].x = 340;
					gOriginalSprites[14].y = 380;
					gOriginalSprites[14].w = 355;
					gOriginalSprites[14].h = 295;

					//Victim/Object Counter
					gOriginalSprites[15].x = 427;
					gOriginalSprites[15].y = 687;
					gOriginalSprites[15].w = 160;
					gOriginalSprites[15].h = 40;

					//Timer
					gOriginalSprites[16].x = 442;
					gOriginalSprites[16].y = 36;
					gOriginalSprites[16].w = 130;
					gOriginalSprites[16].h = 55;

					//Detailed Sprites

					//Camera Numbers Left
					gDetailedSprites[0].x = 445;
					gDetailedSprites[0].y = 23;
					gDetailedSprites[0].w = 25;
					gDetailedSprites[0].h = 33;

					//Camera Numbers Middle
					gDetailedSprites[1].x = 493;
					gDetailedSprites[1].y = 23;
					gDetailedSprites[1].w = 25;
					gDetailedSprites[1].h = 33;

					//Camera Numbers Right
					gDetailedSprites[2].x = 541;
					gDetailedSprites[2].y = 23;
					gDetailedSprites[2].w = 25;
					gDetailedSprites[2].h = 33;

					//Power PC
					gDetailedSprites[3].x = 21;
					gDetailedSprites[3].y = 391;
					gDetailedSprites[3].w = 49;
					gDetailedSprites[3].h = 129;

					//Power Motor
					gDetailedSprites[4].x = 93;
					gDetailedSprites[4].y = 391;
					gDetailedSprites[4].w = 49;
					gDetailedSprites[4].h = 129;

					//Angular Velocity
					gDetailedSprites[5].x = 173;
					gDetailedSprites[5].y = 391;
					gDetailedSprites[5].w = 137;
					gDetailedSprites[5].h = 97;

					//Linear Velocity
					gDetailedSprites[6].x = 173;
					gDetailedSprites[6].y = 495;
					gDetailedSprites[6].w = 137;
					gDetailedSprites[6].h = 113;

					//Temperature
					gDetailedSprites[7].x = 789;
					gDetailedSprites[7].y = 391;
					gDetailedSprites[7].w = 65;
					gDetailedSprites[7].h = 161;

					//Humidity
					gDetailedSprites[8].x = 861;
					gDetailedSprites[8].y = 391;
					gDetailedSprites[8].w = 65;
					gDetailedSprites[8].h = 161;

					//Camera Simulation Sprites

					//Front Camera Left (Original Interface)
					gCamSimulationSprites[0].x = 163;
					gCamSimulationSprites[0].y = 104;
					gCamSimulationSprites[0].w = 343;
					gCamSimulationSprites[0].h = 265;

					//Front Camera Right (Original Interface)
					gCamSimulationSprites[1].x = 515;
					gCamSimulationSprites[1].y = 104;
					gCamSimulationSprites[1].w = 343;
					gCamSimulationSprites[1].h = 265;

					//Rear Camera (Original Interface)
					gCamSimulationSprites[2].x = 342;
					gCamSimulationSprites[2].y = 382;
					gCamSimulationSprites[2].w = 351;
					gCamSimulationSprites[2].h = 291;

					//Front Camera (Vehicle Interface)
					gCamSimulationSprites[3].x = 131;
					gCamSimulationSprites[3].y = 64;
					gCamSimulationSprites[3].w = 768;
					gCamSimulationSprites[3].h = 640;

					//Rear Camera (Vehicle Interface)
					gCamSimulationSprites[4].x = 455;
					gCamSimulationSprites[4].y = 80;
					gCamSimulationSprites[4].w = 120;
					gCamSimulationSprites[4].h = 80;

					//Rear Camera (Increased Size)
					gCamSimulationSprites[5].x = 335;
					gCamSimulationSprites[5].y = 80;
					gCamSimulationSprites[5].w = 360;
					gCamSimulationSprites[5].h = 240;

					//Sensor Symbol Sprites

					//Temperature
					gSensorSymbolSprites[0].x = 910;
					gSensorSymbolSprites[0].y = 695;
					gSensorSymbolSprites[0].w = 45;
					gSensorSymbolSprites[0].h = 60;

					//Humidity
					gSensorSymbolSprites[1].x = 965;
					gSensorSymbolSprites[1].y = 700;
					gSensorSymbolSprites[1].w = 50;
					gSensorSymbolSprites[1].h = 50;
				}
			}
		}
	}

	return success;
}

/*bool loadScreenshot(char* bitmapfinal, int size) {
	bool success = true;

	if (!gTexture.loadFromMemory(bitmapfinal, size)) {
		success = false;
	}
	else {
		gTexture.setBlendMode(SDL_BLENDMODE_BLEND);
	}

	return success;
}*/

bool loadScreenshot(char* bitmapfinal, int size) {
	bool success = true;

	if (!gTexture.loadFromMemory(bitmapfinal, size)) {
		success = false;
	}
	else {
		gTexture.setBlendMode(SDL_BLENDMODE_BLEND);
		if (!gTexture.lockTexture()) {
			printf("Unable to lock texture!\n");
		}
		else {
			Uint32 format = SDL_GetWindowPixelFormat(gWindow);
			SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

			Uint32* pixels = (Uint32*)gTexture.getPixels();
			int pixelCount = (gTexture.getPitch() / 4) * gTexture.getHeight();

			Uint32 powerLow = SDL_MapRGB(mappingFormat, 123, 65, 0);
			Uint32 tempHigh = SDL_MapRGB(mappingFormat, 253, 0, 0);
			
			if (pixels[462 * gTexture.getWidth() + 46] == powerLow || pixels[463 * gTexture.getWidth() + 118] == powerLow) {
				gTexture.togglePower = true;
			}
			else {
				gTexture.togglePower = false;
			}

			if (pixels[483 * gTexture.getWidth() + 800] == tempHigh) {
				gTexture.toggleSensor = true;
			}
			else {
				gTexture.toggleSensor = false;
			}

			gTexture.unlockTexture();
			SDL_FreeFormat(mappingFormat);
		}
	}

	return success;
}

bool loadScreenshotFromFile(char* path) {
	bool success = true;

	if (!gTexture.loadFromFile(path, false)) {
		success = false;
	}

	return success;
}

bool loadFileImage(LTexture& fileImage, std::string path, bool setColorKey) {
	bool success = true;

	if (!fileImage.loadFromFile(path, setColorKey)) {
		success = false;
	}
	else {
		fileImage.setBlendMode(SDL_BLENDMODE_BLEND);
	}

	return success;
}

bool loadMedia() {
	bool success = true;

	if (!loadFileImage(cameraFrontImage, "textures/RaposaCameraFront.png")) {
		success = false;
	}

	if (!loadFileImage(cameraRearImage, "textures/RaposaCameraRear.png")) {
		success = false;
	}

	if (!loadFileImage(sensorTemp, "textures/SensorTemp.png", true)) {
		success = false;
	}
	else {
		sensorTemp.setAlpha(180);
	}

	if (!loadFileImage(sensorHum, "textures/SensorHum.png", true)) {
		success = false;
	}
	else {
		sensorHum.setAlpha(180);
	}

	if (!loadFileImage(classRedButton, "textures/ClassRedButton.png", true)) {
		success = false;
	}
	else {
		classRedButton.setAlpha(180);
	}

	if (!loadFileImage(classGreenButton, "textures/ClassGreenButton.png", true)) {
		success = false;
	}

	else {
		classRedButton.setAlpha(180);
	}

	return success;
}

void renderOriginalInterface(bool classification) {
	//Frame
	gTexture.render(0, 0);
	SDL_RenderFillRect(gRenderer, &gOriginalSprites[0]);

	//Original Interface elements
	//gTexture.render(454, 26, &gOriginalSprites[1]);
	gTexture.render(13, 79, &gOriginalSprites[2]);
	gTexture.render(13, 231, &gOriginalSprites[3]);
	gTexture.render(869, 79, &gOriginalSprites[4]);
	gTexture.render(869, 231, &gOriginalSprites[5]);
	gTexture.render(157, 376, &gOriginalSprites[6]);
	gTexture.render(13, 376, &gOriginalSprites[7]);
	gTexture.render(13, 616, &gOriginalSprites[8]);
	gTexture.render(709, 376, &gOriginalSprites[9]);
	//gTexture.render(341, 679, &gOriginalSprites[10]);
	//gTexture.render(162, 87, &gOriginalSprites[11]);
	gTexture.render(427, 687, &gOriginalSprites[15]);
	gTexture.render(442, 36, &gOriginalSprites[16]);

	/*//Simulated Camera Images
	cameraFrontImage.renderResized(&gCamSimulationSprites[0]);
	cameraFrontImage.renderResized(&gCamSimulationSprites[1]);
	cameraRearImage.renderResized(&gCamSimulationSprites[2]);

	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[0]);
	SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[1]);
	SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[2]);
	SDL_SetRenderDrawColor(gRenderer, 0x71, 0x6F, 0x64, 0xFF);*/

	gTexture.render(161, 102, &gOriginalSprites[12]);
	gTexture.render(513, 102, &gOriginalSprites[13]);
	gTexture.render(340, 380, &gOriginalSprites[14]);

	//Enable/Disable Classification Button
	if (classification) {
		classGreenButton.render(914, 30);
	}
	else {
		classRedButton.render(914, 30);
	}
}

void renderVehicleInterface(bool classification) {
	//Frame
	gTexture.render(0, 0);
	SDL_RenderFillRect(gRenderer, &gOriginalSprites[0]);

	//Camera Numbers, Power and Sensors
	gTexture.render(454, 26, &gDetailedSprites[0]);
	gTexture.render(502, 26, &gDetailedSprites[1]);
	gTexture.render(550, 26, &gDetailedSprites[2]);

	if (gTexture.togglePower) {
		gTexture.render(13, 625, &gDetailedSprites[3]);
		gTexture.render(68, 625, &gDetailedSprites[4]);
	}

	if (gTexture.toggleSensor) {
		gTexture.render(906, 624, &gDetailedSprites[7], 0.8f);
		gTexture.render(964, 624, &gDetailedSprites[8], 0.8f);
	}

	//Attitude, Lights and Arm
	gTexture.render(13, 70, &gOriginalSprites[2], 0.8f);
	gTexture.render(13, 186, &gOriginalSprites[3], 0.8f);
	gTexture.render(908, 70, &gOriginalSprites[4], 0.8f);
	gTexture.render(908, 186, &gOriginalSprites[5], 0.8f);

	//Simulated Camera Images
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
	cameraFrontImage.renderResized(&gCamSimulationSprites[3]);
	SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[3]);
	//if (!reverse) {
		cameraRearImage.renderResized(&gCamSimulationSprites[4]);
		SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[4]);
	//}
	/*else {
		cameraRearImage.renderResized(&gCamSimulationSprites[5]);
		SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[5]);
	}*/
	
	SDL_SetRenderDrawColor(gRenderer, 0x71, 0x6F, 0x64, 0xFF);

	//Enable/Disable Classification Button
	if (classification) {
		classGreenButton.render(914, 30);
	}
	else {
		classRedButton.render(914, 30);
	}
}

void renderGameInterface(bool classification) {
	//Frame
	gTexture.setAlpha(255);
	gTexture.render(0, 0);
	//cameraFrontImage.renderResized(&gOriginalSprites[0]);
	gTexture.renderTo(6, 24, &gCamSimulationSprites[0], &gOriginalSprites[0]);

	//Victim/Object Counter
	//gTexture.render(427, 709, &gOriginalSprites[15]);
	
	//Timer
	//gTexture.render(470, 36, &gOriginalSprites[16], 0.7f);
	/*SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
	if (!reverse) {
		cameraRearImage.renderResized(&gCamSimulationSprites[4]);
		SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[4]);
	}
	else {
		cameraRearImage.renderResized(&gCamSimulationSprites[5]);
		SDL_RenderDrawRect(gRenderer, &gCamSimulationSprites[5]);
	}

	SDL_SetRenderDrawColor(gRenderer, 0x71, 0x6F, 0x64, 0xFF);*/

	//if (!reverse) {
		gTexture.renderTo(455, 80, &gOriginalSprites[14], &gCamSimulationSprites[4]);
	//}
	/*else {
		gTexture.renderTo(335, 80, &gOriginalSprites[14], &gCamSimulationSprites[5]);
	}*/

	//Camera Numbers, Power, Linear Velocity and Sensors
	gTexture.setAlpha(255);
	//gTexture.render(454, 26, &gDetailedSprites[0]);
	//gTexture.render(502, 26, &gDetailedSprites[1]);
	//gTexture.render(550, 26, &gDetailedSprites[2]);

	if (gTexture.togglePower) {
		gTexture.render(13, 664, &gDetailedSprites[3], 0.7f);
		gTexture.render(54, 664, &gDetailedSprites[4], 0.7f);
	}
	
	//gTexture.render(98, 664, &gDetailedSprites[6], 0.8f); 

	if (gTexture.toggleSensor) {
		gTexture.render(971, 640, &gDetailedSprites[7], 0.7f);	//original x - 919
		//gTexture.render(971, 640, &gDetailedSprites[8], 0.7f);
		//sensorTemp.renderResized(&gSensorSymbolSprites[0]);
		//sensorHum.renderResized(&gSensorSymbolSprites[1]);
	}

	//Attitude, Lights and Arm
	gTexture.setAlpha(153);
	gTexture.render(16, 70, &gOriginalSprites[2], 0.7f);
	gTexture.render(16, 172, &gOriginalSprites[3], 0.7f);
	gTexture.render(918, 70, &gOriginalSprites[4], 0.7f);
	gTexture.render(918, 172, &gOriginalSprites[5], 0.7f);

	//Enable/Disable Classification Button
	if (classification) {
		classGreenButton.render(914, 30);
	}
	else {
		classRedButton.render(914, 30);
	}
}

void renderInterface(int emotionalState, bool classification, int tabNumber) {
	SDL_RenderClear(gRenderer);

	if (tabNumber == 0) {
		if (emotionalState == 2 && classification) {
			//renderVehicleInterface(classification);
			renderGameInterface(classification);
		}
		else {
			renderOriginalInterface(classification);
		}
	}
	else {
		gTexture.render(0, 0);
	}

	SDL_RenderPresent(gRenderer);
}

void closeSDL() {
	cout << "Closing SDL..." << endl;

	gTexture.freeTexture();
	cameraFrontImage.freeTexture();
	cameraRearImage.freeTexture();
	sensorTemp.freeTexture();
	sensorHum.freeTexture();
	classRedButton.freeTexture();
	classGreenButton.freeTexture();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}