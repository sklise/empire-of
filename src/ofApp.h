#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxMSATimer.h"
#include "ofxNetwork.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"

//#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
                            // otherwise, we'll use a movie file

class ofApp : public ofBaseApp{

public:

    void setup();
    void update();
    void draw();
	void loadSettings();
    void saveSettings();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // OSC stuff
    //-------------------------------------------------------
    // Utilities to wrap the bundled message being sent by OSC
    void clearBundle();
    void sendBundle();

    // Templates for OSC messages
    template <class T>
    void addMessage(string address, T data);

    string host;
    int port;
    ofxOscSender osc;
    ofxOscBundle bundle;

    // Camera
    //-------------------------------------------------------
    #ifdef _USE_LIVE_VIDEO
        ofVideoGrabber 		vidGrabber;
    #else
        ofVideoPlayer 		vidPlayer;
    #endif
    ofTexture         videoTexture;
    int camWidth;
    int camHeight;
    int totalPixels;

    // Open CV
    //-------------------------------------------------------
    ofxCvColorImage cvImg;
    ofxCvGrayscaleImage cvGray;
    ofxCvGrayscaleImage cvDiff;
    ofxCvGrayscaleImage cvOld;
    ofxCvGrayscaleImage cvOldROI;
    ofxCvGrayscaleImage cvCurrentROI;

    ofxCvContourFinder 	contourFinder;
    int threshold;

    // Samples
    //-------------------------------------------------------
    int skySample;
    int lightsSample;
    ofColor sky;
    ofColor lights;

    int flashX;
    int flashY;
    int flashWidth;
    int flashHeight;

    stringstream flashes;

    // Integer for the number of seconds to wait before sampling the less active sections of the video.
    int environs_refresh_rate;
    int flashRefreshRate;
    ofxMSATimer environsTimer;
    ofxMSATimer flashTimer;


    // User Interface
    //-------------------------------------------------------
    bool showVideo = true;
    string selectMode;
    bool isMousePressed = false;

};
