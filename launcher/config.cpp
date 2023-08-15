#include <string>
#include <toml.hpp>
#include <toml/get.hpp>
#include <toml/parser.hpp>

#include "config.h"

using std::string;

TetradactylConfig TetradactylConfig::fromConfigFile(string filename) {
  const auto data = toml::parse(filename);
  TetradactylConfig config;
  config.hintChars =
      toml::find_or<string>(data, "hintChars", "hjklasdfgyuiopqwertnmzxcvb");
  config.addToSystemTray = toml::find_or<bool>(data, "addToSystemTray", true);
  return config;
}
