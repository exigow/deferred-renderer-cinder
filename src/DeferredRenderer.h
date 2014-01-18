#pragma once

#include "cinder/Vector.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"

using namespace ci;

class DeferredRenderer
{
public:
	DeferredRenderer();
	DeferredRenderer(int w, int h);
	~DeferredRenderer();

	gl::GlslProg *pointLightShader, *deferredShader, *deferredPreviewShader;
	
	gl::Fbo deferredFBO, lightFBO;
	gl::Fbo::Format deferredFBOFormat, lightFBOFormat;

private:
	void setup(int width, int height);
};

	