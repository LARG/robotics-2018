#include <vision/CameraMatrix.h>

using namespace Eigen;

CameraMatrix::CameraMatrix(const ImageParams& iparams, const Camera::Type& camera)
  : iparams_(iparams), camera_(camera) {
  RobotCalibration cal;
  setCalibration(cal);
}

Position CameraMatrix::getWorldPositionByDirectDistance(int imageX, int imageY, float distance) const {
  Coordinates c = undistort(imageX, imageY);
  Vector4f im;
  im << c.x, c.y, 1, 1;
  Vector4f target = camToWorld_ * im;
  Vector3f ray = (target.segment<3>(0) - cameraPosition_);
  float t = distance / ray.norm();
  Vector3f w = ray * t + cameraPosition_;
  return Position(w[0],w[1],w[2]);
}

Position CameraMatrix::getWorldPositionByDirectDistance(Coordinates c, float distance) const {
  return getWorldPositionByDirectDistance(c.x, c.y, distance);
}

Position CameraMatrix::getWorldPositionByGroundDistance(int imageX, int imageY, float distance) const {
  Coordinates c = undistort(imageX, imageY);
  Vector4f im;
  im << c.x, c.y, 1, 1;
  Vector4f target = camToWorld_ * im;
  Vector3f ray = (target.segment<3>(0) - cameraPosition_);
  float t = distance / ray.segment<2>(0).norm();
  Vector3f w = ray * t + cameraPosition_;
  return Position(w[0],w[1],w[2]);
}

Position CameraMatrix::getWorldPositionByGroundDistance(Coordinates c, float distance) const {
  return getWorldPositionByDirectDistance(c.x, c.y, distance);
}

Position CameraMatrix::getWorldPosition(int imageX, int imageY, float height) const {
  Coordinates c = undistort(imageX, imageY);
  Vector4f im;
  im << c.x, c.y, 1, 1;
  Vector4f target = camToWorld_ * im;
  Vector3f ray = (target.segment<3>(0) - cameraPosition_);
  float t = (height - cameraPosition_[2]) / ray[2];
  Vector3f w = ray * t + cameraPosition_;
  return Position(w[0],w[1],w[2]);
}

Position CameraMatrix::getWorldPosition(Coordinates c, float height) const {
  return getWorldPosition(c.x, c.y, height);
}

Coordinates CameraMatrix::getImageCoordinates(float x, float y, float z) const {
  Vector4f v;
  v << x, y, z, 1;
  Vector4f c = worldToCam_ * v;
  c = c / c[2];
  return Coordinates(c[0], c[1]);
}

Coordinates CameraMatrix::undistort(float x, float y) const {
  //Disabled until this can be fully debugged - JM 05/25/13
  return Coordinates(x,y);
  float ifx = 1.0f / fx_, ify = 1.0f / fy_;
  const float 
    &k1 = cal_.k1,
    &k2 = cal_.k2,
    &k3 = cal_.k3,
    &p1 = cal_.p1,
    &p2 = cal_.p2;

  float x0 = x = (x - cx_)*ifx;
  float y0 = y = (y - cy_)*ify;
  int iters = 1;


  // compensate distortion iteratively
  for(int j = 0; j < iters; j++ )
  {
    float r2 = x*x + y*y;
    float icdist = 1.0f / (1 + ((k3*r2 + k2)*r2 + k1)*r2);
    float deltaX = 2*p1*x*y + p2*(r2 + 2*x*x);
    float deltaY = p1*(r2 + 2*y*y) + 2*p2*x*y;
    x = (x0 - deltaX)*icdist;
    y = (y0 - deltaY)*icdist;
  }

  x = x * fx_ + cx_;
  y = y * fy_ + cy_;
  int ix = floor(x + 0.5f), iy = floor(y + 0.5f);
  return Coordinates(ix, iy);
}

Coordinates CameraMatrix::getImageCoordinates(Vector3f worldPosition) const {
  return getImageCoordinates(worldPosition[0], worldPosition[1], worldPosition[2]);
}
  
void CameraMatrix::setCalibration(const RobotCalibration& cal) {
  cal_ = cal;
  if(camera_ == Camera::TOP) {
    fx_ = iparams_.width / 2 / tan((cal.topFOVx + FOVx) / 2);
    fy_ = iparams_.height / 2 / tan((cal.topFOVy + FOVy) / 2);
  }
  else {
    fx_ = iparams_.width / 2 / tan((cal.bottomFOVx + FOVx) / 2);
    fy_ = iparams_.height / 2 / tan((cal.bottomFOVy + FOVy) / 2);
  }
  scale_ = 0;
  cx_ = iparams_.width / 2;
  cy_ = iparams_.height / 2;
  cameraCalibration_ <<
    fx_, scale_,  cx_,
      0,    fy_,  cy_,
      0,      0,    1;
  coordinateShift_ <<
    0, -1,  0,
    0,  0, -1,
    1,  0,  0;
}

