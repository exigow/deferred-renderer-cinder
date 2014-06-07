#include "Material.h"

#include <cinder/Vector.h>
#include <cinder/gl/Texture.h>
#include <cinder/Xml.h>
#include <cinder/Utilities.h>
#include <cinder/ImageIo.h>
#include <cinder/app/App.h>

using namespace ci;
using namespace ci::app;
using namespace std;

Material::Material(cinder::DataSourceRef xmlSource) {
	XmlTree *xml = new XmlTree(xmlSource);
	this->albedo = gl::Texture(loadImage(loadAsset(xml->getChild("material/albedo").getValue())));
	this->normal = gl::Texture(loadImage(loadAsset(xml->getChild("material/normal").getValue())));
	this->specular = gl::Texture(loadImage(loadAsset(xml->getChild("material/specular").getValue())));
	this->gloss = gl::Texture(loadImage(loadAsset(xml->getChild("material/gloss").getValue())));
	delete xml;
}

Material::~Material() {
}
