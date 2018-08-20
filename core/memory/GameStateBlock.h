#ifndef GAMESTATE_
#define GAMESTATE_

#include <memory/MemoryBlock.h>
#include <common/States.h>
#include <ctime>
#include <schema/gen/GameStateBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct GameStateBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(GameStateBlock);
    GameStateBlock():
      isPenaltyKick(false)
    {
      header.version = 2;
      header.size = sizeof(GameStateBlock);


      prevstate_ = state_ = INITIAL;
      ourKickOff = true;
      isFirstHalf = 1;
      gameContTeamNum=1;
      ourScore = 0;
      opponentScore = 0;
      secsTillUnpenalised = 0;
      dropInTime = -1.0;
      lastOutBy = 0;
      secsRemaining = 600;
      frameReceived = 0;
      lastStateChangeFromButton = false;
      lastTimeLeftPenalized = -1;
      whistleTime =  0;
      isFreeKick = false;
      isFreeKickTypeGoal = false;
    }

    State state() { return state_; }
    State prevstate() { return prevstate_; }
    bool change() { return stateElapsedTime() == 0; }
    void setState(State state);
    int stateElapsedTime() {
      return time(NULL) - stateStartTime_;
    }
    int stateStartTime() {
      return stateStartTime_;
    }
    void whistleOverride();
    int whistleElapsedTime() { return time(NULL) - whistleTime; }

    // state of the robot (playing, ready, etc)

    bool spawned_whistle_positions_ = false;

    // What team number are we on
    SCHEMA_FIELD(int gameContTeamNum);

    SCHEMA_FIELD(bool isPenaltyKick);

    // Game controller memory
    SCHEMA_FIELD(bool ourKickOff);
    SCHEMA_FIELD(int secsRemaining);
    SCHEMA_FIELD(int ourScore);
    SCHEMA_FIELD(int opponentScore);
    SCHEMA_FIELD(int secsTillUnpenalised);
    SCHEMA_FIELD(int isFirstHalf);   
    SCHEMA_FIELD(int lastOutBy);           
    SCHEMA_FIELD(int dropInTime);

    SCHEMA_FIELD(int frameReceived);
    SCHEMA_FIELD(int whistleTime);
    SCHEMA_FIELD(bool isFreeKick);
    SCHEMA_FIELD(bool isFreeKickTypeGoal);

    SCHEMA_FIELD(bool lastStateChangeFromButton);
    SCHEMA_FIELD(float lastTimeLeftPenalized);
  private:
    SCHEMA_FIELD(State state_);
    SCHEMA_FIELD(State prevstate_);
    SCHEMA_FIELD(int stateStartTime_);
});

#endif 
