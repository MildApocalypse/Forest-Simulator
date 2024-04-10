#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>
#include <random>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "heightmap.hpp"

using namespace cgra;
using namespace hmap;
using namespace std;

Heightmap::Heightmap() {
	// Default the size to 5 if one is not provided
	size = 5;
	constructHelper();
}

Heightmap::Heightmap(int mapSize) {
	int counter = 1;
	int newSize = 5;
	while(counter < mapSize) {
		newSize = (newSize * 2) - 1;
		counter++;
	}
	size = newSize;
	constructHelper();
}

Heightmap::Heightmap(int mapSize, float mapSeed) {
	size = mapSize;
	seed = mapSeed;
	constructHelper();
}

Heightmap::~Heightmap() {  }

int Heightmap::getSize() {
	return size;
}

vector<vec3> Heightmap::getVertices() {
	return vertices;
}

vector<Triangle> Heightmap::getTriangles() {
	return triangles;
}

void Heightmap::constructHelper() {
	vector<float> vals (size, initial);
	heightmap.assign(size, vals);
}

void Heightmap::render() {
	glCallList(displayList);
}

void Heightmap::generateHeightmap() {
	int start = 0;
	int end = size - 1;

	generateCorners(start, end);

	int distance = (end - start);
	Point squareStart = Point(start, start);

	while(distance > 0) {
		for(int x = 0; x < end; x += distance) {
			for(int y = 0; y < end; y += distance) {
				squareStart = Point(x, y);
				calculateSquareCenter(squareStart, distance);
			}
		}

		for(int x = 0; x < end; x += distance) {
			for(int y = 0; y < end; y += distance) {
				squareStart = Point(x, y);
				calculateDiamondCenters(squareStart, distance);
			}
		}

		distance = distance / 2;

		// Shrink the lower and upper bounds by the decay rate,
		// so less randomness is introduced each iteration
		lower += (lower < 0) ? randomDecayRate : 0;
		upper -= (upper > 0) ? randomDecayRate : 0;
	}

	// Once the heightmap has been generated, turn it into a set of
	// vertices, normals and triangles.
	makeLists();
}

void Heightmap::generateCorners(int start, int end) {
	setAt(Point(start, start), randomValue());
    setAt(Point(start, end), randomValue());
    setAt(Point(end, start), randomValue());
    setAt(Point(end, end), randomValue());
}

void Heightmap::calculateSquareCenter(Point start, int distance) {
	Point centerOfSquare = getSquareCenter(start, distance);
	float averageOfSquare = getAverageOfSquare(start, distance);
	
	setAt(centerOfSquare, averageOfSquare + randomValue());
}

void Heightmap::calculateDiamondCenters(Point square, int distance) {
	int halfDistance = (distance / 2);
	Point centerOfSquare = getSquareCenter(square, distance);

	vector<Point> diamondCenters = {
		Point(centerOfSquare.x-halfDistance, centerOfSquare.y),
		Point(centerOfSquare.x+halfDistance, centerOfSquare.y),
		Point(centerOfSquare.x, centerOfSquare.y-halfDistance),
		Point(centerOfSquare.x, centerOfSquare.y+halfDistance),
	};

	if(distance > 0) {
		for(Point p : diamondCenters) {
			calculateDiamondCenter(p, halfDistance);
		}
	}
}

