#pragma once
#include <common/Serialization.h>
#include <math/Geometry.h>

DECLARE_EXTERNAL_SCHEMA(class LineSegment {
  SCHEMA_FIELD(float start_x);
  SCHEMA_FIELD(float end_x);
  SCHEMA_FIELD(float start_y);
  SCHEMA_FIELD(float end_y);
  SCHEMA_FIELD(float m_a);
  SCHEMA_FIELD(float m_b);
  SCHEMA_FIELD(float m_c);
  SCHEMA_SERIALIZATION({
    schema::LineSegmentBuilder __builder__(*__serializer__);
    const auto& line = __source_object__;
    __builder__.add_start_x(line.start.x);
    __builder__.add_end_x(line.end.x);
    __builder__.add_start_y(line.start.y);
    __builder__.add_end_y(line.end.y);
    __builder__.add_m_a(line.m_a);
    __builder__.add_m_b(line.m_b);
    __builder__.add_m_c(line.m_c);
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& line = __target_object__;
    line.start.x = data->start_x();
    line.end.x = data->end_x();
    line.start.y = data->start_y();
    line.end.y = data->end_y();
    line.m_a = data->m_a();
    line.m_b = data->m_b();
    line.m_c = data->m_c();
  });
});
