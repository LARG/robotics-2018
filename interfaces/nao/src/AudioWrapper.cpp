#include "AudioWrapper.h"
#include "naointerface.h"
#include <memory/AudioProcessingBlock.h>
#include <alcommon/alproxy.h>

AudioWrapper::AudioWrapper(boost::shared_ptr<AL::ALBroker> broker, const std::string & name) :
  AL::ALSoundExtractor(broker, name), memory_(naointerface::MEMORY_INSTANCE) {
}

AudioWrapper::~AudioWrapper() {
  stopDetection();
}

void AudioWrapper::init() {
  if (AudioProcessingBlock::SampleRate == 48000){
    //using all 4 channels at 48000 Hz, default mode
    audioDevice->callVoid("setClientPreferences",
        getName(),                //Name of this module
        AudioProcessingBlock::SampleRate,  //48000 Hz requested
        (int)AL::ALLCHANNELS,        //All Channels requested
        1                         //Deinterleaving is needed here
    );
  }
  else {
    ////using single channel at 16000 Hz
    audioDevice->callVoid("setClientPreferences",
        getName(),                //Name of this module
        AudioProcessingBlock::SampleRate,  //16000 Hz requested
        (int)AL::FRONTCHANNEL,        //Front Channels requested
        0                        //Deinterleaving is not needed here
        );
  }
	if(memory_)
		memory_->getOrAddBlockByName(audio_block_,"audio_processing", MemoryOwner::VISION);
  startDetection();
}

void AudioWrapper::process(const int & nbOfChannels, const int & nbrOfSamplesByChannel, const AL_SOUND_FORMAT * buffer, const AL::ALValue & timestamp) {
  int ts1 = timestamp[0], ts2 = timestamp[1];
  unsigned long long ts = ts1;
  ts *= 1000000ul;
  ts += ts2;
  audio_block_->timestamp_ = ts;
  memcpy(audio_block_->buffer_.data(), buffer, sizeof(AL_SOUND_FORMAT) * AudioProcessingBlock::BufferSampleCount);
}
