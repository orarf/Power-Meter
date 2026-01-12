#ifndef READ_IPM2XXX_H
#define READ_IPM2XXX_H

#include "iPM2xxx.h"
#include <chrono>
#include <cmath> // For std::isnan
#include <iostream>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

// Helper to sanitize float for SQLite (replace NaN with 0)
inline float safe_float_pm(float val) {
  if (std::isnan(val)) {
    return 0.0f;
  }
  return val;
}

// Helper to get historical energy values using a reused prepared statement
inline int64_t get_historical_energy_pm(sqlite3_stmt *stmt, int unit_id, const std::string& gateway_ip,
                                        int seconds_ago) {
  int64_t energy = 0;

  if (stmt) {
    long now = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
    long target_time = now - seconds_ago;
    // Window +/- 30s
    long start_window = target_time - 30;
    long end_window = target_time + 30;

    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, unit_id);
    sqlite3_bind_text(stmt, 2, gateway_ip.c_str(), -1, SQLITE_STATIC); // Gateway IP
    sqlite3_bind_int64(stmt, 3, start_window);
    sqlite3_bind_int64(stmt, 4, end_window);
    sqlite3_bind_int64(stmt, 5, target_time);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      energy = sqlite3_column_int64(stmt, 0);
    }
  }
  return energy;
}

inline void SetupDatabasePM(sqlite3 *db) {
  char *errMsg = 0;
  int rc;

  // 1. Create Table
  const char *sqlCreateTable =
      "CREATE TABLE IF NOT EXISTS readings_pm2xxx ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "timestamp INTEGER, "
      "gateway_ip TEXT, "
      "unit_id INTEGER, "
      "voltage_a REAL, voltage_b REAL, voltage_c REAL, voltage_avg REAL, "
      "current_a REAL, current_b REAL, current_c REAL, current_avg REAL, "
      "active_power_total REAL, "
      "reactive_power_total REAL, "
      "apparent_power_total REAL, "
      "power_factor_total REAL, "
      "frequency REAL, "
      "total_energy INTEGER, "
      "total_energy_last_1M INTEGER DEFAULT 0, "
      "total_energy_last_5M INTEGER DEFAULT 0, "
      "total_energy_last_30M INTEGER DEFAULT 0, "
      "total_energy_last_1H INTEGER DEFAULT 0, "
      "total_energy_last_2H INTEGER DEFAULT 0, "
      "total_energy_last_1D INTEGER DEFAULT 0, "
      "ActiveEnergyDeliveredIntoLoad REAL, "
      "is_read INTEGER DEFAULT 0"
      ");";

  rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error (create table): " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return;
  }

  // 2. Cleanup old data (> 2 days)
  const char *sqlCleanup = "DELETE FROM readings_pm2xxx WHERE timestamp < "
                           "strftime('%s', 'now', '-2 days');";
  rc = sqlite3_exec(db, sqlCleanup, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error (cleanup): " << errMsg << std::endl;
    sqlite3_free(errMsg);
  }
}

