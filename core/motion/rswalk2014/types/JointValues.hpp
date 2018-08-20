#pragma once

#include <utils/body.hpp> // Removed utils - Josiah

/**
 * A container for joint angles & joint stiffnesses
 */
struct JointValues {
   JointValues() {
      for (int i = 0; i < RSJoints::NUMBER_OF_JOINTS; ++i) {
         angles[i] = NAN;
         stiffnesses[i] = NAN;
         temperatures[i] = NAN;
      }
   }
   JointValues(bool zero) {
      for (int i = 0; i < RSJoints::NUMBER_OF_JOINTS; ++i) {
         angles[i] = 0;
         stiffnesses[i] = 0;
         temperatures[i] = 0;
      }
   }
   /* Angles in radians. Used both for sensor reading and actuating */
   float angles[RSJoints::NUMBER_OF_JOINTS];
   /* Stiffnesses [-1.0, 0.0..1.0]. Used only for actuating */
   float stiffnesses[RSJoints::NUMBER_OF_JOINTS];
   /* Temperatures (estimated) in degrees Celcius. Used only for reading */
   float temperatures[RSJoints::NUMBER_OF_JOINTS];

   template<class Archive>
   void serialize(Archive &ar, const unsigned int file_version) {
      ar & angles;
      ar & stiffnesses;
      ar & temperatures;
   }
};
