#ifndef CALIBRATION_NFY4RBXJ
#define CALIBRATION_NFY4RBXJ

#include <fstream>

class Calibration {

  public:

  bool readFromFile(std::string fileName) {
    std::ifstream fin(fileName.c_str());
    if (fin.is_open()) {
      fin >> tilt_bottom_cam_;
      fin >> roll_bottom_cam_;
      fin >> tilt_top_cam_;
      fin >> roll_top_cam_;
      fin >> head_tilt_offset_;
      fin >> head_pan_offset_;
      fin.close();
      return true;
    }
    return false;
  }

  void writeToFile(std::string fileName) {
    std::ofstream fout(fileName.c_str());
    fout << tilt_bottom_cam_ << std::endl;
    fout << roll_bottom_cam_ << std::endl;
    fout << tilt_top_cam_ << std::endl;
    fout << roll_top_cam_ << std::endl;
    fout << head_tilt_offset_ << std::endl;
    fout << head_pan_offset_ << std::endl;
    fout.close();  
  }

  public:

  float tilt_bottom_cam_;
  float roll_bottom_cam_;
  float tilt_top_cam_;
  float roll_top_cam_;
  float head_tilt_offset_;
  float head_pan_offset_;

};

#endif /* end of include guard: CALIBRATION_NFY4RBXJ */


