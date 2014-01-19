#pragma once

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

#include <math.h>
#include <string>

#include "PointLight.h"
#include "DeferredRenderer.h"
#include "TestBox.h"

#include "CinderBullet.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph;

namespace size {
	unsigned const int 
		windowWidth = 1280, windowHeight = 600,
		bufferWidth = 1280, bufferHeight = 600;
	const float windowRatio = (float)windowWidth / (float)windowHeight;
};

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

	vector<TestBox*> testBoxList;

	int rigidCount;
};

void TestApp::prepareSettings(Settings *settings) {
	settings->setWindowSize(size::windowWidth, size::windowHeight);
	settings->setFrameRate(60.0f);
	settings->setTitle("TestApp");
	settings->setResizable(false);
	settings->setBorderless(false);
	settings->setFullScreen(false);
}

void TestApp::resetCamera() {
	mCamera.setEyePoint(Vec3f(0.0f, 0.0f, 32.0f));
	mCamera.setCenterOfInterestPoint(Vec3f( 0.0f, 0.0f, 0.0f )) ;
	mCamera.setPerspective(60.0f, getWindowAspectRatio(), 1.0f, 2048.0f);
	mMayaCamera.setCurrentCam(mCamera);
}

void TestApp::setup() {
	bullet::getWorld()->setGravity(btVector3(0, -9.81f, 0));

	bullet::shape::Box* boxFloorShape(new bullet::shape::Box(Vec3f(0.0f, 8.0f, 0.0f), Vec3f(512.0f, 1.0f, 512.0), false));
	bullet::getWorld()->addRigidBody(boxFloorShape->getRigidBody().get());

	deferredRenderer = new DeferredRenderer(1280, 600);
	deferredRenderer->setCamera(&mCamera);
	deferredRenderer->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferredRenderer->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag")); 

	PointLight *ref;
	for (int i = 0; i < 0; i++) {
		ref = new PointLight();
		ref->setPosition(randFloat(-256.0f, 256.0f), 4.0f, randFloat(-256.0f, 256.0f));
		ref->setRadius(32.0f);
		deferredRenderer->lightList.push_back(ref);
	}

	// Reset kamery.
	resetCamera();

	ObjLoader loader(loadAsset("bunny.obj"));
	loader.load(&mModel);

	// Wylaczenie v-sync.
	gl::disableVerticalSync();

	// Wczytywanie tekstury.
	mTexture = gl::Texture(loadImage(loadAsset("test.png")));
	mBackgroundTexture = gl::Texture(loadImage(loadAsset("background.png")));
}

void TestApp::update() {
	rigidCount = bullet::getWorld()->getNumCollisionObjects();
	bullet::getWorld()->stepSimulation(btScalar( getElapsedSeconds() ), 8);
	btRigidBody *body;
	for (int i = 0; i < testBoxList.size(); i++) {
		body = testBoxList[i]->shape->getRigidBody().get();

		float matrix[16];
		((btDefaultMotionState*)body->getMotionState())->m_graphicsWorldTrans.getOpenGLMatrix(&matrix[0]);

		float x = matrix[3 * 4 + 0];
		float y = matrix[3 * 4 + 1];
		float z = matrix[3 * 4 + 2];
		
		testBoxList[i]->light->setPosition(x, y, z);
	}
}

// toString.
template <typename T> string toStr(const T& t) { 
	return boost::lexical_cast<std::string>(t);
}

void TestApp::draw() {
	viewport = gl::getViewport();
	
	// Rendering sceny do FBO.
	gl::setViewport(deferredRenderer->deferredFBO.getBounds());
	deferredRenderer->deferredFBO.bindFramebuffer();
		gl::clear(Color::black());
		gl::pushMatrices();
			gl::setMatricesWindow(size::windowWidth, size::windowWidth, false);
			renderScene();
		gl::popMatrices();
	deferredRenderer->deferredFBO.unbindFramebuffer();

	deferredRenderer->renderLights();

	gl::setViewport(viewport);
	gl::pushMatrices();
		gl::draw(deferredRenderer->lightFBO.getTexture(0), Rectf(0, 0, (float)size::windowWidth, (float)size::windowHeight));
	gl::popMatrices();

	gl::enableAlphaBlending();
	gl::drawString("fps: " + toStr(getAverageFps()) + ", rigidCount: " + toStr(rigidCount), Vec2f(8.0f, 8.0f), Color::white(), Font("Arial", 16.0f));
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
			gl::pushMatrices();
			float scale = 256.0f;
			gl::scale(Vec3f(scale, scale, scale));
			//gl::draw(mModel);
			gl::popMatrices();
			bullet::drawWorld();
			/*for (size_t i = 0; i < deferredRenderer->lightList.size(); i++) {
				gl::drawSphere(deferredRenderer->lightList[i]->getPosition(), 1.0f, 4);
			}*/
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
			TestBox *ref;
			ref = new TestBox();
			bullet::shape::Box *boxShape(new bullet::shape::Box(Vec3f(0.0f, 128.0f, 0.0f), Vec3f(4.0f, 4.0f, 4.0f), true));
			float scale = 16.0f;
			boxShape->getRigidBody()->applyCentralImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));
			boxShape->getRigidBody()->applyTorqueImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));

			ref->shape = boxShape;
			bullet::getWorld()->addRigidBody(boxShape->getRigidBody().get());
			ref->light = new PointLight();
			deferredRenderer->lightList.push_back(ref->light);
			ref->light->setRadius(24.0f);
			testBoxList.push_back(ref);
			break;
		}
	}
}

CINDER_APP_BASIC(TestApp, RendererGl)
