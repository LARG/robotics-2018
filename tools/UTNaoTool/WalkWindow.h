#ifndef WALK_WINDOW_H
#define WALK_WINDOW_H

#include <QWidget>

#include <memory/MemoryFrame.h>
#include <memory/WalkRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/WalkInfoBlock.h>

class QLabel;
class QWidget;

class WalkWindow : public QWidget {
 Q_OBJECT

  public:
  WalkWindow();
    
  void update(MemoryFrame* memory);

  QLabel* walkLabels;

  QLabel* walkCommands;
  QLabel* walkOdometry;

  QLabel* walkPosition;
  QLabel* walkVelocity;
  QLabel* walkNextPosition;
    
};

#endif
