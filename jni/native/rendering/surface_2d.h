#ifndef SURFACE_2D_H
#define SURFACE_2D_H

#include "global.h"

class Surface2D
{
public:
	Surface2D(unsigned int width, unsigned int height);

	// (GL context)
	void bind();

	// (GL context)
	void updateTexture(unsigned char* data);

	// (GL context)
	void render(const Matrix4& projectionMatrix);

	bool isTextureInitialized() const
	{ return mTextureInitialized; }

private:
	unsigned int mWidth, mHeight;
	bool mBound;
	GLuint mTextureHandle;
	MaterialSharedPtr mMaterial;
	GLint mVertexAttrib, mTexCoordAttrib;
	GLint mProjectionUniform;
	bool mTextureInitialized;
};

#endif /* SURFACE_2D_H */
