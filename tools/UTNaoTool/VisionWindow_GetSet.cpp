#include <VisionWindow.h>

float VisionWindow::getBodyRoll() {
  if (sensor_block_ == NULL)
    return 0;
  return sensor_block_->values_[angleX];
}

float VisionWindow::getBodyTilt() {
  if (sensor_block_ == NULL)
    return 0;
  return sensor_block_->values_[angleY];
}

float VisionWindow::getHeadTilt() {
  if (joint_block_ == NULL)
    return 0;
  return -joint_block_->values_[HeadTilt];
}

float VisionWindow::getHeadPan() {
  if (joint_block_ == NULL)
    return 0;
  return joint_block_->values_[HeadPan];
}

float VisionWindow::getTrueFrontHeight() const {
  if (body_model_block_ == NULL)
    return 0;
  return body_model_block_->abs_parts_[BodyPart::torso].translation.z;
}

Pose3D VisionWindow::getHeadMatrix() {
  if (body_model_block_ == NULL)
    return Pose3D(0,0,0);
  return body_model_block_->rel_parts_[BodyPart::head];
}

Pose3D VisionWindow::getTorsoMatrix() {
  if (body_model_block_ == NULL)
    return Pose3D(0,0,0);
  return body_model_block_->rel_parts_[BodyPart::torso];
}

vector<LineSegment> VisionWindow::getCalibrationLineSegments(ImageWidget* image) {
  ImageProcessor* processor = getImageProcessor(image);
  vector<LineSegment> segments;
  RobotCalibration cal = ecalibration->getCalibration();
  return ExtrinsicCalibrator::getFieldLinesCameraFrame(cal, world_object_block_, processor->getCameraMatrix());
}

Point2D VisionWindow::getNearestLinePoint(ImageWidget* image, Sample s) {
    vector<LineSegment> segments = getCalibrationLineSegments(image);
    if(segments.size() == 0){
        return s;
    }
    Point2D nearest = segments[0].getPointOnSegmentClosestTo(s);
    float minDistance = (s - nearest).getMagnitude();
    for(unsigned int i=0; i<segments.size(); i++){
        LineSegment segment = segments[i];
        Point2D p = segment.getPointOnSegmentClosestTo(s);
        float distance = (s - p).getMagnitude();
        if(minDistance > distance){
            minDistance = distance;
            nearest = p;
        }
    }
    return nearest;
}
