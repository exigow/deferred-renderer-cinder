#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>

#include <cinder/Camera.h>
#include <cinder/MayaCamUI.h>
#include <cinder/gl/Texture.h>
#include <cinder/ImageIo.h>
#include <cinder/Utilities.h>
#include <cinder/TriMesh.h>
#include <cinder/ObjLoader.h>
#include <cinder/gl/Vbo.h>

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
	gl::Texture textureAlbedo, textureNormal, textureSpecular;
	Area viewport;

	DeferredRenderer *deferred;
	PointLight *movingLightRed, *movingLightGreen, *movingLightBlue;
	CubeMap *cubeMap;
	TriMesh mesh;
	gl::VboMesh vbo;
};

void DeferredExampleSimple::prepareSettings(Settings *settings) {
	settings->setWindowSize(1280, 640);
	settings->setFrameRate(60.0f);
	settings->setTitle("Deferred example");
	settings->enableConsoleWindow();
}

void DeferredExampleSimple::setup() {
	// Camera setup.
	camera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	camera.setEyePoint(Vec3f(0.0f, 16.0f, 32.0f));
	camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f));
	mayaCamera.setCurrentCam(camera);

	// Sample texture.
	textureAlbedo = gl::Texture(loadImage(loadAsset("bunny_albedo.png")));
	textureNormal = gl::Texture(loadImage(loadAsset("bunny_normal.png")));
	textureSpecular = gl::Texture(loadImage(loadAsset("bunny_specular.png")));

	ObjLoader loader(loadAsset("bunny.obj"));
	loader.load(&mesh);
	vbo = gl::VboMesh(mesh);

	// Deferred setup.
	deferred = new DeferredRenderer(1280, 640);
	deferred->setCamera(&camera);
	// Set shaders.
	deferred->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferred->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag"));
	deferred->composeShader = new gl::GlslProg(loadAsset("shaders/screenSpace.vert"), loadAsset("shaders/compose.frag"));

	// Set enviro and cube.
	deferred->setCubeMap(new CubeMap(loadAsset("test_cubemap.xml")));

	// Create lights.
	deferred->createLight(Vec3f(96.0f, 8.0f, 0.0f), 128.0f);
	deferred->createLight(Vec3f(-96.0f, 8.0f, 0.0f), 128.0f);
	deferred->createLight(Vec3f(0.0f, 8.0f, 96.0f), 128.0f);
	deferred->createLight(Vec3f(0.0f, 8.0f, -96.0f), 128.0f);

	movingLightRed = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 192.0f);
	movingLightRed->setColor(Vec3f(1.0f, .5f, .5f));
	movingLightGreen = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 192.0f);
	movingLightGreen->setColor(Vec3f(.5f, 1.0f, .5f));
	movingLightBlue = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 192.0f);
	movingLightBlue->setColor(Vec3f(.5f, .5f, 1.0f));
}

void DeferredExampleSimple::update() {
	movingLightRed->setPosition(
		(float)cos(getElapsedSeconds()) * 32.0f, 
		16.0f, 
		(float)sin(getElapsedSeconds()) * 32.0f);

	movingLightGreen->setPosition(
		(float)cos(getElapsedSeconds() + 2) * 32.0f, 
		16.0f, 
		(float)sin(getElapsedSeconds() + 2) * 32.0f);

	movingLightBlue->setPosition(
		(float)cos(getElapsedSeconds() + 4) * 32.0f, 
		16.0f, 
		(float)sin(getElapsedSeconds() + 4) * 32.0f);
}

void DeferredExampleSimple::draw() {
	// Fit persp. to window.
	camera.setAspectRatio(getWindowAspectRatio());

	// Clear background.
	gl::clear(Color(0.125f, 0.0f, 0.0f));

	// Deferred capturing.
	deferred->setTextureAlbedo(&textureAlbedo);
	deferred->setTextureNormal(&textureNormal);
	deferred->setTextureSpecular(&textureSpecular);
	deferred->captureBegin();
	for (size_t i = 0; i < deferred->getLights().size(); i++) {
		Vec3f _v = deferred->getLights()[i]->getPosition();
		gl::drawSphere(_v, 1.0f, 16);
	}
	gl::drawCube(ci::Vec3f(0.0f, 0.0f, 0.0f), ci::Vec3f(256.0f, 2.0f, 256.0f));
	//gl::drawSphere(Vec3f(0.0f, 0.0f, 0.0f), 16.0f, 128);
	//gl::drawTorus(8.0f, 4.0f, 64, 64);
	//gl::drawTorus();
	glPushMatrix();
		gl::scale(32, 32, 32);
		gl::translate(0, 1, 0);
		gl::draw(vbo);
	glPopMatrix();
	deferred->captureEnd();

	// Deferred lights render.
	deferred->renderLights();

	// Compose light with albedo/whatever. Depending on shader.
	deferred->compose();

	// Finally, draw.
	gl::draw(deferred->getBufferTexture(DeferredRenderer::COMPOSITION), Rectf(0.0f, 0.0f, (float)getWindowWidth(), (float)getWindowHeight()));

	// Draw debug text.
	gl::enableAlphaBlending();
	gl::drawString(
		"lights rendering time: " + toString(deferred->getRenderLightsTime()) + "\n" + 
		"capture time: " + toString(deferred->getCaptureTime()) + "\n" +
		"camera direciton: " + toString(camera.getViewDirection()) + "\n" +
		"cam era eye" + toString(camera.getEyePoint()) + "\n",
		Vec2f(64.0f, 64.0f), Color::white(), Font("Arial", 16.0f));
	gl::disableAlphaBlending();
}

void DeferredExampleSimple::mouseDown(MouseEvent event) {
	mayaCamera.mouseDown(event.getPos());
}

void DeferredExampleSimple::mouseDrag(MouseEvent event) {
	mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	camera = mayaCamera.getCamera();
}

CINDER_APP_BASIC(DeferredExampleSimple, RendererGl)
