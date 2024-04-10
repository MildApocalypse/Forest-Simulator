#pragma once

#include <vector>

#include "cgra_math.hpp"

struct Triangle {
	cgra::vec3 normal;
	std::vector<int> vertices;
	std::vector<int> normals;
	Triangle() {};
	Triangle(int v1, int v2, int v3) {
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
	};
};