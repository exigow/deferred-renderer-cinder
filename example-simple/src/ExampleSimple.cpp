#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>

#include <cinder/Camera.h>
#include <cinder/MayaCamUI.h>
#include <cinder/gl/Texture.h>
#include <cinder/ImageIo.h>

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
};

void DeferredExampleSimple::prepareSettings(Settings *settings) {
	settings->setWindowSize(1024, 480);
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
	deferred = new DeferredRenderer(256, 256);
	deferred->setCamera(&camera);
	deferred->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferred->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag"));
}

void DeferredExampleSimple::update() {
}

void DeferredExampleSimple::draw() {
	// Fit persp. to window.
	camera.setAspectRatio(getWindowAspectRatio());

	// Clear background.
	gl::clear(Color(0.125f, 0.0f, 0.0f));

	// Deferred capturing.
	deferred->setTextureAlbedo(&texture);
	deferred->captureBegin();
		gl::drawCube(ci::Vec3f::zero(), ci::Vec3f(4.0f, 4.0f, 4.0f));
	deferred->captureEnd();


	gl::draw(deferred->getBufferTexture(DeferredRenderer::BufferTexture::BUFTEX_ALBEDO_AND_DEPTH), Rectf(16.0f, 16.0f, (float)getWindowWidth() - 16.0f, (float)getWindowHeight() - 16.0f));

	gl::drawString("asdasdasd", Vec2f(0.0f, 0.0f), Color::white(), Font("Arial", 16.0f));
}

void DeferredExampleSimple::mouseDown(MouseEvent event) {
	mayaCamera.mouseDown(event.getPos());
}

void DeferredExampleSimple::mouseDrag(MouseEvent event) {
	mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	camera = mayaCamera.getCamera();
}

CINDER_APP_BASIC(DeferredExampleSimple, RendererGl)
