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
void ofApp::loadSettings() {

	ofxXmlSettings xml;
    xml.loadFile("settings.xml");

	xml.pushTag("sky");
		skySample = xml.getValue("upper", 0);
    cout << xml.getValue("upper", 0)<<endl;
        skyLowerSample = xml.getValue("upper",0);
	xml.popTag();

    xml.pushTag("lights");
        lightsLowerSample = xml.getValue("lower",0);
        lightsUpperSample = xml.getValue("upper",0);
    xml.popTag();

    xml.pushTag("flash");
        flashX = xml.getValue("x",0);
        flashY = xml.getValue("y",0);
        flashWidth = xml.getValue("width",0);
        flashHeight = xml.getValue("height",0);
    xml.popTag();

    xml.pushTag("sampling");
        threshold = xml.getValue("threshold",5);
        environs_refresh_rate = xml.getValue("environsRefreshRate", 10);
        flashRefreshRate = xml.getValue("flashRefreshRate", 1);
    xml.popTag();
}

void ofApp::saveSettings() {
    ofxXmlSettings xml;
    xml.loadFile("settings.xml");

    xml.pushTag("sky");
        xml.setValue("lower", skyLowerSample);
        xml.setValue("upper", skySample);
    xml.popTag();

    xml.pushTag("lights");
        xml.setValue("lower", lightsLowerSample);
        xml.setValue("upper", lightsUpperSample);
    xml.popTag();

    xml.pushTag("sampling");
        xml.setValue("threshold", threshold);
        xml.setValue("environsRefreshRate", environs_refresh_rate);
    xml.popTag();


    xml.pushTag("flash");
        xml.setValue("x",flashX);
        xml.setValue("y",flashY);
        xml.setValue("width",flashWidth);
        xml.setValue("height",flashHeight);
    xml.popTag();

    xml.save("settings.xml");
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
        vidPlayer.setVolume(0);
    #endif

    // Start Timers on setup
    environsTimer.setStartTime();
    flashTimer.setStartTime();

    // Allocate space for two openCV images
    cvImg.allocate(camWidth,camHeight);
    cvGray.allocate(camWidth,camHeight);
    cvOld.allocate(camWidth,camHeight);

    // Initial settings for sampling variables
    loadSettings();
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
            cvGray.resetROI();
            cvOld.resetROI();

            cvImg.setFromPixels(pixels, camWidth, camHeight);

            cvGray = cvImg;
            cvGray.setROI(flashX, flashY, flashWidth, flashHeight);
            
            cvOld.setROI(flashX, flashY, flashWidth, flashHeight);

            cvCurrentROI.allocate(flashWidth,flashHeight);
            cvOldROI.allocate(flashWidth, flashHeight);
            cvDiff.allocate(flashWidth,flashHeight);
            cvCurrentROI.setFromPixels(cvGray.getRoiPixels(), flashWidth, flashHeight);
            cvOldROI.setFromPixels(cvOld.getRoiPixels(), flashWidth, flashHeight);

            cvDiff.absDiff(cvOldROI, cvCurrentROI);
            cvDiff.threshold(threshold);

            contourFinder.findContours(cvDiff, 1, 1000, 10, true);

            if (flashTimer.getElapsedSeconds() >= flashRefreshRate){
                cvOld = cvGray;
                flashTimer.setStartTime();
                cout << "reset sample"<<endl;
            }
        }

        if (environsTimer.getElapsedSeconds() >= environs_refresh_rate) {
            sky = ofFloatColor(pixels[skySample*3]/255.f, pixels[skySample*3+1]/255.f, pixels[skySample*3+2]/255.f);
            lowerSky = ofFloatColor(pixels[skyLowerSample*3]/255.f, pixels[skyLowerSample*3+1]/255.f, pixels[skyLowerSample*3+2]/255.f);

            addMessage("/sky",string(ofToHex(lowerSky.getHex()))+","+string(ofToHex(sky.getHex())));

            lowerLights = ofFloatColor(pixels[lightsLowerSample*3]/255.f, pixels[lightsLowerSample*3+1]/255.f, pixels[lightsLowerSample*3+2]/255.f);
            upperLights = ofFloatColor(pixels[lightsUpperSample*3]/255.f, pixels[lightsUpperSample*3+1]/255.f, pixels[lightsUpperSample*3+2]/255.f);

            addMessage("/lights",string(ofToHex(lowerLights.getHex()))+","+string(ofToHex(upperLights.getHex())));

            // Now restart the timer.
            environsTimer.setStartTime();
        }

        if (contourFinder.nBlobs > 0) {
            flashes.str("");
            flashes << "{\"count\":" << contourFinder.nBlobs << ","
                    << "\"width\":" << flashWidth << ","
                    << "\"height\":" << flashHeight << ","
                    << "\"points\":[";
            for (int i = 0; i < contourFinder.nBlobs; i++){
                ofSetColor(255);
                contourFinder.blobs[i].draw(0,0);
                flashes << "{"
                    << "\"x\":" << contourFinder.blobs[i].boundingRect.getCenter().x << ","
                    << "\"y\":" << contourFinder.blobs[i].boundingRect.getCenter().y << ","
                    << "\"area\":" << contourFinder.blobs[i].area
                    << "}";
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
    ofRect(skyLowerSample%camWidth,skyLowerSample/camWidth,4,4);


    // draw the location of the lights sample
    ofSetColor(0,255,0,255);
    ofRect(lightsLowerSample%camWidth,lightsLowerSample/camWidth,4,4);
    ofSetColor(0,255,255,255);
    ofRect(lightsUpperSample%camWidth,lightsUpperSample/camWidth,4,4);

    ofNoFill();
    ofSetColor(255);
    if (flashX && flashY && isMousePressed) {
        ofRect(flashX, flashY, mouseX-flashX, mouseY-flashY);
    } else if (flashX && flashY && flashWidth && flashHeight) {
        ofRect(flashX, flashY, flashWidth, flashHeight);
        cvDiff.draw(flashX,flashY - flashHeight);
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
        case 's':
            saveSettings();
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
        skyLowerSample = y*camWidth+x;
    } else if (selectMode == "lowerLights") {
        lightsLowerSample = y*camWidth+x;
    } else if (selectMode == "upperLights") {
        lightsUpperSample = y*camWidth+x;
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
