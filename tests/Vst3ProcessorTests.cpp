#include "Portable/PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <cstdlib>
#include <iostream>

namespace {
struct TestPlayHead final : juce::AudioPlayHead {
    juce::int64 sample = 0;
    bool playing = true;
    juce::Optional<PositionInfo> getPosition() const override {
        PositionInfo position; position.setTimeInSamples(sample); position.setIsPlaying(playing); return position;
    }
};
void require(bool value, const char* message) {
    if (!value) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
int countCc64(const juce::MidiBuffer& midi, int value) {
    int count = 0;
    for (const auto item : midi) {
        const auto message = item.getMessage();
        if (message.isController() && message.getControllerNumber() == 64 && message.getControllerValue() == value) ++count;
    }
    return count;
}
juce::AudioProcessorParameter* parameter(juce::AudioProcessor& processor, const char* wanted) {
    for (auto* value : processor.getParameters())
        if (auto* identified = dynamic_cast<juce::AudioProcessorParameterWithID*>(value); identified && identified->paramID == wanted)
            return value;
    return nullptr;
}
}

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    PianoAutoSustainProcessor processor(true);
    processor.prepareToPlay(1000.0, 256);
    juce::AudioBuffer<float> audio(2, 256); audio.clear(); audio.addFrom(0, 0, juce::AudioBuffer<float>(2, 256), 0, 0, 0);
    for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample) audio.setSample(channel, sample, 0.5f);
    juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::noteOn(1, 48, static_cast<juce::uint8>(100)), 0);
    processor.processBlock(audio, midi);
    require(audio.getMagnitude(0, 256) == 0.0f, "processor audio output stays silent");
    require(countCc64(midi, 127) == 1 && midi.getNumEvents() == 2, "default CC64 mode passes note and presses pedal");
    {
        PianoAutoSustainProcessor transport(true); TestPlayHead playHead; transport.setPlayHead(&playHead); transport.prepareToPlay(1000.0, 256);
        juce::MidiBuffer events; events.addEvent(juce::MidiMessage::noteOn(1, 48, static_cast<juce::uint8>(100)), 0);
        transport.processBlock(audio, events); playHead.sample = 1024; events.clear(); transport.processBlock(audio, events);
        require(countCc64(events, 0) == 1, "transport discontinuity releases sustain");
    }

    auto* mode = parameter(processor, "mode"); require(mode != nullptr, "output mode parameter exists");
    mode->setValueNotifyingHost(0.0f);
    processor.prepareToPlay(1000.0, 256);
    midi.clear(); midi.addEvent(juce::MidiMessage::noteOn(1, 50, static_cast<juce::uint8>(100)), 0);
    processor.processBlock(audio, midi);
    require(countCc64(midi, 127) == 0 && midi.getNumEvents() == 1, "Note Hold mode does not emit CC64");
    midi.clear(); midi.addEvent(juce::MidiMessage::noteOff(1, 50), 0); processor.processBlock(audio, midi);
    require(midi.isEmpty(), "Note Hold mode defers note off");
    processor.processBlockBypassed(audio, midi);
    require(!midi.isEmpty(), "bypass releases deferred notes");

    mode->setValueNotifyingHost(1.0f);
    juce::MemoryBlock state; processor.getStateInformation(state);
    auto xml = juce::AudioProcessor::getXmlFromBinary(state.getData(), static_cast<int>(state.getSize()));
    require(xml && xml->getIntAttribute("schema") == 2, "state writes schema 2");
    require(xml->getChildByName("Instrument") == nullptr, "2.0 state never embeds an instrument host");
    xml->removeAttribute("schema");
    juce::AudioProcessor::copyXmlToBinary(*xml, state);
    PianoAutoSustainProcessor migrated(true); migrated.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(parameter(migrated, "mode")->getValue() == 1.0f, "1.0 state migrates output mode");
    require(migrated.getName() == "Piano Auto Sustain", "public product name is unified");
    {
        PianoAutoSustainProcessor midiOnly(true);
        midiOnly.prepareToPlay(48000,256);
        juce::AudioBuffer<float> noAudio(0,256);
        juce::MidiBuffer events;
        events.addEvent(juce::MidiMessage::noteOn(1,48,static_cast<juce::uint8>(100)),0);
        midiOnly.processBlock(noAudio,events);
        require(countCc64(events,127)==1,"MIDI-only AU-style block generates pedal without audio buses");
    }
    {
        PianoAutoSustainProcessor uiProcessor(true);
        std::unique_ptr<juce::AudioProcessorEditor> editor(uiProcessor.createEditor());
        int sliderCount=0,toggleCount=0;
        for(auto* child:editor->getChildren()) {
            if(auto* slider=dynamic_cast<juce::Slider*>(child)) {
                if(sliderCount++==0){slider->setValue(62,juce::sendNotificationSync);require(uiProcessor.parameters.getRawParameterValue("split")->load()==62,"editor slider updates processor");}
            }
            if(dynamic_cast<juce::ToggleButton*>(child))++toggleCount;
        }
        require(sliderCount==5&&toggleCount==3,"editor exposes processing controls");
        require(editor->getWidth()==760&&editor->getHeight()==500,"licensed editor matches Cubase layout size");
        bool hasPlay=false,hasGuide=false,hasRelease=false;
        for(auto* child:editor->getChildren())if(auto* b=dynamic_cast<juce::TextButton*>(child)){
            hasPlay|=b->getButtonText()==juce::String::fromUTF8(u8"演奏");
            hasGuide|=b->getButtonText()==juce::String::fromUTF8(u8"说明书");
            hasRelease|=b->getButtonText()==juce::String::fromUTF8(u8"释放踏板");
        }
        require(hasPlay&&hasGuide&&hasRelease,"licensed editor has unified tabs, guide and pedal actions");
        auto image=editor->createComponentSnapshot(editor->getLocalBounds());
        require(image.isValid(),"VST3 editor renders");
        juce::FileOutputStream stream(juce::File::getCurrentWorkingDirectory().getChildFile("vst3-editor.png"));
        stream.setPosition(0);stream.truncate();require(juce::PNGImageFormat{}.writeImageToStream(image,stream),"VST3 preview saved");
    }
    {
        PianoAutoSustainProcessor unlicensed(false);
        unlicensed.prepareToPlay(1000.0, 256);
        juce::MidiBuffer events;
        events.addEvent(juce::MidiMessage::noteOn(1,48,static_cast<juce::uint8>(100)),0);
        unlicensed.processBlock(audio,events);
        require(events.getNumEvents()==1 && countCc64(events,127)==0,
                "unlicensed VST3 passes MIDI through without generated sustain");
        require(!unlicensed.isAuthorized(),"unlicensed state is visible to the editor");
    }
    if (argc > 1) {
        juce::VST3PluginFormat format; juce::OwnedArray<juce::PluginDescription> types;
        format.findAllTypesForFile(types, juce::String::fromUTF8(argv[1]));
        require(types.size() == 1, "built VST3 scans as exactly one plugin");
        require(types[0]->name == "Piano Auto Sustain" && types[0]->version == "1.0.0",
                "VST3 metadata reports unified product name and version");
    }
    std::cout << "PASS: VST3 MIDI, silent audio, bypass cleanup, schema 2, legacy migration\n";
}
