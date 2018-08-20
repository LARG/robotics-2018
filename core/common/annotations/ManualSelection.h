#pragma once

#include <common/annotations/PolygonSelection.h>

class ManualSelection : public PolygonSelection {
  public:
    using PolygonSelection::PolygonSelection;
    virtual SelectionType getSelectionType() const override {
      return SelectionType::Manual;
    }
    virtual Selection* copy() const override {
      ManualSelection* p = new ManualSelection();
      int count = vertices_.size();
      for(int i = 0; i < count; i++)
        p->vertices_.push_back(vertices_[i]);
      return p;
    }

};
