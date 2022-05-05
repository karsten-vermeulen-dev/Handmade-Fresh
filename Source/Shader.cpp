#include "Shader.h"

Shader::Shader(juce::OpenGLContext& glContext) : juce::OpenGLShaderProgram(glContext)
{
	juce::String sourceCodeVertex, sourceCodeFragment;
	auto file = juce::File(__FILE__).getParentDirectory().getChildFile("Shaders/Main.vert");
	if (!file.exists())
	{
		std::cout << "Shader file " << file.getFullPathName() << " not found" << std::endl;
		jassertfalse;
	}
	sourceCodeVertex = file.loadFileAsString();

	file = file.withFileExtension(".frag");

	if (!file.exists())
	{
		std::cout << "Shader file " << file.getFullPathName() << " not found" << std::endl;
		jassertfalse;
	}

	sourceCodeFragment = file.loadFileAsString();

	if (!addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(sourceCodeVertex)))
	{
		std::cout << getLastError() << std::endl;
		jassertfalse;
	}

	if (!addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(sourceCodeFragment)))
	{
		std::cout << getLastError() << std::endl;
		jassertfalse;
	}

	if (!link())
	{
		std::cout << getLastError() << std::endl;
		jassertfalse;
	}

	vertexIn = std::make_unique<Attribute>(*this, "vertexIn");
	colourIn = std::make_unique<Attribute>(*this, "colourIn");
}