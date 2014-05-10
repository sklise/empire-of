#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxMSATimer.h"
#include "ofxNetwork.h"

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
  ofTexture			videoTexture;
  int 				camWidth;
  int 				camHeight;
  
  int environs_refresh_rate;

  // OSC stuff
  void clearBundle();
  template <class T>
  void addMessage(string address, T data);
  void sendBundle();

  string host;
  int port;
  ofxOscSender osc;
  ofxOscBundle bundle;

  int skySample;
  int lightsSample;
  int flashTopLeft;
  int flashLength;
  int flashHeight;
  ofColor sky;
  ofColor lights;
  bool showVideo = true;
  ofxMSATimer environsTimer;
};
