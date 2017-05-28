#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "heightmap.hpp"

using namespace std;

Heightmap::Heightmap() {
	constructHelper();
}

Heightmap::Heightmap(float mapSize) {
	size = mapSize;
	constructHelper();
}

Heightmap::Heightmap(float mapSize, float mapSeed) {
	size = mapSize;
	seed = mapSeed;
	constructHelper();
}

Heightmap::~Heightmap() {  }

void Heightmap::constructHelper() {
	cout << "yes" << endl;
}

float Heightmap::getAt(Point point) {

	return 0.0;
}

void Heightmap::setAt(Point point, float z) {

}

void Heightmap::generateHeightmap() {

}
