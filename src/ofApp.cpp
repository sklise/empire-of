#include "ofApp.h"



void ofApp::clearBundle() {
	bundle.clear();
}

template <>
void ofApp::addMessage(string address, ofVec3f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	msg.addFloatArg(data.z);
	bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, ofVec2f data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data.x);
	msg.addFloatArg(data.y);
	bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, float data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addFloatArg(data);
	bundle.addMessage(msg);
}

template <>
void ofApp::addMessage(string address, int data) {
	ofxOscMessage msg;
	msg.setAddress(address);
	msg.addIntArg(data);
	bundle.addMessage(msg);
}

void ofApp::sendBundle() {
	osc.sendBundle(bundle);
}

//--------------------------------------------------------------
void ofApp::setup(){
    camWidth = 1280;
    camHeight = 720;
    
    //we can now get back a list of devices.
	vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    // Log out all of the devices
    for(int i = 0; i < devices.size(); i++){
		cout << devices[i].id << ": " << devices[i].deviceName;
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl;
        }
	}
    
    vidGrabber.setDeviceID(0);
	vidGrabber.setDesiredFrameRate(60);
	vidGrabber.initGrabber(camWidth,camHeight);
    videoTexture.allocate(camWidth,camHeight, GL_RGB);
	ofSetVerticalSync(true);
    host = "localhost";
    port = 1337;
    osc.setup(host, port);
}

//--------------------------------------------------------------
void ofApp::update(){
    
	ofBackground(100,100,100);
	
	vidGrabber.update();
	
	if (vidGrabber.isFrameNew()){
		int totalPixels = camWidth*camHeight*3;
		unsigned char * pixels = vidGrabber.getPixels();
	}
    clearBundle();
    addMessage("/test",1);
    sendBundle();

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetHexColor(0xffffff);
	vidGrabber.draw(20,20);
	videoTexture.draw(20+camWidth,20,camWidth,camHeight);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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
