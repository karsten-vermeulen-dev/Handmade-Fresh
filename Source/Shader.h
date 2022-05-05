#pragma once

#include "JuceHeader.h"
#include "juce_core/juce_core.h"
#include <map>
#include <string>

class Shader : public juce::OpenGLShaderProgram
{

public:

	Shader(juce::OpenGLContext& glContext);

	std::unique_ptr<Attribute> vertexIn;
	std::unique_ptr<Attribute> colourIn;
	std::unique_ptr<Uniform> projection;
};