#include <QtGui>

#include "PlotWindow.h"
#include <math/Pose3D.h>
#include <math/Geometry.h>
#include <memory/FrameInfoBlock.h>
#include <memory/GraphableBlock.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <fstream>

PlotWindow::PlotWindow() :
  QWidget(),
  memory_log_(NULL)
{
  QGridLayout *layout = new QGridLayout;

  plot_.setCanvasBackground(QBrush("white"));
  layout->addWidget(&plot_, 0, 0, 5, 8);

  // add marker
  QPen pen;
  pen.setColor("black");
  pen.setWidth(2);
  plot_marker_.setLineStyle(QwtPlotMarker::VLine);
  plot_marker_.setLinePen(pen);
  plot_marker_.attach(&plot_);
  
  // add panner and magnifier
  panner_ = new QwtPlotPanner(plot_.canvas());
  magnifier_ = new QwtPlotMagnifier(plot_.canvas());
  // add legend
  legend_ = new QwtLegend;
  plot_.insertLegend(legend_,QwtPlot::TopLegend);
        
  setLayout(layout);
  resize(1000,600);
  setWindowTitle(tr("Plot System"));
}

PlotWindow::~PlotWindow() {
  for (unsigned int i = 0; i < plot_curves_.size(); i++)
    delete plot_curves_[i];
  delete panner_;
  delete magnifier_;
  delete legend_;
}

void PlotWindow::addLine(std::string name, std::string color) {
  GraphableBlock *graphable(NULL);
  double *data = new double[times_.size()];
  
  for (unsigned int i = 0; i < memory_log_->size(); i++) {
    (*memory_log_)[i].getBlockByName(graphable,"graphable");
    if (graphable == NULL) return;
    data[i] = graphable->getData(name.c_str());
  }
  
  QPen pen;
  pen.setWidth(3);
  pen.setColor(color.c_str());
  
  plot_curves_.push_back(new QwtPlotCurve(name.c_str()));
  int index = plot_curves_.size() - 1;
  plot_curves_[index]->setSamples(&times_[0],data,memory_log_->size());
  plot_curves_[index]->attach(&plot_);
  plot_curves_[index]->setPen(pen);

  delete [] data;
}

void PlotWindow::setMemoryLog(LogViewer* memory_log) {
  memory_log_ = memory_log;

  for (unsigned int i = 0; i < memory_log_->size(); i++) {
    MemoryFrame memory = memory_log_->getFrame(i);
    populateGraphableBlock(memory);
  }

  // set up the time
  FrameInfoBlock *frame_info(NULL);

  // clear times and data for new log
  times_.clear();
  for (unsigned i = 0; i < plot_curves_.size(); i++){
    plot_curves_[i]->detach();
    delete plot_curves_[i];
  }
  plot_curves_.clear();

  for (unsigned int i = 0; i < memory_log_->size(); i++) {
    (*memory_log_)[i].getBlockByName(frame_info,"frame_info",false);
    if (frame_info == NULL)
      (*memory_log_)[i].getBlockByName(frame_info,"vision_frame_info",false);
    if (frame_info == NULL)
      continue;
    times_.push_back(frame_info->seconds_since_start);
  }
  
  std::string filename = std::string(getenv("NAO_HOME")) + "/tools/UTNaoTool/graph.csv";
  std::ifstream in(filename.c_str());
  std::string name;
  std::string color;
  if (!in.good()) {
    std::cout << "ERROR opening: " << filename << std::endl;
    return;
  }
  std::cout << "Graphing from: " << filename << std::endl;
  while (!in.eof()) {
    getline(in,name,',');
    if (in.eof())
      break;
    getline(in,color,'\n');
    if (in.eof())
      break;
    if (name[0] == '#')
      continue;
    addLine(name,color);
  }
  //addLine("com.x","green");
  //addLine("commanded com.x","blue");
  plot_.replot();
}

void PlotWindow::update(MemoryFrame* mem) {
  FrameInfoBlock *frame_info(NULL);
  mem->getBlockByName(frame_info,"frame_info",false);
  if (frame_info == NULL) return;
  plot_marker_.setXValue(frame_info->seconds_since_start);
  plot_.replot();
}

void PlotWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::MidButton)
    return;
  
  int offset = plot_.canvas()->x() + plot_.x();
  int xpos = event->x() - offset;
  double desired_time =  plot_.invTransform(QwtPlot::xBottom,xpos);
  int snapshot_index = -1;

  for (unsigned int i = 0; i < times_.size(); i++) {
    if (times_[i] > desired_time) {
      snapshot_index = i;
      break;
    }
  }
  if (snapshot_index < 0)
    snapshot_index = times_.size() - 1;
  emit gotoSnapshot(snapshot_index);
}

