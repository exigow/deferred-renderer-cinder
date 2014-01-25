#pragma once

#include <cinder/Vector.h>

using namespace ci;

class PointLight
{
public:
	PointLight();
	~PointLight(void);

	Vec3f getColor();
	void randomizeColor();

	void setPosition(float x, float y, float z);
	void setPosition(Vec3f position);
	Vec3f getPosition();

	void setRadius(float radius);
	float getRadius();

private:
	Vec3f color;
	float radius;
	Vec3f position;
};

