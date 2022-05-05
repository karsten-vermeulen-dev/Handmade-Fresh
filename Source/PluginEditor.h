#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Renderer.h"

class VermeulenLadderFilterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:

	VermeulenLadderFilterAudioProcessorEditor(VermeulenLadderFilterAudioProcessor&);
	~VermeulenLadderFilterAudioProcessorEditor() override;
	void resized() override;

private:

	VermeulenLadderFilterAudioProcessor& audioProcessor;

	Renderer renderer{ audioProcessor };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VermeulenLadderFilterAudioProcessorEditor)
};