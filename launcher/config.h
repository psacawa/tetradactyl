#include <string>

using std::string;

class TetradactylConfig {
public:
  static TetradactylConfig fromConfigFile(string filename);

  string hintChars;
  bool addToSystemTray;
};
