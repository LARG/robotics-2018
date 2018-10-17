#include "WorldGLWidget.h"
#include <common/annotations/AnnotationGroup.h>
#include "simulation/Simulation.h"

using namespace qglviewer;
using namespace std;

auto MOUSE_BUTTONS = vector<Qt::MouseButton> { Qt::LeftButton, Qt::MidButton, Qt::RightButton };

WorldGLWidget::WorldGLWidget(QWidget* p): 
  QGLViewer(p), 
  drawer_(this),
  simulation_(NULL),
  annotations_(NULL) {
    connect(this, SIGNAL(groundDragging(Point2D, Point2D, Qt::MouseButton)), this, SLOT(dragStart(Point2D, Point2D, Qt::MouseButton)));
    connect(this, SIGNAL(groundDragged(Point2D, Point2D, Qt::MouseButton)), this, SLOT(dragEnd(Point2D, Point2D, Qt::MouseButton)));
    dragInfo_.dragging = false;
}

void WorldGLWidget::updateMemory(MemoryCache cache){
  cache_ = cache;
  draw();
  update();
}

void WorldGLWidget::init() {
  glEnable(GL_LIGHTING);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  setSceneRadius(250.0);
  setCamera(OVERHEAD);
  setWheelBinding(Qt::NoModifier, CAMERA, MOVE_FORWARD);
  setMouseBinding(Qt::NoModifier, Qt::RightButton, CAMERA, LOOK_AROUND);
  setMouseTracking(true);
}


void WorldGLWidget::setCamera(int position) {
  Vec origin(0,0,0);
  Vec cpos;

  if (position==OVERHEAD) {
    cpos = Vec(0,0, 775);
    Quaternion q(0,0,0,-1.0);
    camera()->setOrientation(q);
  } else if (position==OVERHEADREV){
    cpos = Vec(0,0, 775);
    Quaternion q(0,0,sin(DEG_T_RAD*90.0),cos(DEG_T_RAD*-90.0));
    camera()->setOrientation(q);
  } else if (position==DEFENSIVEHALF) {
    cpos = Vec(-HALF_GRASS_X/2.0/FACT,0, 450);
    Quaternion q(0,0,cos(DEG_T_RAD*45.0),sin(DEG_T_RAD*-45.0));
    camera()->setOrientation(q);
  } else if (position==OFFENSIVEHALF) {
    cpos = Vec(HALF_GRASS_X/2.0/FACT,0, 450);
    Quaternion q(0,0,cos(DEG_T_RAD*45.0),sin(DEG_T_RAD*-45.0));
    camera()->setOrientation(q);
  } else if (position==DEFENSIVEISO) {
    cpos = Vec(-500,-210, 265);
    Quaternion q(-0.4,0.23,0.45,-0.75);
    camera()->setOrientation(q);
  } else if (position==OFFENSIVEISO) {
    cpos = Vec(500,190, 265);
    Quaternion q(-0.23,-0.4,-0.75,-0.45);
    camera()->setOrientation(q);
  } else if (position==ABOVEROBOT) {
    WorldObject* self = &cache_.world_object->objects_[cache_.robot_state->WO_SELF];
    cpos = Vec(self->loc.x/FACT,self->loc.y/FACT, 265);
    Quaternion q(0,0,0,-1.0);
    origin = Vec(self->loc.x/FACT,self->loc.y/FACT, 50);
    camera()->setOrientation(q);
  } else {
    return;
  }
  camera()->setPosition(cpos);
  // camera()->setRevolveAroundPoint(origin);
  camera()->setPivotPoint(origin);

  update();
}
void WorldGLWidget::loadState(char* fileName) {
  setStateFileName(fileName);
  if (!restoreStateFromFile()) {
    Vec c(0,0, 650);
    Quaternion q(0,0,0,-1.0);
    camera()->setPosition(c);
    camera()->setOrientation(q);
  }
  init();
}

void WorldGLWidget::keyPressEvent(QKeyEvent* kevent) {
  kpressed_.insert(kevent->key());
  modifiers_ = kevent->modifiers();
  processCameraKeys();
}

void WorldGLWidget::keyReleaseEvent(QKeyEvent* kevent) {
  kpressed_.erase(kevent->key());
  modifiers_ = Qt::NoModifier;;
}

Point2D WorldGLWidget::projectedMousePosition(int x, int y) {
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winX, winY, winZ;
  GLdouble posX, posY, posZ;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  winX = (float)x;
  winY = (float)viewport[3] - (float)y;
  glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
  gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
  return Point2D(posX * FACT, posY * FACT);
}

void WorldGLWidget::mousePressEvent(QMouseEvent* mevent) {
  QGLViewer::mousePressEvent(mevent);
  auto pos = projectedMousePosition(mevent->x(), mevent->y());
  for(auto b : MOUSE_BUTTONS) {
    if(mevent->buttons() & b) {
      clicked(pos, b);
      mpressed_[b] = pos;
    }
  }
}

void WorldGLWidget::mouseMoveEvent(QMouseEvent* mevent) {
  QGLViewer::mouseMoveEvent(mevent);
  auto pos = projectedMousePosition(mevent->x(), mevent->y());
  for(auto b : MOUSE_BUTTONS) {
    if(mevent->buttons() & b) {
      if(mpressed_.find(b) != mpressed_.end() && (modifiers_ & Qt::ControlModifier)) {
        groundDragging(mpressed_[b], pos, b);
        mdragged_.insert(b);
      }
    }
  }
  moved(pos);
}

