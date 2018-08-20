#include <memory/RobotStateBlock.h>
#include <math/Pose3D.h>
#include <stdio.h>
#include <iostream>
#include <schema/gen/Pose3D.h>
  
std::string to_json(const auto& data, const char* schema_source, auto serialize) {
  std::string file;
  if(!flatbuffers::LoadFile(schema_source, false, &file)) {
    printf("Error loading block schema from '%s'\n", schema_source);
    exit(1);
  }
  flatbuffers::Parser parser;
  const char* include_paths[] = { SCHEMA_DIR, nullptr };
  if(!parser.Parse(file.c_str(), include_paths)) {
    printf("Error parsing block schema: '%s'\n", schema_source);
    exit(1);
  }
  auto serializer = serialization::create_serializer();
  auto location = serialize(serializer, data);
  serializer->Finish(location);

  std::string jsgen;
  GenerateText(parser, serializer->GetBufferPointer(), &jsgen);
  return jsgen;
}

int main(int argc, char** argv) {
  auto pose = Pose3D();
  pose.translation.x = 3;
  pose.translation.y = 4;
  pose.translation.z = 5;

  pose.rotation.rotateX(M_PI / 3);

  std::cout << pose;


  // Create a serializer
  auto serializer = serialization::create_serializer();

  // Serialize the block instance
  auto pose_location = schema::serialize(serializer, pose);
  serializer->Finish(pose_location);

  // Deserialize the data to a new robot instance
  Pose3D target;
  std::cout << target;
  schema::deserialize(serializer->GetBufferPointer(), target);
  std::cout << target;
}
