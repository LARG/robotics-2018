#ifndef GL_DRAWER_H
#define GL_DRAWER_H

#include <memory/MemoryCache.h>
#include <behavior/BehaviorModule.h>

#include "ObjectsGL.h"
#include "BasicGL.h"
#include "RobotGL.h"
#include "LocalizationGL.h"

#include <common/Field.h>
#include <QtGui>

class AnnotationGroup;

class GLDrawer {
  public:
    ENUM(DisplayOption,
      SHOW_FIELD,
      SHOW_ROBOT,
      SHOW_BALL,
      SHOW_ROBOT_UNCERT,
      SHOW_LOCALIZATION_INFO,
      SHOW_FILTERED_OPPONENTS,
      SHOW_TRUTH_ROBOT,
      SHOW_TRUTH_BALL,
      SHOW_BALL_VEL,
      SHOW_BALL_UNCERT,
      SHOW_RELATIVE_OBJECTS,
      SHOW_RELATIVE_OBJECT_UNCERTS,
      SHOW_LOCATION_TEXT_OVERLAY,
      SHOW_OBSERVATION_TEXT_OVERLAY,
      SHOW_ALTERN_LOCATION_TEXT_OVERLAY,
      SHOW_OBJECT_ID_TEXT_OVERLAY,
      SHOW_VISION_RANGE,
      SHOW_TEAMMATES,
      SHOW_SEEN_OPPONENT,
      SHOW_GT_OPPONENT,
      SHOW_OPPONENT_OVERLAY,
      SHOW_TRUTH_OVERLAY,
      SHOW_ODOMETRY,
      SHOW_ODOMETRY_OVERLAY,
      SHOW_TEAM_PACKETS,
      SHOW_TEAM_BALL_PACKETS,
      SHOW_COACH_BALL_PACKETS,
      SHOW_TEAM_OVERLAY,
      SHOW_TARGET_POINT,
      SHOW_SONAR_OVERLAY,
      SHOW_ROLES,
      SHOW_NUMBERS,
      SHOW_KICK_NAME_OVERLAY,
      SHOW_STATIC_KICK_REGION,
      SHOW_LIVE_KICK_REGION,
      SHOW_KICK_CHOICES,
      SHOW_KICK_CHOICE_OVERLAY,
      SHOW_SIM_INFO,
      SHOW_SIM_ROBOTS,
      SHOW_ALL_PACKETS,
      SHOW_ALL_PACKETS_OVERLAY,
      SHOW_TRUE_SIM_LOCATION,
      SHOW_BEACONS,
      SHOW_GOALS,
      SHOW_LINES
    );

    GLDrawer(QGLWidget* parent);

    void setGtCache(MemoryCache cache);
    void setBeliefCache(MemoryCache cache);
    void setAnnotations(AnnotationGroup* annotations);
    void draw(const std::map<DisplayOption, bool>& display);
    
    void drawVisionRanges(MemoryCache c) { std::vector<MemoryCache> cv = { c }; drawVisionRanges(cv); }
    void drawVisionRanges(std::vector<MemoryCache> caches);
    void drawGtOpponents();
    void drawSimRobots(std::vector<MemoryCache> caches);
    void drawAlternateRobots(MemoryCache c) { std::vector<MemoryCache> cv = { c }; drawAlternateRobots(cv); }
    void drawAlternateRobots(std::vector<MemoryCache> caches);
    void drawTrueSimLocations(std::vector<MemoryCache> cachees);
    void displaySimInfo(std::string simInfo);

    int teammate;
    int simControl;
    int currentSim;
    int liveTeamNum;
    int liveTeamColor;
    int kickGridSize;
    int currKickChoice;
    MemoryFrame* memory_;

  private:
    void drawCenterOfMasses();
    void drawField();
    void drawRobot();
    void drawBall();
    void drawSeenOpponents();
    void drawFilteredOpponents();
    void drawTeammates();
    void drawTruthRobot();
    void drawTruthBall();
    void drawOdometry();
    void drawTeamPackets(int player, bool white);
    void drawTargetPoint();
    void drawPathPoints();
    void drawStaticKickRegion();
    void drawLiveKickRegion();
    void drawKickChoices();
    void drawAllTeamPackets();
    void drawTeamBallPackets();
    void drawCoachBallPackets();
    void drawAnnotations();
    void drawBeacons();
   
    void overlayOdometry();
    void overlayObservationText();
    void overlayOpponentText();
    void overlayTruthText();
    void overlayLocationText();
    void overlayAlternLocationText();
    void overlayBasicInfoText();
    void overlayObjectIDText();
    void overlayTeamPackets();
    void overlayKickChoices();
    void overlayAllTeamPackets();
    void overlaySonar();

    MemoryCache gtcache_, bcache_;
    
    ObjectsGL objectsGL;
    BasicGL basicGL;
    RobotGL robotGL;
    LocalizationGL localizationGL;

    QGLWidget* parent_;

    std::map<DisplayOption, bool> display_;
    BehaviorModule* behaviorModule;
    AnnotationGroup* annotations_;
};

#endif
