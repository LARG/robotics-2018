#include <localization/ParticleFilter.h>
#include <memory/FrameInfoBlock.h>
#include <memory/OdometryBlock.h>

ParticleFilter::ParticleFilter(MemoryCache& cache, TextLogger*& tlogger) 
  : cache_(cache), tlogger_(tlogger), dirty_(true) {
}

void ParticleFilter::init(Point2D loc, float orientation) {
  mean_.translation = loc;
  mean_.rotation = orientation;
}

void ParticleFilter::processFrame() {
  // Indicate that the cached mean needs to be updated
  dirty_ = true;

  // Retrieve odometry update - how do we integrate this into the filter?
  const auto& disp = cache_.odometry->displacement;
  log(41, "Updating particles from odometry: %2.f,%2.f @ %2.2f", disp.translation.x, disp.translation.y, disp.rotation * RAD_T_DEG);
  
  // Generate random particles for demonstration
  particles().resize(100);
  auto frame = cache_.frame_info->frame_id;
  for(auto& p : particles()) {
    p.x = rand_.sampleN(frame * 5, 250);
    p.y = rand_.sampleN(0, 250);
    p.t = rand_.sampleN(0, M_PI / 4);
    p.w = rand_.sampleU();
  }
}

const Pose2D& ParticleFilter::pose() const {
  if(dirty_) {
    // Compute the mean pose estimate
    mean_ = Pose2D();
    using T = decltype(mean_.translation);
    for(const auto& p : particles()) {
      mean_.translation += T(p.x,p.y);
      mean_.rotation += p.t;
    }
    if(particles().size() > 0)
      mean_ /= static_cast<float>(particles().size());
    dirty_ = false;
  }
  return mean_;
}
