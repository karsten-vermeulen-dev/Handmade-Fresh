#pragma once
#include <JuceHeader.h>

class VermeulenLadderFilterAudioProcessor : public juce::AudioProcessor
{
public:

	VermeulenLadderFilterAudioProcessor();
	~VermeulenLadderFilterAudioProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	//============================================================================

	int getNumSamples() const;
	const float* getChannelDataLeft() const;
	const float* getChannelDataRight() const;

	void setMode(int id);
	void setDrive(float drive);
	void setVolume(float volume);
	void setResonance(float resonance);
	void setCutoffFrequency(float frequency);

private:

	int numSamples{ 0 };
	float volume{ 0.0 };
	float* channelDataLeft{ nullptr };
	float* channelDataRight{ nullptr };

	juce::dsp::LadderFilter<float> ladderFilter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VermeulenLadderFilterAudioProcessor)
};
