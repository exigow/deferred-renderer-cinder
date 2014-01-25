#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>

#include <cinder/Camera.h>
#include <cinder/MayaCamUI.h>
#include <cinder/gl/Texture.h>
#include <cinder/ImageIo.h>

#include <math.h>

#include <DeferredRenderer.h>

using namespace ci;
using namespace ci::app;
using namespace std;

class DeferredExampleSimple : public AppBasic {
public:
	void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();
	void mouseDown(MouseEvent event);	
	void mouseDrag(MouseEvent event);	
private:
	CameraPersp camera;
	MayaCamUI mayaCamera;
	gl::Texture texture;
	Area viewport;

	DeferredRenderer *deferred;
	PointLight *movingLight;
};

void DeferredExampleSimple::prepareSettings(Settings *settings) {
	settings->setWindowSize(1024, 640);
	settings->setFrameRate(60.0f);
	settings->setTitle("Deferred example");
}

void DeferredExampleSimple::setup() {
	camera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	camera.setEyePoint(Vec3f(0.0f, 16.0f, 32.0f));
	camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f));
	mayaCamera.setCurrentCam(camera);
	texture = gl::Texture(loadImage(loadAsset("cactuscat.png")));

	// Deferred setup.
	deferred = new DeferredRenderer(1024, 640);
	deferred->setCamera(&camera);
	deferred->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferred->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag"));

	// Test lights.
	deferred->createLight(Vec3f(8.0f, 0.0f, 0.0f), 24.0f);
	deferred->createLight(Vec3f(-8.0f, 0.0f, 0.0f), 24.0f);
	deferred->createLight(Vec3f(0.0f, 0.0f, 8.0f), 24.0f);
	deferred->createLight(Vec3f(0.0f, 0.0f, -8.0f), 24.0f);
	movingLight = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 32.0f);
}

void DeferredExampleSimple::update() {
	movingLight->setPosition(
		(float)cos(getElapsedSeconds()) * 32.0f, 
		0.0f, 
		(float)sin(getElapsedSeconds()) * 32.0f);
}

void DeferredExampleSimple::draw() {
	// Fit persp. to window.
	camera.setAspectRatio(getWindowAspectRatio());

	// Clear background.
	gl::clear(Color(0.125f, 0.0f, 0.0f));

	// Deferred capturing.
	deferred->setTextureAlbedo(&texture);
	deferred->captureBegin();
		for (int ix = -4; ix <= 4; ix++) {
			for (int iz = -4; iz <= 4; iz++) {
				gl::drawCube(ci::Vec3f(6.0f * ix, 0.0f + (float)sin(getElapsedSeconds() + (float)ix + (float)iz) * 8.0f, 6.0f * iz), ci::Vec3f(4.0f, 4.0f, 4.0f));
			}
		}
	deferred->captureEnd();

	// Deferred lights render.
	deferred->renderLights();

	gl::draw(deferred->getBufferTexture(DeferredRenderer::BufferTexture::BUFTEX_LIGHT), Rectf(0.0f, 0.0f, (float)getWindowWidth(), (float)getWindowHeight()));
}

void DeferredExampleSimple::mouseDown(MouseEvent event) {
	mayaCamera.mouseDown(event.getPos());
}

void DeferredExampleSimple::mouseDrag(MouseEvent event) {
	mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	camera = mayaCamera.getCamera();
}

CINDER_APP_BASIC(DeferredExampleSimple, RendererGl)
