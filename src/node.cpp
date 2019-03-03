//
//  node.cpp
//  mySketch
//
//  Created by Gunjan Bhutani on 2/20/19.
//

#include "node.h"
#include "ofMain.h"

Node::Node(){
    
}

void Node::setup(glm::vec2 _pos, int _dim, float _fieldOfGlow){
    
    x = _pos.x;      // give some random positioning
    y = _pos.y;
    fieldOfGlow = _fieldOfGlow;
    
    dim = _dim;
    
    color.set(255);
    
}

void Node::update(){
    
}

void Node::draw(){
    ofSetColor(color);
    ofNoFill();
    ofDrawCircle(x, y, dim);
    for(int i = 0; i<10; i++){
        ofDrawCircle(x, y, dim+i*ofMap(sin(ofGetElapsedTimef()), -1, 1, 0, fieldOfGlow));
    }
    
}

