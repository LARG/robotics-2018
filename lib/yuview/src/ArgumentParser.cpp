#include <ArgumentParser.h>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

namespace yuview {
  using namespace boost;
  using namespace boost::program_options;
  using namespace std;

  std::istream& operator>>(std::istream& in, ArgumentParser::Action& action) {
    std::string token;
    in >> token;
    if(token == "Convert")
      action = ArgumentParser::Action::Convert;
    else if(token == "View")
      action = ArgumentParser::Action::View;
    else throw validation_error(validation_error::invalid_option_value, token, "action");
    return in;
  }

  ArgumentParser::Arguments ArgumentParser::Parse(int argc, char **argv) {
    Arguments args;

    options_description general("Options");
    general.add_options()
      ("help,h", "Show this help message")
      ("width", value(&args.width)->default_value(-1), "The width of the input image.")
      ("height", value(&args.height)->default_value(-1), "The height of the input image.")
      ("size", value(&args.size)->default_value(-1), "The size of the output image.")
      ("target", value(&args.target), "The target (output) path.")
      ("verbose,v", bool_switch(&args.verbose)->default_value(false), "Verbose mode.")
      ("raw", bool_switch(&args.raw)->default_value(false), "Read a raw YUV buffer dump instead of a serialized YUV file.");
      ("action", value<ArgumentParser::Action>(&args.action)->multitoken()->default_value(Action::View), "View or Convert")
    ;

    options_description hidden("Hidden Options");
    hidden.add_options()
      ("source", value(&args.source), "The source (input) path.")
      ("others", value<vector<string>>(), "Other arguments that will not be processed.")
    ;
    
    positional_options_description pos;
    pos.add("source", 1).add("others", -1);

    options_description combined;
    combined.add(general).add(hidden);

    variables_map vm;
    store(command_line_parser(argc, argv).options(combined).positional(pos).run(), vm);
    notify(vm);

    if(vm.count("help")) {
      cout << "Usage: yuview [options] [SOURCE_IMAGE]\n";
      cout << "\n";
      cout << general << "\n";
      exit(0);
    }
    if(vm.count("source")) {
      args.source = vm["source"].as<string>();
      if(args.target.length() == 0) args.action = Action::View;
    }
    if(args.target.length() > 0) {
      args.action = Action::Convert;
    }
    if(args.raw) {
      if(args.width < 0 || args.height < 0) {
        fprintf(stderr, "ERROR: You must supply the image dimensions when reading raw YUV data.\n");
        exit(1);
      }
    }
    return args;
  }
}
