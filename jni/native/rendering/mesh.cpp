#include "mesh.h"
#include "material.h"

namespace {
	const char* vertexShader =
		"#version 100\n"
		"uniform highp mat4 projection;\n"
		"uniform highp mat4 modelView;\n"
		"uniform mediump mat3 normalMatrix;\n"
		"attribute highp vec3 vertex;\n"
		"attribute mediump vec3 normal;\n"
		"varying mediump vec3 v_normal;\n"
		"varying lowp vec3 v_lightDir;\n"
		// "varying lowp vec3 v_lightDir2;\n"

		"#ifdef COLORS\n"
		"attribute lowp vec3 vertexColor;\n"
		"varying lowp vec3 v_vertexColor;\n"
		"#endif\n"

		"void main() {\n"
		"  v_normal = normalize(normalMatrix * normal);\n"
		"#ifdef COLORS\n"
		"  v_vertexColor = vertexColor;\n"
		"#endif\n"
		// "  gl_Position = projection * modelView * vec4(vertex, 1.0);\n"
		"  gl_Position = projection * modelView * vec4(vertex*30.0, 1.0);\n"

		"  v_lightDir = normalize(vec3(0.0, 0.0, -1.0));\n"
		// "  v_lightDir2 = normalize(vec3(-1.0, -0.3, -0.4));\n"
		"}"
		;

	const char* fragmentShader =
		"#version 100\n"
		"uniform lowp float value;\n"
		"varying mediump vec3 v_normal;\n"
		"varying lowp vec3 v_lightDir;\n"
		// "varying lowp vec3 v_lightDir2;\n"
		"#ifdef COLORS\n"
		"varying lowp vec3 v_vertexColor;\n"
		"#endif\n"
		"uniform lowp vec4 color;\n"

		"void main() {\n"
		"  lowp float NdotL = max(dot(normalize(v_normal), v_lightDir), 0.0);\n"
		// "  lowp float NdotL2 = max(dot(normalize(v_normal), v_lightDir2), 0.0);\n"
		// "  gl_FragColor = vec4(vec3(0.15+max(NdotL*0.65, NdotL2*0.35)), 1.0);\n"
		"#ifdef COLORS\n"
		"  gl_FragColor = vec4(v_vertexColor.xyz*NdotL, color.a);\n"
		"#else\n"
		"  gl_FragColor = vec4(color.xyz*NdotL, color.a);\n"
		"#endif\n"
		"}"
		;
}

Mesh::Mesh(const MeshData& data)
 : mMaterial(MaterialSharedPtr(
	             data.colors.empty()
	             ? new Material(vertexShader, fragmentShader)
	             : new Material("#define COLORS\n" + std::string(vertexShader), "#define COLORS\n" + std::string(fragmentShader))
             )),
   mBound(false),
   mColor(Vector3(1.0f)),
   mOpacity(1.0f),
   mVertexAttrib(-1), /*mTexCoordAttrib(-1),*/ mNormalAttrib(-1), mVertexColorAttrib(-1),
   mProjectionUniform(-1), mModelViewUniform(-1), mNormalMatrixUniform(-1), mColorUniform(-1),
   mNumFaces(0)
{
	android_assert(!data.indices.empty());

	for (const MeshData::Index idx : data.indices) {
		const Vector3& pos = data.vertices[idx.v];
		mMeshBuffer.push_back(pos.x);
		mMeshBuffer.push_back(pos.y);
		mMeshBuffer.push_back(pos.z);

		// TODO: texcoords

		const Vector3& normal = data.normals[idx.n];
		mMeshBuffer.push_back(normal.x);
		mMeshBuffer.push_back(normal.y);
		mMeshBuffer.push_back(normal.z);

		if (!data.colors.empty()) {
			const Vector3& color = data.colors[idx.c];
			mMeshBuffer.push_back(color.x);
			mMeshBuffer.push_back(color.y);
			mMeshBuffer.push_back(color.z);
		}

		++mNumFaces;
	}
}

void Mesh::setColor(const Vector3& color)
{
	mColor = color;
}

void Mesh::setOpacity(float opacity)
{
	mOpacity = opacity;
}

// (GL context)
void Mesh::bind()
{
	mMaterial->bind();

	mVertexAttrib = mMaterial->getAttribute("vertex");
	// TODO: texcoords
	mNormalAttrib = mMaterial->getAttribute("normal");
	mVertexColorAttrib = mMaterial->getAttribute("vertexColor");
	mModelViewUniform = mMaterial->getUniform("modelView");
	mProjectionUniform = mMaterial->getUniform("projection");
	mNormalMatrixUniform = mMaterial->getUniform("normalMatrix");
	mColorUniform = mMaterial->getUniform("color");

	android_assert(mVertexAttrib != -1);
	// TODO: texcoords
	android_assert(mNormalAttrib != -1);
	// mVertexColorAttrib may be -1
	android_assert(mModelViewUniform != -1);
	android_assert(mProjectionUniform != -1);
	android_assert(mNormalMatrixUniform != -1);
	android_assert(mColorUniform != -1);

	mBound = true;
}

// (GL context)
void Mesh::render(const Matrix4& projectionMatrix, const Matrix4& modelViewMatrix)
{
	if (!mBound)
		bind();

	glUseProgram(mMaterial->getHandle());

	// TODO: texcoords
	// NOTE: after adding texcoords, change the stride and the
	// starting indices accordingly

	const unsigned int stride = (mVertexColorAttrib == -1 ? 6 : 9);

	// Vertices
	glVertexAttribPointer(mVertexAttrib, 3, GL_FLOAT, false, stride*sizeof(GLfloat), mMeshBuffer.data());
	glEnableVertexAttribArray(mVertexAttrib);

	// Normals
	glVertexAttribPointer(mNormalAttrib, 3, GL_FLOAT, false, stride*sizeof(GLfloat), &mMeshBuffer[3]);
	glEnableVertexAttribArray(mNormalAttrib);

	if (mVertexColorAttrib != -1) {
		// Vertex colors
		glVertexAttribPointer(mVertexColorAttrib, 3, GL_FLOAT, false, stride*sizeof(GLfloat), &mMeshBuffer[6]);
		glEnableVertexAttribArray(mVertexColorAttrib);
	}

	// Uniforms
	glUniformMatrix4fv(mProjectionUniform, 1, false, projectionMatrix.data_);
	glUniformMatrix4fv(mModelViewUniform, 1, false, modelViewMatrix.data_);
	glUniformMatrix3fv(mNormalMatrixUniform, 1, false, modelViewMatrix.inverse().transpose().get3x3Matrix().data_);
	glUniform4f(mColorUniform, mColor.x, mColor.y, mColor.z, mOpacity);

	// Rendering
	glDrawArrays(GL_TRIANGLES, 0, mNumFaces);

	glDisableVertexAttribArray(mVertexAttrib);
	// TODO: texcoords
	glDisableVertexAttribArray(mNormalAttrib);

	if (mVertexColorAttrib != -1)
		glDisableVertexAttribArray(mVertexColorAttrib);
}
