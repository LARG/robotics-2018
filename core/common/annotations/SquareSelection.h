#pragma once

#include <common/annotations/RectangleSelection.h>

class SquareSelection : public RectangleSelection {
  public:
    using RectangleSelection::RectangleSelection;
    virtual SelectionType getSelectionType() const override {
      return SelectionType::Square;
    }
    virtual Selection* copy() const override {
      SquareSelection* r = new SquareSelection();
      r->topleft = this->topleft;
      r->bottomright = this->bottomright;
      return r;
    }

};
