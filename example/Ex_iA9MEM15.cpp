#include "Read_iA9MEM15.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <sstream>

int main() {
  std::string ipAddr = "127.0.0.1";
  int port = 502;
  std::vector<int> deviceIds;

  // Modbus IP
  if (const char* env_p = std::getenv("MODBUS_IP")) {
    ipAddr = env_p;
  }

  // Modbus Port
  if (const char* env_p = std::getenv("MODBUS_PORT")) {
    port = std::atoi(env_p);
  }

  // Device IDs (comma separated)
  if (const char* env_p = std::getenv("DEVICE_IDS")) {
    std::stringstream ss(env_p);
    std::string item;
    while (std::getline(ss, item, ',')) {
      try {
        deviceIds.push_back(std::stoi(item));
      } catch (...) {}
    }
  }

  if (deviceIds.empty()) {
    std::cerr << "No DEVICE_IDS found in environment, using default 100" << std::endl;
    deviceIds = {100};
  }

  while (true) {
    Read_iA9MEM15(deviceIds, ipAddr, port);
    std::cout << "Waiting 60 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
  return 0;
}