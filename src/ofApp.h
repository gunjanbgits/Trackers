#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "particle.h"

using namespace ofxCv;
using namespace cv;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void resetParticles();
		void keyPressed(int key);
    
    
        //float threshold;
        //ofVideoPlayer movie;
        ofVideoGrabber movie;
        ofxCv::ContourFinder contourFinder;
        vector< vector<cv::Point> > quads;
        vector<int> sides;
        bool showLabels;
        vector<ofPoint> contPoints;
    
    
        ofxPanel gui;
        ofParameter<float> minArea, maxArea, threshold, fieldOfGlow, proximity;
        ofParameter<bool> holes;
    
//      vector <Node> myNodes;
    
    //  Particle Setup
        vector <Particle> p;
        vector <ofPoint> attractPoints;
        vector <ofPoint> attractPointsWithMovement;
    
        vector <ofPoint> flowField;
    
        ofPoint null;
    
        int width, height, rows, cols;
    
        ofParameter<int> numFrames, gridScale;
        ofParameter<float> timeMult, scaleMult, radius, vScale;

    
    
//        void keyReleased(int key);
//        void mouseMoved(int x, int y );
//        void mouseDragged(int x, int y, int button);
//        void mousePressed(int x, int y, int button);
//        void mouseReleased(int x, int y, int button);
//        void mouseEntered(int x, int y);
//        void mouseExited(int x, int y);
//        void windowResized(int w, int h);
//        void dragEvent(ofDragInfo dragInfo);
//        void gotMessage(ofMessage msg);
		
};
