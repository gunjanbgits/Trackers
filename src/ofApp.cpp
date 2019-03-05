#include "ofApp.h"
#include "node.h"

using namespace ofxCv;
using namespace cv;


// HELPER FUNCTIONS

float ease(float p) {
    return 3*p*p - 2*p*p*p;
}

float ease(float p, float g) {
    if (p < 0.5)
        return 0.5 * pow(2*p, g);
    else
        return 1 - 0.5 * pow(2*(1 - p), g);
}



//--------------------------------------------------------------
void ofApp::setup(){

    //ofSetVerticalSync(true);
    ofBackground(0);
    ofSetFrameRate(60);
    
    width = ofGetWidth();
    height = ofGetHeight();
    
    //GUI SETUP
    gui.setup();
    gui.add(minArea.set("Min area", 10, 1, 100));
    gui.add(maxArea.set("Max area", 200, 1, 500));
    gui.add(threshold.set("Threshold", 168, 0, 255));
    gui.add(holes.set("Holes", false));
    gui.add(fieldOfGlow.set("RangeOfInfluence", 10, 1, 40));
    gui.add(proximity.set("Proximity", 10, 5, 40));
    
    // Particle GUI
    gui.add(timeMult.set("timeMult", .12, .005, .5));
    gui.add(scaleMult.set("ScaleMult", .25, .005, .5));
    gui.add(radius.set("Radius", .9, .1, 5));
    gui.add(numFrames.set("numFrames", 75, 60, 600));
    //gui.add(gridScale.set("gridScale", 50, 20, 80));
    gui.add(vScale.set("vScale", 0.01, 0.01, .5));
    
    //Load Movie
    
//    movie.load("keys2.mov");
//    movie.play();
//
    //CAM FEED
    movie.setDeviceID(1);
    movie.setup(1280, 768);

    contourFinder.setMinAreaRadius(1);
    contourFinder.setMaxAreaRadius(100);
    contourFinder.setThreshold(15);
    
    // wait for half a second before forgetting something
    contourFinder.getTracker().setPersistence(15);
    
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    showLabels = true;
    
    //Particle Setup
    cols = 32;//round(width / gridScale);
    rows = 22;//round(height / gridScale);
    
    null.set(0,0);
    
    flowField.resize(rows*cols);
    
    for(int i = 0; i< flowField.size(); i++){
        flowField[i] = null;
    }
    
    int num = 500;
    p.assign(num, Particle());
    resetParticles();
    
}