void WorldGLWidget::mouseReleaseEvent(QMouseEvent* mevent) {
  QGLViewer::mouseReleaseEvent(mevent);
  auto pos = projectedMousePosition(mevent->x(), mevent->y());
  for(auto b : MOUSE_BUTTONS) {
    if(!(mevent->buttons() & b)) {
      if(mdragged_.find(b) != mdragged_.end())
        groundDragged(mpressed_[b], pos, b);
      mpressed_.erase(b);
      mdragged_.erase(b);
      released(pos, b);
    }
  }
  if(mpressed_.size() == 0) dragInfo_.dragging = false;
}

void WorldGLWidget::dragStart(Point2D start, Point2D end, Qt::MouseButton button) {
  dragInfo_.dragging = true;
  dragInfo_.start = start;
  dragInfo_.end = end;
  dragInfo_.button = button;
}

void WorldGLWidget::dragEnd(Point2D,Point2D,Qt::MouseButton) {
}

void WorldGLWidget::processCameraKeys() {
  auto ispressed = [&] (int key) {
    return kpressed_.find(key) != kpressed_.end();
  };
  Quaternion orientation = camera()->orientation();
  Vec modifier(0,0,0);
  float speed = 10;
  if(ispressed(Qt::Key_W)) {
    if(modifiers_ & Qt::ControlModifier)
      modifier[2] = -speed;
    else
      modifier[1] = speed;
  }
  if(ispressed(Qt::Key_S)) {
    if(modifiers_ & Qt::ControlModifier)
      modifier[2] = speed;
    else
      modifier[1] = -speed;
  }
  if(ispressed(Qt::Key_A)) {
    modifier[0] = -speed;
  }
  if(ispressed(Qt::Key_D)) {
    modifier[0] = speed;
  }
  if(ispressed(Qt::Key_Up)) {
    modifier[1] = speed;
  }
  if(ispressed(Qt::Key_Down)) {
    modifier[1] = -speed;
  }
  if(ispressed(Qt::Key_Left)) {
    modifier[0] = -speed;
  }
  if(ispressed(Qt::Key_Right)) {
    modifier[0] = speed;
  }
  modifier = orientation * modifier;
  if(ispressed(Qt::Key_Space)) {
    modifier[2] = speed;
  }
  if(ispressed(Qt::Key_Z)) {
    modifier[2] = -speed;
  }
  Vec cpos = camera()->position();
  cpos += modifier;
  camera()->setPosition(cpos);

  // The default behavior doesn't work well for looking around
  //Vec rpos = camera()->revolveAroundPoint();
  //rpos += modifier;
  //camera()->setRevolveAroundPoint(cpos);
  
  update();
}

void WorldGLWidget::saveState(char* fileName) {
  setStateFileName(fileName);
  saveStateToFile();
}

void WorldGLWidget::draw() {
  draw(simulation_);
}

void WorldGLWidget::draw(Simulation* simulation) {
  drawer_.memory_ = cache_.memory;
  drawer_.setAnnotations(annotations_);

  if(simulation) {
    drawer_.setGtCache(simulation->getGtMemoryCache());
    drawer_.setBeliefCache(simulation->getBeliefMemoryCache());
    drawer_.draw(display_);
    if (display_[GLDrawer::SHOW_SIM_INFO]) drawer_.displaySimInfo(simulation->getSimInfo());
    if (display_[GLDrawer::SHOW_TRUE_SIM_LOCATION]) {
      auto caches = simulation->getPlayerGtMemoryCaches();
      drawer_.drawTrueSimLocations(caches);
    }
    if (display_[GLDrawer::SHOW_SIM_ROBOTS]) {
      auto caches = simulation->getPlayerGtMemoryCaches();
      drawer_.drawSimRobots(caches);
    }
    auto gtcaches = simulation->getPlayerGtMemoryCaches();
    auto bcaches = simulation->getPlayerBeliefMemoryCaches();
    if (display_[GLDrawer::SHOW_VISION_RANGE]) drawer_.drawVisionRanges(bcaches);

    if (display_[GLDrawer::SHOW_LOCALIZATION_INFO]) drawer_.drawAlternateRobots(bcaches);
  } else if (cache_.memory) {
    drawer_.setGtCache(cache_);
    drawer_.setBeliefCache(cache_);
    drawer_.draw(display_);
    if (display_[GLDrawer::SHOW_LOCALIZATION_INFO]) drawer_.drawAlternateRobots(cache_);
    if (display_[GLDrawer::SHOW_VISION_RANGE]) drawer_.drawVisionRanges(cache_);
  }
  if(dragInfo_.dragging)
    basicGL_.drawArrow(dragInfo_.start, dragInfo_.end);
}

void WorldGLWidget::updateDisplay(const map<GLDrawer::DisplayOption, bool>& display) {
  display_ = display;
  draw();
  update();
}

void WorldGLWidget::updateAnnotations(AnnotationGroup* annotations) {
  annotations_ = annotations;
}

void WorldGLWidget::setSimulation(Simulation* simulation) {
  simulation_ = simulation;
}