void Heightmap::makeLists() {
	float xyModifier = float((size-1) / 2);

	for(int z = 0; z < size; z++) {
		for(int x = 0; x < size; x++) {
			float worldX = x - xyModifier;
			float worldZ = -z + xyModifier;
			float y = getAt(Point(x, z));

			vertices.push_back(vec3(worldX, y, worldZ));
			normals.push_back(vec3(0, 0, 0));
		}
	}

	for(int i = 0; i < int(vertices.size()); i++) {

		// Check we haven't reached the edge of the heightmap
		if((i + 1) % size != 0 && (i + size) < vertices.size()) {
			int nextCol = i + 1;
			int nextRow = i + size;

			Triangle t = Triangle(i, nextCol, nextRow);
			triangles.push_back(t);

			int nextRowCol = nextRow + 1;

			t = Triangle(nextRow, nextCol, nextRowCol);
			triangles.push_back(t);
		}
	}

	// Calculate per face normals iterating over each triangle
	for(int i = 0; i < int(triangles.size()); i++) {
		int v1 = triangles[i].vertices[0];
		int v2 = triangles[i].vertices[1];
		int v3 = triangles[i].vertices[2];

		// point - point = vector, this gives the vectors of
		// two sides of the triangle
		vec3 v = vertices[v2] - vertices[v1];
		vec3 w = vertices[v3] - vertices[v1];
		
		// The face normal is the cross product of the two
		// vectors
		vec3 faceNormal = cross(v, w);

		// Add the new normal and assign it to all vertices in the triangle
		for(int j = 0; j < 3; j++) {
			triangles[i].normal = faceNormal;
		}
	}

	createDisplayList();
}

void Heightmap::createDisplayList() {
	displayList = glGenLists(1);
	glNewList(displayList, GL_COMPILE);
	glBegin(GL_TRIANGLES);

	for(int i = 0; i < int(triangles.size()); i++) {
		Triangle t = triangles[i];
		vec3 n = t.normal;

		vec3 v1 = vertices[t.vertices[0]];
		vec3 v2 = vertices[t.vertices[1]];
		vec3 v3 = vertices[t.vertices[2]];

		glTexCoord2f(0.0, 0.0);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(v1.x, v1.y, v1.z);

		glTexCoord2f(1.0, 0.0);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(v2.x, v2.y, v2.z);

		glTexCoord2f(1.0, 1.0);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(v3.x, v3.y, v3.z);
	}

	glEnd();
	glEndList();
}

void Heightmap::calculateDiamondCenter(Point diamondCenter, int distance) {
	float averageOfDiamond = getAverageOfDiamond(diamondCenter, distance);
	setAt(diamondCenter, averageOfDiamond + randomValue());
}

float Heightmap::getAverageOfSquare(Point start, int distance) {
	vector<Point> values = {
		Point(start.x, start.y),
		Point(start.x, start.y+distance),
		Point(start.x+distance, start.y),
		Point(start.x+distance, start.y+distance),
	};

	return averageOfList(values);
}

float Heightmap::getAverageOfDiamond(Point start, int distance) {
	vector<Point> values = {
		Point(start.x+distance, start.y),
		Point(start.x-distance, start.y),
		Point(start.x, start.y+distance),
		Point(start.x, start.y-distance),
	};
	
	return averageOfList(values);
}

bool Heightmap::isValid(Point point) {
    return (point.x >= 0 && point.y >= 0) && (point.x < size && point.y < size);
}

float Heightmap::averageOfList(vector<Point> list) {
	float count = 0.0;
	float total = 0.0;
	for(Point p : list) {
		if(isValid(p)) {
			total += getAt(p);
			count += 1.0;
		}
	}

	if(count > 0) {
		return total / count;
	}
	return initial;
}

Point Heightmap::getSquareCenter(Point squareStart, int distance) {
	int halfDistance = (distance/2);
	Point centerOfSquare = Point(
		squareStart.x + halfDistance,
		squareStart.y + halfDistance
	);
	return centerOfSquare;
}

void Heightmap::printHeightmap() {
	for(vector<float> v : heightmap) {
		for(float f : v) {
			cout << f << ", ";
		}
		cout << endl;
	}
}

float Heightmap::getAt(Point point) {
	return heightmap[point.y][point.x];
}

void Heightmap::setAt(Point point, float z) {
	if(heightmap[point.y][point.x] == initial) {
		heightmap[point.y][point.x] = z;
	}
}

void Heightmap::printAt(Point point) {
	cout << heightmap[point.y][point.x] << endl;
}

float Heightmap::randomValue() {
	return math::random<float>(lower, upper);
}