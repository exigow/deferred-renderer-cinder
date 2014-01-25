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
	void generateBlurFBO();
	
protected:
	gl::Texture mTexture, mBackgroundTexture;

	CameraPersp mCamera;
	MayaCamUI mMayaCamera;

	TriMesh mModel;

	Area viewport;

	DeferredRenderer *deferredRenderer;
	gl::GlslProg *fxaaShader;

	gl::Fbo *blurFBO[4];

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
	boxFloorShape->getRigidBody()->setRestitution(btScalar(0.825f));
	bullet::getWorld()->addRigidBody(boxFloorShape->getRigidBody().get());

	// Deferred renderer create.
	deferredRenderer = new DeferredRenderer(1366, 768);
	deferredRenderer->setCamera(&mCamera);
	deferredRenderer->deferredShader = new gl::GlslProg(loadAsset("shaders/deferred.vert"), loadAsset("shaders/deferred.frag")); 
	deferredRenderer->pointLightShader = new gl::GlslProg(loadAsset("shaders/pointLight.vert"), loadAsset("shaders/pointLight.frag")); 

	// PP.
	fxaaShader = new gl::GlslProg(loadAsset("shaders/fxaa.vert"), loadAsset("shaders/fxaa.frag")); 

	gl::Fbo::Format blurFBOFormat;
	blurFBOFormat.setColorInternalFormat(GL_RGB8);
	blurFBOFormat.setSamples(0);
	blurFBOFormat.setCoverageSamples(0);
	blurFBO[0] = new gl::Fbo(256, 256, blurFBOFormat);
		blurFBO[0]->getTexture().setFlipped(true);
	/*blurFBO[1] = new gl::Fbo(256, 256, blurFBOFormat);
		blurFBO[1]->getTexture().setFlipped(true);
	blurFBO[2] = new gl::Fbo(128, 128, blurFBOFormat);
		blurFBO[2]->getTexture().setFlipped(true);
	blurFBO[3] = new gl::Fbo(64, 64, blurFBOFormat);
		blurFBO[3]->getTexture().setFlipped(true);*/

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

	ph::bullet::getWorld()->stepSimulation(btScalar(getElapsedSeconds()), 8);
	btRigidBody *body;
	for (size_t i = 0; i < bouncingBoxList.size(); i++) {
		body = bouncingBoxList[i]->shape->getRigidBody().get();
		float matrix[16];
		((btDefaultMotionState*)body->getMotionState())->m_graphicsWorldTrans.getOpenGLMatrix(&matrix[0]);
		bouncingBoxList[i]->light->setPosition(Utilities::getPositionFromBulletMatrix(matrix));
	}
}

void TestApp::generateBlurFBO() {
	//gl::setViewport(blurFBO[0]->getBounds());
	//gl::setViewport(deferredRenderer->getBufferTexture()
	//blurFBO[0]->bindFramebuffer();
	//gl::clear(Color(0.125f, 0.0f, 0.0f));
	//blurFBO[0]->unbindFramebuffer();
}
void TestApp::draw() {
	gl::clear(Color::black());
	gl::setViewport(deferredRenderer->deferredFBO.getBounds());

	// Rendering sceny do FBO
	deferredRenderer->deferredFBO.bindFramebuffer();
	gl::clear(Color::black());
	gl::pushMatrices();
		renderScene();
	gl::popMatrices();
	deferredRenderer->deferredFBO.unbindFramebuffer();

	// Render lights.
	deferredRenderer->renderLights();

	generateBlurFBO();

	fxaaShader->bind();
	fxaaShader->uniform("source", 0);
	fxaaShader->uniform("frameBufSize", Vec2f((float)deferredRenderer->getWidth(), (float)deferredRenderer->getHeight()));
	gl::draw(deferredRenderer->lightFBO.getTexture(0), Rectf(0, 0, (float)WINDOW_W, (float)WINDOW_H));
	fxaaShader->unbind();

	//gl::draw(blurFBO[0]->getTexture(0), Rectf(128.0f, 128.0f, 256.0f, 256.0f));

	Utilities::drawStringSimple("fps: " + toString((int)getAverageFps()) + ", rigidCount: " + toString(rigidCount), 8.0f, 8.0f);
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
			float scale = 512.0f;
			gl::scale(Vec3f(scale, scale, scale));
			gl::draw(mModel);
			gl::popMatrices();
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
			float scale = 16.0f;
			boxShape->getRigidBody()->applyCentralImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));
			boxShape->getRigidBody()->applyTorqueImpulse(btVector3(randFloat(-scale, scale), randFloat(-scale, scale), randFloat(-scale, scale)));
			boxShape->getRigidBody()->setRestitution(btScalar(0.825f));
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
