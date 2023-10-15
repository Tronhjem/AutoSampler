#pragma once

#include <JuceHeader.h>
#include "AudioSettings.h"

//==============================================================================

class MainComponent  :  public juce::AudioAppComponent,
                        public Button::Listener,
                        public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void buttonClicked(Button* b) override;
    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    void resized() override;
    
    void sendNote(int noteNumber, int lengthInMs);
    void addMessageToBuffer (const juce::MidiMessage& message);

private:
    enum PlayState
    {
        none = 0,
        recording,
        playing,
        //=======
        LENGTH
    };
    
    std::unique_ptr<AudioSettingsDemo> settings;
    TextButton recordButton   { "Record" };
    TextButton playButton     { "Play"   };
    TextButton midiButton     { "Midi"   };
    TextButton saveAudioButton{ "Save"   };
    
    AudioBuffer<float> recordBuffer {2, 2 * 44100};
    
    int recordBufferPlayHead;
    PlayState playState = PlayState::none;
    int sampleRate = 44100;
    MidiBuffer midiBuffer {};
    File file {"/Users/christiantronhjem/dev/AutoSampler/AutoSampler/demoWaves/testfile.wav"};
    double startTime = 0.0;
    int previousSampleNumber = 0;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
