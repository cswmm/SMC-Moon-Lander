#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "Particle.h"
#include "ofMain.h"

//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
public:
	virtual void updateForce(Particle *) = 0;
	bool applyOnce = false;
	bool applied = false;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void remove(int);
	void update();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;

public:
	TurbulenceForce(const ofVec3f & min, const ofVec3f & max);
	void set(glm::vec3 min, glm::vec3 max) {
		tmin = min;
		tmax = max;
	}
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude;

public:
	ImpulseRadialForce(float magnitude);
	void updateForce(Particle *);
};
