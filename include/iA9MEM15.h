#ifndef IA9MEM15_H
#define IA9MEM15_H

#include <ModbusClient.h>
#include <ModbusClientPort.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class iA9MEM15 {
public:
  static std::unique_ptr<iA9MEM15>
  createClient(uint8_t unitId,
               const std::string &ipAddress,
               int port = 502,
               int timeout = 2000);

  iA9MEM15(std::shared_ptr<ModbusClientPort> port,
           std::shared_ptr<ModbusClient> client);
  ~iA9MEM15();

  bool isConnected() const;
  void Disconnect();

  // ===== High-level Read =====
  float Read_RmsCurrentOnPhaseA();
  float Read_RmsPhasetoneutralVoltageAn();
  float Read_ActivePowerOnPhaseA();
  float Read_TotalActivePower();
  float Read_TotalApparentPowerArithmetic();
  float Read_TotalPowerFactor();
  float Read_DeviceInternalTemperature();
  uint64_t Read_TotalActiveEnergyDelivered_NotResettable();

private:
  std::shared_ptr<ModbusClientPort> m_port;
  std::shared_ptr<ModbusClient> m_client;

  // ===== Low-level helpers =====
  uint16_t readU16(uint16_t address);
  uint32_t readU32(uint16_t address);
  float readFloat(uint16_t address);
  uint64_t readU64(uint16_t address);
};

#endif // IA9MEM15_H
