#pragma once

#include <JuceHeader.h>
#include "AudioSettings.h"

//==============================================================================
constexpr int BUFFER_LENGTH_SAMPLES = 3 * 44100;
constexpr int NOTE_LENGTH_SECONDS = 2;
constexpr int NOTE_LENGTH_SAMPLES = NOTE_LENGTH_SECONDS * 44100;
constexpr int NOTE_LENGTH_MS = NOTE_LENGTH_SECONDS * 1000;

class MainComponent  :  public juce::AudioAppComponent,
                        public Button::Listener,
                        public juce::Timer,
                        public ChangeListener
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
    void changeListenerCallback (ChangeBroadcaster* source) override;
    
    void sendNote(int noteNumber, int lengthInMs);
    void addMessageToBuffer (const juce::MidiMessage& message);
    void saveBufferToFile();

private:
    enum PlayState
    {
        none = 0,
        recording,
        playing,
        //=======
        LENGTH
    };
    
    AudioBuffer<float> recordBuffer {2, BUFFER_LENGTH_SAMPLES};
    File file {"/Users/christiantronhjem/dev/AutoSampler/AutoSampler/demoWaves/testfile.wav"};
    MidiBuffer midiBuffer {};
    TextButton recordButton   { "Record" };
    TextButton playButton     { "Play"   };
    TextButton midiButton     { "Midi"   };
    TextButton saveAudioButton{ "Save"   };
    ReadWriteLock midiBufferLock;
    std::unique_ptr<MidiOutput> midiOut = nullptr;
    
    double startTime = 0.0;
    std::unique_ptr<AudioSettingsDemo> settings;
    int recordBufferPlayHead;
    PlayState playState = PlayState::none;
    int sampleRate = 44100;
    int previousSampleNumber = 0;
    bool firstTime = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
