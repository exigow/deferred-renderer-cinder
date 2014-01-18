#pragma once
#include "cinder/Vector.h"

using namespace ci;

class PointLight
{
public:
	PointLight();
	~PointLight(void);

	void setColor(float red, float green, float blue);
	Vec3f getColor();

	void setPosition(float x, float y, float z);
	Vec3f getPosition();

	void setRadius(float radius);
	float getRadius();

private:
	Vec3f color;
	float radius;
	Vec3f position;
};

