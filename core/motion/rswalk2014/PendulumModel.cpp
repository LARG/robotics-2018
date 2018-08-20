#include "PendulumModel.hpp"
#include <cmath>
#include <iostream>

using namespace std;

void PendulumModel::predictNext(float dt, float supportPivot){
   x = x + dx * dt + accX * (dt*dt)/2; 
   dx = dx + accX * dt;
   theta = atan2((x - supportPivot), pendulum_height);
   //cout << "x-p: " << (x - supportPivot) << endl;
   accX = g * sin(theta) * cos(theta);
   //walkCycle.next();
}

const float PendulumModel::g = 9806.65f;
