#include <memory/LocalizationBlock.h>

using namespace Eigen;

LocalizationBlock::LocalizationBlock() {
  header.version = 10;
  header.size = sizeof(LocalizationBlock);
  state = decltype(state)::Zero();
  covariance = decltype(covariance)::Identity();
}

Point2D LocalizationBlock::getBallPosition() {
  return Point2D(state[0], state[1]);
}

Point2D LocalizationBlock::getBallVel() {
  return Point2D(/* fill this in */);
}

Matrix2f LocalizationBlock::getBallCov() {
  return covariance.block<2,2>(0,0);
}
/*
void LocalizationBlock::serialize(StreamBuffer& buffer, std::string) {
  std::vector<StreamBuffer> parts(4);
  parts[0].read((unsigned char*)&header, sizeof(header));
  parts[1].read((unsigned char*)&state, sizeof(state));
  parts[2].read((unsigned char*)&covariance, sizeof(covariance));
  parts[3].read(particles);
  StreamBuffer::combine(parts, buffer);
  StreamBuffer::clear(parts);
}

bool LocalizationBlock::deserialize(const StreamBuffer& buffer, std::string) {
  auto parts = buffer.separate();
  if(!validateHeader(parts[0])) {
    StreamBuffer::clear(parts);
    return false;
  }
  parts[0].write(header);
  parts[1].write(state);
  parts[2].write(covariance);
  parts[3].write(particles);
  StreamBuffer::clear(parts);
  return true;
}
*/
