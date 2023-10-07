#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (800, 800);

    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels (2, 2);
        settings = new AudioSettingsDemo(deviceManager);
        
        addAndMakeVisible(settings);
        
//        auto setup = deviceManager.getAudioDeviceSetup();
//        setup.Audio
    }
}

MainComponent::~MainComponent()
{
    delete settings;
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{

    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels  = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    
    auto* inBuffer = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
    auto* outBuffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
 
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        outBuffer[sample] = inBuffer[sample] * 0.6;
    }

//    bufferToFill.clearActiveBufferRegion();
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
