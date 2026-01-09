#include "ThingsBoardClient.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

// Logger helper
void log(const std::string &msg) {
  std::cout << msg << std::endl;
  std::ofstream outfile;
  outfile.open("rpc_test_log.txt",
               std::ios_base::app); // append instead of overwrite
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  outfile << std::ctime(&now_c) << ": " << msg << "\n";
}

void setLed01(const JsonDocument &data, JsonDocument &response) {
  log("RPC Method setLed01 called");
  log("Data: " + data.to_string());
  response = data; // Echo back
}

void setLed02(const JsonDocument &data, JsonDocument &response) {
  log("RPC Method setLed02 called");
  log("Data: " + data.to_string());
  response = data;
}

void setLed03(const JsonDocument &data, JsonDocument &response) {
  log("RPC Method setLed03 called");
  log("Data: " + data.to_string());
  response = data;
}

void setLed04(const JsonDocument &data, JsonDocument &response) {
  log("RPC Method setLed04 called");
  log("Data: " + data.to_string());
  response = data;
}

int main(int argc, char *argv[]) {
  // Check arguments
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <access_token> [host] [port]"
              << std::endl;
    return 1;
  }

  std::string token = argv[1];
  std::string host = (argc > 2) ? argv[2] : "demo.thingsboard.io";
  int port = (argc > 3) ? std::stoi(argv[3]) : 1883;

  log("Starting ThingsBoard RPC Example");
  log("Connecting to " + host + ":" + std::to_string(port));

  try {
    ThingsBoardClient tb(token, host, port);

    // Register RPC callbacks
    tb.RPCRoute("setLed01", setLed01);
    tb.RPCRoute("setLed02", setLed02);
    tb.RPCRoute("setLed03", setLed03);
    tb.RPCRoute("setLed04", setLed04);

    tb.connect();

    log("Connected to ThingsBoard. Waiting for RPC commands...");

    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    tb.disconnect();
  } catch (const mqtt::exception &mex) {
    log("MQTT Error: " + std::string(mex.what()));
    log("Return Code: " + std::to_string(mex.get_return_code()));
  } catch (const std::exception &e) {
    log(std::string("Error: ") + e.what());
  }

  return 0;
}
