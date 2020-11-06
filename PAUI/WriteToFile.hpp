#ifndef WRITETOFILE_HPP
#define WRITETOFILE_HPP


//#include <iostream>
#include <fstream>
#include <string>

class DTF
{
private:
	std::ofstream outfile;

public:

	void DTF::writeData(char* data, char* file)
	{
		if (outfile.is_open())
			outfile << data;
		else
		{
			outfile.open(file);
			outfile << data;
		}
	}
	void DTF::writeData(std::string data, std::string file)
	{
		if (outfile.is_open())
			outfile << data.c_str();
		else
		{
			outfile.open(file.c_str());
			outfile << data.c_str();
		}
	}

	void DTF::fin()
	{
		outfile.close();
	}

};

#endif // !WRITETOFILE_HPP