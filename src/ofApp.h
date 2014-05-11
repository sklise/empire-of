#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxMSATimer.h"
#include "ofxNetwork.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
    void setup();
    void update();
    void draw();

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
    ofVideoGrabber 		vidGrabber;
    ofTexture         videoTexture;
    int camWidth;
    int camHeight;
    int totalPixels;

    // Open CV
    //-------------------------------------------------------
    ofxCvGrayscaleImage cvImg;
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

    // Integer for the number of seconds to wait before sampling the less active sections of the video.
    int environs_refresh_rate;
    ofxMSATimer environsTimer;


    // User Interface
    //-------------------------------------------------------
    bool showVideo = true;
    string selectMode;
    bool isMousePressed = false;

};
