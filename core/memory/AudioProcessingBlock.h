#pragma once

#include <memory/MemoryBlock.h>
#include <common/Enum.h>
#include <schema/gen/AudioProcessingBlock_generated.h>
#include <math/Common.h>

DECLARE_INTERNAL_SCHEMA(struct AudioProcessingBlock : public MemoryBlock {
  public:
#ifndef SWIG
    static constexpr int SampleRate = 48000; // Hz
    static constexpr int NumChannels = 4;
    static constexpr float BufferTime = 85.0f; // ms
    static constexpr int SamplesPerChannelPerBuffer = static_math::round2(SampleRate * BufferTime / 1000.0f);
    static constexpr int BufferSampleCount = NumChannels * SamplesPerChannelPerBuffer;
    
    static constexpr float MsPerSample = BufferTime / BufferSampleCount;
    static constexpr float SamplesPerMs = BufferSampleCount / BufferTime;
#endif
    
    SCHEMA_METHODS(AudioProcessingBlock);
    ENUM(AudioState,
      Detecting,
      TrainingNegative,
      TrainingPositive,
      Off
    );
    AudioProcessingBlock() {
      header.version = 2;
      header.size = sizeof(AudioProcessingBlock);
      whistle_heard_frame_ = -10000;
      teammate_heard_frame_ = -10000;
      state_ = AudioState::Off;
    }

	
    SCHEMA_FIELD(int whistle_heard_frame_);
    SCHEMA_FIELD(int teammate_heard_frame_);
#ifndef SWIG
    SCHEMA_FIELD(std::array<int16_t,BufferSampleCount> buffer_);
#endif
    SCHEMA_FIELD(AudioState state_);
    SCHEMA_FIELD(uint64_t timestamp_);
    SCHEMA_FIELD(float whistle_score_);
    SCHEMA_FIELD(float whistle_sd_);
    
    /*
    // Custom deserialize for converting old logs
    SCHEMA_DESERIALIZATION({ _deserialize(data); });
    void _deserialize(const schema::AudioProcessingBlock* data);
    */
});

inline std::ostream& operator<<(std::ostream& os, AudioProcessingBlock::AudioState s) {
  return os << AudioProcessingBlock::getName(s);
}

