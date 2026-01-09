#include "PM2xxx.h"
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

int main() {
  std::cout << "PM2xxx Monitor started." << std::endl;

  try {
    // Configuration
    std::string ip = "192.168.100.28";
    int port = 502;
    uint8_t unitId = 1;

    // Connection
    // Now returns std::unique_ptr<PM2xxx>
    auto client = PM2xxx::createClient(1, "192.168.100.28", 502);

    // Wait for connection and initial poll
    std::cout << "Waiting for initial data..." << std::endl;
    while (!client->isConnected()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Connected and Data Ready!" << std::endl;

    // Read Power Values once (retrieved from cache instantly)
    while (true) {
      std::cout << "----------------------------------------" << std::endl;
      // Timestamp to show update
      auto now = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      std::cout << "Update: " << std::ctime(&now);

      std::cout << "--- Active Power (kW) ---" << std::endl;
      std::cout << "Active Power A:     " << client->Read_ActivePowerA()
                << std::endl;
      std::cout << "Active Power B:     " << client->Read_ActivePowerB()
                << std::endl;
      std::cout << "Active Power C:     " << client->Read_ActivePowerC()
                << std::endl;
      std::cout << "Active Power Total: " << client->Read_ActivePowerTotal()
                << std::endl;

      std::cout << "--- Reactive Power (kVAR) ---" << std::endl;
      std::cout << "Reactive Power A:   " << client->Read_ReactivePowerA()
                << std::endl;
      std::cout << "Reactive Power B:   " << client->Read_ReactivePowerB()
                << std::endl;
      std::cout << "Reactive Power C:   " << client->Read_ReactivePowerC()
                << std::endl;
      // Note: Total Reactive Power might be Total, check consistency
      std::cout << "Reactive Power Tot: " << client->Read_ReactivePowerTotal()
                << std::endl;

      std::cout << "--- Apparent Power (kVA) ---" << std::endl;
      std::cout << "Apparent Power A:   " << client->Read_ApparentPowerA()
                << std::endl;
      std::cout << "Apparent Power B:   " << client->Read_ApparentPowerB()
                << std::endl;
      std::cout << "Apparent Power C:   " << client->Read_ApparentPowerC()
                << std::endl;
      std::cout << "Apparent Power Tot: " << client->Read_ApparentPowerTotal()
                << std::endl;

      std::cout << "--- Power Factor ---" << std::endl;
      std::cout << "PF A:               " << client->Read_PowerFactorA()
                << std::endl;
      std::cout << "PF B:               " << client->Read_PowerFactorB()
                << std::endl;
      std::cout << "PF C:               " << client->Read_PowerFactorC()
                << std::endl;
      std::cout << "PF Total:           " << client->Read_PowerFactorTotal()
                << std::endl;

      std::cout << "--- Displacement PF ---" << std::endl;
      std::cout << "Disp PF A:          "
                << client->Read_DisplacementPowerFactorA() << std::endl;
      std::cout << "Disp PF B:          "
                << client->Read_DisplacementPowerFactorB() << std::endl;
      std::cout << "Disp PF C:          "
                << client->Read_DisplacementPowerFactorC() << std::endl;
      std::cout << "Disp PF Total:      "
                << client->Read_DisplacementPowerFactorTotal() << std::endl;

      // Simple loop delay or break
      std::this_thread::sleep_for(std::chrono::seconds(5));

      // --- Store JSON in Variable ---
      std::stringstream jsonStream;
      jsonStream << "{\n";
      // Write raw unix timestamp
      jsonStream << "  \"timestamp\": " << (long)now << ",\n";

      jsonStream << "  \"active_power\": {\n";
      jsonStream << "    \"phase_a\": " << client->Read_ActivePowerA() << ",\n";
      jsonStream << "    \"phase_b\": " << client->Read_ActivePowerB() << ",\n";
      jsonStream << "    \"phase_c\": " << client->Read_ActivePowerC() << ",\n";
      jsonStream << "    \"total\": " << client->Read_ActivePowerTotal()
                 << "\n";
      jsonStream << "  },\n";

      jsonStream << "  \"reactive_power\": {\n";
      jsonStream << "    \"phase_a\": " << client->Read_ReactivePowerA()
                 << ",\n";
      jsonStream << "    \"phase_b\": " << client->Read_ReactivePowerB()
                 << ",\n";
      jsonStream << "    \"phase_c\": " << client->Read_ReactivePowerC()
                 << ",\n";
      jsonStream << "    \"total\": " << client->Read_ReactivePowerTotal()
                 << "\n";
      jsonStream << "  },\n";

      jsonStream << "  \"apparent_power\": {\n";
      jsonStream << "    \"phase_a\": " << client->Read_ApparentPowerA()
                 << ",\n";
      jsonStream << "    \"phase_b\": " << client->Read_ApparentPowerB()
                 << ",\n";
      jsonStream << "    \"phase_c\": " << client->Read_ApparentPowerC()
                 << ",\n";
      jsonStream << "    \"total\": " << client->Read_ApparentPowerTotal()
                 << "\n";
      jsonStream << "  },\n";

      jsonStream << "  \"power_factor\": {\n";
      jsonStream << "    \"phase_a\": " << client->Read_PowerFactorA() << ",\n";
      jsonStream << "    \"phase_b\": " << client->Read_PowerFactorB() << ",\n";
      jsonStream << "    \"phase_c\": " << client->Read_PowerFactorC() << ",\n";
      jsonStream << "    \"total\": " << client->Read_PowerFactorTotal()
                 << "\n";
      jsonStream << "  },\n";

      jsonStream << "  \"displacement_power_factor\": {\n";
      jsonStream << "    \"phase_a\": "
                 << client->Read_DisplacementPowerFactorA() << ",\n";
      jsonStream << "    \"phase_b\": "
                 << client->Read_DisplacementPowerFactorB() << ",\n";
      jsonStream << "    \"phase_c\": "
                 << client->Read_DisplacementPowerFactorC() << ",\n";
      jsonStream << "    \"total\": "
                 << client->Read_DisplacementPowerFactorTotal() << "\n";
      jsonStream << "  }\n";

      jsonStream << "}\n";

      std::string jsonVariable = jsonStream.str();
      // For demonstration, print the size or the variable itself
      std::cout << "[JSON Variable Updated. Size: " << jsonVariable.size()
                << " bytes]" << std::endl;
      // std::cout << jsonVariable << std::endl; // Uncomment to see full JSON
    }

    client->Disconnect();

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}