#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <vector>
#include "Buffer.h"
#include "Shader.h"
#include <JuceHeader.h>
#include "PluginProcessor.h"

const int maxSampleSize{ 2048 };

class Renderer : public juce::Component, public juce::OpenGLRenderer
{

public:

	const int startWidth = 1280;
	const int startHeight = 720;

	Renderer(VermeulenLadderFilterAudioProcessor& audioProcessor);
	~Renderer();

	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;
	void resized() override;

private:

	const int maxChannels{ 2 };
	const int maxHistory{ 1000 };
	const int verticesPerLine{ 2 };
	const int maxVertices{ maxHistory * maxSampleSize * maxChannels };

	VermeulenLadderFilterAudioProcessor& audioProcessor;

	int history{ 50 };
	float drive{ 1.0 };
	float volume{ 0.5 };
	float resonance{ 0.0 };
	float frequency{ 44100.0 };

	struct AudioData
	{
		//float data[maxSampleSize];
		std::vector<float> data;
	};

	std::deque<AudioData> timeline;

	Buffer buffer;
	juce::OpenGLContext context;
	std::unique_ptr<Shader> shader;
	juce::Matrix3D<GLfloat> projectionMatrix;

	juce::LookAndFeel_V4 lookAndFeelModeBox;
	juce::ComboBox modeBox{ "ModeBox" };

	juce::LookAndFeel_V4 lookAndFeelDriveSlider;
	juce::Label driveLabel{ "DriveLabel", "Drive" };
	juce::Slider driveSlider{ juce::Slider::SliderStyle::RotaryHorizontalDrag,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow };

	juce::LookAndFeel_V4 lookAndFeelResonanceSlider;
	juce::Label resonanceLabel{ "ResonanceLabel", "Resonance" };
	juce::Slider resonanceSlider{ juce::Slider::SliderStyle::RotaryHorizontalDrag,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow };

	juce::LookAndFeel_V4 lookAndFeelFrequencySlider;
	juce::Label frequencyLabel{ "FrequencyLabel", "Frequency" };
	juce::Slider frequencySlider{ juce::Slider::SliderStyle::RotaryHorizontalDrag,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow };

	juce::LookAndFeel_V4 lookAndFeelHistorySlider;
	juce::Label historyLabel{ "HistoryLabel", "History" };
	juce::Slider historySlider{ juce::Slider::SliderStyle::RotaryHorizontalDrag,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow };

	juce::LookAndFeel_V4 lookAndFeelVolumeSlider;
	juce::Label volumeLabel{ "VolumeLabel", "Volume" };
	juce::Slider volumeSlider{ juce::Slider::SliderStyle::LinearHorizontal,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow };
};