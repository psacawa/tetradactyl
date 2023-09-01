#pragma once

#include <string>

using std::string;

namespace Tetradactyl {

class Config {
public:
  static Config fromConfigFile(string filename);

  string hintChars;
  bool addToSystemTray;
};
} // namespace Tetradactyl
