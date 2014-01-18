#pragma once

#include "cinder/Vector.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer(void);

	enum {
		GBUFFER = 0,
		LIGHTS = 1
	};

	void setPointLightShader(gl::GlslProg *shader);

private:

	gl::GlslProg *pointLightShader;
};

	