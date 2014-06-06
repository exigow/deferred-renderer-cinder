#pragma once

#include <cinder/Vector.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/gl/Fbo.h>
#include <cinder/Camera.h>
#include <cinder/Timer.h>

#include "PointLight.h"
#include "CubeMap.h"

#include <vector>

using namespace ci;

const int CUBE_MAP_LOC = 4;

class DeferredRenderer
{
public:
	DeferredRenderer();
	DeferredRenderer(int w, int h);

	~DeferredRenderer();

	gl::GlslProg *pointLightShader, *deferredShader, *composeShader;

	void setCamera(CameraPersp *camera);

	void renderLights();
	void compose();

	int getWidth();
	int getHeight();
	int getNumberOfLights();

	void setTextureAlbedo(gl::Texture *texture);
	void setTextureNormal(gl::Texture *texture);
	void setTextureSpecular(gl::Texture *texture);

	void setCubeMap(CubeMap *map);

	void captureBegin();
	void captureEnd();

	double getRenderLightsTime();
	double getCaptureTime();

	std::vector<PointLight*> getLights();

	PointLight *createLight(Vec3f position, float radius);

	enum BufferTexture {
		ALBEDO_AND_DEPTH,
		NORMAL,
		POSITION,
		LIGHT,
		ENVIRO,
		COMPOSITION
	};

	gl::Texture getBufferTexture(BufferTexture texture);

private:
	gl::Fbo::Format deferredFBOFormat, lightFBOFormat, compositionFBOFormat;
	gl::Fbo deferredFBO, lightFBO, compositionFBO;

	void setup(int width, int height);
	CameraPersp *camera;
	int width, height;
	gl::Texture *textureAlbedo, *textureNormal, *textureSpecular;

	CubeMap *cubeMap;

	Area viewportPrev;
	
	std::vector<PointLight*> lightList;
	Timer tempTime;
	double lightsRenderTime, captureTime;
};