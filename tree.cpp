#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "opengl.hpp"
#include "cgra_math.hpp"
#include "cgra_geometry.hpp"
#include "tree.hpp"

using namespace cgra;
using namespace std;
using namespace tree;

void tMaterial() {
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 30.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setMaterial(vec3 colour) {
	GLfloat mat_specular[] = { colour.x, colour.y, colour.z, 1.0 };
	GLfloat mat_diffuse[] = { colour.x, colour.y, colour.z, 1.0 };
	GLfloat mat_shininess[] = { 30.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess);
}

vec3 translate(vec3 start, vec3 translation) {
	vec4 newStart = vec4(start.x, start.y, start.z, 1);
	mat4 translationMatrix = mat4(
		vec4(1, 0, 0, translation.x),
		vec4(0, 1, 0, translation.y),
		vec4(0, 0, 1, translation.z),
		vec4(0, 0, 0, 1)
	);

	vec3 ns = vec3(
		dot(newStart, translationMatrix[0]),
		dot(newStart, translationMatrix[1]),
		dot(newStart, translationMatrix[2])
	);

	return ns;
}

mat4 makeRotationMatrix(vec3 rotationAxis, float angle) {
	// Ensure rotation axis is normalized
	vec3 u = normalize(rotationAxis);

	// Convert angle from degrees to radians
	float a = radians(angle);

	// Create the rotation matrix based on axis and angle.
	// See: https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
	mat4 rotationMatrix = mat4(
		vec4(cos(a) + (pow(u.x, 2) * (1 - cos(a))), u.x * u.y * (1 - cos(a)) - u.z * sin(a), u.x * u.y * (1 - cos(a)) + u.y * sin(a), 0.0f),
		vec4(u.y * u.x * (1 - cos(a)) + u.z * sin(a), cos(a) + pow(u.y, 2) * (1 - cos(a)), u.y * u.x * (1 - cos(a)) - u.x * sin(a), 0.0f),
		vec4(u.z * u.x * (1 - cos(a)) - u.y * sin(a), u.z * u.y * (1 - cos(a)) + u.x * sin(a), cos(a) + pow(u.z, 2) * (1 - cos(a)), 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return rotationMatrix;
}

mat4 rotate(mat4 start, vec3 rotationAxis, float angle) {
	mat4 rotationMatrix = makeRotationMatrix(rotationAxis, angle);
	mat4 m = rotationMatrix * start;
	return m;
}

Tree::Tree() {
	
}

Tree::Tree(vec3 startingPos, vector<string> s, float a, float l, vector<vec3> colours) {
	strings = s;
	state.position = startingPos;
	state.angle = a;
	state.length = l;
	state.colours = colours;

	functionMap = {
		{'F', &Tree::drawBranchPlaceVertex},
		{'G', &Tree::drawBranch},
		{'f', &Tree::moveForwardPlaceVertex},
		{'g', &Tree::moveForward},
		{'[', &Tree::pushState},
		{']', &Tree::popState},
		{'{', &Tree::beginPoly},
		{'}', &Tree::endPoly},
		{'+', &Tree::turnLeft},
		{'-', &Tree::turnRight},
		{'^', &Tree::pitchUp},
		{'&', &Tree::pitchDown},
		{'\\', &Tree::rollLeft},
		{'/', &Tree::rollRight},
		{'|', &Tree::turnAround},
		{'’', &Tree::increaseColourIndex},
		{';', &Tree::decreaseColourIndex},
		{'#', &Tree::increaseLineWidth},
		{'!', &Tree::decreaseLineWidth},
		{'.', &Tree::placeVertex}
	};

	createDisplayList();
}

void Tree::createDisplayList() {
	displayList = glGenLists(1);
	glNewList(displayList, GL_COMPILE);

	glRotatef(-90, 1, 0, 0);

	createFromString();
	
	glEndList();
}

void Tree::createFromString() {
	string s = strings.back();

	for(int i = 0; i < int(s.size()); i++) {
		char c = s.at(i);
		// Get the corresponding function for character
		// c and call it on this object
		if(functionMap.find(c) != functionMap.end()) {
			RenderFunction func = functionMap.at(c);
			(this->*func)();
		}
	}
}

void Tree::render() {
	glPushMatrix();
		glCallList(displayList);
	glPopMatrix();
}

void Tree::turnPointsToTriangles(vec3 posStart, vec3 posEnd) {
	float radiusEnd = max(state.radiusStart - state.radiusDecay, 0.1f);

	vec3 topLeft = vec3(posEnd.x - radiusEnd, posEnd.y, posEnd.z);
	vec3 topRight = vec3(posEnd.x, posEnd.y + radiusEnd, posEnd.z);
	vec3 botLeft = vec3(posStart.x - state.radiusStart, posStart.y, posStart.z);
	vec3 botRight = vec3(posStart.x, posStart.y + state.radiusStart, posStart.z);

	makeTriangle(topLeft, topRight, botRight);
	makeTriangle(botRight, botLeft, topLeft);

	topLeft = topRight;
	topRight = vec3(posEnd.x + radiusEnd, posEnd.y, posEnd.z);
	botLeft = botRight;
	botRight = vec3(posStart.x + state.radiusStart, posStart.y, posStart.z);

	makeTriangle(topLeft, topRight, botRight);
	makeTriangle(botRight, botLeft, topLeft);

	topLeft = topRight;
	topRight = vec3(posEnd.x, posEnd.y - radiusEnd, posEnd.z);
	botLeft = botRight;
	botRight = vec3(posStart.x, posStart.y - state.radiusStart, posStart.z);

	makeTriangle(topLeft, topRight, botRight);
	makeTriangle(botRight, botLeft, topLeft);

	topLeft = topRight;
	topRight = vec3(posEnd.x - radiusEnd, posEnd.y, posEnd.z);
	botLeft = botRight;
	botRight = vec3(posStart.x - state.radiusStart, posStart.y, posStart.z);

	makeTriangle(topLeft, topRight, botRight);
	makeTriangle(botRight, botLeft, topLeft);

	state.radiusStart = radiusEnd;
}

void Tree::makeTriangle(vec3 first, vec3 second, vec3 third) {
	vertices.push_back(first);
	vertices.push_back(second);
	vertices.push_back(third);

	vec3 v = second - first;
	vec3 w = third - first;

	vec3 faceNormal = cross(v, w);

	normals.push_back(faceNormal);
	Triangle t = Triangle(vertices.size()-3, vertices.size()-2, vertices.size()-1);
	for(int i = 0; i < 3; i++) {
		t.normals.push_back(normals.size() - 1);
	}
	triangles.push_back(t);
}

std::vector<cgra::vec3> Tree::getBranchVertices() {
	return vertices;
}

void Tree::drawBranchPlaceVertex() {
	drawBranch();
	placeVertex();
}

void Tree::drawBranch() {
	vec3 posStart = state.position;
	vec3 move = state.heading() * state.length;

	vec3 posEnd = translate(posStart, move);
	state.position = posEnd;

	turnPointsToTriangles(posStart, posEnd);

	tMaterial();
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < int(triangles.size()); i++) {
		Triangle t = triangles[i];
		vec3 n1 = normals[t.normals[0]];
		vec3 n2 = normals[t.normals[1]];
		vec3 n3 = normals[t.normals[2]];
		glNormal3f(n1.x, n1.y, n1.z);
		glNormal3f(n2.x, n2.y, n2.z);
		glNormal3f(n3.x, n3.y, n3.z);

		vec3 v1 = vertices[t.vertices[0]];
		vec3 v2 = vertices[t.vertices[1]];
		vec3 v3 = vertices[t.vertices[2]];
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
	}
	glEnd();
}

void Tree::moveForwardPlaceVertex() {
	moveForward();
	placeVertex();
}

void Tree::moveForward() {
	vec3 posStart = state.position;
	vec3 move = state.heading() * state.length;
	vec3 posEnd = translate(posStart, move);
	state.position = posEnd;
}

void Tree::placeVertex() {
	if(!polygonStack.empty()) {
		TreePolygon* tp = polygonStack.top();
		tp->vertices.push_back(state.position);
	}
}

void Tree::turnLeft() {
	mat4 re = rotate(state.orientation, up, state.angle);
	state.orientation = re;
}

void Tree::turnRight() {
	mat4 re = rotate(state.orientation, up, -state.angle);
	state.orientation = re;
}

void Tree::pitchUp() {
	mat4 re = rotate(state.orientation, left, state.angle);
	state.orientation = re;
}

void Tree::pitchDown() {
	mat4 re = rotate(state.orientation, left, -state.angle);
	state.orientation = re;
}

void Tree::rollLeft() {
	mat4 re = rotate(state.orientation, heading, state.angle);
	state.orientation = re;
}

void Tree::rollRight() {
	mat4 re = rotate(state.orientation, heading, -state.angle);
	state.orientation = re;
}

void Tree::turnAround() {
	// Flips 180 degrees
	mat4 re = rotate(state.orientation, up, 180.0);
	state.orientation = re;
}

void Tree::pushState() {
	stateStack.push(state);
}

void Tree::popState() {
	state = stateStack.top();
	stateStack.pop();
}

void Tree::beginPoly() {
	TreePolygon* tp = new TreePolygon();
	polygonStack.push(tp);
}

void Tree::endPoly() {
	TreePolygon* tp = polygonStack.top();
	polygonStack.pop();

	glBegin(GL_POLYGON);
	for(vec3 v : tp->vertices) {
		glVertex3f(v.x, v.y, v.z);
	}
	glEnd();

	delete tp;
}

void Tree::increaseColourIndex() {
	int newColourIndex = state.colourIndex + 1;
	if(newColourIndex < int(state.colours.size())) {
		state.colourIndex = newColourIndex;
	} else {
		state.colourIndex = 0;
	}

	setMaterial(state.colours[state.colourIndex]);
}

void Tree::decreaseColourIndex() {
	int newColourIndex = state.colourIndex - 1;
	if(newColourIndex >= 0) {
		state.colourIndex = newColourIndex;
	} else {
		state.colourIndex = int(state.colours.size()-1);
	}

	setMaterial(state.colours[state.colourIndex]);
}

void Tree::increaseLineWidth() {
	float newLength = state.length / 0.8;
	if(newLength < 4.0) state.length = newLength;
}

void Tree::decreaseLineWidth() {
	float newLength = state.length * 0.8;
	if(newLength > 0.2) state.length = newLength;
}