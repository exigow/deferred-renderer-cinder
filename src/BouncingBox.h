#pragma once
#include "cinder/Vector.h"
#include "PointLight.h"
#include "CinderBullet.h"

using namespace ci;

class BouncingBox
{
public:
	BouncingBox();

	Vec3f position;
	PointLight *light;
	ph::bullet::shape::Box *shape;
};

BouncingBox::BouncingBox() {
}