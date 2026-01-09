#include "iPM2xxx.h"
#include <ModbusPort.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

std::unique_ptr<iPM2xxx> iPM2xxx::createClient(uint8_t unitId,
                                               const std::string &ipAddress,
                                               int port, int timeout) {
  Modbus::TcpSettings settings;
  settings.host = ipAddress.c_str();
  settings.port = port;
  settings.timeout = timeout;

  ModbusClientPort *rawPort =
      Modbus::createClientPort(Modbus::TCP, &settings, true);
  if (!rawPort) {
    throw std::runtime_error("Failed to create Modbus client port");
  }

  // Explicitly open the port
  Modbus::StatusCode status = rawPort->port()->open();
  if (Modbus::StatusIsBad(status)) {
    std::cerr << "[DEBUG] Port open failed. Status: " << status
              << " Error: " << rawPort->port()->lastErrorText() << std::endl;
    // We continue, returning the object, so calling code can check
    // isConnected() and see false
  }

  std::shared_ptr<ModbusClientPort> sharedPort(rawPort);

  ModbusClient *rawClient = new ModbusClient(unitId, rawPort);
  std::shared_ptr<ModbusClient> sharedClient(rawClient);

  return std::make_unique<iPM2xxx>(sharedPort, sharedClient);
}

iPM2xxx::iPM2xxx(std::shared_ptr<ModbusClientPort> port,
                 std::shared_ptr<ModbusClient> client)
    : m_port(port), m_client(client) {}

iPM2xxx::~iPM2xxx() { Disconnect(); }

bool iPM2xxx::isConnected() const {
  return m_client != nullptr && m_client->isOpen();
}

void iPM2xxx::Disconnect() {
  if (m_port) {
    m_port->close();
  }
}

// ---------------- Helpers ----------------
uint16_t iPM2xxx::readU16(uint16_t address) {
  uint16_t val = 0;
  auto status = m_client->readHoldingRegisters(address, 1, &val);
  return Modbus::StatusIsGood(status) ? val : 0;
}

uint32_t iPM2xxx::readU32(uint16_t address) {
  uint16_t r[2] = {0};
  auto status = m_client->readHoldingRegisters(address, 2, r);
  if (!Modbus::StatusIsGood(status))
    return 0;

  // Big-endian WORD
  return (uint32_t(r[0]) << 16) | r[1];
}

float iPM2xxx::readFloat(uint16_t address) {
  uint32_t raw = readU32(address);
  float f;
  std::memcpy(&f, &raw, sizeof(f));
  return f;
}

