#pragma once

#include <cinder/Vector.h>
#include <string>

using namespace ci;
using namespace std;

namespace Utilities {
	Vec3f getPositionFromBulletMatrix(float matrix[16]);
	void drawStringSimple(string text, float x, float y);
}
