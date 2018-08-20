#ifndef IMAGE_PARAMS_H
#define IMAGE_PARAMS_H

#include <common/Serialization.h>
#include <schema/gen/ImageParams_generated.h>
#include <common/Camera.h>

DECLARE_INTERNAL_SCHEMA(struct ImageParams {
  public:
    SCHEMA_METHODS(ImageParams);
    SCHEMA_FIELD(int width);
    SCHEMA_FIELD(int height);
    SCHEMA_FIELD(int size);
    SCHEMA_FIELD(int rawSize);
    SCHEMA_FIELD(int defaultHorizontalStepScale);
    SCHEMA_FIELD(int defaultVerticalStepScale);
    SCHEMA_FIELD(int factor);
    SCHEMA_FIELD(float origFactor);
    ImageParams(Camera::Type camera);
    static ImageParams Empty;
  private:
    ImageParams();
});

#endif
