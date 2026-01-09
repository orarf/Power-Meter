#ifndef ENV_H
#define ENV_H

#include <cstdlib>
#include <fstream>
#include <string>

inline void loadEnv() {
  std::ifstream file(".env");
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#')
        continue;
      size_t pos = line.find('=');
      if (pos != std::string::npos) {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        setenv(key.c_str(), value.c_str(), 1);
      }
    }
  }
}

#endif // ENV_H