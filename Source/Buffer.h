#pragma once

#include <JuceHeader.h>

class Buffer
{
public:
	enum class Fill
	{
		once = juce::gl::GL_STATIC_DRAW,
		ongoing = juce::gl::GL_DYNAMIC_DRAW
	};

	enum class ComponentSize
	{
		xy = 2,
		xyz = 3,
		rgb = 3,
		rgba = 4,
		uv = 2
	};

	enum class DataType
	{
		integer = juce::gl::GL_INT,
		floatingPoint = juce::gl::GL_FLOAT,
		unsignedInteger = juce::gl::GL_UNSIGNED_INT
	};

	enum Vbo
	{
		vertexBuffer,
		colourBuffer,
		textureBuffer,
		normalBuffer
	};

	enum class RenderMode
	{
		points = juce::gl::GL_POINTS,
		triangles = juce::gl::GL_TRIANGLES,
		lines = juce::gl::GL_LINES,
		lineLoop = juce::gl::GL_LINE_LOOP,
		lineStrip = juce::gl::GL_LINE_STRIP
	};

	Buffer();

	static juce::OpenGLContext* getContext();

	static void setGLStates();
	static void setLineWidth(GLfloat lineWidth);
	static void setPointSize(GLfloat pointSize);
	static void setContext(juce::OpenGLContext* context);

	void create(GLuint totalVertices, bool hasEbo = false);
	void fillVbo(Vbo vbo, GLfloat* data, GLsizeiptr bufferSize, Fill fill = Fill::once);
	void fillEbo(GLuint* data, GLsizeiptr bufferSize, Fill fill = Fill::once);
	void appendVbo(Vbo vbo, GLfloat* data, GLsizeiptr size, GLuint offset);
	void appendEbo(GLuint* data, GLsizeiptr size, GLuint offset);
	void linkVbo(GLuint attributeID, Vbo vbo, ComponentSize componentSize, DataType dataType);
	void disableAttribute(GLuint attributeID);
	void render(RenderMode renderMode, GLuint maxRenderVertices = 0);
	void destroy();

private:

	bool hasEbo;
	GLuint ebo;
	GLuint totalVertices;
	GLuint vbos[4];

	static juce::OpenGLContext* glContext;
};