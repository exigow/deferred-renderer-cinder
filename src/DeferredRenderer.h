#pragma once

#include "cinder/Vector.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"

#include "PointLight.h"

#include <vector>

using namespace ci;

class DeferredRenderer
{
public:
	DeferredRenderer();
	DeferredRenderer(int w, int h);
	~DeferredRenderer();
	
	gl::GlslProg *pointLightShader, *deferredShader;
	gl::Fbo deferredFBO, lightFBO;
	gl::Fbo::Format deferredFBOFormat, lightFBOFormat;

	std::vector<PointLight*> lightList;

	void setCamera(CameraPersp *camera);
	void renderLights();
	int getWidth();
	int getHeight();

private:
	void setup(int width, int height);
	CameraPersp *camera;
	void drawQuad();
	int width, height;
};

	


	// Preview
	/*switch (preview) {
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
	}*/