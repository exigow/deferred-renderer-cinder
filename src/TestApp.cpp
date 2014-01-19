#pragma once

// Cinder.
#include "cinder/Filesystem.h"
#include "cinder/ImageIo.h"
#include "cinder/Matrix.h"
#include "cinder/Surface.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/TriMesh.h"
#include "cinder/ObjLoader.h"
#include "cinder/Font.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

// Physics.
#include "CinderBullet.h"

// Other.
#include <math.h>
#include <string>

// My.
#include "PointLight.h"
#include "DeferredRenderer.h"
#include "BouncingBox.h"
#include "Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph;

#define WINDOW_W 1366
#define WINDOW_H 768

class TestApp : public AppBasic {
public:
	void setup();
	void update();
	void draw();
	void prepareSettings(Settings *settings);
	void renderScene();

	void mouseDown(MouseEvent event);	
	void mouseDrag(MouseEvent event);	
	void keyDown(KeyEvent event);

	void resetCamera();
	
protected:
	gl::Texture mTexture, mBackgroundTexture;

	CameraPersp mCamera;
	MayaCamUI mMayaCamera;

	TriMesh mModel;

	Area viewport;

	DeferredRenderer *deferredRenderer;

	vector<BouncingBox*> bouncingBoxList;
	int rigidCount;
};

void TestApp::prepareSettings(Settings *settings) {
	settings->setWindowSize(WINDOW_W, WINDOW_H);
	settings->setFrameRate(60.0f);
	settings->setTitle("TestApp");
	settings->setResizable(false);
	settings->setBorderless(false);
	settings->setFullScreen(true);
}

void TestApp::resetCamera() {
	mCamera.setEyePoint(Vec3f(0.0f, 128.0f, 256.0f));
	mCamera.setCenterOfInterestPoint(Vec3f(0.0f, 0.0f, 0.0f)) ;
	mCamera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	mMayaCamera.setCurrentCam(mCamera);
}

void TestApp::setup() {
	// Physics gravity set.
	bullet::getWorld()->setGravity(btVector3(0.0f, -9.81f, 0.0f));

	// Physics floor shape.
	bullet::shape::Box* boxFloorShape(new bullet::shape::Box(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1024.0f, 1.0f, 1024.0f), false));
	boxFloorShape->getRigidBody()->setRestitution(btScalar(0.75f));
	bullet::getWorld()->addRigidBody(boxFloorShape->getRigidBody().get());

	// Deferred renderer create.
	deferredRenderer = new DeferredRenderer(640, 480);
	deferredRenderer->setCamera(&mCamera);
	deferredRenderer->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferredRenderer->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag")); 

	// Camera reset.
	resetCamera();

	// Loading model.
	ObjLoader loader(loadAsset("bunny.obj"));
	loader.load(&mModel);

	// Vsync off.
	gl::disableVerticalSync();

	// Loading textures.
	mTexture = gl::Texture(loadImage(loadAsset("test.png")));
	mBackgroundTexture = gl::Texture(loadImage(loadAsset("background.png")));
}

void TestApp::update() {
	rigidCount = ph::bullet::getWorld()->getNumCollisionObjects();

	ph::bullet::getWorld()->stepSimulation(btScalar( getElapsedSeconds() ), 8);
	btRigidBody *body;
	for (size_t i = 0; i < bouncingBoxList.size(); i++) {
		body = bouncingBoxList[i]->shape->getRigidBody().get();
		float matrix[16];
		((btDefaultMotionState*)body->getMotionState())->m_graphicsWorldTrans.getOpenGLMatrix(&matrix[0]);
		bouncingBoxList[i]->light->setPosition(Utilities::getPositionFromBulletMatrix(matrix));
	}
}

void TestApp::draw() {
	viewport = gl::getViewport();
	
	// Rendering sceny do FBO.
	gl::setViewport(deferredRenderer->deferredFBO.getBounds());
	deferredRenderer->deferredFBO.bindFramebuffer();
		gl::clear(Color::black());
		gl::pushMatrices();
			gl::setMatricesWindow(WINDOW_W, WINDOW_H, false);
			renderScene();
		gl::popMatrices();
	deferredRenderer->deferredFBO.unbindFramebuffer();

	deferredRenderer->renderLights();

	gl::setViewport(viewport);
	gl::pushMatrices();
		gl::draw(deferredRenderer->lightFBO.getTexture(0), Rectf(0, 0, (float)WINDOW_W, (float)WINDOW_H));
	gl::popMatrices();

	gl::enableAlphaBlending();
	gl::drawString("fps: " + toString((int)getAverageFps()) + ", rigidCount: " + toString(rigidCount), Vec2f(8.0f, 8.0f), Color::white(), Font("Arial", 16.0f));
	gl::disableAlphaBlending();
}

void TestApp::renderScene() {
	mTexture.bind(0);
	deferredRenderer->deferredShader->bind();
	deferredRenderer->deferredShader->uniform("mTexture", 1);

	gl::pushMatrices();
		gl::setMatrices(mCamera);
		gl::enableDepthRead();
		gl::enableDepthWrite();
			/*gl::pushMatrices();
			float scale = 256.0f;
			gl::scale(Vec3f(scale, scale, scale));
			gl::draw(mModel);
			gl::popMatrices();*/
			bullet::drawWorld();
		gl::disableDepthWrite();
		gl::disableDepthRead();
	gl::popMatrices();
	deferredRenderer->deferredShader->unbind();
}

void TestApp::mouseDown(MouseEvent event) {
	mMayaCamera.mouseDown(event.getPos());
}

void TestApp::mouseDrag(MouseEvent event) {
	mMayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	mCamera = mMayaCamera.getCamera();
}

void TestApp::keyDown(KeyEvent event) {
	switch(event.getCode()) {
		case KeyEvent::KEY_ESCAPE: {
			quit();
			break;
		}
		case KeyEvent::KEY_SPACE: {
			BouncingBox *ref;
			ref = new BouncingBox();
			bullet::shape::Box *boxShape(new bullet::shape::Box(Vec3f(0.0f, 128.0f, 0.0f), Vec3f(4.0f, 4.0f, 4.0f), true));
			float scale = 32.0f;
			boxShape->getRigidBody()->applyCentralImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));
			boxShape->getRigidBody()->applyTorqueImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));
			boxShape->getRigidBody()->setRestitution(btScalar(0.75f));
			ref->shape = boxShape;
			bullet::getWorld()->addRigidBody(boxShape->getRigidBody().get());
			ref->light = new PointLight();
			deferredRenderer->lightList.push_back(ref->light);
			ref->light->setRadius(64.0f);
			bouncingBoxList.push_back(ref);
			break;
		}
	}
}

CINDER_APP_BASIC(TestApp, RendererGl)
