#ifndef STATE_WINDOW_H
#define STATE_WINDOW_H

#include <QWidget>

#include <memory/MemoryFrame.h>
#include <memory/RobotStateBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/FrameInfoBlock.h>

#include <common/States.h>
#include <common/Roles.h>

class QLabel;
class QWidget;

class StateWindow : public QWidget {
 Q_OBJECT

  public:
   enum {
     FrameNumber = 0,
     Time,
     MemorySource,
     WO_SELF,
     HeadVersion,
     BodyVersion,
     RobotID,
     BodyID,
     RBTeam,
     Role,
     State,
     GCTeam,
     PenaltySeconds,
     OurScore,
     TheirScore,
     SecondsLeft,
     PKState,
     WhistleTime,
     ProcessingAudio,
     WhistleFrame,
     TeammateWhistleFrame,
     NumItems
   };
    StateWindow();
      
    void update(MemoryFrame* memory);
    QLabel* labels;
    QLabel* values;
    QString names[NumItems];
    int idxmap[NumItems];

};

#endif
