#include "surface_2d.h"
#include "material.h"

namespace {
	const GLfloat texturedQuadBuffer[] = {
		//X, Y, Z,  U,  V
		-1, -1, 0,  0, 1,  // bottom left
		 1, -1, 0,  1, 1,  // bottom right
		-1,  1, 0,  0, 0,  // top left
		 1,  1, 0,  1, 0   // top right
	};

	const GLsizei bufferTypeSize = sizeof(texturedQuadBuffer[0]);
	const GLsizei bufferStride = 5 * bufferTypeSize;
	const GLsizei numPoints = sizeof(texturedQuadBuffer)/bufferStride;

	const char* vertexShader =
		"#version 100\n"
		"uniform highp mat4 projection;\n"
		"attribute highp vec3 vertex;\n"
		"attribute mediump vec2 texCoord;\n"
		"varying mediump vec2 v_texCoord;\n"
		"void main() {\n"
		"  v_texCoord = texCoord;\n"
		"  gl_Position = projection * vec4(vertex, 1.0);\n"
		"}";

	const char* fragmentShader =
		"#version 100\n"
		"varying mediump vec2 v_texCoord;\n"
		"uniform highp sampler2D texture;\n"
		"void main() {\n"
		"  gl_FragColor = vec4(texture2D(texture, v_texCoord).rgb, 1.0);\n"
		"}";
} // namespace

Surface2D::Surface2D(unsigned int width, unsigned int height)
 : mWidth(width), mHeight(height),
   mBound(false),
   mTextureHandle(0),
   mMaterial(MaterialSharedPtr(new Material(vertexShader, fragmentShader))),
   mVertexAttrib(-1), mTexCoordAttrib(-1),
   mProjectionUniform(-1),
   mTextureInitialized(false)
{}

// (GL context)
void Surface2D::bind()
{
	mMaterial->bind();

	mVertexAttrib = mMaterial->getAttribute("vertex");
	mTexCoordAttrib = mMaterial->getAttribute("texCoord");
	mProjectionUniform = mMaterial->getUniform("projection");

	android_assert(mVertexAttrib != -1);
	android_assert(mTexCoordAttrib != -1);
	android_assert(mProjectionUniform != -1);

	glGenTextures(1, &mTextureHandle);

	glBindTexture(GL_TEXTURE_2D, mTextureHandle);

	// Only GL_NEAREST/GL_LINEAR and GL_CLAMP_TO_EDGE are
	// supported for non-power-of-two textures according to the
	// OpenGL ES 2.0 specification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Initialize the texture with no content
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		mWidth, mHeight,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	mBound = true;
}

// (GL context)
void Surface2D::updateTexture(unsigned char* data)
{
	if (!mBound)
		bind();

	android_assert(data);
	android_assert(mTextureHandle != 0);

	glBindTexture(GL_TEXTURE_2D, mTextureHandle);

	// Input data is RGB (3 bytes per pixel), not aligned to a 4-byte
	// boundary (default unpack alignment)
	// http://www.opengl.org/wiki/Common_Mistakes#Texture_upload_and_pixel_reads
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Update the texture contents
	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		0, 0,
		mWidth, mHeight,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		data
	);

	mTextureInitialized = true;
}

// (GL context)
void Surface2D::render(const Matrix4& projectionMatrix)
{
	if (!mBound)
		bind();

	if (!mTextureInitialized)
		return;

	// Vertices
	glVertexAttribPointer(mVertexAttrib, 3, GL_FLOAT, false,
		bufferStride, texturedQuadBuffer);
	glEnableVertexAttribArray(mVertexAttrib);

	// Texture coordinates
	glVertexAttribPointer(mTexCoordAttrib, 2, GL_FLOAT, false,
		bufferStride, &texturedQuadBuffer[3]);
	glEnableVertexAttribArray(mTexCoordAttrib);

	// Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureHandle);

	// Projection (modelView = identity)
	glUseProgram(mMaterial->getHandle());
	glUniformMatrix4fv(mProjectionUniform, 1, false, projectionMatrix.data_);

	// Rendering
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numPoints);

	glDisableVertexAttribArray(mVertexAttrib);
	glDisableVertexAttribArray(mTexCoordAttrib);
}
