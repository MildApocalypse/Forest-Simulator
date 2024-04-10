#include "cgra_math.hpp"
#include "oct_tree.hpp"
#include "opengl.hpp"

using namespace std;
using namespace cgra;

OctTree::OctTree(){
	bounding_box = boundingBox();
	bounding_box.pos = vec3(-50, -50, -50);
	bounding_box.size = vec3(100, 100, 100);
} 

OctTree::OctTree(boundingBox box){
	bounding_box = box;
}

OctTree::OctTree(boundingBox box, vector<Boid*> obj){
	bounding_box = box;
	objects = obj;
}

OctTree::OctTree(vec3 pos, vec3 size, vector<Boid*> obj){
	bounding_box = boundingBox();
	bounding_box.pos = pos;
	bounding_box.size = size;
	objects = obj;

	buildTree(0);
}

/*Recursively build an oct tree*/
void OctTree::buildTree(int level){
	// cout << "bilbo" << level << endl;
	//Terminate if there are too few objects in this cell
	if(objects.size() <= 4){
		return;
	}

	//Terminate if we are getting too small
	if(bounding_box.size.x < MIN_SIZE 
		|| bounding_box.size.y < MIN_SIZE 
		|| bounding_box.size.z < MIN_SIZE){
		return;
	}
	

	//Time to make some new children. First divide the current bounding box by 2
	vec3 factorHalf = bounding_box.size / 2.0f;

	//We need to find which octant each object is in. Divide current cell into 8
	boundingBox octants[8];
	addOctant(bounding_box.pos, factorHalf, &octants[0]);
	addOctant(bounding_box.pos + vec3(factorHalf.x, 0, 0), factorHalf, &octants[1]);
	addOctant(bounding_box.pos + vec3(0, factorHalf.y, 0), factorHalf, &octants[2]);
	addOctant(bounding_box.pos + vec3(factorHalf.x, factorHalf.y, 0), factorHalf, &octants[3]);
	addOctant(bounding_box.pos + vec3(0, 0, factorHalf.z), factorHalf, &octants[4]);
	addOctant(bounding_box.pos + vec3(factorHalf.x, 0, factorHalf.z), factorHalf, &octants[5]);
	addOctant(bounding_box.pos + vec3(0, factorHalf.y, factorHalf.z), factorHalf, &octants[6]);
	addOctant(bounding_box.pos + vec3(factorHalf.x, factorHalf.y, factorHalf.z), factorHalf, &octants[7]);


	//Create a temp list of indexes for objects that are going to be moved from the current 
	//cell into its children
	vector<int> moved;

	//Create a list for each of the octants to store objects in
	vector<Boid*> octantObjects[8];
	for (int i = 0; i < 8; ++i){
		octantObjects[i] = vector<Boid*>();
	}

	//For each object in the current cell...
	int s = objects.size();
	for(int i = 0; i < s; ++i){

		//For each octant...
		for(int j = 0; j < 8; ++j){

			// cout << "Checking octant " << j << ", object " << i;
			//Check if the object is in this octant
			if(contains(objects[i], octants[j])){

				//Add to this octant's object list and add the moving list
				octantObjects[j].push_back(objects[i]);
				moved.push_back(i);
				break;

			}
		}
	}

	// Remove moving objects from the current cell's object list
	s = moved.size();
	for(int i = 0; i < s; ++i){
		objects.erase(objects.begin() + (moved[i] - i));
	}

	//Finally, create the child nodes
	for(int i = 0; i < 8; ++i){
		//cout << "objects in node" << i << ": " << octantObjects[i].size() << endl;
		if(octantObjects[i].size() != 0){
			hasChildren = true;
			//cout << "creating child" << endl;
			children[i] = createNode(octants[i], octantObjects[i]);
			activeNodes |= (uint8_t)(1 << i);
			children[i]->buildTree(level + 1);
			//cout << "done" << endl;
		}
	}
	treeBuilt = true;
	treeReady = true;
}

