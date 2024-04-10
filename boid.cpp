#include <cmath>
#include <cstdlib>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "boid.hpp"

using namespace std;
using namespace cgra;

Boid::Boid(vec3 pos){
	position = pos;
	addTriangles();
	createDisplayList();
}

void Boid::addTriangles(){
	triangle tri;

	tri.vertices[0] = vec3( 0.4f, 0.0f, 0.0f );
	tri.vertices[1] = vec3( 0.0f, -0.2f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, 0.2f, 0.0f );

	process(tri);

	tri.vertices[0] = vec3( -0.4f, 0.0f, 0.0f );
	tri.vertices[1] = vec3( 0.0f, -0.2f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, 0.2f, 0.0f );

	process(tri);

	tri.vertices[0] = vec3( 0.0f, 0.0f, 1.2f );
	tri.vertices[1] = vec3( 0.4f, 0.0f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, 0.2f, 0.0f );

	process(tri);

	tri.vertices[0] = vec3( 0.0f, 0.0f, 1.2f );
	tri.vertices[1] = vec3( -0.4f, 0.0f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, 0.2f, 0.0f );

	process(tri);

	tri.vertices[0] = vec3( 0.0f, 0.0f, 1.2f );
	tri.vertices[1] = vec3( 0.4f, 0.0f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, -0.2f, 0.0f );

	process(tri);

	tri.vertices[0] = vec3( 0.0f, 0.0f, 1.2f );
	tri.vertices[1] = vec3( -0.4f, 0.0f, 0.0f );
	tri.vertices[2] = vec3( 0.0f, -0.2f, 0.0f );

	process(tri);
}

void Boid::process(triangle tri){
	vec3 v1 = tri.vertices[0] - tri.vertices[1];
	vec3 v2 = tri.vertices[0] - tri.vertices[2];
		
	tri.normal = cross(v1, v2);

	triangles.push_back(tri);
}
void Boid::createDisplayList()
{
	// Delete old list if there is one
	if (m_displayList) glDeleteLists(m_displayList, 1);

	// Create a new list
	m_displayList = glGenLists(1);
	glNewList(m_displayList, GL_COMPILE);

	glBegin(GL_TRIANGLES);

	for(int i = 0; i < triangles.size(); ++i){
		glVertex3f(triangles[i].vertices[0].x, triangles[i].vertices[0].y, triangles[i].vertices[0].z);
		glNormal3f(triangles[i].normal.x, triangles[i].normal.y, triangles[i].normal.z);

		glVertex3f(triangles[i].vertices[1].x, triangles[i].vertices[1].y, triangles[i].vertices[1].z);
		glNormal3f(triangles[i].normal.x, triangles[i].normal.y, triangles[i].normal.z);

		glVertex3f(triangles[i].vertices[2].x, triangles[i].vertices[2].y, triangles[i].vertices[2].z);
		glNormal3f(triangles[i].normal.x, triangles[i].normal.y, triangles[i].normal.z);

	}

	glEnd();

	glEndList();
}

void Boid::render(){
	glPushMatrix();

	glTranslatef(position.x, position.y, position.z);

	vec3 axis = cross(normalize(velocity), vec3(0, 0, 1));
	float d = dot(normalize(velocity), vec3(0, 0, 1));
	float th = -degrees(acos(d));
	glRotatef(th, axis.x, axis.y, axis.z);
	
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glCallList(m_displayList);
	
	glPopMatrix();
}