#pragma once

#include <common/YamlConfig.h>
#include <common/annotations/SelectionType.h>
#include <map>

#include <math/Point.h>
#include <stdint.h>

class Selection : public YamlConfig {
  private:
    std::map<int,Point> offsets_;
    int currentFrame_;
    bool hovered_ = false;
  protected:
    Point getOffset(int frame) const {
      auto it = offsets_.find(frame);
      if(it != offsets_.end())
        return it->second;
      return Point(0,0);
    }
    Point getOffset() const {
      return getOffset(currentFrame_);
    }
    bool hasOffsets() const {
      return !offsets_.empty();
    }
  public:
    virtual ~Selection() = default;
    virtual SelectionType getSelectionType() const = 0;
    std::string getName() const { return SelectionTypeMethods::getName(getSelectionType()); }
    virtual std::vector<Point> getEnclosedPoints() const = 0;
    virtual std::vector<Point> getEnclosedPoints(int frame) const {
      if(!hasOffsets()) return getEnclosedPoints();
      std::vector<Point> originals = getEnclosedPoints();
      std::vector<Point> offsets;
      Point offset = getOffset(frame);
      uint16_t count = originals.size();
      for(uint16_t i = 0; i < count; i++)
        offsets.push_back(originals[i] + offset);
      return offsets;
    }
    virtual bool enclosesPoint(int x,int y) const = 0;
    virtual bool enclosesPoint(int x, int y, int frame) {
      if(!hasOffsets()) return enclosesPoint(x,y);
      Point offset = getOffset(frame);
      return enclosesPoint(x - offset.x, y - offset.y);
    }
    virtual Selection* copy() const = 0;
    virtual Point getCenter() const = 0;
    inline void setHovered(bool enabled=true) { hovered_ = enabled; }
    inline bool hovered() const { return hovered_; }

    void setOffset(int frame, Point offset) {
      offsets_[frame] = offset;
    }
    void setCurrentFrame(int frame) {
      currentFrame_ = frame;
    }
    void clearOffsets() {
      offsets_.clear();
    }
    inline bool operator==(const Selection& other) {
      return offsets_ == other.offsets_;
    }
    inline bool operator!=(const Selection& other) {
      return !(*this == other);
    }
};
