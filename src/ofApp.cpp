#include "ofApp.h"
#include "node.h"

using namespace ofxCv;
using namespace cv;



//--------------------------------------------------------------
void ofApp::setup(){

    //ofSetVerticalSync(true);
    ofBackground(0);
    
    
    //GUI SETUP
    gui.setup();
    gui.add(minArea.set("Min area", 10, 1, 100));
    gui.add(maxArea.set("Max area", 200, 1, 500));
    gui.add(threshold.set("Threshold", 128, 0, 255));
    gui.add(holes.set("Holes", false));
    gui.add(fieldOfGlow.set("RangeOfInfluence", 10, 1, 40));
    gui.add(proximity.set("Proximity", 10, 5, 40));
    
    //Load Movie
    
    movie.load("keys2.mov");
    movie.play();
//
    //CAM FEED
//    movie.setDeviceID(1);
//    movie.setup(1280, 768);

    contourFinder.setMinAreaRadius(1);
    contourFinder.setMaxAreaRadius(100);
    contourFinder.setThreshold(15);
    
    // wait for half a second before forgetting something
    contourFinder.getTracker().setPersistence(15);
    
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    showLabels = true;
    
}

//--------------------------------------------------------------
void ofApp::update(){

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
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofSetBackgroundAuto(showLabels);
    RectTracker& tracker = contourFinder.getTracker();
    
    vector<glm::vec3> contPoints;

    if(showLabels) {
        ofSetColor(255);
        movie.draw(0,0);
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
    ofPolyline shape(contPoints);
    ofSetLineWidth(2);
    shape.draw();
    
    
   
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
    
    
    // Gui Draw
    gui.draw();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    
    if(key == ' ') {
        showLabels = !showLabels;
    }
    
    
}

