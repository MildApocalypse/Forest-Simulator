#include <cmath>
#include <cstdlib>

#include "cgra_math.hpp"
#include "flock.hpp"

using namespace std;
using namespace cgra;

Flock::Flock(int size){
	for (int i = 0; i < size - 1; ++i){
		Boid *b = new Boid(vec3(rand() % 10, rand() % 10, rand() % 10));
		boids.push_back(b);
		arrange(leader, b);
	}
	oct_tree = new OctTree(vec3(-50, -50, -50), vec3(100, 100, 100), boids);
} 

void Flock::update(bool useTree){
	use_tree = useTree;

	leader->destination = destination;
	steer(leader);
	checkChangeDest();
	leader->render();

	if(use_tree){
		octSeparate();
	}
	int s = boids.size();
	for(int i = 0; i < s; ++i){
		boids[i]->destination = boids[i]->parent->position;
		steer(boids[i]);
		boids[i]->render();
	}

	delete oct_tree;
	oct_tree = nullptr;

	oct_tree = new OctTree(vec3(-50, -50, -50), vec3(100, 100, 100), boids);
}

void Flock::steer(Boid *b){
	vec3 v = vec3(0, 0, 0);
	v += align(b) * 0.015;
	if(!use_tree){
		v += separate(b);
	}

	b->velocity += v;

	float length = lengthVector(b->velocity);
	if(length > max_speed){
		b->velocity = b->velocity*(max_speed/length);
	}

	b->position += b->velocity;
}

void Flock::checkChangeDest(){
	vec3 v = leader->position - destination;
	if(lengthVector(v) < 5){
		int x = rand() % 20;
		int z = rand() % 20;
		if(rand() % 2){x *= -1;}
		if(rand() % 2){z *= -1;}
		destination = vec3(x, 12, z);
	}
}

float Flock::lengthVector(vec3 v){
	vec3 u = v;
	float n = (u.x * u.x + u.y * u.y + u.z * u.z);
	if(n != 0){
		float l = sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
		return l;
	}
	return 0;
}

vec3 Flock::normalizeVector(vec3 v){
	vec3 u = v;
	float l = lengthVector(u);
	if(l != 0){
		if(u.x != 0){ u.x /= l; }
		if(u.y != 0){ u.y /= l; }
		if(u.z != 0){ u.z /= l; }
		return u;
	}
	return vec3(0, 0, 0);
}

vec3 Flock::align(Boid *b){
	vec3 a = b->destination - b->position;
	return normalizeVector(a);
}

vec3 Flock::separate(Boid *b){
	int neighbours = 0;
	vec3 velocity = vec3(0, 0, 0);

	int s = boids.size();
	for(int i = 0; i < s-1; ++i){
		if(boids[i] != b){
			float distance = lengthVector(b->position - boids[i]->position);
			if(distance < b->minimum_separation){
				vec3 force = b->position - boids[i]->position;
				velocity += normalizeVector(force);

				++neighbours;
			}
		}
	}

	if(neighbours > 0){
		if(velocity.x != 0){velocity.x /= neighbours;}
		if(velocity.y != 0){velocity.y /= neighbours;}
		if(velocity.z != 0){velocity.z /= neighbours;}
		return velocity;
	}
	return velocity;
}

void Flock::octSeparate(){
	vector<Boid*> obsList;
	vector<hitRecord> collisions = oct_tree->findCollisions(obsList);

	for(hitRecord hr : collisions){
		hr.boid->o_velocity += normalizeVector(hr.force);
		hr.boid->o_neighbours++;
	}

	for(hitRecord hr : collisions){
		if(hr.boid->o_neighbours > 0){
			if(hr.boid->o_velocity.x != 0){hr.boid->o_velocity.x /= hr.boid->o_neighbours;}
			if(hr.boid->o_velocity.y != 0){hr.boid->o_velocity.y /= hr.boid->o_neighbours;}
			if(hr.boid->o_velocity.z != 0){hr.boid->o_velocity.z /= hr.boid->o_neighbours;}
		}
		hr.boid->velocity += hr.boid->o_velocity;
	}
}

void Flock::setDestination(vec3 dest){
	destination = dest;
}

void Flock::arrange(Boid *node, Boid *b){
	if(node->left == nullptr){
		node->left = b;
		b->parent = node;
		return;
	}
	else if(node->right == nullptr){
		node->right = b;
		b->parent = node;
		return;
	}
	else{
		if(rand()%2){
			arrange(node->left, b);
		}
		else{
			arrange(node->right, b);
		}
	}
}

void Flock::render(){
	leader->render();

	int s = boids.size();
	for(int i = 0; i < s; ++i){
		boids[i]->render();
	}
}

void Flock::showOctTree(){
	oct_tree->renderTree(0);
}
