

#include <fstream>
#include "bitalino.h"

class DTF
{
public:
	std::ofstream outfile;

	void DTF::init(char* file, bool bitalion)
	{
		outfile.open(file);
		if (outfile.is_open() && bitalion)
			outfile << "#Seq;analog0;analog1;analog2;analog3;analog4;analog5;digital0;digital1;digital2;digital3\n";
	}

	void DTF::writeData(char* data)
	{
		if (outfile.is_open())
			outfile << data;
	}

	void DTF::fin()
	{
		outfile.close();
	}

	void DTF::writeBitalinoData(BITalino::VFrame frames)
	{
		for (int i = 0; i < frames.size(); i++)
		{
			char str[1000];
	
			sprintf(str, "%d;%hi;%hi;%hi;%hi;%hi;%hi;%d;%d;%d;%d\n", frames[i].seq, frames[i].analog[0], frames[i].analog[1],
				frames[i].analog[2], frames[i].analog[3], frames[i].analog[4], frames[i].analog[5],
				frames[i].digital[0], frames[i].digital[1], frames[i].digital[2], frames[i].digital[3]);
	
			if (outfile.is_open())
				outfile << str;
		}
	}
};