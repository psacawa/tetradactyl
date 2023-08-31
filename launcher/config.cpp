#include <toml/get.hpp>
#include <toml/parser.hpp>

#include "config.h"

using std::string;
namespace Tetradactyl
{
  
  Config Config::fromConfigFile(string filename) {
    const auto data = toml::parse(filename);
    Config config;
    config.hintChars =
        toml::find_or<string>(data, "hintChars", "hjklasdfgyuiopqwertnmzxcvb");
    config.addToSystemTray = toml::find_or<bool>(data, "addToSystemTray", true);
    return config;
  }
} /* Tetradactyl */ 
