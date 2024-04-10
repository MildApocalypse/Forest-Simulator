#include "cgra_math.hpp"
#include "opengl.hpp"
#include "oct_tree.hpp"

using namespace std;
using namespace cgra;

class Flock{
private:

	OctTree *oct_tree = nullptr;
	bool use_tree = false;

	Boid *leader = new Boid(vec3(0, 0, 0));
	vec3 destination = vec3(15, 15, 15);
	vector<Boid*> boids;

	float max_speed = 0.2f;

	void steer(Boid *b);
	float lengthVector(cgra::vec3 v);
	cgra::vec3 normalizeVector(cgra::vec3 v);
	cgra::vec3 separate(Boid *b);
	void octSeparate();
	cgra::vec3 align(Boid *b);
	void checkChangeDest();
	
	void arrange(Boid *node, Boid *b);
public:
	Flock(int size);
	void setDestination(vec3 dest);
	void update(bool useTree);
	void showOctTree();
	void render();
};
