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
	void setPosition(glm::vec3 p) { position = p; }
	glm::vec3 getPosition() { return position; }

	virtual glm::vec3 getHeadingD() { return glm::vec3(0, 0, 0); }
	virtual glm::vec3 getHeadingP() { return glm::vec3(0, 0, 0); }

	glm::mat4 getTransform() {
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(position));
		glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		return T * R * S;
	}

	float defaultSize = 20.0;
};

class PhysicsObject : public Shape {
public:
	PhysicsObject() { }

	glm::mat4 getTransform() {
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(position));

		glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
		glm::mat4 R = Rz * Ry * Rx;

		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
		return T * R * S;
	}

	void integrate() {
		double dt = ofGetLastFrameTime();
		position += velocity * dt;
		glm::vec3 accel = acceleration + (1.0 / mass) * force;
		velocity += accel * dt;
		velocity *= damping;

		glm::vec3 angularAcc = rotAcc + (1.0f / moment) * torque;
		rotVel += angularAcc * dt;
		rotVel *= damping;
		rotation += rotVel * dt;

		force = glm::vec3(0);
		torque = glm::vec3(0);

	}

	ofVec3f velocity = glm::vec3(0, 0, 0);
	ofVec3f acceleration = glm::vec3(0, 0, 0);
	glm::vec3 force = glm::vec3(0, 0, 0);
	float damping = .995;
	float mass = 1;

	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 rotVel = glm::vec3(0.0f); 
	glm::vec3 rotAcc = glm::vec3(0.0f); 
	glm::vec3 torque = glm::vec3(0.0f); 
	float moment = 1.0f;
};

class Player : public PhysicsObject {
public:
	ofxAssimpModelLoader model;

	bool fwdPressed = false;
	bool bwdPressed = false;
	bool upPressed = false;
	bool leftPressed = false;
	bool rightPressed = false;
	bool alive = true;
	bool showHeading = true;

	float gravity = -4.3f;

	glm::vec3 headingP = glm::vec3(-5, 0, 0);

	Player() { }

	void draw() {

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
		float moveForce = 2.0f;
		float upForce = 8.0f;
		float torqueForce = 50.0f;
		float movef = 0;
		float upf = 0;
		float t = 0;
		if (alive) {
			if (upPressed) upf += 1;
			if (fwdPressed) movef += 1;
			if (bwdPressed) movef -= 1;
			if (leftPressed) t += 1;
			if (rightPressed) t -= 1;
		}

		force += (upf * glm::vec3(0, 1, 0) * upForce);
		force += (movef * getHeadingD() * moveForce);
		force += glm::vec3(0, gravity, 0);

		torque.y += t * torqueForce;

		PhysicsObject::integrate();
	}

	glm::vec3 getHeadingD() {
		return glm::normalize(glm::vec3(getTransform() * glm::vec4(headingP, 1.0)) - position);
	}

	glm::vec3 getHeadingP() {
		return glm::vec3(getTransform() * glm::vec4(headingP, 1.0));
	}

	glm::vec3 getCenter() {
		float height = model.getSceneMax().y - model.getSceneMin().y;
		return glm::vec3(position.x, position.y+(height/2), position.z);
	}

	glm::vec3 getBack() {
		return glm::vec3(getTransform() * glm::vec4(-headingP/2, 1.0));
	}

	void crash() {
		alive = false;
		force = glm::vec3(0);
		torque = glm::vec3(0);
		rotVel = glm::vec3(0);
		velocity = glm::vec3(0);
		gravity = 0;

		rotation.x = -10;
		rotation.z = 20;
	}
};

class Landing : public PhysicsObject {
public:
	Landing() { }

	ofCylinderPrimitive cylinder;
	float radius;
	float height;

	void setRadius(float r) { radius = r; }
	void setHeight(float h) { height = h; }

	void draw() {
		ofPushMatrix();
		ofMultMatrix(getTransform());

		ofSetColor(ofColor::aquamarine);
		ofSetCircleResolution(50);
		ofDrawCylinder(radius, height);

		ofPopMatrix();
	}

	void integrate() {
		float torqueForce = 150.0f*(radius/100);

		torque.y += torqueForce * sin((ofGetElapsedTimef() * 0.5)+radius);

		PhysicsObject::integrate();
	}
};
