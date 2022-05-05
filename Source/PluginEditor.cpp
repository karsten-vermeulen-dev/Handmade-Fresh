#include "PluginProcessor.h"
#include "PluginEditor.h"

VermeulenLadderFilterAudioProcessorEditor::VermeulenLadderFilterAudioProcessorEditor(VermeulenLadderFilterAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(renderer.startWidth, renderer.startHeight);
	addAndMakeVisible(renderer);
	setResizable(true, true);
}

VermeulenLadderFilterAudioProcessorEditor::~VermeulenLadderFilterAudioProcessorEditor()
{
}

void VermeulenLadderFilterAudioProcessorEditor::resized()
{
	renderer.setBounds(getLocalBounds());
}