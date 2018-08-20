#pragma once

#include <vector>
#include <map>
#include <common/annotations/Annotation.h>
#include <common/annotations/Selection.h>
#include <common/annotations/PolygonSelection.h>
#include <common/annotations/ManualSelection.h>
#include <common/annotations/EllipseSelection.h>
#include <common/annotations/RectangleSelection.h>
#include <common/annotations/SquareSelection.h>
#include <vision/enums/Colors.h>
#include <math/Point.h>
#include <common/ImageParams.h>
#include <algorithm>

class VisionAnnotation : public Annotation {
  private:
    std::vector<Selection*> selections_;
    std::string name_;
    int minFrame_, maxFrame_;
    Color color_;
    Camera::Type camera_;
    bool isSample_;
    std::map<int,Point> centerPoints_;
    void serialize(YAML::Emitter& emitter) const override;
    void deserialize(const YAML::Node& node) override;
  public:
    VisionAnnotation();
    VisionAnnotation(std::string name);
    void addSelection(Selection* selection);
    void removeSelection(Selection* selection);
    bool hasSelection(Selection* selection);
    std::string getName();
    void setName(std::string);
    const std::vector<Selection*> getSelections() const;
    bool isInFrame(int frame);
    void setMaxFrame(int frame);
    void setMinFrame(int frame);
    int getMaxFrame();
    int getMinFrame();
    Color getColor();
    void setColor(Color);
    Camera::Type getCamera();
    void setCamera(Camera::Type camera);
    bool isSample();
    void setSample(bool isSample);
    std::vector<Point> getEnclosedPoints();
    std::vector<Point> getEnclosedPoints(int frame);
    bool enclosesPoint(int x, int y);
    bool enclosesPoint(int x, int y, int frame);

    VisionAnnotation* copy();

    void setCenterPoint(int x, int y, int frame);
    Point getCenter();
    Point getCenter(int frame);
    void updateSelectionOffsets();
    void setCurrentFrame(int frame);
    void clearCenterPoints();
    void remapCenterPoints(int offset);

    bool operator==(const VisionAnnotation& other);
};
