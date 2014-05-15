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

void ofApp::sendBundle() {
	osc.sendBundle(bundle);
}

//--------------------------------------------------------------
void ofApp::setup(){
    camWidth = 1280;
    camHeight = 720;
    
    environs_refresh_rate = 1;

    
    #ifdef _USE_LIVE_VIDEO
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
        vidGrabber.setDeviceID(0);
        vidGrabber.setDesiredFrameRate(60);
        vidGrabber.setUseTexture(false);
        vidGrabber.initGrabber(camWidth,camHeight);
    #else
        vidPlayer.loadMovie("2014-05-12-empirecapture.mp4");
        vidPlayer.play();
    #endif
    
    cvImg.allocate(camWidth, camHeight);
    ofSetVerticalSync(true);
    host = "localhost";
    port = 1337;
    osc.setup(host, port);
    totalPixels = camWidth*camHeight*3;
    
    skySample = 4000;
    lowerSkySample = 400000;
    lowerLightsSample = 640800;
    upperLightsSample = 840800;
    threshold = 30;
    
    // Start Timers on setup
    environsTimer.setStartTime();
    cvImg.allocate(camWidth,camHeight);
    cvOld.allocate(camWidth,camHeight);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofBackground(100,100,100);
	
    bool bNewFrame = false;
    
    #ifdef _USE_LIVE_VIDEO
        vidGrabber.update();
        bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.update();
        bNewFrame = vidPlayer.isFrameNew();
    #endif
    
	if (bNewFrame) {
        clearBundle();

        #ifdef _USE_LIVE_VIDEO
            unsigned char * pixels = vidGrabber.getPixels();
        #else
            unsigned char * pixels = vidPlayer.getPixels();
        #endif

        if (flashX && flashY && flashWidth && flashHeight) {
            cvImg.resetROI();
            cvImg.setFromPixels(pixels, camWidth, camHeight);
            
            cvImg.setROI(flashX, flashY, flashWidth, flashHeight);
            
            cvOld.setROI(flashX, flashY, flashWidth, flashHeight);
            
            cvCurrentROI.allocate(flashWidth,flashHeight);
            cvOldROI.allocate(flashWidth, flashHeight);
            cvDiff.allocate(flashWidth,flashHeight);
            cvCurrentROI.setFromPixels(cvImg.getRoiPixels(), flashWidth, flashHeight);
            cvOldROI.setFromPixels(cvOld.getRoiPixels(), flashWidth, flashHeight);
            
            cvDiff.absDiff(cvOldROI, cvCurrentROI);
            cvDiff.threshold(threshold);

            contourFinder.findContours(cvDiff, 1, 1000, 10, true);
//            cout << contourFinder.nBlobs << endl;
            
            cvOld = cvImg;
            
            if (contourFinder.nBlobs > 0) {
                
                string* flashes [contourFinder.nBlobs];
                
                for (int i = 0; i < contourFinder.nBlobs; i++){
                    // draw over the centroid if the blob is a hole
                    if(contourFinder.blobs[i].hole){
                        int x = contourFinder.blobs[i].boundingRect.getCenter().x;
                        int y = contourFinder.blobs[i].boundingRect.getCenter().y;
                        string tmp = ofToString(x);
                        tmp += ",";
                        tmp += ofToString(y);
                        flashes[i] = &tmp;
                    }
                }
                cout << flashes << endl;
                cout << "end"<<endl;
            }
        }

        // Clear the bundle to get ready to send info to OSC

        if (environsTimer.getElapsedSeconds() >= environs_refresh_rate) {
            sky = ofFloatColor(pixels[skySample*3]/255.f, pixels[skySample*3+1]/255.f, pixels[skySample*3+2]/255.f);
            lowerSky = ofFloatColor(pixels[lowerSkySample*3]/255.f, pixels[lowerSkySample*3+1]/255.f, pixels[lowerSkySample*3+2]/255.f);
            
            addMessage("/sky",string(ofToHex(lowerSky.getHex()))+","+string(ofToHex(sky.getHex())));
            
            lowerLights = ofFloatColor(pixels[lowerLightsSample*3]/255.f, pixels[lowerLightsSample*3+1]/255.f, pixels[lowerLightsSample*3+2]/255.f);
            upperLights = ofFloatColor(pixels[upperLightsSample*3]/255.f, pixels[upperLightsSample*3+1]/255.f, pixels[upperLightsSample*3+2]/255.f);
            
            addMessage("/lights",string(ofToHex(lowerLights.getHex()))+","+string(ofToHex(upperLights.getHex())));

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
    #ifdef _USE_LIVE_VIDEO
        vidGrabber.draw(0,0);
    #else
          vidPlayer.draw(0,0);
    #endif

  }
  
  
    // draw the location of the sky sample
    ofSetColor(255,0,0,255);
    ofRect(skySample%camWidth,skySample/camWidth,4,4);
    ofSetColor(255,128,0,255);
    ofRect(lowerSkySample%camWidth,lowerSkySample/camWidth,4,4);
    

    // draw the location of the lights sample
    ofSetColor(0,255,0,255);
    ofRect(lowerLightsSample%camWidth,lowerLightsSample/camWidth,4,4);
    ofSetColor(0,255,255,255);
    ofRect(upperLightsSample%camWidth,upperLightsSample/camWidth,4,4);

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
            selectMode = "lowerSky";
            break;
        case '2':
            selectMode = "sky";
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
        case '=': // the '+' button without shift
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 2) threshold = 1;
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
    } else if (selectMode == "lowerSky") {
        lowerSkySample = y*camWidth+x;
    } else if (selectMode == "lowerLights") {
        lowerLightsSample = y*camWidth+x;
    } else if (selectMode == "upperLights") {
        upperLightsSample = y*camWidth+x;
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
