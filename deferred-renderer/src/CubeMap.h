#include <cinder/Surface.h>
#include <cinder/gl/gl.h>
#include <cinder/ImageIo.h>
#include <cinder/gl/Texture.h>
#include <cinder/Xml.h>
#include <cinder/app/AppBasic.h>
#include <cinder/Utilities.h>

class CubeMap {
public:
	CubeMap(cinder::DataSourceRef xmlSource);
	void bind();
	void bindMulti(int loc);
	void unbind();
	static void enableFixedMapping();
	static void disableFixedMapping();
private:
	unsigned int textureObject;	
};