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

  ofVideoGrabber 		vidGrabber;
  ofTexture         videoTexture;
  int camWidth;
  int camHeight;
  int totalPixels;
  
  // Integer for the number of seconds to wait before sampling the less active sections of the video.
  int environs_refresh_rate;
  ofxMSATimer environsTimer;

  // OSC stuff
  
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

  // Open CV stuff
  ofxCvColorImage       cvImg;
  ofxCvGrayscaleImage 	grayImage;
  ofxCvGrayscaleImage 	grayBg;
  ofxCvGrayscaleImage 	grayDiff;
  
  // Pixel positions for samples for the sky and lights etc
  int skySample;
  int lightsSample;
  ofColor sky;
  ofColor lights;
  
  int flashX;
  int flashY;
  int flashWidth;
  int flashHeight;

  bool showVideo = true;
  string selectMode;
  bool isMousePressed = false;

};
