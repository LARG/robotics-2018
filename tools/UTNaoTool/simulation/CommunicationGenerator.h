#ifndef COMMUNICATION_GENERATOR_H
#define COMMUNICATION_GENERATOR_H

#include <common/Random.h>
#include <memory/MemoryCache.h>

class CommunicationGenerator {
  public:
    CommunicationGenerator(int team, int player);
    ~CommunicationGenerator();
    void setCaches(MemoryCache gtcache, MemoryCache bcache);
    void setCaches(MemoryCache gtcache, std::vector<MemoryCache> bcaches);
    void generateAllCommunications();
  private:
    void generateTeamBallCommunications();
    void fillCaches();

    MemoryCache gtcache_, bcache_;
    std::vector<MemoryCache> bcaches_;
    int teamBroadcastFrame_, teamSilenceFrame_, coachframe_;
    bool teamMode_;
};

#endif
