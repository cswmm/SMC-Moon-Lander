#pragma once

#include "ofMain.h"
#include <ofxAssimpModelLoader.h>
#include "TransformObject.h"

// Basic Shape class supporting matrix transformations and drawing.
//
//
class Shape : public TransformObject{
public:
	Shape() { }
	virtual void draw() {

		// draw a box by defaultd if not overridden
		//
		ofPushMatrix();
		ofMultMatrix(getTransform());
		ofDrawBox(defaultSize);
		ofPopMatrix();
	}

	virtual bool inside(glm::vec3 p) {
		return false;
	}

	void setPosition(float x, float y, float z) { position = glm::vec3(x, y, z); }
	glm::vec3 getPosition() { return position; }

	virtual glm::vec3 getHeadingD() { return glm::vec3(0, 0, 0); }
	virtual glm::vec3 getHeadingP() { return glm::vec3(0, 0, 0); }

	glm::mat4 getTransform() {
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(position));
		glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 0, 1));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		return T * R * S;
	}

	float defaultSize = 20.0;
};

class PhysicsObject : public Shape {
public:
	PhysicsObject() { }

	void integrate() {
		double dt = ofGetLastFrameTime();
		position += velocity * dt;
		glm::vec3 accel = acceleration + (1.0 / mass) * force;
		velocity += accel * dt;
		velocity *= damping;

		float angularAcc = rotAcc + (1.0f / moment) * torque;
		rotVel += angularAcc * dt;
		rotVel *= damping;
		rotation += rotVel * dt;

		force = glm::vec3(0, 0, 0);
		torque = 0.0f;

		if (position.x > ofGetWidth()) {
			position.x = 0;
		} else if (position.x < 0) {
			position.x = ofGetWidth();
		}
		if (position.y > ofGetHeight()) {
			position.y = 0;
		} else if (position.y < 0) {
			position.y = ofGetHeight();
		}
	}

	ofVec3f velocity = glm::vec3(0, 0, 0);
	ofVec3f acceleration = glm::vec3(0, 0, 0);
	glm::vec3 force = glm::vec3(0, 0, 0);
	float damping = .995;
	float mass = 1;

	float rotVel = 0.0;
	float rotAcc = 0.0;
	float torque = 0.0;
	float moment = 1.0;
};

class Player : public PhysicsObject {
public:
	ofxAssimpModelLoader model;

	bool fwdPressed = false;
	bool bwdPressed = false;
	bool upPressed = false;
	bool downPressed = false;
	bool leftPressed = false;
	bool rightPressed = false;
	bool alive = true;
	bool showHeading = false;

	glm::vec3 headingP = glm::vec3(0, -100, 0);

	Player() { }
	void draw() {
		if (!alive) {
			return;
		}

		ofPushMatrix();
		ofMultMatrix(getTransform());

		model.drawFaces();

		if (showHeading) {
			ofSetColor(ofColor::white);
			ofDrawLine(glm::vec3(0, 0, 0), headingP);
		}

		ofPopMatrix();
	}

	void integrate() {
		float moveForce = 250.0f;
		float torqueForce = 200.0f;
		float f = 0;
		float t = 0;

		if (upPressed) f += 1;
		if (downPressed) f -= 1;
		if (leftPressed) t -= 1;
		if (rightPressed) t += 1;

		force += f * getHeadingD() * moveForce;
		torque += t * torqueForce;

		PhysicsObject::integrate();
	}

	glm::vec3 getHeadingD() {
		return glm::normalize(glm::vec3(getTransform() * glm::vec4(headingP, 1.0)) - position);
	}

	glm::vec3 getHeadingP() {
		return glm::vec3(getTransform() * glm::vec4(headingP, 1.0));
	}
};
