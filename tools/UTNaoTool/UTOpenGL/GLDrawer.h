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
      SHOWFIELD,
      SHOWROBOT,
      SHOWROBOTUNCERT,
      SHOWFILTEREDOPPONENTS,
      SHOWTRUTHROBOT,
      SHOWTRUTHBALL,
      SHOWBALL,
      SHOWBALLVEL,
      SHOWBALLUNCERT,
      SHOWRELATIVEOBJECTS,
      SHOWRELATIVEOBJECTUNCERTS,
      SHOWLOCATIONTEXTOVERLAY,
      SHOWOBSERVATIONTEXTOVERLAY,
      SHOWALTERNLOCATIONTEXTOVERLAY,
      SHOWOBJECTIDTEXTOVERLAY,
      SHOWVISIONRANGE,
      SHOWALTERNATEROBOTS,
      SHOWTEAMMATES,
      SHOWSEENOPPONENT,
      SHOWGTOPPONENT,
      SHOWOPPONENTOVERLAY,
      SHOWTRUTHOVERLAY,
      SHOWODOMETRY,
      SHOWODOMETRYOVERLAY,
      SHOWTEAMPACKETS,
      SHOWTEAMBALLPACKETS,
      SHOWCOACHBALLPACKETS,
      SHOWTEAMOVERLAY,
      SHOWTARGETPOINT,
      SHOWSONAROVERLAY,
      SHOWROLES,
      SHOWNUMBERS,
      SHOWKICKNAMEOVERLAY,
      SHOWSTATICKICKREGION,
      SHOWLIVEKICKREGION,
      SHOWKICKCHOICES,
      SHOWKICKCHOICEOVERLAY,
      SHOWSIMINFO,
      SHOWSIMROBOTS,
      SHOWALLPACKETS,
      SHOWALLPACKETSOVERLAY,
      SHOWTRUESIMLOCATION,
      SHOWBEACONS,
      SHOWGOALS,
      SHOWLINES
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