// Reset Particles System !!
//--------------------------------------------------------------
void ofApp::resetParticles(){
    
    attractPoints.clear();
//    for(int i = 0; i < 4; i++){
//        attractPoints.push_back( ofPoint( ofMap(i, 0, 4, 100, ofGetWidth()-100) , ofRandom(100, ofGetHeight()-100) ) );
//    }
    
    attractPointsWithMovement = contPoints;
    
    for(unsigned int i = 0; i < p.size(); i++){
        p[i].setAttractPoints(&attractPointsWithMovement);
        p[i].reset();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    contPoints.clear();
    
    // Flow Fields Setup
    
//    cols = round(width / gridScale);
//    rows = round(height / gridScale);
    
    // ------------------
    
    
    // Contour FInder Setup Area !!
    
    // Video Frames Updates
    //movie.update();
    movie.update();
    if(movie.isFrameNew()) {
        blur(movie, 10);
        contourFinder.setMinAreaRadius(minArea);
        contourFinder.setMaxAreaRadius(maxArea);
        contourFinder.setThreshold(threshold);
        contourFinder.findContours(movie);
        contourFinder.setSimplify(true);
        contourFinder.setFindHoles(holes);
        
        int n = contourFinder.size();
        quads.clear();
        quads.resize(n);
        for(int i = 0; i < n; i++) {
            quads[i] = contourFinder.getFitQuad(i);
        }
    }
    
    
    // ------------------
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    // Flow Field Setup Area !!
    float t = timeMult*ofGetFrameNum()/numFrames;
    //int t = 1;
    for (int y = 0; y < rows; y++){
        for (int x = 0; x < cols; x++){
            int index = x + y * cols;
            float noiseF = ofMap(ofNoise(scaleMult*x,scaleMult*y,t),0,1,0,1);
            float noiseF2 = ofMap(ofNoise(scaleMult*x,scaleMult*y,t),0,1,-1,1);
            
            float vx = 40*cos(noiseF);
            float vy = 40*sin(noiseF);
            float intensity = noiseF2; //0.5 + 1.5 * ease(noiseF2,3.5);
            vx*=intensity;
            vy*=intensity;
            ofPoint flow;
            flow.x = vx;
            flow.y = vy;
            flow.limit(vScale);
            //flow.normalize();
            flowField[index] = flow;
            ofPushMatrix();
            ofTranslate(x+x*40, y+y*40);
            ofSetColor(55);
            ofDrawLine(0,0,vx,vy);
            //ofDrawBitmapString(ofToString(noiseF), 0, 0);
            ofPopMatrix();
        }
    }
    
    
    // Particle Drawing Area !!
    
    ofSetColor(255, 127);
    
    for(unsigned int i = 0; i < p.size(); i++){
        
        p[i].follow(flowField, 40, cols);
        p[i].update();
        p[i].draw();
        p[i].edges();
    }
    
    // Countour Finder Area !!
    
    ofSetBackgroundAuto(showLabels);
    RectTracker& tracker = contourFinder.getTracker();
    

    if(showLabels) {
        ofSetColor(255);
        //movie.draw(0,0);
        ofDrawLine(10, 10, 100, 10);
        //movie.draw(0,0, 1280, 768);
        contourFinder.draw();
        for(int i = 0; i < contourFinder.size(); i++) {
            ofPoint center = toOf(contourFinder.getCenter(i));
            
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            //ofDrawCircle(circleCenter, circleRadius);
            
            ofPushMatrix();
            ofTranslate(center.x, center.y);
            int label = contourFinder.getLabel(i);
            string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label)) + ":" + ofToString(i);
            ofDrawBitmapString(msg, 0, -20);
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            ofScale(5, 5);
            ofDrawLine(0, 0, velocity.x, velocity.y);
            ofPopMatrix();
            contPoints.push_back(center);
            
            // Proximity Check
                for(int j = 0; j<contourFinder.size(); j++){
                        ofPoint centerProximity = toOf(contourFinder.getCenter(j));
                    
                        float circleProximityRadius;
                        ofVec2f circleProximityCenter = toOf(contourFinder.getMinEnclosingCircle(j, circleProximityRadius));
                        float distance = center.distance(circleProximityCenter);
                    
                        if(distance < circleRadius + circleProximityRadius + proximity && i != j){
                            float midX = (center.x+circleProximityCenter.x)/2;
                            float midY = (center.y+circleProximityCenter.y)/2;
                            ofSetColor(cyanPrint);
                            ofNoFill();
                            ofDrawCircle(midX, midY, distance);
                            //string location = ofToString(i) + ":" + ofToString(j);
                            ofDrawBitmapStringHighlight("Such Proximity", midX, midY, ofColor(cyanPrint), ofColor(255));
                            ofSetColor(255);
                        }
                    }
            //Proximity Check End
            
            }
            
        }
    else {
        ofSetColor(255);
        movie.draw(0,0);
        for(int i = 0; i < contourFinder.size(); i++) {
            unsigned int label = contourFinder.getLabel(i);
            // only draw a line if this is not a new label
            if(tracker.existsPrevious(label)) {
                // use the label to pick a random color
                ofSeedRandom(label << 24);
                ofSetColor(ofColor::fromHsb(ofRandom(255), 255, 255));
                // get the tracked object (cv::Rect) at current and previous position
                const cv::Rect& previous = tracker.getPrevious(label);
                const cv::Rect& current = tracker.getCurrent(label);
                // get the centers of the rectangles
                ofVec2f previousPosition(previous.x + previous.width / 2, previous.y + previous.height / 2);
                ofVec2f currentPosition(current.x + current.width / 2, current.y + current.height / 2);
                ofDrawLine(previousPosition, currentPosition);
            }
        }
    }

    
    //Quads
    ofNoFill();
    ofSetColor(magentaPrint);
    for(int i = 0; i < quads.size(); i++) {
        toOf(quads[i]).draw();
    }
    
    ofSetColor(255);
    //polyline drawing
    //ofPolyline shape(contPoints);
    ofSetLineWidth(2);
    //shape.draw();
    
    
   
    /*
    // Node Drawing Array object Creation and Destruction
    
    myNodes.clear();
    for (int i = 0; i<contourFinder.size(); i++){
        float circleRadius;
        ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
        Node myNode;
        myNode.setup(circleCenter, circleRadius, fieldOfGlow);
        myNodes.push_back(myNode);
    }
    
    for(int i = 0; i<myNodes.size(); i++){
        myNodes[i].draw();
    }
    */
    
    
    // Particle Drawing Area
    
    
    
    attractPointsWithMovement = contPoints;
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 20, 20);
    // Gui Draw
    gui.draw();
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    
    if(key == ' ') {
        showLabels = !showLabels;
        resetParticles();
    }
    
    
}

