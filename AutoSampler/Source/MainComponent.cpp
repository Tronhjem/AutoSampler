#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (1000, 800);

    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels (2, 2);
        
        // Settings
        settings = new AudioSettingsDemo(deviceManager);
        addAndMakeVisible(settings);
       
        // record button setup
        recordButton.setBounds(400, 100, 150, 50);
        recordButton.addListener(this);
        addAndMakeVisible(&recordButton);
        
        // play button setup
        playButton.setBounds(400, 250, 150, 50);
        playButton.addListener(this);
        addAndMakeVisible(&playButton);
    }
}

MainComponent::~MainComponent()
{
    delete settings;
    shutdownAudio();
}


void MainComponent::buttonClicked(Button* b)
{
    if (b == &recordButton)
    {
        if (playState != PlayState::recording)
        {
            playButton.setButtonText("Play");
            recordButton.setButtonText("Recording...");
            playState = PlayState::recording;
            recordBufferPlayHead = 0;
        }
        else if (playState == PlayState::recording)
        {
            recordButton.setButtonText("Record");
            playState = PlayState::none;
            recordBufferPlayHead = 0;
        }
    }
    
    else if(b == &playButton)
    {
        if (playState != PlayState::playing)
        {
            recordButton.setButtonText("Record");
            playButton.setButtonText("Playing...");
            playState = PlayState::playing;
            recordBufferPlayHead = 0;
        }
        else if (playState == PlayState::playing)
        {
            playButton.setButtonText("Play");
            playState = PlayState::none;
            recordBufferPlayHead = 0;
        }
    }
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    const float gain = 0.5f;
    
    
    switch (playState)
    {
        case PlayState::recording:
            { // SCOPE START
                
            auto* inBuffer = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
            auto* recordBufferR = recordBuffer.getWritePointer(0);
            auto* recordBufferL = recordBuffer.getWritePointer(1);
                
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                if (recordBufferPlayHead >= recordBuffer.getNumSamples())
                {
                    const MessageManagerLock mmLock;
                    playState = PlayState::none;
                    recordBufferPlayHead = 0;
                    
                    recordButton.setButtonText("Record");
                    playButton.setButtonText("Play");
                    break;
                }

                recordBufferR[recordBufferPlayHead] = inBuffer[sample] * gain;
                recordBufferL[recordBufferPlayHead] = inBuffer[sample] * gain;

                ++recordBufferPlayHead;
            }
                
            }// SCOPE END
            break;
            
        case PlayState::playing:
            { // SCOPE START
            auto* recordBufferR = recordBuffer.getReadPointer(0);
            auto* recordBufferL = recordBuffer.getReadPointer(1);
                
            auto* outBufferL = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
            auto* outBufferR = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
                
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                if (recordBufferPlayHead >= recordBuffer.getNumSamples())
                {
                    recordBufferPlayHead = 0;
                }

                outBufferL[sample] = recordBufferR[recordBufferPlayHead];
                outBufferR[sample] = recordBufferL[recordBufferPlayHead];

                ++recordBufferPlayHead;
            }
                
            }// SCOPE END
            break;
            
        case PlayState::none:
            break;
            
        default:
            break;
    }
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    if (settings != nullptr)
        settings->resized();
}
