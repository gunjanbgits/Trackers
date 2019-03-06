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
    
    myFont.load("GT-Zirkon-Book.ttf", 24, true, true);

    //ofSetVerticalSync(true);
    ofBackground(0);
    ofSetFrameRate(60);
    
    width = ofGetWidth();
    height = ofGetHeight();
    modeSelector == "0";
    
    //GUI SETUP
    gui.setup();
    gui.add(minArea.set("Min area", 10, 1, 100));
    gui.add(maxArea.set("Max area", 200, 1, 500));
    gui.add(blurAmount.set("Blur", 5, 1, 100));
    gui.add(thresholdVal.set("Threshold", 168, 0, 255));
    gui.add(holes.set("Holes", false));
    gui.add(fieldOfGlow.set("RangeOfInfluence", 10, 1, 40));
    gui.add(proximity.set("Proximity", 10, 5, 40));
    
    // Particle GUI
    gui.add(timeMult.set("timeMult", .02, .005, .5));
    gui.add(scaleMult.set("ScaleMult", .02, .005, .5));
    gui.add(radius.set("Radius", .2, .1, 5));
    gui.add(numFrames.set("numFrames", 75, 60, 600));
    //gui.add(gridScale.set("gridScale", 50, 20, 80));
    gui.add(vScale.set("vScale", 0.05, 0.01, .5));
    
    //Load Movie
    
//    movie.load("keys2.mov");
//    movie.play();
    
    
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
    cols = 64;//round(width / gridScale);
    rows = 42;//round(height / gridScale);
    
    null.set(0,0);
    contPoints.push_back(null);
    
    flowField.resize(rows*cols);
    
    for(int i = 0; i< flowField.size(); i++){
        flowField[i] = null;
    }
    
    int num = 1000;
    p.assign(num, Particle());
    resetParticles();
    
}

// Reset Particles System !!
//--------------------------------------------------------------
void ofApp::resetParticles(){
    
//    repulsePoints.clear();
//    repulsePointsWithMovement.clear();
    
    attractPoints.clear();
    attractPointsWithMovement.clear();
    
    for(unsigned int i = 0; i < p.size(); i++){
        //p[i].setRepulsePoints(&repulsePointsWithMovement);
        p[i].setAttractPoints(&attractPointsWithMovement);
        p[i].reset();
    }
}


//--------------------------------------------------------------
// Create Flow Field

void ofApp::createFlowField(){
    
    // Flow Field Setup Area !!
    float t = timeMult*ofGetFrameNum()/numFrames;
    //int t = 1;
    for (int y = 0; y < rows; y++){
        for (int x = 0; x < cols; x++){
            int index = x + y * cols;
//            float noiseF = ofMap(ofNoise(scaleMult*x,scaleMult*y,t),0,1,0,1);
//            float noiseF2 = ofMap(ofNoise(scaleMult*x,scaleMult*y,t),0,1,-1,1);
            
            float nsFactor2 = ofMap(ofNoise(scaleMult*x,scaleMult*y,radius*cos(TWO_PI*t),radius*sin(TWO_PI*t)),0,1,-25,25);
            float nsFactor2a = ofMap(ofNoise(200+scaleMult*x,scaleMult*y,radius*cos(TWO_PI*t),radius*sin(TWO_PI*t)),0,1,0,1);
            float l = glm::sqrt(glm::pow(20,2)+glm::pow(20,2));
            float vx = l*cos(nsFactor2);
            float vy = l*sin(nsFactor2);
            float intensity = 0.1+1.5 * ease(nsFactor2a,3.5); //0.5 + 1.5 * ease(noiseF2,3.5);
            vx*=intensity;
            vy*=intensity;
            ofPoint flow;
            flow.x = vx;
            flow.y = vy;
            flow.limit(vScale);
            //flow.normalize();
            flowField[index] = flow;
            ofPushMatrix();
            ofTranslate(x+x*20, y+y*20);
            ofSetColor(25);
            ofDrawLine(0,0,vx,vy);
            //ofDrawBitmapString(ofToString(noiseF), 0, 0);
            ofPopMatrix();
        }
    }
}

//--------------------------------------------------------------
// Draw Particles

void ofApp::drawParticles(){
    
    ofSetColor(127, 127);
    
    for(unsigned int i = 0; i < p.size(); i++){
        
        if(flowFieldFlag){
            p[i].follow(flowField, 40, cols);
        }
        p[i].update();
        p[i].draw();
        p[i].edges();
    }
}


