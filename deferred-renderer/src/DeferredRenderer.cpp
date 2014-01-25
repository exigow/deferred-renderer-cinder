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

	// Setup FBOs.
	deferredFBOFormat.setColorInternalFormat(GL_RGBA16F_ARB);
	deferredFBOFormat.enableColorBuffer(true, 3);
	deferredFBOFormat.setSamples(0);
	deferredFBOFormat.setCoverageSamples(0);
	deferredFBO = gl::Fbo(width, height, deferredFBOFormat);
		deferredFBO.getTexture(0).setFlipped(true);
		deferredFBO.getTexture(1).setFlipped(true);
		deferredFBO.getTexture(2).setFlipped(true);

	lightFBOFormat.setColorInternalFormat(GL_RGB8);
	lightFBOFormat.setSamples(0);
	lightFBOFormat.setCoverageSamples(0);
	lightFBO = gl::Fbo(width, height, lightFBOFormat);
		lightFBO.getTexture(0).setFlipped(true);
}

gl::Texture DeferredRenderer::getBufferTexture(DeferredRenderer::BufferTexture texture) {
	gl::Fbo *fbo;
	int fromBuffer;
	if (texture == BUFTEX_ALBEDO_AND_DEPTH || texture == BUFTEX_NORMAL || texture == BUFTEX_POSITION) {
		fbo = &deferredFBO;
		switch (texture) {
			case BUFTEX_ALBEDO_AND_DEPTH: {
				fromBuffer = 0;
				break;
			}
			case BUFTEX_NORMAL: {
				fromBuffer = 1;
				break;
			}
			case BUFTEX_POSITION: {
				fromBuffer = 2;
				break;
			}
		}
	} else {
		if (texture == BUFTEX_LIGHT) {
			fbo = &lightFBO;
			fromBuffer = 0;
		}
	}

	return fbo->getTexture(fromBuffer);
}


void DeferredRenderer::setTextureAlbedo(gl::Texture *texture) {
	this->texture = texture;
}

void DeferredRenderer::captureBegin() {
	// Bind deferred FBO.
	deferredFBO.bindFramebuffer();

	// Set viewport as gbuffer size.
	viewportPrev = gl::getViewport(); // Store prev viewport.
	gl::setViewport(Area(0, 0, width, height));

	// Bind albedo tex. as 0.
	texture->bind(0);

	// Bind shader and set uniforms.
	deferredShader->bind();
	deferredShader->uniform("mTexture", 0); 

	// Clear buffer.
	gl::clear(Color(.5f, 0.0f, 0.0f));

	// Push, set camera matrix.
	glPushMatrix();
	gl::setMatrices(*camera);

	// Enable depth.
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void DeferredRenderer::captureEnd() {
	// Disable depth.
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Pop camera matrix.
	glPopMatrix();

	// Undind albedo.
	texture->unbind();

	// Unbind shader.
	deferredShader->unbind();

	// Unbind deferred FBO.
	deferredFBO.unbindFramebuffer();

	// Restore viewport.
	gl::setViewport(viewportPrev);
}

PointLight *(DeferredRenderer::createLight)(Vec3f position, float radius) {
	PointLight *light = new PointLight();
	light->setPosition(position);
	light->setRadius(radius);
	lightList.push_back(light);
	return light;
}

void DeferredRenderer::renderLights() {
	// Bind buffers.
	getBufferTexture(BUFTEX_ALBEDO_AND_DEPTH).bind(0);
	getBufferTexture(BUFTEX_NORMAL).bind(1);
	getBufferTexture(BUFTEX_POSITION).bind(2);

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
					pointLightShader->uniform("lightPosition", camera->getModelViewMatrix().transformPointAffine(lightList[i]->getPosition()));
					pointLightShader->uniform("lightRadius", lightList[i]->getRadius() * .5f);
					pointLightShader->uniform("lightColor", lightList[i]->getColor());
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

	// Unbind shader.
	pointLightShader->unbind();

	// Restore viewport.
	gl::setViewport(viewportPrev);
}