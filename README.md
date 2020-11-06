
## PAUI

Biggest problem with robot tele-operation is too many screens with too much data, and the person supposed to keep track of all of this along with driving the robot and performing the tasks. That makes operators spend most of their effort and mental energy in dealing with driving related stuff rather performing other important operations. This package is designed to continuously monitor operators Physiological (EMG, ECG, EDA, and EMG), Eye moments (Tobii), and facial emotions and expressions. And then use them to classify resting, stressed, and workload state of the person. And finally to make changes in the User Interface to decrease workload and stress. 

## Installation 

* [AfdexSDK 32bit](https://developer.affectiva.com/) installed at C:\Program Files (x86)\Affectiva\AffdexSDK\include    
* [OpenCV 2.4.13](https://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.13/)
* [boost 1.63.0](http://www.boost.org/users/download/#live)
* DLLs and other libraries are present along the project, You need to provide dll location in system path is you use version other than mentioned above. 

After installing all three, you need to change the paths in 
	* Include Locations: Properties > C/C++ > General > Additional Include Directories
	* Library Locations: Properties > Linker > General > Additional Library Directories
	
## Running Instructions 

All of the global variables and macros are provided in stdafx.h file and shared with other sub modules in the project. 
Shift between 1/0 to run or to restrict any module. 

## Disclaimer

PAUI is in active use at M-ITI and ISR (Portugal) and constantly evolving. There could be bugs.