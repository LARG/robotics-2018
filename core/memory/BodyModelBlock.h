#ifndef BODY_MODEL_BLOCK_
#define BODY_MODEL_BLOCK_

#include <common/RobotInfo.h>
#include <math/Pose3D.h>
#include <math/Vector3.h>
#include <math/Vector2.h>
#include <common/TiltRoll.h>
#include <kinematics/TorsoMatrix.h>

#include <memory/MemoryBlock.h>
#include <schema/gen/BodyModelBlock_generated.h>
#include <Eigen/Core>

DECLARE_INTERNAL_SCHEMA(struct BodyModelBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(BodyModelBlock);
    BodyModelBlock()  {
      header.version = 11;
      header.size = sizeof(BodyModelBlock);
      is_calculated_ = false;
      feet_on_ground_ = true;
      feet_on_ground_inst_ = true;
    }  
    SCHEMA_FIELD(bool is_calculated_);

    Pose3D* getRelPartPtr(int i) { return &rel_parts_[i]; }
    Pose3D* getAbsPartPtr(int i) { return &abs_parts_[i]; }
    // Body relative to origin // no rotations
    SCHEMA_FIELD(std::array<Pose3D,BodyPart::NUM_PARTS> rel_parts_);
    // Body translated and relative to ground
    SCHEMA_FIELD(std::array<Pose3D,BodyPart::NUM_PARTS> abs_parts_);

    SCHEMA_FIELD(TorsoMatrix torso_matrix_);
    SCHEMA_FIELD(Vector3<float> center_of_mass_);

    SCHEMA_FIELD(TiltRoll left_foot_body_tilt_roll_);
    SCHEMA_FIELD(TiltRoll right_foot_body_tilt_roll_);
    SCHEMA_FIELD(TiltRoll sensors_tilt_roll_);

    SCHEMA_FIELD(Vector2<float> zmpFromFSRs);

    SCHEMA_FIELD(bool feet_on_ground_); // if up for over 25 frames
    SCHEMA_FIELD(bool feet_on_ground_inst_); // instantaneous
});

#endif 
