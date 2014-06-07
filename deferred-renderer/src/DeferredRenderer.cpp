#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(int width, int height) {
	this->setup(width, height);
}

DeferredRenderer::DeferredRenderer() {
	this->setup(640, 480); // Default size.
}

DeferredRenderer::~DeferredRenderer() {
}

// Get size.
int DeferredRenderer::getWidth() {
	return width;
}
int DeferredRenderer::getHeight() {
	return height;
}

// Set camera pointer.
void DeferredRenderer::setCamera(CameraPersp *camera) {
	this->camera = camera;
}

// Setup deferred.
void DeferredRenderer::setup(int width, int height) {
	// Set size.
	this->width = width;
	this->height = height;

	gl::Fbo::Format deferredFBOFormat, lightFBOFormat, compositionFBOFormat;

	// Setup FBOs.
	deferredFBOFormat.setColorInternalFormat(GL_RGBA16F_ARB);
	deferredFBOFormat.enableColorBuffer(true, 4);
	deferredFBOFormat.setSamples(0);
	deferredFBOFormat.setCoverageSamples(0);
	deferredFBO = gl::Fbo(width, height, deferredFBOFormat);
		deferredFBO.getTexture(0).setFlipped(true);
		deferredFBO.getTexture(1).setFlipped(true);
		deferredFBO.getTexture(2).setFlipped(true);
		deferredFBO.getTexture(3).setFlipped(true);

	lightFBOFormat.setColorInternalFormat(GL_RGB8);
	lightFBOFormat.setSamples(0);
	lightFBOFormat.setCoverageSamples(0);
	lightFBO = gl::Fbo(width, height, lightFBOFormat);
		lightFBO.getTexture().setFlipped(true);

	compositionFBOFormat.setColorInternalFormat(GL_RGB8);
	compositionFBOFormat.setSamples(0);
	compositionFBOFormat.setCoverageSamples(0);
	compositionFBO = gl::Fbo(width, height, compositionFBOFormat);
		compositionFBO.getTexture().setFlipped(false);
}

gl::Texture DeferredRenderer::getBufferTexture(DeferredRenderer::BufferTexture texture) {
	gl::Fbo *fbo;
	int fromBuffer;
	if (texture == ALBEDO_AND_DEPTH || texture == NORMAL || texture == POSITION || texture == ENVIRO) {
		fbo = &deferredFBO;
		switch (texture) {
			case ALBEDO_AND_DEPTH: {
				fromBuffer = 0;
				break;
			}
			case NORMAL: {
				fromBuffer = 1;
				break;
			}
			case POSITION: {
				fromBuffer = 2;
				break;
			}
			case ENVIRO: {
				fromBuffer = 3;
				break;
			}
		}
	} else {
		if (texture == LIGHT) {
			fbo = &lightFBO;
			fromBuffer = 0;
		} else {
			if (texture == MIXED) {
				fbo = &compositionFBO;
				fromBuffer = 0;
			} 
		}
	}

	return fbo->getTexture(fromBuffer);
}


void DeferredRenderer::setTextureAlbedo(gl::Texture *texture) {
	this->textureAlbedo = texture;
}

void DeferredRenderer::setTextureNormal(gl::Texture *texture) {
	this->textureNormal = texture;
}

void DeferredRenderer::setTextureSpecular(gl::Texture *texture) {
	this->textureSpecular = texture;
}

void DeferredRenderer::setTextureGloss(gl::Texture *texture) {
	this->textureGloss = texture;
}

void DeferredRenderer::setMaterial(Material *material) {
	setTextureAlbedo(&material->albedo);
	setTextureNormal(&material->normal);
	setTextureSpecular(&material->specular);
	setTextureGloss(&material->gloss);
}

void DeferredRenderer::setCubeMap(CubeMap *map) {
	this->cubeMap = map;
}

void DeferredRenderer::captureBegin() {
	// Reset timer.
	tempTime.start();

	// Bind deferred FBO.
	deferredFBO.bindFramebuffer();

	// Set viewport as gbuffer size.
	viewportPrev = gl::getViewport(); // Store prev viewport.
	gl::setViewport(Area(0, 0, width, height));

	// Bind textures/else.
	textureAlbedo->bind(0);
	textureNormal->bind(1);
	textureSpecular->bind(2);
	textureGloss->bind(3);
	cubeMap->bindMulti(CUBE_MAP_LOC);

	// Bind shader and set uniforms.
	deferredShader->bind();
	deferredShader->uniform("textureAlbedo", 0); 
	deferredShader->uniform("textureNormal", 1); 
	deferredShader->uniform("textureSpecular", 2); 
	deferredShader->uniform("textureGloss", 3); 

	deferredShader->uniform("cubeMap", CUBE_MAP_LOC);
	deferredShader->uniform("cameraDirection", camera->getViewDirection());
	deferredShader->uniform("cameraEyePoint", camera->getEyePoint());

	// Clear buffer.
	gl::clear(Color(0.0f, 0.0f, 0.0f));

	// Push, set camera matrix.
	glPushMatrix();
	gl::setMatrices(*camera);

	// Enable depth.
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Enable multisampling.
	glEnable(GL_MULTISAMPLE_ARB);
}