void CameraMatrix::updateCameraPose(Pose3D& pose) {
  for(int i = 0; i < 3; i++) for (int j = 0; j < 3; j++)
    cameraRotation_(i, j) = pose.rotation[j][i]; // Matrix3x3 uses backward indexing
  
  for(int i = 0; i < 3; i++)
    cameraPosition_[i] = pose.translation[i];

  Matrix3f R = coordinateShift_ * cameraRotation_.inverse();
  Vector3f t = R * -cameraPosition_;

  Matrix4f K0 = Matrix4f::Zero(), Rt = Matrix4f::Zero();

  Rt.block<3,3>(0, 0) = R;
  Rt.block<3,1>(0, 3) = t;
  Rt(3, 3) = 1;

  K0.block<3,3>(0, 0) = cameraCalibration_;
  K0(3, 3) = 1;

  worldToCam_ = K0 * Rt;
  camToWorld_ = worldToCam_.inverse();
  
  //std::cout << "Pos: \n" << cameraPosition_ << "\n";
  //std::cout << "Rot: \n" << cameraRotation_ << "\n";
  //std::cout << "Inv Rot: \n" << cameraRotation_.inverse() << "\n";
  //std::cout << "Shift: \n" << coordinateShift_ << "\n";
  //std::cout << "R: \n" << R << "\n";
  //std::cout << "t: \n" << t << "\n";
  //std::cout << "pose:\n " << pose << "\n";
  //std::cout << "k0:\n " << K0 << "\n";
  //std::cout << "rt:\n " << Rt << "\n";
  //std::cout << "P:\n " << worldToCam_ << "\n";
}

float CameraMatrix::groundDistance(const Position& p) const {
  return sqrtf(p.x * p.x + p.y * p.y);
}

float CameraMatrix::directDistance(const Position& p) const {
  Vector3f rel = Vector3f(
    p.x - cameraPosition_[0],
    p.y - cameraPosition_[1],
    p.z - cameraPosition_[2]
  );
  return rel.norm();
}


float CameraMatrix::elevation(const Position& p) const {
  Vector3f rel = Vector3f(
    p.x - cameraPosition_[0],
    p.y - cameraPosition_[1],
    p.z - cameraPosition_[2]
  );
  return acosf(rel[2] / rel.norm()) - M_PI / 2;
}

float CameraMatrix::bearing(const Position& p) const {
  return atan2(p.y, p.x);
}

float CameraMatrix::getWorldDistanceByWidth(float cameraWidth, float worldWidth) const {
  float theta = cameraWidth / iparams_.width * FOVx;
  float worldDistance = (worldWidth / 2) / tanf(theta / 2);
  return worldDistance;
}

float CameraMatrix::getWorldDistanceByHeight(float cameraHeight, float worldHeight) const {
  float theta = cameraHeight / iparams_.height * FOVy;
  float worldDistance = (worldHeight / 2) / tanf(theta / 2);
  return worldDistance;
}
    
float CameraMatrix::getCameraWidthByDistance(float distance, float worldWidth) const {
  auto theta = 2.0f * atanf(worldWidth / 2.0f / distance);
  auto cameraWidth = theta / FOVx * iparams_.width;
  return cameraWidth;
}
 
float CameraMatrix::getCameraHeightByDistance(float distance, float worldHeight) const {
  auto theta = 2.0f * atanf(worldHeight / 2.0f / distance);
  auto cameraHeight = theta / FOVy * iparams_.height;
  return cameraHeight;
}

float CameraMatrix::getWorldHeight(Coordinates top, Coordinates bottom) const {
  Vector4f im;
  im << top.x, top.y, 1, 1;
  Vector4f ttarget = camToWorld_ * im;
  Vector3f rayTop = (ttarget.segment<3>(0) - cameraPosition_);
  im << bottom.x, bottom.y, 1, 1;
  Vector4f btarget = camToWorld_ * im;
  Vector3f rayBottom = (btarget.segment<3>(0) - cameraPosition_);
  float u = -cameraPosition_[2] / rayBottom[2];
  Vector3f pointBottom = rayBottom * u + cameraPosition_;
  float t = u * rayBottom.segment<2>(0).norm() / rayTop.segment<2>(0).norm();
  Vector3f pointTop = rayTop * t + cameraPosition_;
  float height = pointTop[2] - pointBottom[2];
  return height;
}
