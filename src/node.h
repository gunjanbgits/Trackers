//
//  node.h
//  mySketch
//
//  Created by Gunjan Bhutani on 2/20/19.
//

#ifndef node_h
#define node_h

#include "ofMain.h"

class Node {
    
public: // place public functions or variables declarations here
    
    // methods, equivalent to specific functions of your class objects
    void setup(glm::vec2 _pos, int _dim, float _fieldOfGlow);    // setup method, use this to setup your object's initial state
    void update();  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing
    
    // variables
    float x;        // position
    float y;
    float speedY;   // speed and direction
    float speedX;
    int dim;        // size
    float fieldOfGlow; //glow area
    ofColor color;  // color using ofColor type
    
    Node();  // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
private: // place private functions or variables declarations here
};

#endif /* node_h */