void DeferredRenderer::captureEnd() {
	// Disable depth.
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Pop camera matrix.
	glPopMatrix();

	// Undind albedo.
	textureAlbedo->unbind();
	textureNormal->unbind();
	textureSpecular->unbind();

	// Unbind shader.
	deferredShader->unbind();

	// Unbind deferred FBO.
	deferredFBO.unbindFramebuffer();

	// Restore viewport.
	gl::setViewport(viewportPrev);

	// Stop timer. Save.
	captureTime = tempTime.getSeconds();
	tempTime.stop();
}

PointLight *(DeferredRenderer::createLight)(Vec3f position, float radius) {
	// Create light.
	PointLight *light = new PointLight();

	// Params.
	light->setPosition(position);
	light->setRadius(radius);

	// Add to list.
	lightList.push_back(light);

	// Return this light.
	return light;
}

void DeferredRenderer::renderLights() {
	// Reset timer.
	tempTime.start();

	// Bind buffers.
	getBufferTexture(ALBEDO_AND_DEPTH).bind(0);
	getBufferTexture(NORMAL).bind(1);
	getBufferTexture(POSITION).bind(2);
	getBufferTexture(ENVIRO).bind(3);

	// Set viewport.
	viewportPrev = gl::getViewport(); // Store prev.
	gl::setViewport(lightFBO.getBounds());

	// Bind shader.
	pointLightShader->bind();

		// Uniforms.
		pointLightShader->uniform("albedoAndDepthMap", 0);
		pointLightShader->uniform("normalMap", 1);
		pointLightShader->uniform("positionMap", 2);
		pointLightShader->uniform("cameraPosition", camera->getEyePoint());

		// Additive blending (enable).
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		// Backface culling on.
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT); // Front, because light is invisible if camera is inside of cube.

		// Bind FBO.
		lightFBO.bindFramebuffer();

			// Clear.
			gl::clear(Color::black());

			// Camera matrix.
			glPushMatrix();
			gl::setMatrices(*camera);
				// Draw light-cubes.
				for (size_t i = 0; i < lightList.size(); i++) {
					// Set uniform per light.
					pointLightShader->uniform("lightTransformedPosition", camera->getModelViewMatrix().transformPointAffine(lightList[i]->getPosition()));
					pointLightShader->uniform("lightPosition", lightList[i]->getPosition());
					pointLightShader->uniform("lightRadius", lightList[i]->getRadius() * .5f);
					pointLightShader->uniform("lightColor", lightList[i]->getColor());
					pointLightShader->uniform("lightScreenPosition", camera->worldToScreen(lightList[i]->getPosition(), 1.0f, 1.0f));
					// Draw cube.
					gl::drawCube(lightList[i]->getPosition(), Vec3f(
						lightList[i]->getRadius(), 
						lightList[i]->getRadius(), 
						lightList[i]->getRadius()));
				}
			glPopMatrix();

		// Unbind FBO.
		lightFBO.unbindFramebuffer();

		// Stop blending.
		glDisable(GL_BLEND);

		// Off culling.
		glDisable(GL_CULL_FACE);

	// Unbind shader.
	pointLightShader->unbind();

	// Restore viewport.
	gl::setViewport(viewportPrev);

	lightsRenderTime = tempTime.getSeconds();
	tempTime.stop();
}

double DeferredRenderer::getRenderLightsTime() {
	return lightsRenderTime;
}

double DeferredRenderer::getCaptureTime() {
	return captureTime;
}

int DeferredRenderer::getNumberOfLights() {
	return lightList.size();
}


std::vector<PointLight*> DeferredRenderer::getLights() {
	return lightList;
}

void DeferredRenderer::compose() {
	viewportPrev = gl::getViewport();
	gl::setViewport(compositionFBO.getBounds());

	deferredFBO.getTexture(0).bind(0);
	deferredFBO.getTexture(1).bind(1);
	deferredFBO.getTexture(2).bind(2);
	deferredFBO.getTexture(3).bind(3);
	lightFBO.getTexture(0).bind(4);

	compositionFBO.bindFramebuffer();

		composeShader->bind();
		composeShader->uniform("albedoAndDepthMap", 0);
		composeShader->uniform("normalMap", 1);
		composeShader->uniform("positionMap", 2);
		composeShader->uniform("enviroMap", 3);
		composeShader->uniform("lightMap", 4);

		glPushMatrix();
			glScalef(
				(float)compositionFBO.getTexture().getWidth(), 
				(float)compositionFBO.getTexture().getHeight(), 1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f); // 0, 1
				glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f); // 1, 1
				glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f); // 1, 0
				glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f); // 0, 0
			glEnd();
		glPopMatrix(); 

		composeShader->unbind();

	compositionFBO.unbindFramebuffer();

	gl::setViewport(viewportPrev);
}
