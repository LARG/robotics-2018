#pragma once

#include <common/annotations/Selection.h>
#include <vector>

//TODO: Just store x, y, width, height
class RectangleSelection : public Selection {
  public:
    RectangleSelection() = default;
    virtual ~RectangleSelection() = default;
    RectangleSelection(Point tl, Point br){
      init(tl,br);
    }
    inline auto x() const { return topleft.x; }
    inline auto y() const { return topleft.y; }
    inline auto width() const { return bottomright.x - topleft.x; }
    inline auto height() const { return bottomright.y - topleft.y; }
    inline auto getTopLeft() const { return topleft + getOffset(); }
    inline auto getBottomRight() const { return bottomright + getOffset(); }
    inline virtual SelectionType getSelectionType() const override {
      return SelectionType::Rectangle;
    }
    std::vector<Point> getEnclosedPoints() const final;
    bool enclosesPoint(int x, int y) const final;
    virtual Selection* copy() const override;
    Point getCenter() const final;
  
  protected:
    Point topleft, bottomright;
    void init(Point tl, Point br);

  private:
    void serialize(YAML::Emitter& emitter) const final;
    void deserialize(const YAML::Node& node) final;
};
