
#ifndef NND_H
#define NND_H

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

using std::cout;
using std::endl;

typedef std::vector<std::vector<float>> csvMatrix;

csvMatrix csv2matrix(const char* path) {
	std::ifstream  data(path);
	std::string line;
	csvMatrix parsedCsv;
	while (std::getline(data, line)) {
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<float> parsedRow;
		while (std::getline(lineStream, cell, ';')) {
			parsedRow.push_back(std::stof(cell));
		}

		parsedCsv.push_back(parsedRow);
	}

	return parsedCsv;
}

csvMatrix string2matrix(std::string data) {
	csvMatrix parsedString;
	std::stringstream lineStream(data);
	std::string cell;
	std::vector<float> parsedRow;
	while (std::getline(lineStream, cell, ';')) {
		parsedRow.push_back(std::stof(cell));
	}

	parsedString.push_back(parsedRow);
	return parsedString;
}

csvMatrix matrixSlice(csvMatrix m, int begin, int end) {
	csvMatrix slice;
	std::vector<float> vec(m[0].begin() + begin, m[0].end() - end);
	slice.push_back(vec);

	return slice;
}

csvMatrix matrixSliceSpecific(csvMatrix m, std::vector<int> indices) {
	csvMatrix slice;
	std::vector<float> vec;

	for (int i = 0; i < indices.size(); i++) {
		vec.push_back(m[0][indices[i]]);
	}

	slice.push_back(vec);

	return slice;
}

void matrixInitialize(csvMatrix& m, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		std::vector<float> parsedRow;
		for (int j = 0; j < cols; j++) {
			parsedRow.push_back(0.0);
		}

		m.push_back(parsedRow);
	}
}

csvMatrix matrixAdd(csvMatrix m1, csvMatrix m2) {
	csvMatrix result;
	matrixInitialize(result, m1.size(), m1[0].size());

	for (int i = 0; i < m1.size(); ++i) {
		for (int j = 0; j < m1[0].size(); ++j) {
			result[i][j] = m1[i][j] + m2[i][j];
		}
	}

	return result;
}

csvMatrix matrixMultiply(csvMatrix m1, csvMatrix m2) {
	csvMatrix result;
	matrixInitialize(result, m1.size(), m2[0].size());

	for (int i = 0; i < m1.size(); ++i) {
		for (int j = 0; j < m2[0].size(); ++j) {
			for (int k = 0; k < m1[0].size(); ++k) {
				result[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}

	return result;
}

csvMatrix matrixTranspose(csvMatrix m) {
	csvMatrix result;
	matrixInitialize(result, m[0].size(), m.size());

	for (int i = 0; i < m.size(); i++) {
		for (int j = 0; j < m[0].size(); j++) {
			result[j][i] = m[i][j];
		}
	}

	return result;
}

csvMatrix ReLu(csvMatrix m) {
	csvMatrix result;
	matrixInitialize(result, m.size(), m[0].size());

	for (int i = 0; i < m.size(); i++) {
		for (int j = 0; j < m[0].size(); j++) {
			if (m[i][j] <= 0) {
				result[i][j] = 0.0;
			}
			else result[i][j] = m[i][j];
		}
	}

	return result;
}

csvMatrix softmax(csvMatrix m) {
	csvMatrix result;
	matrixInitialize(result, m.size(), m[0].size());

	float ymax = *max_element(std::begin(m[0]), std::end(m[0])); // c++11
	for (int i = 0; i < m[0].size(); i++) {
		result[0][i] = exp(m[0][i] - ymax);
	}

	float ysum = 0;

	for (int i = 0; i < m[0].size(); i++) {
		ysum = ysum + result[0][i];
	}

	for (int i = 0; i < m[0].size(); i++) {
		result[0][i] /= ysum;
	}

	return result;
}

int predictClass(csvMatrix m) {
	int result = 0;
	for (int i = 0; i < m[0].size(); i++) {
		if (m[0][i] > m[0][result]) {
			result = i;
		}
	}

	return result;
}

void normalizeData(csvMatrix& m, csvMatrix mean, csvMatrix std) {
	for (int i = 0; i < m[0].size(); i++) {
		m[0][i] = m[0][i] - mean[0][i];
		m[0][i] = m[0][i] / std[0][i];
	}
}

void matrixPrint(csvMatrix m) {
	for (int i = 0; i < m.size(); i++) {
		for (int j = 0; j < m[i].size(); j++) {
			cout << "Element [" << i << ", " << j << "]: " << m[i][j] << endl;
		}
	}
}

#endif