void OctTree::addOctant(vec3 pos, vec3 size, boundingBox *octant){
	octant->pos = pos;
	octant->size = size;
}

OctTree* OctTree::createNode(boundingBox region, vector<Boid*> obs){
	OctTree *oct = new OctTree(region, obs);
	oct->parent = this;
	return oct;
}


bool OctTree::contains(Boid *boid, boundingBox box){
	float minX = box.pos.x + boid->minimum_separation;
	float maxX = box.pos.x + box.size.x - boid->minimum_separation;
	float minY = box.pos.y + boid->minimum_separation;
	float maxY = box.pos.y + box.size.y - boid->minimum_separation;
	float minZ = box.pos.z + boid->minimum_separation;
	float maxZ = box.pos.z + box.size.z - boid->minimum_separation;

	// cout << ", position: " << box.pos << ", size: " << box.size.x << endl;
	// cout << "Boid position: x: " << boid->position.x << ", y: " << boid->position.y << ", z: " << boid->position.z << endl;

	if(boid->position.x > minX && boid->position.x < maxX 
		&& boid->position.y > minY && boid->position.y < maxY
		&& boid->position.z > minZ && boid->position.z < maxZ){
		// cout << "true" << endl;
		return true;
	}
	// cout << "false" << endl;
	return false;
}

float OctTree::lengthVector(vec3 v){
	vec3 u = v;
	float n = (u.x * u.x + u.y * u.y + u.z * u.z);
	if(n != 0){
		float l = sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
		return l;
	}
	return 0;
}

vector<hitRecord> OctTree::findCollisions(vector<Boid*> parentObs){
	vector<hitRecord> collisions;

	//Check parent collisions with objects in this node
	for(Boid *pBoid : parentObs){

		for(Boid *lBoid : objects){

			vec3 f = pBoid->position - lBoid->position;
			if(lengthVector(f) < pBoid->minimum_separation){
				hitRecord hr = hitRecord();
				hr.boid = pBoid;
				hr.neighbour = lBoid;
				hr.force = f;

				collisions.push_back(hr);
			}
		}
	}

	//Check local collisions in this node
	for(Boid *lBoid : objects){

		for(Boid *b : objects){

			if(lBoid != b){

				vec3 f = lBoid->position - b->position;
				if(lengthVector(f) < lBoid->minimum_separation){
					hitRecord hr = hitRecord();
					hr.boid = lBoid;
					hr.neighbour = b;
					hr.force = f;

					collisions.push_back(hr);
				}
			}
		}
	}

	for(Boid *b : objects){
		parentObs.push_back(b);
	}

	vector<hitRecord> tmp;
	int i = 0;
	for(uint8_t flags = activeNodes; flags > 0; flags >>= 1){

		if((flags & 1) == 1){

			tmp = children[i]->findCollisions(parentObs);
			for(hitRecord hr : tmp){

				collisions.push_back(hr);
			}
		}
		++i;
	}
	return collisions;
}

void OctTree::renderTree(int level){
	glPushMatrix();

	glBegin(GL_LINES);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.size.y + bounding_box.pos.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);

	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);

	glVertex3f(bounding_box.pos.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z + bounding_box.size.z);
	glVertex3f(bounding_box.pos.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z + bounding_box.size.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y, bounding_box.pos.z + bounding_box.size.z);

	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z + bounding_box.size.z);
	glVertex3f(bounding_box.pos.x + bounding_box.size.x, bounding_box.pos.y + bounding_box.size.y, bounding_box.pos.z);
	glEnd();
	
	glPopMatrix();

	if((int)activeNodes == 0){
		return;
	}
	else{
		int i = 0;
		for(uint8_t flags = activeNodes; (int)flags > 0; flags >>= 1){
			if((flags & 1) == 1){
				children[i]->renderTree(level + 1);
			}
			++i;
		}
	}
}