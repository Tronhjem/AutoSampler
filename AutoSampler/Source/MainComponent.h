#pragma once

#include <JuceHeader.h>
#include "AudioSettings.h"

class MainComponent  : public juce::AudioAppComponent,
                       public Button::Listener
{
public:
    
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void buttonClicked(Button* b) override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    enum PlayState
    {
        none = 0,
        recording,
        playing,
        //=======
        LENGTH
    };
    
    AudioSettingsDemo* settings = nullptr;
    TextButton recordButton  { "Record" };
    TextButton playButton { "Play"   };
    
    AudioBuffer<float> recordBuffer {2, 2 * 44100};
    int recordBufferPlayHead;
    PlayState playState = PlayState::none;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
