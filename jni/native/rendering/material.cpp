#include "material.h"

Material::Material(const std::string& vertexShaderSrc,
                   const std::string& fragmentShaderSrc)
 : mProgramHandle(0),
   mVertexShaderSrc(vertexShaderSrc), mFragmentShaderSrc(fragmentShaderSrc)
{}

// (GL context)
void Material::bind()
{
	mProgramHandle = compileProgram(mVertexShaderSrc, mFragmentShaderSrc);
}

// (GL context)
GLint Material::getAttribute(const std::string& name) const
{
	android_assert(mProgramHandle != 0);
	return glGetAttribLocation(mProgramHandle, name.c_str());
}

// (GL context)
GLint Material::getUniform(const std::string& name) const
{
	android_assert(mProgramHandle != 0);
	return glGetUniformLocation(mProgramHandle, name.c_str());
}

// (GL context)
GLuint Material::compileProgram(
	const std::string& vertexShaderSrc,
	const std::string& fragmentShaderSrc)
{
	GLuint vertexShader = 0, fragmentShader = 0;

	if ((vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc)) == 0)
		return 0;

	if ((fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc)) == 0)
		return 0;

	GLuint program = glCreateProgram();
	if (program != 0) {
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		GLint linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			LOGE("Could not link program:\n"
			     "---- vertex shader ----\n%s\n"
			     "---- fragment shader ---\n%s",
			     vertexShaderSrc.c_str(),
			     fragmentShaderSrc.c_str());
			int maxLength;
			int infologLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &infologLength, infoLog.data());
			if (infologLength > 0)
				LOGE("%s", infoLog.data());
			glDeleteProgram(program);
			program = 0;
		}
	}

	return program;
}

// (GL context)
GLuint Material::compileShader(GLenum type, const std::string& source)
{
	GLuint shader = glCreateShader(type);
	if (shader != 0) {
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == 0) {
			LOGE("Could not compile %s shader:", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
			LOGE("%s", src);
			int maxLength;
			int infologLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<char> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &infologLength, infoLog.data());
			if (infologLength > 0)
				LOGE("%s", infoLog.data());
			glDeleteShader(shader);
			shader = 0;
		}
	}

	return shader;
}
