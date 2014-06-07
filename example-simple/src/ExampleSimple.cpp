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
	Material *material;

	DeferredRenderer *deferred;
	PointLight *movingLightRed, *movingLightGreen, *movingLightBlue;
	CubeMap *cubeMap;
	TriMesh mesh;
	gl::VboMesh vbo;
};

void DeferredExampleSimple::prepareSettings(Settings *settings) {
	settings->setWindowSize(800, 600);
	settings->setFrameRate(60.0f);
	settings->setTitle("Deferred example");
}

void DeferredExampleSimple::setup() {
	// Camera setup.
	camera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	camera.setEyePoint(Vec3f(24.0f, 32.0f, -32.0f));
	camera.setCenterOfInterestPoint(Vec3f(0.0f, 16.0f, 0.0f));
	mayaCamera.setCurrentCam(camera);

	// Load material.
	material = new Material(loadAsset("material.xml"));

	ObjLoader loader(loadAsset("testModel.obj"));
	loader.load(&mesh);
	vbo = gl::VboMesh(mesh);

	// Deferred setup.
	deferred = new DeferredRenderer(800, 600);
	deferred->setCamera(&camera);
	// Set shaders.
	deferred->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferred->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag"));
	deferred->composeShader = new gl::GlslProg(loadAsset("shaders/screenSpace.vert"), loadAsset("shaders/compose.frag"));

	// Set enviro and cube.
	deferred->setCubeMap(new CubeMap(loadAsset("cubemap.xml")));

	movingLightRed = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 64.0f);
	movingLightRed->setColor(Vec3f(1.0f, .5f, .5f));
	movingLightGreen = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 64.0f);
	movingLightGreen->setColor(Vec3f(.5f, 1.0f, .5f));
	movingLightBlue = deferred->createLight(Vec3f(0.0f, 0.0f, 0.0f), 64.0f);
	movingLightBlue->setColor(Vec3f(.5f, .5f, 1.0f));
}

void DeferredExampleSimple::update() {
	movingLightRed->setPosition(
		(float)cos(getElapsedSeconds()) * 16.0f, 
		8.0f, 
		(float)sin(getElapsedSeconds()) * 16.0f);

	movingLightGreen->setPosition(
		(float)cos(getElapsedSeconds() + 2) * 16.0f, 
		8.0f, 
		(float)sin(getElapsedSeconds() + 2) * 16.0f);

	movingLightBlue->setPosition(
		(float)cos(getElapsedSeconds() + 4) * 16.0f, 
		8.0f, 
		(float)sin(getElapsedSeconds() + 4) * 16.0f);
}

void DeferredExampleSimple::draw() {
	// Fit persp. to window.
	camera.setAspectRatio(getWindowAspectRatio());

	// Clear background.
	gl::clear(Color(0.125f, 0.0f, 0.0f));

	// Deferred capturing.
	deferred->setMaterial(material);
	deferred->captureBegin();

	for (size_t i = 0; i < deferred->getLights().size(); i++) {
		Vec3f _v = deferred->getLights()[i]->getPosition();
		gl::drawSphere(_v, 0.5f, 8);
	}
	gl::draw(vbo);

	deferred->captureEnd();

	// Deferred lights render.
	deferred->renderLights();

	// Compose light with albedo/whatever. Depending on shader.
	deferred->compose();

	// Finally, draw.
	gl::draw(deferred->getBufferTexture(DeferredRenderer::MIXED), Rectf(0.0f, 0.0f, (float)getWindowWidth(), (float)getWindowHeight()));

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
