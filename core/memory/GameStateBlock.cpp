#include <memory/GameStateBlock.h>
#include <common/tinyformat.h>

void GameStateBlock::setState(State state) {
  // If we've heard the whistle recently don't go back into set
  if(whistleElapsedTime() <= 20 && state == SET && state_ == PLAYING) {
    return;
  }

  // Otherwise proceed normally
  if(state != state_) {
    stateStartTime_ = time(NULL);
    prevstate_ = state_;
    state_ = state;
  }
  if(state_ == SET) {
    spawned_whistle_positions_ = false;
  }
}

void GameStateBlock::whistleOverride() {
  whistleTime = time(NULL);
  setState(PLAYING);
}
