#pragma once

#include <QtGui>
#include <QWidget>

#include <map>
#include <vector>
#include <fstream>

#include <common/annotations/AnnotationGroup.h>
#include <common/annotations/VisionAnnotation.h>
#include <common/annotations/RectangleSelection.h>
#include <common/annotations/PolygonSelection.h>
#include <common/annotations/EllipseSelection.h>
#include <common/annotations/SelectionType.h>
#include <tool/annotations/AnnotationAnalyzer.h>
#include <tool/ConfigWidget.h>
#include <tool/ToolConfig.h>

#include "ui_AnnotationWidget.h"

#include <vision/VisionModule.h>
#include <vision/ColorTableMethods.h>
#include <math/Point.h>

class AnnotationListWidgetItem : public QListWidgetItem {
  private:
    VisionAnnotation* annotation_;
    void init(VisionAnnotation* annotation){
      annotation_ = annotation;
      QString camera(annotation->getCamera() == Camera::TOP ? "Top: " : "Bottom: ");
      setText(camera + QString::fromStdString(annotation->getName()));
    }
  public:
    AnnotationListWidgetItem(VisionAnnotation* annotation) {
      init(annotation);
    }
    AnnotationListWidgetItem(const AnnotationListWidgetItem& other) : QListWidgetItem(other) {
      init(other.annotation_);
    }
    VisionAnnotation* getAnnotation() {
      return annotation_;
    }
    void resetText(){
      init(annotation_);
    }
};

class SelectionListWidgetItem : public QListWidgetItem {
  private:
    Selection* selection_;
    void init(Selection* selection){
      selection_ = selection;
      setText(QString::fromStdString(selection->getName()));
    }
  public:
    SelectionListWidgetItem(Selection* selection) {
      init(selection);
    }
    SelectionListWidgetItem(const SelectionListWidgetItem& other) : QListWidgetItem(other) {
      init(other.selection_);
    }
    Selection* getSelection() {
      return selection_;
    }
    void resetText(){
      init(selection_);
    }
};

class AnnotationWidget : public ConfigWidget, public Ui_UTAnnotationWidget {
  Q_OBJECT

  private:
    int maxFrames_, currentFrame_;
    Color selectedColor_;
    std::map<std::string, VisionAnnotation*> annotations_;
    VisionAnnotation *selectedAnnotation_;
    void clearSelections();
    void loadSelections(VisionAnnotation*);
    void loadChoices(VisionAnnotation*);
    bool filterAnnotation(VisionAnnotation*);
    Camera::Type selectedCamera_, currentCamera_;
    ImageProcessor* topProcessor_, *bottomProcessor_;
    AnnotationAnalyzer analyzer;
    LogViewer* log_;
    QPoint offset_, previous_;
    std::map<Annotation*, AnnotationListWidgetItem*> annotationItems_;
    std::map<Selection*, SelectionListWidgetItem*> selectionItems_;
    bool selecting_ = false;
    AnnotationConfig config_;

  protected:
    void loadConfig(const ToolConfig& config);
    void saveConfig(ToolConfig& config);
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    inline bool enabled() { return cbxEnable->isChecked(); }

  public:
    AnnotationWidget(QWidget* parent);
    void setImageProcessors(ImageProcessor*,ImageProcessor*);
    void setMaxFrames(int);
    std::vector<VisionAnnotation*> getAnnotations();
    std::vector<Selection*> getSelections();

  public slots:
    void controlsChanged() override;
    void selected(Selection*);
    void annotationSelected();
    void selectionSelected();
    void selectionBoxIndexChanged(const QString& label);
    void colorBoxIndexChanged(const QString& label);
    void cameraBoxIndexChanged(const QString& label);
    void enableToggled(bool);
    void handleNewLogFrame(int frame);
    void setCurrentCamera(Camera::Type camera);
    void handleNewLogLoaded(LogViewer* log);
    void handleDragged(int x, int y, Qt::MouseButton button=Qt::NoButton);
    void handleHovered(int x, int y);
    void clearMovements();

    void insert();
    void update();
    void deleteAnnotation();
    void deleteSelection();
    void redrawCurrentSelections();

    void saveToFile();
    void updateFrameSelection();

  signals:
    void selectionTypeChanged(SelectionType);
    void selectionEnabled(bool);
    void setCurrentSelections(std::vector<Selection*>);
    void setCurrentAnnotations(std::vector<VisionAnnotation*>);
    void setCurrentLogFrame(int frame);
};
