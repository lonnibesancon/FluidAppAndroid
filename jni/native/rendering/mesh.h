#ifndef MESH_H
#define MESH_H

#include "global.h"

#include "renderable.h"

struct MeshData
{
	std::vector<Vector3> vertices;
	std::vector<Vector2> texCoords;
	std::vector<Vector3> normals;
	std::vector<Vector3> colors;

	// Indices for vertex/texcoord/normal values in the
	// vertices/texCoords/normals arrays
	struct Index { unsigned int v, t, n, c; };
	std::vector<Index> indices;
};

class Mesh : public Renderable
{
public:
	Mesh(const MeshData& data);

	// (GL context)
	void bind();

	// Ignored if the mesh already contains vertex colors
	void setColor(const Vector3& color);

	void setOpacity(float opacity);

	// (GL context)
	void render(const Matrix4& projectionMatrix, const Matrix4& modelViewMatrix);

private:
	MaterialSharedPtr mMaterial;
	bool mBound;
	Vector3 mColor;
	float mOpacity;
	GLint mVertexAttrib, /*mTexCoordAttrib,*/ mNormalAttrib, mVertexColorAttrib;
	GLint mProjectionUniform, mModelViewUniform, mNormalMatrixUniform, mColorUniform;
	std::vector<GLfloat> mMeshBuffer;
	unsigned int mNumFaces;
};

#endif /* MESH_H */
