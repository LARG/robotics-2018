#include <memory/TeamPacketsBlock.h>

TeamPacketsBlock::TeamPacketsBlock() {
  header.version = 12;
  header.size = sizeof(TeamPacketsBlock);
  for(int i = WO_TEAM_FIRST; i <= WO_TEAM_LAST; i++) {
    frameReceived[i] = -10000;
    ballUpdated[i] = -10000;
    oppUpdated[i] = false;
    relayData[i].sentTime = 0;
  }
  sinceLastTeamPacketIn = 100;
}
    
Point2D TeamPacketsBlock::altBall(int self, int role, Point2D ball) {
  auto nothing = Point2D(-10'000,-10'000);
  //if(role != SUPPORTER) return nothing;
  RelayStruct* relay = nullptr;
  for(int i = WO_TEAM_FIRST; i <= WO_TEAM_LAST; i++) {
    if(i == self) continue;
    if(relayData[i].bvrData.role == CHASER) {
      relay = &relayData[i];
      break;
    }
  }
  if(relay == nullptr) return nothing;
  auto alt1 = relay->locData.altBall1();
  auto mag1 = (ball - alt1).getMagnitude();
  auto alt2 = relay->locData.altBall2();
  auto mag2 = (ball - alt2).getMagnitude();
  if(mag1 > mag2) return alt1;
  else return alt2;
}
