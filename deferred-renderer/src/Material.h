#pragma once

#include <cinder/Vector.h>
#include <cinder/gl/Texture.h>
#include <cinder/Xml.h>

using namespace ci;

class Material {
public:
	Material(cinder::DataSourceRef xmlSource);
	~Material();

	gl::Texture 
		albedo, 
		normal, 
		specular,
		gloss;
};