//---------------------------------------------------------------------------
//    ____                 __    _____            __     __          
//   / __/__  _______ ___ / /_  / __(_)_ _  __ __/ /__ _/ /____  ____
//  / _// _ \/ __/ -_|_-</ __/ _\ \/ /  ' \/ // / / _ `/ __/ _ \/ __/
// /_/  \___/_/  \__/___/\__/ /___/_/_/_/_/\_,_/_/\_,_/\__/\___/_/   
//                                                                  
// A COMP308 project by Lawrence Buck and Sam Costigan.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "cgra_math.hpp"
#include "simple_image.hpp"
#include "opengl.hpp"
#include "heightmap.hpp"
#include "lsystem.hpp"
#include "tree.hpp"
#include "treefactory.hpp"

#include "flock.hpp"

using namespace std;
using namespace cgra;

// Window
//
GLFWwindow* g_window;


// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.1;
float g_zfar = 1000.0;


// Mouse controlled Camera values
//
bool g_leftMouseDown = false;
vec2 g_mousePosition;
float g_pitch = 0;
float g_yaw = 0;
float g_zoom = 1.0;

// Textures
//
GLuint snow_texture = 0;

// Command line argument defaults
//
int mapSize = 4;
string treeFile = "res/trees/trees.txt";
bool useOctTree = false;
int num_boids = 200;

// Base Heightmap to be rendered upon
//
hmap::Heightmap* heightmap;
tree::TreeFactory* treeFactory;
vector<tree::Tree*> trees;

//Flock of birds
//
Flock* flock;
Boid* boid;

//flags
//
bool showOctTree = false;
bool debugMode = false;


// Mouse Button callback
// Called for mouse movement event on since the last glfwPollEvents
//
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	// cout << "Mouse Movement Callback :: xpos=" << xpos << "ypos=" << ypos << endl;
	if (g_leftMouseDown) {
		g_yaw -= g_mousePosition.x - xpos;
		g_pitch -= g_mousePosition.y - ypos;
	}
	g_mousePosition = vec2(xpos, ypos);
}


// Mouse Button callback
// Called for mouse button event on since the last glfwPollEvents
//
void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
	// cout << "Mouse Button Callback :: button=" << button << "action=" << action << "mods=" << mods << endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		g_leftMouseDown = (action == GLFW_PRESS);
}


// Scroll callback
// Called for scroll event on since the last glfwPollEvents
//
void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
	// cout << "Scroll Callback :: xoffset=" << xoffset << "yoffset=" << yoffset << endl;
	g_zoom -= yoffset * g_zoom * 0.2;
}

void step() {
	flock->update(useOctTree);
}


// Keyboard callback
// Called for every key event on since the last glfwPollEvents
//
void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	// cout << "Key Callback :: key=" << key << "scancode=" << scancode
	// << "action=" << action << "mods=" << mods << endl;
	if (key == 83 && action == 1) {
		showOctTree = !showOctTree;
	}
	if (key == 68 && action == 1) {
		debugMode = !debugMode;
	}
	if (key == 65 && action == 1) {
		if (debugMode) {
			step();
		}
	}
	if (key == 79 && action == 1) {
		useOctTree = !useOctTree;
		if (useOctTree) {
			cout << "Using oct tree" << endl;
		}
		else {
			cout << "Not using oct tree" << endl;
		}
	}
}


// Character callback
// Called for every character input event on since the last glfwPollEvents
//
void charCallback(GLFWwindow* win, unsigned int c) {
	// cout << "Char Callback :: c=" << char(c) << endl;
	// Not needed for this assignment, but useful to have later on
}


// Sets up where the camera is in the scene
// 
void setupCamera(int width, int height) {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, 0, -50 * g_zoom);
	glRotatef(g_pitch, 1, 0, 0);
	glRotatef(g_yaw, 0, 1, 0);
}

GLuint getTexture(string filename) {
	GLuint texture = 0;
	Image tex("res/textures/" + filename);

	glGenTextures(1, &texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Setup sampling strategies
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tex.w, tex.h, tex.glFormat(), GL_UNSIGNED_BYTE, tex.dataPointer());

	return texture;
}

void initTextures() {
	snow_texture = getTexture("snow.jpg");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, snow_texture);
}

void initAmbientLight() {
	float light[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light);

	glEnable(GL_LIGHT0);
}

void initSecondAmbientLight() {
	float light[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light);

	glEnable(GL_LIGHT1);

}

void initDirectionalLight() {
	float position[] = { 5.0f, 40.0f, 5.0f, 1.0f };
	float light[] = { 1.0f, 1.0f, 0.878f, 0.5f };
	float noLight[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float exponent[] = { 2.0f };
	float attenuation[] = { 0.0f };

	glLightfv(GL_LIGHT2, GL_LINEAR_ATTENUATION, attenuation);
	glLightfv(GL_LIGHT2, GL_POSITION, position);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light);

	glEnable(GL_LIGHT2);
}

