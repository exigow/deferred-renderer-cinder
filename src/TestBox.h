#pragma once
#include "cinder/Vector.h"
#include "PointLight.h"
#include "CinderBullet.h"

using namespace ci;

class TestBox
{
public:
	TestBox();

	Vec3f position;
	PointLight *light;
	ph::bullet::shape::Box *shape;
};

TestBox::TestBox() {
}

