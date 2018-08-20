#include <tool/simulation/PhysicsSimulator.h>
#include <memory/WorldObjectBlock.h>

#define TIME_INC (1.0f/30.0f)
#define DECAY_RATE 0.966
#define FOOT_X_FRONT 10
#define FOOT_X_BACK -10
#define FOOT_Y_OUT 30

#define getObject(obj, idx) auto& obj = world_object_->objects_[idx]

void PhysicsSimulator::setObjects(WorldObjectBlock* objects) {
  world_object_ = objects;
}

void PhysicsSimulator::step() {
  stepBall();
}

void PhysicsSimulator::stepBall() {
  getObject(ball, WO_BALL);
  Point2D delta = ball.absVel * TIME_INC;
  ball.loc += delta;
  ball.absVel *= DECAY_RATE;
  for (int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
    getObject(player, i);
    auto relBall = ball.loc.globalToRelative(player.loc, player.orientation);
    if(relBall.x >= FOOT_X_BACK && relBall.x <= FOOT_X_FRONT && fabs(relBall.y) <= FOOT_Y_OUT) {
      // If the ball is moving, do an inelastic collision
      if(ball.absVel.getMagnitude() > EPSILON) {
        Point2D collisionDir = player.loc - ball.loc;
        auto dot = ball.absVel.x * collisionDir.x + ball.absVel.y * collisionDir.y;
        float ctheta = dot / (ball.absVel.getMagnitude() * collisionDir.getMagnitude());
        Point2D component = collisionDir / collisionDir.getMagnitude() * ball.absVel.getMagnitude() * ctheta;
        ball.absVel = ball.absVel - component * 2;
        ball.absVel *= .5; //Inelastic
      // Otherwise just move it to the closest open location
      } else {
        float pdist = 250;
        Point2D placement;
        if(relBall.x >= FOOT_X_FRONT - 10) // front
          placement = Point2D(pdist, 0);
        else if (relBall.x <= FOOT_X_BACK + 10) // back
          placement = Point2D(-pdist, 0);
        else if(relBall.y > 0) // left
          placement = Point2D(0, pdist);
        else // right
          placement = Point2D(0, -pdist);
        placement = relBall + placement;
        ball.loc = placement.relativeToGlobal(player.loc, player.orientation);
      }
    }
  }
}

void PhysicsSimulator::moveBall(Point2D target) {
  getObject(ball, WO_BALL);
  ball.absVel = (target - ball.loc) * (1 - DECAY_RATE) / TIME_INC;
}