uint64_t iPM2xxx::readU64(uint16_t address) {
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

std::string iPM2xxx::readString(uint16_t address, uint16_t length) {
  std::vector<uint16_t> buff(length);
  Modbus::StatusCode status =
      m_client->readHoldingRegisters(address, length, buff.data());
  if (!Modbus::StatusIsGood(status))
    return "";

  std::string s;
  s.reserve(length * 2);
  for (uint16_t r : buff) {
    char high = (char)(r >> 8);
    char low = (char)(r & 0xFF);
    if (high != 0)
      s.push_back(high);
    if (low != 0)
      s.push_back(low);
  }
  return s;
}

// ---------------- Generated Methods ----------------
// Meter Name (Addr: 29)
std::string iPM2xxx::Read_MeterName() { return readString(29, 20); }

// Meter Model (Addr: 49)
std::string iPM2xxx::Read_MeterModel() { return readString(49, 20); }

// Manufacturer (Addr: 69)
std::string iPM2xxx::Read_Manufacturer() { return readString(69, 20); }

// Product ID Number (Addr: 89)
uint16_t iPM2xxx::Read_ProductIdNumber() { return readU16(89); }

// Hardware Detected (Addr: 90)
uint16_t iPM2xxx::Read_HardwareDetected() { return readU16(90); }

// Supported Features (Addr: 94)
uint16_t iPM2xxx::Read_SupportedFeatures() { return readU16(94); }

// Unused (Addr: 98)
uint16_t iPM2xxx::Read_Unused() { return readU16(98); }

// Installed Option � Slot A (Addr: 399)
uint16_t iPM2xxx::Read_InstalledOptionSlotA() { return readU16(399); }

// Serial Number (Addr: 402)
uint16_t iPM2xxx::Read_SerialNumber() { return readU16(402); }

// Date of Manufacture (Addr: 404)
uint16_t iPM2xxx::Read_DateOfManufacture() { return readU16(404); }

// Hardware Revision (Addr: 408)
std::string iPM2xxx::Read_HardwareRevision() { return readString(408, 5); }

// FW version (Addr: 413)
uint16_t iPM2xxx::Read_FwVersion() { return readU16(413); }

// Present Firmware Version (DLF Format) X.Y.T (Addr: 1636)
uint16_t iPM2xxx::Read_Read_FirmwareVer_XYT() {
  return readU16(1636);
}

// X � Major (Addr: 1637)
uint16_t iPM2xxx::Read_XMajor() { return readU16(1637); }

// Y � Minor (Addr: 1638)
uint16_t iPM2xxx::Read_YMinor() { return readU16(1638); }

// Z � Quality (Addr: 1639)
uint16_t iPM2xxx::Read_ZQuality() { return readU16(1639); }

// T � Internal evolutions (Addr: 1640)
uint16_t iPM2xxx::Read_TInternalEvolutions() { return readU16(1640); }

// Previous Firmware Version (DLF Format) X.Y.T (Addr: 1641)
uint16_t iPM2xxx::Read_Read_PrevFirmwareVer_XYT() {
  return readU16(1641);
}

// X � Major (Addr: 1642)
uint16_t iPM2xxx::Read_XMajor_1642() { return readU16(1642); }

// Y � Minor (Addr: 1643)
uint16_t iPM2xxx::Read_YMinor_1643() { return readU16(1643); }

// Z � Quality (Addr: 1644)
uint16_t iPM2xxx::Read_ZQuality_1644() { return readU16(1644); }

// T � Internal evolutions (Addr: 1645)
uint16_t iPM2xxx::Read_TInternalEvolutions_1645() { return readU16(1645); }

// Date/Time of Last Firmware Download (Addr: 1646)
uint16_t iPM2xxx::Read_DateTimeOfLastFirmwareDownload() {
  return readU16(1646);
}

// Present Firmware Version (DLF Format) X.Y.T (Addr: 1668)
uint16_t iPM2xxx::Read_Read_FirmwareVer_XYT_1668() {
  return readU16(1668);
}

// X � Major (Addr: 1669)
uint16_t iPM2xxx::Read_XMajor_1669() { return readU16(1669); }

// Y � Minor (Addr: 1670)
uint16_t iPM2xxx::Read_YMinor_1670() { return readU16(1670); }

// Z � Quality (Addr: 1671)
uint16_t iPM2xxx::Read_ZQuality_1671() { return readU16(1671); }

// T � Internal evolutions (Addr: 1672)
uint16_t iPM2xxx::Read_TInternalEvolutions_1672() { return readU16(1672); }

// Previous Firmware Version (DLF Format) X.Y.T (Addr: 1673)
uint16_t iPM2xxx::Read_Read_PrevFirmwareVer_XYT_1673() {
  return readU16(1673);
}

// X � Major (Addr: 1674)
uint16_t iPM2xxx::Read_XMajor_1674() { return readU16(1674); }

// Y � Minor (Addr: 1675)
uint16_t iPM2xxx::Read_YMinor_1675() { return readU16(1675); }

// Z � Quality (Addr: 1676)
uint16_t iPM2xxx::Read_ZQuality_1676() { return readU16(1676); }

// T � Internal evolutions (Addr: 1677)
uint16_t iPM2xxx::Read_TInternalEvolutions_1677() { return readU16(1677); }

// Date/Time of Last Firmware Download (Addr: 1678)
uint16_t iPM2xxx::Read_DateTimeOfLastFirmwareDownload_1678() {
  return readU16(1678);
}

// Present Firmware Version (DLF Format) X.Y.T (Addr: 1700)
uint16_t iPM2xxx::Read_Read_FirmwareVer_XYT_1700() {
  return readU16(1700);
}

// X � Major (Addr: 1701)
uint16_t iPM2xxx::Read_XMajor_1701() { return readU16(1701); }

// Y � Minor (Addr: 1702)
uint16_t iPM2xxx::Read_YMinor_1702() { return readU16(1702); }

// Z � Quality (Addr: 1703)
uint16_t iPM2xxx::Read_ZQuality_1703() { return readU16(1703); }

// T � Internal evolutions (Addr: 1704)
uint16_t iPM2xxx::Read_TInternalEvolutions_1704() { return readU16(1704); }

// Previous Firmware Version (DLF Format) X.Y.T (Addr: 1705)
uint16_t iPM2xxx::Read_Read_PrevFirmwareVer_XYT_1705() {
  return readU16(1705);
}

// X � Major (Addr: 1706)
uint16_t iPM2xxx::Read_XMajor_1706() { return readU16(1706); }

// Y � Minor (Addr: 1707)
uint16_t iPM2xxx::Read_YMinor_1707() { return readU16(1707); }

// Z � Quality (Addr: 1708)
uint16_t iPM2xxx::Read_ZQuality_1708() { return readU16(1708); }

// T � Internal evolutions (Addr: 1709)
uint16_t iPM2xxx::Read_TInternalEvolutions_1709() { return readU16(1709); }

// Date/Time of Last Firmware Download (Addr: 1710)
uint16_t iPM2xxx::Read_DateTimeOfLastFirmwareDownload_1710() {
  return readU16(1710);
}

// Checksum (Addr: 1714)
uint16_t iPM2xxx::Read_Checksum() { return readU16(1714); }

// Bridge Code Version (Addr: 1715)
uint16_t iPM2xxx::Read_BridgeCodeVersion() { return readU16(1715); }

// Download � CRC of Last FW Download (Addr: 1746)
uint16_t iPM2xxx::Read_DownloadCrcOfLastFwDownload() { return readU16(1746); }

// Download � Flash Write Failure (Addr: 1747)
uint16_t iPM2xxx::Read_DownloadFlashWriteFailure() { return readU16(1747); }

// Download � % Complete (Addr: 1748)
uint16_t iPM2xxx::Read_DownloadComplete() { return readU16(1748); }

// Last Unit Restart DateTime (Addr: 1823)
uint16_t iPM2xxx::Read_LastUnitRestartDatetime() { return readU16(1823); }

// Number of Metering System Restarts (Addr: 1827)
uint16_t iPM2xxx::Read_NumberOfMeteringSystemRestarts() {
  return readU16(1827);
}

// Number of Control Power Failures (Addr: 1828)
uint16_t iPM2xxx::Read_NumberOfControlPowerFailures() { return readU16(1828); }

// Date/Time of Last Control Power Failure (Addr: 1829)
uint16_t iPM2xxx::Read_DateTimeOfLastControlPowerFailure() {
  return readU16(1829);
}

// Duration of Last Control Power Failure (Addr: 1833)
uint16_t iPM2xxx::Read_DurationOfLastControlPowerFailure() {
  return readU16(1833);
}

// Cause of Last Meter Reset (Addr: 1835)
uint16_t iPM2xxx::Read_CauseOfLastMeterReset() { return readU16(1835); }

// Year (Addr: 1836)
uint16_t iPM2xxx::Read_Year() { return readU16(1836); }

// Month (Addr: 1837)
uint16_t iPM2xxx::Read_Month() { return readU16(1837); }

// Day (Addr: 1838)
uint16_t iPM2xxx::Read_Day() { return readU16(1838); }

// Hour (Addr: 1839)
uint16_t iPM2xxx::Read_Hour() { return readU16(1839); }

// Minute (Addr: 1840)
uint16_t iPM2xxx::Read_Minute() { return readU16(1840); }

// Second (Addr: 1841)
uint16_t iPM2xxx::Read_Second() { return readU16(1841); }

// Millisecond (Addr: 1842)
uint16_t iPM2xxx::Read_Millisecond() { return readU16(1842); }

// Day of Week (Addr: 1843)
uint16_t iPM2xxx::Read_DayOfWeek() { return readU16(1843); }

// Year (Addr: 1844)
uint16_t iPM2xxx::Read_Year_1844() { return readU16(1844); }

// Month & Day (Addr: 1845)
uint16_t iPM2xxx::Read_MonthDay() { return readU16(1845); }

// Hour & Minute (Addr: 1846)
uint16_t iPM2xxx::Read_HourMinute() { return readU16(1846); }

// Milliseconds (Addr: 1847)
uint16_t iPM2xxx::Read_Milliseconds() { return readU16(1847); }

// Setup Password (Addr: 1879)
std::string iPM2xxx::Read_SetupPassword() { return readString(1879, 4); }

// Energy Reset Password (Addr: 1883)
std::string iPM2xxx::Read_EnergyResetPassword() { return readString(1883, 4); }

// Demand  Reset Password (Addr: 1887)
std::string iPM2xxx::Read_DemandResetPassword() {
  return readString(1887, 4);
}

// Min/Max Reset Password (Addr: 1891)
std::string iPM2xxx::Read_MinMaxResetPassword() { return readString(1891, 4); }

// Diagnostics Password (Addr: 1895)
std::string iPM2xxx::Read_DiagnosticsPassword() { return readString(1895, 4); }

// Active Load Timer (Addr: 2001)
uint16_t iPM2xxx::Read_ActiveLoadTimer() { return readU16(2001); }

// Meter Operation Timer (Addr: 2003)
uint16_t iPM2xxx::Read_MeterOperationTimer() { return readU16(2003); }

// Cycle Count (Addr: 2005)
uint16_t iPM2xxx::Read_CycleCount() { return readU16(2005); }

// Number of Phases (Addr: 2013)
uint16_t iPM2xxx::Read_NumberOfPhases() { return readU16(2013); }

// Number of Wires (Addr: 2014)
uint16_t iPM2xxx::Read_NumberOfWires() { return readU16(2014); }

// Power System Configuration (Addr: 2015)
uint16_t iPM2xxx::Read_PowerSystemConfiguration() { return readU16(2015); }

// Nominal Frequency (Addr: 2016)
uint16_t iPM2xxx::Read_NominalFrequency() { return readU16(2016); }

// Nominal Voltage (Addr: 2017)
float iPM2xxx::Read_NominalVoltage() { return readFloat(2017); }

// Nominal Current (Addr: 2019)
float iPM2xxx::Read_NominalCurrent() { return readFloat(2019); }

// Nominal Power Factor (Addr: 2021)
float iPM2xxx::Read_NominalPowerFactor() { return readFloat(2021); }

// Normal Phase Rotation (Addr: 2023)
uint16_t iPM2xxx::Read_NormalPhaseRotation() { return readU16(2023); }

// Number VTs (Addr: 2024)
uint16_t iPM2xxx::Read_NumberVts() { return readU16(2024); }

// VT Primary (Addr: 2025)
float iPM2xxx::Read_VtPrimary() { return readFloat(2025); }

// VT Secondary (Addr: 2027)
uint16_t iPM2xxx::Read_VtSecondary() { return readU16(2027); }

// Number CTs (Addr: 2028)
uint16_t iPM2xxx::Read_NumberCts() { return readU16(2028); }

// CT Primary (Addr: 2029)
uint16_t iPM2xxx::Read_CtPrimary() { return readU16(2029); }

// CT Secondary (Addr: 2030)
uint16_t iPM2xxx::Read_CtSecondary() { return readU16(2030); }

// CT Primary N (Addr: 2031)
uint16_t iPM2xxx::Read_CtPrimaryN() { return readU16(2031); }

// CT Secondary N (Addr: 2032)
uint16_t iPM2xxx::Read_CtSecondaryN() { return readU16(2032); }

// CT Location for 1  or 2 CT Metering (Addr: 2033)
uint16_t iPM2xxx::Read_CtLocationFor1Minusor2CtMetering() {
  return readU16(2033);
}

// VT Location for 1  or 2 VT Metering (Addr: 2034)
uint16_t iPM2xxx::Read_VtLocationFor1Minusor2VtMetering() {
  return readU16(2034);
}

// VT Connection Type (Addr: 2035)
uint16_t iPM2xxx::Read_VtConnectionType() { return readU16(2035); }

// Active Load Timer Setpoint (Addr: 2049)
float iPM2xxx::Read_ActiveLoadTimerSetpoint() { return readFloat(2049); }

// Alarm / Energy LED Mode (Addr: 2125)
uint16_t iPM2xxx::Read_AlarmEnergyLedMode() { return readU16(2125); }

// Energy Channel (Addr: 2129)
uint16_t iPM2xxx::Read_EnergyChannel() { return readU16(2129); }

// Digital Output Association (Addr: 2130)
uint16_t iPM2xxx::Read_DigitalOutputAssociation() { return readU16(2130); }

// Pulse Weight (Addr: 2131)
uint16_t iPM2xxx::Read_PulseWeight() { return readU16(2131); }

// Energy Channel (Addr: 2133)
uint16_t iPM2xxx::Read_EnergyChannel_2133() { return readU16(2133); }

// Digital Output Association (Addr: 2134)
uint16_t iPM2xxx::Read_DigitalOutputAssociation_2134() { return readU16(2134); }

// Pulse Weight (Addr: 2135)
uint16_t iPM2xxx::Read_PulseWeight_2135() { return readU16(2135); }

// Energy Channel (Addr: 2137)
uint16_t iPM2xxx::Read_EnergyChannel_2137() { return readU16(2137); }

// Digital Output Association (Addr: 2138)
uint16_t iPM2xxx::Read_DigitalOutputAssociation_2138() { return readU16(2138); }

// Pulse Weight (Addr: 2139)
uint16_t iPM2xxx::Read_PulseWeight_2139() { return readU16(2139); }

// Label (Addr: 2263)
std::string iPM2xxx::Read_Label() { return readString(2263, 20); }

// Energy Channel (Addr: 2264)
uint16_t iPM2xxx::Read_EnergyChannel_2264() { return readU16(2264); }

// Factor per k__h (Addr: 2266)
float iPM2xxx::Read_FactorPerKH() { return readFloat(2266); }

// Label (Addr: 2286)
std::string iPM2xxx::Read_Label_2286() { return readString(2286, 20); }

// Energy Channel (Addr: 2287)
uint16_t iPM2xxx::Read_EnergyChannel_2287() { return readU16(2287); }

// Factor per k__h (Addr: 2289)
float iPM2xxx::Read_FactorPerKH_2289() { return readFloat(2289); }

// Standard - 1 second Group 1 Validity (Addr: 2419)
uint16_t iPM2xxx::Read_Standard1SecondGroup1Validity() {
  return readU16(2419);
}

// Standard - 1 second Group 1 (Addr: 2420)
uint16_t iPM2xxx::Read_Standard1SecondGroup1() { return readU16(2420); }

// Standard - 1 second Group 2 Validity (Addr: 2421)
uint16_t iPM2xxx::Read_Standard1SecondGroup2Validity() {
  return readU16(2421);
}

// Standard - 1 second Group 2 (Addr: 2422)
uint16_t iPM2xxx::Read_Standard1SecondGroup2() { return readU16(2422); }

// Standard - 1 second Group 3 Validity (Addr: 2423)
uint16_t iPM2xxx::Read_Standard1SecondGroup3Validity() {
  return readU16(2423);
}

// Standard - 1 second Group 3 (Addr: 2424)
uint16_t iPM2xxx::Read_Standard1SecondGroup3() { return readU16(2424); }

// Unary Validity (Addr: 2439)
uint16_t iPM2xxx::Read_UnaryValidity() { return readU16(2439); }

// Unary (Addr: 2440)
uint16_t iPM2xxx::Read_Unary() { return readU16(2440); }

// Digital Group 1 Validity (Addr: 2441)
uint16_t iPM2xxx::Read_DigitalGroup1Validity() { return readU16(2441); }

// Digital Group 1 (Addr: 2442)
uint16_t iPM2xxx::Read_DigitalGroup1() { return readU16(2442); }

// Digital Input Status Validity � Option Slot A (Addr: 2549)
uint16_t iPM2xxx::Read_DigitalInputStatusValidityOptionSlotA() {
  return readU16(2549);
}

// Digital Input Status � Option Slot A (Addr: 2550)
uint16_t iPM2xxx::Read_DigitalInputStatusOptionSlotA() { return readU16(2550); }

// Digital Output Status Validity � Option Slot A (Addr: 2565)
uint16_t iPM2xxx::Read_DigitalOutputStatusValidityOptionSlotA() {
  return readU16(2565);
}

// Digital Output Status � Option Slot A (Addr: 2566)
uint16_t iPM2xxx::Read_DigitalOutputStatusOptionSlotA() {
  return readU16(2566);
}

// Active Energy Delivered (Into Load) (Addr: 2699)
float iPM2xxx::Read_ActiveEnergyDeliveredIntoLoad() { return readFloat(2699); }

// Active Energy Received (Out of Load) (Addr: 2701)
float iPM2xxx::Read_ActiveEnergyReceivedOutOfLoad() { return readFloat(2701); }

// Active Energy Delivered + Received (Addr: 2703)
float iPM2xxx::Read_ActiveEnergyDeliveredPlusReceived() {
  return readFloat(2703);
}

// Active Energy Delivered- Received (Addr: 2705)
float iPM2xxx::Read_ActiveEnergyDeliveredReceived() { return readFloat(2705); }

// Reactive Energy Delivered (Addr: 2707)
float iPM2xxx::Read_ReactiveEnergyDelivered() { return readFloat(2707); }

// Reactive Energy Received (Addr: 2709)
float iPM2xxx::Read_ReactiveEnergyReceived() { return readFloat(2709); }

// Reactive Energy Delivered + Received (Addr: 2711)
float iPM2xxx::Read_ReactiveEnergyDeliveredPlusReceived() {
  return readFloat(2711);
}

// Reactive Energy Delivered - Received (Addr: 2713)
float iPM2xxx::Read_ReactiveEnergyDeliveredMinusReceived() {
  return readFloat(2713);
}

// Apparent Energy Delivered (Addr: 2715)
float iPM2xxx::Read_ApparentEnergyDelivered() { return readFloat(2715); }

// Apparent Energy Received (Addr: 2717)
float iPM2xxx::Read_ApparentEnergyReceived() { return readFloat(2717); }

// Apparent Energy Delivered + Received (Addr: 2719)
float iPM2xxx::Read_ApparentEnergyDeliveredPlusReceived() {
  return readFloat(2719);
}

// Apparent Energy Delivered - Received (Addr: 2721)
float iPM2xxx::Read_ApparentEnergyDeliveredMinusReceived() {
  return readFloat(2721);
}

// Rate1 value (Addr: 2853)
float iPM2xxx::Read_Rate1Value() { return readFloat(2853); }

// Rate 2 Value (Addr: 2855)
float iPM2xxx::Read_Rate2Value() { return readFloat(2855); }

// Current A (Addr: 2999)
float iPM2xxx::Read_CurrentA() { return readFloat(2999); }

// Current B (Addr: 3001)
float iPM2xxx::Read_CurrentB() { return readFloat(3001); }

// Current C (Addr: 3003)
float iPM2xxx::Read_CurrentC() { return readFloat(3003); }

// Current N (Addr: 3005)
float iPM2xxx::Read_CurrentN() { return readFloat(3005); }

// Current G (Addr: 3007)
float iPM2xxx::Read_CurrentG() { return readFloat(3007); }

// Current Avg (Addr: 3009)
float iPM2xxx::Read_CurrentAvg() { return readFloat(3009); }

// Current Unbalance A (Addr: 3011)
float iPM2xxx::Read_CurrentUnbalanceA() { return readFloat(3011); }

// Current Unbalance B (Addr: 3013)
float iPM2xxx::Read_CurrentUnbalanceB() { return readFloat(3013); }

// Current Unbalance C (Addr: 3015)
float iPM2xxx::Read_CurrentUnbalanceC() { return readFloat(3015); }

// Current Unbalance Worst (Addr: 3017)
float iPM2xxx::Read_CurrentUnbalanceWorst() { return readFloat(3017); }

// Voltage A-B (Addr: 3019)
float iPM2xxx::Read_VoltageAB() { return readFloat(3019); }

// Voltage B-C (Addr: 3021)
float iPM2xxx::Read_VoltageBC() { return readFloat(3021); }

// Voltage C-A (Addr: 3023)
float iPM2xxx::Read_VoltageCA() { return readFloat(3023); }

// Voltage L-L Avg (Addr: 3025)
float iPM2xxx::Read_VoltageLLAvg() { return readFloat(3025); }

// Voltage A-N (Addr: 3027)
float iPM2xxx::Read_VoltageAN() { return readFloat(3027); }

// Voltage B-N (Addr: 3029)
float iPM2xxx::Read_VoltageBN() { return readFloat(3029); }

// Voltage C-N (Addr: 3031)
float iPM2xxx::Read_VoltageCN() { return readFloat(3031); }

// Voltage L-N Avg (Addr: 3035)
float iPM2xxx::Read_VoltageLNAvg() { return readFloat(3035); }

// Voltage Unbalance A-B (Addr: 3037)
float iPM2xxx::Read_VoltageUnbalanceAB() { return readFloat(3037); }

// Voltage Unbalance B-C (Addr: 3039)
float iPM2xxx::Read_VoltageUnbalanceBC() { return readFloat(3039); }

// Voltage Unbalance C-A (Addr: 3041)
float iPM2xxx::Read_VoltageUnbalanceCA() { return readFloat(3041); }

// Voltage Unbalance L-L Worst (Addr: 3043)
float iPM2xxx::Read_VoltageUnbalanceLLWorst() { return readFloat(3043); }

// Voltage Unbalance A-N (Addr: 3045)
float iPM2xxx::Read_VoltageUnbalanceAN() { return readFloat(3045); }

// Voltage Unbalance B-N (Addr: 3047)
float iPM2xxx::Read_VoltageUnbalanceBN() { return readFloat(3047); }

// Voltage Unbalance C-N (Addr: 3049)
float iPM2xxx::Read_VoltageUnbalanceCN() { return readFloat(3049); }

// Voltage Unbalance L-N Worst (Addr: 3051)
float iPM2xxx::Read_VoltageUnbalanceLNWorst() { return readFloat(3051); }

// Active Power A (Addr: 3053)
float iPM2xxx::Read_ActivePowerA() { return readFloat(3053); }

// Active Power B (Addr: 3055)
float iPM2xxx::Read_ActivePowerB() { return readFloat(3055); }

// Active Power C (Addr: 3057)
float iPM2xxx::Read_ActivePowerC() { return readFloat(3057); }

// Active Power Total (Addr: 3059)
float iPM2xxx::Read_ActivePowerTotal() { return readFloat(3059); }

// Reactive Power A (Addr: 3061)
float iPM2xxx::Read_ReactivePowerA() { return readFloat(3061); }

// Reactive Power B (Addr: 3063)
float iPM2xxx::Read_ReactivePowerB() { return readFloat(3063); }

// Reactive Power C (Addr: 3065)
float iPM2xxx::Read_ReactivePowerC() { return readFloat(3065); }

// Reactive Power Total (Addr: 3067)
float iPM2xxx::Read_ReactivePowerTotal() { return readFloat(3067); }

// Apparent Power A (Addr: 3069)
float iPM2xxx::Read_ApparentPowerA() { return readFloat(3069); }

// Apparent Power B (Addr: 3071)
float iPM2xxx::Read_ApparentPowerB() { return readFloat(3071); }

// Apparent Power C (Addr: 3073)
float iPM2xxx::Read_ApparentPowerC() { return readFloat(3073); }

// Apparent Power Total (Addr: 3075)
float iPM2xxx::Read_ApparentPowerTotal() { return readFloat(3075); }

// Power Factor A (Addr: 3077)
float iPM2xxx::Read_PowerFactorA() { return readFloat(3077); }

// Power Factor B (Addr: 3079)
float iPM2xxx::Read_PowerFactorB() { return readFloat(3079); }

// Power Factor C (Addr: 3081)
float iPM2xxx::Read_PowerFactorC() { return readFloat(3081); }

// Power Factor Total (Addr: 3083)
float iPM2xxx::Read_PowerFactorTotal() { return readFloat(3083); }

// Displacement Power Factor A (Addr: 3085)
float iPM2xxx::Read_DisplacementPowerFactorA() { return readFloat(3085); }

// Displacement Power Factor B (Addr: 3087)
float iPM2xxx::Read_DisplacementPowerFactorB() { return readFloat(3087); }

// Displacement Power Factor C (Addr: 3089)
float iPM2xxx::Read_DisplacementPowerFactorC() { return readFloat(3089); }

// Displacement Power Factor Total (Addr: 3091)
float iPM2xxx::Read_DisplacementPowerFactorTotal() { return readFloat(3091); }

// Frequency (Addr: 3109)
float iPM2xxx::Read_Frequency() { return readFloat(3109); }

// Power Factor Total (Addr: 3191)
float iPM2xxx::Read_PowerFactorTotal_3191() { return readFloat(3191); }

// Power Factor Total (Addr: 3193)
float iPM2xxx::Read_PowerFactorTotal_3193() { return readFloat(3193); }

// Power Factor Total (Addr: 3195)
int16_t iPM2xxx::Read_PowerFactorTotal_3195() { return (int16_t)readU16(3195); }

// Power Factor Total (Addr: 3196)
int16_t iPM2xxx::Read_PowerFactorTotal_3196() { return (int16_t)readU16(3196); }

// Accumulated Energy Reset Date/Time (Addr: 3199)
uint16_t iPM2xxx::Read_AccumulatedEnergyResetDateTime() {
  return readU16(3199);
}

// Active Energy Delivered (Into Load) (Addr: 3203)
uint64_t iPM2xxx::Read_ActiveEnergy_Delivered() {
  return readU64(3203);
}

// Active Energy Received (Out of Load) (Addr: 3207)
uint64_t iPM2xxx::Read_ActiveEnergy_Received() {
  return readU64(3207);
}

// Active Energy Delivered + Received (Addr: 3211)
uint64_t iPM2xxx::Read_ActiveEnergy_Total() {
  return readU64(3211);
}

// Active Energy Delivered- Received (Addr: 3215)
uint64_t iPM2xxx::Read_ActiveEnergy_DeliveredReceived() {
  return readU64(3215);
}

// Reactive Energy Delivered (Addr: 3219)
uint64_t iPM2xxx::Read_ReactiveEnergy_Delivered() { return readU64(3219); }

// Reactive Energy Received (Addr: 3223)
uint64_t iPM2xxx::Read_ReactiveEnergy_Received() { return readU64(3223); }

// Reactive Energy Delivered + Received (Addr: 3227)
uint64_t iPM2xxx::Read_ReactiveEnergy_Total() {
  return readU64(3227);
}

// Reactive Energy Delivered - Received (Addr: 3231)
uint64_t iPM2xxx::Read_ReactiveEnergy_Net() {
  return readU64(3231);
}

// Apparent Energy Delivered (Addr: 3235)
uint64_t iPM2xxx::Read_ApparentEnergy_Delivered() { return readU64(3235); }

// Apparent Energy Received (Addr: 3239)
uint64_t iPM2xxx::Read_ApparentEnergy_Received() { return readU64(3239); }

// Apparent Energy Delivered + Received (Addr: 3243)
uint64_t iPM2xxx::Read_ApparentEnergy_Total() {
  return readU64(3243);
}

// Apparent Energy Delivered - Received (Addr: 3247)
uint64_t iPM2xxx::Read_ApparentEnergy_Net() {
  return readU64(3247);
}

// Power Demand Method (Addr: 3700)
uint16_t iPM2xxx::Read_PowerDemandMethod() { return readU16(3700); }

// Power Demand Interval Duration (Addr: 3701)
uint16_t iPM2xxx::Read_PowerDemandIntervalDuration() { return readU16(3701); }

// Power Demand Subinterval Duration (Addr: 3702)
uint16_t iPM2xxx::Read_PowerDemandSubintervalDuration() {
  return readU16(3702);
}

// Power Demand Elapsed Time in Interval (Addr: 3703)
uint16_t iPM2xxx::Read_PowerDemandElapsedTimeInInterval() {
  return readU16(3703);
}

// Power Demand Elapsed Time in Subinterval (Addr: 3704)
uint16_t iPM2xxx::Read_PowerDemandElapsedTimeInSubinterval() {
  return readU16(3704);
}

// Power Demand Peak Reset Date/Time (Addr: 3705)
uint16_t iPM2xxx::Read_PowerDemandPeakResetDateTime() { return readU16(3705); }

// Current Demand Method (Addr: 3710)
uint16_t iPM2xxx::Read_CurrentDemandMethod() { return readU16(3710); }

// Current Demand Interval Duration (Addr: 3711)
uint16_t iPM2xxx::Read_CurrentDemandIntervalDuration() { return readU16(3711); }

// Current Demand Subinterval Duration (Addr: 3712)
uint16_t iPM2xxx::Read_CurrentDemandSubintervalDuration() {
  return readU16(3712);
}

// Current Demand Elapsed Time in Interval (Addr: 3713)
uint16_t iPM2xxx::Read_CurrentDemandElapsedTimeInInterval() {
  return readU16(3713);
}

// Current Demand Elapsed Time in Subinterval (Addr: 3714)
uint16_t iPM2xxx::Read_CurrentDemandElapsedTimeInSubinterval() {
  return readU16(3714);
}

// Current Demand Peak Reset Date/Time (Addr: 3715)
uint16_t iPM2xxx::Read_CurrentDemandPeakResetDateTime() {
  return readU16(3715);
}

// Demand System Assignment (Addr: 3760)
uint16_t iPM2xxx::Read_DemandSystemAssignment() { return readU16(3760); }

// Register Number of Metered Quantity (Addr: 3761)
uint16_t iPM2xxx::Read_RegisterNumberOfMeteredQuantity() {
  return readU16(3761);
}

// Units Code (Addr: 3762)
uint16_t iPM2xxx::Read_UnitsCode() { return readU16(3762); }

// Last Demand (Addr: 3763)
float iPM2xxx::Read_LastDemand() { return readFloat(3763); }

// Present Demand (Addr: 3765)
float iPM2xxx::Read_PresentDemand() { return readFloat(3765); }

// Predicted Demand (Addr: 3767)
float iPM2xxx::Read_PredictedDemand() { return readFloat(3767); }

// Peak Demand (Addr: 3769)
float iPM2xxx::Read_PeakDemand() { return readFloat(3769); }

// Peak Demand DateTime (Addr: 3771)
uint16_t iPM2xxx::Read_PeakDemandDatetime() { return readU16(3771); }

// Demand System Assignment (Addr: 3776)
uint16_t iPM2xxx::Read_DemandSystemAssignment_3776() { return readU16(3776); }

// Register Number of Metered Quantity (Addr: 3777)
uint16_t iPM2xxx::Read_RegisterNumberOfMeteredQuantity_3777() {
  return readU16(3777);
}

// Units Code (Addr: 3778)
uint16_t iPM2xxx::Read_UnitsCode_3778() { return readU16(3778); }

// Last Demand (Addr: 3779)
float iPM2xxx::Read_LastDemand_3779() { return readFloat(3779); }

// Present Demand (Addr: 3781)
float iPM2xxx::Read_PresentDemand_3781() { return readFloat(3781); }

// Predicted Demand (Addr: 3783)
float iPM2xxx::Read_PredictedDemand_3783() { return readFloat(3783); }

// Peak Demand (Addr: 3785)
float iPM2xxx::Read_PeakDemand_3785() { return readFloat(3785); }

// Peak Demand DateTime (Addr: 3787)
uint16_t iPM2xxx::Read_PeakDemandDatetime_3787() { return readU16(3787); }

// Demand System Assignment (Addr: 3792)
uint16_t iPM2xxx::Read_DemandSystemAssignment_3792() { return readU16(3792); }

// Register Number of Metered Quantity (Addr: 3793)
uint16_t iPM2xxx::Read_RegisterNumberOfMeteredQuantity_3793() {
  return readU16(3793);
}

// Units Code (Addr: 3794)
uint16_t iPM2xxx::Read_UnitsCode_3794() { return readU16(3794); }

// Last Demand (Addr: 3795)
float iPM2xxx::Read_LastDemand_3795() { return readFloat(3795); }

// Present Demand (Addr: 3797)
float iPM2xxx::Read_PresentDemand_3797() { return readFloat(3797); }

// Predicted Demand (Addr: 3799)
float iPM2xxx::Read_PredictedDemand_3799() { return readFloat(3799); }

// Peak Demand (Addr: 3801)
float iPM2xxx::Read_PeakDemand_3801() { return readFloat(3801); }

// Peak Demand DateTime (Addr: 3803)
uint16_t iPM2xxx::Read_PeakDemandDatetime_3803() { return readU16(3803); }

// Demand System Assignment (Addr: 3872)
uint16_t iPM2xxx::Read_DemandSystemAssignment_3872() { return readU16(3872); }

// Register Number of Metered Quantity (Addr: 3873)
uint16_t iPM2xxx::Read_RegisterNumberOfMeteredQuantity_3873() {
  return readU16(3873);
}

// Units Code (Addr: 3874)
uint16_t iPM2xxx::Read_UnitsCode_3874() { return readU16(3874); }

// Last Demand (Addr: 3875)
float iPM2xxx::Read_LastDemand_3875() { return readFloat(3875); }

// Present Demand (Addr: 3877)
float iPM2xxx::Read_PresentDemand_3877() { return readFloat(3877); }

// Predicted Demand (Addr: 3879)
float iPM2xxx::Read_PredictedDemand_3879() { return readFloat(3879); }

// Peak Demand (Addr: 3881)
float iPM2xxx::Read_PeakDemand_3881() { return readFloat(3881); }

// Peak Demand DateTime (Addr: 3883)
uint16_t iPM2xxx::Read_PeakDemandDatetime_3883() { return readU16(3883); }

// Requested Command (Addr: 4999)
uint16_t iPM2xxx::Read_RequestedCommand() { return readU16(4999); }

// Command Semaphore (Addr: 5000)
uint16_t iPM2xxx::Read_CommandSemaphore() { return readU16(5000); }

// Command Parameter 001 (Addr: 5001)
uint16_t iPM2xxx::Read_CommandParameter001() { return readU16(5001); }

// Command Parameter 002 (Addr: 5002)
uint16_t iPM2xxx::Read_CommandParameter002() { return readU16(5002); }

// Command Parameter 003 (Addr: 5003)
uint16_t iPM2xxx::Read_CommandParameter003() { return readU16(5003); }

// Command Parameter 004 (Addr: 5004)
uint16_t iPM2xxx::Read_CommandParameter004() { return readU16(5004); }

// Command Parameter 005 (Addr: 5005)
uint16_t iPM2xxx::Read_CommandParameter005() { return readU16(5005); }

// Command Parameter 006 (Addr: 5006)
uint16_t iPM2xxx::Read_CommandParameter006() { return readU16(5006); }

// Command Parameter 007 (Addr: 5007)
uint16_t iPM2xxx::Read_CommandParameter007() { return readU16(5007); }

// Command Parameter 008 (Addr: 5008)
uint16_t iPM2xxx::Read_CommandParameter008() { return readU16(5008); }

// Command Parameter 009 (Addr: 5009)
uint16_t iPM2xxx::Read_CommandParameter009() { return readU16(5009); }

// Command Parameter 010 (Addr: 5010)
uint16_t iPM2xxx::Read_CommandParameter010() { return readU16(5010); }

// Command Parameter 011 (Addr: 5011)
uint16_t iPM2xxx::Read_CommandParameter011() { return readU16(5011); }

// Command Parameter 012 (Addr: 5012)
uint16_t iPM2xxx::Read_CommandParameter012() { return readU16(5012); }

// Command Parameter 013 (Addr: 5013)
uint16_t iPM2xxx::Read_CommandParameter013() { return readU16(5013); }

// Command Parameter 014 (Addr: 5014)
uint16_t iPM2xxx::Read_CommandParameter014() { return readU16(5014); }

// Command Parameter 015 (Addr: 5015)
uint16_t iPM2xxx::Read_CommandParameter015() { return readU16(5015); }

// Command Parameter 016 (Addr: 5016)
uint16_t iPM2xxx::Read_CommandParameter016() { return readU16(5016); }

// Command Parameter 017 (Addr: 5017)
uint16_t iPM2xxx::Read_CommandParameter017() { return readU16(5017); }

// Command Parameter 018 (Addr: 5018)
uint16_t iPM2xxx::Read_CommandParameter018() { return readU16(5018); }

// Command Parameter 019 (Addr: 5019)
uint16_t iPM2xxx::Read_CommandParameter019() { return readU16(5019); }

// Command Parameter 020 (Addr: 5020)
uint16_t iPM2xxx::Read_CommandParameter020() { return readU16(5020); }

// Command Parameter 021 (Addr: 5021)
uint16_t iPM2xxx::Read_CommandParameter021() { return readU16(5021); }

// Command Parameter 022 (Addr: 5022)
uint16_t iPM2xxx::Read_CommandParameter022() { return readU16(5022); }

// Command Parameter 023 (Addr: 5023)
uint16_t iPM2xxx::Read_CommandParameter023() { return readU16(5023); }

// Command Parameter 024 (Addr: 5024)
uint16_t iPM2xxx::Read_CommandParameter024() { return readU16(5024); }

// Command Parameter 025 (Addr: 5025)
uint16_t iPM2xxx::Read_CommandParameter025() { return readU16(5025); }

// Command Parameter 026 (Addr: 5026)
uint16_t iPM2xxx::Read_CommandParameter026() { return readU16(5026); }

// Command Parameter 027 (Addr: 5027)
uint16_t iPM2xxx::Read_CommandParameter027() { return readU16(5027); }

// Command Parameter 028 (Addr: 5028)
uint16_t iPM2xxx::Read_CommandParameter028() { return readU16(5028); }

// Command Parameter 029 (Addr: 5029)
uint16_t iPM2xxx::Read_CommandParameter029() { return readU16(5029); }

// Command Parameter 030 (Addr: 5030)
uint16_t iPM2xxx::Read_CommandParameter030() { return readU16(5030); }

// Command Parameter 031 (Addr: 5031)
uint16_t iPM2xxx::Read_CommandParameter031() { return readU16(5031); }

// Command Parameter 032 (Addr: 5032)
uint16_t iPM2xxx::Read_CommandParameter032() { return readU16(5032); }

// Command Parameter 033 (Addr: 5033)
uint16_t iPM2xxx::Read_CommandParameter033() { return readU16(5033); }

// Command Parameter 034 (Addr: 5034)
uint16_t iPM2xxx::Read_CommandParameter034() { return readU16(5034); }

// Command Parameter 035 (Addr: 5035)
uint16_t iPM2xxx::Read_CommandParameter035() { return readU16(5035); }

// Command Parameter 036 (Addr: 5036)
uint16_t iPM2xxx::Read_CommandParameter036() { return readU16(5036); }

// Command Parameter 037 (Addr: 5037)
uint16_t iPM2xxx::Read_CommandParameter037() { return readU16(5037); }

// Command Parameter 038 (Addr: 5038)
uint16_t iPM2xxx::Read_CommandParameter038() { return readU16(5038); }

// Command Parameter 039 (Addr: 5039)
uint16_t iPM2xxx::Read_CommandParameter039() { return readU16(5039); }

// Command Parameter 040 (Addr: 5040)
uint16_t iPM2xxx::Read_CommandParameter040() { return readU16(5040); }

// Command Parameter 041 (Addr: 5041)
uint16_t iPM2xxx::Read_CommandParameter041() { return readU16(5041); }

// Command Parameter 042 (Addr: 5042)
uint16_t iPM2xxx::Read_CommandParameter042() { return readU16(5042); }

// Command Parameter 043 (Addr: 5043)
uint16_t iPM2xxx::Read_CommandParameter043() { return readU16(5043); }

// Command Parameter 044 (Addr: 5044)
uint16_t iPM2xxx::Read_CommandParameter044() { return readU16(5044); }

// Command Parameter 045 (Addr: 5045)
uint16_t iPM2xxx::Read_CommandParameter045() { return readU16(5045); }

// Command Parameter 046 (Addr: 5046)
uint16_t iPM2xxx::Read_CommandParameter046() { return readU16(5046); }

// Command Parameter 047 (Addr: 5047)
uint16_t iPM2xxx::Read_CommandParameter047() { return readU16(5047); }

// Command Parameter 048 (Addr: 5048)
uint16_t iPM2xxx::Read_CommandParameter048() { return readU16(5048); }

// Command Parameter 049 (Addr: 5049)
uint16_t iPM2xxx::Read_CommandParameter049() { return readU16(5049); }

// Command Parameter 050 (Addr: 5050)
uint16_t iPM2xxx::Read_CommandParameter050() { return readU16(5050); }

// Command Parameter 051 (Addr: 5051)
uint16_t iPM2xxx::Read_CommandParameter051() { return readU16(5051); }

// Command Parameter 052 (Addr: 5052)
uint16_t iPM2xxx::Read_CommandParameter052() { return readU16(5052); }

// Command Parameter 053 (Addr: 5053)
uint16_t iPM2xxx::Read_CommandParameter053() { return readU16(5053); }

// Command Parameter 054 (Addr: 5054)
uint16_t iPM2xxx::Read_CommandParameter054() { return readU16(5054); }

// Command Parameter 055 (Addr: 5055)
uint16_t iPM2xxx::Read_CommandParameter055() { return readU16(5055); }

// Command Parameter 056 (Addr: 5056)
uint16_t iPM2xxx::Read_CommandParameter056() { return readU16(5056); }

// Command Parameter 057 (Addr: 5057)
uint16_t iPM2xxx::Read_CommandParameter057() { return readU16(5057); }

// Command Parameter 058 (Addr: 5058)
uint16_t iPM2xxx::Read_CommandParameter058() { return readU16(5058); }

// Command Parameter 059 (Addr: 5059)
uint16_t iPM2xxx::Read_CommandParameter059() { return readU16(5059); }

// Command Parameter 060 (Addr: 5060)
uint16_t iPM2xxx::Read_CommandParameter060() { return readU16(5060); }

// Command Parameter 061 (Addr: 5061)
uint16_t iPM2xxx::Read_CommandParameter061() { return readU16(5061); }

// Command Parameter 062 (Addr: 5062)
uint16_t iPM2xxx::Read_CommandParameter062() { return readU16(5062); }

// Command Parameter 063 (Addr: 5063)
uint16_t iPM2xxx::Read_CommandParameter063() { return readU16(5063); }

// Command Parameter 064 (Addr: 5064)
uint16_t iPM2xxx::Read_CommandParameter064() { return readU16(5064); }

// Command Parameter 065 (Addr: 5065)
uint16_t iPM2xxx::Read_CommandParameter065() { return readU16(5065); }

// Command Parameter 066 (Addr: 5066)
uint16_t iPM2xxx::Read_CommandParameter066() { return readU16(5066); }

// Command Parameter 067 (Addr: 5067)
uint16_t iPM2xxx::Read_CommandParameter067() { return readU16(5067); }

// Command Parameter 068 (Addr: 5068)
uint16_t iPM2xxx::Read_CommandParameter068() { return readU16(5068); }

// Command Parameter 069 (Addr: 5069)
uint16_t iPM2xxx::Read_CommandParameter069() { return readU16(5069); }

// Command Parameter 070 (Addr: 5070)
uint16_t iPM2xxx::Read_CommandParameter070() { return readU16(5070); }

// Command Parameter 071 (Addr: 5071)
uint16_t iPM2xxx::Read_CommandParameter071() { return readU16(5071); }

// Command Parameter 072 (Addr: 5072)
uint16_t iPM2xxx::Read_CommandParameter072() { return readU16(5072); }

// Command Parameter 073 (Addr: 5073)
uint16_t iPM2xxx::Read_CommandParameter073() { return readU16(5073); }

// Command Parameter 074 (Addr: 5074)
uint16_t iPM2xxx::Read_CommandParameter074() { return readU16(5074); }

// Command Parameter 075 (Addr: 5075)
uint16_t iPM2xxx::Read_CommandParameter075() { return readU16(5075); }

// Command Parameter 076 (Addr: 5076)
uint16_t iPM2xxx::Read_CommandParameter076() { return readU16(5076); }

// Command Parameter 077 (Addr: 5077)
uint16_t iPM2xxx::Read_CommandParameter077() { return readU16(5077); }

// Command Parameter 078 (Addr: 5078)
uint16_t iPM2xxx::Read_CommandParameter078() { return readU16(5078); }

// Command Parameter 079 (Addr: 5079)
uint16_t iPM2xxx::Read_CommandParameter079() { return readU16(5079); }

// Command Parameter 080 (Addr: 5080)
uint16_t iPM2xxx::Read_CommandParameter080() { return readU16(5080); }

// Command Parameter 081 (Addr: 5081)
uint16_t iPM2xxx::Read_CommandParameter081() { return readU16(5081); }

// Command Parameter 082 (Addr: 5082)
uint16_t iPM2xxx::Read_CommandParameter082() { return readU16(5082); }

// Command Parameter 083 (Addr: 5083)
uint16_t iPM2xxx::Read_CommandParameter083() { return readU16(5083); }

// Command Parameter 084 (Addr: 5084)
uint16_t iPM2xxx::Read_CommandParameter084() { return readU16(5084); }

// Command Parameter 085 (Addr: 5085)
uint16_t iPM2xxx::Read_CommandParameter085() { return readU16(5085); }

// Command Parameter 086 (Addr: 5086)
uint16_t iPM2xxx::Read_CommandParameter086() { return readU16(5086); }

// Command Parameter 087 (Addr: 5087)
uint16_t iPM2xxx::Read_CommandParameter087() { return readU16(5087); }

// Command Parameter 088 (Addr: 5088)
uint16_t iPM2xxx::Read_CommandParameter088() { return readU16(5088); }

// Command Parameter 089 (Addr: 5089)
uint16_t iPM2xxx::Read_CommandParameter089() { return readU16(5089); }

// Command Parameter 090 (Addr: 5090)
uint16_t iPM2xxx::Read_CommandParameter090() { return readU16(5090); }

// Command Parameter 091 (Addr: 5091)
uint16_t iPM2xxx::Read_CommandParameter091() { return readU16(5091); }

// Command Parameter 092 (Addr: 5092)
uint16_t iPM2xxx::Read_CommandParameter092() { return readU16(5092); }

// Command Parameter 093 (Addr: 5093)
uint16_t iPM2xxx::Read_CommandParameter093() { return readU16(5093); }

// Command Parameter 094 (Addr: 5094)
uint16_t iPM2xxx::Read_CommandParameter094() { return readU16(5094); }

// Command Parameter 095 (Addr: 5095)
uint16_t iPM2xxx::Read_CommandParameter095() { return readU16(5095); }

// Command Parameter 096 (Addr: 5096)
uint16_t iPM2xxx::Read_CommandParameter096() { return readU16(5096); }

// Command Parameter 097 (Addr: 5097)
uint16_t iPM2xxx::Read_CommandParameter097() { return readU16(5097); }

// Command Parameter 098 (Addr: 5098)
uint16_t iPM2xxx::Read_CommandParameter098() { return readU16(5098); }

// Command Parameter 099 (Addr: 5099)
uint16_t iPM2xxx::Read_CommandParameter099() { return readU16(5099); }

// Command Parameter 100 (Addr: 5100)
uint16_t iPM2xxx::Read_CommandParameter100() { return readU16(5100); }

// Command Parameter 101 (Addr: 5101)
uint16_t iPM2xxx::Read_CommandParameter101() { return readU16(5101); }

// Command Parameter 102 (Addr: 5102)
uint16_t iPM2xxx::Read_CommandParameter102() { return readU16(5102); }

// Command Parameter 103 (Addr: 5103)
uint16_t iPM2xxx::Read_CommandParameter103() { return readU16(5103); }

// Command Parameter 104 (Addr: 5104)
uint16_t iPM2xxx::Read_CommandParameter104() { return readU16(5104); }

// Command Parameter 105 (Addr: 5105)
uint16_t iPM2xxx::Read_CommandParameter105() { return readU16(5105); }

// Command Parameter 106 (Addr: 5106)
uint16_t iPM2xxx::Read_CommandParameter106() { return readU16(5106); }

// Command Parameter 107 (Addr: 5107)
uint16_t iPM2xxx::Read_CommandParameter107() { return readU16(5107); }

// Command Parameter 108 (Addr: 5108)
uint16_t iPM2xxx::Read_CommandParameter108() { return readU16(5108); }

// Command Parameter 109 (Addr: 5109)
uint16_t iPM2xxx::Read_CommandParameter109() { return readU16(5109); }

// Command Parameter 110 (Addr: 5110)
uint16_t iPM2xxx::Read_CommandParameter110() { return readU16(5110); }

// Command Parameter 111 (Addr: 5111)
uint16_t iPM2xxx::Read_CommandParameter111() { return readU16(5111); }

// Command Parameter 112 (Addr: 5112)
uint16_t iPM2xxx::Read_CommandParameter112() { return readU16(5112); }

// Command Parameter 113 (Addr: 5113)
uint16_t iPM2xxx::Read_CommandParameter113() { return readU16(5113); }

// Command Parameter 114 (Addr: 5114)
uint16_t iPM2xxx::Read_CommandParameter114() { return readU16(5114); }

// Command Parameter 115 (Addr: 5115)
uint16_t iPM2xxx::Read_CommandParameter115() { return readU16(5115); }

// Command Parameter 116 (Addr: 5116)
uint16_t iPM2xxx::Read_CommandParameter116() { return readU16(5116); }

// Command Parameter 117 (Addr: 5117)
uint16_t iPM2xxx::Read_CommandParameter117() { return readU16(5117); }

// Command Parameter 118 (Addr: 5118)
uint16_t iPM2xxx::Read_CommandParameter118() { return readU16(5118); }

// Command Parameter 119 (Addr: 5119)
uint16_t iPM2xxx::Read_CommandParameter119() { return readU16(5119); }

// Command Parameter 120 (Addr: 5120)
uint16_t iPM2xxx::Read_CommandParameter120() { return readU16(5120); }

// Command Parameter 121 (Addr: 5121)
uint16_t iPM2xxx::Read_CommandParameter121() { return readU16(5121); }

// Command Parameter 122 (Addr: 5122)
uint16_t iPM2xxx::Read_CommandParameter122() { return readU16(5122); }

// Command Parameter 123 (Addr: 5123)
uint16_t iPM2xxx::Read_CommandParameter123() { return readU16(5123); }

// Command Status (Addr: 5124)
uint16_t iPM2xxx::Read_CommandStatus() { return readU16(5124); }

// Command Result (Addr: 5125)
uint16_t iPM2xxx::Read_CommandResult() { return readU16(5125); }

// Command Data 001 (Addr: 5126)
uint16_t iPM2xxx::Read_CommandData001() { return readU16(5126); }

// Command Data 002 (Addr: 5127)
uint16_t iPM2xxx::Read_CommandData002() { return readU16(5127); }

// Command Data 003 (Addr: 5128)
uint16_t iPM2xxx::Read_CommandData003() { return readU16(5128); }

// Command Data 004 (Addr: 5129)
uint16_t iPM2xxx::Read_CommandData004() { return readU16(5129); }

// Command Data 005 (Addr: 5130)
uint16_t iPM2xxx::Read_CommandData005() { return readU16(5130); }

// Command Data 006 (Addr: 5131)
uint16_t iPM2xxx::Read_CommandData006() { return readU16(5131); }

// Command Data 007 (Addr: 5132)
uint16_t iPM2xxx::Read_CommandData007() { return readU16(5132); }

// Command Data 008 (Addr: 5133)
uint16_t iPM2xxx::Read_CommandData008() { return readU16(5133); }

// Command Data 009 (Addr: 5134)
uint16_t iPM2xxx::Read_CommandData009() { return readU16(5134); }

// Command Data 010 (Addr: 5135)
uint16_t iPM2xxx::Read_CommandData010() { return readU16(5135); }

// Command Data 011 (Addr: 5136)
uint16_t iPM2xxx::Read_CommandData011() { return readU16(5136); }

// Command Data 012 (Addr: 5137)
uint16_t iPM2xxx::Read_CommandData012() { return readU16(5137); }

// Command Data 013 (Addr: 5138)
uint16_t iPM2xxx::Read_CommandData013() { return readU16(5138); }

// Command Data 014 (Addr: 5139)
uint16_t iPM2xxx::Read_CommandData014() { return readU16(5139); }

// Command Data 015 (Addr: 5140)
uint16_t iPM2xxx::Read_CommandData015() { return readU16(5140); }

// Command Data 016 (Addr: 5141)
uint16_t iPM2xxx::Read_CommandData016() { return readU16(5141); }

// Command Data 017 (Addr: 5142)
uint16_t iPM2xxx::Read_CommandData017() { return readU16(5142); }

// Command Data 018 (Addr: 5143)
uint16_t iPM2xxx::Read_CommandData018() { return readU16(5143); }

// Command Data 019 (Addr: 5144)
uint16_t iPM2xxx::Read_CommandData019() { return readU16(5144); }

// Command Data 020 (Addr: 5145)
uint16_t iPM2xxx::Read_CommandData020() { return readU16(5145); }

// Command Data 021 (Addr: 5146)
uint16_t iPM2xxx::Read_CommandData021() { return readU16(5146); }

// Command Data 022 (Addr: 5147)
uint16_t iPM2xxx::Read_CommandData022() { return readU16(5147); }

// Command Data 023 (Addr: 5148)
uint16_t iPM2xxx::Read_CommandData023() { return readU16(5148); }

// Command Data 024 (Addr: 5149)
uint16_t iPM2xxx::Read_CommandData024() { return readU16(5149); }

// Command Data 025 (Addr: 5150)
uint16_t iPM2xxx::Read_CommandData025() { return readU16(5150); }

// Command Data 026 (Addr: 5151)
uint16_t iPM2xxx::Read_CommandData026() { return readU16(5151); }

// Command Data 027 (Addr: 5152)
uint16_t iPM2xxx::Read_CommandData027() { return readU16(5152); }

// Command Data 028 (Addr: 5153)
uint16_t iPM2xxx::Read_CommandData028() { return readU16(5153); }

// Command Data 029 (Addr: 5154)
uint16_t iPM2xxx::Read_CommandData029() { return readU16(5154); }

// Command Data 030 (Addr: 5155)
uint16_t iPM2xxx::Read_CommandData030() { return readU16(5155); }

// Command Data 031 (Addr: 5156)
uint16_t iPM2xxx::Read_CommandData031() { return readU16(5156); }

// Command Data 032 (Addr: 5157)
uint16_t iPM2xxx::Read_CommandData032() { return readU16(5157); }

// Command Data 033 (Addr: 5158)
uint16_t iPM2xxx::Read_CommandData033() { return readU16(5158); }

// Command Data 034 (Addr: 5159)
uint16_t iPM2xxx::Read_CommandData034() { return readU16(5159); }

// Command Data 035 (Addr: 5160)
uint16_t iPM2xxx::Read_CommandData035() { return readU16(5160); }

// Command Data 036 (Addr: 5161)
uint16_t iPM2xxx::Read_CommandData036() { return readU16(5161); }

// Command Data 037 (Addr: 5162)
uint16_t iPM2xxx::Read_CommandData037() { return readU16(5162); }

// Command Data 038 (Addr: 5163)
uint16_t iPM2xxx::Read_CommandData038() { return readU16(5163); }

// Command Data 039 (Addr: 5164)
uint16_t iPM2xxx::Read_CommandData039() { return readU16(5164); }

// Command Data 040 (Addr: 5165)
uint16_t iPM2xxx::Read_CommandData040() { return readU16(5165); }

// Command Data 041 (Addr: 5166)
uint16_t iPM2xxx::Read_CommandData041() { return readU16(5166); }

// Command Data 042 (Addr: 5167)
uint16_t iPM2xxx::Read_CommandData042() { return readU16(5167); }

// Command Data 043 (Addr: 5168)
uint16_t iPM2xxx::Read_CommandData043() { return readU16(5168); }

// Command Data 044 (Addr: 5169)
uint16_t iPM2xxx::Read_CommandData044() { return readU16(5169); }

// Command Data 045 (Addr: 5170)
uint16_t iPM2xxx::Read_CommandData045() { return readU16(5170); }

// Command Data 046 (Addr: 5171)
uint16_t iPM2xxx::Read_CommandData046() { return readU16(5171); }

// Command Data 047 (Addr: 5172)
uint16_t iPM2xxx::Read_CommandData047() { return readU16(5172); }

// Command Data 048 (Addr: 5173)
uint16_t iPM2xxx::Read_CommandData048() { return readU16(5173); }

// Command Data 049 (Addr: 5174)
uint16_t iPM2xxx::Read_CommandData049() { return readU16(5174); }

// Command Data 050 (Addr: 5175)
uint16_t iPM2xxx::Read_CommandData050() { return readU16(5175); }

// Command Data 051 (Addr: 5176)
uint16_t iPM2xxx::Read_CommandData051() { return readU16(5176); }

// Command Data 052 (Addr: 5177)
uint16_t iPM2xxx::Read_CommandData052() { return readU16(5177); }

// Command Data 053 (Addr: 5178)
uint16_t iPM2xxx::Read_CommandData053() { return readU16(5178); }

// Command Data 054 (Addr: 5179)
uint16_t iPM2xxx::Read_CommandData054() { return readU16(5179); }

// Command Data 055 (Addr: 5180)
uint16_t iPM2xxx::Read_CommandData055() { return readU16(5180); }

// Command Data 056 (Addr: 5181)
uint16_t iPM2xxx::Read_CommandData056() { return readU16(5181); }

// Command Data 057 (Addr: 5182)
uint16_t iPM2xxx::Read_CommandData057() { return readU16(5182); }

// Command Data 058 (Addr: 5183)
uint16_t iPM2xxx::Read_CommandData058() { return readU16(5183); }

// Command Data 059 (Addr: 5184)
uint16_t iPM2xxx::Read_CommandData059() { return readU16(5184); }

// Command Data 060 (Addr: 5185)
uint16_t iPM2xxx::Read_CommandData060() { return readU16(5185); }

// Command Data 061 (Addr: 5186)
uint16_t iPM2xxx::Read_CommandData061() { return readU16(5186); }

// Command Data 062 (Addr: 5187)
uint16_t iPM2xxx::Read_CommandData062() { return readU16(5187); }

// Command Data 063 (Addr: 5188)
uint16_t iPM2xxx::Read_CommandData063() { return readU16(5188); }

// Command Data 064 (Addr: 5189)
uint16_t iPM2xxx::Read_CommandData064() { return readU16(5189); }

// Command Data 065 (Addr: 5190)
uint16_t iPM2xxx::Read_CommandData065() { return readU16(5190); }

// Command Data 066 (Addr: 5191)
uint16_t iPM2xxx::Read_CommandData066() { return readU16(5191); }

// Command Data 067 (Addr: 5192)
uint16_t iPM2xxx::Read_CommandData067() { return readU16(5192); }

// Command Data 068 (Addr: 5193)
uint16_t iPM2xxx::Read_CommandData068() { return readU16(5193); }

// Command Data 069 (Addr: 5194)
uint16_t iPM2xxx::Read_CommandData069() { return readU16(5194); }

// Command Data 070 (Addr: 5195)
uint16_t iPM2xxx::Read_CommandData070() { return readU16(5195); }

// Command Data 071 (Addr: 5196)
uint16_t iPM2xxx::Read_CommandData071() { return readU16(5196); }

// Command Data 072 (Addr: 5197)
uint16_t iPM2xxx::Read_CommandData072() { return readU16(5197); }

// Command Data 073 (Addr: 5198)
uint16_t iPM2xxx::Read_CommandData073() { return readU16(5198); }

// Command Data 074 (Addr: 5199)
uint16_t iPM2xxx::Read_CommandData074() { return readU16(5199); }

// Command Data 075 (Addr: 5200)
uint16_t iPM2xxx::Read_CommandData075() { return readU16(5200); }

// Command Data 076 (Addr: 5201)
uint16_t iPM2xxx::Read_CommandData076() { return readU16(5201); }

// Command Data 077 (Addr: 5202)
uint16_t iPM2xxx::Read_CommandData077() { return readU16(5202); }

// Command Data 078 (Addr: 5203)
uint16_t iPM2xxx::Read_CommandData078() { return readU16(5203); }

// Command Data 079 (Addr: 5204)
uint16_t iPM2xxx::Read_CommandData079() { return readU16(5204); }

// Command Data 080 (Addr: 5205)
uint16_t iPM2xxx::Read_CommandData080() { return readU16(5205); }

// Command Data 081 (Addr: 5206)
uint16_t iPM2xxx::Read_CommandData081() { return readU16(5206); }

// Command Data 082 (Addr: 5207)
uint16_t iPM2xxx::Read_CommandData082() { return readU16(5207); }

// Command Data 083 (Addr: 5208)
uint16_t iPM2xxx::Read_CommandData083() { return readU16(5208); }

// Command Data 084 (Addr: 5209)
uint16_t iPM2xxx::Read_CommandData084() { return readU16(5209); }

// Command Data 085 (Addr: 5210)
uint16_t iPM2xxx::Read_CommandData085() { return readU16(5210); }

// Command Data 086 (Addr: 5211)
uint16_t iPM2xxx::Read_CommandData086() { return readU16(5211); }

// Command Data 087 (Addr: 5212)
uint16_t iPM2xxx::Read_CommandData087() { return readU16(5212); }

// Command Data 088 (Addr: 5213)
uint16_t iPM2xxx::Read_CommandData088() { return readU16(5213); }

// Command Data 089 (Addr: 5214)
uint16_t iPM2xxx::Read_CommandData089() { return readU16(5214); }

// Command Data 090 (Addr: 5215)
uint16_t iPM2xxx::Read_CommandData090() { return readU16(5215); }

// Command Data 091 (Addr: 5216)
uint16_t iPM2xxx::Read_CommandData091() { return readU16(5216); }

// Command Data 092 (Addr: 5217)
uint16_t iPM2xxx::Read_CommandData092() { return readU16(5217); }

// Command Data 093 (Addr: 5218)
uint16_t iPM2xxx::Read_CommandData093() { return readU16(5218); }

// Command Data 094 (Addr: 5219)
uint16_t iPM2xxx::Read_CommandData094() { return readU16(5219); }

// Command Data 095 (Addr: 5220)
uint16_t iPM2xxx::Read_CommandData095() { return readU16(5220); }

// Command Data 096 (Addr: 5221)
uint16_t iPM2xxx::Read_CommandData096() { return readU16(5221); }

// Command Data 097 (Addr: 5222)
uint16_t iPM2xxx::Read_CommandData097() { return readU16(5222); }

// Command Data 098 (Addr: 5223)
uint16_t iPM2xxx::Read_CommandData098() { return readU16(5223); }

// Command Data 099 (Addr: 5224)
uint16_t iPM2xxx::Read_CommandData099() { return readU16(5224); }

// Command Data 100 (Addr: 5225)
uint16_t iPM2xxx::Read_CommandData100() { return readU16(5225); }

// Command Data 101 (Addr: 5226)
uint16_t iPM2xxx::Read_CommandData101() { return readU16(5226); }

// Command Data 102 (Addr: 5227)
uint16_t iPM2xxx::Read_CommandData102() { return readU16(5227); }

// Command Data 103 (Addr: 5228)
uint16_t iPM2xxx::Read_CommandData103() { return readU16(5228); }

// Command Data 104 (Addr: 5229)
uint16_t iPM2xxx::Read_CommandData104() { return readU16(5229); }

// Command Data 105 (Addr: 5230)
uint16_t iPM2xxx::Read_CommandData105() { return readU16(5230); }

// Command Data 106 (Addr: 5231)
uint16_t iPM2xxx::Read_CommandData106() { return readU16(5231); }

// Command Data 107 (Addr: 5232)
uint16_t iPM2xxx::Read_CommandData107() { return readU16(5232); }

// Command Data 108 (Addr: 5233)
uint16_t iPM2xxx::Read_CommandData108() { return readU16(5233); }

// Command Data 109 (Addr: 5234)
uint16_t iPM2xxx::Read_CommandData109() { return readU16(5234); }

// Command Data 110 (Addr: 5235)
uint16_t iPM2xxx::Read_CommandData110() { return readU16(5235); }

// Command Data 111 (Addr: 5236)
uint16_t iPM2xxx::Read_CommandData111() { return readU16(5236); }

// Command Data 112 (Addr: 5237)
uint16_t iPM2xxx::Read_CommandData112() { return readU16(5237); }

// Command Data 113 (Addr: 5238)
uint16_t iPM2xxx::Read_CommandData113() { return readU16(5238); }

// Command Data 114 (Addr: 5239)
uint16_t iPM2xxx::Read_CommandData114() { return readU16(5239); }

// Command Data 115 (Addr: 5240)
uint16_t iPM2xxx::Read_CommandData115() { return readU16(5240); }

// Command Data 116 (Addr: 5241)
uint16_t iPM2xxx::Read_CommandData116() { return readU16(5241); }

// Command Data 117 (Addr: 5242)
uint16_t iPM2xxx::Read_CommandData117() { return readU16(5242); }

// Command Data 118 (Addr: 5243)
uint16_t iPM2xxx::Read_CommandData118() { return readU16(5243); }

// Command Data 119 (Addr: 5244)
uint16_t iPM2xxx::Read_CommandData119() { return readU16(5244); }

// Command Data 120 (Addr: 5245)
uint16_t iPM2xxx::Read_CommandData120() { return readU16(5245); }

// Command Data 121 (Addr: 5246)
uint16_t iPM2xxx::Read_CommandData121() { return readU16(5246); }

// Command Data 122 (Addr: 5247)
uint16_t iPM2xxx::Read_CommandData122() { return readU16(5247); }

// Command Data 123 (Addr: 5248)
uint16_t iPM2xxx::Read_CommandData123() { return readU16(5248); }

// Requested Command (Addr: 5249)
uint16_t iPM2xxx::Read_RequestedCommand_5249() { return readU16(5249); }

// Command Parameter 001 (Addr: 5251)
uint16_t iPM2xxx::Read_CommandParameter001_5251() { return readU16(5251); }

// Command Parameter 002 (Addr: 5252)
uint16_t iPM2xxx::Read_CommandParameter002_5252() { return readU16(5252); }

// Command Parameter 003 (Addr: 5253)
uint16_t iPM2xxx::Read_CommandParameter003_5253() { return readU16(5253); }

// Command Parameter 004 (Addr: 5254)
uint16_t iPM2xxx::Read_CommandParameter004_5254() { return readU16(5254); }

// Command Parameter 005 (Addr: 5255)
uint16_t iPM2xxx::Read_CommandParameter005_5255() { return readU16(5255); }

// Command Parameter 006 (Addr: 5256)
uint16_t iPM2xxx::Read_CommandParameter006_5256() { return readU16(5256); }

// Command Parameter 007 (Addr: 5257)
uint16_t iPM2xxx::Read_CommandParameter007_5257() { return readU16(5257); }

// Command Parameter 008 (Addr: 5258)
uint16_t iPM2xxx::Read_CommandParameter008_5258() { return readU16(5258); }

// Command Parameter 009 (Addr: 5259)
uint16_t iPM2xxx::Read_CommandParameter009_5259() { return readU16(5259); }

// Command Parameter 010 (Addr: 5260)
uint16_t iPM2xxx::Read_CommandParameter010_5260() { return readU16(5260); }

// Command Parameter 011 (Addr: 5261)
uint16_t iPM2xxx::Read_CommandParameter011_5261() { return readU16(5261); }

// Command Parameter 012 (Addr: 5262)
uint16_t iPM2xxx::Read_CommandParameter012_5262() { return readU16(5262); }

// Command Parameter 013 (Addr: 5263)
uint16_t iPM2xxx::Read_CommandParameter013_5263() { return readU16(5263); }

// Command Parameter 014 (Addr: 5264)
uint16_t iPM2xxx::Read_CommandParameter014_5264() { return readU16(5264); }

// Command Parameter 015 (Addr: 5265)
uint16_t iPM2xxx::Read_CommandParameter015_5265() { return readU16(5265); }

// Command Parameter 016 (Addr: 5266)
uint16_t iPM2xxx::Read_CommandParameter016_5266() { return readU16(5266); }

// Command Parameter 017 (Addr: 5267)
uint16_t iPM2xxx::Read_CommandParameter017_5267() { return readU16(5267); }

// Command Parameter 018 (Addr: 5268)
uint16_t iPM2xxx::Read_CommandParameter018_5268() { return readU16(5268); }

// Command Parameter 019 (Addr: 5269)
uint16_t iPM2xxx::Read_CommandParameter019_5269() { return readU16(5269); }

// Command Parameter 020 (Addr: 5270)
uint16_t iPM2xxx::Read_CommandParameter020_5270() { return readU16(5270); }

// Command Parameter 021 (Addr: 5271)
uint16_t iPM2xxx::Read_CommandParameter021_5271() { return readU16(5271); }

// Command Parameter 022 (Addr: 5272)
uint16_t iPM2xxx::Read_CommandParameter022_5272() { return readU16(5272); }

// Command Parameter 023 (Addr: 5273)
uint16_t iPM2xxx::Read_CommandParameter023_5273() { return readU16(5273); }

// Command Parameter 024 (Addr: 5274)
uint16_t iPM2xxx::Read_CommandParameter024_5274() { return readU16(5274); }

// Command Parameter 025 (Addr: 5275)
uint16_t iPM2xxx::Read_CommandParameter025_5275() { return readU16(5275); }

// Command Parameter 026 (Addr: 5276)
uint16_t iPM2xxx::Read_CommandParameter026_5276() { return readU16(5276); }

// Command Parameter 027 (Addr: 5277)
uint16_t iPM2xxx::Read_CommandParameter027_5277() { return readU16(5277); }

// Command Parameter 028 (Addr: 5278)
uint16_t iPM2xxx::Read_CommandParameter028_5278() { return readU16(5278); }

// Command Parameter 029 (Addr: 5279)
uint16_t iPM2xxx::Read_CommandParameter029_5279() { return readU16(5279); }

// Command Parameter 030 (Addr: 5280)
uint16_t iPM2xxx::Read_CommandParameter030_5280() { return readU16(5280); }

// Command Parameter 031 (Addr: 5281)
uint16_t iPM2xxx::Read_CommandParameter031_5281() { return readU16(5281); }

// Command Parameter 032 (Addr: 5282)
uint16_t iPM2xxx::Read_CommandParameter032_5282() { return readU16(5282); }

// Command Parameter 033 (Addr: 5283)
uint16_t iPM2xxx::Read_CommandParameter033_5283() { return readU16(5283); }

// Command Parameter 034 (Addr: 5284)
uint16_t iPM2xxx::Read_CommandParameter034_5284() { return readU16(5284); }

// Command Parameter 035 (Addr: 5285)
uint16_t iPM2xxx::Read_CommandParameter035_5285() { return readU16(5285); }

// Command Parameter 036 (Addr: 5286)
uint16_t iPM2xxx::Read_CommandParameter036_5286() { return readU16(5286); }

// Command Parameter 037 (Addr: 5287)
uint16_t iPM2xxx::Read_CommandParameter037_5287() { return readU16(5287); }

// Command Parameter 038 (Addr: 5288)
uint16_t iPM2xxx::Read_CommandParameter038_5288() { return readU16(5288); }

// Command Parameter 039 (Addr: 5289)
uint16_t iPM2xxx::Read_CommandParameter039_5289() { return readU16(5289); }

// Command Parameter 040 (Addr: 5290)
uint16_t iPM2xxx::Read_CommandParameter040_5290() { return readU16(5290); }

// Command Parameter 041 (Addr: 5291)
uint16_t iPM2xxx::Read_CommandParameter041_5291() { return readU16(5291); }

// Command Parameter 042 (Addr: 5292)
uint16_t iPM2xxx::Read_CommandParameter042_5292() { return readU16(5292); }

// Command Parameter 043 (Addr: 5293)
uint16_t iPM2xxx::Read_CommandParameter043_5293() { return readU16(5293); }

// Command Parameter 044 (Addr: 5294)
uint16_t iPM2xxx::Read_CommandParameter044_5294() { return readU16(5294); }

// Command Parameter 045 (Addr: 5295)
uint16_t iPM2xxx::Read_CommandParameter045_5295() { return readU16(5295); }

// Command Parameter 046 (Addr: 5296)
uint16_t iPM2xxx::Read_CommandParameter046_5296() { return readU16(5296); }

// Command Parameter 047 (Addr: 5297)
uint16_t iPM2xxx::Read_CommandParameter047_5297() { return readU16(5297); }

// Command Parameter 048 (Addr: 5298)
uint16_t iPM2xxx::Read_CommandParameter048_5298() { return readU16(5298); }

// Command Parameter 049 (Addr: 5299)
uint16_t iPM2xxx::Read_CommandParameter049_5299() { return readU16(5299); }

// Command Parameter 050 (Addr: 5300)
uint16_t iPM2xxx::Read_CommandParameter050_5300() { return readU16(5300); }

// Command Parameter 051 (Addr: 5301)
uint16_t iPM2xxx::Read_CommandParameter051_5301() { return readU16(5301); }

// Command Parameter 052 (Addr: 5302)
uint16_t iPM2xxx::Read_CommandParameter052_5302() { return readU16(5302); }

// Command Parameter 053 (Addr: 5303)
uint16_t iPM2xxx::Read_CommandParameter053_5303() { return readU16(5303); }

// Command Parameter 054 (Addr: 5304)
uint16_t iPM2xxx::Read_CommandParameter054_5304() { return readU16(5304); }

// Command Parameter 055 (Addr: 5305)
uint16_t iPM2xxx::Read_CommandParameter055_5305() { return readU16(5305); }

// Command Parameter 056 (Addr: 5306)
uint16_t iPM2xxx::Read_CommandParameter056_5306() { return readU16(5306); }

// Command Parameter 057 (Addr: 5307)
uint16_t iPM2xxx::Read_CommandParameter057_5307() { return readU16(5307); }

// Command Parameter 058 (Addr: 5308)
uint16_t iPM2xxx::Read_CommandParameter058_5308() { return readU16(5308); }

// Command Parameter 059 (Addr: 5309)
uint16_t iPM2xxx::Read_CommandParameter059_5309() { return readU16(5309); }

// Command Parameter 060 (Addr: 5310)
uint16_t iPM2xxx::Read_CommandParameter060_5310() { return readU16(5310); }

// Command Parameter 061 (Addr: 5311)
uint16_t iPM2xxx::Read_CommandParameter061_5311() { return readU16(5311); }

// Command Parameter 062 (Addr: 5312)
uint16_t iPM2xxx::Read_CommandParameter062_5312() { return readU16(5312); }

// Command Parameter 063 (Addr: 5313)
uint16_t iPM2xxx::Read_CommandParameter063_5313() { return readU16(5313); }

// Command Parameter 064 (Addr: 5314)
uint16_t iPM2xxx::Read_CommandParameter064_5314() { return readU16(5314); }

// Command Parameter 065 (Addr: 5315)
uint16_t iPM2xxx::Read_CommandParameter065_5315() { return readU16(5315); }

// Command Parameter 066 (Addr: 5316)
uint16_t iPM2xxx::Read_CommandParameter066_5316() { return readU16(5316); }

// Command Parameter 067 (Addr: 5317)
uint16_t iPM2xxx::Read_CommandParameter067_5317() { return readU16(5317); }

// Command Parameter 068 (Addr: 5318)
uint16_t iPM2xxx::Read_CommandParameter068_5318() { return readU16(5318); }

// Command Parameter 069 (Addr: 5319)
uint16_t iPM2xxx::Read_CommandParameter069_5319() { return readU16(5319); }

// Command Parameter 070 (Addr: 5320)
uint16_t iPM2xxx::Read_CommandParameter070_5320() { return readU16(5320); }

// Command Parameter 071 (Addr: 5321)
uint16_t iPM2xxx::Read_CommandParameter071_5321() { return readU16(5321); }

// Command Parameter 072 (Addr: 5322)
uint16_t iPM2xxx::Read_CommandParameter072_5322() { return readU16(5322); }

// Command Parameter 073 (Addr: 5323)
uint16_t iPM2xxx::Read_CommandParameter073_5323() { return readU16(5323); }

// Command Parameter 074 (Addr: 5324)
uint16_t iPM2xxx::Read_CommandParameter074_5324() { return readU16(5324); }

// Command Parameter 075 (Addr: 5325)
uint16_t iPM2xxx::Read_CommandParameter075_5325() { return readU16(5325); }

// Command Parameter 076 (Addr: 5326)
uint16_t iPM2xxx::Read_CommandParameter076_5326() { return readU16(5326); }

// Command Parameter 077 (Addr: 5327)
uint16_t iPM2xxx::Read_CommandParameter077_5327() { return readU16(5327); }

// Command Parameter 078 (Addr: 5328)
uint16_t iPM2xxx::Read_CommandParameter078_5328() { return readU16(5328); }

// Command Parameter 079 (Addr: 5329)
uint16_t iPM2xxx::Read_CommandParameter079_5329() { return readU16(5329); }

// Command Parameter 080 (Addr: 5330)
uint16_t iPM2xxx::Read_CommandParameter080_5330() { return readU16(5330); }

// Command Parameter 081 (Addr: 5331)
uint16_t iPM2xxx::Read_CommandParameter081_5331() { return readU16(5331); }

// Command Parameter 082 (Addr: 5332)
uint16_t iPM2xxx::Read_CommandParameter082_5332() { return readU16(5332); }

// Command Parameter 083 (Addr: 5333)
uint16_t iPM2xxx::Read_CommandParameter083_5333() { return readU16(5333); }

// Command Parameter 084 (Addr: 5334)
uint16_t iPM2xxx::Read_CommandParameter084_5334() { return readU16(5334); }

// Command Parameter 085 (Addr: 5335)
uint16_t iPM2xxx::Read_CommandParameter085_5335() { return readU16(5335); }

// Command Parameter 086 (Addr: 5336)
uint16_t iPM2xxx::Read_CommandParameter086_5336() { return readU16(5336); }

// Command Parameter 087 (Addr: 5337)
uint16_t iPM2xxx::Read_CommandParameter087_5337() { return readU16(5337); }

// Command Parameter 088 (Addr: 5338)
uint16_t iPM2xxx::Read_CommandParameter088_5338() { return readU16(5338); }

// Command Parameter 089 (Addr: 5339)
uint16_t iPM2xxx::Read_CommandParameter089_5339() { return readU16(5339); }

// Command Parameter 090 (Addr: 5340)
uint16_t iPM2xxx::Read_CommandParameter090_5340() { return readU16(5340); }

// Command Parameter 091 (Addr: 5341)
uint16_t iPM2xxx::Read_CommandParameter091_5341() { return readU16(5341); }

// Command Parameter 092 (Addr: 5342)
uint16_t iPM2xxx::Read_CommandParameter092_5342() { return readU16(5342); }

// Command Parameter 093 (Addr: 5343)
uint16_t iPM2xxx::Read_CommandParameter093_5343() { return readU16(5343); }

// Command Parameter 094 (Addr: 5344)
uint16_t iPM2xxx::Read_CommandParameter094_5344() { return readU16(5344); }

// Command Parameter 095 (Addr: 5345)
uint16_t iPM2xxx::Read_CommandParameter095_5345() { return readU16(5345); }

// Command Parameter 096 (Addr: 5346)
uint16_t iPM2xxx::Read_CommandParameter096_5346() { return readU16(5346); }

// Command Parameter 097 (Addr: 5347)
uint16_t iPM2xxx::Read_CommandParameter097_5347() { return readU16(5347); }

// Command Parameter 098 (Addr: 5348)
uint16_t iPM2xxx::Read_CommandParameter098_5348() { return readU16(5348); }

// Command Parameter 099 (Addr: 5349)
uint16_t iPM2xxx::Read_CommandParameter099_5349() { return readU16(5349); }

// Command Parameter 100 (Addr: 5350)
uint16_t iPM2xxx::Read_CommandParameter100_5350() { return readU16(5350); }

// Command Parameter 101 (Addr: 5351)
uint16_t iPM2xxx::Read_CommandParameter101_5351() { return readU16(5351); }

// Command Parameter 102 (Addr: 5352)
uint16_t iPM2xxx::Read_CommandParameter102_5352() { return readU16(5352); }

// Command Parameter 103 (Addr: 5353)
uint16_t iPM2xxx::Read_CommandParameter103_5353() { return readU16(5353); }

// Command Parameter 104 (Addr: 5354)
uint16_t iPM2xxx::Read_CommandParameter104_5354() { return readU16(5354); }

// Command Parameter 105 (Addr: 5355)
uint16_t iPM2xxx::Read_CommandParameter105_5355() { return readU16(5355); }

// Command Parameter 106 (Addr: 5356)
uint16_t iPM2xxx::Read_CommandParameter106_5356() { return readU16(5356); }

// Command Parameter 107 (Addr: 5357)
uint16_t iPM2xxx::Read_CommandParameter107_5357() { return readU16(5357); }

// Command Parameter 108 (Addr: 5358)
uint16_t iPM2xxx::Read_CommandParameter108_5358() { return readU16(5358); }

// Command Parameter 109 (Addr: 5359)
uint16_t iPM2xxx::Read_CommandParameter109_5359() { return readU16(5359); }

// Command Parameter 110 (Addr: 5360)
uint16_t iPM2xxx::Read_CommandParameter110_5360() { return readU16(5360); }

// Command Parameter 111 (Addr: 5361)
uint16_t iPM2xxx::Read_CommandParameter111_5361() { return readU16(5361); }

// Command Parameter 112 (Addr: 5362)
uint16_t iPM2xxx::Read_CommandParameter112_5362() { return readU16(5362); }

// Command Parameter 113 (Addr: 5363)
uint16_t iPM2xxx::Read_CommandParameter113_5363() { return readU16(5363); }

// Command Parameter 114 (Addr: 5364)
uint16_t iPM2xxx::Read_CommandParameter114_5364() { return readU16(5364); }

// Command Parameter 115 (Addr: 5365)
uint16_t iPM2xxx::Read_CommandParameter115_5365() { return readU16(5365); }

// Command Parameter 116 (Addr: 5366)
uint16_t iPM2xxx::Read_CommandParameter116_5366() { return readU16(5366); }

// Command Parameter 117 (Addr: 5367)
uint16_t iPM2xxx::Read_CommandParameter117_5367() { return readU16(5367); }

// Command Parameter 118 (Addr: 5368)
uint16_t iPM2xxx::Read_CommandParameter118_5368() { return readU16(5368); }

// Command Parameter 119 (Addr: 5369)
uint16_t iPM2xxx::Read_CommandParameter119_5369() { return readU16(5369); }

// Command Parameter 120 (Addr: 5370)
uint16_t iPM2xxx::Read_CommandParameter120_5370() { return readU16(5370); }

// Command Parameter 121 (Addr: 5371)
uint16_t iPM2xxx::Read_CommandParameter121_5371() { return readU16(5371); }

// Command Parameter 122 (Addr: 5372)
uint16_t iPM2xxx::Read_CommandParameter122_5372() { return readU16(5372); }

// Command Parameter 123 (Addr: 5373)
uint16_t iPM2xxx::Read_CommandParameter123_5373() { return readU16(5373); }

// Command Status (Addr: 5374)
uint16_t iPM2xxx::Read_CommandStatus_5374() { return readU16(5374); }

// Command Result (Addr: 5375)
uint16_t iPM2xxx::Read_CommandResult_5375() { return readU16(5375); }

// Command Data 001 (Addr: 5376)
uint16_t iPM2xxx::Read_CommandData001_5376() { return readU16(5376); }

// Command Data 002 (Addr: 5377)
uint16_t iPM2xxx::Read_CommandData002_5377() { return readU16(5377); }

// Command Data 003 (Addr: 5378)
uint16_t iPM2xxx::Read_CommandData003_5378() { return readU16(5378); }

// Command Data 004 (Addr: 5379)
uint16_t iPM2xxx::Read_CommandData004_5379() { return readU16(5379); }

// Command Data 005 (Addr: 5380)
uint16_t iPM2xxx::Read_CommandData005_5380() { return readU16(5380); }

// Command Data 006 (Addr: 5381)
uint16_t iPM2xxx::Read_CommandData006_5381() { return readU16(5381); }

// Command Data 007 (Addr: 5382)
uint16_t iPM2xxx::Read_CommandData007_5382() { return readU16(5382); }

// Command Data 008 (Addr: 5383)
uint16_t iPM2xxx::Read_CommandData008_5383() { return readU16(5383); }

// Command Data 009 (Addr: 5384)
uint16_t iPM2xxx::Read_CommandData009_5384() { return readU16(5384); }

// Command Data 010 (Addr: 5385)
uint16_t iPM2xxx::Read_CommandData010_5385() { return readU16(5385); }

// Command Data 011 (Addr: 5386)
uint16_t iPM2xxx::Read_CommandData011_5386() { return readU16(5386); }

// Command Data 012 (Addr: 5387)
uint16_t iPM2xxx::Read_CommandData012_5387() { return readU16(5387); }

// Command Data 013 (Addr: 5388)
uint16_t iPM2xxx::Read_CommandData013_5388() { return readU16(5388); }

// Command Data 014 (Addr: 5389)
uint16_t iPM2xxx::Read_CommandData014_5389() { return readU16(5389); }

// Command Data 015 (Addr: 5390)
uint16_t iPM2xxx::Read_CommandData015_5390() { return readU16(5390); }

// Command Data 016 (Addr: 5391)
uint16_t iPM2xxx::Read_CommandData016_5391() { return readU16(5391); }

// Command Data 017 (Addr: 5392)
uint16_t iPM2xxx::Read_CommandData017_5392() { return readU16(5392); }

// Command Data 018 (Addr: 5393)
uint16_t iPM2xxx::Read_CommandData018_5393() { return readU16(5393); }

// Command Data 019 (Addr: 5394)
uint16_t iPM2xxx::Read_CommandData019_5394() { return readU16(5394); }

// Command Data 020 (Addr: 5395)
uint16_t iPM2xxx::Read_CommandData020_5395() { return readU16(5395); }

// Command Data 021 (Addr: 5396)
uint16_t iPM2xxx::Read_CommandData021_5396() { return readU16(5396); }

// Command Data 022 (Addr: 5397)
uint16_t iPM2xxx::Read_CommandData022_5397() { return readU16(5397); }

// Command Data 023 (Addr: 5398)
uint16_t iPM2xxx::Read_CommandData023_5398() { return readU16(5398); }

// Command Data 024 (Addr: 5399)
uint16_t iPM2xxx::Read_CommandData024_5399() { return readU16(5399); }

// Command Data 025 (Addr: 5400)
uint16_t iPM2xxx::Read_CommandData025_5400() { return readU16(5400); }

// Command Data 026 (Addr: 5401)
uint16_t iPM2xxx::Read_CommandData026_5401() { return readU16(5401); }

// Command Data 027 (Addr: 5402)
uint16_t iPM2xxx::Read_CommandData027_5402() { return readU16(5402); }

// Command Data 028 (Addr: 5403)
uint16_t iPM2xxx::Read_CommandData028_5403() { return readU16(5403); }

// Command Data 029 (Addr: 5404)
uint16_t iPM2xxx::Read_CommandData029_5404() { return readU16(5404); }

// Command Data 030 (Addr: 5405)
uint16_t iPM2xxx::Read_CommandData030_5405() { return readU16(5405); }

// Command Data 031 (Addr: 5406)
uint16_t iPM2xxx::Read_CommandData031_5406() { return readU16(5406); }

// Command Data 032 (Addr: 5407)
uint16_t iPM2xxx::Read_CommandData032_5407() { return readU16(5407); }

// Command Data 033 (Addr: 5408)
uint16_t iPM2xxx::Read_CommandData033_5408() { return readU16(5408); }

// Command Data 034 (Addr: 5409)
uint16_t iPM2xxx::Read_CommandData034_5409() { return readU16(5409); }

// Command Data 035 (Addr: 5410)
uint16_t iPM2xxx::Read_CommandData035_5410() { return readU16(5410); }

// Command Data 036 (Addr: 5411)
uint16_t iPM2xxx::Read_CommandData036_5411() { return readU16(5411); }

// Command Data 037 (Addr: 5412)
uint16_t iPM2xxx::Read_CommandData037_5412() { return readU16(5412); }

// Command Data 038 (Addr: 5413)
uint16_t iPM2xxx::Read_CommandData038_5413() { return readU16(5413); }

// Command Data 039 (Addr: 5414)
uint16_t iPM2xxx::Read_CommandData039_5414() { return readU16(5414); }

// Command Data 040 (Addr: 5415)
uint16_t iPM2xxx::Read_CommandData040_5415() { return readU16(5415); }

// Command Data 041 (Addr: 5416)
uint16_t iPM2xxx::Read_CommandData041_5416() { return readU16(5416); }

// Command Data 042 (Addr: 5417)
uint16_t iPM2xxx::Read_CommandData042_5417() { return readU16(5417); }

// Command Data 043 (Addr: 5418)
uint16_t iPM2xxx::Read_CommandData043_5418() { return readU16(5418); }

// Command Data 044 (Addr: 5419)
uint16_t iPM2xxx::Read_CommandData044_5419() { return readU16(5419); }

// Command Data 045 (Addr: 5420)
uint16_t iPM2xxx::Read_CommandData045_5420() { return readU16(5420); }

// Command Data 046 (Addr: 5421)
uint16_t iPM2xxx::Read_CommandData046_5421() { return readU16(5421); }

// Command Data 047 (Addr: 5422)
uint16_t iPM2xxx::Read_CommandData047_5422() { return readU16(5422); }

// Command Data 048 (Addr: 5423)
uint16_t iPM2xxx::Read_CommandData048_5423() { return readU16(5423); }

// Command Data 049 (Addr: 5424)
uint16_t iPM2xxx::Read_CommandData049_5424() { return readU16(5424); }

// Command Data 050 (Addr: 5425)
uint16_t iPM2xxx::Read_CommandData050_5425() { return readU16(5425); }

// Command Data 051 (Addr: 5426)
uint16_t iPM2xxx::Read_CommandData051_5426() { return readU16(5426); }

// Command Data 052 (Addr: 5427)
uint16_t iPM2xxx::Read_CommandData052_5427() { return readU16(5427); }

// Command Data 053 (Addr: 5428)
uint16_t iPM2xxx::Read_CommandData053_5428() { return readU16(5428); }

// Command Data 054 (Addr: 5429)
uint16_t iPM2xxx::Read_CommandData054_5429() { return readU16(5429); }

// Command Data 055 (Addr: 5430)
uint16_t iPM2xxx::Read_CommandData055_5430() { return readU16(5430); }

// Command Data 056 (Addr: 5431)
uint16_t iPM2xxx::Read_CommandData056_5431() { return readU16(5431); }

// Command Data 057 (Addr: 5432)
uint16_t iPM2xxx::Read_CommandData057_5432() { return readU16(5432); }

// Command Data 058 (Addr: 5433)
uint16_t iPM2xxx::Read_CommandData058_5433() { return readU16(5433); }

// Command Data 059 (Addr: 5434)
uint16_t iPM2xxx::Read_CommandData059_5434() { return readU16(5434); }

// Command Data 060 (Addr: 5435)
uint16_t iPM2xxx::Read_CommandData060_5435() { return readU16(5435); }

// Command Data 061 (Addr: 5436)
uint16_t iPM2xxx::Read_CommandData061_5436() { return readU16(5436); }

// Command Data 062 (Addr: 5437)
uint16_t iPM2xxx::Read_CommandData062_5437() { return readU16(5437); }

// Command Data 063 (Addr: 5438)
uint16_t iPM2xxx::Read_CommandData063_5438() { return readU16(5438); }

// Command Data 064 (Addr: 5439)
uint16_t iPM2xxx::Read_CommandData064_5439() { return readU16(5439); }

// Command Data 065 (Addr: 5440)
uint16_t iPM2xxx::Read_CommandData065_5440() { return readU16(5440); }

// Command Data 066 (Addr: 5441)
uint16_t iPM2xxx::Read_CommandData066_5441() { return readU16(5441); }

// Command Data 067 (Addr: 5442)
uint16_t iPM2xxx::Read_CommandData067_5442() { return readU16(5442); }

// Command Data 068 (Addr: 5443)
uint16_t iPM2xxx::Read_CommandData068_5443() { return readU16(5443); }

// Command Data 069 (Addr: 5444)
uint16_t iPM2xxx::Read_CommandData069_5444() { return readU16(5444); }

// Command Data 070 (Addr: 5445)
uint16_t iPM2xxx::Read_CommandData070_5445() { return readU16(5445); }

// Command Data 071 (Addr: 5446)
uint16_t iPM2xxx::Read_CommandData071_5446() { return readU16(5446); }

// Command Data 072 (Addr: 5447)
uint16_t iPM2xxx::Read_CommandData072_5447() { return readU16(5447); }

// Command Data 073 (Addr: 5448)
uint16_t iPM2xxx::Read_CommandData073_5448() { return readU16(5448); }

// Command Data 074 (Addr: 5449)
uint16_t iPM2xxx::Read_CommandData074_5449() { return readU16(5449); }

// Command Data 075 (Addr: 5450)
uint16_t iPM2xxx::Read_CommandData075_5450() { return readU16(5450); }

// Command Data 076 (Addr: 5451)
uint16_t iPM2xxx::Read_CommandData076_5451() { return readU16(5451); }

// Command Data 077 (Addr: 5452)
uint16_t iPM2xxx::Read_CommandData077_5452() { return readU16(5452); }

// Command Data 078 (Addr: 5453)
uint16_t iPM2xxx::Read_CommandData078_5453() { return readU16(5453); }

// Command Data 079 (Addr: 5454)
uint16_t iPM2xxx::Read_CommandData079_5454() { return readU16(5454); }

// Command Data 080 (Addr: 5455)
uint16_t iPM2xxx::Read_CommandData080_5455() { return readU16(5455); }

// Command Data 081 (Addr: 5456)
uint16_t iPM2xxx::Read_CommandData081_5456() { return readU16(5456); }

// Command Data 082 (Addr: 5457)
uint16_t iPM2xxx::Read_CommandData082_5457() { return readU16(5457); }

// Command Data 083 (Addr: 5458)
uint16_t iPM2xxx::Read_CommandData083_5458() { return readU16(5458); }

// Command Data 084 (Addr: 5459)
uint16_t iPM2xxx::Read_CommandData084_5459() { return readU16(5459); }

// Command Data 085 (Addr: 5460)
uint16_t iPM2xxx::Read_CommandData085_5460() { return readU16(5460); }

// Command Data 086 (Addr: 5461)
uint16_t iPM2xxx::Read_CommandData086_5461() { return readU16(5461); }

// Command Data 087 (Addr: 5462)
uint16_t iPM2xxx::Read_CommandData087_5462() { return readU16(5462); }

// Command Data 088 (Addr: 5463)
uint16_t iPM2xxx::Read_CommandData088_5463() { return readU16(5463); }

// Command Data 089 (Addr: 5464)
uint16_t iPM2xxx::Read_CommandData089_5464() { return readU16(5464); }

// Command Data 090 (Addr: 5465)
uint16_t iPM2xxx::Read_CommandData090_5465() { return readU16(5465); }

// Command Data 091 (Addr: 5466)
uint16_t iPM2xxx::Read_CommandData091_5466() { return readU16(5466); }

// Command Data 092 (Addr: 5467)
uint16_t iPM2xxx::Read_CommandData092_5467() { return readU16(5467); }

// Command Data 093 (Addr: 5468)
uint16_t iPM2xxx::Read_CommandData093_5468() { return readU16(5468); }

// Command Data 094 (Addr: 5469)
uint16_t iPM2xxx::Read_CommandData094_5469() { return readU16(5469); }

// Command Data 095 (Addr: 5470)
uint16_t iPM2xxx::Read_CommandData095_5470() { return readU16(5470); }

// Command Data 096 (Addr: 5471)
uint16_t iPM2xxx::Read_CommandData096_5471() { return readU16(5471); }

// Command Data 097 (Addr: 5472)
uint16_t iPM2xxx::Read_CommandData097_5472() { return readU16(5472); }

// Command Data 098 (Addr: 5473)
uint16_t iPM2xxx::Read_CommandData098_5473() { return readU16(5473); }

// Command Data 099 (Addr: 5474)
uint16_t iPM2xxx::Read_CommandData099_5474() { return readU16(5474); }

// Command Data 100 (Addr: 5475)
uint16_t iPM2xxx::Read_CommandData100_5475() { return readU16(5475); }

// Command Data 101 (Addr: 5476)
uint16_t iPM2xxx::Read_CommandData101_5476() { return readU16(5476); }

// Command Data 102 (Addr: 5477)
uint16_t iPM2xxx::Read_CommandData102_5477() { return readU16(5477); }

// Command Data 103 (Addr: 5478)
uint16_t iPM2xxx::Read_CommandData103_5478() { return readU16(5478); }

// Command Data 104 (Addr: 5479)
uint16_t iPM2xxx::Read_CommandData104_5479() { return readU16(5479); }

// Command Data 105 (Addr: 5480)
uint16_t iPM2xxx::Read_CommandData105_5480() { return readU16(5480); }

// Command Data 106 (Addr: 5481)
uint16_t iPM2xxx::Read_CommandData106_5481() { return readU16(5481); }

// Command Data 107 (Addr: 5482)
uint16_t iPM2xxx::Read_CommandData107_5482() { return readU16(5482); }

// Command Data 108 (Addr: 5483)
uint16_t iPM2xxx::Read_CommandData108_5483() { return readU16(5483); }

// Command Data 109 (Addr: 5484)
uint16_t iPM2xxx::Read_CommandData109_5484() { return readU16(5484); }

// Command Data 110 (Addr: 5485)
uint16_t iPM2xxx::Read_CommandData110_5485() { return readU16(5485); }

// Command Data 111 (Addr: 5486)
uint16_t iPM2xxx::Read_CommandData111_5486() { return readU16(5486); }

// Command Data 112 (Addr: 5487)
uint16_t iPM2xxx::Read_CommandData112_5487() { return readU16(5487); }

// Command Data 113 (Addr: 5488)
uint16_t iPM2xxx::Read_CommandData113_5488() { return readU16(5488); }

// Command Data 114 (Addr: 5489)
uint16_t iPM2xxx::Read_CommandData114_5489() { return readU16(5489); }

// Command Data 115 (Addr: 5490)
uint16_t iPM2xxx::Read_CommandData115_5490() { return readU16(5490); }

// Command Data 116 (Addr: 5491)
uint16_t iPM2xxx::Read_CommandData116_5491() { return readU16(5491); }

// Command Data 117 (Addr: 5492)
uint16_t iPM2xxx::Read_CommandData117_5492() { return readU16(5492); }

// Command Data 118 (Addr: 5493)
uint16_t iPM2xxx::Read_CommandData118_5493() { return readU16(5493); }

// Command Data 119 (Addr: 5494)
uint16_t iPM2xxx::Read_CommandData119_5494() { return readU16(5494); }

// Command Data 120 (Addr: 5495)
uint16_t iPM2xxx::Read_CommandData120_5495() { return readU16(5495); }

// Command Data 121 (Addr: 5496)
uint16_t iPM2xxx::Read_CommandData121_5496() { return readU16(5496); }

// Command Data 122 (Addr: 5497)
uint16_t iPM2xxx::Read_CommandData122_5497() { return readU16(5497); }

// Command Data 123 (Addr: 5498)
uint16_t iPM2xxx::Read_CommandData123_5498() { return readU16(5498); }

// Last Command Date/Time (Addr: 5499)
uint16_t iPM2xxx::Read_LastCommandDateTime() { return readU16(5499); }

// Last Command Number (Addr: 5503)
uint16_t iPM2xxx::Read_LastCommandNumber() { return readU16(5503); }

// Last Command Result (Addr: 5504)
uint16_t iPM2xxx::Read_LastCommandResult() { return readU16(5504); }

// Last Command Source (Addr: 5505)
uint16_t iPM2xxx::Read_LastCommandSource() { return readU16(5505); }

// Count if Duplicate (Addr: 5506)
uint16_t iPM2xxx::Read_CountIfDuplicate() { return readU16(5506); }

// Command Date/Time (Addr: 5507)
uint16_t iPM2xxx::Read_CommandDateTime() { return readU16(5507); }

// Command Number (Addr: 5511)
uint16_t iPM2xxx::Read_CommandNumber() { return readU16(5511); }

// Command Result (Addr: 5512)
uint16_t iPM2xxx::Read_CommandResult_5512() { return readU16(5512); }

// Command Source (Addr: 5513)
uint16_t iPM2xxx::Read_CommandSource() { return readU16(5513); }

// Count if Duplicate (Addr: 5514)
uint16_t iPM2xxx::Read_CountIfDuplicate_5514() { return readU16(5514); }

// Command Date/Time (Addr: 5515)
uint16_t iPM2xxx::Read_CommandDateTime_5515() { return readU16(5515); }

// Command Number (Addr: 5519)
uint16_t iPM2xxx::Read_CommandNumber_5519() { return readU16(5519); }

// Command Result (Addr: 5520)
uint16_t iPM2xxx::Read_CommandResult_5520() { return readU16(5520); }

// Command Source (Addr: 5521)
uint16_t iPM2xxx::Read_CommandSource_5521() { return readU16(5521); }

// Count if Duplicate (Addr: 5522)
uint16_t iPM2xxx::Read_CountIfDuplicate_5522() { return readU16(5522); }

// Command Date/Time (Addr: 5523)
uint16_t iPM2xxx::Read_CommandDateTime_5523() { return readU16(5523); }

// Command Number (Addr: 5527)
uint16_t iPM2xxx::Read_CommandNumber_5527() { return readU16(5527); }

// Command Result (Addr: 5528)
uint16_t iPM2xxx::Read_CommandResult_5528() { return readU16(5528); }

// Command Source (Addr: 5529)
uint16_t iPM2xxx::Read_CommandSource_5529() { return readU16(5529); }

// Count if Duplicate (Addr: 5530)
uint16_t iPM2xxx::Read_CountIfDuplicate_5530() { return readU16(5530); }

// Command Date/Time (Addr: 5531)
uint16_t iPM2xxx::Read_CommandDateTime_5531() { return readU16(5531); }

// Command Number (Addr: 5535)
uint16_t iPM2xxx::Read_CommandNumber_5535() { return readU16(5535); }

// Command Result (Addr: 5536)
uint16_t iPM2xxx::Read_CommandResult_5536() { return readU16(5536); }

// Command Source (Addr: 5537)
uint16_t iPM2xxx::Read_CommandSource_5537() { return readU16(5537); }

// Count if Duplicate (Addr: 5538)
uint16_t iPM2xxx::Read_CountIfDuplicate_5538() { return readU16(5538); }

// Command Date/Time (Addr: 5539)
uint16_t iPM2xxx::Read_CommandDateTime_5539() { return readU16(5539); }

// Command Number (Addr: 5543)
uint16_t iPM2xxx::Read_CommandNumber_5543() { return readU16(5543); }

// Command Result (Addr: 5544)
uint16_t iPM2xxx::Read_CommandResult_5544() { return readU16(5544); }

// Command Source (Addr: 5545)
uint16_t iPM2xxx::Read_CommandSource_5545() { return readU16(5545); }

// Count if Duplicate (Addr: 5546)
uint16_t iPM2xxx::Read_CountIfDuplicate_5546() { return readU16(5546); }

// Command Date/Time (Addr: 5547)
uint16_t iPM2xxx::Read_CommandDateTime_5547() { return readU16(5547); }

// Command Number (Addr: 5551)
uint16_t iPM2xxx::Read_CommandNumber_5551() { return readU16(5551); }

// Command Result (Addr: 5552)
uint16_t iPM2xxx::Read_CommandResult_5552() { return readU16(5552); }

// Command Source (Addr: 5553)
uint16_t iPM2xxx::Read_CommandSource_5553() { return readU16(5553); }

// Count if Duplicate (Addr: 5554)
uint16_t iPM2xxx::Read_CountIfDuplicate_5554() { return readU16(5554); }

// Command Date/Time (Addr: 5555)
uint16_t iPM2xxx::Read_CommandDateTime_5555() { return readU16(5555); }

// Command Number (Addr: 5559)
uint16_t iPM2xxx::Read_CommandNumber_5559() { return readU16(5559); }

// Command Result (Addr: 5560)
uint16_t iPM2xxx::Read_CommandResult_5560() { return readU16(5560); }

// Command Source (Addr: 5561)
uint16_t iPM2xxx::Read_CommandSource_5561() { return readU16(5561); }

// Count if Duplicate (Addr: 5562)
uint16_t iPM2xxx::Read_CountIfDuplicate_5562() { return readU16(5562); }

// Command Date/Time (Addr: 5563)
uint16_t iPM2xxx::Read_CommandDateTime_5563() { return readU16(5563); }

// Command Number (Addr: 5567)
uint16_t iPM2xxx::Read_CommandNumber_5567() { return readU16(5567); }

// Command Result (Addr: 5568)
uint16_t iPM2xxx::Read_CommandResult_5568() { return readU16(5568); }

// Command Source (Addr: 5569)
uint16_t iPM2xxx::Read_CommandSource_5569() { return readU16(5569); }

// Count if Duplicate (Addr: 5570)
uint16_t iPM2xxx::Read_CountIfDuplicate_5570() { return readU16(5570); }

// Command Date/Time (Addr: 5571)
uint16_t iPM2xxx::Read_CommandDateTime_5571() { return readU16(5571); }

// Command Number (Addr: 5575)
uint16_t iPM2xxx::Read_CommandNumber_5575() { return readU16(5575); }

// Command Result (Addr: 5576)
uint16_t iPM2xxx::Read_CommandResult_5576() { return readU16(5576); }

// Command Source (Addr: 5577)
uint16_t iPM2xxx::Read_CommandSource_5577() { return readU16(5577); }

// Count if Duplicate (Addr: 5578)
uint16_t iPM2xxx::Read_CountIfDuplicate_5578() { return readU16(5578); }

// Mailbox Register 001 (Addr: 5579)
uint16_t iPM2xxx::Read_MailboxRegister001() { return readU16(5579); }

// Mailbox Register 002 (Addr: 5580)
uint16_t iPM2xxx::Read_MailboxRegister002() { return readU16(5580); }

// Mailbox Register 003 (Addr: 5581)
uint16_t iPM2xxx::Read_MailboxRegister003() { return readU16(5581); }

// Mailbox Register 004 (Addr: 5582)
uint16_t iPM2xxx::Read_MailboxRegister004() { return readU16(5582); }

// Mailbox Register 005 (Addr: 5583)
uint16_t iPM2xxx::Read_MailboxRegister005() { return readU16(5583); }

// Mailbox Register 006 (Addr: 5584)
uint16_t iPM2xxx::Read_MailboxRegister006() { return readU16(5584); }

// Mailbox Register 007 (Addr: 5585)
uint16_t iPM2xxx::Read_MailboxRegister007() { return readU16(5585); }

// Mailbox Register 008 (Addr: 5586)
uint16_t iPM2xxx::Read_MailboxRegister008() { return readU16(5586); }

// Mailbox Register 009 (Addr: 5587)
uint16_t iPM2xxx::Read_MailboxRegister009() { return readU16(5587); }

// Mailbox Register 010 (Addr: 5588)
uint16_t iPM2xxx::Read_MailboxRegister010() { return readU16(5588); }

// Mailbox Register 011 (Addr: 5589)
uint16_t iPM2xxx::Read_MailboxRegister011() { return readU16(5589); }

// Mailbox Register 012 (Addr: 5590)
uint16_t iPM2xxx::Read_MailboxRegister012() { return readU16(5590); }

// Mailbox Register 013 (Addr: 5591)
uint16_t iPM2xxx::Read_MailboxRegister013() { return readU16(5591); }

// Mailbox Register 014 (Addr: 5592)
uint16_t iPM2xxx::Read_MailboxRegister014() { return readU16(5592); }

// Mailbox Register 015 (Addr: 5593)
uint16_t iPM2xxx::Read_MailboxRegister015() { return readU16(5593); }

// Mailbox Register 016 (Addr: 5594)
uint16_t iPM2xxx::Read_MailboxRegister016() { return readU16(5594); }

// Mailbox Register 017 (Addr: 5595)
uint16_t iPM2xxx::Read_MailboxRegister017() { return readU16(5595); }

// Mailbox Register 018 (Addr: 5596)
uint16_t iPM2xxx::Read_MailboxRegister018() { return readU16(5596); }

// Mailbox Register 019 (Addr: 5597)
uint16_t iPM2xxx::Read_MailboxRegister019() { return readU16(5597); }

// Mailbox Register 020 (Addr: 5598)
uint16_t iPM2xxx::Read_MailboxRegister020() { return readU16(5598); }

// Mailbox Register 021 (Addr: 5599)
uint16_t iPM2xxx::Read_MailboxRegister021() { return readU16(5599); }

// Mailbox Register 022 (Addr: 5600)
uint16_t iPM2xxx::Read_MailboxRegister022() { return readU16(5600); }

// Mailbox Register 023 (Addr: 5601)
uint16_t iPM2xxx::Read_MailboxRegister023() { return readU16(5601); }

// Mailbox Register 024 (Addr: 5602)
uint16_t iPM2xxx::Read_MailboxRegister024() { return readU16(5602); }

// Mailbox Register 025 (Addr: 5603)
uint16_t iPM2xxx::Read_MailboxRegister025() { return readU16(5603); }

// Mailbox Register 026 (Addr: 5604)
uint16_t iPM2xxx::Read_MailboxRegister026() { return readU16(5604); }

// Mailbox Register 027 (Addr: 5605)
uint16_t iPM2xxx::Read_MailboxRegister027() { return readU16(5605); }

// Mailbox Register 028 (Addr: 5606)
uint16_t iPM2xxx::Read_MailboxRegister028() { return readU16(5606); }

// Mailbox Register 029 (Addr: 5607)
uint16_t iPM2xxx::Read_MailboxRegister029() { return readU16(5607); }

// Mailbox Register 030 (Addr: 5608)
uint16_t iPM2xxx::Read_MailboxRegister030() { return readU16(5608); }

// Mailbox Register 031 (Addr: 5609)
uint16_t iPM2xxx::Read_MailboxRegister031() { return readU16(5609); }

// Mailbox Register 032 (Addr: 5610)
uint16_t iPM2xxx::Read_MailboxRegister032() { return readU16(5610); }

// Mailbox Register 033 (Addr: 5611)
uint16_t iPM2xxx::Read_MailboxRegister033() { return readU16(5611); }

// Mailbox Register 034 (Addr: 5612)
uint16_t iPM2xxx::Read_MailboxRegister034() { return readU16(5612); }

// Mailbox Register 035 (Addr: 5613)
uint16_t iPM2xxx::Read_MailboxRegister035() { return readU16(5613); }

// Mailbox Register 036 (Addr: 5614)
uint16_t iPM2xxx::Read_MailboxRegister036() { return readU16(5614); }

// Mailbox Register 037 (Addr: 5615)
uint16_t iPM2xxx::Read_MailboxRegister037() { return readU16(5615); }

// Mailbox Register 038 (Addr: 5616)
uint16_t iPM2xxx::Read_MailboxRegister038() { return readU16(5616); }

// Mailbox Register 039 (Addr: 5617)
uint16_t iPM2xxx::Read_MailboxRegister039() { return readU16(5617); }

// Mailbox Register 040 (Addr: 5618)
uint16_t iPM2xxx::Read_MailboxRegister040() { return readU16(5618); }

// Mailbox Register 041 (Addr: 5619)
uint16_t iPM2xxx::Read_MailboxRegister041() { return readU16(5619); }

// Mailbox Register 042 (Addr: 5620)
uint16_t iPM2xxx::Read_MailboxRegister042() { return readU16(5620); }

// Mailbox Register 043 (Addr: 5621)
uint16_t iPM2xxx::Read_MailboxRegister043() { return readU16(5621); }

// Mailbox Register 044 (Addr: 5622)
uint16_t iPM2xxx::Read_MailboxRegister044() { return readU16(5622); }

// Mailbox Register 045 (Addr: 5623)
uint16_t iPM2xxx::Read_MailboxRegister045() { return readU16(5623); }

// Mailbox Register 046 (Addr: 5624)
uint16_t iPM2xxx::Read_MailboxRegister046() { return readU16(5624); }

// Mailbox Register 047 (Addr: 5625)
uint16_t iPM2xxx::Read_MailboxRegister047() { return readU16(5625); }

// Mailbox Register 048 (Addr: 5626)
uint16_t iPM2xxx::Read_MailboxRegister048() { return readU16(5626); }

// Mailbox Register 049 (Addr: 5627)
uint16_t iPM2xxx::Read_MailboxRegister049() { return readU16(5627); }

// Mailbox Register 050 (Addr: 5628)
uint16_t iPM2xxx::Read_MailboxRegister050() { return readU16(5628); }

// Mailbox Register 051 (Addr: 5629)
uint16_t iPM2xxx::Read_MailboxRegister051() { return readU16(5629); }

// Mailbox Register 052 (Addr: 5630)
uint16_t iPM2xxx::Read_MailboxRegister052() { return readU16(5630); }

// Mailbox Register 053 (Addr: 5631)
uint16_t iPM2xxx::Read_MailboxRegister053() { return readU16(5631); }

// Mailbox Register 054 (Addr: 5632)
uint16_t iPM2xxx::Read_MailboxRegister054() { return readU16(5632); }

// Mailbox Register 055 (Addr: 5633)
uint16_t iPM2xxx::Read_MailboxRegister055() { return readU16(5633); }

// Mailbox Register 056 (Addr: 5634)
uint16_t iPM2xxx::Read_MailboxRegister056() { return readU16(5634); }

// Mailbox Register 057 (Addr: 5635)
uint16_t iPM2xxx::Read_MailboxRegister057() { return readU16(5635); }

// Mailbox Register 058 (Addr: 5636)
uint16_t iPM2xxx::Read_MailboxRegister058() { return readU16(5636); }

// Mailbox Register 059 (Addr: 5637)
uint16_t iPM2xxx::Read_MailboxRegister059() { return readU16(5637); }

// Mailbox Register 060 (Addr: 5638)
uint16_t iPM2xxx::Read_MailboxRegister060() { return readU16(5638); }

// Mailbox Register 061 (Addr: 5639)
uint16_t iPM2xxx::Read_MailboxRegister061() { return readU16(5639); }

// Mailbox Register 062 (Addr: 5640)
uint16_t iPM2xxx::Read_MailboxRegister062() { return readU16(5640); }

// Mailbox Register 063 (Addr: 5641)
uint16_t iPM2xxx::Read_MailboxRegister063() { return readU16(5641); }

// Mailbox Register 064 (Addr: 5642)
uint16_t iPM2xxx::Read_MailboxRegister064() { return readU16(5642); }

// Mailbox Register 065 (Addr: 5643)
uint16_t iPM2xxx::Read_MailboxRegister065() { return readU16(5643); }

// Mailbox Register 066 (Addr: 5644)
uint16_t iPM2xxx::Read_MailboxRegister066() { return readU16(5644); }

// Mailbox Register 067 (Addr: 5645)
uint16_t iPM2xxx::Read_MailboxRegister067() { return readU16(5645); }

// Mailbox Register 068 (Addr: 5646)
uint16_t iPM2xxx::Read_MailboxRegister068() { return readU16(5646); }

// Mailbox Register 069 (Addr: 5647)
uint16_t iPM2xxx::Read_MailboxRegister069() { return readU16(5647); }

// Mailbox Register 070 (Addr: 5648)
uint16_t iPM2xxx::Read_MailboxRegister070() { return readU16(5648); }

// Mailbox Register 071 (Addr: 5649)
uint16_t iPM2xxx::Read_MailboxRegister071() { return readU16(5649); }

// Mailbox Register 072 (Addr: 5650)
uint16_t iPM2xxx::Read_MailboxRegister072() { return readU16(5650); }

// Mailbox Register 073 (Addr: 5651)
uint16_t iPM2xxx::Read_MailboxRegister073() { return readU16(5651); }

// Mailbox Register 074 (Addr: 5652)
uint16_t iPM2xxx::Read_MailboxRegister074() { return readU16(5652); }

// Mailbox Register 075 (Addr: 5653)
uint16_t iPM2xxx::Read_MailboxRegister075() { return readU16(5653); }

// Mailbox Register 076 (Addr: 5654)
uint16_t iPM2xxx::Read_MailboxRegister076() { return readU16(5654); }

// Mailbox Register 077 (Addr: 5655)
uint16_t iPM2xxx::Read_MailboxRegister077() { return readU16(5655); }

// Mailbox Register 078 (Addr: 5656)
uint16_t iPM2xxx::Read_MailboxRegister078() { return readU16(5656); }

// Mailbox Register 079 (Addr: 5657)
uint16_t iPM2xxx::Read_MailboxRegister079() { return readU16(5657); }

// Mailbox Register 080 (Addr: 5658)
uint16_t iPM2xxx::Read_MailboxRegister080() { return readU16(5658); }

// Mailbox Register 081 (Addr: 5659)
uint16_t iPM2xxx::Read_MailboxRegister081() { return readU16(5659); }

// Mailbox Register 082 (Addr: 5660)
uint16_t iPM2xxx::Read_MailboxRegister082() { return readU16(5660); }

// Mailbox Register 083 (Addr: 5661)
uint16_t iPM2xxx::Read_MailboxRegister083() { return readU16(5661); }

// Mailbox Register 084 (Addr: 5662)
uint16_t iPM2xxx::Read_MailboxRegister084() { return readU16(5662); }

// Mailbox Register 085 (Addr: 5663)
uint16_t iPM2xxx::Read_MailboxRegister085() { return readU16(5663); }

// Mailbox Register 086 (Addr: 5664)
uint16_t iPM2xxx::Read_MailboxRegister086() { return readU16(5664); }

// Mailbox Register 087 (Addr: 5665)
uint16_t iPM2xxx::Read_MailboxRegister087() { return readU16(5665); }

// Mailbox Register 088 (Addr: 5666)
uint16_t iPM2xxx::Read_MailboxRegister088() { return readU16(5666); }

// Mailbox Register 089 (Addr: 5667)
uint16_t iPM2xxx::Read_MailboxRegister089() { return readU16(5667); }

// Mailbox Register 090 (Addr: 5668)
uint16_t iPM2xxx::Read_MailboxRegister090() { return readU16(5668); }

// Mailbox Register 091 (Addr: 5669)
uint16_t iPM2xxx::Read_MailboxRegister091() { return readU16(5669); }

// Mailbox Register 092 (Addr: 5670)
uint16_t iPM2xxx::Read_MailboxRegister092() { return readU16(5670); }

// Mailbox Register 093 (Addr: 5671)
uint16_t iPM2xxx::Read_MailboxRegister093() { return readU16(5671); }

// Mailbox Register 094 (Addr: 5672)
uint16_t iPM2xxx::Read_MailboxRegister094() { return readU16(5672); }

// Mailbox Register 095 (Addr: 5673)
uint16_t iPM2xxx::Read_MailboxRegister095() { return readU16(5673); }

// Mailbox Register 096 (Addr: 5674)
uint16_t iPM2xxx::Read_MailboxRegister096() { return readU16(5674); }

// Mailbox Register 097 (Addr: 5675)
uint16_t iPM2xxx::Read_MailboxRegister097() { return readU16(5675); }

// Mailbox Register 098 (Addr: 5676)
uint16_t iPM2xxx::Read_MailboxRegister098() { return readU16(5676); }

// Mailbox Register 099 (Addr: 5677)
uint16_t iPM2xxx::Read_MailboxRegister099() { return readU16(5677); }

// Mailbox Register 100 (Addr: 5678)
uint16_t iPM2xxx::Read_MailboxRegister100() { return readU16(5678); }

// HMI Model Present (Addr: 5999)
uint16_t iPM2xxx::Read_HmiModelPresent() { return readU16(5999); }

// HMI Contrast Setting (Addr: 6000)
uint16_t iPM2xxx::Read_HmiContrastSetting() { return readU16(6000); }

// HMI Language (Addr: 6002)
uint16_t iPM2xxx::Read_HmiLanguage() { return readU16(6002); }

// HMI Date Format (Addr: 6003)
uint16_t iPM2xxx::Read_HmiDateFormat() { return readU16(6003); }

// HMI Time Format (Addr: 6004)
uint16_t iPM2xxx::Read_HmiTimeFormat() { return readU16(6004); }

// HMI IEC/IEEE Mode (Addr: 6005)
uint16_t iPM2xxx::Read_HmiIecIeeeMode() { return readU16(6005); }

// HMI Screen Timeout (Addr: 6006)
uint16_t iPM2xxx::Read_HmiScreenTimeout() { return readU16(6006); }

// HMI Backlight Timeout (Addr: 6007)
uint16_t iPM2xxx::Read_HmiBacklightTimeout() { return readU16(6007); }

// HMI Language 01 (Addr: 6078)
uint16_t iPM2xxx::Read_HmiLanguage01() { return readU16(6078); }

// HMI Language 02 (Addr: 6079)
uint16_t iPM2xxx::Read_HmiLanguage02() { return readU16(6079); }

// HMI Language 03 (Addr: 6080)
uint16_t iPM2xxx::Read_HmiLanguage03() { return readU16(6080); }

// HMI Language 04 (Addr: 6081)
uint16_t iPM2xxx::Read_HmiLanguage04() { return readU16(6081); }

// HMI Language 05 (Addr: 6082)
uint16_t iPM2xxx::Read_HmiLanguage05() { return readU16(6082); }

// HMI Language 06 (Addr: 6083)
uint16_t iPM2xxx::Read_HmiLanguage06() { return readU16(6083); }

// HMI Language 07 (Addr: 6084)
uint16_t iPM2xxx::Read_HmiLanguage07() { return readU16(6084); }

// HMI Language 08 (Addr: 6085)
uint16_t iPM2xxx::Read_HmiLanguage08() { return readU16(6085); }

// HMI Language 09 (Addr: 6086)
uint16_t iPM2xxx::Read_HmiLanguage09() { return readU16(6086); }

// HMI Language 10 (Addr: 6087)
uint16_t iPM2xxx::Read_HmiLanguage10() { return readU16(6087); }

// RS-485 Comm Port (M/S) Protocol (Addr: 6499)
uint16_t iPM2xxx::Read_Rs485CommPortMSProtocol() { return readU16(6499); }

// RS-485 Comm Port (M/S) Address (Addr: 6500)
uint16_t iPM2xxx::Read_Rs485CommPortMSAddress() { return readU16(6500); }

// RS-485 Comm Port (M/S) Baud Rate (Addr: 6501)
uint16_t iPM2xxx::Read_Rs485CommPortMSBaudRate() { return readU16(6501); }

// RS-485 Comm Port (M/S) Parity (Addr: 6502)
uint16_t iPM2xxx::Read_Rs485CommPortMSParity() { return readU16(6502); }

// Type (Addr: 7416)
uint16_t iPM2xxx::Read_Type() { return readU16(7416); }

// Control Mode (Addr: 7417)
uint16_t iPM2xxx::Read_ControlMode() { return readU16(7417); }

// Label (Addr: 7418)
std::string iPM2xxx::Read_Label_7418() { return readString(7418, 20); }

// Debounce Time (Addr: 7438)
uint16_t iPM2xxx::Read_DebounceTime() { return readU16(7438); }

// Type (Addr: 7440)
uint16_t iPM2xxx::Read_Type_7440() { return readU16(7440); }

// Control Mode (Addr: 7441)
uint16_t iPM2xxx::Read_ControlMode_7441() { return readU16(7441); }

// Label (Addr: 7442)
std::string iPM2xxx::Read_Label_7442() { return readString(7442, 20); }

// Debounce Time (Addr: 7462)
uint16_t iPM2xxx::Read_DebounceTime_7462() { return readU16(7462); }

// Count (Addr: 8938)
uint16_t iPM2xxx::Read_Count() { return readU16(8938); }

// On Time (Addr: 8940)
uint16_t iPM2xxx::Read_OnTime() { return readU16(8940); }

// Count (Addr: 8942)
uint16_t iPM2xxx::Read_Count_8942() { return readU16(8942); }

// On Time (Addr: 8944)
uint16_t iPM2xxx::Read_OnTime_8944() { return readU16(8944); }

// Type (Addr: 9282)
uint16_t iPM2xxx::Read_Type_9282() { return readU16(9282); }

// Label (Addr: 9283)
std::string iPM2xxx::Read_Label_9283() { return readString(9283, 20); }

// Enable/Disable (Addr: 9303)
uint16_t iPM2xxx::Read_EnableDisable() { return readU16(9303); }

// Behavioral Mode (Addr: 9304)
uint16_t iPM2xxx::Read_BehavioralMode() { return readU16(9304); }

// On Time For Timed Mode (Addr: 9305)
uint16_t iPM2xxx::Read_OnTimeForTimedMode() { return readU16(9305); }

// Type (Addr: 9306)
uint16_t iPM2xxx::Read_Type_9306() { return readU16(9306); }

// Label (Addr: 9307)
std::string iPM2xxx::Read_Label_9307() { return readString(9307, 20); }

// Enable/Disable (Addr: 9327)
uint16_t iPM2xxx::Read_EnableDisable_9327() { return readU16(9327); }

// Behavioral Mode (Addr: 9328)
uint16_t iPM2xxx::Read_BehavioralMode_9328() { return readU16(9328); }

// On Time For Timed Mode (Addr: 9329)
uint16_t iPM2xxx::Read_OnTimeForTimedMode_9329() { return readU16(9329); }

// Operating Mode Status (Addr: 9703)
uint16_t iPM2xxx::Read_OperatingModeStatus() { return readU16(9703); }

// Control Mode Status (Addr: 9704)
uint16_t iPM2xxx::Read_ControlModeStatus() { return readU16(9704); }

// Behavioral Mode Status (Addr: 9705)
uint16_t iPM2xxx::Read_BehavioralModeStatus() { return readU16(9705); }

// Count (Addr: 9706)
uint16_t iPM2xxx::Read_Count_9706() { return readU16(9706); }

// On Time (Addr: 9708)
uint16_t iPM2xxx::Read_OnTime_9708() { return readU16(9708); }

// Operating Mode Status (Addr: 9711)
uint16_t iPM2xxx::Read_OperatingModeStatus_9711() { return readU16(9711); }

// Control Mode Status (Addr: 9712)
uint16_t iPM2xxx::Read_ControlModeStatus_9712() { return readU16(9712); }

// Behavioral Mode Status (Addr: 9713)
uint16_t iPM2xxx::Read_BehavioralModeStatus_9713() { return readU16(9713); }

// Count (Addr: 9714)
uint16_t iPM2xxx::Read_Count_9714() { return readU16(9714); }

// On Time (Addr: 9716)
uint16_t iPM2xxx::Read_OnTime_9716() { return readU16(9716); }

// Analog input 1 Scaled Value (Addr: 10015)
float iPM2xxx::Read_AnalogInput1ScaledValue() { return readFloat(10015); }

// Analog input 2 Scaled Value (Addr: 10017)
float iPM2xxx::Read_AnalogInput2ScaledValue() { return readFloat(10017); }

// Analog output 1 Normalized Value (Addr: 10215)
float iPM2xxx::Read_AnalogOutput1NormalizedValue() { return readFloat(10215); }

// Analog output 2 Normalized Value (Addr: 10217)
float iPM2xxx::Read_AnalogOutput2NormalizedValue() { return readFloat(10217); }

// Detected Priority Status Bitmap (Addr: 11009)
uint16_t iPM2xxx::Read_DetectedPriorityStatusBitmap() { return readU16(11009); }

// Standard � 1 second 1 (Addr: 11039)
uint16_t iPM2xxx::Read_Standard1Second1() { return readU16(11039); }

// Standard � 1 second 2 (Addr: 11040)
uint16_t iPM2xxx::Read_Standard1Second2() { return readU16(11040); }

// Standard � 1 second 3 (Addr: 11041)
uint16_t iPM2xxx::Read_Standard1Second3() { return readU16(11041); }

// Custom � 1 second (Addr: 11042)
uint16_t iPM2xxx::Read_Custom1Second() { return readU16(11042); }

// Standard � High Speed 1 (Addr: 11043)
uint16_t iPM2xxx::Read_StandardHighSpeed1() { return readU16(11043); }

// Standard � High Speed 2 (Addr: 11044)
uint16_t iPM2xxx::Read_StandardHighSpeed2() { return readU16(11044); }

// Custom � High Speed (Addr: 11045)
uint16_t iPM2xxx::Read_CustomHighSpeed() { return readU16(11045); }

// Disturbance (Addr: 11046)
uint16_t iPM2xxx::Read_Disturbance() { return readU16(11046); }

// Transient (Addr: 11047)
uint16_t iPM2xxx::Read_Transient() { return readU16(11047); }

// Waveshape (Addr: 11048)
uint16_t iPM2xxx::Read_Waveshape() { return readU16(11048); }

// Unary (Addr: 11049)
uint16_t iPM2xxx::Read_Unary_11049() { return readU16(11049); }

// Digital 1 (Addr: 11050)
uint16_t iPM2xxx::Read_Digital1() { return readU16(11050); }

// Digital 2 (Addr: 11051)
uint16_t iPM2xxx::Read_Digital2() { return readU16(11051); }

// Digital 3 (Addr: 11052)
uint16_t iPM2xxx::Read_Digital3() { return readU16(11052); }

// Digital 4 (Addr: 11053)
uint16_t iPM2xxx::Read_Digital4() { return readU16(11053); }

// Digital 5 (Addr: 11054)
uint16_t iPM2xxx::Read_Digital5() { return readU16(11054); }

// Binary (Addr: 11055)
uint16_t iPM2xxx::Read_Binary() { return readU16(11055); }

// Time_of_day (Addr: 11056)
uint16_t iPM2xxx::Read_TimeOfDay() { return readU16(11056); }

// Logic (Addr: 11057)
uint16_t iPM2xxx::Read_Logic() { return readU16(11057); }

// Standard � 1 second Group 1 (Addr: 11058)
uint16_t iPM2xxx::Read_Standard1SecondGroup1_11058() { return readU16(11058); }

// Standard � 1 second Group 2 (Addr: 11059)
uint16_t iPM2xxx::Read_Standard1SecondGroup2_11059() { return readU16(11059); }

// Standard � 1 second Group 3 (Addr: 11060)
uint16_t iPM2xxx::Read_Standard1SecondGroup3_11060() { return readU16(11060); }

// Custom � 1 second (Addr: 11061)
uint16_t iPM2xxx::Read_Custom1Second_11061() { return readU16(11061); }

// Standard � High Speed Group 1 (Addr: 11062)
uint16_t iPM2xxx::Read_StandardHighSpeedGroup1() { return readU16(11062); }

// Standard � High Speed Group 2 (Addr: 11063)
uint16_t iPM2xxx::Read_StandardHighSpeedGroup2() { return readU16(11063); }

// Custom � High Speed (Addr: 11064)
uint16_t iPM2xxx::Read_CustomHighSpeed_11064() { return readU16(11064); }

// Disturbance (Addr: 11065)
uint16_t iPM2xxx::Read_Disturbance_11065() { return readU16(11065); }

// Transient (Addr: 11066)
uint16_t iPM2xxx::Read_Transient_11066() { return readU16(11066); }

// Waveshape (Addr: 11067)
uint16_t iPM2xxx::Read_Waveshape_11067() { return readU16(11067); }

// Unary (Addr: 11068)
uint16_t iPM2xxx::Read_Unary_11068() { return readU16(11068); }

// Digital Group 1 (Addr: 11069)
uint16_t iPM2xxx::Read_DigitalGroup1_11069() { return readU16(11069); }

// Digital Group 2 (Addr: 11070)
uint16_t iPM2xxx::Read_DigitalGroup2() { return readU16(11070); }

// Digital Group 3 (Addr: 11071)
uint16_t iPM2xxx::Read_DigitalGroup3() { return readU16(11071); }

// Digital Group 4 (Addr: 11072)
uint16_t iPM2xxx::Read_DigitalGroup4() { return readU16(11072); }

// Digital Group 5 (Addr: 11073)
uint16_t iPM2xxx::Read_DigitalGroup5() { return readU16(11073); }

// Binary (Addr: 11074)
uint16_t iPM2xxx::Read_Binary_11074() { return readU16(11074); }

// Time_of_day (Addr: 11075)
uint16_t iPM2xxx::Read_TimeOfDay_11075() { return readU16(11075); }

// Logic (Addr: 11076)
uint16_t iPM2xxx::Read_Logic_11076() { return readU16(11076); }

// Standard � 1 second 1 (Addr: 11077)
uint16_t iPM2xxx::Read_Standard1Second1_11077() { return readU16(11077); }

// Standard � 1 second 2 (Addr: 11078)
uint16_t iPM2xxx::Read_Standard1Second2_11078() { return readU16(11078); }

// Standard � 1 second 3 (Addr: 11079)
uint16_t iPM2xxx::Read_Standard1Second3_11079() { return readU16(11079); }

// Custom � 1 second (Addr: 11080)
uint16_t iPM2xxx::Read_Custom1Second_11080() { return readU16(11080); }

// Standard � High Speed 1 (Addr: 11081)
uint16_t iPM2xxx::Read_StandardHighSpeed1_11081() { return readU16(11081); }

// Standard � High Speed 2 (Addr: 11082)
uint16_t iPM2xxx::Read_StandardHighSpeed2_11082() { return readU16(11082); }

// Custom � High Speed (Addr: 11083)
uint16_t iPM2xxx::Read_CustomHighSpeed_11083() { return readU16(11083); }

// Disturbance (Addr: 11084)
uint16_t iPM2xxx::Read_Disturbance_11084() { return readU16(11084); }

// Transient (Addr: 11085)
uint16_t iPM2xxx::Read_Transient_11085() { return readU16(11085); }

// Waveshape (Addr: 11086)
uint16_t iPM2xxx::Read_Waveshape_11086() { return readU16(11086); }

// Unary (Addr: 11087)
uint16_t iPM2xxx::Read_Unary_11087() { return readU16(11087); }

// Digital 1 (Addr: 11088)
uint16_t iPM2xxx::Read_Digital1_11088() { return readU16(11088); }

// Digital 2 (Addr: 11089)
uint16_t iPM2xxx::Read_Digital2_11089() { return readU16(11089); }

// Digital 3 (Addr: 11090)
uint16_t iPM2xxx::Read_Digital3_11090() { return readU16(11090); }

// Digital 4 (Addr: 11091)
uint16_t iPM2xxx::Read_Digital4_11091() { return readU16(11091); }

// Digital 5 (Addr: 11092)
uint16_t iPM2xxx::Read_Digital5_11092() { return readU16(11092); }

// Binary (Addr: 11093)
uint16_t iPM2xxx::Read_Binary_11093() { return readU16(11093); }

// Time_of_day (Addr: 11094)
uint16_t iPM2xxx::Read_TimeOfDay_11094() { return readU16(11094); }

// Logic (Addr: 11095)
uint16_t iPM2xxx::Read_Logic_11095() { return readU16(11095); }

// Version of Event Queue (Addr: 11110)
uint16_t iPM2xxx::Read_VersionOfEventQueue() { return readU16(11110); }

// Event Types to Log (Addr: 11111)
uint16_t iPM2xxx::Read_EventTypesToLog() { return readU16(11111); }

// Size of Event Queue (Addr: 11112)
uint16_t iPM2xxx::Read_SizeOfEventQueue() { return readU16(11112); }

// Number of Entries in Event Queue (Addr: 11113)
uint16_t iPM2xxx::Read_NumberOfEntriesInEventQueue() { return readU16(11113); }

// Entry Number of Most Recent Event (Addr: 11114)
uint16_t iPM2xxx::Read_EntryNumberOfMostRecentEvent() { return readU16(11114); }

// Entry Number (Addr: 11115)
uint16_t iPM2xxx::Read_EntryNumber() { return readU16(11115); }

// Date/Time (Addr: 11116)
uint16_t iPM2xxx::Read_DateTime() { return readU16(11116); }

// Record Type (Addr: 11120)
uint16_t iPM2xxx::Read_RecordType() { return readU16(11120); }

// Register Number or Event Code (Addr: 11121)
uint16_t iPM2xxx::Read_RegOrEventCode() { return readU16(11121); }

// Value (Addr: 11122)
uint16_t iPM2xxx::Read_Value() { return readU16(11122); }

// Sequence Number (Addr: 11126)
uint16_t iPM2xxx::Read_SequenceNumber() { return readU16(11126); }

// Entry Number (Addr: 11127)
uint16_t iPM2xxx::Read_EntryNumber_11127() { return readU16(11127); }

// Date/Time (Addr: 11128)
uint16_t iPM2xxx::Read_DateTime_11128() { return readU16(11128); }

// Record Type (Addr: 11132)
uint16_t iPM2xxx::Read_RecordType_11132() { return readU16(11132); }

// Register Number or Event Code (Addr: 11133)
uint16_t iPM2xxx::Read_RegOrEventCode_11133() {
  return readU16(11133);
}

// Value (Addr: 11134)
uint16_t iPM2xxx::Read_Value_11134() { return readU16(11134); }

// Sequence Number (Addr: 11138)
uint16_t iPM2xxx::Read_SequenceNumber_11138() { return readU16(11138); }

// Entry Number (Addr: 11139)
uint16_t iPM2xxx::Read_EntryNumber_11139() { return readU16(11139); }

// Date/Time (Addr: 11140)
uint16_t iPM2xxx::Read_DateTime_11140() { return readU16(11140); }

// Record Type (Addr: 11144)
uint16_t iPM2xxx::Read_RecordType_11144() { return readU16(11144); }

// Register Number or Event Code (Addr: 11145)
uint16_t iPM2xxx::Read_RegOrEventCode_11145() {
  return readU16(11145);
}

// Value (Addr: 11146)
uint16_t iPM2xxx::Read_Value_11146() { return readU16(11146); }

// Sequence Number (Addr: 11150)
uint16_t iPM2xxx::Read_SequenceNumber_11150() { return readU16(11150); }

// Entry Number (Addr: 11151)
uint16_t iPM2xxx::Read_EntryNumber_11151() { return readU16(11151); }

// Date/Time (Addr: 11152)
uint16_t iPM2xxx::Read_DateTime_11152() { return readU16(11152); }

// Record Type (Addr: 11156)
uint16_t iPM2xxx::Read_RecordType_11156() { return readU16(11156); }

// Register Number or Event Code (Addr: 11157)
uint16_t iPM2xxx::Read_RegOrEventCode_11157() {
  return readU16(11157);
}

// Value (Addr: 11158)
uint16_t iPM2xxx::Read_Value_11158() { return readU16(11158); }

// Sequence Number (Addr: 11162)
uint16_t iPM2xxx::Read_SequenceNumber_11162() { return readU16(11162); }

// Entry Number (Addr: 11163)
uint16_t iPM2xxx::Read_EntryNumber_11163() { return readU16(11163); }

// Date/Time (Addr: 11164)
uint16_t iPM2xxx::Read_DateTime_11164() { return readU16(11164); }

// Record Type (Addr: 11168)
uint16_t iPM2xxx::Read_RecordType_11168() { return readU16(11168); }

// Register Number or Event Code (Addr: 11169)
uint16_t iPM2xxx::Read_RegOrEventCode_11169() {
  return readU16(11169);
}

// Value (Addr: 11170)
uint16_t iPM2xxx::Read_Value_11170() { return readU16(11170); }

// Sequence Number (Addr: 11174)
uint16_t iPM2xxx::Read_SequenceNumber_11174() { return readU16(11174); }

// Entry Number (Addr: 11175)
uint16_t iPM2xxx::Read_EntryNumber_11175() { return readU16(11175); }

// Date/Time (Addr: 11176)
uint16_t iPM2xxx::Read_DateTime_11176() { return readU16(11176); }

// Record Type (Addr: 11180)
uint16_t iPM2xxx::Read_RecordType_11180() { return readU16(11180); }

// Register Number or Event Code (Addr: 11181)
uint16_t iPM2xxx::Read_RegOrEventCode_11181() {
  return readU16(11181);
}

// Value (Addr: 11182)
uint16_t iPM2xxx::Read_Value_11182() { return readU16(11182); }

// Sequence Number (Addr: 11186)
uint16_t iPM2xxx::Read_SequenceNumber_11186() { return readU16(11186); }

// Entry Number (Addr: 11187)
uint16_t iPM2xxx::Read_EntryNumber_11187() { return readU16(11187); }

// Date/Time (Addr: 11188)
uint16_t iPM2xxx::Read_DateTime_11188() { return readU16(11188); }

// Record Type (Addr: 11192)
uint16_t iPM2xxx::Read_RecordType_11192() { return readU16(11192); }

// Register Number or Event Code (Addr: 11193)
uint16_t iPM2xxx::Read_RegOrEventCode_11193() {
  return readU16(11193);
}

// Value (Addr: 11194)
uint16_t iPM2xxx::Read_Value_11194() { return readU16(11194); }

// Sequence Number (Addr: 11198)
uint16_t iPM2xxx::Read_SequenceNumber_11198() { return readU16(11198); }

// Entry Number (Addr: 11199)
uint16_t iPM2xxx::Read_EntryNumber_11199() { return readU16(11199); }

// Date/Time (Addr: 11200)
uint16_t iPM2xxx::Read_DateTime_11200() { return readU16(11200); }

// Record Type (Addr: 11204)
uint16_t iPM2xxx::Read_RecordType_11204() { return readU16(11204); }

// Register Number or Event Code (Addr: 11205)
uint16_t iPM2xxx::Read_RegOrEventCode_11205() {
  return readU16(11205);
}

// Value (Addr: 11206)
uint16_t iPM2xxx::Read_Value_11206() { return readU16(11206); }

// Sequence Number (Addr: 11210)
uint16_t iPM2xxx::Read_SequenceNumber_11210() { return readU16(11210); }

// Entry Number (Addr: 11211)
uint16_t iPM2xxx::Read_EntryNumber_11211() { return readU16(11211); }

// Date/Time (Addr: 11212)
uint16_t iPM2xxx::Read_DateTime_11212() { return readU16(11212); }

// Record Type (Addr: 11216)
uint16_t iPM2xxx::Read_RecordType_11216() { return readU16(11216); }

// Register Number or Event Code (Addr: 11217)
uint16_t iPM2xxx::Read_RegOrEventCode_11217() {
  return readU16(11217);
}

// Value (Addr: 11218)
uint16_t iPM2xxx::Read_Value_11218() { return readU16(11218); }

// Sequence Number (Addr: 11222)
uint16_t iPM2xxx::Read_SequenceNumber_11222() { return readU16(11222); }

// Entry Number (Addr: 11223)
uint16_t iPM2xxx::Read_EntryNumber_11223() { return readU16(11223); }

// Date/Time (Addr: 11224)
uint16_t iPM2xxx::Read_DateTime_11224() { return readU16(11224); }

// Record Type (Addr: 11228)
uint16_t iPM2xxx::Read_RecordType_11228() { return readU16(11228); }

// Register Number or Event Code (Addr: 11229)
uint16_t iPM2xxx::Read_RegOrEventCode_11229() {
  return readU16(11229);
}

// Value (Addr: 11230)
uint16_t iPM2xxx::Read_Value_11230() { return readU16(11230); }

// Sequence Number (Addr: 11234)
uint16_t iPM2xxx::Read_SequenceNumber_11234() { return readU16(11234); }

// Entry Number (Addr: 11235)
uint16_t iPM2xxx::Read_EntryNumber_11235() { return readU16(11235); }

// Date/Time (Addr: 11236)
uint16_t iPM2xxx::Read_DateTime_11236() { return readU16(11236); }

// Record Type (Addr: 11240)
uint16_t iPM2xxx::Read_RecordType_11240() { return readU16(11240); }

// Register Number or Event Code (Addr: 11241)
uint16_t iPM2xxx::Read_RegOrEventCode_11241() {
  return readU16(11241);
}

// Value (Addr: 11242)
uint16_t iPM2xxx::Read_Value_11242() { return readU16(11242); }

// Sequence Number (Addr: 11246)
uint16_t iPM2xxx::Read_SequenceNumber_11246() { return readU16(11246); }

// Entry Number (Addr: 11247)
uint16_t iPM2xxx::Read_EntryNumber_11247() { return readU16(11247); }

// Date/Time (Addr: 11248)
uint16_t iPM2xxx::Read_DateTime_11248() { return readU16(11248); }

// Record Type (Addr: 11252)
uint16_t iPM2xxx::Read_RecordType_11252() { return readU16(11252); }

// Register Number or Event Code (Addr: 11253)
uint16_t iPM2xxx::Read_RegOrEventCode_11253() {
  return readU16(11253);
}

// Value (Addr: 11254)
uint16_t iPM2xxx::Read_Value_11254() { return readU16(11254); }

// Sequence Number (Addr: 11258)
uint16_t iPM2xxx::Read_SequenceNumber_11258() { return readU16(11258); }

// Entry Number (Addr: 11259)
uint16_t iPM2xxx::Read_EntryNumber_11259() { return readU16(11259); }

// Date/Time (Addr: 11260)
uint16_t iPM2xxx::Read_DateTime_11260() { return readU16(11260); }

// Record Type (Addr: 11264)
uint16_t iPM2xxx::Read_RecordType_11264() { return readU16(11264); }

// Register Number or Event Code (Addr: 11265)
uint16_t iPM2xxx::Read_RegOrEventCode_11265() {
  return readU16(11265);
}

// Value (Addr: 11266)
uint16_t iPM2xxx::Read_Value_11266() { return readU16(11266); }

// Sequence Number (Addr: 11270)
uint16_t iPM2xxx::Read_SequenceNumber_11270() { return readU16(11270); }

// Entry Number (Addr: 11271)
uint16_t iPM2xxx::Read_EntryNumber_11271() { return readU16(11271); }

// Date/Time (Addr: 11272)
uint16_t iPM2xxx::Read_DateTime_11272() { return readU16(11272); }

// Record Type (Addr: 11276)
uint16_t iPM2xxx::Read_RecordType_11276() { return readU16(11276); }

// Register Number or Event Code (Addr: 11277)
uint16_t iPM2xxx::Read_RegOrEventCode_11277() {
  return readU16(11277);
}

// Value (Addr: 11278)
uint16_t iPM2xxx::Read_Value_11278() { return readU16(11278); }

// Sequence Number (Addr: 11282)
uint16_t iPM2xxx::Read_SequenceNumber_11282() { return readU16(11282); }

// Entry Number (Addr: 11283)
uint16_t iPM2xxx::Read_EntryNumber_11283() { return readU16(11283); }

// Date/Time (Addr: 11284)
uint16_t iPM2xxx::Read_DateTime_11284() { return readU16(11284); }

// Record Type (Addr: 11288)
uint16_t iPM2xxx::Read_RecordType_11288() { return readU16(11288); }

// Register Number or Event Code (Addr: 11289)
uint16_t iPM2xxx::Read_RegOrEventCode_11289() {
  return readU16(11289);
}

// Value (Addr: 11290)
uint16_t iPM2xxx::Read_Value_11290() { return readU16(11290); }

// Sequence Number (Addr: 11294)
uint16_t iPM2xxx::Read_SequenceNumber_11294() { return readU16(11294); }

// Entry Number (Addr: 11295)
uint16_t iPM2xxx::Read_EntryNumber_11295() { return readU16(11295); }

// Date/Time (Addr: 11296)
uint16_t iPM2xxx::Read_DateTime_11296() { return readU16(11296); }

// Record Type (Addr: 11300)
uint16_t iPM2xxx::Read_RecordType_11300() { return readU16(11300); }

// Register Number or Event Code (Addr: 11301)
uint16_t iPM2xxx::Read_RegOrEventCode_11301() {
  return readU16(11301);
}

// Value (Addr: 11302)
uint16_t iPM2xxx::Read_Value_11302() { return readU16(11302); }

// Sequence Number (Addr: 11306)
uint16_t iPM2xxx::Read_SequenceNumber_11306() { return readU16(11306); }

// Entry Number (Addr: 11307)
uint16_t iPM2xxx::Read_EntryNumber_11307() { return readU16(11307); }

// Date/Time (Addr: 11308)
uint16_t iPM2xxx::Read_DateTime_11308() { return readU16(11308); }

// Record Type (Addr: 11312)
uint16_t iPM2xxx::Read_RecordType_11312() { return readU16(11312); }

// Register Number or Event Code (Addr: 11313)
uint16_t iPM2xxx::Read_RegOrEventCode_11313() {
  return readU16(11313);
}

// Value (Addr: 11314)
uint16_t iPM2xxx::Read_Value_11314() { return readU16(11314); }

// Sequence Number (Addr: 11318)
uint16_t iPM2xxx::Read_SequenceNumber_11318() { return readU16(11318); }

// Entry Number (Addr: 11319)
uint16_t iPM2xxx::Read_EntryNumber_11319() { return readU16(11319); }

// Date/Time (Addr: 11320)
uint16_t iPM2xxx::Read_DateTime_11320() { return readU16(11320); }

// Record Type (Addr: 11324)
uint16_t iPM2xxx::Read_RecordType_11324() { return readU16(11324); }

// Register Number or Event Code (Addr: 11325)
uint16_t iPM2xxx::Read_RegOrEventCode_11325() {
  return readU16(11325);
}

// Value (Addr: 11326)
uint16_t iPM2xxx::Read_Value_11326() { return readU16(11326); }

// Sequence Number (Addr: 11330)
uint16_t iPM2xxx::Read_SequenceNumber_11330() { return readU16(11330); }

// Entry Number (Addr: 11331)
uint16_t iPM2xxx::Read_EntryNumber_11331() { return readU16(11331); }

// Date/Time (Addr: 11332)
uint16_t iPM2xxx::Read_DateTime_11332() { return readU16(11332); }

// Record Type (Addr: 11336)
uint16_t iPM2xxx::Read_RecordType_11336() { return readU16(11336); }

// Register Number or Event Code (Addr: 11337)
uint16_t iPM2xxx::Read_RegOrEventCode_11337() {
  return readU16(11337);
}

// Value (Addr: 11338)
uint16_t iPM2xxx::Read_Value_11338() { return readU16(11338); }

// Sequence Number (Addr: 11342)
uint16_t iPM2xxx::Read_SequenceNumber_11342() { return readU16(11342); }

// Entry Number (Addr: 11343)
uint16_t iPM2xxx::Read_EntryNumber_11343() { return readU16(11343); }

// Date/Time (Addr: 11344)
uint16_t iPM2xxx::Read_DateTime_11344() { return readU16(11344); }

// Record Type (Addr: 11348)
uint16_t iPM2xxx::Read_RecordType_11348() { return readU16(11348); }

// Register Number or Event Code (Addr: 11349)
uint16_t iPM2xxx::Read_RegOrEventCode_11349() {
  return readU16(11349);
}

// Value (Addr: 11350)
uint16_t iPM2xxx::Read_Value_11350() { return readU16(11350); }

// Sequence Number (Addr: 11354)
uint16_t iPM2xxx::Read_SequenceNumber_11354() { return readU16(11354); }

// Entry Number (Addr: 11355)
uint16_t iPM2xxx::Read_EntryNumber_11355() { return readU16(11355); }

// Date/Time (Addr: 11356)
uint16_t iPM2xxx::Read_DateTime_11356() { return readU16(11356); }

// Record Type (Addr: 11360)
uint16_t iPM2xxx::Read_RecordType_11360() { return readU16(11360); }

// Register Number or Event Code (Addr: 11361)
uint16_t iPM2xxx::Read_RegOrEventCode_11361() {
  return readU16(11361);
}

// Value (Addr: 11362)
uint16_t iPM2xxx::Read_Value_11362() { return readU16(11362); }

// Sequence Number (Addr: 11366)
uint16_t iPM2xxx::Read_SequenceNumber_11366() { return readU16(11366); }

// Entry Number (Addr: 11367)
uint16_t iPM2xxx::Read_EntryNumber_11367() { return readU16(11367); }

// Date/Time (Addr: 11368)
uint16_t iPM2xxx::Read_DateTime_11368() { return readU16(11368); }

// Record Type (Addr: 11372)
uint16_t iPM2xxx::Read_RecordType_11372() { return readU16(11372); }

// Register Number or Event Code (Addr: 11373)
uint16_t iPM2xxx::Read_RegOrEventCode_11373() {
  return readU16(11373);
}

// Value (Addr: 11374)
uint16_t iPM2xxx::Read_Value_11374() { return readU16(11374); }

// Sequence Number (Addr: 11378)
uint16_t iPM2xxx::Read_SequenceNumber_11378() { return readU16(11378); }

// Entry Number (Addr: 11379)
uint16_t iPM2xxx::Read_EntryNumber_11379() { return readU16(11379); }

// Date/Time (Addr: 11380)
uint16_t iPM2xxx::Read_DateTime_11380() { return readU16(11380); }

// Record Type (Addr: 11384)
uint16_t iPM2xxx::Read_RecordType_11384() { return readU16(11384); }

// Register Number or Event Code (Addr: 11385)
uint16_t iPM2xxx::Read_RegOrEventCode_11385() {
  return readU16(11385);
}

// Value (Addr: 11386)
uint16_t iPM2xxx::Read_Value_11386() { return readU16(11386); }

// Sequence Number (Addr: 11390)
uint16_t iPM2xxx::Read_SequenceNumber_11390() { return readU16(11390); }

// Entry Number (Addr: 11391)
uint16_t iPM2xxx::Read_EntryNumber_11391() { return readU16(11391); }

// Date/Time (Addr: 11392)
uint16_t iPM2xxx::Read_DateTime_11392() { return readU16(11392); }

// Record Type (Addr: 11396)
uint16_t iPM2xxx::Read_RecordType_11396() { return readU16(11396); }

// Register Number or Event Code (Addr: 11397)
uint16_t iPM2xxx::Read_RegOrEventCode_11397() {
  return readU16(11397);
}

// Value (Addr: 11398)
uint16_t iPM2xxx::Read_Value_11398() { return readU16(11398); }

// Sequence Number (Addr: 11402)
uint16_t iPM2xxx::Read_SequenceNumber_11402() { return readU16(11402); }

// Entry Number (Addr: 11403)
uint16_t iPM2xxx::Read_EntryNumber_11403() { return readU16(11403); }

// Date/Time (Addr: 11404)
uint16_t iPM2xxx::Read_DateTime_11404() { return readU16(11404); }

// Record Type (Addr: 11408)
uint16_t iPM2xxx::Read_RecordType_11408() { return readU16(11408); }

// Register Number or Event Code (Addr: 11409)
uint16_t iPM2xxx::Read_RegOrEventCode_11409() {
  return readU16(11409);
}

// Value (Addr: 11410)
uint16_t iPM2xxx::Read_Value_11410() { return readU16(11410); }

// Sequence Number (Addr: 11414)
uint16_t iPM2xxx::Read_SequenceNumber_11414() { return readU16(11414); }

// Entry Number (Addr: 11415)
uint16_t iPM2xxx::Read_EntryNumber_11415() { return readU16(11415); }

// Date/Time (Addr: 11416)
uint16_t iPM2xxx::Read_DateTime_11416() { return readU16(11416); }

// Record Type (Addr: 11420)
uint16_t iPM2xxx::Read_RecordType_11420() { return readU16(11420); }

// Register Number or Event Code (Addr: 11421)
uint16_t iPM2xxx::Read_RegOrEventCode_11421() {
  return readU16(11421);
}

// Value (Addr: 11422)
uint16_t iPM2xxx::Read_Value_11422() { return readU16(11422); }

// Sequence Number (Addr: 11426)
uint16_t iPM2xxx::Read_SequenceNumber_11426() { return readU16(11426); }

// Entry Number (Addr: 11427)
uint16_t iPM2xxx::Read_EntryNumber_11427() { return readU16(11427); }

// Date/Time (Addr: 11428)
uint16_t iPM2xxx::Read_DateTime_11428() { return readU16(11428); }

// Record Type (Addr: 11432)
uint16_t iPM2xxx::Read_RecordType_11432() { return readU16(11432); }

// Register Number or Event Code (Addr: 11433)
uint16_t iPM2xxx::Read_RegOrEventCode_11433() {
  return readU16(11433);
}

// Value (Addr: 11434)
uint16_t iPM2xxx::Read_Value_11434() { return readU16(11434); }

// Sequence Number (Addr: 11438)
uint16_t iPM2xxx::Read_SequenceNumber_11438() { return readU16(11438); }

// Entry Number (Addr: 11439)
uint16_t iPM2xxx::Read_EntryNumber_11439() { return readU16(11439); }

// Date/Time (Addr: 11440)
uint16_t iPM2xxx::Read_DateTime_11440() { return readU16(11440); }

// Record Type (Addr: 11444)
uint16_t iPM2xxx::Read_RecordType_11444() { return readU16(11444); }

// Register Number or Event Code (Addr: 11445)
uint16_t iPM2xxx::Read_RegOrEventCode_11445() {
  return readU16(11445);
}

// Value (Addr: 11446)
uint16_t iPM2xxx::Read_Value_11446() { return readU16(11446); }

// Sequence Number (Addr: 11450)
uint16_t iPM2xxx::Read_SequenceNumber_11450() { return readU16(11450); }

// Entry Number (Addr: 11451)
uint16_t iPM2xxx::Read_EntryNumber_11451() { return readU16(11451); }

// Date/Time (Addr: 11452)
uint16_t iPM2xxx::Read_DateTime_11452() { return readU16(11452); }

// Record Type (Addr: 11456)
uint16_t iPM2xxx::Read_RecordType_11456() { return readU16(11456); }

// Register Number or Event Code (Addr: 11457)
uint16_t iPM2xxx::Read_RegOrEventCode_11457() {
  return readU16(11457);
}

// Value (Addr: 11458)
uint16_t iPM2xxx::Read_Value_11458() { return readU16(11458); }

// Sequence Number (Addr: 11462)
uint16_t iPM2xxx::Read_SequenceNumber_11462() { return readU16(11462); }

// Entry Number (Addr: 11463)
uint16_t iPM2xxx::Read_EntryNumber_11463() { return readU16(11463); }

// Date/Time (Addr: 11464)
uint16_t iPM2xxx::Read_DateTime_11464() { return readU16(11464); }

// Record Type (Addr: 11468)
uint16_t iPM2xxx::Read_RecordType_11468() { return readU16(11468); }

// Register Number or Event Code (Addr: 11469)
uint16_t iPM2xxx::Read_RegOrEventCode_11469() {
  return readU16(11469);
}

// Value (Addr: 11470)
uint16_t iPM2xxx::Read_Value_11470() { return readU16(11470); }

// Sequence Number (Addr: 11474)
uint16_t iPM2xxx::Read_SequenceNumber_11474() { return readU16(11474); }

// Entry Number (Addr: 11475)
uint16_t iPM2xxx::Read_EntryNumber_11475() { return readU16(11475); }

// Date/Time (Addr: 11476)
uint16_t iPM2xxx::Read_DateTime_11476() { return readU16(11476); }

// Record Type (Addr: 11480)
uint16_t iPM2xxx::Read_RecordType_11480() { return readU16(11480); }

// Register Number or Event Code (Addr: 11481)
uint16_t iPM2xxx::Read_RegOrEventCode_11481() {
  return readU16(11481);
}

// Value (Addr: 11482)
uint16_t iPM2xxx::Read_Value_11482() { return readU16(11482); }

// Sequence Number (Addr: 11486)
uint16_t iPM2xxx::Read_SequenceNumber_11486() { return readU16(11486); }

// Entry Number (Addr: 11487)
uint16_t iPM2xxx::Read_EntryNumber_11487() { return readU16(11487); }

// Date/Time (Addr: 11488)
uint16_t iPM2xxx::Read_DateTime_11488() { return readU16(11488); }

// Record Type (Addr: 11492)
uint16_t iPM2xxx::Read_RecordType_11492() { return readU16(11492); }

// Register Number or Event Code (Addr: 11493)
uint16_t iPM2xxx::Read_RegOrEventCode_11493() {
  return readU16(11493);
}

// Value (Addr: 11494)
uint16_t iPM2xxx::Read_Value_11494() { return readU16(11494); }

// Sequence Number (Addr: 11498)
uint16_t iPM2xxx::Read_SequenceNumber_11498() { return readU16(11498); }

// Entry Number (Addr: 11499)
uint16_t iPM2xxx::Read_EntryNumber_11499() { return readU16(11499); }

// Date/Time (Addr: 11500)
uint16_t iPM2xxx::Read_DateTime_11500() { return readU16(11500); }

// Record Type (Addr: 11504)
uint16_t iPM2xxx::Read_RecordType_11504() { return readU16(11504); }

// Register Number or Event Code (Addr: 11505)
uint16_t iPM2xxx::Read_RegOrEventCode_11505() {
  return readU16(11505);
}

// Value (Addr: 11506)
uint16_t iPM2xxx::Read_Value_11506() { return readU16(11506); }

// Sequence Number (Addr: 11510)
uint16_t iPM2xxx::Read_SequenceNumber_11510() { return readU16(11510); }

// Entry Number (Addr: 11511)
uint16_t iPM2xxx::Read_EntryNumber_11511() { return readU16(11511); }

// Date/Time (Addr: 11512)
uint16_t iPM2xxx::Read_DateTime_11512() { return readU16(11512); }

// Record Type (Addr: 11516)
uint16_t iPM2xxx::Read_RecordType_11516() { return readU16(11516); }

// Register Number or Event Code (Addr: 11517)
uint16_t iPM2xxx::Read_RegOrEventCode_11517() {
  return readU16(11517);
}

// Value (Addr: 11518)
uint16_t iPM2xxx::Read_Value_11518() { return readU16(11518); }

// Sequence Number (Addr: 11522)
uint16_t iPM2xxx::Read_SequenceNumber_11522() { return readU16(11522); }

// Entry Number (Addr: 11523)
uint16_t iPM2xxx::Read_EntryNumber_11523() { return readU16(11523); }

// Date/Time (Addr: 11524)
uint16_t iPM2xxx::Read_DateTime_11524() { return readU16(11524); }

// Record Type (Addr: 11528)
uint16_t iPM2xxx::Read_RecordType_11528() { return readU16(11528); }

// Register Number or Event Code (Addr: 11529)
uint16_t iPM2xxx::Read_RegOrEventCode_11529() {
  return readU16(11529);
}

// Value (Addr: 11530)
uint16_t iPM2xxx::Read_Value_11530() { return readU16(11530); }

// Sequence Number (Addr: 11534)
uint16_t iPM2xxx::Read_SequenceNumber_11534() { return readU16(11534); }

// Entry Number (Addr: 11535)
uint16_t iPM2xxx::Read_EntryNumber_11535() { return readU16(11535); }

// Date/Time (Addr: 11536)
uint16_t iPM2xxx::Read_DateTime_11536() { return readU16(11536); }

// Record Type (Addr: 11540)
uint16_t iPM2xxx::Read_RecordType_11540() { return readU16(11540); }

// Register Number or Event Code (Addr: 11541)
uint16_t iPM2xxx::Read_RegOrEventCode_11541() {
  return readU16(11541);
}

// Value (Addr: 11542)
uint16_t iPM2xxx::Read_Value_11542() { return readU16(11542); }

// Sequence Number (Addr: 11546)
uint16_t iPM2xxx::Read_SequenceNumber_11546() { return readU16(11546); }

// Entry Number (Addr: 11547)
uint16_t iPM2xxx::Read_EntryNumber_11547() { return readU16(11547); }

// Date/Time (Addr: 11548)
uint16_t iPM2xxx::Read_DateTime_11548() { return readU16(11548); }

// Record Type (Addr: 11552)
uint16_t iPM2xxx::Read_RecordType_11552() { return readU16(11552); }

// Register Number or Event Code (Addr: 11553)
uint16_t iPM2xxx::Read_RegOrEventCode_11553() {
  return readU16(11553);
}

// Value (Addr: 11554)
uint16_t iPM2xxx::Read_Value_11554() { return readU16(11554); }

// Sequence Number (Addr: 11558)
uint16_t iPM2xxx::Read_SequenceNumber_11558() { return readU16(11558); }

// Entry Number (Addr: 11559)
uint16_t iPM2xxx::Read_EntryNumber_11559() { return readU16(11559); }

// Date/Time (Addr: 11560)
uint16_t iPM2xxx::Read_DateTime_11560() { return readU16(11560); }

// Record Type (Addr: 11564)
uint16_t iPM2xxx::Read_RecordType_11564() { return readU16(11564); }

// Register Number or Event Code (Addr: 11565)
uint16_t iPM2xxx::Read_RegOrEventCode_11565() {
  return readU16(11565);
}

// Value (Addr: 11566)
uint16_t iPM2xxx::Read_Value_11566() { return readU16(11566); }

// Sequence Number (Addr: 11570)
uint16_t iPM2xxx::Read_SequenceNumber_11570() { return readU16(11570); }

// Entry Number (Addr: 11571)
uint16_t iPM2xxx::Read_EntryNumber_11571() { return readU16(11571); }

// Date/Time (Addr: 11572)
uint16_t iPM2xxx::Read_DateTime_11572() { return readU16(11572); }

// Record Type (Addr: 11576)
uint16_t iPM2xxx::Read_RecordType_11576() { return readU16(11576); }

// Register Number or Event Code (Addr: 11577)
uint16_t iPM2xxx::Read_RegOrEventCode_11577() {
  return readU16(11577);
}

// Value (Addr: 11578)
uint16_t iPM2xxx::Read_Value_11578() { return readU16(11578); }

// Sequence Number (Addr: 11582)
uint16_t iPM2xxx::Read_SequenceNumber_11582() { return readU16(11582); }

// Entry Number (Addr: 11583)
uint16_t iPM2xxx::Read_EntryNumber_11583() { return readU16(11583); }

// Date/Time (Addr: 11584)
uint16_t iPM2xxx::Read_DateTime_11584() { return readU16(11584); }

// Record Type (Addr: 11588)
uint16_t iPM2xxx::Read_RecordType_11588() { return readU16(11588); }

// Register Number or Event Code (Addr: 11589)
uint16_t iPM2xxx::Read_RegOrEventCode_11589() {
  return readU16(11589);
}

// Value (Addr: 11590)
uint16_t iPM2xxx::Read_Value_11590() { return readU16(11590); }

// Sequence Number (Addr: 11594)
uint16_t iPM2xxx::Read_SequenceNumber_11594() { return readU16(11594); }

// Size of History Log (Addr: 12315)
uint16_t iPM2xxx::Read_SizeOfHistoryLog() { return readU16(12315); }

// Number of Entries in History Log (Addr: 12316)
uint16_t iPM2xxx::Read_NumberOfEntriesInHistoryLog() { return readU16(12316); }

// Entry Number of Most Recent Event (Addr: 12317)
uint16_t iPM2xxx::Read_EntryNumberOfMostRecentEvent_12317() {
  return readU16(12317);
}

// Entry Number (Addr: 12318)
uint16_t iPM2xxx::Read_EntryNumber_12318() { return readU16(12318); }

// Date/Time (Addr: 12319)
uint16_t iPM2xxx::Read_DateTime_12319() { return readU16(12319); }

// Record Type (Addr: 12323)
uint16_t iPM2xxx::Read_RecordType_12323() { return readU16(12323); }

// Register Number or Event Code (Addr: 12324)
uint16_t iPM2xxx::Read_RegOrEventCode_12324() {
  return readU16(12324);
}

// Value (Addr: 12325)
uint16_t iPM2xxx::Read_Value_12325() { return readU16(12325); }

// Sequence Number (Addr: 12329)
uint16_t iPM2xxx::Read_SequenceNumber_12329() { return readU16(12329); }

// Entry Number (Addr: 12330)
uint16_t iPM2xxx::Read_EntryNumber_12330() { return readU16(12330); }

// Date/Time (Addr: 12331)
uint16_t iPM2xxx::Read_DateTime_12331() { return readU16(12331); }

// Record Type (Addr: 12335)
uint16_t iPM2xxx::Read_RecordType_12335() { return readU16(12335); }

// Register Number or Event Code (Addr: 12336)
uint16_t iPM2xxx::Read_RegOrEventCode_12336() {
  return readU16(12336);
}

// Value (Addr: 12337)
uint16_t iPM2xxx::Read_Value_12337() { return readU16(12337); }

// Sequence Number (Addr: 12341)
uint16_t iPM2xxx::Read_SequenceNumber_12341() { return readU16(12341); }

// Entry Number (Addr: 12342)
uint16_t iPM2xxx::Read_EntryNumber_12342() { return readU16(12342); }

// Date/Time (Addr: 12343)
uint16_t iPM2xxx::Read_DateTime_12343() { return readU16(12343); }

// Record Type (Addr: 12347)
uint16_t iPM2xxx::Read_RecordType_12347() { return readU16(12347); }

// Register Number or Event Code (Addr: 12348)
uint16_t iPM2xxx::Read_RegOrEventCode_12348() {
  return readU16(12348);
}

// Value (Addr: 12349)
uint16_t iPM2xxx::Read_Value_12349() { return readU16(12349); }

// Sequence Number (Addr: 12353)
uint16_t iPM2xxx::Read_SequenceNumber_12353() { return readU16(12353); }

// Entry Number (Addr: 12354)
uint16_t iPM2xxx::Read_EntryNumber_12354() { return readU16(12354); }

// Date/Time (Addr: 12355)
uint16_t iPM2xxx::Read_DateTime_12355() { return readU16(12355); }

// Record Type (Addr: 12359)
uint16_t iPM2xxx::Read_RecordType_12359() { return readU16(12359); }

// Register Number or Event Code (Addr: 12360)
uint16_t iPM2xxx::Read_RegOrEventCode_12360() {
  return readU16(12360);
}

// Value (Addr: 12361)
uint16_t iPM2xxx::Read_Value_12361() { return readU16(12361); }

// Sequence Number (Addr: 12365)
uint16_t iPM2xxx::Read_SequenceNumber_12365() { return readU16(12365); }

// Entry Number (Addr: 12366)
uint16_t iPM2xxx::Read_EntryNumber_12366() { return readU16(12366); }

// Date/Time (Addr: 12367)
uint16_t iPM2xxx::Read_DateTime_12367() { return readU16(12367); }

// Record Type (Addr: 12371)
uint16_t iPM2xxx::Read_RecordType_12371() { return readU16(12371); }

// Register Number or Event Code (Addr: 12372)
uint16_t iPM2xxx::Read_RegOrEventCode_12372() {
  return readU16(12372);
}

// Value (Addr: 12373)
uint16_t iPM2xxx::Read_Value_12373() { return readU16(12373); }

// Sequence Number (Addr: 12377)
uint16_t iPM2xxx::Read_SequenceNumber_12377() { return readU16(12377); }

// Entry Number (Addr: 12378)
uint16_t iPM2xxx::Read_EntryNumber_12378() { return readU16(12378); }

// Date/Time (Addr: 12379)
uint16_t iPM2xxx::Read_DateTime_12379() { return readU16(12379); }

// Record Type (Addr: 12383)
uint16_t iPM2xxx::Read_RecordType_12383() { return readU16(12383); }

// Register Number or Event Code (Addr: 12384)
uint16_t iPM2xxx::Read_RegOrEventCode_12384() {
  return readU16(12384);
}

// Value (Addr: 12385)
uint16_t iPM2xxx::Read_Value_12385() { return readU16(12385); }

// Sequence Number (Addr: 12389)
uint16_t iPM2xxx::Read_SequenceNumber_12389() { return readU16(12389); }

// Entry Number (Addr: 12390)
uint16_t iPM2xxx::Read_EntryNumber_12390() { return readU16(12390); }

// Date/Time (Addr: 12391)
uint16_t iPM2xxx::Read_DateTime_12391() { return readU16(12391); }

// Record Type (Addr: 12395)
uint16_t iPM2xxx::Read_RecordType_12395() { return readU16(12395); }

// Register Number or Event Code (Addr: 12396)
uint16_t iPM2xxx::Read_RegOrEventCode_12396() {
  return readU16(12396);
}

// Value (Addr: 12397)
uint16_t iPM2xxx::Read_Value_12397() { return readU16(12397); }

// Sequence Number (Addr: 12401)
uint16_t iPM2xxx::Read_SequenceNumber_12401() { return readU16(12401); }

// Entry Number (Addr: 12402)
uint16_t iPM2xxx::Read_EntryNumber_12402() { return readU16(12402); }

// Date/Time (Addr: 12403)
uint16_t iPM2xxx::Read_DateTime_12403() { return readU16(12403); }

// Record Type (Addr: 12407)
uint16_t iPM2xxx::Read_RecordType_12407() { return readU16(12407); }

// Register Number or Event Code (Addr: 12408)
uint16_t iPM2xxx::Read_RegOrEventCode_12408() {
  return readU16(12408);
}

// Value (Addr: 12409)
uint16_t iPM2xxx::Read_Value_12409() { return readU16(12409); }

// Sequence Number (Addr: 12413)
uint16_t iPM2xxx::Read_SequenceNumber_12413() { return readU16(12413); }

// Entry Number (Addr: 12414)
uint16_t iPM2xxx::Read_EntryNumber_12414() { return readU16(12414); }

// Date/Time (Addr: 12415)
uint16_t iPM2xxx::Read_DateTime_12415() { return readU16(12415); }

// Record Type (Addr: 12419)
uint16_t iPM2xxx::Read_RecordType_12419() { return readU16(12419); }

// Register Number or Event Code (Addr: 12420)
uint16_t iPM2xxx::Read_RegOrEventCode_12420() {
  return readU16(12420);
}

// Value (Addr: 12421)
uint16_t iPM2xxx::Read_Value_12421() { return readU16(12421); }

// Sequence Number (Addr: 12425)
uint16_t iPM2xxx::Read_SequenceNumber_12425() { return readU16(12425); }

// Entry Number (Addr: 12426)
uint16_t iPM2xxx::Read_EntryNumber_12426() { return readU16(12426); }

// Date/Time (Addr: 12427)
uint16_t iPM2xxx::Read_DateTime_12427() { return readU16(12427); }

// Record Type (Addr: 12431)
uint16_t iPM2xxx::Read_RecordType_12431() { return readU16(12431); }

// Register Number or Event Code (Addr: 12432)
uint16_t iPM2xxx::Read_RegOrEventCode_12432() {
  return readU16(12432);
}

// Value (Addr: 12433)
uint16_t iPM2xxx::Read_Value_12433() { return readU16(12433); }

// Sequence Number (Addr: 12437)
uint16_t iPM2xxx::Read_SequenceNumber_12437() { return readU16(12437); }

// Entry Number (Addr: 12438)
uint16_t iPM2xxx::Read_EntryNumber_12438() { return readU16(12438); }

// Date/Time (Addr: 12439)
uint16_t iPM2xxx::Read_DateTime_12439() { return readU16(12439); }

// Record Type (Addr: 12443)
uint16_t iPM2xxx::Read_RecordType_12443() { return readU16(12443); }

// Register Number or Event Code (Addr: 12444)
uint16_t iPM2xxx::Read_RegOrEventCode_12444() {
  return readU16(12444);
}

// Value (Addr: 12445)
uint16_t iPM2xxx::Read_Value_12445() { return readU16(12445); }

// Sequence Number (Addr: 12449)
uint16_t iPM2xxx::Read_SequenceNumber_12449() { return readU16(12449); }

// Entry Number (Addr: 12450)
uint16_t iPM2xxx::Read_EntryNumber_12450() { return readU16(12450); }

// Date/Time (Addr: 12451)
uint16_t iPM2xxx::Read_DateTime_12451() { return readU16(12451); }

// Record Type (Addr: 12455)
uint16_t iPM2xxx::Read_RecordType_12455() { return readU16(12455); }

// Register Number or Event Code (Addr: 12456)
uint16_t iPM2xxx::Read_RegOrEventCode_12456() {
  return readU16(12456);
}

// Value (Addr: 12457)
uint16_t iPM2xxx::Read_Value_12457() { return readU16(12457); }

// Sequence Number (Addr: 12461)
uint16_t iPM2xxx::Read_SequenceNumber_12461() { return readU16(12461); }

// Entry Number (Addr: 12462)
uint16_t iPM2xxx::Read_EntryNumber_12462() { return readU16(12462); }

// Date/Time (Addr: 12463)
uint16_t iPM2xxx::Read_DateTime_12463() { return readU16(12463); }

// Record Type (Addr: 12467)
uint16_t iPM2xxx::Read_RecordType_12467() { return readU16(12467); }

// Register Number or Event Code (Addr: 12468)
uint16_t iPM2xxx::Read_RegOrEventCode_12468() {
  return readU16(12468);
}

// Value (Addr: 12469)
uint16_t iPM2xxx::Read_Value_12469() { return readU16(12469); }

// Sequence Number (Addr: 12473)
uint16_t iPM2xxx::Read_SequenceNumber_12473() { return readU16(12473); }

// Entry Number (Addr: 12474)
uint16_t iPM2xxx::Read_EntryNumber_12474() { return readU16(12474); }

// Date/Time (Addr: 12475)
uint16_t iPM2xxx::Read_DateTime_12475() { return readU16(12475); }

// Record Type (Addr: 12479)
uint16_t iPM2xxx::Read_RecordType_12479() { return readU16(12479); }

// Register Number or Event Code (Addr: 12480)
uint16_t iPM2xxx::Read_RegOrEventCode_12480() {
  return readU16(12480);
}

// Value (Addr: 12481)
uint16_t iPM2xxx::Read_Value_12481() { return readU16(12481); }

// Sequence Number (Addr: 12485)
uint16_t iPM2xxx::Read_SequenceNumber_12485() { return readU16(12485); }

// Entry Number (Addr: 12486)
uint16_t iPM2xxx::Read_EntryNumber_12486() { return readU16(12486); }

// Date/Time (Addr: 12487)
uint16_t iPM2xxx::Read_DateTime_12487() { return readU16(12487); }

// Record Type (Addr: 12491)
uint16_t iPM2xxx::Read_RecordType_12491() { return readU16(12491); }

// Register Number or Event Code (Addr: 12492)
uint16_t iPM2xxx::Read_RegOrEventCode_12492() {
  return readU16(12492);
}

// Value (Addr: 12493)
uint16_t iPM2xxx::Read_Value_12493() { return readU16(12493); }

// Sequence Number (Addr: 12497)
uint16_t iPM2xxx::Read_SequenceNumber_12497() { return readU16(12497); }

// Entry Number (Addr: 12498)
uint16_t iPM2xxx::Read_EntryNumber_12498() { return readU16(12498); }

// Date/Time (Addr: 12499)
uint16_t iPM2xxx::Read_DateTime_12499() { return readU16(12499); }

// Record Type (Addr: 12503)
uint16_t iPM2xxx::Read_RecordType_12503() { return readU16(12503); }

// Register Number or Event Code (Addr: 12504)
uint16_t iPM2xxx::Read_RegOrEventCode_12504() {
  return readU16(12504);
}

// Value (Addr: 12505)
uint16_t iPM2xxx::Read_Value_12505() { return readU16(12505); }

// Sequence Number (Addr: 12509)
uint16_t iPM2xxx::Read_SequenceNumber_12509() { return readU16(12509); }

// Entry Number (Addr: 12510)
uint16_t iPM2xxx::Read_EntryNumber_12510() { return readU16(12510); }

// Date/Time (Addr: 12511)
uint16_t iPM2xxx::Read_DateTime_12511() { return readU16(12511); }

// Record Type (Addr: 12515)
uint16_t iPM2xxx::Read_RecordType_12515() { return readU16(12515); }

// Register Number or Event Code (Addr: 12516)
uint16_t iPM2xxx::Read_RegOrEventCode_12516() {
  return readU16(12516);
}

// Value (Addr: 12517)
uint16_t iPM2xxx::Read_Value_12517() { return readU16(12517); }

// Sequence Number (Addr: 12521)
uint16_t iPM2xxx::Read_SequenceNumber_12521() { return readU16(12521); }

// Entry Number (Addr: 12522)
uint16_t iPM2xxx::Read_EntryNumber_12522() { return readU16(12522); }

// Date/Time (Addr: 12523)
uint16_t iPM2xxx::Read_DateTime_12523() { return readU16(12523); }

// Record Type (Addr: 12527)
uint16_t iPM2xxx::Read_RecordType_12527() { return readU16(12527); }

// Register Number or Event Code (Addr: 12528)
uint16_t iPM2xxx::Read_RegOrEventCode_12528() {
  return readU16(12528);
}

// Value (Addr: 12529)
uint16_t iPM2xxx::Read_Value_12529() { return readU16(12529); }

// Sequence Number (Addr: 12533)
uint16_t iPM2xxx::Read_SequenceNumber_12533() { return readU16(12533); }

// Entry Number (Addr: 12534)
uint16_t iPM2xxx::Read_EntryNumber_12534() { return readU16(12534); }

// Date/Time (Addr: 12535)
uint16_t iPM2xxx::Read_DateTime_12535() { return readU16(12535); }

// Record Type (Addr: 12539)
uint16_t iPM2xxx::Read_RecordType_12539() { return readU16(12539); }

// Register Number or Event Code (Addr: 12540)
uint16_t iPM2xxx::Read_RegOrEventCode_12540() {
  return readU16(12540);
}

// Value (Addr: 12541)
uint16_t iPM2xxx::Read_Value_12541() { return readU16(12541); }

// Sequence Number (Addr: 12545)
uint16_t iPM2xxx::Read_SequenceNumber_12545() { return readU16(12545); }

// Entry Number (Addr: 12546)
uint16_t iPM2xxx::Read_EntryNumber_12546() { return readU16(12546); }

// Date/Time (Addr: 12547)
uint16_t iPM2xxx::Read_DateTime_12547() { return readU16(12547); }

// Record Type (Addr: 12551)
uint16_t iPM2xxx::Read_RecordType_12551() { return readU16(12551); }

// Register Number or Event Code (Addr: 12552)
uint16_t iPM2xxx::Read_RegOrEventCode_12552() {
  return readU16(12552);
}

// Value (Addr: 12553)
uint16_t iPM2xxx::Read_Value_12553() { return readU16(12553); }

// Sequence Number (Addr: 12557)
uint16_t iPM2xxx::Read_SequenceNumber_12557() { return readU16(12557); }

// Entry Number (Addr: 12558)
uint16_t iPM2xxx::Read_EntryNumber_12558() { return readU16(12558); }

// Date/Time (Addr: 12559)
uint16_t iPM2xxx::Read_DateTime_12559() { return readU16(12559); }

// Record Type (Addr: 12563)
uint16_t iPM2xxx::Read_RecordType_12563() { return readU16(12563); }

// Register Number or Event Code (Addr: 12564)
uint16_t iPM2xxx::Read_RegOrEventCode_12564() {
  return readU16(12564);
}

// Value (Addr: 12565)
uint16_t iPM2xxx::Read_Value_12565() { return readU16(12565); }

// Sequence Number (Addr: 12569)
uint16_t iPM2xxx::Read_SequenceNumber_12569() { return readU16(12569); }

// Entry Number (Addr: 12570)
uint16_t iPM2xxx::Read_EntryNumber_12570() { return readU16(12570); }

// Date/Time (Addr: 12571)
uint16_t iPM2xxx::Read_DateTime_12571() { return readU16(12571); }

// Record Type (Addr: 12575)
uint16_t iPM2xxx::Read_RecordType_12575() { return readU16(12575); }

// Register Number or Event Code (Addr: 12576)
uint16_t iPM2xxx::Read_RegOrEventCode_12576() {
  return readU16(12576);
}

// Value (Addr: 12577)
uint16_t iPM2xxx::Read_Value_12577() { return readU16(12577); }

// Sequence Number (Addr: 12581)
uint16_t iPM2xxx::Read_SequenceNumber_12581() { return readU16(12581); }

// Entry Number (Addr: 12582)
uint16_t iPM2xxx::Read_EntryNumber_12582() { return readU16(12582); }

// Date/Time (Addr: 12583)
uint16_t iPM2xxx::Read_DateTime_12583() { return readU16(12583); }

// Record Type (Addr: 12587)
uint16_t iPM2xxx::Read_RecordType_12587() { return readU16(12587); }

// Register Number or Event Code (Addr: 12588)
uint16_t iPM2xxx::Read_RegOrEventCode_12588() {
  return readU16(12588);
}

// Value (Addr: 12589)
uint16_t iPM2xxx::Read_Value_12589() { return readU16(12589); }

// Sequence Number (Addr: 12593)
uint16_t iPM2xxx::Read_SequenceNumber_12593() { return readU16(12593); }

// Entry Number (Addr: 12594)
uint16_t iPM2xxx::Read_EntryNumber_12594() { return readU16(12594); }

// Date/Time (Addr: 12595)
uint16_t iPM2xxx::Read_DateTime_12595() { return readU16(12595); }

// Record Type (Addr: 12599)
uint16_t iPM2xxx::Read_RecordType_12599() { return readU16(12599); }

// Register Number or Event Code (Addr: 12600)
uint16_t iPM2xxx::Read_RegOrEventCode_12600() {
  return readU16(12600);
}

// Value (Addr: 12601)
uint16_t iPM2xxx::Read_Value_12601() { return readU16(12601); }

// Sequence Number (Addr: 12605)
uint16_t iPM2xxx::Read_SequenceNumber_12605() { return readU16(12605); }

// Entry Number (Addr: 12606)
uint16_t iPM2xxx::Read_EntryNumber_12606() { return readU16(12606); }

// Date/Time (Addr: 12607)
uint16_t iPM2xxx::Read_DateTime_12607() { return readU16(12607); }

// Record Type (Addr: 12611)
uint16_t iPM2xxx::Read_RecordType_12611() { return readU16(12611); }

// Register Number or Event Code (Addr: 12612)
uint16_t iPM2xxx::Read_RegOrEventCode_12612() {
  return readU16(12612);
}

// Value (Addr: 12613)
uint16_t iPM2xxx::Read_Value_12613() { return readU16(12613); }

// Sequence Number (Addr: 12617)
uint16_t iPM2xxx::Read_SequenceNumber_12617() { return readU16(12617); }

// Entry Number (Addr: 12618)
uint16_t iPM2xxx::Read_EntryNumber_12618() { return readU16(12618); }

// Date/Time (Addr: 12619)
uint16_t iPM2xxx::Read_DateTime_12619() { return readU16(12619); }

// Record Type (Addr: 12623)
uint16_t iPM2xxx::Read_RecordType_12623() { return readU16(12623); }

// Register Number or Event Code (Addr: 12624)
uint16_t iPM2xxx::Read_RegOrEventCode_12624() {
  return readU16(12624);
}

// Value (Addr: 12625)
uint16_t iPM2xxx::Read_Value_12625() { return readU16(12625); }

// Sequence Number (Addr: 12629)
uint16_t iPM2xxx::Read_SequenceNumber_12629() { return readU16(12629); }

// Entry Number (Addr: 12630)
uint16_t iPM2xxx::Read_EntryNumber_12630() { return readU16(12630); }

// Date/Time (Addr: 12631)
uint16_t iPM2xxx::Read_DateTime_12631() { return readU16(12631); }

// Record Type (Addr: 12635)
uint16_t iPM2xxx::Read_RecordType_12635() { return readU16(12635); }

// Register Number or Event Code (Addr: 12636)
uint16_t iPM2xxx::Read_RegOrEventCode_12636() {
  return readU16(12636);
}

// Value (Addr: 12637)
uint16_t iPM2xxx::Read_Value_12637() { return readU16(12637); }

// Sequence Number (Addr: 12641)
uint16_t iPM2xxx::Read_SequenceNumber_12641() { return readU16(12641); }

// Entry Number (Addr: 12642)
uint16_t iPM2xxx::Read_EntryNumber_12642() { return readU16(12642); }

// Date/Time (Addr: 12643)
uint16_t iPM2xxx::Read_DateTime_12643() { return readU16(12643); }

// Record Type (Addr: 12647)
uint16_t iPM2xxx::Read_RecordType_12647() { return readU16(12647); }

// Register Number or Event Code (Addr: 12648)
uint16_t iPM2xxx::Read_RegOrEventCode_12648() {
  return readU16(12648);
}

// Value (Addr: 12649)
uint16_t iPM2xxx::Read_Value_12649() { return readU16(12649); }

// Sequence Number (Addr: 12653)
uint16_t iPM2xxx::Read_SequenceNumber_12653() { return readU16(12653); }

// Entry Number (Addr: 12654)
uint16_t iPM2xxx::Read_EntryNumber_12654() { return readU16(12654); }

// Date/Time (Addr: 12655)
uint16_t iPM2xxx::Read_DateTime_12655() { return readU16(12655); }

// Record Type (Addr: 12659)
uint16_t iPM2xxx::Read_RecordType_12659() { return readU16(12659); }

// Register Number or Event Code (Addr: 12660)
uint16_t iPM2xxx::Read_RegOrEventCode_12660() {
  return readU16(12660);
}

// Value (Addr: 12661)
uint16_t iPM2xxx::Read_Value_12661() { return readU16(12661); }

// Sequence Number (Addr: 12665)
uint16_t iPM2xxx::Read_SequenceNumber_12665() { return readU16(12665); }

// Entry Number (Addr: 12666)
uint16_t iPM2xxx::Read_EntryNumber_12666() { return readU16(12666); }

// Date/Time (Addr: 12667)
uint16_t iPM2xxx::Read_DateTime_12667() { return readU16(12667); }

// Record Type (Addr: 12671)
uint16_t iPM2xxx::Read_RecordType_12671() { return readU16(12671); }

// Register Number or Event Code (Addr: 12672)
uint16_t iPM2xxx::Read_RegOrEventCode_12672() {
  return readU16(12672);
}

// Value (Addr: 12673)
uint16_t iPM2xxx::Read_Value_12673() { return readU16(12673); }

// Sequence Number (Addr: 12677)
uint16_t iPM2xxx::Read_SequenceNumber_12677() { return readU16(12677); }

// Entry Number (Addr: 12678)
uint16_t iPM2xxx::Read_EntryNumber_12678() { return readU16(12678); }

// Date/Time (Addr: 12679)
uint16_t iPM2xxx::Read_DateTime_12679() { return readU16(12679); }

// Record Type (Addr: 12683)
uint16_t iPM2xxx::Read_RecordType_12683() { return readU16(12683); }

// Register Number or Event Code (Addr: 12684)
uint16_t iPM2xxx::Read_RegOrEventCode_12684() {
  return readU16(12684);
}

// Value (Addr: 12685)
uint16_t iPM2xxx::Read_Value_12685() { return readU16(12685); }

// Sequence Number (Addr: 12689)
uint16_t iPM2xxx::Read_SequenceNumber_12689() { return readU16(12689); }

// Entry Number (Addr: 12690)
uint16_t iPM2xxx::Read_EntryNumber_12690() { return readU16(12690); }

// Date/Time (Addr: 12691)
uint16_t iPM2xxx::Read_DateTime_12691() { return readU16(12691); }

// Record Type (Addr: 12695)
uint16_t iPM2xxx::Read_RecordType_12695() { return readU16(12695); }

// Register Number or Event Code (Addr: 12696)
uint16_t iPM2xxx::Read_RegOrEventCode_12696() {
  return readU16(12696);
}

// Value (Addr: 12697)
uint16_t iPM2xxx::Read_Value_12697() { return readU16(12697); }

// Sequence Number (Addr: 12701)
uint16_t iPM2xxx::Read_SequenceNumber_12701() { return readU16(12701); }

// Entry Number (Addr: 12702)
uint16_t iPM2xxx::Read_EntryNumber_12702() { return readU16(12702); }

// Date/Time (Addr: 12703)
uint16_t iPM2xxx::Read_DateTime_12703() { return readU16(12703); }

// Record Type (Addr: 12707)
uint16_t iPM2xxx::Read_RecordType_12707() { return readU16(12707); }

// Register Number or Event Code (Addr: 12708)
uint16_t iPM2xxx::Read_RegOrEventCode_12708() {
  return readU16(12708);
}

// Value (Addr: 12709)
uint16_t iPM2xxx::Read_Value_12709() { return readU16(12709); }

// Sequence Number (Addr: 12713)
uint16_t iPM2xxx::Read_SequenceNumber_12713() { return readU16(12713); }

// Entry Number (Addr: 12714)
uint16_t iPM2xxx::Read_EntryNumber_12714() { return readU16(12714); }

// Date/Time (Addr: 12715)
uint16_t iPM2xxx::Read_DateTime_12715() { return readU16(12715); }

// Record Type (Addr: 12719)
uint16_t iPM2xxx::Read_RecordType_12719() { return readU16(12719); }

// Register Number or Event Code (Addr: 12720)
uint16_t iPM2xxx::Read_RegOrEventCode_12720() {
  return readU16(12720);
}

// Value (Addr: 12721)
uint16_t iPM2xxx::Read_Value_12721() { return readU16(12721); }

// Sequence Number (Addr: 12725)
uint16_t iPM2xxx::Read_SequenceNumber_12725() { return readU16(12725); }

// Entry Number (Addr: 12726)
uint16_t iPM2xxx::Read_EntryNumber_12726() { return readU16(12726); }

// Date/Time (Addr: 12727)
uint16_t iPM2xxx::Read_DateTime_12727() { return readU16(12727); }

// Record Type (Addr: 12731)
uint16_t iPM2xxx::Read_RecordType_12731() { return readU16(12731); }

// Register Number or Event Code (Addr: 12732)
uint16_t iPM2xxx::Read_RegOrEventCode_12732() {
  return readU16(12732);
}

// Value (Addr: 12733)
uint16_t iPM2xxx::Read_Value_12733() { return readU16(12733); }

// Sequence Number (Addr: 12737)
uint16_t iPM2xxx::Read_SequenceNumber_12737() { return readU16(12737); }

// Entry Number (Addr: 12738)
uint16_t iPM2xxx::Read_EntryNumber_12738() { return readU16(12738); }

// Date/Time (Addr: 12739)
uint16_t iPM2xxx::Read_DateTime_12739() { return readU16(12739); }

// Record Type (Addr: 12743)
uint16_t iPM2xxx::Read_RecordType_12743() { return readU16(12743); }

// Register Number or Event Code (Addr: 12744)
uint16_t iPM2xxx::Read_RegOrEventCode_12744() {
  return readU16(12744);
}

// Value (Addr: 12745)
uint16_t iPM2xxx::Read_Value_12745() { return readU16(12745); }

// Sequence Number (Addr: 12749)
uint16_t iPM2xxx::Read_SequenceNumber_12749() { return readU16(12749); }

// Entry Number (Addr: 12750)
uint16_t iPM2xxx::Read_EntryNumber_12750() { return readU16(12750); }

// Date/Time (Addr: 12751)
uint16_t iPM2xxx::Read_DateTime_12751() { return readU16(12751); }

// Record Type (Addr: 12755)
uint16_t iPM2xxx::Read_RecordType_12755() { return readU16(12755); }

// Register Number or Event Code (Addr: 12756)
uint16_t iPM2xxx::Read_RegOrEventCode_12756() {
  return readU16(12756);
}

// Value (Addr: 12757)
uint16_t iPM2xxx::Read_Value_12757() { return readU16(12757); }

// Sequence Number (Addr: 12761)
uint16_t iPM2xxx::Read_SequenceNumber_12761() { return readU16(12761); }

// Entry Number (Addr: 12762)
uint16_t iPM2xxx::Read_EntryNumber_12762() { return readU16(12762); }

// Date/Time (Addr: 12763)
uint16_t iPM2xxx::Read_DateTime_12763() { return readU16(12763); }

// Record Type (Addr: 12767)
uint16_t iPM2xxx::Read_RecordType_12767() { return readU16(12767); }

// Register Number or Event Code (Addr: 12768)
uint16_t iPM2xxx::Read_RegOrEventCode_12768() {
  return readU16(12768);
}

// Value (Addr: 12769)
uint16_t iPM2xxx::Read_Value_12769() { return readU16(12769); }

// Sequence Number (Addr: 12773)
uint16_t iPM2xxx::Read_SequenceNumber_12773() { return readU16(12773); }

// Entry Number (Addr: 12774)
uint16_t iPM2xxx::Read_EntryNumber_12774() { return readU16(12774); }

// Date/Time (Addr: 12775)
uint16_t iPM2xxx::Read_DateTime_12775() { return readU16(12775); }

// Record Type (Addr: 12779)
uint16_t iPM2xxx::Read_RecordType_12779() { return readU16(12779); }

// Register Number or Event Code (Addr: 12780)
uint16_t iPM2xxx::Read_RegOrEventCode_12780() {
  return readU16(12780);
}

// Value (Addr: 12781)
uint16_t iPM2xxx::Read_Value_12781() { return readU16(12781); }

// Sequence Number (Addr: 12785)
uint16_t iPM2xxx::Read_SequenceNumber_12785() { return readU16(12785); }

// Entry Number (Addr: 12786)
uint16_t iPM2xxx::Read_EntryNumber_12786() { return readU16(12786); }

// Date/Time (Addr: 12787)
uint16_t iPM2xxx::Read_DateTime_12787() { return readU16(12787); }

// Record Type (Addr: 12791)
uint16_t iPM2xxx::Read_RecordType_12791() { return readU16(12791); }

// Register Number or Event Code (Addr: 12792)
uint16_t iPM2xxx::Read_RegOrEventCode_12792() {
  return readU16(12792);
}

// Value (Addr: 12793)
uint16_t iPM2xxx::Read_Value_12793() { return readU16(12793); }

// Sequence Number (Addr: 12797)
uint16_t iPM2xxx::Read_SequenceNumber_12797() { return readU16(12797); }

// Total Counter (Addr: 13518)
uint16_t iPM2xxx::Read_TotalCounter() { return readU16(13518); }

// Over Current, Phase (Addr: 13522)
uint16_t iPM2xxx::Read_OverCurrentPhase() { return readU16(13522); }

// Under Current, Phase (Addr: 13523)
uint16_t iPM2xxx::Read_UnderCurrentPhase() { return readU16(13523); }

// Over Voltage, L-L (Addr: 13526)
uint16_t iPM2xxx::Read_OverVoltageLL() { return readU16(13526); }

// Under Voltage, L-L (Addr: 13527)
uint16_t iPM2xxx::Read_UnderVoltageLL() { return readU16(13527); }

// Over Voltage, L-N (Addr: 13528)
uint16_t iPM2xxx::Read_OverVoltageLN() { return readU16(13528); }

// Under Voltage, L-N (Addr: 13529)
uint16_t iPM2xxx::Read_UnderVoltageLN() { return readU16(13529); }

// Over Power, Active (Addr: 13530)
uint16_t iPM2xxx::Read_OverPowerActive() { return readU16(13530); }

// Over Power, Reactive (Addr: 13531)
uint16_t iPM2xxx::Read_OverPowerReactive() { return readU16(13531); }

// Over Power, Apparent (Addr: 13532)
uint16_t iPM2xxx::Read_OverPowerApparent() { return readU16(13532); }

// Lead Power Factor, True (Addr: 13533)
uint16_t iPM2xxx::Read_LeadPowerFactorTrue() { return readU16(13533); }

// Lag Power Factor, True (Addr: 13534)
uint16_t iPM2xxx::Read_LagPowerFactorTrue() { return readU16(13534); }

// Over Frequency (Addr: 13546)
uint16_t iPM2xxx::Read_OverFrequency() { return readU16(13546); }

// Under Frequency (Addr: 13547)
uint16_t iPM2xxx::Read_UnderFrequency() { return readU16(13547); }

// Over Voltage Total Harmonic Distortion (Addr: 13549)
uint16_t iPM2xxx::Read_OverVoltageTotalHarmonicDistortion() {
  return readU16(13549);
}

// Meter Powerup (Control Power Loss) (Addr: 13622)
uint16_t iPM2xxx::Read_MeterPowerupControlPowerLoss() { return readU16(13622); }

// Meter Reset (Addr: 13623)
uint16_t iPM2xxx::Read_MeterReset() { return readU16(13623); }

// Meter Diagnostic (Addr: 13624)
uint16_t iPM2xxx::Read_MeterDiagnostic() { return readU16(13624); }

// Phase Reversal (Addr: 13625)
uint16_t iPM2xxx::Read_PhaseReversal() { return readU16(13625); }

// Digital Alarm DI1 (Addr: 13632)
uint16_t iPM2xxx::Read_DigitalAlarmDi1() { return readU16(13632); }

// Digital Alarm DI2 (Addr: 13633)
uint16_t iPM2xxx::Read_DigitalAlarmDi2() { return readU16(13633); }

// Attributes (Addr: 13999)
uint16_t iPM2xxx::Read_Attributes() { return readU16(13999); }

// Source Register A (Addr: 14001)
uint16_t iPM2xxx::Read_SourceRegisterA() { return readU16(14001); }

// Source Register B (Addr: 14002)
uint16_t iPM2xxx::Read_SourceRegisterB() { return readU16(14002); }

// Source Register C (Addr: 14003)
uint16_t iPM2xxx::Read_SourceRegisterC() { return readU16(14003); }

// Pickup Setpoint (Addr: 14004)
float iPM2xxx::Read_PickupSetpoint() { return readFloat(14004); }

// Pickup Time Delay (Addr: 14006)
uint16_t iPM2xxx::Read_PickupTimeDelay() { return readU16(14006); }

// Dropout Setpoint (Addr: 14008)
float iPM2xxx::Read_DropoutSetpoint() { return readFloat(14008); }

// Dropout Time Delay (Addr: 14010)
uint16_t iPM2xxx::Read_DropoutTimeDelay() { return readU16(14010); }

// Digital Outputs to Associate � Base (Addr: 14012)
uint16_t iPM2xxx::Read_DigitalOutputsToAssociateBase() {
  return readU16(14012);
}

// Attributes (Addr: 14019)
uint16_t iPM2xxx::Read_Attributes_14019() { return readU16(14019); }

// Source Register A (Addr: 14021)
uint16_t iPM2xxx::Read_SourceRegisterA_14021() { return readU16(14021); }

// Source Register B (Addr: 14022)
uint16_t iPM2xxx::Read_SourceRegisterB_14022() { return readU16(14022); }

// Source Register C (Addr: 14023)
uint16_t iPM2xxx::Read_SourceRegisterC_14023() { return readU16(14023); }

// Pickup Setpoint (Addr: 14024)
float iPM2xxx::Read_PickupSetpoint_14024() { return readFloat(14024); }

// Pickup Time Delay (Addr: 14026)
uint16_t iPM2xxx::Read_PickupTimeDelay_14026() { return readU16(14026); }

// Dropout Setpoint (Addr: 14028)
float iPM2xxx::Read_DropoutSetpoint_14028() { return readFloat(14028); }

// Dropout Time Delay (Addr: 14030)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14030() { return readU16(14030); }

// Digital Outputs to Associate �  Standard (Addr: 14032)
uint16_t iPM2xxx::Read_DigitalOutputsToAssociateMinusstandard() {
  return readU16(14032);
}

// Attributes (Addr: 14079)
uint16_t iPM2xxx::Read_Attributes_14079() { return readU16(14079); }

// Source Register A (Addr: 14081)
uint16_t iPM2xxx::Read_SourceRegisterA_14081() { return readU16(14081); }

// Source Register B (Addr: 14082)
uint16_t iPM2xxx::Read_SourceRegisterB_14082() { return readU16(14082); }

// Source Register C (Addr: 14083)
uint16_t iPM2xxx::Read_SourceRegisterC_14083() { return readU16(14083); }

// Pickup Setpoint (Addr: 14084)
float iPM2xxx::Read_PickupSetpoint_14084() { return readFloat(14084); }

// Pickup Time Delay (Addr: 14086)
uint16_t iPM2xxx::Read_PickupTimeDelay_14086() { return readU16(14086); }

// Dropout Setpoint (Addr: 14088)
float iPM2xxx::Read_DropoutSetpoint_14088() { return readFloat(14088); }

// Dropout Time Delay (Addr: 14090)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14090() { return readU16(14090); }

// Digital Outputs to Associate �  Standard (Addr: 14092)
uint16_t iPM2xxx::Read_DigOutAssoc_14092() {
  return readU16(14092);
}

// Attributes (Addr: 14099)
uint16_t iPM2xxx::Read_Attributes_14099() { return readU16(14099); }

// Source Register A (Addr: 14101)
uint16_t iPM2xxx::Read_SourceRegisterA_14101() { return readU16(14101); }

// Source Register B (Addr: 14102)
uint16_t iPM2xxx::Read_SourceRegisterB_14102() { return readU16(14102); }

// Source Register C (Addr: 14103)
uint16_t iPM2xxx::Read_SourceRegisterC_14103() { return readU16(14103); }

// Pickup Setpoint (Addr: 14104)
float iPM2xxx::Read_PickupSetpoint_14104() { return readFloat(14104); }

// Pickup Time Delay (Addr: 14106)
uint16_t iPM2xxx::Read_PickupTimeDelay_14106() { return readU16(14106); }

// Dropout Setpoint (Addr: 14108)
float iPM2xxx::Read_DropoutSetpoint_14108() { return readFloat(14108); }

// Dropout Time Delay (Addr: 14110)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14110() { return readU16(14110); }

// Digital Outputs to Associate �  Standard (Addr: 14112)
uint16_t iPM2xxx::Read_DigOutAssoc_14112() {
  return readU16(14112);
}

// Attributes (Addr: 14119)
uint16_t iPM2xxx::Read_Attributes_14119() { return readU16(14119); }

// Source Register A (Addr: 14121)
uint16_t iPM2xxx::Read_SourceRegisterA_14121() { return readU16(14121); }

// Source Register B (Addr: 14122)
uint16_t iPM2xxx::Read_SourceRegisterB_14122() { return readU16(14122); }

// Source Register C (Addr: 14123)
uint16_t iPM2xxx::Read_SourceRegisterC_14123() { return readU16(14123); }

// Pickup Setpoint (Addr: 14124)
float iPM2xxx::Read_PickupSetpoint_14124() { return readFloat(14124); }

// Pickup Time Delay (Addr: 14126)
uint16_t iPM2xxx::Read_PickupTimeDelay_14126() { return readU16(14126); }

// Dropout Setpoint (Addr: 14128)
float iPM2xxx::Read_DropoutSetpoint_14128() { return readFloat(14128); }

// Dropout Time Delay (Addr: 14130)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14130() { return readU16(14130); }

// Digital Outputs to Associate �  Standard (Addr: 14132)
uint16_t iPM2xxx::Read_DigOutAssoc_14132() {
  return readU16(14132);
}

// Attributes (Addr: 14139)
uint16_t iPM2xxx::Read_Attributes_14139() { return readU16(14139); }

// Source Register A (Addr: 14141)
uint16_t iPM2xxx::Read_SourceRegisterA_14141() { return readU16(14141); }

// Source Register B (Addr: 14142)
uint16_t iPM2xxx::Read_SourceRegisterB_14142() { return readU16(14142); }

// Source Register C (Addr: 14143)
uint16_t iPM2xxx::Read_SourceRegisterC_14143() { return readU16(14143); }

// Pickup Setpoint (Addr: 14144)
float iPM2xxx::Read_PickupSetpoint_14144() { return readFloat(14144); }

// Pickup Time Delay (Addr: 14146)
uint16_t iPM2xxx::Read_PickupTimeDelay_14146() { return readU16(14146); }

// Dropout Setpoint (Addr: 14148)
float iPM2xxx::Read_DropoutSetpoint_14148() { return readFloat(14148); }

// Dropout Time Delay (Addr: 14150)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14150() { return readU16(14150); }

// Digital Outputs to Associate �  Standard (Addr: 14152)
uint16_t iPM2xxx::Read_DigOutAssoc_14152() {
  return readU16(14152);
}

// Attributes (Addr: 14159)
uint16_t iPM2xxx::Read_Attributes_14159() { return readU16(14159); }

// Source Register A (Addr: 14161)
uint16_t iPM2xxx::Read_SourceRegisterA_14161() { return readU16(14161); }

// Source Register B (Addr: 14162)
uint16_t iPM2xxx::Read_SourceRegisterB_14162() { return readU16(14162); }

// Source Register C (Addr: 14163)
uint16_t iPM2xxx::Read_SourceRegisterC_14163() { return readU16(14163); }

// Pickup Setpoint (Addr: 14164)
float iPM2xxx::Read_PickupSetpoint_14164() { return readFloat(14164); }

// Pickup Time Delay (Addr: 14166)
uint16_t iPM2xxx::Read_PickupTimeDelay_14166() { return readU16(14166); }

// Dropout Setpoint (Addr: 14168)
float iPM2xxx::Read_DropoutSetpoint_14168() { return readFloat(14168); }

// Dropout Time Delay (Addr: 14170)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14170() { return readU16(14170); }

// Digital Outputs to Associate �  Standard (Addr: 14172)
uint16_t iPM2xxx::Read_DigOutAssoc_14172() {
  return readU16(14172);
}

// Attributes (Addr: 14179)
uint16_t iPM2xxx::Read_Attributes_14179() { return readU16(14179); }

// Source Register A (Addr: 14181)
uint16_t iPM2xxx::Read_SourceRegisterA_14181() { return readU16(14181); }

// Source Register B (Addr: 14182)
uint16_t iPM2xxx::Read_SourceRegisterB_14182() { return readU16(14182); }

// Source Register C (Addr: 14183)
uint16_t iPM2xxx::Read_SourceRegisterC_14183() { return readU16(14183); }

// Pickup Setpoint (Addr: 14184)
float iPM2xxx::Read_PickupSetpoint_14184() { return readFloat(14184); }

// Pickup Time Delay (Addr: 14186)
uint16_t iPM2xxx::Read_PickupTimeDelay_14186() { return readU16(14186); }

// Dropout Setpoint (Addr: 14188)
float iPM2xxx::Read_DropoutSetpoint_14188() { return readFloat(14188); }

// Dropout Time Delay (Addr: 14190)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14190() { return readU16(14190); }

// Digital Outputs to Associate �  Standard (Addr: 14192)
uint16_t iPM2xxx::Read_DigOutAssoc_14192() {
  return readU16(14192);
}

// Attributes (Addr: 14199)
uint16_t iPM2xxx::Read_Attributes_14199() { return readU16(14199); }

// Source Register A (Addr: 14201)
uint16_t iPM2xxx::Read_SourceRegisterA_14201() { return readU16(14201); }

// Source Register B (Addr: 14202)
uint16_t iPM2xxx::Read_SourceRegisterB_14202() { return readU16(14202); }

// Source Register C (Addr: 14203)
uint16_t iPM2xxx::Read_SourceRegisterC_14203() { return readU16(14203); }

// Pickup Setpoint (Addr: 14204)
float iPM2xxx::Read_PickupSetpoint_14204() { return readFloat(14204); }

// Pickup Time Delay (Addr: 14206)
uint16_t iPM2xxx::Read_PickupTimeDelay_14206() { return readU16(14206); }

// Dropout Setpoint (Addr: 14208)
float iPM2xxx::Read_DropoutSetpoint_14208() { return readFloat(14208); }

// Dropout Time Delay (Addr: 14210)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14210() { return readU16(14210); }

// Digital Outputs to Associate �  Standard (Addr: 14212)
uint16_t iPM2xxx::Read_DigOutAssoc_14212() {
  return readU16(14212);
}

// Attributes (Addr: 14219)
uint16_t iPM2xxx::Read_Attributes_14219() { return readU16(14219); }

// Source Register A (Addr: 14221)
uint16_t iPM2xxx::Read_SourceRegisterA_14221() { return readU16(14221); }

// Pickup Setpoint (Addr: 14224)
float iPM2xxx::Read_PickupSetpoint_14224() { return readFloat(14224); }

// Pickup Time Delay (Addr: 14226)
uint16_t iPM2xxx::Read_PickupTimeDelay_14226() { return readU16(14226); }

// Dropout Setpoint (Addr: 14228)
float iPM2xxx::Read_DropoutSetpoint_14228() { return readFloat(14228); }

// Dropout Time Delay (Addr: 14230)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14230() { return readU16(14230); }

// Digital Outputs to Associate �  Standard (Addr: 14232)
uint16_t iPM2xxx::Read_DigOutAssoc_14232() {
  return readU16(14232);
}

// Attributes (Addr: 14239)
uint16_t iPM2xxx::Read_Attributes_14239() { return readU16(14239); }

// Source Register A (Addr: 14241)
uint16_t iPM2xxx::Read_SourceRegisterA_14241() { return readU16(14241); }

// Pickup Setpoint (Addr: 14244)
float iPM2xxx::Read_PickupSetpoint_14244() { return readFloat(14244); }

// Pickup Time Delay (Addr: 14246)
uint16_t iPM2xxx::Read_PickupTimeDelay_14246() { return readU16(14246); }

// Dropout Setpoint (Addr: 14248)
float iPM2xxx::Read_DropoutSetpoint_14248() { return readFloat(14248); }

// Dropout Time Delay (Addr: 14250)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14250() { return readU16(14250); }

// Digital Outputs to Associate �  Standard (Addr: 14252)
uint16_t iPM2xxx::Read_DigOutAssoc_14252() {
  return readU16(14252);
}

// Attributes (Addr: 14479)
uint16_t iPM2xxx::Read_Attributes_14479() { return readU16(14479); }

// Source Register A (Addr: 14481)
uint16_t iPM2xxx::Read_SourceRegisterA_14481() { return readU16(14481); }

// Pickup Setpoint (Addr: 14484)
float iPM2xxx::Read_PickupSetpoint_14484() { return readFloat(14484); }

// Pickup Time Delay (Addr: 14486)
uint16_t iPM2xxx::Read_PickupTimeDelay_14486() { return readU16(14486); }

// Dropout Setpoint (Addr: 14488)
float iPM2xxx::Read_DropoutSetpoint_14488() { return readFloat(14488); }

// Dropout Time Delay (Addr: 14490)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14490() { return readU16(14490); }

// Digital Outputs to Associate �  Standard (Addr: 14492)
uint16_t iPM2xxx::Read_DigOutAssoc_14492() {
  return readU16(14492);
}

// Attributes (Addr: 14499)
uint16_t iPM2xxx::Read_Attributes_14499() { return readU16(14499); }

// Source Register A (Addr: 14501)
uint16_t iPM2xxx::Read_SourceRegisterA_14501() { return readU16(14501); }

// Pickup Setpoint (Addr: 14504)
float iPM2xxx::Read_PickupSetpoint_14504() { return readFloat(14504); }

// Pickup Time Delay (Addr: 14506)
uint16_t iPM2xxx::Read_PickupTimeDelay_14506() { return readU16(14506); }

// Dropout Setpoint (Addr: 14508)
float iPM2xxx::Read_DropoutSetpoint_14508() { return readFloat(14508); }

// Dropout Time Delay (Addr: 14510)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14510() { return readU16(14510); }

// Digital Outputs to Associate �  Standard (Addr: 14512)
uint16_t iPM2xxx::Read_DigOutAssoc_14512() {
  return readU16(14512);
}

// Attributes (Addr: 14539)
uint16_t iPM2xxx::Read_Attributes_14539() { return readU16(14539); }

// Source Register A (Addr: 14541)
uint16_t iPM2xxx::Read_SourceRegisterA_14541() { return readU16(14541); }

// Source Register B (Addr: 14542)
uint16_t iPM2xxx::Read_SourceRegisterB_14542() { return readU16(14542); }

// Source Register C (Addr: 14543)
uint16_t iPM2xxx::Read_SourceRegisterC_14543() { return readU16(14543); }

// Pickup Setpoint (Addr: 14544)
float iPM2xxx::Read_PickupSetpoint_14544() { return readFloat(14544); }

// Pickup Time Delay (Addr: 14546)
uint16_t iPM2xxx::Read_PickupTimeDelay_14546() { return readU16(14546); }

// Dropout Setpoint (Addr: 14548)
float iPM2xxx::Read_DropoutSetpoint_14548() { return readFloat(14548); }

// Dropout Time Delay (Addr: 14550)
uint16_t iPM2xxx::Read_DropoutTimeDelay_14550() { return readU16(14550); }

// Digital Outputs to Associate �  Standard (Addr: 14552)
uint16_t iPM2xxx::Read_DigOutAssoc_14552() {
  return readU16(14552);
}

// Attributes (Addr: 16199)
uint16_t iPM2xxx::Read_Attributes_16199() { return readU16(16199); }

// Digital Outputs to Associate �  Standard (Addr: 16201)
uint16_t iPM2xxx::Read_DigOutAssoc_16201() {
  return readU16(16201);
}

// Attributes (Addr: 16209)
uint16_t iPM2xxx::Read_Attributes_16209() { return readU16(16209); }

// Digital Outputs to Associate �  Standard (Addr: 16211)
uint16_t iPM2xxx::Read_DigOutAssoc_16211() {
  return readU16(16211);
}

// Attributes (Addr: 16219)
uint16_t iPM2xxx::Read_Attributes_16219() { return readU16(16219); }

// Digital Outputs to Associate �  Standard (Addr: 16221)
uint16_t iPM2xxx::Read_DigOutAssoc_16221() {
  return readU16(16221);
}

// Attributes (Addr: 16229)
uint16_t iPM2xxx::Read_Attributes_16229() { return readU16(16229); }

// Digital Outputs to Associate �  Standard (Addr: 16231)
uint16_t iPM2xxx::Read_DigOutAssoc_16231() {
  return readU16(16231);
}

// Attributes (Addr: 16299)
uint16_t iPM2xxx::Read_Attributes_16299() { return readU16(16299); }

// Pickup Time Delay (Addr: 16301)
uint16_t iPM2xxx::Read_PickupTimeDelay_16301() { return readU16(16301); }

// Dropout Time Delay (Addr: 16303)
uint16_t iPM2xxx::Read_DropoutTimeDelay_16303() { return readU16(16303); }

// Digital Outputs to Associate �  Standard (Addr: 16305)
uint16_t iPM2xxx::Read_DigOutAssoc_16305() {
  return readU16(16305);
}

// Attributes (Addr: 16313)
uint16_t iPM2xxx::Read_Attributes_16313() { return readU16(16313); }

// Pickup Time Delay (Addr: 16315)
uint16_t iPM2xxx::Read_PickupTimeDelay_16315() { return readU16(16315); }

// Dropout Time Delay (Addr: 16317)
uint16_t iPM2xxx::Read_DropoutTimeDelay_16317() { return readU16(16317); }

// Digital Outputs to Associate �  Standard (Addr: 16319)
uint16_t iPM2xxx::Read_DigOutAssoc_16319() {
  return readU16(16319);
}

// Logging Status (Addr: 18999)
uint16_t iPM2xxx::Read_LoggingStatus() { return readU16(18999); }

// Allocated File Size (Addr: 19000)
uint16_t iPM2xxx::Read_AllocatedFileSize() { return readU16(19000); }

// Allocated Record Size (Addr: 19001)
uint16_t iPM2xxx::Read_AllocatedRecordSize() { return readU16(19001); }

// Record Management Method (Addr: 19002)
uint16_t iPM2xxx::Read_RecordManagementMethod() { return readU16(19002); }

// File Status (Addr: 19003)
uint16_t iPM2xxx::Read_FileStatus() { return readU16(19003); }

// Number of records in file (Addr: 19004)
uint16_t iPM2xxx::Read_NumberOfRecordsInFile() { return readU16(19004); }

// First Record Sequence Number (Addr: 19005)
uint16_t iPM2xxx::Read_FirstRecordSequenceNumber() { return readU16(19005); }

// Last Record Sequence Number (Addr: 19006)
uint16_t iPM2xxx::Read_LastRecordSequenceNumber() { return readU16(19006); }

// Topic Mode (Addr: 19007)
uint16_t iPM2xxx::Read_TopicMode() { return readU16(19007); }

// Start Time (Addr: 19008)
uint16_t iPM2xxx::Read_StartTime() { return readU16(19008); }

// Stop Time (Addr: 19009)
uint16_t iPM2xxx::Read_StopTime() { return readU16(19009); }

// Interval Control Minutes (Addr: 19010)
uint16_t iPM2xxx::Read_IntervalControlMinutes() { return readU16(19010); }

// Interval Control Seconds (Addr: 19011)
uint16_t iPM2xxx::Read_IntervalControlSeconds() { return readU16(19011); }

// Date/Time Last clear (Addr: 19012)
uint16_t iPM2xxx::Read_DateTimeLastClear() { return readU16(19012); }

// Record Item 1 (Addr: 19016)
uint16_t iPM2xxx::Read_RecordItem1() { return readU16(19016); }

// Record Item 2 (Addr: 19017)
uint16_t iPM2xxx::Read_RecordItem2() { return readU16(19017); }

// Processor Loading (Addr: 19999)
uint16_t iPM2xxx::Read_ProcessorLoading() { return readU16(19999); }

// Meter Self-Test (Addr: 20002)
uint16_t iPM2xxx::Read_MeterSelfTest() { return readU16(20002); }

// HS Frequency (Addr: 21015)
float iPM2xxx::Read_HsFrequency() { return readFloat(21015); }

// THD Current A (Addr: 21299)
float iPM2xxx::Read_ThdCurrentA() { return readFloat(21299); }

// THD Current B (Addr: 21301)
float iPM2xxx::Read_ThdCurrentB() { return readFloat(21301); }

// THD Current C (Addr: 21303)
float iPM2xxx::Read_ThdCurrentC() { return readFloat(21303); }

// THD Current N (Addr: 21305)
float iPM2xxx::Read_ThdCurrentN() { return readFloat(21305); }

// THD Current G (Addr: 21307)
float iPM2xxx::Read_ThdCurrentG() { return readFloat(21307); }

// THD Voltage A-B (Addr: 21321)
float iPM2xxx::Read_ThdVoltageAB() { return readFloat(21321); }

// THD Voltage B-C (Addr: 21323)
float iPM2xxx::Read_ThdVoltageBC() { return readFloat(21323); }

// THD Voltage C-A (Addr: 21325)
float iPM2xxx::Read_ThdVoltageCA() { return readFloat(21325); }

// THD Voltage L-L (Addr: 21327)
float iPM2xxx::Read_ThdVoltageLL() { return readFloat(21327); }

// THD Voltage A-N (Addr: 21329)
float iPM2xxx::Read_ThdVoltageAN() { return readFloat(21329); }

// THD Voltage B-N (Addr: 21331)
float iPM2xxx::Read_ThdVoltageBN() { return readFloat(21331); }

// THD Voltage C-N (Addr: 21333)
float iPM2xxx::Read_ThdVoltageCN() { return readFloat(21333); }

// THD Voltage L-N (Addr: 21337)
float iPM2xxx::Read_ThdVoltageLN() { return readFloat(21337); }

// Voltage A-B, H1 Magnitude, % (Addr: 21711)
float iPM2xxx::Read_VoltageABH1Magnitude() { return readFloat(21711); }

// Voltage A-B, H2 Magnitude, % (Addr: 21717)
float iPM2xxx::Read_VoltageABH2Magnitude() { return readFloat(21717); }

// Voltage A-B, H3 Magnitude, % (Addr: 21723)
float iPM2xxx::Read_VoltageABH3Magnitude() { return readFloat(21723); }

// Voltage A-B, H4 Magnitude, % (Addr: 21729)
float iPM2xxx::Read_VoltageABH4Magnitude() { return readFloat(21729); }

// Voltage A-B, H5 Magnitude, % (Addr: 21735)
float iPM2xxx::Read_VoltageABH5Magnitude() { return readFloat(21735); }

// Voltage A-B, H6 Magnitude, % (Addr: 21741)
float iPM2xxx::Read_VoltageABH6Magnitude() { return readFloat(21741); }

// Voltage A-B, H7 Magnitude, % (Addr: 21747)
float iPM2xxx::Read_VoltageABH7Magnitude() { return readFloat(21747); }

// Voltage A-B, H8 Magnitude, % (Addr: 21753)
float iPM2xxx::Read_VoltageABH8Magnitude() { return readFloat(21753); }

// Voltage A-B, H9 Magnitude, % (Addr: 21759)
float iPM2xxx::Read_VoltageABH9Magnitude() { return readFloat(21759); }

// Voltage A-B, H10 Magnitude, % (Addr: 21765)
float iPM2xxx::Read_VoltageABH10Magnitude() { return readFloat(21765); }

// Voltage A-B, H11 Magnitude, % (Addr: 21771)
float iPM2xxx::Read_VoltageABH11Magnitude() { return readFloat(21771); }

// Voltage A-B, H12 Magnitude, % (Addr: 21777)
float iPM2xxx::Read_VoltageABH12Magnitude() { return readFloat(21777); }

// Voltage A-B, H13 Magnitude, % (Addr: 21783)
float iPM2xxx::Read_VoltageABH13Magnitude() { return readFloat(21783); }

// Voltage A-B, H14 Magnitude, % (Addr: 21789)
float iPM2xxx::Read_VoltageABH14Magnitude() { return readFloat(21789); }

// Voltage A-B, H15 Magnitude, % (Addr: 21795)
float iPM2xxx::Read_VoltageABH15Magnitude() { return readFloat(21795); }

// Voltage A-B, H16 Magnitude, % (Addr: 21801)
float iPM2xxx::Read_VoltageABH16Magnitude() { return readFloat(21801); }

// Voltage A-B, H17 Magnitude, % (Addr: 21807)
float iPM2xxx::Read_VoltageABH17Magnitude() { return readFloat(21807); }

// Voltage A-B, H18 Magnitude, % (Addr: 21813)
float iPM2xxx::Read_VoltageABH18Magnitude() { return readFloat(21813); }

// Voltage A-B, H19 Magnitude, % (Addr: 21819)
float iPM2xxx::Read_VoltageABH19Magnitude() { return readFloat(21819); }

// Voltage A-B, H20 Magnitude, % (Addr: 21825)
float iPM2xxx::Read_VoltageABH20Magnitude() { return readFloat(21825); }

// Voltage A-B, H21 Magnitude, % (Addr: 21831)
float iPM2xxx::Read_VoltageABH21Magnitude() { return readFloat(21831); }

// Voltage A-B, H22 Magnitude, % (Addr: 21837)
float iPM2xxx::Read_VoltageABH22Magnitude() { return readFloat(21837); }

// Voltage A-B, H23 Magnitude, % (Addr: 21843)
float iPM2xxx::Read_VoltageABH23Magnitude() { return readFloat(21843); }

// Voltage A-B, H24 Magnitude, % (Addr: 21849)
float iPM2xxx::Read_VoltageABH24Magnitude() { return readFloat(21849); }

// Voltage A-B, H25 Magnitude, % (Addr: 21855)
float iPM2xxx::Read_VoltageABH25Magnitude() { return readFloat(21855); }

// Voltage A-B, H26 Magnitude, % (Addr: 21861)
float iPM2xxx::Read_VoltageABH26Magnitude() { return readFloat(21861); }

// Voltage A-B, H27 Magnitude, % (Addr: 21867)
float iPM2xxx::Read_VoltageABH27Magnitude() { return readFloat(21867); }

// Voltage A-B, H28 Magnitude, % (Addr: 21873)
float iPM2xxx::Read_VoltageABH28Magnitude() { return readFloat(21873); }

// Voltage A-B, H29 Magnitude, % (Addr: 21879)
float iPM2xxx::Read_VoltageABH29Magnitude() { return readFloat(21879); }

// Voltage A-B, H30 Magnitude, % (Addr: 21885)
float iPM2xxx::Read_VoltageABH30Magnitude() { return readFloat(21885); }

// Voltage A-B, H31 Magnitude, % (Addr: 21891)
float iPM2xxx::Read_VoltageABH31Magnitude() { return readFloat(21891); }

// Voltage B-C, H1 Magnitude, % (Addr: 22099)
float iPM2xxx::Read_VoltageBCH1Magnitude() { return readFloat(22099); }

// Voltage B-C, H2 Magnitude, % (Addr: 22105)
float iPM2xxx::Read_VoltageBCH2Magnitude() { return readFloat(22105); }

// Voltage B-C, H3 Magnitude, % (Addr: 22111)
float iPM2xxx::Read_VoltageBCH3Magnitude() { return readFloat(22111); }

// Voltage B-C, H4 Magnitude, % (Addr: 22117)
float iPM2xxx::Read_VoltageBCH4Magnitude() { return readFloat(22117); }

// Voltage B-C, H5 Magnitude, % (Addr: 22123)
float iPM2xxx::Read_VoltageBCH5Magnitude() { return readFloat(22123); }

// Voltage B-C, H6 Magnitude, % (Addr: 22129)
float iPM2xxx::Read_VoltageBCH6Magnitude() { return readFloat(22129); }

// Voltage B-C, H7 Magnitude, % (Addr: 22135)
float iPM2xxx::Read_VoltageBCH7Magnitude() { return readFloat(22135); }

// Voltage B-C, H8 Magnitude, % (Addr: 22141)
float iPM2xxx::Read_VoltageBCH8Magnitude() { return readFloat(22141); }

// Voltage B-C, H9 Magnitude, % (Addr: 22147)
float iPM2xxx::Read_VoltageBCH9Magnitude() { return readFloat(22147); }

// Voltage B-C, H10 Magnitude, % (Addr: 22153)
float iPM2xxx::Read_VoltageBCH10Magnitude() { return readFloat(22153); }

// Voltage B-C, H11 Magnitude, % (Addr: 22159)
float iPM2xxx::Read_VoltageBCH11Magnitude() { return readFloat(22159); }

// Voltage B-C, H12 Magnitude, % (Addr: 22165)
float iPM2xxx::Read_VoltageBCH12Magnitude() { return readFloat(22165); }

// Voltage B-C, H13 Magnitude, % (Addr: 22171)
float iPM2xxx::Read_VoltageBCH13Magnitude() { return readFloat(22171); }

// Voltage B-C, H14 Magnitude, % (Addr: 22177)
float iPM2xxx::Read_VoltageBCH14Magnitude() { return readFloat(22177); }

// Voltage B-C, H15 Magnitude, % (Addr: 22183)
float iPM2xxx::Read_VoltageBCH15Magnitude() { return readFloat(22183); }

// Voltage B-C, H16 Magnitude, % (Addr: 22189)
float iPM2xxx::Read_VoltageBCH16Magnitude() { return readFloat(22189); }

// Voltage B-C, H17 Magnitude, % (Addr: 22195)
float iPM2xxx::Read_VoltageBCH17Magnitude() { return readFloat(22195); }

// Voltage B-C, H18 Magnitude, % (Addr: 22201)
float iPM2xxx::Read_VoltageBCH18Magnitude() { return readFloat(22201); }

// Voltage B-C, H19 Magnitude, % (Addr: 22207)
float iPM2xxx::Read_VoltageBCH19Magnitude() { return readFloat(22207); }

// Voltage B-C, H20 Magnitude, % (Addr: 22213)
float iPM2xxx::Read_VoltageBCH20Magnitude() { return readFloat(22213); }

// Voltage B-C, H21 Magnitude, % (Addr: 22219)
float iPM2xxx::Read_VoltageBCH21Magnitude() { return readFloat(22219); }

// Voltage B-C, H22 Magnitude, % (Addr: 22225)
float iPM2xxx::Read_VoltageBCH22Magnitude() { return readFloat(22225); }

// Voltage B-C, H23 Magnitude, % (Addr: 22231)
float iPM2xxx::Read_VoltageBCH23Magnitude() { return readFloat(22231); }

// Voltage B-C, H24 Magnitude, % (Addr: 22237)
float iPM2xxx::Read_VoltageBCH24Magnitude() { return readFloat(22237); }

// Voltage B-C, H25 Magnitude, % (Addr: 22243)
float iPM2xxx::Read_VoltageBCH25Magnitude() { return readFloat(22243); }

// Voltage B-C, H26 Magnitude, % (Addr: 22249)
float iPM2xxx::Read_VoltageBCH26Magnitude() { return readFloat(22249); }

// Voltage B-C, H27 Magnitude, % (Addr: 22255)
float iPM2xxx::Read_VoltageBCH27Magnitude() { return readFloat(22255); }

// Voltage B-C, H28 Magnitude, % (Addr: 22261)
float iPM2xxx::Read_VoltageBCH28Magnitude() { return readFloat(22261); }

// Voltage B-C, H29 Magnitude, % (Addr: 22267)
float iPM2xxx::Read_VoltageBCH29Magnitude() { return readFloat(22267); }

// Voltage B-C, H30 Magnitude, % (Addr: 22273)
float iPM2xxx::Read_VoltageBCH30Magnitude() { return readFloat(22273); }

// Voltage B-C, H31 Magnitude, % (Addr: 22279)
float iPM2xxx::Read_VoltageBCH31Magnitude() { return readFloat(22279); }

// Voltage C-A, H1 Magnitude, % (Addr: 22487)
float iPM2xxx::Read_VoltageCAH1Magnitude() { return readFloat(22487); }

// Voltage C-A, H2 Magnitude, % (Addr: 22493)
float iPM2xxx::Read_VoltageCAH2Magnitude() { return readFloat(22493); }

// Voltage C-A, H3 Magnitude, % (Addr: 22499)
float iPM2xxx::Read_VoltageCAH3Magnitude() { return readFloat(22499); }

// Voltage C-A, H4 Magnitude, % (Addr: 22505)
float iPM2xxx::Read_VoltageCAH4Magnitude() { return readFloat(22505); }

// Voltage C-A, H5 Magnitude, % (Addr: 22511)
float iPM2xxx::Read_VoltageCAH5Magnitude() { return readFloat(22511); }

// Voltage C-A, H6 Magnitude, % (Addr: 22517)
float iPM2xxx::Read_VoltageCAH6Magnitude() { return readFloat(22517); }

// Voltage C-A, H7 Magnitude, % (Addr: 22523)
float iPM2xxx::Read_VoltageCAH7Magnitude() { return readFloat(22523); }

// Voltage C-A, H8 Magnitude, % (Addr: 22529)
float iPM2xxx::Read_VoltageCAH8Magnitude() { return readFloat(22529); }

// Voltage C-A, H9 Magnitude, % (Addr: 22535)
float iPM2xxx::Read_VoltageCAH9Magnitude() { return readFloat(22535); }

// Voltage C-A, H10 Magnitude, % (Addr: 22541)
float iPM2xxx::Read_VoltageCAH10Magnitude() { return readFloat(22541); }

// Voltage C-A, H11 Magnitude, % (Addr: 22547)
float iPM2xxx::Read_VoltageCAH11Magnitude() { return readFloat(22547); }

// Voltage C-A, H12 Magnitude, % (Addr: 22553)
float iPM2xxx::Read_VoltageCAH12Magnitude() { return readFloat(22553); }

// Voltage C-A, H13 Magnitude, % (Addr: 22559)
float iPM2xxx::Read_VoltageCAH13Magnitude() { return readFloat(22559); }

// Voltage C-A, H14 Magnitude, % (Addr: 22565)
float iPM2xxx::Read_VoltageCAH14Magnitude() { return readFloat(22565); }

// Voltage C-A, H15 Magnitude, % (Addr: 22571)
float iPM2xxx::Read_VoltageCAH15Magnitude() { return readFloat(22571); }

// Voltage C-A, H16 Magnitude, % (Addr: 22577)
float iPM2xxx::Read_VoltageCAH16Magnitude() { return readFloat(22577); }

// Voltage C-A, H17 Magnitude, % (Addr: 22583)
float iPM2xxx::Read_VoltageCAH17Magnitude() { return readFloat(22583); }

// Voltage C-A, H18 Magnitude, % (Addr: 22589)
float iPM2xxx::Read_VoltageCAH18Magnitude() { return readFloat(22589); }

// Voltage C-A, H19 Magnitude, % (Addr: 22595)
float iPM2xxx::Read_VoltageCAH19Magnitude() { return readFloat(22595); }

// Voltage C-A, H20 Magnitude, % (Addr: 22601)
float iPM2xxx::Read_VoltageCAH20Magnitude() { return readFloat(22601); }

// Voltage C-A, H21 Magnitude, % (Addr: 22607)
float iPM2xxx::Read_VoltageCAH21Magnitude() { return readFloat(22607); }

// Voltage C-A, H22 Magnitude, % (Addr: 22613)
float iPM2xxx::Read_VoltageCAH22Magnitude() { return readFloat(22613); }

// Voltage C-A, H23 Magnitude, % (Addr: 22619)
float iPM2xxx::Read_VoltageCAH23Magnitude() { return readFloat(22619); }

// Voltage C-A, H24 Magnitude, % (Addr: 22625)
float iPM2xxx::Read_VoltageCAH24Magnitude() { return readFloat(22625); }

// Voltage C-A, H25 Magnitude, % (Addr: 22631)
float iPM2xxx::Read_VoltageCAH25Magnitude() { return readFloat(22631); }

// Voltage C-A, H26 Magnitude, % (Addr: 22637)
float iPM2xxx::Read_VoltageCAH26Magnitude() { return readFloat(22637); }

// Voltage C-A, H27 Magnitude, % (Addr: 22643)
float iPM2xxx::Read_VoltageCAH27Magnitude() { return readFloat(22643); }

// Voltage C-A, H28 Magnitude, % (Addr: 22649)
float iPM2xxx::Read_VoltageCAH28Magnitude() { return readFloat(22649); }

// Voltage C-A, H29 Magnitude, % (Addr: 22655)
float iPM2xxx::Read_VoltageCAH29Magnitude() { return readFloat(22655); }

// Voltage C-A, H30 Magnitude, % (Addr: 22661)
float iPM2xxx::Read_VoltageCAH30Magnitude() { return readFloat(22661); }

// Voltage C-A, H31 Magnitude, % (Addr: 22667)
float iPM2xxx::Read_VoltageCAH31Magnitude() { return readFloat(22667); }

// Voltage A-N, H1 Magnitude, % (Addr: 22875)
float iPM2xxx::Read_VoltageANH1Magnitude() { return readFloat(22875); }

// Voltage A-N, H2 Magnitude, % (Addr: 22881)
float iPM2xxx::Read_VoltageANH2Magnitude() { return readFloat(22881); }

// Voltage A-N, H3 Magnitude, % (Addr: 22887)
float iPM2xxx::Read_VoltageANH3Magnitude() { return readFloat(22887); }

// Voltage A-N, H4 Magnitude, % (Addr: 22893)
float iPM2xxx::Read_VoltageANH4Magnitude() { return readFloat(22893); }

// Voltage A-N, H5 Magnitude, % (Addr: 22899)
float iPM2xxx::Read_VoltageANH5Magnitude() { return readFloat(22899); }

// Voltage A-N, H6 Magnitude, % (Addr: 22905)
float iPM2xxx::Read_VoltageANH6Magnitude() { return readFloat(22905); }

// Voltage A-N, H7 Magnitude, % (Addr: 22911)
float iPM2xxx::Read_VoltageANH7Magnitude() { return readFloat(22911); }

// Voltage A-N, H8 Magnitude, % (Addr: 22917)
float iPM2xxx::Read_VoltageANH8Magnitude() { return readFloat(22917); }

// Voltage A-N, H9 Magnitude, % (Addr: 22923)
float iPM2xxx::Read_VoltageANH9Magnitude() { return readFloat(22923); }

// Voltage A-N, H10 Magnitude, % (Addr: 22929)
float iPM2xxx::Read_VoltageANH10Magnitude() { return readFloat(22929); }

// Voltage A-N, H11 Magnitude, % (Addr: 22935)
float iPM2xxx::Read_VoltageANH11Magnitude() { return readFloat(22935); }

// Voltage A-N, H12 Magnitude, % (Addr: 22941)
float iPM2xxx::Read_VoltageANH12Magnitude() { return readFloat(22941); }

// Voltage A-N, H13 Magnitude, % (Addr: 22947)
float iPM2xxx::Read_VoltageANH13Magnitude() { return readFloat(22947); }

// Voltage A-N, H14 Magnitude, % (Addr: 22953)
float iPM2xxx::Read_VoltageANH14Magnitude() { return readFloat(22953); }

// Voltage A-N, H15 Magnitude, % (Addr: 22959)
float iPM2xxx::Read_VoltageANH15Magnitude() { return readFloat(22959); }

// Voltage A-N, H16 Magnitude, % (Addr: 22965)
float iPM2xxx::Read_VoltageANH16Magnitude() { return readFloat(22965); }

// Voltage A-N, H17 Magnitude, % (Addr: 22971)
float iPM2xxx::Read_VoltageANH17Magnitude() { return readFloat(22971); }

// Voltage A-N, H18 Magnitude, % (Addr: 22977)
float iPM2xxx::Read_VoltageANH18Magnitude() { return readFloat(22977); }

// Voltage A-N, H19 Magnitude, % (Addr: 22983)
float iPM2xxx::Read_VoltageANH19Magnitude() { return readFloat(22983); }

// Voltage A-N, H20 Magnitude, % (Addr: 22989)
float iPM2xxx::Read_VoltageANH20Magnitude() { return readFloat(22989); }

// Voltage A-N, H21 Magnitude, % (Addr: 22995)
float iPM2xxx::Read_VoltageANH21Magnitude() { return readFloat(22995); }

// Voltage A-N, H22 Magnitude, % (Addr: 23001)
float iPM2xxx::Read_VoltageANH22Magnitude() { return readFloat(23001); }

// Voltage A-N, H23 Magnitude, % (Addr: 23007)
float iPM2xxx::Read_VoltageANH23Magnitude() { return readFloat(23007); }

// Voltage A-N, H24 Magnitude, % (Addr: 23013)
float iPM2xxx::Read_VoltageANH24Magnitude() { return readFloat(23013); }

// Voltage A-N, H25 Magnitude, % (Addr: 23019)
float iPM2xxx::Read_VoltageANH25Magnitude() { return readFloat(23019); }

// Voltage A-N, H26 Magnitude, % (Addr: 23025)
float iPM2xxx::Read_VoltageANH26Magnitude() { return readFloat(23025); }

// Voltage A-N, H27 Magnitude, % (Addr: 23031)
float iPM2xxx::Read_VoltageANH27Magnitude() { return readFloat(23031); }

// Voltage A-N, H28 Magnitude, % (Addr: 23037)
float iPM2xxx::Read_VoltageANH28Magnitude() { return readFloat(23037); }

// Voltage A-N, H29 Magnitude, % (Addr: 23043)
float iPM2xxx::Read_VoltageANH29Magnitude() { return readFloat(23043); }

// Voltage A-N, H30 Magnitude, % (Addr: 23049)
float iPM2xxx::Read_VoltageANH30Magnitude() { return readFloat(23049); }

// Voltage A-N, H31 Magnitude, % (Addr: 23055)
float iPM2xxx::Read_VoltageANH31Magnitude() { return readFloat(23055); }

// Voltage B-N, H1 Magnitude, % (Addr: 23263)
float iPM2xxx::Read_VoltageBNH1Magnitude() { return readFloat(23263); }

// Voltage B-N, H2 Magnitude, % (Addr: 23269)
float iPM2xxx::Read_VoltageBNH2Magnitude() { return readFloat(23269); }

// Voltage B-N, H3 Magnitude, % (Addr: 23275)
float iPM2xxx::Read_VoltageBNH3Magnitude() { return readFloat(23275); }

// Voltage B-N, H4 Magnitude, % (Addr: 23281)
float iPM2xxx::Read_VoltageBNH4Magnitude() { return readFloat(23281); }

// Voltage B-N, H5 Magnitude, % (Addr: 23287)
float iPM2xxx::Read_VoltageBNH5Magnitude() { return readFloat(23287); }

// Voltage B-N, H6 Magnitude, % (Addr: 23293)
float iPM2xxx::Read_VoltageBNH6Magnitude() { return readFloat(23293); }

// Voltage B-N, H7 Magnitude, % (Addr: 23299)
float iPM2xxx::Read_VoltageBNH7Magnitude() { return readFloat(23299); }

// Voltage B-N, H8 Magnitude, % (Addr: 23305)
float iPM2xxx::Read_VoltageBNH8Magnitude() { return readFloat(23305); }

// Voltage B-N, H9 Magnitude, % (Addr: 23311)
float iPM2xxx::Read_VoltageBNH9Magnitude() { return readFloat(23311); }

// Voltage B-N, H10 Magnitude, % (Addr: 23317)
float iPM2xxx::Read_VoltageBNH10Magnitude() { return readFloat(23317); }

// Voltage B-N, H11 Magnitude, % (Addr: 23323)
float iPM2xxx::Read_VoltageBNH11Magnitude() { return readFloat(23323); }

// Voltage B-N, H12 Magnitude, % (Addr: 23329)
float iPM2xxx::Read_VoltageBNH12Magnitude() { return readFloat(23329); }

// Voltage B-N, H13 Magnitude, % (Addr: 23335)
float iPM2xxx::Read_VoltageBNH13Magnitude() { return readFloat(23335); }

// Voltage B-N, H14 Magnitude, % (Addr: 23341)
float iPM2xxx::Read_VoltageBNH14Magnitude() { return readFloat(23341); }

// Voltage B-N, H15 Magnitude, % (Addr: 23347)
float iPM2xxx::Read_VoltageBNH15Magnitude() { return readFloat(23347); }

// Voltage B-N, H16 Magnitude, % (Addr: 23353)
float iPM2xxx::Read_VoltageBNH16Magnitude() { return readFloat(23353); }

// Voltage B-N, H17 Magnitude, % (Addr: 23359)
float iPM2xxx::Read_VoltageBNH17Magnitude() { return readFloat(23359); }

// Voltage B-N, H18 Magnitude, % (Addr: 23365)
float iPM2xxx::Read_VoltageBNH18Magnitude() { return readFloat(23365); }

// Voltage B-N, H19 Magnitude, % (Addr: 23371)
float iPM2xxx::Read_VoltageBNH19Magnitude() { return readFloat(23371); }

// Voltage B-N, H20 Magnitude, % (Addr: 23377)
float iPM2xxx::Read_VoltageBNH20Magnitude() { return readFloat(23377); }

// Voltage B-N, H21 Magnitude, % (Addr: 23383)
float iPM2xxx::Read_VoltageBNH21Magnitude() { return readFloat(23383); }

// Voltage B-N, H22 Magnitude, % (Addr: 23389)
float iPM2xxx::Read_VoltageBNH22Magnitude() { return readFloat(23389); }

// Voltage B-N, H23 Magnitude, % (Addr: 23395)
float iPM2xxx::Read_VoltageBNH23Magnitude() { return readFloat(23395); }

// Voltage B-N, H24 Magnitude, % (Addr: 23401)
float iPM2xxx::Read_VoltageBNH24Magnitude() { return readFloat(23401); }

// Voltage B-N, H25 Magnitude, % (Addr: 23407)
float iPM2xxx::Read_VoltageBNH25Magnitude() { return readFloat(23407); }

// Voltage B-N, H26 Magnitude, % (Addr: 23413)
float iPM2xxx::Read_VoltageBNH26Magnitude() { return readFloat(23413); }

// Voltage B-N, H27 Magnitude, % (Addr: 23419)
float iPM2xxx::Read_VoltageBNH27Magnitude() { return readFloat(23419); }

// Voltage B-N, H28 Magnitude, % (Addr: 23425)
float iPM2xxx::Read_VoltageBNH28Magnitude() { return readFloat(23425); }

// Voltage B-N, H29 Magnitude, % (Addr: 23431)
float iPM2xxx::Read_VoltageBNH29Magnitude() { return readFloat(23431); }

// Voltage B-N, H30 Magnitude, % (Addr: 23437)
float iPM2xxx::Read_VoltageBNH30Magnitude() { return readFloat(23437); }

// Voltage B-N, H31 Magnitude, % (Addr: 23443)
float iPM2xxx::Read_VoltageBNH31Magnitude() { return readFloat(23443); }

// Voltage C-N, H1 Magnitude, % (Addr: 23651)
float iPM2xxx::Read_VoltageCNH1Magnitude() { return readFloat(23651); }

// Voltage C-N, H2 Magnitude, % (Addr: 23657)
float iPM2xxx::Read_VoltageCNH2Magnitude() { return readFloat(23657); }

// Voltage C-N, H3 Magnitude, % (Addr: 23663)
float iPM2xxx::Read_VoltageCNH3Magnitude() { return readFloat(23663); }

// Voltage C-N, H4 Magnitude, % (Addr: 23669)
float iPM2xxx::Read_VoltageCNH4Magnitude() { return readFloat(23669); }

// Voltage C-N, H5 Magnitude, % (Addr: 23675)
float iPM2xxx::Read_VoltageCNH5Magnitude() { return readFloat(23675); }

// Voltage C-N, H6 Magnitude, % (Addr: 23681)
float iPM2xxx::Read_VoltageCNH6Magnitude() { return readFloat(23681); }

// Voltage C-N, H7 Magnitude, % (Addr: 23687)
float iPM2xxx::Read_VoltageCNH7Magnitude() { return readFloat(23687); }

// Voltage C-N, H8 Magnitude, % (Addr: 23693)
float iPM2xxx::Read_VoltageCNH8Magnitude() { return readFloat(23693); }

// Voltage C-N, H9 Magnitude, % (Addr: 23699)
float iPM2xxx::Read_VoltageCNH9Magnitude() { return readFloat(23699); }

// Voltage C-N, H10 Magnitude, % (Addr: 23705)
float iPM2xxx::Read_VoltageCNH10Magnitude() { return readFloat(23705); }

// Voltage C-N, H11 Magnitude, % (Addr: 23711)
float iPM2xxx::Read_VoltageCNH11Magnitude() { return readFloat(23711); }

// Voltage C-N, H12 Magnitude, % (Addr: 23717)
float iPM2xxx::Read_VoltageCNH12Magnitude() { return readFloat(23717); }

// Voltage C-N, H13 Magnitude, % (Addr: 23723)
float iPM2xxx::Read_VoltageCNH13Magnitude() { return readFloat(23723); }

// Voltage C-N, H14 Magnitude, % (Addr: 23729)
float iPM2xxx::Read_VoltageCNH14Magnitude() { return readFloat(23729); }

// Voltage C-N, H15 Magnitude, % (Addr: 23735)
float iPM2xxx::Read_VoltageCNH15Magnitude() { return readFloat(23735); }

// Voltage C-N, H16 Magnitude, % (Addr: 23741)
float iPM2xxx::Read_VoltageCNH16Magnitude() { return readFloat(23741); }

// Voltage C-N, H17 Magnitude, % (Addr: 23747)
float iPM2xxx::Read_VoltageCNH17Magnitude() { return readFloat(23747); }

// Voltage C-N, H18 Magnitude, % (Addr: 23753)
float iPM2xxx::Read_VoltageCNH18Magnitude() { return readFloat(23753); }

// Voltage C-N, H19 Magnitude, % (Addr: 23759)
float iPM2xxx::Read_VoltageCNH19Magnitude() { return readFloat(23759); }

// Voltage C-N, H20 Magnitude, % (Addr: 23765)
float iPM2xxx::Read_VoltageCNH20Magnitude() { return readFloat(23765); }

// Voltage C-N, H21 Magnitude, % (Addr: 23771)
float iPM2xxx::Read_VoltageCNH21Magnitude() { return readFloat(23771); }

// Voltage C-N, H22 Magnitude, % (Addr: 23777)
float iPM2xxx::Read_VoltageCNH22Magnitude() { return readFloat(23777); }

// Voltage C-N, H23 Magnitude, % (Addr: 23783)
float iPM2xxx::Read_VoltageCNH23Magnitude() { return readFloat(23783); }

// Voltage C-N, H24 Magnitude, % (Addr: 23789)
float iPM2xxx::Read_VoltageCNH24Magnitude() { return readFloat(23789); }

// Voltage C-N, H25 Magnitude, % (Addr: 23795)
float iPM2xxx::Read_VoltageCNH25Magnitude() { return readFloat(23795); }

// Voltage C-N, H26 Magnitude, % (Addr: 23801)
float iPM2xxx::Read_VoltageCNH26Magnitude() { return readFloat(23801); }

// Voltage C-N, H27 Magnitude, % (Addr: 23807)
float iPM2xxx::Read_VoltageCNH27Magnitude() { return readFloat(23807); }

// Voltage C-N, H28 Magnitude, % (Addr: 23813)
float iPM2xxx::Read_VoltageCNH28Magnitude() { return readFloat(23813); }

// Voltage C-N, H29 Magnitude, % (Addr: 23819)
float iPM2xxx::Read_VoltageCNH29Magnitude() { return readFloat(23819); }

// Voltage C-N, H30 Magnitude, % (Addr: 23825)
float iPM2xxx::Read_VoltageCNH30Magnitude() { return readFloat(23825); }

// Voltage C-N, H31 Magnitude, % (Addr: 23831)
float iPM2xxx::Read_VoltageCNH31Magnitude() { return readFloat(23831); }

// Current A, H1 Magnitude, % (Addr: 24427)
float iPM2xxx::Read_CurrentAH1Magnitude() { return readFloat(24427); }

// Current A, H2 Magnitude, % (Addr: 24433)
float iPM2xxx::Read_CurrentAH2Magnitude() { return readFloat(24433); }

// Current A, H3 Magnitude, % (Addr: 24439)
float iPM2xxx::Read_CurrentAH3Magnitude() { return readFloat(24439); }

// Current A, H4 Magnitude, % (Addr: 24445)
float iPM2xxx::Read_CurrentAH4Magnitude() { return readFloat(24445); }

// Current A, H5 Magnitude, % (Addr: 24451)
float iPM2xxx::Read_CurrentAH5Magnitude() { return readFloat(24451); }

// Current A, H6 Magnitude, % (Addr: 24457)
float iPM2xxx::Read_CurrentAH6Magnitude() { return readFloat(24457); }

// Current A, H7 Magnitude, % (Addr: 24463)
float iPM2xxx::Read_CurrentAH7Magnitude() { return readFloat(24463); }

// Current A, H8 Magnitude, % (Addr: 24469)
float iPM2xxx::Read_CurrentAH8Magnitude() { return readFloat(24469); }

// Current A, H9 Magnitude, % (Addr: 24475)
float iPM2xxx::Read_CurrentAH9Magnitude() { return readFloat(24475); }

// Current A, H10 Magnitude, % (Addr: 24481)
float iPM2xxx::Read_CurrentAH10Magnitude() { return readFloat(24481); }

// Current A, H11 Magnitude, % (Addr: 24487)
float iPM2xxx::Read_CurrentAH11Magnitude() { return readFloat(24487); }

// Current A, H12 Magnitude, % (Addr: 24493)
float iPM2xxx::Read_CurrentAH12Magnitude() { return readFloat(24493); }

// Current A, H13 Magnitude, % (Addr: 24499)
float iPM2xxx::Read_CurrentAH13Magnitude() { return readFloat(24499); }

// Current A, H14 Magnitude, % (Addr: 24505)
float iPM2xxx::Read_CurrentAH14Magnitude() { return readFloat(24505); }

// Current A, H15 Magnitude, % (Addr: 24511)
float iPM2xxx::Read_CurrentAH15Magnitude() { return readFloat(24511); }

// Current A, H16 Magnitude, % (Addr: 24517)
float iPM2xxx::Read_CurrentAH16Magnitude() { return readFloat(24517); }

// Current A, H17 Magnitude, % (Addr: 24523)
float iPM2xxx::Read_CurrentAH17Magnitude() { return readFloat(24523); }

// Current A, H18 Magnitude, % (Addr: 24529)
float iPM2xxx::Read_CurrentAH18Magnitude() { return readFloat(24529); }

// Current A, H19 Magnitude, % (Addr: 24535)
float iPM2xxx::Read_CurrentAH19Magnitude() { return readFloat(24535); }

// Current A, H20 Magnitude, % (Addr: 24541)
float iPM2xxx::Read_CurrentAH20Magnitude() { return readFloat(24541); }

// Current A, H21 Magnitude, % (Addr: 24547)
float iPM2xxx::Read_CurrentAH21Magnitude() { return readFloat(24547); }

// Current A, H22 Magnitude, % (Addr: 24553)
float iPM2xxx::Read_CurrentAH22Magnitude() { return readFloat(24553); }

// Current A, H23 Magnitude, % (Addr: 24559)
float iPM2xxx::Read_CurrentAH23Magnitude() { return readFloat(24559); }

// Current A, H24 Magnitude, % (Addr: 24565)
float iPM2xxx::Read_CurrentAH24Magnitude() { return readFloat(24565); }

// Current A, H25 Magnitude, % (Addr: 24571)
float iPM2xxx::Read_CurrentAH25Magnitude() { return readFloat(24571); }

// Current A, H26 Magnitude, % (Addr: 24577)
float iPM2xxx::Read_CurrentAH26Magnitude() { return readFloat(24577); }

// Current A, H27 Magnitude, % (Addr: 24583)
float iPM2xxx::Read_CurrentAH27Magnitude() { return readFloat(24583); }

// Current A, H28 Magnitude, % (Addr: 24589)
float iPM2xxx::Read_CurrentAH28Magnitude() { return readFloat(24589); }

// Current A, H29 Magnitude, % (Addr: 24595)
float iPM2xxx::Read_CurrentAH29Magnitude() { return readFloat(24595); }

// Current A, H30 Magnitude, % (Addr: 24601)
float iPM2xxx::Read_CurrentAH30Magnitude() { return readFloat(24601); }

// Current A, H31 Magnitude, % (Addr: 24607)
float iPM2xxx::Read_CurrentAH31Magnitude() { return readFloat(24607); }

// Current B, H1 Magnitude, % (Addr: 24815)
float iPM2xxx::Read_CurrentBH1Magnitude() { return readFloat(24815); }

// Current B, H2 Magnitude, % (Addr: 24821)
float iPM2xxx::Read_CurrentBH2Magnitude() { return readFloat(24821); }

// Current B, H3 Magnitude, % (Addr: 24827)
float iPM2xxx::Read_CurrentBH3Magnitude() { return readFloat(24827); }

// Current B, H4 Magnitude, % (Addr: 24833)
float iPM2xxx::Read_CurrentBH4Magnitude() { return readFloat(24833); }

// Current B, H5 Magnitude, % (Addr: 24839)
float iPM2xxx::Read_CurrentBH5Magnitude() { return readFloat(24839); }

// Current B, H6 Magnitude, % (Addr: 24845)
float iPM2xxx::Read_CurrentBH6Magnitude() { return readFloat(24845); }

// Current B, H7 Magnitude, % (Addr: 24851)
float iPM2xxx::Read_CurrentBH7Magnitude() { return readFloat(24851); }

// Current B, H8 Magnitude, % (Addr: 24857)
float iPM2xxx::Read_CurrentBH8Magnitude() { return readFloat(24857); }

// Current B, H9 Magnitude, % (Addr: 24863)
float iPM2xxx::Read_CurrentBH9Magnitude() { return readFloat(24863); }

// Current B, H10 Magnitude, % (Addr: 24869)
float iPM2xxx::Read_CurrentBH10Magnitude() { return readFloat(24869); }

// Current B, H11 Magnitude, % (Addr: 24875)
float iPM2xxx::Read_CurrentBH11Magnitude() { return readFloat(24875); }

// Current B, H12 Magnitude, % (Addr: 24881)
float iPM2xxx::Read_CurrentBH12Magnitude() { return readFloat(24881); }

// Current B, H13 Magnitude, % (Addr: 24887)
float iPM2xxx::Read_CurrentBH13Magnitude() { return readFloat(24887); }

// Current B, H14 Magnitude, % (Addr: 24893)
float iPM2xxx::Read_CurrentBH14Magnitude() { return readFloat(24893); }

// Current B, H15 Magnitude, % (Addr: 24899)
float iPM2xxx::Read_CurrentBH15Magnitude() { return readFloat(24899); }

// Current B, H16 Magnitude, % (Addr: 24905)
float iPM2xxx::Read_CurrentBH16Magnitude() { return readFloat(24905); }

// Current B, H17 Magnitude, % (Addr: 24911)
float iPM2xxx::Read_CurrentBH17Magnitude() { return readFloat(24911); }

// Current B, H18 Magnitude, % (Addr: 24917)
float iPM2xxx::Read_CurrentBH18Magnitude() { return readFloat(24917); }

// Current B, H19 Magnitude, % (Addr: 24923)
float iPM2xxx::Read_CurrentBH19Magnitude() { return readFloat(24923); }

// Current B, H20 Magnitude, % (Addr: 24929)
float iPM2xxx::Read_CurrentBH20Magnitude() { return readFloat(24929); }

// Current B, H21 Magnitude, % (Addr: 24935)
float iPM2xxx::Read_CurrentBH21Magnitude() { return readFloat(24935); }

// Current B, H22 Magnitude, % (Addr: 24941)
float iPM2xxx::Read_CurrentBH22Magnitude() { return readFloat(24941); }

// Current B, H23 Magnitude, % (Addr: 24947)
float iPM2xxx::Read_CurrentBH23Magnitude() { return readFloat(24947); }

// Current B, H24 Magnitude, % (Addr: 24953)
float iPM2xxx::Read_CurrentBH24Magnitude() { return readFloat(24953); }

// Current B, H25 Magnitude, % (Addr: 24959)
float iPM2xxx::Read_CurrentBH25Magnitude() { return readFloat(24959); }

// Current B, H26 Magnitude, % (Addr: 24965)
float iPM2xxx::Read_CurrentBH26Magnitude() { return readFloat(24965); }

// Current B, H27 Magnitude, % (Addr: 24971)
float iPM2xxx::Read_CurrentBH27Magnitude() { return readFloat(24971); }

// Current B, H28 Magnitude, % (Addr: 24977)
float iPM2xxx::Read_CurrentBH28Magnitude() { return readFloat(24977); }

// Current B, H29 Magnitude, % (Addr: 24983)
float iPM2xxx::Read_CurrentBH29Magnitude() { return readFloat(24983); }

// Current B, H30 Magnitude, % (Addr: 24989)
float iPM2xxx::Read_CurrentBH30Magnitude() { return readFloat(24989); }

// Current B, H31 Magnitude, % (Addr: 24995)
float iPM2xxx::Read_CurrentBH31Magnitude() { return readFloat(24995); }

// Current C, H1 Magnitude, % (Addr: 25203)
float iPM2xxx::Read_CurrentCH1Magnitude() { return readFloat(25203); }

// Current C, H2 Magnitude, % (Addr: 25209)
float iPM2xxx::Read_CurrentCH2Magnitude() { return readFloat(25209); }

// Current C, H3 Magnitude, % (Addr: 25215)
float iPM2xxx::Read_CurrentCH3Magnitude() { return readFloat(25215); }

// Current C, H4 Magnitude, % (Addr: 25221)
float iPM2xxx::Read_CurrentCH4Magnitude() { return readFloat(25221); }

// Current C, H5 Magnitude, % (Addr: 25227)
float iPM2xxx::Read_CurrentCH5Magnitude() { return readFloat(25227); }

// Current C, H6 Magnitude, % (Addr: 25233)
float iPM2xxx::Read_CurrentCH6Magnitude() { return readFloat(25233); }

// Current C, H7 Magnitude, % (Addr: 25239)
float iPM2xxx::Read_CurrentCH7Magnitude() { return readFloat(25239); }

// Current C, H8 Magnitude, % (Addr: 25245)
float iPM2xxx::Read_CurrentCH8Magnitude() { return readFloat(25245); }

// Current C, H9 Magnitude, % (Addr: 25251)
float iPM2xxx::Read_CurrentCH9Magnitude() { return readFloat(25251); }

// Current C, H10 Magnitude, % (Addr: 25257)
float iPM2xxx::Read_CurrentCH10Magnitude() { return readFloat(25257); }

// Current C, H11 Magnitude, % (Addr: 25263)
float iPM2xxx::Read_CurrentCH11Magnitude() { return readFloat(25263); }

// Current C, H12 Magnitude, % (Addr: 25269)
float iPM2xxx::Read_CurrentCH12Magnitude() { return readFloat(25269); }

// Current C, H13 Magnitude, % (Addr: 25275)
float iPM2xxx::Read_CurrentCH13Magnitude() { return readFloat(25275); }

// Current C, H14 Magnitude, % (Addr: 25281)
float iPM2xxx::Read_CurrentCH14Magnitude() { return readFloat(25281); }

// Current C, H15 Magnitude, % (Addr: 25287)
float iPM2xxx::Read_CurrentCH15Magnitude() { return readFloat(25287); }

// Current C, H16 Magnitude, % (Addr: 25293)
float iPM2xxx::Read_CurrentCH16Magnitude() { return readFloat(25293); }

// Current C, H17 Magnitude, % (Addr: 25299)
float iPM2xxx::Read_CurrentCH17Magnitude() { return readFloat(25299); }

// Current C, H18 Magnitude, % (Addr: 25305)
float iPM2xxx::Read_CurrentCH18Magnitude() { return readFloat(25305); }

// Current C, H19 Magnitude, % (Addr: 25311)
float iPM2xxx::Read_CurrentCH19Magnitude() { return readFloat(25311); }

// Current C, H20 Magnitude, % (Addr: 25317)
float iPM2xxx::Read_CurrentCH20Magnitude() { return readFloat(25317); }

// Current C, H21 Magnitude, % (Addr: 25323)
float iPM2xxx::Read_CurrentCH21Magnitude() { return readFloat(25323); }

// Current C, H22 Magnitude, % (Addr: 25329)
float iPM2xxx::Read_CurrentCH22Magnitude() { return readFloat(25329); }

// Current C, H23 Magnitude, % (Addr: 25335)
float iPM2xxx::Read_CurrentCH23Magnitude() { return readFloat(25335); }

// Current C, H24 Magnitude, % (Addr: 25341)
float iPM2xxx::Read_CurrentCH24Magnitude() { return readFloat(25341); }

// Current C, H25 Magnitude, % (Addr: 25347)
float iPM2xxx::Read_CurrentCH25Magnitude() { return readFloat(25347); }

// Current C, H26 Magnitude, % (Addr: 25353)
float iPM2xxx::Read_CurrentCH26Magnitude() { return readFloat(25353); }

// Current C, H27 Magnitude, % (Addr: 25359)
float iPM2xxx::Read_CurrentCH27Magnitude() { return readFloat(25359); }

// Current C, H28 Magnitude, % (Addr: 25365)
float iPM2xxx::Read_CurrentCH28Magnitude() { return readFloat(25365); }

// Current C, H29 Magnitude, % (Addr: 25371)
float iPM2xxx::Read_CurrentCH29Magnitude() { return readFloat(25371); }

// Current C, H30 Magnitude, % (Addr: 25377)
float iPM2xxx::Read_CurrentCH30Magnitude() { return readFloat(25377); }

// Current C, H31 Magnitude, % (Addr: 25383)
float iPM2xxx::Read_CurrentCH31Magnitude() { return readFloat(25383); }

// Max/Min Reset date and time (Addr: 42299)
uint16_t iPM2xxx::Read_MaxMinResetDateAndTime() { return readU16(42299); }

// Max Current Avg (Addr: 42303)
float iPM2xxx::Read_MaxCurrentAvg() { return readFloat(42303); }

// Max current avg occurrence date time (Addr: 42305)
uint16_t iPM2xxx::Read_MaxCurrentAvgTimestamp() {
  return readU16(42305);
}

// Max Voltage L-L Avg (Addr: 42309)
float iPM2xxx::Read_MaxVoltageLLAvg() { return readFloat(42309); }

// Max Voltage L-L Avg - Occurrence date time (Addr: 42311)
uint16_t iPM2xxx::Read_MaxVoltageLLAvgTimestamp() {
  return readU16(42311);
}

// Max Voltage L-N Avg (Addr: 42315)
float iPM2xxx::Read_MaxVoltageLNAvg() { return readFloat(42315); }

// Max Voltage L-N Avg - Occurrence date time (Addr: 42317)
uint16_t iPM2xxx::Read_MaxVoltageLNAvgTimestamp() {
  return readU16(42317);
}

// Max Active Power Total (Addr: 42321)
float iPM2xxx::Read_MaxActivePowerTotal() { return readFloat(42321); }

// Max Active Power Total -occurrence date time (Addr: 42323)
uint16_t iPM2xxx::Read_MaxActivePowerTotalTimestamp() {
  return readU16(42323);
}

// Max Reactive Power Total (Addr: 42327)
float iPM2xxx::Read_MaxReactivePowerTotal() { return readFloat(42327); }

// Max Reactive Power Total -occurrence date time (Addr: 42329)
uint16_t iPM2xxx::Read_MaxReactivePowerTotalTimestamp() {
  return readU16(42329);
}

// Max Apparent Power Total (Addr: 42333)
float iPM2xxx::Read_MaxApparentPowerTotal() { return readFloat(42333); }

// Max Apparent Power Total - occurrence date time (Addr: 42335)
uint16_t iPM2xxx::Read_MaxApparentPowerTotalTimestamp() {
  return readU16(42335);
}

// Max Power Factor Total (Addr: 42339)
float iPM2xxx::Read_MaxPowerFactorTotal() { return readFloat(42339); }

// Max Power Factor Total -occurrence date time (Addr: 42341)
uint16_t iPM2xxx::Read_MaxPowerFactorTotalTimestamp() {
  return readU16(42341);
}

// Max Frequency (Addr: 42345)
float iPM2xxx::Read_MaxFrequency() { return readFloat(42345); }

// Max Frequency occurrence date time (Addr: 42347)
uint16_t iPM2xxx::Read_MaxFrequencyTimestamp() {
  return readU16(42347);
}

// Min Current Avg (Addr: 42591)
float iPM2xxx::Read_MinCurrentAvg() { return readFloat(42591); }

// Min Current Avg occurrence date time (Addr: 42593)
uint16_t iPM2xxx::Read_MinCurrentAvgTimestamp() {
  return readU16(42593);
}

// Min Voltage L-L Avg (Addr: 42597)
float iPM2xxx::Read_MinVoltageLLAvg() { return readFloat(42597); }

// Min Voltage L-L Avg occurrence date time (Addr: 42599)
uint16_t iPM2xxx::Read_MinVoltageLLAvgTimestamp() {
  return readU16(42599);
}

// Min Voltage L-N Avg (Addr: 42603)
float iPM2xxx::Read_MinVoltageLNAvg() { return readFloat(42603); }

// Min Voltage L-N Avg occurrence date time (Addr: 42605)
uint16_t iPM2xxx::Read_MinVoltageLNAvgTimestamp() {
  return readU16(42605);
}

// Min Active Power Total (Addr: 42609)
float iPM2xxx::Read_MinActivePowerTotal() { return readFloat(42609); }

// Min Active Power Total occurrence date time (Addr: 42611)
uint16_t iPM2xxx::Read_MinActivePowerTotalTimestamp() {
  return readU16(42611);
}

// Min Reactive Power Total (Addr: 42615)
float iPM2xxx::Read_MinReactivePowerTotal() { return readFloat(42615); }

// Min Reactive Power Total occurrence date time (Addr: 42617)
uint16_t iPM2xxx::Read_MinReactivePowerTotalTimestamp() {
  return readU16(42617);
}

// Min Apparent Power Total (Addr: 42621)
float iPM2xxx::Read_MinApparentPowerTotal() { return readFloat(42621); }

// Min Apparent Power Total occurrence date time (Addr: 42623)
uint16_t iPM2xxx::Read_MinApparentPowerTotalTimestamp() {
  return readU16(42623);
}

// Min Power Factor Total (Addr: 42627)
float iPM2xxx::Read_MinPowerFactorTotal() { return readFloat(42627); }

// Min Power Factor Total occurrence date time (Addr: 42629)
uint16_t iPM2xxx::Read_MinPowerFactorTotalTimestamp() {
  return readU16(42629);
}

// Min Frequency (Addr: 42633)
float iPM2xxx::Read_MinFrequency() { return readFloat(42633); }

// Min Frequency occurrence date time (Addr: 42635)
uint16_t iPM2xxx::Read_MinFrequencyTimestamp() {
  return readU16(42635);
}

// Active Energy Delivered (Into Load) (Addr: 42975)
float iPM2xxx::Read_ActiveEnergyDeliveredIntoLoad_42975() {
  return readFloat(42975);
}

// Active Energy Received (Out of Load) (Addr: 42977)
float iPM2xxx::Read_ActiveEnergyReceivedOutOfLoad_42977() {
  return readFloat(42977);
}

// Active Energy Delivered + Received (Addr: 42979)
float iPM2xxx::Read_ActiveEnergyDeliveredPlusReceived_42979() {
  return readFloat(42979);
}

// Active Energy Delivered � Received (Addr: 42981)
float iPM2xxx::Read_ActiveEnergyDeliveredReceived_42981() {
  return readFloat(42981);
}

// Reactive Energy Delivered (Addr: 42983)
float iPM2xxx::Read_ReactiveEnergyDelivered_42983() { return readFloat(42983); }

// Reactive Energy Received (Addr: 42985)
float iPM2xxx::Read_ReactiveEnergyReceived_42985() { return readFloat(42985); }

// Reactive Energy Delivered + Received (Addr: 42987)
float iPM2xxx::Read_ReactiveEnergyDeliveredPlusReceived_42987() {
  return readFloat(42987);
}

// Reactive Energy Delivered � Received (Addr: 42989)
float iPM2xxx::Read_ReactiveEnergyDeliveredReceived() {
  return readFloat(42989);
}

// Apparent Energy Delivered (Addr: 42991)
float iPM2xxx::Read_ApparentEnergyDelivered_42991() { return readFloat(42991); }

// Apparent Energy Received (Addr: 42993)
float iPM2xxx::Read_ApparentEnergyReceived_42993() { return readFloat(42993); }

// Apparent Energy Delivered + Received (Addr: 42995)
float iPM2xxx::Read_ApparentEnergyDeliveredPlusReceived_42995() {
  return readFloat(42995);
}

// Apparent Energy Delivered � Received (Addr: 42997)
float iPM2xxx::Read_ApparentEnergyDeliveredReceived() {
  return readFloat(42997);
}

// Phase angle between Voltage A and Current A (Addr: 43507)
float iPM2xxx::Read_PhaseAngleBetweenVoltageAAndCurrentA() {
  return readFloat(43507);
}

// Phase angle between Voltage B and Current B (Addr: 43509)
float iPM2xxx::Read_PhaseAngleBetweenVoltageBAndCurrentB() {
  return readFloat(43509);
}

// Phase angle between Voltage C and Current C (Addr: 43511)
float iPM2xxx::Read_PhaseAngleBetweenVoltageCAndCurrentC() {
  return readFloat(43511);
}

// Run sec Delivered (Into Load) (Addr: 43545)
uint16_t iPM2xxx::Read_RunSecDeliveredIntoLoad() { return readU16(43545); }

// Run sec Received (Out of Load) (Addr: 43547)
uint16_t iPM2xxx::Read_RunSecReceivedOutOfLoad() { return readU16(43547); }

// Active Energy Delivered (Into Load) (Addr: 43599)
uint64_t iPM2xxx::Read_ActiveEnergyDeliveredIntoLoad_43599() {
  return readU64(43599);
}

// Active Energy Received (Out of Load) (Addr: 43603)
uint64_t iPM2xxx::Read_ActiveEnergyReceivedOutOfLoad_43603() {
  return readU64(43603);
}

// Active Energy Delivered + Received (Addr: 43607)
uint64_t iPM2xxx::Read_ActiveEnergyDeliveredPlusReceived_43607() {
  return readU64(43607);
}

// Active Energy Delivered � Received (Addr: 43611)
uint64_t iPM2xxx::Read_ActiveEnergyDeliveredReceived_43611() {
  return readU64(43611);
}

// Reactive Energy Delivered (Addr: 43615)
uint64_t iPM2xxx::Read_ReactiveEnergyDelivered_43615() {
  return readU64(43615);
}

// Reactive Energy Received (Addr: 43619)
uint64_t iPM2xxx::Read_ReactiveEnergyReceived_43619() { return readU64(43619); }

// Reactive Energy Delivered + Received (Addr: 43623)
uint64_t iPM2xxx::Read_ReactiveEnergyDeliveredPlusReceived_43623() {
  return readU64(43623);
}

// Reactive Energy Delivered � Received (Addr: 43627)
uint64_t iPM2xxx::Read_ReactiveEnergyDeliveredReceived_43627() {
  return readU64(43627);
}

// Apparent Energy Delivered (Addr: 43631)
uint64_t iPM2xxx::Read_ApparentEnergyDelivered_43631() {
  return readU64(43631);
}

// Apparent Energy Received (Addr: 43635)
uint64_t iPM2xxx::Read_ApparentEnergyReceived_43635() { return readU64(43635); }

// Apparent Energy Delivered + Received (Addr: 43639)
uint64_t iPM2xxx::Read_ApparentEnergyDeliveredPlusReceived_43639() {
  return readU64(43639);
}

// Apparent Energy Delivered � Received (Addr: 43643)
uint64_t iPM2xxx::Read_ApparentEnergyDeliveredReceived_43643() {
  return readU64(43643);
}

// Run sec Delivered (Into Load) (Addr: 43671)
uint16_t iPM2xxx::Read_RunSecDeliveredIntoLoad_43671() {
  return readU16(43671);
}

// Run sec Received (Out of Load) (Addr: 43673)
uint16_t iPM2xxx::Read_RunSecReceivedOutOfLoad_43673() {
  return readU16(43673);
}

// Rate 1 Value (Addr: 43723)
float iPM2xxx::Read_Rate1Value_43723() { return readFloat(43723); }

// Rate 2 Value (Addr: 43725)
float iPM2xxx::Read_Rate2Value_43725() { return readFloat(43725); }

// Type (Addr: 44107)
uint16_t iPM2xxx::Read_Type_44107() { return readU16(44107); }

// Label (Addr: 44108)
std::string iPM2xxx::Read_Label_44108() { return readString(44108, 20); }

// Units Code (Addr: 44128)
uint16_t iPM2xxx::Read_UnitsCode_44128() { return readU16(44128); }

// Scale Code (Addr: 44129)
int16_t iPM2xxx::Read_ScaleCode() { return (int16_t)readU16(44129); }

// Range Select (Addr: 44130)
uint16_t iPM2xxx::Read_RangeSelect() { return readU16(44130); }

// Analog Input Minimum (Addr: 44131)
float iPM2xxx::Read_AnalogInputMinimum() { return readFloat(44131); }

// Analog Input Maximum (Addr: 44133)
float iPM2xxx::Read_AnalogInputMaximum() { return readFloat(44133); }

// Lower Limit Analog Value (Addr: 44135)
float iPM2xxx::Read_LowerLimitAnalogValue() { return readFloat(44135); }

// Upper Limit Analog Value (Addr: 44137)
float iPM2xxx::Read_UpperLimitAnalogValue() { return readFloat(44137); }

// Lower Limit Register Value (Addr: 44139)
float iPM2xxx::Read_LowerLimitRegisterValue() { return readFloat(44139); }

// Upper Limit Register Value (Addr: 44141)
float iPM2xxx::Read_UpperLimitRegisterValue() { return readFloat(44141); }

// User Gain Adjustment (Addr: 44143)
float iPM2xxx::Read_UserGainAdjustment() { return readFloat(44143); }

// User Offset Adjustment (Addr: 44145)
float iPM2xxx::Read_UserOffsetAdjustment() { return readFloat(44145); }

// Lower Limit Digital Value (Addr: 44147)
int16_t iPM2xxx::Read_LowerLimitDigitalValue() {
  return (int16_t)readU16(44147);
}

// Upper Limit Digital Value (Addr: 44148)
int16_t iPM2xxx::Read_UpperLimitDigitalValue() {
  return (int16_t)readU16(44148);
}

// Present Raw Value (Addr: 44149)
float iPM2xxx::Read_PresentRawValue() { return readFloat(44149); }

// Present Scaled Value (Addr: 44151)
float iPM2xxx::Read_PresentScaledValue() { return readFloat(44151); }

// Calibration Offset (Addr: 44153)
float iPM2xxx::Read_CalibrationOffset() { return readFloat(44153); }

// Calibration Gain (Addr: 44155)
float iPM2xxx::Read_CalibrationGain() { return readFloat(44155); }

// Calibration Gain (Current) (Addr: 44157)
float iPM2xxx::Read_CalibrationGainCurrent() { return readFloat(44157); }

// IO Point Diagnostic Bitmap (Addr: 44159)
uint16_t iPM2xxx::Read_IoPointDiagnosticBitmap() { return readU16(44159); }

// Type (Addr: 44161)
uint16_t iPM2xxx::Read_Type_44161() { return readU16(44161); }

// Label (Addr: 44162)
std::string iPM2xxx::Read_Label_44162() { return readString(44162, 20); }

// Units Code (Addr: 44182)
uint16_t iPM2xxx::Read_UnitsCode_44182() { return readU16(44182); }

// Scale Code (Addr: 44183)
uint16_t iPM2xxx::Read_ScaleCode_44183() { return readU16(44183); }

// Range Select (Addr: 44184)
uint16_t iPM2xxx::Read_RangeSelect_44184() { return readU16(44184); }

// Analog Input Minimum (Addr: 44185)
float iPM2xxx::Read_AnalogInputMinimum_44185() { return readFloat(44185); }

// Analog Input Maximum (Addr: 44187)
float iPM2xxx::Read_AnalogInputMaximum_44187() { return readFloat(44187); }

// Lower Limit Analog Value (Addr: 44189)
float iPM2xxx::Read_LowerLimitAnalogValue_44189() { return readFloat(44189); }

// Upper Limit Analog Value (Addr: 44191)
float iPM2xxx::Read_UpperLimitAnalogValue_44191() { return readFloat(44191); }

// Lower Limit Register Value (Addr: 44193)
float iPM2xxx::Read_LowerLimitRegisterValue_44193() { return readFloat(44193); }

// Upper Limit Register Value (Addr: 44195)
float iPM2xxx::Read_UpperLimitRegisterValue_44195() { return readFloat(44195); }

// User Gain Adjustment (Addr: 44197)
float iPM2xxx::Read_UserGainAdjustment_44197() { return readFloat(44197); }

// User Offset Adjustment (Addr: 44199)
float iPM2xxx::Read_UserOffsetAdjustment_44199() { return readFloat(44199); }

// Lower Limit Digital Value (Addr: 44201)
int16_t iPM2xxx::Read_LowerLimitDigitalValue_44201() {
  return (int16_t)readU16(44201);
}

// Upper Limit Digital Value (Addr: 44202)
int16_t iPM2xxx::Read_UpperLimitDigitalValue_44202() {
  return (int16_t)readU16(44202);
}

// Present Raw Value (Addr: 44203)
float iPM2xxx::Read_PresentRawValue_44203() { return readFloat(44203); }

// Present Scaled Value (Addr: 44205)
float iPM2xxx::Read_PresentScaledValue_44205() { return readFloat(44205); }

// Calibration Offset (Addr: 44207)
float iPM2xxx::Read_CalibrationOffset_44207() { return readFloat(44207); }

// Calibration Gain (Addr: 44209)
float iPM2xxx::Read_CalibrationGain_44209() { return readFloat(44209); }

// Calibration Gain (Current) (Addr: 44211)
float iPM2xxx::Read_CalibrationGainCurrent_44211() { return readFloat(44211); }

// IO Point Diagnostic Bitmap (Addr: 44213)
uint16_t iPM2xxx::Read_IoPointDiagnosticBitmap_44213() {
  return readU16(44213);
}

// Type (Addr: 44747)
uint16_t iPM2xxx::Read_Type_44747() { return readU16(44747); }

// Label (Addr: 44748)
std::string iPM2xxx::Read_Label_44748() { return readString(44748, 20); }

// Range Select (Addr: 44768)
uint16_t iPM2xxx::Read_RangeSelect_44768() { return readU16(44768); }

// Output Enable (Addr: 44769)
uint16_t iPM2xxx::Read_OutputEnable() { return readU16(44769); }

// Reference Register Number (Addr: 44770)
uint16_t iPM2xxx::Read_ReferenceRegisterNumber() { return readU16(44770); }

// Lower Limit Analog Value (Addr: 44771)
float iPM2xxx::Read_LowerLimitAnalogValue_44771() { return readFloat(44771); }

// Upper Limit Analog Value (Addr: 44773)
float iPM2xxx::Read_UpperLimitAnalogValue_44773() { return readFloat(44773); }

// Lower Limit Register Value (Addr: 44775)
float iPM2xxx::Read_LowerLimitRegisterValue_44775() { return readFloat(44775); }

// Upper Limit Register Value (Addr: 44777)
float iPM2xxx::Read_UpperLimitRegisterValue_44777() { return readFloat(44777); }

// User Gain Adjustment (Addr: 44779)
float iPM2xxx::Read_UserGainAdjustment_44779() { return readFloat(44779); }

// User Offset Adjustment (Addr: 44781)
float iPM2xxx::Read_UserOffsetAdjustment_44781() { return readFloat(44781); }

// Lower Limit Digital Value (Addr: 44783)
uint16_t iPM2xxx::Read_LowerLimitDigitalValue_44783() { return readU16(44783); }

// Upper Limit Digital Value (Addr: 44784)
uint16_t iPM2xxx::Read_UpperLimitDigitalValue_44784() { return readU16(44784); }

// Present Analog Value (Addr: 44785)
float iPM2xxx::Read_PresentAnalogValue() { return readFloat(44785); }

// Present Raw (Register) Value (Addr: 44787)
float iPM2xxx::Read_PresentRawRegisterValue() { return readFloat(44787); }

// Calibration Offset (Addr: 44789)
float iPM2xxx::Read_CalibrationOffset_44789() { return readFloat(44789); }

// Calibration Gain (Addr: 44791)
float iPM2xxx::Read_CalibrationGain_44791() { return readFloat(44791); }

// Present Digital Value (Addr: 44793)
uint16_t iPM2xxx::Read_PresentDigitalValue() { return readU16(44793); }

// IO Point Diagnostic Bitmap (Addr: 44794)
uint16_t iPM2xxx::Read_IoPointDiagnosticBitmap_44794() {
  return readU16(44794);
}

// Type (Addr: 44799)
uint16_t iPM2xxx::Read_Type_44799() { return readU16(44799); }

// Label (Addr: 44800)
std::string iPM2xxx::Read_Label_44800() { return readString(44800, 20); }

// Range Select (Addr: 44820)
uint16_t iPM2xxx::Read_RangeSelect_44820() { return readU16(44820); }

// Output Enable (Addr: 44821)
uint16_t iPM2xxx::Read_OutputEnable_44821() { return readU16(44821); }

// Reference Register Number (Addr: 44822)
uint16_t iPM2xxx::Read_ReferenceRegisterNumber_44822() {
  return readU16(44822);
}

// Lower Limit Analog Value (Addr: 44823)
float iPM2xxx::Read_LowerLimitAnalogValue_44823() { return readFloat(44823); }

// Upper Limit Analog Value (Addr: 44825)
float iPM2xxx::Read_UpperLimitAnalogValue_44825() { return readFloat(44825); }

// Lower Limit Register Value (Addr: 44827)
float iPM2xxx::Read_LowerLimitRegisterValue_44827() { return readFloat(44827); }

// Upper Limit Register Value (Addr: 44829)
float iPM2xxx::Read_UpperLimitRegisterValue_44829() { return readFloat(44829); }

// User Gain Adjustment (Addr: 44831)
float iPM2xxx::Read_UserGainAdjustment_44831() { return readFloat(44831); }

// User Offset Adjustment (Addr: 44833)
float iPM2xxx::Read_UserOffsetAdjustment_44833() { return readFloat(44833); }

// Lower Limit Digital Value (Addr: 44835)
uint16_t iPM2xxx::Read_LowerLimitDigitalValue_44835() { return readU16(44835); }

// Upper Limit Digital Value (Addr: 44836)
uint16_t iPM2xxx::Read_UpperLimitDigitalValue_44836() { return readU16(44836); }

// Present Analog Value (Addr: 44837)
float iPM2xxx::Read_PresentAnalogValue_44837() { return readFloat(44837); }

// Present Raw (Register) Value (Addr: 44839)
float iPM2xxx::Read_PresentRawRegisterValue_44839() { return readFloat(44839); }

// Calibration Offset (Addr: 44841)
float iPM2xxx::Read_CalibrationOffset_44841() { return readFloat(44841); }

// Calibration Gain (Addr: 44843)
float iPM2xxx::Read_CalibrationGain_44843() { return readFloat(44843); }

// Present Digital Value (Addr: 44845)
uint16_t iPM2xxx::Read_PresentDigitalValue_44845() { return readU16(44845); }

// IO Point Diagnostic Bitmap (Addr: 44846)
uint16_t iPM2xxx::Read_IoPointDiagnosticBitmap_44846() {
  return readU16(44846);
}
