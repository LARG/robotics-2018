#include <string>
#include <cstdio>
#include <cstdlib>
#include <YUVImage.h>
#include <Common.h>
#include <ArgumentParser.h>
#include <Util.h>
#include <FileNavigator.h>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#define ENABLE_DEBUG false

using namespace std;
using namespace yuview;

int main(int argc, char **argv) {
  if(ENABLE_DEBUG) {
    Util::mkdirp(HOME_DIRECTORY "/.debug");
    std::ofstream debugger(HOME_DIRECTORY "/.debug/output", std::ofstream::out | std::ofstream::app);
    debugger << "------------------\n";
    for(int i = 0; i < argc; i++) {
      debugger << i << "]: " << argv[i] << "\n";
    }
  }
  auto args = ArgumentParser::Parse(argc, argv);
 
  try {
    if(args.verbose) printf("Reading from source: %s\n", args.source.c_str());
    YUVImage image;
    std::unique_ptr<FileNavigator> nav;
    int keyCode;
    do {
      if(args.raw)
        image = YUVImage::ReadRawBuffer(args.source, args.width, args.height);
      else
        image = YUVImage::ReadSerializedObject(args.source);
      if(args.action == ArgumentParser::View) {
        nav = std::make_unique<FileNavigator>(args.source);
        image.show();
        keyCode = cv::waitKey();
        args.source = nav->nextPath(keyCode);
      }
    } while(nav != nullptr && !nav->quit(keyCode));
    if(nav != nullptr) {
      return EXIT_SUCCESS;
    }
    auto target = args.target;
    if(!endswith(target, ".png") && !endswith(target, ".jpg") && !endswith(target, ".bmp")) {
      target += ".png";
    }
    if(args.verbose) printf("Converting to %ix%i target: %s\n", args.size, args.size, target.c_str());
    if(args.size > 0)
      image.convert(target, args.size, args.size);
    else image.convert(target);
    if(target != args.target) {
      std::ifstream s(target, std::fstream::binary);
      std::ofstream t(args.target, std::fstream::trunc|std::fstream::binary);
      t << s.rdbuf();
    }
  } catch(std::exception& e) {
    if(ENABLE_DEBUG) {
      std::ofstream debugger(HOME_DIRECTORY "/.debug/output", std::ofstream::out | std::ofstream::app);
      debugger << e.what() << "\n";
    }
  }
  return EXIT_SUCCESS;
}
