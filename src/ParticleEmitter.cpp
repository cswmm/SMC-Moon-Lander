
//  Kevin M. Smith - CS 134 SJSU

#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter() {
	sys = new ParticleSystem();
	createdSys = true;
	init();
}

ParticleEmitter::ParticleEmitter(ParticleSystem * s) {
	if (s == NULL) {
		cout << "fatal error: null particle system passed to ParticleEmitter()" << endl;
		ofExit();
	}
	sys = s;
	createdSys = false;
	init();
}

ParticleEmitter::~ParticleEmitter() {

	// deallocate particle system if emitter created one internally
	//
	if (createdSys) delete sys;
}

void ParticleEmitter::init() {
	rate = 20;
	velocity = ofVec3f(0, -5, 0);
	lifespan = 3;
	started = false;
	lastSpawned = 0;
	radius = 1;
	particleRadius = .1;
	visible = false;
	type = DirectionalEmitter;
}

void ParticleEmitter::draw() {
	if (visible) {
		switch (type) {
		case DirectionalEmitter:
			ofDrawSphere(position, radius / 10); // just draw a small sphere for point emitters
			break;
		case SphereEmitter:
		case RadialEmitter:
			ofDrawSphere(position, radius / 10); // just draw a small sphere as a placeholder
			break;
		default:
			break;
		}
	}
	sys->draw();
}
void ParticleEmitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void ParticleEmitter::stop() {
	started = false;
}

void ParticleEmitter::update() {

	if (!started) return;

	float time = ofGetElapsedTimeMillis();

	if (oneShot) {
		if (!fired) {

			// spawn a new particle(s)
			//
			for (int i = 0; i < groupSize; i++)
				spawn(time);

			lastSpawned = time;
		}
		fired = true;
	} else if ((time - lastSpawned) > (1000.0 / rate) and active) {

		for (int i = 0; i < groupSize; i++)
			spawn(time);

		lastSpawned = time;
	}
	sys->update();
}


void ParticleEmitter::spawn(float time) {

	Particle particle;

	// set initial velocity and position
	// based on emitter type
	//
	switch (type) {
	case RadialEmitter: {
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		float speed = velocity.length();
		particle.velocity = dir.getNormalized() * speed;
		particle.position = position;
	} break;
	case DirectionalEmitter:
		particle.position = position;
		particle.velocity = velocity;
		break;
	}

	// other particle attributes
	//
	particle.lifespan = lifespan;
	particle.birthtime = time;
	particle.radius = particleRadius;

	// add to system
	//
	sys->add(particle);
}