inline void Read_iPM2xxx(const std::vector<int> &ids, const std::string &ipAddr,
                         int port) {
  sqlite3 *db;
  int rc;

  // 1. Open Database
  rc = sqlite3_open("iPM2xxx.db", &db);
  if (rc) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    return;
  }

  // 2. Setup
  SetupDatabasePM(db);

  // 3. Prepare Statements
  sqlite3_stmt *stmtHistory = nullptr;
  const char *sqlHistory = "SELECT total_energy FROM readings_pm2xxx "
                           "WHERE unit_id = ? AND gateway_ip = ? "
                           "AND timestamp BETWEEN ? AND ? "
                           "ORDER BY ABS(timestamp - ?) LIMIT 1;";
  if (sqlite3_prepare_v2(db, sqlHistory, -1, &stmtHistory, 0) != SQLITE_OK) {
    std::cerr << "SQL Prepare History Error: " << sqlite3_errmsg(db)
              << std::endl;
    sqlite3_close(db);
    return;
  }

  sqlite3_stmt *stmtInsert = nullptr;
  const char *sqlInsert =
      "INSERT INTO readings_pm2xxx (timestamp, gateway_ip, unit_id, "
      "voltage_a, voltage_b, voltage_c, voltage_avg, "
      "current_a, current_b, current_c, current_avg, "
      "active_power_total, reactive_power_total, apparent_power_total, "
      "power_factor_total, frequency, total_energy, "
      "total_energy_last_1M, total_energy_last_5M, "
      "total_energy_last_30M, total_energy_last_1H, "
      "total_energy_last_2H, ActiveEnergyDeliveredIntoLoad, current_unbalanceA, current_unbalanceB, current_unbalanceC, "
      "current_unbalanceWorst, ActiveEnergyReceived_OutofLoad, ActiveEnergyDeliveredPlussReceived, ActiveEnergyDeliveredDelReceived, "
      "ReactiveEnergyDelivered, ReactiveEnergyReceived, ReactiveEnergyDeliveredPlussReceived, ReactiveEnergyDeliveredDelReceived, "
      "ApparentEnergyDelivered, ApparentEnergyReceived, ApparentEnergyDeliveredPlussReceived, "
      "ApparentEnergyDeliveredDelReceived, ActivePowerA, ActivePowerB, ActivePowerC, ReactivePowerA, ReactivePowerB, ReactivePowerC, "
      "ApparentPowerA, ApparentPowerB, ApparentPowerC, PowerFactorA, PowerFactorB, PowerFactorC, "
      "PowerDemandMethod, PowerDemandIntervalDuration, PowerDemandSubintervalDuration, PowerDemandElapsedTimeinInterval, PowerDemandElapsedTimeinSubinterval, "
      "CurrentDemandMethod, CurrentDemandIntervalDuration, CurrentDemandElapsedTimein, CurrentDemandSubintervalDuration, CurrentDemandElapsedTimeinInterval, "
      "VoltageAB, VoltageBC, VoltageCA, VoltageLLAvg, "
      "VoltageUnbalanceAB, VoltageUnbalanceBC, VoltageUnbalanceCA, VoltageUnbalanceLLWorst, "
      "VoltageUnbalanceAN, VoltageUnbalanceBN, VoltageUnbalanceCN, VoltageUnbalanceLNWorst, "
      "DisplacementPowerFactorA, DisplacementPowerFactorB, DisplacementPowerFactorC, DisplacementPowerFactorTotal, "
      "ActiveEnergyDeliveredIntoLoad64, ActiveEnergyReceivedOutofLoad64, ActiveEnergyDeliveredPlussReceived64, ActiveEnergyDeliveredDelReceived64) VALUES "
      "(strftime('%s', 'now'), ?, ?, "
      "?, ?, ?, ?, "
      "?, ?, ?, ?, "
      "?, ?, ?, "
      "?, ?, ?, "
      "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
      "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
      "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

  if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmtInsert, 0) != SQLITE_OK) {
    std::cerr << "SQL Prepare Insert Error: " << sqlite3_errmsg(db)
              << std::endl;
    if (stmtHistory)
      sqlite3_finalize(stmtHistory);
    sqlite3_close(db);
    return;
  }

  // 4. Loop through IDs
  for (int unitId : ids) {
    std::cout << "\nStarting Monitor iPM2xxx (Device " << unitId << ")..."
              << std::endl;
    std::unique_ptr<iPM2xxx> client;
    bool connected = false;

    // Retry logic
    for (int j = 0; j < 3; j++) {
      client = iPM2xxx::createClient(unitId, ipAddr, port);
      if (client->isConnected()) {
        std::cout << "Port Opened Successfully (Device " << unitId << ")."
                  << std::endl;
        connected = true;
        break;
      }
      std::cerr << "Failed to open port. Retrying in 1s..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (connected && client) {
      std::cout << "----------------------------------------" << std::endl;
      std::cout << "Reading Device " << unitId << "..." << std::endl;

      // --- Read Values ---
      // Voltage
      float vA = client->Read_VoltageAN();
      float vB = client->Read_VoltageBN();
      float vC = client->Read_VoltageCN();
      float vAvg = client->Read_VoltageLNAvg();

      // Current
      float cA = client->Read_CurrentA();
      float cB = client->Read_CurrentB();
      float cC = client->Read_CurrentC();
      float cAvg = client->Read_CurrentAvg();
      

      // Power
      float pTotal = client->Read_ActivePowerTotal();
      float qTotal = client->Read_ReactivePowerTotal();
      float sTotal = client->Read_ApparentPowerTotal();

      // Basics
      float pf = client->Read_PowerFactorTotal();
      float freq = client->Read_Frequency();

      // Energy (64-bit)
      int64_t energy = client->Read_ActiveEnergy_Total();

     

      // History
      int64_t last_1M = get_historical_energy_pm(stmtHistory, unitId, ipAddr, 60);
      int64_t last_5M = get_historical_energy_pm(stmtHistory, unitId, ipAddr, 300);
      int64_t last_30M = get_historical_energy_pm(stmtHistory, unitId, ipAddr, 1800);
      int64_t last_1H = get_historical_energy_pm(stmtHistory, unitId, ipAddr, 3600);
      int64_t last_2H = get_historical_energy_pm(stmtHistory, unitId, ipAddr, 7200);

      // Energy (Float 32-bit)
      float energy1 = client->Read_ActiveEnergyDeliveredIntoLoad();
      float cUnbA = client->Read_CurrentUnbalanceA();
      float cUnbB = client->Read_CurrentUnbalanceB();
      float cUnbC = client->Read_CurrentUnbalanceC();
      float cUnbWorst = client->Read_CurrentUnbalanceWorst();

      float ActiveEnergyReceived_OutofLoad = client->Read_ActiveEnergyReceivedOutOfLoad();
      float ActiveEnergyDeliveredPlussReceived = client->Read_ActiveEnergyDeliveredPlusReceived();
      float ActiveEnergyDeliveredDelReceived = client->Read_ActiveEnergyDeliveredReceived();
      float ReactiveEnergyDelivered = client->Read_ReactiveEnergyDelivered();
      float ReactiveEnergyReceived = client->Read_ReactiveEnergyReceived();
      float ReactiveEnergyDeliveredPlussReceived = client->Read_ReactiveEnergyDeliveredPlusReceived();
      float ReactiveEnergyDeliveredDelReceived = client->Read_ReactiveEnergyDeliveredReceived();
      float ApparentEnergyDelivered = client->Read_ApparentEnergyDelivered();
      float ApparentEnergyReceived = client->Read_ApparentEnergyReceived();
      float ApparentEnergyDeliveredPlussReceived = client->Read_ApparentEnergyDeliveredPlusReceived();
      float ApparentEnergyDeliveredDelReceived = client->Read_ApparentEnergyDeliveredReceived();
      float ApparentPowerA = client->Read_ApparentPowerA();
      float ApparentPowerB = client->Read_ApparentPowerB();
      float ApparentPowerC = client->Read_ApparentPowerC();
      float ActivePowerA = client->Read_ActivePowerA();
      float ActivePowerB = client->Read_ActivePowerB();
      float ActivePowerC = client->Read_ActivePowerC();
      float ReactivePowerA = client->Read_ReactivePowerA();
      float ReactivePowerB = client->Read_ReactivePowerB();
      float ReactivePowerC = client->Read_ReactivePowerC();
      float PowerFactorA = client->Read_PowerFactorA();
      float PowerFactorB = client->Read_PowerFactorB();
      float PowerFactorC = client->Read_PowerFactorC();
      uint16_t PowerDemandMethod = client->Read_PowerDemandMethod();
      uint16_t PowerDemandIntervalDuration = client->Read_PowerDemandIntervalDuration();
      uint16_t PowerDemandSubintervalDuration = client->Read_PowerDemandSubintervalDuration();
      uint16_t PowerDemandElapsedTimeinInterval = client->Read_PowerDemandElapsedTimeInInterval();
      uint16_t PowerDemandElapsedTimeinSubinterval = client->Read_PowerDemandElapsedTimeInSubinterval();
      uint16_t CurrentDemandMethod = client->Read_CurrentDemandMethod();
      uint16_t CurrentDemandIntervalDuration = client->Read_CurrentDemandIntervalDuration();
      uint16_t CurrentDemandElapsedTimein = client->Read_CurrentDemandElapsedTimeInInterval();
      uint16_t CurrentDemandSubintervalDuration = client->Read_CurrentDemandSubintervalDuration();
      uint16_t CurrentDemandElapsedTimeinInterval = client->Read_CurrentDemandElapsedTimeInInterval();
      float VoltageAB = client->Read_VoltageAB();
      float VoltageBC = client->Read_VoltageBC();
      float VoltageCA = client->Read_VoltageCA();
      float VoltageLLAvg = client->Read_VoltageLLAvg();
      float VoltageUnbalanceAB = client->Read_VoltageUnbalanceAB();
      float VoltageUnbalanceBC = client->Read_VoltageUnbalanceBC();
      float VoltageUnbalanceCA = client->Read_VoltageUnbalanceCA();
      float VoltageUnbalanceLLWorst = client->Read_VoltageUnbalanceLLWorst();
      float VoltageUnbalanceAN = client->Read_VoltageUnbalanceAN();
      float VoltageUnbalanceBN = client->Read_VoltageUnbalanceBN();
      float VoltageUnbalanceCN = client->Read_VoltageUnbalanceCN();
      float VoltageUnbalanceLNWorst = client->Read_VoltageUnbalanceLNWorst();
      float DisplacementPowerFactorA = client->Read_DisplacementPowerFactorA();
      float DisplacementPowerFactorB = client->Read_DisplacementPowerFactorB();
      float DisplacementPowerFactorC = client->Read_DisplacementPowerFactorC();
      float DisplacementPowerFactorTotal = client->Read_DisplacementPowerFactorTotal();

      int64_t ActiveEnergyDeliveredIntoLoad64 = client->Read_ActiveEnergy_Delivered();
      int64_t ActiveEnergyReceivedOutofLoad64 = client->Read_ActiveEnergy_Received();
      int64_t ActiveEnergyDeliveredPlussReceived64 = client->Read_ActiveEnergy_Total();
      int64_t ActiveEnergyDeliveredDelReceived64 = client->Read_ActiveEnergy_DeliveredReceived();



      // --- Print to Console ---
      std::cout << "Voltage (L-N): A=" << vA << ", B=" << vB << ", C=" << vC
                << " V" << std::endl;
      std::cout << "Current: A=" << cA << ", B=" << cB << ", C=" << cC << " A"
                << std::endl;
      std::cout << "Power: Active=" << pTotal << " W, Reactive=" << qTotal
                << " VAR, Apparent=" << sTotal << " VA" << std::endl;
      std::cout << "Power Factor: " << pf << ", Freq: " << freq << " Hz"
                << std::endl;
      std::cout << "Total Energy: " << energy << " Wh" << std::endl;
      std::cout << "  - Last 1M: " << last_1M << " Wh" << std::endl;
      std::cout << "  - Last 5M: " << last_5M << " Wh" << std::endl;
      std::cout << "  - Last 30M: " << last_30M << " Wh" << std::endl;
      std::cout << "  - Last 1H: " << last_1H << " Wh" << std::endl;
      std::cout << "  - Last 2H: " << last_2H << " Wh" << std::endl;
      std::cout << "Active Energy Delivered Into Load: " << energy1 << " Wh"
                << std::endl;
      std::cout << "Current Unbalance: A=" << cUnbA << "%, B=" << cUnbB
                << "%, C=" << cUnbC << "%, Worst=" << cUnbWorst << "%" << std::endl;
      std::cout << "Active Energy Received Out of Load: " << ActiveEnergyReceived_OutofLoad << " Wh" << std::endl;
      std::cout << "Active Energy Delivered Plus Received: " << ActiveEnergyDeliveredPlussReceived << " Wh" << std::endl;
      std::cout << "Active Energy Delivered Delivered Received: " << ActiveEnergyDeliveredDelReceived << " Wh" << std::endl;
      std::cout << "Reactive Energy Delivered: " << ReactiveEnergyDelivered << " VARh" << std::endl;
      std::cout << "Reactive Energy Received: " << ReactiveEnergyReceived << " VARh" << std::endl;

      // --- Insert to DB ---
      sqlite3_reset(stmtInsert);
      int idx = 1;
      sqlite3_bind_text(stmtInsert, idx++, ipAddr.c_str(), -1, SQLITE_STATIC); // Gateway IP
      sqlite3_bind_int(stmtInsert, idx++, unitId);
      // Voltage
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(vA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(vB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(vC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(vAvg));
      // Current
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cAvg));
      // Power
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(pTotal));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(qTotal));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(sTotal));
      // Basics
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(pf));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(freq));
      // Energy
      sqlite3_bind_int64(stmtInsert, idx++, energy);
      
      // History
      sqlite3_bind_int64(stmtInsert, idx++, last_1M);
      sqlite3_bind_int64(stmtInsert, idx++, last_5M);
      sqlite3_bind_int64(stmtInsert, idx++, last_30M);
      sqlite3_bind_int64(stmtInsert, idx++, last_1H);
      sqlite3_bind_int64(stmtInsert, idx++, last_2H);
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(energy1));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cUnbA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cUnbB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cUnbC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(cUnbWorst));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActiveEnergyReceived_OutofLoad));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActiveEnergyDeliveredPlussReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActiveEnergyDeliveredDelReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactiveEnergyDelivered));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactiveEnergyReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactiveEnergyDeliveredPlussReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactiveEnergyDeliveredDelReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentEnergyDelivered));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentEnergyReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentEnergyDeliveredPlussReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentEnergyDeliveredDelReceived));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActivePowerA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActivePowerB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ActivePowerC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactivePowerA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactivePowerB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ReactivePowerC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentPowerA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentPowerB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(ApparentPowerC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(PowerFactorA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(PowerFactorB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(PowerFactorC));
      sqlite3_bind_int(stmtInsert, idx++, PowerDemandMethod);
      sqlite3_bind_int(stmtInsert, idx++, PowerDemandIntervalDuration);
      sqlite3_bind_int(stmtInsert, idx++, PowerDemandSubintervalDuration);
      sqlite3_bind_int(stmtInsert, idx++, PowerDemandElapsedTimeinInterval);
      sqlite3_bind_int(stmtInsert, idx++, PowerDemandElapsedTimeinSubinterval);
      sqlite3_bind_int(stmtInsert, idx++, CurrentDemandMethod);
      sqlite3_bind_int(stmtInsert, idx++, CurrentDemandIntervalDuration);
      sqlite3_bind_int(stmtInsert, idx++, CurrentDemandElapsedTimein);
      sqlite3_bind_int(stmtInsert, idx++, CurrentDemandSubintervalDuration);
      sqlite3_bind_int(stmtInsert, idx++, CurrentDemandElapsedTimeinInterval);
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageAB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageBC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageCA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageLLAvg));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceAB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceBC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceCA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceLLWorst));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceAN));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceBN));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceCN));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(VoltageUnbalanceLNWorst));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(DisplacementPowerFactorA));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(DisplacementPowerFactorB));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(DisplacementPowerFactorC));
      sqlite3_bind_double(stmtInsert, idx++, safe_float_pm(DisplacementPowerFactorTotal));
      sqlite3_bind_int64(stmtInsert, idx++, ActiveEnergyDeliveredIntoLoad64);
      sqlite3_bind_int64(stmtInsert, idx++, ActiveEnergyReceivedOutofLoad64);
      sqlite3_bind_int64(stmtInsert, idx++, ActiveEnergyDeliveredPlussReceived64);
      sqlite3_bind_int64(stmtInsert, idx++, ActiveEnergyDeliveredDelReceived64);

      if (sqlite3_step(stmtInsert) != SQLITE_DONE) {
        std::cerr << "SQL Insert Error: " << sqlite3_errmsg(db) << std::endl;
      } else {
        std::cout << "Data saved to SQLite (readings_pm2xxx)." << std::endl;
        std::cout << "----------------------------------------"
                  << std::endl;
      }

      client->Disconnect();
    } else {
      std::cerr << "Skipping Device " << unitId << " (Not Connected)"
                << std::endl;
    }
  }
  if (stmtHistory)
    sqlite3_finalize(stmtHistory);
  if (stmtInsert)
    sqlite3_finalize(stmtInsert);
  sqlite3_close(db);
}

#endif // READ_IPM2XXX_H
