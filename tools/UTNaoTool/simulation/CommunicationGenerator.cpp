#include <tool/simulation/CommunicationGenerator.h>
#include <memory/FrameInfoBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/TeamPacketsBlock.h>

using namespace std;

#define gtframe() gtcache_.frame_info->frame_id

CommunicationGenerator::CommunicationGenerator(int team, int player) {
  bcache_ = MemoryCache::create(team, player);
  coachframe_ = -10000;
  teamBroadcastFrame_ = teamSilenceFrame_ = 0;
  teamMode_ = true;
}

CommunicationGenerator::~CommunicationGenerator() {
}

void CommunicationGenerator::setCaches(MemoryCache gtcache, MemoryCache bcache) {
  vector<MemoryCache> bcaches = { bcache };
  setCaches(gtcache, bcaches);
}

void CommunicationGenerator::setCaches(MemoryCache gtcache, vector<MemoryCache> bcaches) {
  gtcache_ = gtcache;
  bcaches_ = bcaches;
}

void CommunicationGenerator::generateAllCommunications() {
  generateTeamBallCommunications();
  fillCaches();
}

void CommunicationGenerator::generateTeamBallCommunications() {
}


void CommunicationGenerator::fillCaches() {
  for(auto& cache : bcaches_) {
    *(cache.team_packets) = *(bcache_.team_packets);
  }
}
