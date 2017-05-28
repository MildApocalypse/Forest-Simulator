#pragma once

struct Point {
	int x;
	int y;
};

class Heightmap
{
private:
	void constructHelper();
	int size;
	int seed;
public:
	Heightmap();
	Heightmap(float);
	Heightmap(float, float);
	~Heightmap();


	float getAt(Point);
	void setAt(Point, float);
	void generateHeightmap();
	
};