#pragma once
#include "CubeMap.h"

using ci::Surface8u;
using boost::shared_ptr;
using namespace ci;

CubeMap::CubeMap(cinder::DataSourceRef xmlSource) {
	XmlTree *xml = new XmlTree(xmlSource);
	const std::string root = "cubemap/";

	// Parse size.
	int width = fromString<int>(xml->getChild(root + "width").getValue()), 
		height = fromString<int>(xml->getChild(root + "height").getValue()); 

	// Parse texture asset file paths.
	const std::string xmlNames[6] = {"xpos", "ypos", "zpos", "xneg", "yneg", "zneg"};
	std::string xmlStrings[6];
	for (int i = 0; i < 6; i++) {
		xmlStrings[i] = xml->getChild(root + xmlNames[i]).getValue(); 
	}

	// Gen and bind texture.
	glGenTextures(1, &textureObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);

	// Assign positions.
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[0]))).getData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[1]))).getData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[2]))).getData());

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[3]))).getData());	
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[4]))).getData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Surface8u(loadImage(app::loadAsset(xmlStrings[5]))).getData());

	// Set texture filters.
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	// Delete xml tree.
	delete xml;
}

void CubeMap::bindMulti(int pos) {
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
}

void CubeMap::bind() {
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
}

void CubeMap::unbind() {
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
}

void CubeMap::enableFixedMapping() {
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_CUBE_MAP);
}

void CubeMap::disableFixedMapping() {
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_CUBE_MAP);
}
