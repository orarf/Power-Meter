#ifndef READ_IA9MEM15_H
#define READ_IA9MEM15_H

#include "iA9MEM15.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <vector>

/* ---------- Helpers ---------- */

inline float safe_float(float v) {
  return std::isnan(v) ? 0.0f : v;
}

inline uint64_t get_historical_energy(sqlite3_stmt *stmt,
                                      int unit_id,
                                      const std::string &gateway_ip,
                                      int seconds_ago) {
  if (!stmt)
    return 0;

  uint64_t energy = 0;
  long now = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();

  long target = now - seconds_ago;

  sqlite3_reset(stmt);
  sqlite3_bind_int(stmt, 1, unit_id);
  sqlite3_bind_text(stmt, 2, gateway_ip.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int64(stmt, 3, target - 30);
  sqlite3_bind_int64(stmt, 4, target + 30);
  sqlite3_bind_int64(stmt, 5, target);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    energy = (uint64_t)sqlite3_column_int64(stmt, 0);
  }

  return energy;
}

/* ---------- DB Setup ---------- */

inline void SetupDatabase(sqlite3 *db) {
  const char *sql =
      "CREATE TABLE IF NOT EXISTS readings ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "timestamp INTEGER,"
      "gateway_ip TEXT,"
      "unit_id INTEGER,"
      "power_a REAL,"
      "voltage_an REAL,"
      "current_a REAL,"
      "total_active_power REAL,"
      "total_apparent_power REAL,"
      "total_power_factor REAL,"
      "total_energy INTEGER,"
      "temp REAL,"
      "total_energy_last_1M INTEGER DEFAULT 0,"
      "total_energy_last_5M INTEGER DEFAULT 0,"
      "total_energy_last_30M INTEGER DEFAULT 0,"
      "total_energy_last_1H INTEGER DEFAULT 0,"
      "total_energy_last_2H INTEGER DEFAULT 0"
      ");";

  char *err = nullptr;
  if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
    std::cerr << "SQLite create table error: " << err << std::endl;
    sqlite3_free(err);
  }

  // 2. Cleanup old data (> 2 days)
  const char *sqlCleanup ="DELETE FROM readings WHERE timestamp < "
                           "strftime('%s', 'now', '-2 days');";
  char *errMsg = 0;
  int rc = sqlite3_exec(db, sqlCleanup, 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error (cleanup): " << errMsg << std::endl;
    sqlite3_free(errMsg);
  } 
}

/* ---------- Main Reader ---------- */