void initLights() {
	initAmbientLight();
	//initSecondAmbientLight();
	initDirectionalLight();
}

void initHeightmap() {
	heightmap = new hmap::Heightmap(mapSize);
	heightmap->generateHeightmap();
}

void initTrees() {
	treeFactory = new tree::TreeFactory(treeFile);

	int incr = 8;
	float halfIncr = incr / 2;
	int halfSize = (heightmap->getSize() - (incr * 2)) / 2;
	int y = halfSize;

	for (int x = -halfSize; x <= halfSize; x += incr) {
		// Randomly offset the trees
		float offsetX = math::random(-halfIncr, halfIncr);
		float offsetY = math::random(-halfIncr, halfIncr);
		trees.push_back(treeFactory->generate(vec3(x + offsetX, -y + offsetY, 0)));
		if (x == halfSize && y > -halfSize) {
			y -= incr;
			x = -halfSize - incr;
		}
	}

	// Trees have been generated, so release the treeFactory object
	// from memory and set its pointer to null
	delete treeFactory;
	treeFactory = nullptr;
}

void initFlock() {
	flock = new Flock(num_boids);
	boid = new Boid(vec3(1, 1, 1));
}

void groundMaterial() {
	glBindTexture(GL_TEXTURE_2D, snow_texture);

	GLfloat mat_specular[] = { 0.7, 0.0, 0.0, 1.0 };
	GLfloat mat_diffuse[] = { 0.7, 0.0, 0.0, 1.0 };
	GLfloat mat_shininess[] = { 100.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void boidMaterial() {
	glBindTexture(GL_TEXTURE_2D, snow_texture);

	GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat mat_diffuse[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat mat_shininess[] = { 0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}




void renderObjects(int width, int height) {
	// Render a single square as our geometry
	// You would normally render your geometry here
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix();
	groundMaterial();
	heightmap->render();
	glPopMatrix();

	glPushMatrix();
	for (tree::Tree* t : trees) {
		t->render();
	}
	glPopMatrix();

	boidMaterial();
	if (!debugMode) {
		flock->update(useOctTree);
	}
	else {
		flock->render();
	}
	if (showOctTree) {
		flock->showOctTree();
	}
	//boid->render();


	glPopMatrix();

	glFlush();
}

// Draw function
//
void render(int width, int height) {

	// Light blue background
	glClearColor(0.66f, 0.77f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Enable flags for normal rendering
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	setupCamera(width, height);

	// Texture setup
	//

	// Enable Drawing texures
	glEnable(GL_TEXTURE_2D);

	renderObjects(width, height);

	// Disable flags for cleanup (optional)
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}


// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);


//Main program
// 
int main(int argc, char** argv) {

	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	if (argc >= 2) {
		int tempSize = *argv[1] - '0';
		if (0 > tempSize || tempSize > 6) {
			cerr << "Error: Map Size " << tempSize << " is out of bounds (0 - 5)" << endl;
			abort();
		}
		mapSize = tempSize;
	}

	if (argc == 3) {
		treeFile = argv[2];
	}
	else if (argc == 4) {
		treeFile = argv[2];
		string nBoids = argv[3];
		num_boids = stoi(nBoids);
	}
	else if (argc == 5) {
		treeFile = argv[2];
		string nBoids = argv[3];
		num_boids = stoi(nBoids);
		useOctTree = argv[4];
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Create a windowed mode window and its OpenGL context
	g_window = glfwCreateWindow(640, 480, "Forest Simulator", nullptr, nullptr);
	if (!g_window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(g_window);

	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}



	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;



	// Attach input callbacks to g_window
	glfwSetCursorPosCallback(g_window, cursorPosCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetCharCallback(g_window, charCallback);



	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	//if (glfwExtensionSupported("GL_ARB_debug_output")) {
	//	// This allows the error location to be determined from a stacktrace
	//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//	// Set the up callback
	//	glDebugMessageCallbackARB(debugCallbackARB, nullptr);
	//	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
	//	cout << "GL_ARB_debug_output callback installed" << endl;
	//}
	//else {
	//	cout << "GL_ARB_debug_output not available. No worries." << endl;
	//}


	// Initialize Geometry/Material/Lights
	initTextures();
	initFlock();
	initHeightmap();
	initTrees();
	initLights();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(g_window)) {

		// Make sure we draw to the WHOLE window
		int width, height;
		glfwGetFramebufferSize(g_window, &width, &height);

		// Main Render
		render(width, height);

		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
}






//-------------------------------------------------------------
// Fancy debug stuff
//-------------------------------------------------------------

// function to translate source to string
string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// function to translate severity to string
string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return("Low");
	default:
		return("n/a");
	}
}

// function to translate type to string
string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	cerr << endl; // extra space

	cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << endl;

	cerr << message << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("");
}