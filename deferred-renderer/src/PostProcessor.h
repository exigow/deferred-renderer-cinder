#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor();

	gl::Texture *lightTexture;


	gl::GlslProg fxaaShader;
};

