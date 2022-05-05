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

void Renderer::newOpenGLContextCreated()
{
	Buffer::setContext(&context);

	shader = std::make_unique<Shader>(context);

	buffer.create(totalVertices);

	buffer.fillVbo(Buffer::Vbo::vertexBuffer,
		(GLfloat*) nullptr,
		totalVertices * static_cast<int> (Buffer::ComponentSize::xyz) * sizeof(GLfloat));

	buffer.fillVbo(Buffer::Vbo::colourBuffer,
		(GLfloat*) nullptr,
		totalVertices * static_cast<int> (Buffer::ComponentSize::rgba) * sizeof(GLfloat));

	const float farClip = 1000.0f;
	const float nearClip = 0.001f;
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
}

void Renderer::renderOpenGL()
{
	juce::OpenGLHelpers::clear(juce::Colours::darkgrey);

	shader->use();
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

	auto halfSamples = numSamples * 0.5f;
	auto startX = 0.0f - (halfSamples * 0.001f);

	AudioData data;

	for (int i = 0; i < numSamples; i++)
	{
		data.data[i] = std::abs(channelDataLeft[i]);
	}

	timeline.push_front(data);

	while (timeline.size() > history)
	{
		timeline.pop_back();
	}

	auto index = 0;

	//Data for left channel only (for now!)
	for (auto& frame : timeline)
	{
		auto vertexBufferOffset = 0;
		auto colourBufferOffset = 0;

		shader->index->set(static_cast<float> (index));

		for (int i = 0; i < numSamples; i++)
		{
			GLfloat vertex[] = { startX + i * 0.001f,
								 -0.5f + frame.data[i],
								 -2.5f - index * 0.25f };

			GLfloat colour[] = { 1.0f,
								 1.0f - resonance - driveNormalized,
								 1.0f - driveNormalized,
								 1.0f - abs(static_cast<float>(i) / halfSamples - 1) };

			buffer.appendVbo(Buffer::Vbo::vertexBuffer, vertex, sizeof(vertex), vertexBufferOffset);
			buffer.appendVbo(Buffer::Vbo::colourBuffer, colour, sizeof(colour), colourBufferOffset);

			vertexBufferOffset += sizeof(vertex);
			colourBufferOffset += sizeof(colour);
		}

		Buffer::setGLStates();
		Buffer::setLineWidth(static_cast<GLfloat>(1.0f + (3.0f * driveNormalized)));

		buffer.linkVbo(shader->vertexIn->attributeID, Buffer::vertexBuffer, Buffer::ComponentSize::xyz, Buffer::DataType::floatingPoint);
		buffer.linkVbo(shader->colourIn->attributeID, Buffer::colourBuffer, Buffer::ComponentSize::rgba, Buffer::DataType::floatingPoint);
		buffer.render(Buffer::RenderMode::lineStrip, numSamples * verticesPerLine);

		index++;
	}
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