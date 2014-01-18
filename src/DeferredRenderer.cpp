#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(int width, int height) {
	this->setup(width, height);
}

DeferredRenderer::DeferredRenderer() {
	this->setup(640, 480);
}

DeferredRenderer::~DeferredRenderer() {
}

void DeferredRenderer::setCamera(CameraPersp *camera) {
	this->camera = camera;
}

void DeferredRenderer::setup(int width, int height) {
	deferredFBOFormat.setColorInternalFormat(GL_RGBA16F_ARB);
	deferredFBOFormat.enableColorBuffer(true, 3);
	deferredFBOFormat.setSamples(0);
	deferredFBOFormat.setCoverageSamples(0);
	deferredFBO = gl::Fbo(width, height, deferredFBOFormat);
		deferredFBO.getTexture(0).setFlipped(true);
		deferredFBO.getTexture(1).setFlipped(true);
		deferredFBO.getTexture(2).setFlipped(true);

	lightFBOFormat.setColorInternalFormat(GL_RGBA8);
	lightFBOFormat.setSamples(0);
	lightFBOFormat.setCoverageSamples(0);
	lightFBO = gl::Fbo(width, height, lightFBOFormat);
		lightFBO.getTexture(0).setFlipped(true);
}

void DeferredRenderer::renderLights() {
	deferredFBO.getTexture(0).bind(0);
	deferredFBO.getTexture(1).bind(1);
	deferredFBO.getTexture(2).bind(2);

	gl::setViewport(lightFBO.getBounds());
	pointLightShader->bind();

		pointLightShader->uniform("albedoAndDepthMap", 0);
		pointLightShader->uniform("normalMap", 1);
		pointLightShader->uniform("positionMap", 2);

		pointLightShader->uniform("cameraPosition", camera->getEyePoint());

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		lightFBO.bindFramebuffer();
			gl::clear(Color::black());

			gl::pushMatrices();
				gl::setMatrices(*camera);
				for (size_t i = 0; i < lightList.size(); i++) {
					pointLightShader->uniform("lightPosition", camera->getModelViewMatrix().transformPointAffine(lightList[i]->getPosition()));
					pointLightShader->uniform("lightRadius", lightList[i]->getRadius() * .5f);
					pointLightShader->uniform("lightColor", lightList[i]->getColor());
					gl::drawCube(lightList[i]->getPosition(), Vec3f(
						lightList[i]->getRadius(), 
						lightList[i]->getRadius(), 
						lightList[i]->getRadius()));
				}
			gl::popMatrices();

		lightFBO.unbindFramebuffer();

		glDisable(GL_BLEND);

	pointLightShader->unbind();
}

void DeferredRenderer::drawQuad() {
	gl::pushMatrices();
	gl::scale(128.0f, 128.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
	glEnd();
	gl::popMatrices();
}