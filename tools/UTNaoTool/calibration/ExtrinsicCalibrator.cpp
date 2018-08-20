#include "ExtrinsicCalibrator.h"
#define DEBUG_CALIBRATION 1

using namespace Eigen;

float ExtrinsicCalibrator::computeErrorParameterVector(const Sample& sample, const VectorXf& parameters) {
  RobotCalibration cal;
  //TODO: load calibration settings like selected joints, selected camera, etc
  cal.fromExtrinsicVector(parameters);
  float error = getSampleErrorCameraFrame(sample, cal);
  //std::cout << "Error calculated as " << error << "\n";
  return error;
}

float ExtrinsicCalibrator::getSampleErrorGlobalFrame(const Sample& sample, const RobotCalibration& cal) const{
    Pose3D position(cal.poseX, cal.poseY, 0);
    position.rotateZ(cal.poseTheta);
    float error = 3000;
    vector<LineSegment> segments = getFieldLinesGlobalFrame();
    const CameraMatrix& cmatrix(processor_->getCameraMatrix());
    for(vector<LineSegment>::iterator segI = segments.begin(); segI != segments.end(); segI++){
        LineSegment seg = *segI;
        Position p = cmatrix.getWorldPosition(sample.x, sample.y);
        Pose3D globS = Pose3D(p.x, p.y, 0).relativeToGlobal(position);
        Point2D globPtS = Point2D(globS.translation.x, globS.translation.y);
        float distance = seg.getDistanceToPoint(globPtS);
        if(distance < error)
            error = distance;
    }
    return error * error;
}

float ExtrinsicCalibrator::getSampleErrorCameraFrame(const vector<Sample>& samples, const RobotCalibration& cal) const {
  float error = 0;
  for(int i = 0; i < (int)samples.size(); i++)
    error += getSampleErrorCameraFrame(samples[i], cal);
  return error;
}

float ExtrinsicCalibrator::getSampleErrorCameraFrame(const Sample& sample, const RobotCalibration& cal) const {
    float error = 1000;
    vector<LineSegment> segments = getFieldLinesCameraFrame(cal, processor_->getCameraMatrix());
    LineSegment s;
    for(vector<LineSegment>::iterator segI = segments.begin(); segI != segments.end(); segI++){
        LineSegment seg = *segI;
        Line2D line(seg.start, seg.end);
        float distance = seg.getDistanceToPoint(sample);
        if(distance < error) {
            error = distance;
            s = seg;
        }
    }
    return error * error;
}

vector<LineSegment> ExtrinsicCalibrator::getFieldLinesGlobalFrame() const {
    vector<LineSegment> segments;
    for (int i = LINE_OFFSET; i < LINE_OFFSET + NUM_LINES; i++) {
        WorldObject &wo = world_object_block_->objects_[i];
        LineSegment segment;
        segment.start = wo.loc;
        segment.end = wo.endLoc;
        segments.push_back(segment);
    }
    return segments;
}

vector<LineSegment> ExtrinsicCalibrator::getFieldLinesCameraFrame(const RobotCalibration& cal, const CameraMatrix& cmatrix) const {
  return getFieldLinesCameraFrame(cal, world_object_block_, cmatrix);
}

vector<LineSegment> ExtrinsicCalibrator::getFieldLinesCameraFrame(const RobotCalibration& cal, WorldObjectBlock* world_object_block, const CameraMatrix& cmatrix) {
  Pose3D position(cal.poseX, cal.poseY, 0);
  position.rotateZ(cal.poseTheta);
  vector<LineSegment> segments;
  for (int i = LINE_OFFSET; i < LINE_OFFSET + NUM_LINES; i++) {
    WorldObject &wo = world_object_block->objects_[i];

    Pose3D pt1 = Pose3D(wo.loc.x, wo.loc.y, 0).globalToRelative(position);
    Pose3D pt2 = Pose3D(wo.endLoc.x, wo.endLoc.y, 0).globalToRelative(position);
    float distance = (pt1.translation - pt2.translation).abs();

    // Split each line into a number of segments - helps prevent distortion
    int numSegments = ceil(distance / 500);
    if (ceil(distance/0.5) == floor(distance/500))
      numSegments++;

    for (int i = 0; i < numSegments; i++) {
      float r1 = (500 * i) / distance;
      float r2 = (500 * (i+1)) / distance;
      if (i == numSegments - 1)
        r2 = 1;
      Vector3<float> p1 = pt1.translation * (1-r1) + pt2.translation * r1;
      Vector3<float> p2 = pt1.translation * (1-r2) + pt2.translation * r2;
      Coordinates
        p1i = cmatrix.getImageCoordinates(p1.x, p1.y, p1.z),
        p2i = cmatrix.getImageCoordinates(p2.x, p2.y, p2.z);
      if(p1i.x < 0 && p2i.x < 0) {
        continue;
      }
      LineSegment segment(p1i.x, p1i.y, p2i.x, p2i.y);
      segments.push_back(segment);
    }
  }
  for (int i = 0; i < 24; i++) {
    Pose3D pt1 = Pose3D(CIRCLE_RADIUS * sinf(i * M_PI / 12.0), CIRCLE_RADIUS * cosf(i * M_PI / 12.0), 0).globalToRelative(position);
    Pose3D pt2 = Pose3D(CIRCLE_RADIUS * sinf((i + 1) * M_PI / 12.0), CIRCLE_RADIUS * cosf((i + 1) * M_PI / 12.0), 0).globalToRelative(position);
    Vector3<float> p1 = pt1.translation, p2 = pt2.translation;
      Coordinates 
        p1i = cmatrix.getImageCoordinates(p1.x, p1.y, p1.z),
        p2i = cmatrix.getImageCoordinates(p2.x, p2.y, p2.z);
    if(p1i.x < 0 && p2i.x < 0) {
        continue;
    }

    LineSegment segment(p1i.x, p1i.y, p2i.x, p2i.y);
    const ImageParams& iparams = cmatrix.getImageParams();
    if(segment.start.x < 0 && segment.end.x > iparams.width)
        continue;
    if(segment.start.x > iparams.width && segment.end.x < 0)
        continue;
    segments.push_back(segment);
  }
  return segments;
}

void ExtrinsicCalibrator::calibrate(RobotCalibration& cal, vector<Sample> samples, int iterations) {
  // New optimizer and this calibrator doesn't work anyway - JM 03/05/15
  //VectorXf parameters = cal.extrinsicVector();
	//GaussNewtonOptimizer<Sample, ExtrinsicCalibrator> optimizer(parameters, samples, *this, &ExtrinsicCalibrator::computeErrorParameterVector);
  //for(int i=0; i < iterations; i++) {
    //float epsilon = optimizer.iterate();
    //if(epsilon == 0.0f) {
      //std::cout << "Optimizer diverged.\n";
      //break;
    //}
  //}
  //parameters = optimizer.getParameters();
  //cal.fromExtrinsicVector(parameters);
}
