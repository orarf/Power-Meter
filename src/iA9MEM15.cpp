#include "iA9MEM15.h"
#include <ModbusPort.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

std::unique_ptr<iA9MEM15>
iA9MEM15::createClient(uint8_t unitId,
                       const std::string &ipAddress,
                       int port,
                       int timeout) {
  Modbus::TcpSettings settings;
  settings.host = ipAddress.c_str();
  settings.port = port;
  settings.timeout = timeout;

  ModbusClientPort *rawPort =
      Modbus::createClientPort(Modbus::TCP, &settings, true);
  if (!rawPort)
    throw std::runtime_error("Failed to create Modbus port");

  auto status = rawPort->port()->open();
  if (Modbus::StatusIsBad(status))
    std::cerr << "Port open failed\n";

  std::shared_ptr<ModbusClientPort> portPtr(rawPort);
  std::shared_ptr<ModbusClient> clientPtr(
      new ModbusClient(unitId, rawPort));

  return std::make_unique<iA9MEM15>(portPtr, clientPtr);
}

iA9MEM15::iA9MEM15(std::shared_ptr<ModbusClientPort> port,
                   std::shared_ptr<ModbusClient> client)
    : m_port(port), m_client(client) {}

iA9MEM15::~iA9MEM15() { Disconnect(); }

bool iA9MEM15::isConnected() const {
  return m_client && m_client->isOpen();
}

void iA9MEM15::Disconnect() {
  if (m_port)
    m_port->close();
}

// ================= LOW LEVEL =================

uint16_t iA9MEM15::readU16(uint16_t address) {
  uint16_t val = 0;
  auto status = m_client->readHoldingRegisters(address, 1, &val);
  return Modbus::StatusIsGood(status) ? val : 0;
}

uint32_t iA9MEM15::readU32(uint16_t address) {
  uint16_t r[2] = {0};
  auto status = m_client->readHoldingRegisters(address, 2, r);
  if (!Modbus::StatusIsGood(status))
    return 0;

  // Big-endian WORD
  return (uint32_t(r[0]) << 16) | r[1];
}

float iA9MEM15::readFloat(uint16_t address) {
  uint32_t raw = readU32(address);
  float f;
  std::memcpy(&f, &raw, sizeof(f));
  return f;
}

uint64_t iA9MEM15::readU64(uint16_t address) {
  uint16_t r[4] = {0};
  auto status = m_client->readHoldingRegisters(address, 4, r);
  if (!Modbus::StatusIsGood(status))
    return 0;

  // Big-endian WORD (Schneider iA9 MEM15)
  uint64_t v = 0;
  v |= uint64_t(r[0]) << 48;
  v |= uint64_t(r[1]) << 32;
  v |= uint64_t(r[2]) << 16;
  v |= uint64_t(r[3]);
  return v;
}

// ================= HIGH LEVEL =================
// (address สมมติ – แก้ตาม datasheet จริงได้)

float iA9MEM15::Read_RmsCurrentOnPhaseA() {
  return readFloat(2999);
}

float iA9MEM15::Read_RmsPhasetoneutralVoltageAn() {
  return readFloat(3019);
}

float iA9MEM15::Read_ActivePowerOnPhaseA() {
  return readFloat(3053);
}

float iA9MEM15::Read_TotalActivePower() {
  return readFloat(3059);
}

float iA9MEM15::Read_TotalApparentPowerArithmetic() {
  return readFloat(3069);
}

float iA9MEM15::Read_TotalPowerFactor() {
  return readFloat(3079);
}

float iA9MEM15::Read_DeviceInternalTemperature() {
  return readFloat(3099);
}

uint64_t iA9MEM15::Read_TotalActiveEnergyDelivered_NotResettable() {
  return readU64(3203);
}
