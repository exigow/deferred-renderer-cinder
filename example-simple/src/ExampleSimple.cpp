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
	gl::Texture texture, enviroTex;
	Area viewport;

	DeferredRenderer *deferred;
	PointLight *movingLight;
	CubeMap *cubeMap;
	TriMesh mesh;
	gl::VboMesh vbo;
};

void DeferredExampleSimple::prepareSettings(Settings *settings) {
	settings->setWindowSize(1280, 640);
	settings->setFrameRate(60.0f);
	settings->setTitle("Deferred example");
}

void DeferredExampleSimple::setup() {
	// Camera setup.
	camera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	camera.setEyePoint(Vec3f(0.0f, 16.0f, 32.0f));
	camera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f));
	mayaCamera.setCurrentCam(camera);

	// Sample texture.
	texture = gl::Texture(loadImage(loadAsset("bunny_albedo.png")));
	enviroTex = gl::Texture(loadImage(loadAsset("enviro.png")));

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
	deferred->setTextureEnviro(&enviroTex);
	deferred->setCubeMap(new CubeMap(GLsizei(512), GLsizei(512),
		Surface8u(loadImage(loadAsset("cube/x_plus.png"))),
		Surface8u(loadImage(loadAsset("cube/y_plus.png"))),
		Surface8u(loadImage(loadAsset("cube/z_plus.png"))),
		Surface8u(loadImage(loadAsset("cube/x_minus.png"))),
		Surface8u(loadImage(loadAsset("cube/y_minus.png"))),
		Surface8u(loadImage(loadAsset("cube/z_minus.png")))));

	// Create lights.
	deferred->createLight(Vec3f(8.0f, 0.0f, 0.0f), 16.0f);
	deferred->createLight(Vec3f(-8.0f, 0.0f, 0.0f), 16.0f);
	deferred->createLight(Vec3f(0.0f, 0.0f, 8.0f), 16.0f);
	deferred->createLight(Vec3f(0.0f, 0.0f, -8.0f), 16.0f);
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
	/*for (size_t i = 0; i < deferred->getLights().size(); i++) {
		Vec3f _v = deferred->getLights()[i]->getPosition();
		gl::drawSphere(_v, .25f, 4);
	}*/
	//gl::drawCube(ci::Vec3f(0.0f, -2.0f, 0.0f), ci::Vec3f(64.0f, 1.0f, 64.0f));
	//gl::drawSphere(Vec3f(0.0f, 0.0f, 0.0f), 16.0f, 128);
	//gl::drawTorus(8.0f, 4.0f, 64, 64);
	//gl::drawTorus();
	glPushMatrix();
		gl::scale(32, 32, 32);
		gl::draw(vbo);
	glPopMatrix();
	deferred->captureEnd();

	// Deferred lights render.
	deferred->renderLights();

	// Compose light with albedo/whatever. Depending on shader.
	deferred->compose();

	// Finally, draw.
	gl::draw(deferred->getBufferTexture(DeferredRenderer::ENVIRO), Rectf(0.0f, 0.0f, (float)getWindowWidth(), (float)getWindowHeight()));

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
