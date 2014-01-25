#include "Utilities.h"

#include <cinder/Vector.h>
#include <string>
#include <cinder/Cinder.h>
#include <cinder/gl/gl.h>

using namespace ci;
using namespace std;

Vec3f Utilities::getPositionFromBulletMatrix(float matrix[16]) {
	return Vec3f(matrix[3 * 4 + 0], matrix[3 * 4 + 1], matrix[3 * 4 + 2]);
}

void Utilities::drawStringSimple(string text, float x, float y) {
	gl::enableAlphaBlending();
	gl::drawString(text, Vec2f(x, y), Color::white(), Font("Arial", 16.0f));
	gl::disableAlphaBlending();
}


