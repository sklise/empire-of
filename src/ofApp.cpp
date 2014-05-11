#include "ofApp.h"
#include "ofUtils.h"

void ofApp::clearBundle() {
	bundle.clear();
}

// Set up templates for sending OSC messages. This is becuase I don't feel like figuring out Redis in C++

template <>
void ofApp::addMessage(string address, float data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data);
	bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, string data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addStringArg(data);
    bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, int data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addIntArg(data);
	bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, ofColor data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addStringArg(string(ofToHex(data.getHex())));
    cout << string(ofToHex(data.getHex())) << endl;
    bundle.addMessage(msg);
}

void ofApp::sendBundle() {
	osc.sendBundle(bundle);
}

//--------------------------------------------------------------
void ofApp::setup(){
    camWidth = 1280;
    camHeight = 720;
    
    environs_refresh_rate = 1;

    //we can now get back a list of devices.
    vector<ofVideoDevice> devices = vidGrabber.listDevices();

    // Log out all of the devices
    for(int i = 0; i < devices.size(); i++){
    cout << devices[i].id << ": " << devices[i].deviceName;
    if( devices[i].bAvailable ){
      cout << endl;
    } else {
      cout << " - unavailable " << endl;
    }
    }

    // Normally #9 with Blackmagic
    vidGrabber.setDeviceID(5);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.setUseTexture(false);
    vidGrabber.initGrabber(camWidth,camHeight);
    cvImg.allocate(camWidth, camHeight);
    ofSetVerticalSync(true);
    host = "localhost";
    port = 1337;
    osc.setup(host, port);
    totalPixels = camWidth*camHeight*3;
    skySample = 4000;
    lightsSample = 640800;
    
    // Start Timers on setup
    environsTimer.setStartTime();
}

//--------------------------------------------------------------
void ofApp::update(){
	ofBackground(100,100,100);
	vidGrabber.update();
	
	if (vidGrabber.isFrameNew()) {
        clearBundle();

        unsigned char * pixels = vidGrabber.getPixels();
        cvImg.resetROI();
        cvImg.setFromPixels(pixels, camWidth, camHeight);
        cvImg.setROI(flashX, flashY, flashWidth, flashHeight);

        // Clear the bundle to get ready to send info to OSC

        if (environsTimer.getElapsedSeconds() >= environs_refresh_rate) {
            sky = ofFloatColor(vidGrabber.getPixels()[skySample*3]/255.f, vidGrabber.getPixels()[skySample*3+1]/255.f, vidGrabber.getPixels()[skySample*3+2]/255.f);
            lights = ofFloatColor(vidGrabber.getPixels()[lightsSample*3]/255.f, vidGrabber.getPixels()[lightsSample*3+1]/255.f, vidGrabber.getPixels()[lightsSample*3+2]/255.f);
            addMessage("/lights",lights);
            addMessage("/sky",sky);
            // Now restart the timer.
            environsTimer.setStartTime();
        }
        
        sendBundle();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
  if (showVideo) {
    ofSetHexColor(0xffffff);
    vidGrabber.draw(0,0);
  }
  
  ofSetColor(sky);
  ofRect(10, 10, 20, 20);
  ofSetColor(lights);
  ofRect(10, 30, 20, 20);
  
  // draw the location of the sky sample
  ofSetColor(255,0,0,255);
  ofRect(skySample%camWidth,skySample/camWidth,4,4);

  // draw the location of the lights sample
  ofSetColor(0,255,0,255);
  ofRect(lightsSample%camWidth,lightsSample/camWidth,4,4);

    ofNoFill();
    ofSetColor(255);
    if (flashX && flashY && isMousePressed) {

        ofRect(flashX, flashY, mouseX-flashX, mouseY-flashY);
    } else if (flashX && flashY && flashWidth && flashHeight) {
        ofRect(flashX, flashY, flashWidth, flashHeight);
    }
    ofFill();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case '1':
            selectMode = "sky";
            break;
        case '2':
            selectMode = "lowerSky";
            break;
        case '3':
            selectMode = "lowerLights";
            break;
        case '4':
            selectMode = "upperLights";
            break;
        case 'a':
            selectMode = "flash";
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    selectMode = "";
    switch (key) {
        case ' ':
            showVideo = !showVideo;
            break;
  }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (selectMode == "sky"){
        skySample = y*camWidth+x;
    } else if (selectMode == "lowerLights") {
        lightsSample = y*camWidth+x;
    } else if (selectMode == "flash" && !isMousePressed) {
        isMousePressed = true;
        flashX = x;
        flashY = y;
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    isMousePressed = false;
    if (selectMode == "flash") {
        flashWidth = x - flashX;
        flashHeight = y - flashY;
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
