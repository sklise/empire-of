#include "ofApp.h"
#include "ofUtils.h"

void ofApp::clearBundle() {
    bundle.clear();
}

// Set up templates for sending OSC messages. This is becuase I don't feel like figuring out Redis in C++
template <>
void ofApp::addMessage(string address, string data) {
    ofxOscMessage msg;
    msg.setAddress(address);
    msg.addStringArg(data);
    bundle.addMessage(msg);
}

void ofApp::sendBundle() {
	osc.sendBundle(bundle);
}

//--------------------------------------------------------------
void ofApp::setup(){
    camWidth = 1280;
    camHeight = 720;
    host = "localhost";
    port = 1337;
    totalPixels = camWidth*camHeight*3;
    osc.setup(host, port);

    // Set up the camera. Either from a video or from a live camera feed
    #ifdef _USE_LIVE_VIDEO
        //we can now get back a list of devices.
        vector<ofVideoDevice> devices = vidGrabber.listDevices();

        // Log out all of the devices so we can see which one to try and
        // connect to.
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
        // Load a movie, in fact, load this movie.
        vidPlayer.loadMovie("2014-05-12-empirecapture.mp4");
        vidPlayer.play();
    #endif

    // Start Timers on setup
    environsTimer.setStartTime();

    // Allocate space for two openCV images
    cvImg.allocate(camWidth,camHeight);
    cvOld.allocate(camWidth,camHeight);

    // Initial settings for sampling variables
    skySample = 4000;
    lowerSkySample = 400000;
    lowerLightsSample = 640800;
    upperLightsSample = 840800;
    threshold = 5;
    environs_refresh_rate = 1;

}

//--------------------------------------------------------------
void ofApp::update(){
    ofBackground(100,100,100);

    bool bNewFrame = false;

    // See if there is a new frame.
    #ifdef _USE_LIVE_VIDEO
        vidGrabber.update();
        bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.update();
        bNewFrame = vidPlayer.isFrameNew();
    #endif

    // Now if there is a new frame do all of the stuff
    if (bNewFrame) {
        // Clear the bundle to get ready to send info to OSC
        clearBundle();

        unsigned char * pixels;
        // Get the pixels
        #ifdef _USE_LIVE_VIDEO
            pixels = vidGrabber.getPixels();
        #else
            pixels = vidPlayer.getPixels();
        #endif

        if (flashX && flashY && flashWidth && flashHeight) {
            cvImg.resetROI();
            cvOld.resetROI();

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

            cvOld = cvImg;
        }

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

        if (contourFinder.nBlobs > 0) {
            flashes.str("");
            flashes << "{\"count\":" << contourFinder.nBlobs << ","
                    << "\"points\":[";
            for (int i = 0; i < contourFinder.nBlobs; i++){
                ofSetColor(255);
                contourFinder.blobs[i].draw(0,0);
                flashes << "["
                    << contourFinder.blobs[i].boundingRect.getCenter().x
                    << ","
                    << contourFinder.blobs[i].boundingRect.getCenter().y
                    << "]";
                if (i + 1 < contourFinder.nBlobs) {
                    flashes << ",";
                }
            }
            flashes << "]}";
            addMessage("/flashes",flashes.str());
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
    stringstream reportStr;
    reportStr << "threshold " << threshold << endl
                << "lowerSky" << lowerSky << endl;
    ofDrawBitmapString(reportStr.str(),20,40 );
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
