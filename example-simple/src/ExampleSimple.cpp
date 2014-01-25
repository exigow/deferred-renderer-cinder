#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>

using namespace ci;
using namespace ci::app;
using namespace std;

class DeferredExampleSimple : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void DeferredExampleSimple::setup() {
}

void DeferredExampleSimple::mouseDown(MouseEvent event) {
}

void DeferredExampleSimple::update() {
}

void DeferredExampleSimple::draw() {
	gl::clear(Color(0, 0, 0)); 
}

CINDER_APP_BASIC(DeferredExampleSimple, RendererGl)
