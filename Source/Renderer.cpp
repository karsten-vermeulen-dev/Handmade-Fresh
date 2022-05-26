#include "Buffer.h"
#include "Renderer.h"

Renderer::Renderer(VermeulenLadderFilterAudioProcessor& audioProcessor) : audioProcessor(audioProcessor)
{
	context.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
	context.setRenderer(this);
	context.setContinuousRepainting(true);
	context.attachTo(*this);

	audioProcessor.setDrive(drive);
	audioProcessor.setVolume(volume);
	audioProcessor.setResonance(resonance);
	audioProcessor.setCutoffFrequency(frequency);
	audioProcessor.setMode(static_cast<int>(juce::dsp::LadderFilterMode::LPF24));

	auto SetupComponent = [&](juce::Label& label,
		juce::Slider& slider,
		juce::LookAndFeel_V4& lookAndFeel,
		double min, double max, double startValue,
		juce::Colour thumbColour,
		juce::Colour trackColour)
	{
		addAndMakeVisible(label);
		addAndMakeVisible(slider);

		slider.setRange(min, max);
		slider.setValue(startValue);
		slider.setNumDecimalPlacesToDisplay(2);

		lookAndFeel.setColour(juce::Slider::thumbColourId, thumbColour);
		lookAndFeel.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
		slider.setLookAndFeel(&lookAndFeel);
	};

	SetupComponent(driveLabel, driveSlider, lookAndFeelDriveSlider, 1.0, 100.0, drive,
		juce::Colour::fromFloatRGBA(0.15f, 0.25f, 0.8f, 1.0f),
		juce::Colour::fromFloatRGBA(0.15f, 0.5f, 0.8f, 1.0f));

	SetupComponent(resonanceLabel, resonanceSlider, lookAndFeelResonanceSlider, 0.0, 1.0, resonance,
		juce::Colour::fromFloatRGBA(0.15f, 0.25f, 0.8f, 1.0f),
		juce::Colour::fromFloatRGBA(0.15f, 0.5f, 0.8f, 1.0f));

	SetupComponent(frequencyLabel, frequencySlider, lookAndFeelFrequencySlider, 1.0, 24000.0, frequency,
		juce::Colour::fromFloatRGBA(0.15f, 0.25f, 0.8f, 1.0f),
		juce::Colour::fromFloatRGBA(0.15f, 0.5f, 0.8f, 1.0f));

	SetupComponent(historyLabel, historySlider, lookAndFeelHistorySlider, 1, maxHistory, history,
		juce::Colour::fromFloatRGBA(0.15f, 0.25f, 0.8f, 1.0f),
		juce::Colour::fromFloatRGBA(0.15f, 0.5f, 0.8f, 1.0f));

	SetupComponent(volumeLabel, volumeSlider, lookAndFeelVolumeSlider, 0.0, 1.0, volume,
		juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 1.0f),
		juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 1.0f));

	driveSlider.onValueChange = [&]
	{
		drive = static_cast<float>(driveSlider.getValue());
		audioProcessor.setDrive(drive);
	};

	resonanceSlider.onValueChange = [&]
	{
		resonance = static_cast<float>(resonanceSlider.getValue());
		audioProcessor.setResonance(resonance);
	};

	frequencySlider.onValueChange = [&]
	{
		audioProcessor.setCutoffFrequency(static_cast<float>((frequencySlider.getValue())));
	};

	historySlider.onValueChange = [&]
	{
		history = static_cast<int>(historySlider.getValue());
	};

	volumeSlider.onValueChange = [&]
	{
		volume = static_cast<float>(volumeSlider.getValue());
		audioProcessor.setVolume(volume);

		lookAndFeelVolumeSlider.setColour(juce::Slider::thumbColourId,
			juce::Colour::fromFloatRGBA(0.75f, 0.75f, 0.75f, 1.0f));

		lookAndFeelVolumeSlider.setColour(juce::Slider::trackColourId,
			juce::Colour::fromFloatRGBA(static_cast<float>(1.0 * volume),
				static_cast<float>(1.0 * (1.0 - volume)),
				0.0f,
				1.0f));

		volumeSlider.setLookAndFeel(&lookAndFeelVolumeSlider);
	};

	addAndMakeVisible(modeBox);

	const juce::StringArray list =
	{ "Low-pass, 12 dB/octave",
	  "High-pass, 12 dB/octave",
	  "Band-pass, 12 dB/octave",
	  "Low-pass, 24 dB/octave",
	  "High-pass, 24 dB/octave",
	  "Band-pass, 24 dB/octave" };

	modeBox.addItemList(list, 1);
	modeBox.setSelectedId(4);

	modeBox.onChange = [&]
	{
		audioProcessor.setMode(modeBox.getSelectedId() - 1);
	};

	lookAndFeelModeBox.setColour(juce::ComboBox::textColourId, juce::Colour::fromFloatRGBA(0.75f, 0.75f, 0.75f, 1.0f));
	modeBox.setLookAndFeel(&lookAndFeelModeBox);
}

