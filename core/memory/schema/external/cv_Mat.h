#pragma once

#include <common/Serialization.h>
#include <opencv2/core/core.hpp>
#include <cstddef>

DECLARE_EXTERNAL_SCHEMA(class cv::Mat {
  SCHEMA_FIELD(std::vector<uint8_t> data);
  SCHEMA_FIELD(int rows);
  SCHEMA_FIELD(int cols);
  SCHEMA_SERIALIZATION({
    // TODO: Enable more dimensions
    const auto& so = __source_object__;
    const unsigned char* data_ptr = so.ptr();
    ::std::size_t sz = so.total();
    auto data_alloc = __serializer__->CreateVector(data_ptr, sz);
    schema::cv::MatBuilder __builder__(*__serializer__);
    __builder__.add_data(data_alloc);
    __builder__.add_rows(so.rows);
    __builder__.add_cols(so.cols);
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& p = __target_object__;
    p = ::cv::Mat(data->rows(), data->cols(), CV_8UC1);
    for(int j = 0; j < p.rows; j++)
      for(int i = 0; i < p.cols; i++)
        p.at<unsigned char>(j, i) = data->data()->Get(j * p.cols + i);
    //p = ::cv::Mat(
    //    ::cv::Size(data->cols(), data->rows()), 
    //    CV_8UC1, 
    //    data->mutable_data()->data()
    //  );
  });
});
