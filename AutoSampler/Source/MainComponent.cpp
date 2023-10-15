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
        settings = std::make_unique<AudioSettingsDemo>(deviceManager);
        addAndMakeVisible(*settings);
       
        // record button setup
        recordButton.setBounds(400, 100, 150, 50);
        recordButton.addListener(this);
        addAndMakeVisible(&recordButton);
        
        // play button setup
        playButton.setBounds(400, 150, 150, 50);
        playButton.addListener(this);
        addAndMakeVisible(&playButton);
        
        // midi button setup
        midiButton.setBounds(400, 200, 150, 50);
        midiButton.addListener(this);
        addAndMakeVisible(&midiButton);
        
        // save audio button
        saveAudioButton.setBounds(400, 250, 150, 50);
        saveAudioButton.addListener(this);
        addAndMakeVisible(&saveAudioButton);
    }
    startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimer (1);
}

MainComponent::~MainComponent()
{
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
    
    else if (b == &midiButton)
    {
        sendNote(64, 2000);
    }
    
    else if (b == &saveAudioButton)
    {
        file.deleteFile();

        if (auto fileStream = std::unique_ptr<FileOutputStream> (file.createOutputStream()))
        {
            std::unique_ptr<AudioFormatWriter> writer;
            WavAudioFormat wavFormat;
            writer.reset (wavFormat.createWriterFor (fileStream.get(),
                                                     (double)sampleRate,
                                                     recordBuffer.getNumChannels(),
                                                     16,
                                                     {},
                                                     0));
            
            if (writer != nullptr)
                writer->writeFromAudioSampleBuffer (recordBuffer, 0, recordBuffer.getNumSamples());
            
            writer.reset(); // to make sure we are closing the writer.
            fileStream.release(); // properly releae the filestream for delete.
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


void MainComponent::sendNote (int noteNumber, int lengthInMs)
{
    auto message = juce::MidiMessage::noteOn (1, noteNumber, (juce::uint8) 100);
    message.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);
    addMessageToBuffer (message);
   
    auto messageOff = juce::MidiMessage::noteOff (message.getChannel(), message.getNoteNumber());
    messageOff.setTimeStamp (message.getTimeStamp() + (lengthInMs * 0.001));
    addMessageToBuffer (messageOff);
}

void MainComponent::addMessageToBuffer (const juce::MidiMessage& message)
{
    auto timestamp = message.getTimeStamp();
    auto sampleNumber =  (int) (timestamp * sampleRate);
    midiBuffer.addEvent (message, sampleNumber);
}

void MainComponent::timerCallback()
{
    auto defaultOutIdent = deviceManager.getDefaultMidiOutputIdentifier();
    auto midiout = MidiOutput::openDevice(defaultOutIdent);
   
    if (midiout == nullptr)
        return;
    
    auto currentTime = juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime;
    auto currentSampleNumber = (int) (currentTime * sampleRate);     // [4]

    for (const auto metadata : midiBuffer)                           // [5]
    {
        if (metadata.samplePosition > currentSampleNumber)           // [6]
            break;

        auto message = metadata.getMessage();
        message.setTimeStamp (metadata.samplePosition / sampleRate); // [7]
//         addMessageToList (message);
        midiout->sendMessageNow(message);
    }

    midiBuffer.clear (previousSampleNumber, currentSampleNumber - previousSampleNumber); // [8]
    previousSampleNumber = currentSampleNumber;                                          // [9]
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
