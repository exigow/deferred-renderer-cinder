#include "PointLight.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"

PointLight::PointLight() {
	position = Vec3f::zero();
	radius = 8.0f + randFloat() * 4.0f;
	setColor(randFloat(), randFloat(), randFloat());
}

PointLight::~PointLight(void) {
}

void PointLight::setColor(float red, float green, float blue) {
	color.set(red, green, blue);
}
Vec3f PointLight::getColor() {
	return color;
}

void PointLight::setPosition(float x, float y, float z) {
	position.set(x, y, z);
}
Vec3f PointLight::getPosition() {
	return position;
}

void PointLight::setRadius(float radius) {
	this->radius = radius;
}
float PointLight::getRadius() {
	return radius;
}