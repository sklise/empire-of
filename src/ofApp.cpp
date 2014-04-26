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
    
  vidGrabber.setDeviceID(5);
  vidGrabber.setDesiredFrameRate(60);
  vidGrabber.initGrabber(camWidth,camHeight);
  videoTexture.allocate(camWidth,camHeight, GL_RGB);
  ofSetVerticalSync(true);
  host = "localhost";
  port = 1337;
  osc.setup(host, port);
  skySample = 4000;
  lightsSample = 640800;
}

//--------------------------------------------------------------
void ofApp::update(){
  clearBundle();
	ofBackground(100,100,100);
	
	vidGrabber.update();
	
	if (vidGrabber.isFrameNew()){
		int totalPixels = camWidth*camHeight*3;

		unsigned char * pixels = vidGrabber.getPixels();
    sky = ofFloatColor(vidGrabber.getPixels()[skySample*3]/255.f,
                  vidGrabber.getPixels()[skySample*3+1]/255.f,
                  vidGrabber.getPixels()[skySample*3+2]/255.f);
    lights = ofFloatColor(vidGrabber.getPixels()[lightsSample*3]/255.f,
                          vidGrabber.getPixels()[lightsSample*3+1]/255.f,
                          vidGrabber.getPixels()[lightsSample*3+2]/255.f);
    addMessage("/lights",lights);
    addMessage("/sky",sky);
	}
  sendBundle();
}

//--------------------------------------------------------------
void ofApp::draw(){
  if (showVideo) {
    ofSetHexColor(0xffffff);
    vidGrabber.draw(0,0);
    videoTexture.draw(camWidth,0,camWidth,camHeight);
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

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  switch (key) {
    case 32:
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
