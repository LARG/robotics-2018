#include <tool/TeamPacketWidget.h>
#include <memory/FrameInfoBlock.h>
#include <memory/TeamPacketsBlock.h>

TeamPacketWidget::TeamPacketWidget(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  statuses_ = {
    {Robot1,status1},
    {Robot2,status2},
    {Robot3,status3},
    {Robot4,status4},
    {Robot5,status5}
  };
  for(int i = 0; i < NUM_RobotSelections; i++) {
    auto robot = (RobotSelection)i;
    robotBox->addItem(getName(robot));
  }
  QTimer* timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(displayPacketData()));
  timer->setInterval(100);
  timer->setSingleShot(false);
  timer->start();

  connect(robotBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setSelectedRobot(int)));
}

void TeamPacketWidget::updateMemory(MemoryCache cache) {
  cache_ = cache;
}

void TeamPacketWidget::updateStatuses() {
  for(auto kvp : statuses_) {
    auto label = kvp.second;
    label->setPixmap(QPixmap(":/images/question.png"));
  }
}

void TeamPacketWidget::displayPacketData() {
}

void TeamPacketWidget::setSelectedRobot(int i) {
  current_ = (RobotSelection)i;
  std::string display;
  for(auto kvp : displays_) {
    if(current_ == kvp.first || current_ == AllRobots)
      display += kvp.second;
  }
  displayText->setText(QString::fromStdString(display));
}
