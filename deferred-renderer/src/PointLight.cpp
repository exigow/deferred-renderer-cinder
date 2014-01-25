#include "PointLight.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Color.h"

PointLight::PointLight() {
	this->position = Vec3f::zero();
	this->radius = 16.0f + randFloat() * 8.0f;
	this->randomizeColor();
}

PointLight::~PointLight(void) {
}

void PointLight::randomizeColor() {
	Color *_col = new Color(CM_HSV, Vec3f(randFloat(), 1.0f, 1.0f));
	color.x = _col->r;
	color.y = _col->g;
	color.z = _col->b;
	delete _col;
}

Vec3f PointLight::getColor() {
	return color;
}

void PointLight::setPosition(Vec3f position) {
	this->position.set(position);
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