inline void Read_iA9MEM15(const std::vector<int> &ids,
                          const std::string &ipAddr,
                          int port) {
  sqlite3 *db = nullptr;

  if (sqlite3_open("iA9MEM15.db", &db) != SQLITE_OK) {
    std::cerr << "SQLite open error: " << sqlite3_errmsg(db) << std::endl;
    db = nullptr;
  }

  if (db)
    SetupDatabase(db);

  /* ---- Prepare statements ---- */

  sqlite3_stmt *stmtHistory = nullptr;
  sqlite3_stmt *stmtInsert = nullptr;

  if (db) {
    const char *sqlHistory =
        "SELECT total_energy FROM readings "
        "WHERE unit_id=? AND gateway_ip=? "
        "AND timestamp BETWEEN ? AND ? "
        "ORDER BY ABS(timestamp-?) LIMIT 1;";

    if (sqlite3_prepare_v2(db, sqlHistory, -1, &stmtHistory, nullptr) !=
        SQLITE_OK) {
      std::cerr << "SQLite prepare history error: "
                << sqlite3_errmsg(db) << std::endl;
      stmtHistory = nullptr;
    }

    const char *sqlInsert =
        "INSERT INTO readings ("
        "timestamp, gateway_ip, unit_id, power_a, voltage_an, current_a,"
        "total_active_power, total_apparent_power, total_power_factor,"
        "total_energy, temp,"
        "total_energy_last_1M, total_energy_last_5M,"
        "total_energy_last_30M, total_energy_last_1H, total_energy_last_2H"
        ") VALUES (strftime('%s','now'),?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmtInsert, nullptr) !=
        SQLITE_OK) {
      std::cerr << "SQLite prepare insert error: "
                << sqlite3_errmsg(db) << std::endl;
      stmtInsert = nullptr; // ❗ ไม่ return
    }
  }

  /* ---- Loop Devices ---- */

  for (int unitId : ids) {
    std::cout << "\nStarting Monitor (Device " << unitId << ")...\n";

    auto client = iA9MEM15::createClient(unitId, ipAddr, port);
    if (!client || !client->isConnected()) {
      std::cerr << "Failed to connect device " << unitId << std::endl;
      continue;
    }

    float powerA = client->Read_ActivePowerOnPhaseA();
    float voltage = client->Read_RmsPhasetoneutralVoltageAn();
    float current = client->Read_RmsCurrentOnPhaseA();
    float totalP = client->Read_TotalActivePower();
    float apparent = client->Read_TotalApparentPowerArithmetic();
    float pf = client->Read_TotalPowerFactor();
    float temp = client->Read_DeviceInternalTemperature();
    uint64_t energy =
        client->Read_TotalActiveEnergyDelivered_NotResettable();

    uint64_t e1m = get_historical_energy(stmtHistory, unitId, ipAddr, 60);
    uint64_t e5m = get_historical_energy(stmtHistory, unitId, ipAddr, 300);
    uint64_t e30m = get_historical_energy(stmtHistory, unitId, ipAddr, 1800);
    uint64_t e1h = get_historical_energy(stmtHistory, unitId, ipAddr, 3600);
    uint64_t e2h = get_historical_energy(stmtHistory, unitId, ipAddr, 7200);

    std::cout << "Active Power A: " << powerA << " W\n";
    std::cout << "Total Power: " << totalP << " W\n";
    std::cout << "Total Energy: " << energy << " Wh\n";
    std::cout << " - Last 1M: " << e1m << " Wh\n";
    std::cout << " - Last 5M: " << e5m << " Wh\n";
    std::cout << " - Last 30M: " << e30m << " Wh\n";
    std::cout << " - Last 1H: " << e1h << " Wh\n";
    std::cout << " - Last 2H: " << e2h << " Wh\n";

    /* ---- Insert DB (ถ้า DB ใช้ได้) ---- */
    if (stmtInsert) {
      sqlite3_reset(stmtInsert);
      sqlite3_bind_text(stmtInsert, 1, ipAddr.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_int(stmtInsert, 2, unitId);
      sqlite3_bind_double(stmtInsert, 3, safe_float(powerA));
      sqlite3_bind_double(stmtInsert, 4, safe_float(voltage));
      sqlite3_bind_double(stmtInsert, 5, safe_float(current));
      sqlite3_bind_double(stmtInsert, 6, safe_float(totalP));
      sqlite3_bind_double(stmtInsert, 7, safe_float(apparent));
      sqlite3_bind_double(stmtInsert, 8, safe_float(pf));
      sqlite3_bind_int64(stmtInsert, 9, energy);
      sqlite3_bind_double(stmtInsert, 10, safe_float(temp));
      sqlite3_bind_int64(stmtInsert, 11, e1m);
      sqlite3_bind_int64(stmtInsert, 12, e5m);
      sqlite3_bind_int64(stmtInsert, 13, e30m);
      sqlite3_bind_int64(stmtInsert, 14, e1h);
      sqlite3_bind_int64(stmtInsert, 15, e2h);

      if (sqlite3_step(stmtInsert) != SQLITE_DONE) {
        std::cerr << "SQLite insert error: "
                  << sqlite3_errmsg(db) << std::endl;
      } else {
        std::cout << "Data saved to SQLite.\n";
      }
    }

    client->Disconnect();
  }
    /* >>> CLEANUP 7 DAYS <<< */
  if (db) {
    const char* sqlCleanup =
        "DELETE FROM readings "
        "WHERE timestamp < strftime('%s','now','-7 days');";

    char* err = nullptr;
    if (sqlite3_exec(db, sqlCleanup, nullptr, nullptr, &err) != SQLITE_OK) {
      std::cerr << "SQLite cleanup error: " << err << std::endl;
      sqlite3_free(err);
    }
  }
  /* <<< CLEANUP 7 DAYS <<< */

  if (stmtHistory)
    sqlite3_finalize(stmtHistory);
  if (stmtInsert)
    sqlite3_finalize(stmtInsert);
  if (db)
    sqlite3_close(db);

}

#endif // READ_IA9MEM15_H
