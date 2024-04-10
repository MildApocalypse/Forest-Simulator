#pragma once

#include <map>
#include <stack>

#include "opengl.hpp"
#include "cgra_math.hpp"
#include "triangle.hpp"

namespace tree {

	const cgra::vec3 up = cgra::vec3(0, 1, 0);
	const cgra::vec3 left = cgra::vec3(1, 0, 0);
	const cgra::vec3 heading = cgra::vec3(0, 0, 1);

	struct TreeState {
		float angle;
		float length;
		float radiusDecay = 0.05;
		float radiusStart = 0.4;
		int colourIndex = 0;
		cgra::mat4 orientation = cgra::mat4(
			cgra::vec4(1, 0, 0, 0),
			cgra::vec4(0, 1, 0, 0),
			cgra::vec4(0, 0, 1, 0),
			cgra::vec4(0, 0, 0, 1)
		);
		cgra::vec3 position;
		std::vector<cgra::vec3> colours;

		cgra::vec3 up() {
			cgra::vec4 u = cgra::vec4(0, 1, 0, 1);
			cgra::vec4 i = cgra::vec4(
				dot(orientation[0], u),
				dot(orientation[1], u),
				dot(orientation[2], u),
				dot(orientation[3], u)
			);
			return cgra::vec3(i.x, i.y, i.z);
		}

		cgra::vec3 left() {
			cgra::vec4 l = cgra::vec4(1, 0, 0, 1);
			cgra::vec4 i = cgra::vec4(
				dot(orientation[0], l),
				dot(orientation[1], l),
				dot(orientation[2], l),
				dot(orientation[3], l)
			);
			return cgra::vec3(i.x, i.y, i.z);
		}

		cgra::vec3 heading() {
			cgra::vec4 h = cgra::vec4(0, 0, 1, 1);
			cgra::vec4 i = cgra::vec4(
				dot(orientation[0], h),
				dot(orientation[1], h),
				dot(orientation[2], h),
				dot(orientation[3], h)
			);
			return cgra::vec3(i.x, i.y, i.z);
		}
	};

	struct TreePolygon {
		std::vector<cgra::vec3> vertices;
	};

	// Forward declare Tree so that pointers to 
	// member-methods can be constructed
	class Tree;

	typedef void (Tree::*RenderFunction)(void);

	class Tree {
	private:
		
		std::vector<std::string> strings;

		TreeState state;
		std::stack<TreeState> stateStack;
		std::stack<TreePolygon*> polygonStack;
		std::map<char, RenderFunction> functionMap;
		std::vector<cgra::vec3> vertices;
		std::vector<cgra::vec3> normals;
		std::vector<Triangle> triangles;

		GLuint displayList = 0;

		void drawBranchPlaceVertex();
		void drawBranch();
		void drawLeaf();
		void moveForwardPlaceVertex();
		void moveForward();
		void placeVertex();
		void turnLeft();
		void turnRight();
		void pitchUp();
		void pitchDown();
		void rollLeft();
		void rollRight();
		void pushState();
		void popState();
		void beginPoly();
		void endPoly();
		void turnAround();
		void increaseColourIndex();
		void decreaseColourIndex();
		void increaseLineWidth();
		void decreaseLineWidth();

		void createFromString();
		void createDisplayList();
		void turnPointsToTriangles(cgra::vec3, cgra::vec3);
		void makeTriangle(cgra::vec3, cgra::vec3, cgra::vec3);
	public:
		Tree();
		Tree(cgra::vec3, std::vector<std::string>, float, float, std::vector<cgra::vec3>);
		void render();
		std::vector<cgra::vec3> getBranchVertices();
	};
}