void PlotWindow::keyPressEvent(QKeyEvent *event) {
  //bool ctrl = event->modifiers() & (Qt::ControlModifier);
  switch (event->key()) {
  case Qt::Key_Left:
    emit prevSnapshot();
    break;
  case Qt::Key_Right:
    emit nextSnapshot();
    break;
  case Qt::Key_Up:
    emit play();
    break;
  case Qt::Key_Down:
    emit pause();
    break;
  }
}

#include <memory/WalkEngineBlock.h>
#include <memory/WalkParamBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/SensorBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/WorldObjectBlock.h>

void PlotWindow::populateGraphableBlock(MemoryFrame &memory) {
  GraphableBlock *graphable = NULL;
  memory.getOrAddBlockByName(graphable,"graphable");

  WalkEngineBlock *walk_engine;
  memory.getBlockByName(walk_engine,"walk_engine",false);
  if (walk_engine != NULL) {
    graphable->addData("abs_left_foot.x",walk_engine->abs_left_foot_.translation.x);
    graphable->addData("abs_left_foot.y",walk_engine->abs_left_foot_.translation.y);
    graphable->addData("abs_left_foot.z",walk_engine->abs_left_foot_.translation.z);
    graphable->addData("abs_left_foot.rot",walk_engine->abs_left_foot_.rotation.getZAngle());
    graphable->addData("abs_right_foot.x",walk_engine->abs_right_foot_.translation.x);
    graphable->addData("abs_right_foot.y",walk_engine->abs_right_foot_.translation.y);
    graphable->addData("abs_right_foot.z",walk_engine->abs_right_foot_.translation.z);
    graphable->addData("abs_right_foot.rot",walk_engine->abs_right_foot_.rotation.getZAngle());

    graphable->addData("sensor_zmp.x",walk_engine->sensor_zmp_.x);
    graphable->addData("sensor_zmp.y",walk_engine->sensor_zmp_.y);
    graphable->addData("current_state_zmp.x",walk_engine->current_state_.zmp_.x);
    graphable->addData("current_state_zmp.y",walk_engine->current_state_.zmp_.y);
    graphable->addData("current_state_pen_pos.x",walk_engine->current_state_.pen_pos_.x);
    graphable->addData("current_state_pen_pos.y",walk_engine->current_state_.pen_pos_.y);
    graphable->addData("desired_state_zmp.x",walk_engine->desired_next_state_.zmp_.x);
    graphable->addData("desired_state_zmp.y",walk_engine->desired_next_state_.zmp_.y);
    graphable->addData("desired_state_pen_pos.x",walk_engine->desired_next_state_.pen_pos_.x);
    graphable->addData("desired_state_pen_pos.y",walk_engine->desired_next_state_.pen_pos_.y);
    graphable->addData("desired_state_pen_vel.x",walk_engine->desired_next_state_.pen_vel_.x);
    graphable->addData("desired_state_pen_vel.y",walk_engine->desired_next_state_.pen_vel_.y);
    graphable->addData("desired_state_open_zmp.x",walk_engine->desired_next_without_closed_loop_.zmp_.x);
    graphable->addData("desired_state_open_zmp.y",walk_engine->desired_next_without_closed_loop_.zmp_.y);
    graphable->addData("desired_state_open_pen_pos.x",walk_engine->desired_next_without_closed_loop_.pen_pos_.x);
    graphable->addData("desired_state_open_pen_pos.y",walk_engine->desired_next_without_closed_loop_.pen_pos_.y);
    graphable->addData("desired_state_open_pen_vel.x",walk_engine->desired_next_without_closed_loop_.pen_vel_.x);
    graphable->addData("desired_state_open_pen_vel.y",walk_engine->desired_next_without_closed_loop_.pen_vel_.y);
    graphable->addData("ref_zmp.x",walk_engine->zmp_ref_[0].x);
    graphable->addData("ref_zmp.y",walk_engine->zmp_ref_[0].y);
    graphable->addData("delayed_zmp.x",walk_engine->delayed_zmp_state_.x);
    graphable->addData("delayed_zmp.y",walk_engine->delayed_zmp_state_.y);
    graphable->addData("delayed_com.x",walk_engine->delayed_pen_state_.x);
    graphable->addData("delayed_com.y",walk_engine->delayed_pen_state_.y);
    graphable->addData("sensor_com.x",walk_engine->sensor_pen_.x);
    graphable->addData("sensor_com.y",walk_engine->sensor_pen_.y);
    
    graphable->addData("swing_target.x",walk_engine->swing_foot_.translation.x);
    graphable->addData("swing_target.y",walk_engine->swing_foot_.translation.y);
    graphable->addData("swing_target.z",walk_engine->swing_foot_.translation.z);
    graphable->addData("swing_target.rot",walk_engine->swing_foot_.rotation.getZAngle());

    graphable->addData("step_current.x",walk_engine->step_current_.position_.translation.x);
    graphable->addData("step_current.y",walk_engine->step_current_.position_.translation.y);
    graphable->addData("step_current.rot",RAD_T_DEG * walk_engine->step_current_.position_.rotation);

    float support_foot;
    if (walk_engine->step_current_.is_left_foot_)
      support_foot = -10;
    else
      support_foot = 10;
    WalkParamBlock *param_block(NULL);
    memory.getBlockByName(param_block,"walk_param",false);
    FrameInfoBlock *frame_info(NULL);
    memory.getBlockByName(frame_info,"frame_info",false);
    if (param_block != NULL && frame_info != NULL) {
      if (frame_info->frame_id < walk_engine->step_current_.frame_ + walk_engine->num_double_support_frames_)
        support_foot = 0;
  
      float phase_frac = (frame_info->frame_id - walk_engine->step_current_.frame_) / (float)(walk_engine->step_next_.frame_ - walk_engine->step_current_.frame_);
      //std::cout << phase_frac << std::endl;
      phase_frac = crop(phase_frac,-10,10);

      //float single_support_frac = (phase_frac - param_block->params_.double_support_frac_ - 0.01 / param_block->params_.phase_length_) / (1.0 - param_block->params_.double_support_frac_);
      float denom = walk_engine->step_next_.frame_ - walk_engine->step_current_.frame_;
      if (denom <= 0)
        denom = 1;
      float single_support_frac = ((int)frame_info->frame_id - (int)walk_engine->step_current_.frame_ - (int)walk_engine->num_double_support_frames_) / denom;


      graphable->addData("phase_frac",phase_frac);
      graphable->addData("single_support_frac",single_support_frac);
    }
    graphable->addData("support_foot",support_foot);
  }

  BodyModelBlock *body_model;
  memory.getBlockByName(body_model,"body_model",false);
  if (body_model != NULL) {
    graphable->addData("com.x",body_model->center_of_mass_.x);
    graphable->addData("com.y",body_model->center_of_mass_.y);
    graphable->addData("com.z",body_model->center_of_mass_.z);
     
    if (walk_engine != NULL) {
      Vector3<float> abs_to_stance_offset;
      Vector3<float> abs_to_swing_offset;
      if (walk_engine->step_current_.is_left_foot_) {
        abs_to_stance_offset = -body_model->abs_parts_[BodyPart::left_foot].translation;
        abs_to_swing_offset = -body_model->abs_parts_[BodyPart::right_foot].translation;
      } else {
        abs_to_stance_offset = -body_model->abs_parts_[BodyPart::right_foot].translation;
        abs_to_swing_offset = -body_model->abs_parts_[BodyPart::left_foot].translation;
      }

      graphable->addData("abs_to_stance_offset.x",abs_to_stance_offset.x);
      graphable->addData("abs_to_stance_offset.y",abs_to_stance_offset.y);
      graphable->addData("abs_to_swing_offset.x",abs_to_swing_offset.x);
      graphable->addData("abs_to_swing_offset.y",abs_to_swing_offset.y);

      // convert from abs to stance
      Pose3D com(0,0,0);
      com.translation = body_model->center_of_mass_ + abs_to_stance_offset;
      // convert from stance back to global
      com = com.relativeToGlobal(walk_engine->global_frame_offset_);
      com.translation.z += robot_dimensions_.footHeight;
      graphable->addData("global_com.x",com.translation.x);
      graphable->addData("global_com.y",com.translation.y);
      graphable->addData("global_com.z",com.translation.z);
    }

    graphable->addData("sensor_tilt", RAD_T_DEG*body_model->sensors_tilt_roll_.tilt_);
    graphable->addData("sensor_roll", RAD_T_DEG*body_model->sensors_tilt_roll_.roll_);
    graphable->addData("left_foot_tilt", RAD_T_DEG*body_model->left_foot_body_tilt_roll_.tilt_);
    graphable->addData("left_foot_roll",RAD_T_DEG* body_model->left_foot_body_tilt_roll_.roll_);
    graphable->addData("right_foot_tilt", RAD_T_DEG*body_model->right_foot_body_tilt_roll_.tilt_);
    graphable->addData("right_foot_roll", RAD_T_DEG*body_model->right_foot_body_tilt_roll_.roll_);
  }

  SensorBlock* sensors;
  memory.getBlockByName(sensors,"processed_sensors",false);
  if (sensors != NULL){
    graphable->addData("sensor_accel.x",sensors->values_[accelX]);
    graphable->addData("sensor_accel.y",sensors->values_[accelY]);
    graphable->addData("sensor_accel.z",sensors->values_[accelZ]);
    graphable->addData("sensor_tilt",sensors->values_[angleY]);
    graphable->addData("sensor_roll",sensors->values_[angleX]);
  }
  
  SensorBlock* raw_sensors;
  memory.getBlockByName(raw_sensors,"raw_sensors",false);
  if (raw_sensors != NULL) {
    graphable->addData("raw_sensor_tilt",raw_sensors->values_[angleY]);
    graphable->addData("raw_sensor_roll",raw_sensors->values_[angleX]);
  }

  JointBlock *joint_angles;
  memory.getBlockByName(joint_angles,"processed_joint_angles",false);
  if (joint_angles != NULL) {
    graphable->addData("sensed_lhiproll",RAD_T_DEG * joint_angles->values_[LHipRoll]);
    graphable->addData("sensed_rhiproll",RAD_T_DEG * joint_angles->values_[RHipRoll]);
    graphable->addData("sensed_lhippitch",RAD_T_DEG * joint_angles->values_[LHipPitch]);
    graphable->addData("sensed_rhippitch",RAD_T_DEG * joint_angles->values_[RHipPitch]);

    graphable->addData("sensed_lanklepitch",RAD_T_DEG * joint_angles->values_[LAnklePitch]);
    graphable->addData("sensed_ranklepitch",RAD_T_DEG * joint_angles->values_[RAnklePitch]);
    graphable->addData("sensed_lankleroll",RAD_T_DEG * joint_angles->values_[LAnkleRoll]);
    graphable->addData("sensed_rankleroll",RAD_T_DEG * joint_angles->values_[RAnkleRoll]);
  }
  
  JointCommandBlock *joint_commands;
  memory.getBlockByName(joint_commands,"processed_joint_commands",false);
  if (joint_commands != NULL) {
    graphable->addData("commanded_lhiproll",RAD_T_DEG * joint_commands ->angles_[LHipRoll]);
    graphable->addData("commanded_rhiproll",RAD_T_DEG * joint_commands ->angles_[RHipRoll]);
    graphable->addData("commanded_lhippitch",RAD_T_DEG * joint_commands ->angles_[LHipPitch]);
    graphable->addData("commanded_rhippitch",RAD_T_DEG * joint_commands ->angles_[RHipPitch]);
    
    graphable->addData("commanded_lkneepitch",RAD_T_DEG * joint_commands ->angles_[LKneePitch]);
    graphable->addData("commanded_rkneepitch",RAD_T_DEG * joint_commands ->angles_[RKneePitch]);
    
    graphable->addData("commanded_lanklepitch",RAD_T_DEG * joint_commands->angles_[LAnklePitch]);
    graphable->addData("commanded_ranklepitch",RAD_T_DEG * joint_commands->angles_[RAnklePitch]);
    graphable->addData("commanded_lankleroll",RAD_T_DEG * joint_commands->angles_[LAnkleRoll]);
    graphable->addData("commanded_rankleroll",RAD_T_DEG * joint_commands->angles_[RAnkleRoll]);
  }
  
  OdometryBlock *odometry;
  memory.getBlockByName(odometry,"vision_odometry",false);
  if (odometry != NULL) {
    graphable->addData("odom.y",odometry->displacement.translation.y);
  }

  WorldObjectBlock *world_object;
  memory.getBlockByName(world_object,"world_objects",false);
  if (world_object != NULL) {
    WorldObject &ball = world_object->objects_[WO_BALL];
    graphable->addData("rel_ball.y",ball.relPos.y);
    graphable->addData("vision_rel_ball.y",ball.visionDistance * sin(ball.visionBearing));
  }
}
