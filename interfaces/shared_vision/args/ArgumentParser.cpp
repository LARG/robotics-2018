#include "ArgumentParser.h"
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <iostream>

Arguments ArgumentParser::Parse(int argc, char **argv) {
  using namespace boost;
  using namespace boost::program_options;
  using namespace std;
  Arguments args;

  options_description options("Options");
  options.add_options()
    ("help,h", "Show this help message")
    ("python-debug", bool_switch(&args.python_debug)->default_value(false), "Disable python optimizations and enable state machine printing and trace output.")
  ;
  
  variables_map vm;
  store(command_line_parser(argc, argv).options(options).run(), vm);
  notify(vm);

  if(vm.count("help")) {
    cout << "Usage: vision [options]\n";
    cout << "\n";
    cout << options << "\n";
    exit(0);
  }
  return args;
}
