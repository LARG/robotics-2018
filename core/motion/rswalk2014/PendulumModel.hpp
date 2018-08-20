#pragma once

#include "WalkCycle.hpp"

struct PendulumModel {
   PendulumModel() : x(0), pendulum_height(300.0f), dx(0), accX(0), theta(0), walkCycle(0.0f, 0.0f, 0.0f, 0.0f, 0.5, 0.0f) {}
   // position of top of pendulum releative to support foot in x plane
   float x;

   float pendulum_height;

   // velocity of the Centre of Mass in x plane per 1 frame @ 100hz
   float dx;

   // acceleration of CoM in its x axis
   float accX;

   // position of top of pendulum relative to the RR cordinate system in x plane
   float psi;

   //lean of inverted pendulum in clock-wise direction
   float theta;

   static const float h;
   static const float g;// = 9806.65f;

   // our current position in the walkCycle
   WalkCycle walkCycle; 
   
   void predictNext(float dt, float pivot);

   void setPendulumHeight(float height) { pendulum_height = height;}

   template<class Archive>
   void serialize(Archive &ar, const unsigned int file_version) {
      ar & x;
      ar & dx;
      ar & psi;
   }
};

