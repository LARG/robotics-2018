#ifndef ANALYSIS_WIDGET_H
#define ANALYSIS_WIDGET_H

#include <QtGui>
#include <QWidget>

#include <vision/VisionModule.h>
#include <vision/ColorTableMethods.h>
#include <VisionCore.h>

#include <common/annotations/VisionAnnotation.h>
#include <tool/annotations/AnnotationAnalyzer.h>
#include "ui_AnalysisWidget.h"

struct ballstats {
  int falseBalls, falseCandidates, missingBalls, missingCandidates;
  ballstats(){ falseBalls = falseCandidates = missingBalls = missingCandidates = 0; }
};

struct goalstats {
  int falsePosts, truePosts, missingPosts;
  goalstats(){falsePosts = truePosts = missingPosts = 0; }
};

class AnalysisWidget : public QWidget, public Ui_UTAnalysisWidget {
  Q_OBJECT
  private:
    vector<VisionAnnotation*> annotations_;
    AnnotationAnalyzer analyzer_;
    ImageProcessor *topProcessor_, *bottomProcessor_;
    Camera::Type camera_;
    QString colorStrings[Color::NUM_Colors];
    Color selectedColor_;
    LogViewer* log_;
    VisionCore* core_;
    int currentFrame_;

    void getBallStatistics(ballstats &stats, QString &statsText, const Camera::Type &camera_);
    ballstats getBallStatistics();
    goalstats getGoalStatistics();

    void displayGoalInfo(bool show);
    void displayBallInfo(bool show);

  public:
    AnalysisWidget(QWidget*);
    void setCore(VisionCore*);
    public slots:
      void handleNewLogLoaded(LogViewer*);
    void analyze();
    void prune();
    void setAnnotations(std::vector<VisionAnnotation*>);
    void setImageProcessors(ImageProcessor*,ImageProcessor*);
    void setCurrentCamera(Camera::Type);
    void colorBoxIndexChanged(const QString&);
    void handleColorTableGenerated();
    void handleNewLogFrame(int);
    void undo();
signals:
    void colorTableGenerated();
    void memoryChanged();

};

#endif
