/*
 * This is an example that demonstrates how to connect to the EyeX Engine and subscribe to the fixation data stream.
 *
 * Copyright 2013-2014 Tobii Technology AB. All rights reserved.
 */
#include "stdafx.h"

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "eyex/EyeX.h"

#include "FixationDataClass.hpp"

#pragma comment (lib, "Tobii.EyeX.Client.lib")


// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "Rainbow Dash";

// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;

FDC *fdcObj1;

/*
 * Initializes g_hGlobalInteractorSnapshot with an interactor that has the Fixation Data behavior.
 */
BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
	fdcObj1 = FDC::Instance();	// Initializing FixationDataClass object.
	fdcObj1->eyeTrackingStart = std::chrono::steady_clock::now();

	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_FIXATIONDATAPARAMS params = { TX_FIXATIONDATAMODE_SENSITIVE };
	BOOL success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		InteractorId,
		&g_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateFixationDataBehavior(hInteractor, &params) == TX_RESULT_OK;

	txReleaseObject(&hInteractor);

	return success;
}

/*
 * Callback function invoked when a snapshot has been committed.
 */
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	assert(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}

/*
 * Callback function invoked when the status of the connection to the EyeX Engine has changed.
 */
void TX_CALLCONVENTION OnEngineConnectionStateChanged2(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	switch (connectionState) {
	case TX_CONNECTIONSTATE_CONNECTED: {
			BOOL success;
			printf("The connection state is now CONNECTED (We are connected to the EyeX Engine)\n");
			// commit the snapshot with the global interactor as soon as the connection to the engine is established.
			// (it cannot be done earlier because committing means "send to the engine".)
			success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
			if (!success) {
				printf("Failed to initialize the data stream.\n");
			}
			else
			{
				printf("Waiting for fixation data to start streaming...\n");
			}
		}
		break;

	case TX_CONNECTIONSTATE_DISCONNECTED:
		printf("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
		printf("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
		printf("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		printf("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.\n");
		break;
	}
}

/*
 * Handles an event from the fixation data stream.
 */

#define Get_Fixation_Count 1
int timeInMS = 0;
bool fixationBegins = true;
float preX = 0.0, preY = 0.0;
int smallestFixation = INT_MAX, LongestFixation = INT_MIN; // in seconds
float xSF = FLT_MAX, ySF = FLT_MAX, xLF = FLT_MIN, yLF = FLT_MIN;
float biggestX = 0, biggestY = 0;

static string id = "", afterThis = "0_0";
static int startTime = 0, fixationDuration = 0;
static float xSum = 0.0, ySum = 0.0;
static bool firstFixationValue = true;

void OnFixationDataEvent(TX_HANDLE hFixationDataBehavior)
{
	TX_FIXATIONDATAEVENTPARAMS eventParams;
	TX_FIXATIONDATAEVENTTYPE eventType;
	std::string eventDescription;

	if (txGetFixationDataEventParams(hFixationDataBehavior, &eventParams) == TX_RESULT_OK) {
		eventType = eventParams.EventType;

		eventDescription = (eventType == TX_FIXATIONDATAEVENTTYPE_DATA) ? "Data"
			: ((eventType == TX_FIXATIONDATAEVENTTYPE_END) ? "End"
				: "Begin");

		fixationBegins = (eventDescription == "Begin") ? true : fixationBegins;
		if (eventDescription == "End" && (eventParams.X < preX - 5 || eventParams.X > preX + 5 || eventParams.Y < preY - 5 || eventParams.Y > preY + 5))
		{
			fixationBegins = false;
		}
		if (fixationBegins)
		{
			if (firstFixationValue)
			{
				startTime = eventParams.Timestamp;
				firstFixationValue = false;
			}
			xSum += std::isnan(eventParams.X) ? 0.0 : eventParams.X;
			ySum += std::isnan(eventParams.Y) ? 0.0 : eventParams.Y;

			timeInMS += 1;	// it is actually counter that count timestams of a fixation, it may be different from eventparam.Timestamp because frequency of Tobii is 70Hz 
			//printf("Fixation at X: %f, \tY: %f \t For: %f sec\n", eventParams.X, eventParams.Y, ((float)timeInMS / 1000));
			/*if (eventParams.X < xSF)
				xSF = eventParams.X;
			if (eventParams.X > xLF)
				xLF = eventParams.X;
			if (eventParams.Y < ySF)
				ySF = eventParams.Y;
			if (eventParams.Y > yLF)
				yLF = eventParams.Y;*/
		}
		else
		{
			//timeInMS			= eventParams.Timestamp - startTime;
			fixationDuration = eventParams.Timestamp - startTime; //timeInMS;
			float xAvg = (xSum / timeInMS);
			float yAvg = (ySum / timeInMS);
			id = to_string((int)xAvg) + "_" + to_string((int)yAvg);
			afterThis = to_string((int)eventParams.X) + "_" + to_string((int)eventParams.Y);

			if (xAvg >= 0 && xAvg <= SCREEN_WIDTH && yAvg >= 0 && yAvg <= SCREEN_HEIGHT) {
				fdcObj1->updateFixationMap(id, startTime, fixationDuration, afterThis);
			}

			firstFixationValue = true;
			timeInMS = 0;
			xSum = 0;
			ySum = 0;

			//if (xSF != FLT_MAX && ySF != FLT_MAX && xLF != FLT_MIN && yLF != FLT_MIN)
			//{
			//	float cx = xLF - xSF;
			//	float cy = yLF - ySF;
			//	if (biggestX < cx)
			//	{
			//		biggestX = cx;
			//		printf("In one Fixation --- Biggest-X: %0.2f\t Smallest-X: %0.2f\t PreX: %0.2f\t Difference-X: %0.2f\t\n", xLF, xSF, preX, biggestX);
			//	}
			//	if (biggestY < cy)
			//	{
			//		biggestY = cy;
			//		printf("In one Fixation --- Biggest-Y: %0.2f\t Smallest-Y: %0.2f\t PreY: %0.2f\t Difference-Y: %0.2f\n", yLF, ySF, preY, biggestY);
			//	}

			//	//printf("Bx: %f\t By: %f\n", biggestX, biggestY);
			//	//printf("xSF: %0.2f\t ySF: %0.2f\t xLF: %0.2f\t yLF: %0.2f\t Bx: %0.2f\t By: %0.2f\n", xSF, ySF, xLF, yLF, biggestX, biggestY);
			//}
			//xSF = FLT_MAX, ySF = FLT_MAX, xLF = FLT_MIN, yLF = FLT_MIN;
		}
		preX = eventParams.X;
		preY = eventParams.Y;

		CURRENT_X = eventParams.X;
		CURRENT_Y = eventParams.Y;

	}
	else {
		printf("Failed to interpret fixation data event packet.\n");
	}
}

/*
 * Callback function invoked when an event has been received from the EyeX Engine.
 */
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

	txGetAsyncDataContent(hAsyncData, &hEvent);

	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));

	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_FIXATIONDATA) == TX_RESULT_OK) {
		OnFixationDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}

	// NOTE since this is a very simple application with a single interactor and a single data stream, 
	// our event handling code can be very simple too. A more complex application would typically have to 
	// check for multiple behaviors and route events based on interactor IDs.
	
	txReleaseObject(&hEvent);
}