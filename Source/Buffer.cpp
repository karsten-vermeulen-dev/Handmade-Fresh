#include "Buffer.h"

juce::OpenGLContext* Buffer::glContext = nullptr;

juce::OpenGLContext* Buffer::getContext()
{
	return glContext;
}

void Buffer::setGLStates()
{
	juce::gl::glEnable(juce::gl::GL_BLEND);
	juce::gl::glEnable(juce::gl::GL_LINE_SMOOTH);
	juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
}

void Buffer::setLineWidth(GLfloat lineWidth)
{
	juce::gl::glLineWidth(lineWidth);
}

void Buffer::setPointSize(GLfloat pointSize)
{
	juce::gl::glPointSize(pointSize);
}

void Buffer::setContext(juce::OpenGLContext* context)
{
	glContext = context;
}

Buffer::Buffer()
{
	ebo = 0;
	hasEbo = false;
	totalVertices = 0;
	vbos[vertexBuffer] = 0;
	vbos[colourBuffer] = 0;
	vbos[textureBuffer] = 0;
	vbos[normalBuffer] = 0;
}

void Buffer::create(GLuint totalVertices, bool hasEbo)
{
	glContext->extensions.glGenBuffers(4, vbos);

	if (hasEbo)
	{
		glContext->extensions.glGenBuffers(1, &ebo);
	}

	this->hasEbo = hasEbo;
	this->totalVertices = totalVertices;
}

void Buffer::fillVbo(Vbo vbo, GLfloat* data, GLsizeiptr bufferSize, Fill fill)
{
	glContext->extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbos[vbo]);
	glContext->extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, bufferSize, data, static_cast<GLenum> (fill));
}

void Buffer::fillEbo(GLuint* data, GLsizeiptr bufferSize, Fill fill)
{
	glContext->extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ebo);
	glContext->extensions.glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, bufferSize, data, static_cast<GLenum> (fill));
}

void Buffer::appendVbo(Vbo vbo, GLfloat* data, GLsizeiptr size, GLuint offset)
{
	glContext->extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbos[vbo]);
	glContext->extensions.glBufferSubData(juce::gl::GL_ARRAY_BUFFER, offset, size, data);
}

void Buffer::appendEbo(GLuint* data, GLsizeiptr size, GLuint offset)
{
	glContext->extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ebo);
	glContext->extensions.glBufferSubData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
}

void Buffer::linkVbo(GLuint attributeID, Vbo vbo, ComponentSize componentSize, DataType dataType)
{
	glContext->extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, vbos[vbo]);
	glContext->extensions.glVertexAttribPointer(attributeID, static_cast<GLint> (componentSize), static_cast<GLenum> (dataType), juce::gl::GL_FALSE, 0, nullptr);
	glContext->extensions.glEnableVertexAttribArray(attributeID);
}

void Buffer::disableAttribute(GLuint attributeID)
{
	glContext->extensions.glDisableVertexAttribArray(attributeID);
}

void Buffer::render(RenderMode renderMode, GLuint maxRenderVertices)
{
	auto verticesToRender = (maxRenderVertices > 0) ? maxRenderVertices : totalVertices;

	if (hasEbo)
	{
		glContext->extensions.glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, ebo);
		juce::gl::glDrawElements(static_cast<GLenum> (renderMode), static_cast<GLsizei> (verticesToRender), static_cast<GLenum>(DataType::unsignedInteger), nullptr);
	}

	else
	{
		juce::gl::glDrawArrays(static_cast<GLenum> (renderMode), 0, static_cast<GLsizei> (verticesToRender));
	}
}

void Buffer::destroy()
{
	if (hasEbo)
	{
		glContext->extensions.glDeleteBuffers(1, &ebo);
	}

	glContext->extensions.glDeleteBuffers(4, vbos);
}