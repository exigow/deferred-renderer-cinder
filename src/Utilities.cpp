#include "Utilities.h"

#include <cinder/Vector.h>

Vec3f Utilities::getPositionFromBulletMatrix(float matrix[16]) {
	return Vec3f(matrix[3 * 4 + 0], matrix[3 * 4 + 1], matrix[3 * 4 + 2]);
}