#include <memory/AudioProcessingBlock.h>
#include <common/SchemaExtensions.h>

constexpr int AudioProcessingBlock::SampleRate;
constexpr int AudioProcessingBlock::NumChannels;
constexpr float AudioProcessingBlock::BufferTime;
constexpr int AudioProcessingBlock::SamplesPerChannelPerBuffer;
constexpr int AudioProcessingBlock::BufferSampleCount;

constexpr float AudioProcessingBlock::MsPerSample;
constexpr float AudioProcessingBlock::SamplesPerMs;

/*
// Custom deserialize for converting old logs
void AudioProcessingBlock::_deserialize(const schema::AudioProcessingBlock* data) {
  this->whistle_heard_frame_ = data->whistle_heard_frame_();
  this->teammate_heard_frame_ = data->teammate_heard_frame_();
  schema::std::deserialize(data->buffer_(), this->buffer_);
  this->timestamp_ = data->timestamp_();
  this->whistle_score_ = 0.0f;
  this->whistle_sd_ = 0.0f;
}
*/