Renderer::~Renderer()
{

}

void Renderer::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
	cameraPosition.z -= wheel.deltaY * cameraSpeed;
}

void Renderer::mouseDrag(const juce::MouseEvent& event)
{
	auto deltaX = (event.x - event.mouseDownPosition.x) / 1280.0f;
	auto deltaY = (event.y - event.mouseDownPosition.y) / 720.0f;
	
	cameraPosition.x -= deltaX * mouseDragSpeed;
	cameraPosition.y += deltaY * mouseDragSpeed;
}

void Renderer::newOpenGLContextCreated()
{
	Buffer::setContext(&context);

	shader = std::make_unique<Shader>(context);

	buffer.create(maxVertices);

	buffer.fillVbo(Buffer::Vbo::vertexBuffer,
		(GLfloat*) nullptr,
		maxHistory * maxVertices * static_cast<int> (Buffer::ComponentSize::xy) * sizeof(GLfloat), Buffer::Fill::ongoing);

	buffer.fillVbo(Buffer::Vbo::colourBuffer,
		(GLfloat*) nullptr,
		maxHistory * maxVertices * static_cast<int> (Buffer::ComponentSize::rgba) * sizeof(GLfloat), Buffer::Fill::ongoing);

	const float farClip = 1000.0f;
	const float nearClip = 0.1f;
	const auto aspectRatio = 1280.0f / 720.0f;
	const auto tanHalfFov = tan(juce::degreesToRadians(30.0f) / 2.0f);

	//Formula to build a 3D perspective view using data above (taken from GLM math library)
	//We are using a column-major matrix implementation here
	projectionMatrix.mat[0] = 1.0f / (aspectRatio * tanHalfFov);
	projectionMatrix.mat[5] = 1.0f / (tanHalfFov);
	projectionMatrix.mat[10] = -(farClip + nearClip) / (farClip - nearClip);
	projectionMatrix.mat[11] = -1.0f;
	projectionMatrix.mat[14] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
	projectionMatrix.mat[15] = 0.0f;

	//Build a model matrix to simulate the opposite of a camera movement
	//We want to move the 'camera' back and up a little
	cameraPosition.x = 0.0f;
	cameraPosition.y = 0.15f;
	cameraPosition.z = 0.25f;
}

