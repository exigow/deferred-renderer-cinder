#pragma once

#include <cinder/Vector.h>

using namespace ci;

namespace Utilities {
	Vec3f getPositionFromBulletMatrix(float matrix[16]);
}