//--------------------------------------------------------------
void ofApp::update(){
    
    contPoints.clear();
    
    // Contour FInder Setup Area !!

    movie.update();
    if(movie.isFrameNew()) {
        
        
        blur(movie, blurAmount);
        contourFinder.setMinAreaRadius(minArea);
        contourFinder.setMaxAreaRadius(maxArea);
        contourFinder.setThreshold(thresholdVal);
        contourFinder.findContours(movie);
        contourFinder.setSortBySize(true);
        contourFinder.setSimplify(true);
        contourFinder.setFindHoles(holes);
        
        if(modeSelector == "0"){
            ofxCv::copy(movie, imgBlur);
            blur(imgBlur, blurAmount);
            imgBlur.update();
            
            convertColor(imgBlur, imgThresh, CV_RGB2GRAY);
            threshold(imgThresh, thresholdVal);
            imgThresh.update();
            
            ofxCv::copy(imgThresh, imgInvert);
            invert(imgInvert);
            imgInvert.update();
        }
        
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
    
    // Countour Finder Area !!
    
    ofSetBackgroundAuto(showLabels);
    RectTracker& tracker = contourFinder.getTracker();
    
    
    if(modeSelector == "0"){
        ofBackground(0);
        movie.draw(0,0,640,400);
        imgBlur.draw(640,0,640,400);
        imgThresh.draw(0,400,640,400);
        imgInvert.draw(640,400,640,400); 
        
    }
    else if(modeSelector == "1") {
        ofBackground(0,225);
        createFlowField();
        drawParticles();
        ofSetColor(255);
        //movie.draw(0,0);
        ofDrawLine(10, 10, 100, 10);
        //movie.draw(0,0, 1280, 768);
        contourFinder.draw();
        for(int i = 0; i < contourFinder.size(); i++) {
            
            ofPoint center = toOf(contourFinder.getCenter(i));  // Get Centers of the Shape
            contPoints.push_back(center);

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
            
            
            // Proximity Check
            
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            
            for(int j = 0; j<contourFinder.size(); j++){
                ofPoint centerProximity = toOf(contourFinder.getCenter(j));
                
                float circleProximityRadius;
                ofVec2f circleProximityCenter = toOf(contourFinder.getMinEnclosingCircle(j, circleProximityRadius));
                float distance = center.distance(circleProximityCenter);
                
                if(distance < circleRadius + circleProximityRadius + proximity && i != j){
                    float midX = (center.x+circleProximityCenter.x)/2;
                    float midY = (center.y+circleProximityCenter.y)/2;
                    ofSetColor(255,200);
                    ofDrawCircle(midX, midY, distance);
                    //string location = ofToString(i) + ":" + ofToString(j);
//                    ofSetColor(255);
//                    myFont.drawString("Such Proximity", midX, midY);
                    ofSetColor(255);
                }
            }
            //Proximity Check End
            
            }
        
        
            
        }
    
    else if(modeSelector == "2"){
        ofSetColor(255);
        ofBackground(0);
        //myFont.drawString("Orientation", 100, 200);
        // movie.draw(0,0);
        for(int i = 0; i < contourFinder.size(); i++) {
            unsigned int label = contourFinder.getLabel(i);
            
            ofPoint center = toOf(contourFinder.getCenter(i));  // Get Centers of the Shape
            contPoints.push_back(center);
            
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
            
            // Proximity Check
            
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            
            for(int j = 0; j<contourFinder.size(); j++){
                ofPoint centerProximity = toOf(contourFinder.getCenter(j));
                
                float circleProximityRadius;
                ofVec2f circleProximityCenter = toOf(contourFinder.getMinEnclosingCircle(j, circleProximityRadius));
                float distance = center.distance(circleProximityCenter);
                
                if(distance < circleRadius + circleProximityRadius + proximity && i != j){
                    float midX = (center.x+circleProximityCenter.x)/2;
                    float midY = (center.y+circleProximityCenter.y)/2;
                    ofSetColor(magentaPrint);
                    ofNoFill();
                    ofDrawCircle(midX, midY, distance);
                    //string location = ofToString(i) + ":" + ofToString(j);
                    ofSetColor(255);
                    myFont.drawString("Proximity", midX, midY);
                    ofSetColor(255);
                }
            }
            //Proximity Check End
        }
        
        //Quads
        ofNoFill();
        for(int i = 0; i < quads.size(); i++) {
            //ofDrawBitmapString(ofToString(quads[i]), 20, 60+20*i);
            toOf(quads[i]).draw();
            float small = 0;
            ofPoint smallPoint;
            ofPolyline quadAngles(toOf(quads[i]));
            for(int j=0; j<4; j++){
                string msg = ofToString(i)+":"+ofToString(j)+":"+ofToString(quadAngles.getRadiansAtIndex(j));
                ofDrawBitmapString(msg, 20+100*j, 60+20*i);
                //ofDrawBitmapString(msg, quadAngles.getPointAtIndexInterpolated(j));
                
                if(quadAngles.getRadiansAtIndex(j)>small){
                    small = quadAngles.getRadiansAtIndex(j);
                    smallPoint = quadAngles.getPointAtIndexInterpolated(j);
                }
            }
            if (small > 1.8){
                ofDrawLine(quadAngles.getCentroid2D(), smallPoint);
                ofFill();
                ofDrawCircle(smallPoint,10);
                ofNoFill();
                ofPoint dirV = quadAngles.getCentroid2D() - smallPoint;
                dirV = dirV*-2;
                ofPushMatrix();
                ofTranslate(quadAngles.getCentroid2D());
                ofSetColor(255);
                myFont.drawString("Orientation", dirV.x, dirV.y);
                ofPopMatrix();
                
            }
            //        ofPopMatrix();
            
        }
        
        
        ofSetColor(255);
        //polyline drawing;
        ofPolyline shape(contPoints);
        ofSetLineWidth(2);
        shape.draw();
        
    }


    
    attractPointsWithMovement.clear();

    for(int i=0; i< contPoints.size(); i++){
        attractPointsWithMovement.push_back(contPoints[i]);
    }
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 20, 20);
    ofDrawBitmapString(ofToString(contPoints), 20, 40);

    
        // Gui Draw
    if(guiFlag){
        gui.draw();
        
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    
    if(key == ' ') {
        showLabels = !showLabels;
        resetParticles();
        modeSelector = "0";
    }
    
    if(key == '1') {
        modeSelector = "1";
    }
    
    if(key == '2') {
        modeSelector = "2";
    }
    
    if(key == '3') {
        modeSelector = "3";
    }
    
    if(key == 'p') {
        guiFlag = !guiFlag;
    }
    
    if(key == 'l') {
        flowFieldFlag = !flowFieldFlag;
    }
    
    
}




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
