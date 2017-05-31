#include "opengl.hpp"

using namespace std;
using namespace cgra;

class Boid{
private:
	cgra::vec3 position;
	cgra::vec3 velocity;
	cgra::vec3 destination;

	void vertData();
public:
	Boid(vec3 position);
	void render();
} 
