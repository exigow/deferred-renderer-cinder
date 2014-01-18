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

#include <math.h>
#include <string>

#include "PointLight.h"
#include "DeferredRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace size {
	unsigned const int 
		windowWidth = 1280, windowHeight = 600,
		bufferWidth = 1280, bufferHeight = 600;
	const float windowRatio = (float)windowWidth / (float)windowHeight;
};

namespace fboPreview {
	enum {
		GBUFFER = 0,
		LIGHTS = 1
	};
}

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

	Vec3f mRight, mUp;
	TriMesh mModel;

	vector<PointLight> list;

	int preview;

	DeferredRenderer *deferredRenderer;
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
	deferredRenderer = new DeferredRenderer(1280, 600);
	deferredRenderer->deferredShader = new gl::GlslProg(loadAsset("deferred.vert"), loadAsset("deferred.frag")); 
	deferredRenderer->deferredPreviewShader = new gl::GlslProg(loadAsset("deferredPreview.vert"), loadAsset("deferredPreview.frag")); 
	deferredRenderer->pointLightShader = new gl::GlslProg(loadAsset("light.vert"), loadAsset("light.frag")); 

	preview = fboPreview::GBUFFER;

	for (int i = 0; i < 20; i++) {
		list.push_back(PointLight());
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
	for (size_t i = 0; i < list.size(); i++) {
		float t = (float)getElapsedSeconds() + (float)i * 2.0f;
		list[i].setPosition(
			(float)cos(t) * 8.0f,
			8.0f + (float)sin(t * 2.0f) * 4.0f,
			(float)sin(t) * 8.0f);
	}
}

// toString.
template <typename T> string toStr(const T& t) { 
	return boost::lexical_cast<std::string>(t);
}

void TestApp::draw() {
	Area viewport = gl::getViewport();
	mMayaCamera.getCamera().getBillboardVectors(&mRight, &mUp);
	
	// Rendering sceny do FBO.
	gl::setViewport(deferredRenderer->deferredFBO.getBounds());
	deferredRenderer->deferredFBO.bindFramebuffer();
		gl::clear(Color::black());
		gl::pushMatrices();
			gl::setMatricesWindow(size::windowWidth, size::windowWidth, false);
			renderScene();
		gl::popMatrices();
	deferredRenderer->deferredFBO.unbindFramebuffer();

	// Render LIGHT
	deferredRenderer->deferredFBO.getTexture(0).bind(0);
	deferredRenderer->deferredFBO.getTexture(1).bind(1);
	deferredRenderer->deferredFBO.getTexture(2).bind(2);
	gl::setViewport(deferredRenderer->lightFBO.getBounds());
	deferredRenderer->pointLightShader->bind();
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		deferredRenderer->pointLightShader->uniform("frag0", 0);
		deferredRenderer->pointLightShader->uniform("frag1", 1);
		deferredRenderer->pointLightShader->uniform("frag2", 2);
		deferredRenderer->lightFBO.bindFramebuffer();
			gl::clear(Color::black());
			gl::pushMatrices();
				gl::setMatrices(mCamera);
				for (size_t i = 0; i < list.size(); i++) {
					deferredRenderer->pointLightShader->uniform("lightPosition", mCamera.getModelViewMatrix().transformPointAffine(list[i].getPosition()));
					deferredRenderer->pointLightShader->uniform("lightRadius", list[i].getRadius() * .5f);
					deferredRenderer->pointLightShader->uniform("lightColor", list[i].getColor());
					gl::drawCube(list[i].getPosition(), Vec3f(list[i].getRadius(), list[i].getRadius(), list[i].getRadius()));
				}
			gl::popMatrices();

			glDisable(GL_BLEND);
		deferredRenderer->lightFBO.unbindFramebuffer();
	deferredRenderer->pointLightShader->unbind();

	// Preview
	switch (preview) {
		case fboPreview::LIGHTS: {
			gl::setViewport(viewport);
			gl::pushModelView();
			gl::draw(deferredRenderer->lightFBO.getTexture(0), Rectf(0, 0, (float)size::windowWidth, (float)size::windowHeight));
			gl::popModelView();
			break;
		}
		case fboPreview::GBUFFER: {
			deferredRenderer->deferredFBO.getTexture(0).bind(0);
			deferredRenderer->deferredFBO.getTexture(1).bind(1);
			deferredRenderer->deferredFBO.getTexture(2).bind(2);
			gl::setViewport(viewport);
			deferredRenderer->deferredPreviewShader->bind();
				deferredRenderer->deferredPreviewShader->uniform("frag0", 0);
				deferredRenderer->deferredPreviewShader->uniform("frag1", 1);
				deferredRenderer->deferredPreviewShader->uniform("frag2", 2);
				gl::pushMatrices();
					gl::draw(deferredRenderer->deferredFBO.getTexture(0), Rectf(0, 0, (float)size::windowWidth, (float)size::windowHeight));
				gl::popMatrices();
			deferredRenderer->deferredPreviewShader->unbind();
			break;
		}
	}

	

	gl::enableAlphaBlending();
	gl::drawString("fps: " + toStr(getAverageFps()), Vec2f(8.0f, 8.0f), Color::white(), Font("Arial", 16.0f));
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
			float scale = 96.0f;
			gl::scale(Vec3f(scale, scale, scale));
			gl::draw(mModel);
			gl::popMatrices();
			for (size_t i = 0; i < list.size(); i++) {
				gl::drawSphere(list[i].getPosition(), 1.0f, 6);
			}
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
		case KeyEvent::KEY_1: {
			preview = fboPreview::GBUFFER;
			break;
		}
		case KeyEvent::KEY_2: {
			preview = fboPreview::LIGHTS;
			break;
		}
	}
}

CINDER_APP_BASIC(TestApp, RendererGl)
