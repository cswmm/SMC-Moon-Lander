
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   < Your Name goes Here >
//  Date: <date of last version>


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);


	cam.setDistance(10);
	cam.setNearClip(1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	//ofEnableSmoothing();
	ofEnableDepthTest();

	fixedCam1.setPosition(-100, 200, -550);
	fixedCam1.lookAt(glm::vec3(0, 0, 0));

	fixedCam2.setPosition(550, 250, 250);
	fixedCam2.lookAt(glm::vec3(0, 0, 250));

	fixedCam3.setPosition(-450, 400, 450);
	fixedCam3.lookAt(glm::vec3(0, 0, 0));

	// setup rudimentary lighting 
	//
	star.setPosition(0, 500, 1000);
	ofFloatColor sunColor(0.8, 0.8, 1, 1.0);
	star.setDiffuseColor(sunColor);
	ofFloatColor spec(1, 1, 1, 1);
	star.setSpecularColor(spec);
	star.setAmbientColor(ofFloatColor(0.8, 0.2, 0.8, 1));
	star.setSpotlight();
	star.setSpotlightCutOff(45);
	star.setSpotConcentration(2);

	fill.setPointLight();

	fill.setPosition(0, 500, -1000);
	

	ofFloatColor fillColor(1, 0.7, 0.7, 1.0);
	fill.setDiffuseColor(fillColor);
	fill.setSpecularColor(ofFloatColor(0.5, 0.4, 0.4, 1.0));
	fill.setAttenuation(1.0, 0, 0.0000005);

	rim.setup();
	rim.setSpotlight();
	rim.setScale(.05);
	rim.setSpotlightCutOff(90);
	rim.setAttenuation(0.5, 0, .0000005);
	rim.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rim.setDiffuseColor(ofFloatColor(0.1, 0.1, 0.1));
	rim.setSpecularColor(ofFloatColor(0.1, 0.1, 0.1));
	rim.setPosition(1000, 300, 0);
	rim.rotate(90, glm::vec3(0, 1, 0));

	initLightingAndMaterials();

	mars.loadModel("geo/terrain.obj");
	cout << mars.getMeshCount() << endl;
	mars.setScaleNormalization(false);

	player.model.loadModel("geo/rocket.obj");
	player.model.setScaleNormalization(false);
	player.setPosition(1, 15, 0);

	bLanderLoaded = true;
	for (int i = 0; i < player.model.getMeshCount(); i++) {
		bboxList.push_back(Octree::meshBounds(player.model.getMesh(i)));
	}

	cout << "Mesh Count: " << player.model.getMeshCount() << endl;

	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(bTimingInfo.setup("Record timing info", true));
	bHide = false;

	gui.add(camSelection.setup("Camera Selection: ", 4, 0, 4));

	//  Create Octree for testing.
	//

	if (bTimingInfo) {
		uint64_t start = ofGetElapsedTimeMillis();
		octree.create(mars.getMesh(0), 20);
		uint64_t end = ofGetElapsedTimeMillis();
		cout << "Time to build the tree: " << end - start << " ms" << endl;
	} else {
		octree.create(mars.getMesh(1), 20);
	}
	
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	boxColors.push_back(ofColor::red);
	boxColors.push_back(ofColor::orange);
	boxColors.push_back(ofColor::yellow);
	boxColors.push_back(ofColor::green);
	boxColors.push_back(ofColor::blue);
	boxColors.push_back(ofColor::indigo);
	boxColors.push_back(ofColor::violet);

	bMovingLanderUp = false;
	ofNoFill();

	float turbulence = 20;
	turbulenceForce = new TurbulenceForce(glm::vec3(-turbulence, -turbulence, -turbulence),
		glm::vec3(float(turbulence), float(turbulence), float(turbulence)));
	bottomThruster.sys->addForce(turbulenceForce);
	backThruster.sys->addForce(turbulenceForce);

	bottomThruster.groupSize = 1;
	backThruster.groupSize = 1;

	radialForce = new ImpulseRadialForce(120.0);
	explosionEmitter.sys->addForce(radialForce);
	explosionEmitter.setVelocity(ofVec3f(0, 0, 0));
	explosionEmitter.setEmitterType(RadialEmitter);
	explosionEmitter.oneShot = true;
	explosionEmitter.groupSize = 200;
	explosionEmitter.setLifespan(0.9);

	bottomThruster.start();
	backThruster.start();

	craterLanding.setPosition(210, -20, 280);
	craterLanding.setRadius(150);
	craterLanding.setHeight(30);

	hillLanding.setPosition(290, 160, -300);
	hillLanding.setRadius(120);
	hillLanding.setHeight(30);

	flatLanding.setPosition(-310, 80, 320);
	flatLanding.setRadius(100);
	flatLanding.setHeight(30);

	holeLanding.setPosition(-60, -80, -90);
	holeLanding.setRadius(70);
	holeLanding.setHeight(30);


	thrusterFuelLimit = 120000; //2 minutes

	landingAreas = { craterLanding, hillLanding, flatLanding, holeLanding };

	int l = ofRandom(4);

	cout << "Landing: " << l << endl;
	landing = &landingAreas[l];
	cout << "landing pos: " << landing->getPosition().x << " " << landing->getPosition().z << endl;

	engineThrust.load("sounds/thrust.mp3");
	engineThrust.setLoop(true);
	playerDeath.load("sounds/astdestroy.mp3");

	background.load("images/house.jpg");
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	if (player.alive and (player.bwdPressed || player.fwdPressed || player.leftPressed || player.rightPressed)) {
		backThruster.active = true;
		backThruster.setVelocity(-player.getHeadingD() * 10);
	}

	if (player.alive and player.upPressed) {
		bottomThruster.active = true;
	}

	if (player.alive and (!player.bwdPressed && !player.fwdPressed && !player.leftPressed && !player.rightPressed)) {
		backThruster.active = false;
	}

	if (player.alive and !player.upPressed) {
		bottomThruster.active = false;
	}

	bottomThruster.setPosition(player.getPosition());
	bottomThruster.update();

	backThruster.setPosition(player.getBack());
	backThruster.update();

	explosionEmitter.update();

	landing->integrate();

	ofVec3f min = player.model.getSceneMin() + player.getPosition();
	ofVec3f max = player.model.getSceneMax() + player.getPosition();

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	glm::vec3 pos = player.getPosition();
	TreeNode nodeRet;
	octree.intersect(Ray(Vector3(pos.x, pos.y, pos.z), Vector3(pos.x, -1000, pos.z)), octree.root, nodeRet);
	agi = pos.y - nodeRet.box.parameters->y();

	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	if (colBoxList.size() >= 10) {

		glm::vec3 n = glm::vec3(0, 0, 0);
		for (int i = 0; i < colBoxList.size(); i++) {
			Box currBox = colBoxList[i];
			
			Vector3 center = currBox.center();

			glm::vec3 force = player.getCenter() - glm::vec3(center.x(), center.y(), center.z());

			n += glm::normalize(force);
		}
		n = glm::normalize(n);

		glm::vec3 v = player.velocity;

		float maxVelocity = abs(player.gravity);
		if (glm::length(v) > maxVelocity) {
			explosionEmitter.setPosition(player.getPosition());
			explosionEmitter.start();
			explosionEmitter.setVelocity(ofVec3f(10, 10, 10));
			cout << "EXPLODED!" << endl;

			player.crash();
		} else {
			glm::vec2 lpos = glm::vec2(landing->getPosition().x, landing->getPosition().z);
			glm::vec2 ppos = glm::vec2(player.getPosition().x, player.getPosition().z);
			if (agi < 3 && glm::distance(lpos, ppos) < landing->radius) {
				player.alive = false;
				cout << "LANDED SAFE!" << endl;
			}
		}

		float mag = glm::dot(n, glm::vec3(v.x, v.y, v.z));
		float resolution = 0.2f;

		if (mag < 0) { 
			glm::vec3 p = (resolution + 1) * -mag * n;
			player.velocity = ofVec3f(p.x, p.y, p.z);
		}
	}

	player.integrate();
	landerCam.setPosition(player.getPosition());
	landerCam.lookAt(landing->getPosition());


	if (player.alive) {
		if (thrusterFuelLimit <= 0) {
			player.alive = false;
			thrusterFuelLimit = 0;
			return;
		}
		if (player.fwdPressed || player.bwdPressed || player.leftPressed || player.rightPressed || player.upPressed) {
			thrusterFuelLimit -= 1000 / ofGetFrameRate();
		}
	}

}
//--------------------------------------------------------------
void ofApp::draw() {

	background.draw(ofGetScreenWidth(), ofGetScreenHeight());

	

	ofDrawBitmapString("Fuel Left: " + to_string((int)std::round(thrusterFuelLimit/1000)) + " seconds", (ofGetWindowWidth() / 2)-50, 25);
	ofDrawBitmapString("AGI: " + to_string(agi), (ofGetWindowWidth() / 2) - 50, 50);

	switch (camSelection) {
	case 0:
		fixedCam1.begin();
		break;
	case 1:
		fixedCam2.begin();
		break;
	case 2:
		fixedCam3.begin();
		break;
	case 3:
		if (bLanderLoaded) {
			landerCam.begin();
			break;
		}
	default:
		cam.begin();
		break;
	}

	ofPushMatrix();
	if (bWireframe) { // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			player.model.drawWireframe();
			if (!bTerrainSelected) drawAxis(player.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	} else {
		ofEnableLighting(); // shaded mode
		star.enable();
		fill.enable();
		rim.enable();
		mars.drawFaces();

		ofMesh mesh;
		player.draw();

		if (bDisplayBBoxes) {
			ofNoFill();
			ofSetColor(ofColor::white);
			for (int i = 0; i < player.model.getNumMeshes(); i++) {
				ofPushMatrix();
				ofMultMatrix(player.model.getModelMatrix());
				ofRotate(-90, 1, 0, 0);
				Octree::drawBox(bboxList[i]);
				ofPopMatrix();
			}
		}

		if (bLanderSelected) {

			ofVec3f min = player.model.getSceneMin() + player.getPosition();
			ofVec3f max = player.model.getSceneMax() + player.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
			ofSetColor(ofColor::white);
			Octree::drawBox(bounds);

			// draw colliding boxes
			//
			ofSetColor(ofColor::lightBlue);
			for (int i = 0; i < colBoxList.size(); i++) {
				Octree::drawBox(colBoxList[i]);
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	if (bDisplayPoints) { // display points as an option
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}

	// recursively draw octree
	//
	star.draw();
	fill.draw();
	rim.draw();

	star.disable();
	fill.disable();
	rim.disable();
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
	} else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0, boxColors);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
		//cout << "selected point: " << p << endl;
	}

	bottomThruster.draw();
	backThruster.draw();
	explosionEmitter.draw();

	/* craterLanding.draw();
	hillLanding.draw();
	flatLanding.draw();
	holeLanding.draw();*/
	landing->draw();
	ofPopMatrix();

	switch (camSelection) {
	case 0:
		fixedCam1.end();
		break;
	case 1:
		fixedCam2.end();
		break;
	case 2:
		fixedCam3.end();
		break;
	case 3:
		landerCam.end();
		break;
	default:
		cam.end();
		break;
	}

	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);
}

// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled())
			cam.disableMouseInput();
		else
			cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		cam.lookAt(player.getPosition());
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'r':
		player.setPosition(1, 15, 0);
		player.acceleration = glm::vec3(0);
		player.velocity = glm::vec3(0);
		player.rotVel = glm::vec3(0);
		player.rotation = glm::vec3(0);
		player.alive = true;
		player.gravity = -4.3f;

		explosionEmitter.started = false;
		explosionEmitter.fired = false;
		//cam.reset();
		break;
	case 'p':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
	{
		if (colBoxList.size() >= 10) bMovingLanderUp = true;
		break;
	}
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'm':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case ' ':
		player.upPressed = true;
		break;
	case OF_KEY_UP:
	case 'W':
	case 'w':
		player.fwdPressed = true;
		break;
	case OF_KEY_LEFT:
	case 'A':
	case 'a':
		player.leftPressed = true;
		break;
	case OF_KEY_DOWN:
	case 'S':
	case 's':
		player.bwdPressed = true;
		break;
	case OF_KEY_RIGHT:
	case 'D':
	case 'd':
		player.rightPressed = true;
		break;

	// for testing rotation
	//case 'i':
	//	player.rotation.x += 5;
	//	break;
	//case 'j':
	//	player.rotation.z -= 5;
	//case 'k':
	//	player.rotation.x -= 5;
	//	break;
	//case 'l':
	//	player.rotation.z += 5;
	//	break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case ' ':
		player.upPressed = false;
		break;
	case OF_KEY_UP:
	case 'W':
	case 'w':
		player.fwdPressed = false;
		break;
	case OF_KEY_LEFT:
	case 'A':
	case 'a':
		player.leftPressed = false;
		break;
	case OF_KEY_DOWN:
	case 'S':
	case 's':
		player.bwdPressed = false;
		break;
	case OF_KEY_RIGHT:
	case 'D':
	case 'd':
		player.rightPressed = false;
		break;
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = player.model.getSceneMin() + player.getPosition();
		ofVec3f max = player.model.getSceneMax() + player.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(player.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		bool rayIntersected;
		if (bTimingInfo) {
			uint64_t start = ofGetElapsedTimeMillis();
			rayIntersected = raySelectWithOctree(p);
			uint64_t end = ofGetElapsedTimeMillis();
			cout << "Time to search data with ray intersection: " << end - start << " ms" << endl;
		} else {
			rayIntersected = raySelectWithOctree(p);
		}
		if (rayIntersected) {
			ofDrawSphere(p, 100);
		}
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = player.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		landerPos += delta;
		player.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = player.model.getSceneMin() + player.getPosition();
		ofVec3f max = player.model.getSceneMax() + player.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/
	} else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (player.model.loadModel(dragInfo.files[0])) {
		player.model.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		player.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < player.model.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(player.model.getMesh(i)));
		}

		cout << "Mesh Count: " << player.model.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (player.model.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		player.model.setScaleNormalization(false);
		player.setPosition(0, 0, 0);
		cout << "number of meshes: " << player.model.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < player.model.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(player.model.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = player.model.getSceneMin();
			glm::vec3 max = player.model.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			player.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
