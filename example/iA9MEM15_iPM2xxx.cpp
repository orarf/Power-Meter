#include "iA9MEM15.h"
#include "iPM2xxx.h"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
  std::cout << "Starting Direct Read Monitor (iPM2xxx)..." << std::endl;

  // Connection for iPM2xxx
  std::unique_ptr<iPM2xxx> client;
  for (int j = 0; j < 5; j++) {
    client = iPM2xxx::createClient(1, "192.168.100.28", 502);
    if (client->isConnected()) {
      std::cout << "Port Opened Successfully (iPM2xxx)." << std::endl;
      break;
    }
    std::cerr << "Failed to open port. Retrying in 2s..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  if (client && client->isConnected()) {
    // Direct Read Loop
    for (int i = 0; i < 2; ++i) {
      std::cout << "----------------------------------------" << std::endl;
      std::cout << "Reading iPM2xxx (Attempt " << i + 1 << ")..." << std::endl;

      float powerA = client->Read_ActivePowerA();
      float powerTotal = client->Read_ActivePowerTotal();
      float pfTotal = client->Read_PowerFactorTotal();

      std::cout << "Active Power A:     " << powerA << " kW" << std::endl;
      std::cout << "Active Power Total: " << powerTotal << " kW" << std::endl;
      std::cout << "Power Factor Total: " << pfTotal << std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    client->Disconnect();
    std::cout << "Disconnected iPM2xxx." << std::endl;
  }

  std::cout << "\nStarting Direct Read Monitor (iA9MEM15)..." << std::endl;
  // Connection for iA9MEM15
  std::unique_ptr<iA9MEM15> clientA9;
  for (int j = 0; j < 5; j++) {
    clientA9 = iA9MEM15::createClient(100, "192.168.100.28", 502);
    if (clientA9->isConnected()) {
      std::cout << "Port Opened Successfully (iA9MEM15)." << std::endl;
      break;
    }
    std::cerr << "Failed to open port. Retrying in 2s..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  if (clientA9 && clientA9->isConnected()) {
    // Direct Read Loop
    for (int i = 0; i < 2; ++i) {
      std::cout << "----------------------------------------" << std::endl;
      std::cout << "Reading iA9MEM15 (Attempt " << i + 1 << ")..." << std::endl;

      // Using generated names - verify if these matched the CSV descriptions
      // CSV: "Active power on phase A", "Active power on phase B", "Active
      // power on phase C"
      float powerA = clientA9->Read_ActivePowerOnPhaseA();
      float powerB = clientA9->Read_ActivePowerOnPhaseB();
      float powerC = clientA9->Read_ActivePowerOnPhaseC();

      std::cout << "Active Power Phase A: " << powerA << " W" << std::endl;
      std::cout << "Active Power Phase B: " << powerB << " W" << std::endl;
      std::cout << "Active Power Phase C: " << powerC << " W" << std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    clientA9->Disconnect();
    std::cout << "Disconnected iA9MEM15." << std::endl;
  }

  return 0;
}
