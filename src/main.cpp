#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(){
    
//    ofGLFWWindowSettings settings;
//    settings.multiMonitorFullScreen = true;
//    settings.windowMode = OF_FULLSCREEN;
//    settings.monitor = 1;
//    ofCreateWindow(settings);
    
	ofSetupOpenGL(1280,800,OF_FULLSCREEN);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