void Renderer::renderOpenGL()
{
	//Build a model matrix to simulate the opposite of a camera movement
	//We want to move the 'camera' back and up a little
	modelMatrix.mat[12] = -cameraPosition.x;
	modelMatrix.mat[13] = -cameraPosition.y;
	modelMatrix.mat[14] = -cameraPosition.z;

	juce::OpenGLHelpers::clear(juce::Colour(23, 24, 23));

	shader->use();
	shader->model->setMatrix4(modelMatrix.mat, 1, juce::gl::GL_FALSE);
	shader->projection->setMatrix4(projectionMatrix.mat, 1, juce::gl::GL_FALSE);

	juce::gl::glViewport(0,
		static_cast<int>(getBounds().getHeight() * 0.17f),
		getBounds().getWidth(),
		static_cast<GLsizei>(getBounds().getHeight() * 0.83f));

	auto numSamples = audioProcessor.getNumSamples();
	auto driveNormalized = drive / 100.0f;

	//We are only using left channel data for now
	auto channelDataLeft = audioProcessor.getChannelDataLeft();
	auto channelDataRight = audioProcessor.getChannelDataLeft();

	

	//FILL===================================================================================

	std::vector<float> vertices;
	std::vector<float> colors;

	auto halfSamples = numSamples * 0.5f;
	auto startX = 0.0f - (halfSamples * 0.001f);

	//One 'time slice' of data
	for (int i = 0; i < numSamples; i++)
	{
		auto data = std::abs(channelDataLeft[i]);

		vertices.push_back(startX + i * 0.001f);   //x
		vertices.push_back(-0.5f + data);          //y

		colors.push_back(0.0f);                                                 //r
		colors.push_back(0.57f - resonance - driveNormalized);                  //g
		colors.push_back(1.0f - driveNormalized);                               //b
		colors.push_back(1.0f - abs(static_cast<float>(i) / halfSamples - 1));  //a
	}

	auto dataSizeVertex = vertices.size() * sizeof(GLfloat);
	auto dataSizeColor = colors.size() * sizeof(GLfloat);

	static auto startPos = 0;
	static auto oldHistory = history;

	if (oldHistory < history)
	{
		oldHistory = history;
	}

	else if (oldHistory > history)
	{
		/*auto d = (oldHistory - history);
		auto v = d * maxVertices * static_cast<int> (Buffer::ComponentSize::xy) * sizeof(GLfloat);
		auto c = d * maxVertices * static_cast<int> (Buffer::ComponentSize::rgba) * sizeof(GLfloat);

		buffer.appendVbo(Buffer::Vbo::vertexBuffer, (GLfloat*)nullptr, v, history * dataSizeVertex);
		buffer.appendVbo(Buffer::Vbo::colourBuffer, (GLfloat*)nullptr, c, history * dataSizeColor);*/

		buffer.fillVbo(Buffer::Vbo::vertexBuffer,
			(GLfloat*) nullptr,
			maxHistory * maxVertices * static_cast<int> (Buffer::ComponentSize::xy) * sizeof(GLfloat), Buffer::Fill::ongoing);

		buffer.fillVbo(Buffer::Vbo::colourBuffer,
			(GLfloat*) nullptr,
			maxHistory * maxVertices * static_cast<int> (Buffer::ComponentSize::rgba) * sizeof(GLfloat), Buffer::Fill::ongoing);

		oldHistory = history;
	}

	if (startPos >= history)
	{
		startPos = 0;
	}

	buffer.appendVbo(Buffer::Vbo::vertexBuffer, vertices.data(), dataSizeVertex, startPos * dataSizeVertex);
	buffer.appendVbo(Buffer::Vbo::colourBuffer, colors.data(), dataSizeColor, startPos * dataSizeColor);

	//RENDER=================================================================================

	auto zPos = 0.0f;
	auto renderStart = startPos;

	Buffer::setGLStates();
	Buffer::setLineWidth(static_cast<GLfloat>(1.0f + (3.0f * driveNormalized)));

	for (int i = 0; i < history; i++)
	{
		shader->zPos->set(zPos);

		buffer.linkVbo(shader->vertexIn->attributeID, Buffer::vertexBuffer, Buffer::ComponentSize::xy, Buffer::DataType::floatingPoint);
		buffer.linkVbo(shader->colourIn->attributeID, Buffer::colourBuffer, Buffer::ComponentSize::rgba, Buffer::DataType::floatingPoint);
		buffer.render(Buffer::RenderMode::lineStrip, renderStart * numSamples, numSamples);

		zPos -= 0.5f;
		renderStart -= 1;

		if (renderStart < 0)
		{
			renderStart = history - 1;
		}
	}

	startPos++;
}

void Renderer::openGLContextClosing()
{
	buffer.destroy();
}

void Renderer::resized()
{
	auto bounds = getBounds();
	auto heightScale = bounds.getHeight() / static_cast<float>(startHeight);

	modeBox.setBounds(static_cast<int>(bounds.getWidth() * 0.05f),
		static_cast<int>(bounds.getHeight() * 0.88f), 175, 30);

	auto SetBounds = [&heightScale, &bounds](juce::Slider& slider, juce::Label& label, float x, int sliderWidth)
	{
		slider.setBounds(static_cast<int>(x),
			static_cast<int>(bounds.getHeight() * 0.86f),
			sliderWidth,
			static_cast<int>(90 * heightScale));

		label.setBounds(static_cast<int>(x),
			static_cast<int>(bounds.getHeight() * 0.82f),
			100,
			static_cast<int>(50 * heightScale));
	};

	SetBounds(driveSlider, driveLabel, bounds.getWidth() * 0.25f, 100);
	SetBounds(resonanceSlider, resonanceLabel, bounds.getWidth() * 0.375f, 100);
	SetBounds(frequencySlider, frequencyLabel, bounds.getWidth() * 0.5f, 100);
	SetBounds(historySlider, historyLabel, bounds.getWidth() * 0.625f, 100);
	SetBounds(volumeSlider, volumeLabel, bounds.getWidth() * 0.8f, 175);
}