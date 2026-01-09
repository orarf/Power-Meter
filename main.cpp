#include "Read_iA9MEM15.h"
#include "Read_iPM2xxx.h"
#include "ThingsBoardClient.h"
#include "energy_calc.h"

#include <sqlite3.h>
#include <chrono>
#include <iostream>
#include <thread>

constexpr int SEND_INTERVAL_SEC = 60;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TB_TOKEN>\n";
        return 1;
    }

    ThingsBoardClient tb(argv[1], "thingsboard.tricommtha.com");
    tb.connect();

    sqlite3 *dbA9 = nullptr;
    sqlite3 *dbPM = nullptr;
    sqlite3_open("iA9MEM15.db", &dbA9);
    sqlite3_open("iPM2xxx.db", &dbPM);

    while (true) {
        time_t now = time(nullptr);
        tm* lt = localtime(&now);

        static int last_hour = -1;
        bool newHour = (last_hour != -1 && lt->tm_hour != last_hour);
        last_hour = lt->tm_hour;
        Read_iA9MEM15({100,101,102}, "192.168.100.28", 502);
        Read_iPM2xxx({1}, "192.168.100.28", 502);   
        
        /* ===== iA9MEM15 ===== */
        const char *sqlA9 =
            "SELECT id, timestamp, unit_id, voltage_an, current_a,"
            " total_active_power, total_energy "
            "FROM readings WHERE is_read=0 LIMIT 100;";

        sqlite3_stmt *stmt = nullptr;
        sqlite3_prepare_v2(dbA9, sqlA9, -1, &stmt, nullptr);

       while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id       = sqlite3_column_int(stmt, 0);
    int64_t ts   = sqlite3_column_int64(stmt, 1) * 1000;
    int unit_id  = sqlite3_column_int(stmt, 2);

    double voltage = sqlite3_column_double(stmt, 3);
    double current = sqlite3_column_double(stmt, 4);
    double power   = sqlite3_column_double(stmt, 5);
    double energy  = sqlite3_column_double(stmt, 6);

    JsonDocument doc;

    // 🔑 สร้าง key แยกตาม unit_id
    doc.set("voltage_iA9MEM15_" + std::to_string(unit_id), voltage);
    doc.set("current_iA9MEM15_" + std::to_string(unit_id), current);
    doc.set("power_iA9MEM15_"   + std::to_string(unit_id), power);
    doc.set("energy_iA9MEM15_"  + std::to_string(unit_id), energy);

    tb.sendTelemetry(ts, doc);

    sqlite3_exec(
        dbA9,
        ("UPDATE readings SET is_read=1 WHERE id=" +
         std::to_string(id)).c_str(),
        nullptr, nullptr, nullptr);

    std::cout << "Sent iA9MEM15 unit=" << unit_id
              << " id=" << id << "\n";
}

        sqlite3_finalize(stmt);

        /* ===== iPM2xxx ===== */
        const char *sqlPM =
            "SELECT id, timestamp, unit_id, voltage_a, voltage_b, voltage_c, voltage_avg, current_a, current_b, current_c, current_avg, "
            " active_power_total, frequency, total_energy, ActiveEnergyDeliveredIntoLoad, current_unbalanceA, current_unbalanceB, current_unbalanceC, current_unbalanceWorst, "
            " ActiveEnergyReceived_OutofLoad, ActiveEnergyDeliveredPlussReceived, ActiveEnergyDeliveredDelReceived, ReactiveEnergyDelivered, ReactiveEnergyReceived, "
            " ReactiveEnergyDeliveredPlussReceived, ReactiveEnergyDeliveredDelReceived, ApparentEnergyDelivered, ApparentEnergyReceived, ApparentEnergyDeliveredPlussReceived, ApparentEnergyDeliveredDelReceived, "
            " ActivePowerA, ActivePowerB, ActivePowerC, ReactivePowerA, ReactivePowerB, ReactivePowerC, ApparentPowerA, ApparentPowerB, ApparentPowerC, "
            " PowerFactorA, PowerFactorB, PowerFactorC, PowerDemandMethod, PowerDemandIntervalDuration, PowerDemandSubintervalDuration, PowerDemandElapsedTimeinInterval, PowerDemandElapsedTimeinSubinterval, "
            " CurrentDemandMethod, CurrentDemandIntervalDuration, CurrentDemandElapsedTimein, CurrentDemandSubintervalDuration, CurrentDemandElapsedTimeinInterval, "
            " VoltageAB, VoltageBC, VoltageCA, VoltageLLAvg, "
            " VoltageUnbalanceAB, VoltageUnbalanceBC, VoltageUnbalanceCA, VoltageUnbalanceLLWorst, "
            " VoltageUnbalanceAN, VoltageUnbalanceBN, VoltageUnbalanceCN, VoltageUnbalanceLNWorst, "
            " DisplacementPowerFactorA, DisplacementPowerFactorB, DisplacementPowerFactorC, DisplacementPowerFactorTotal, "
            "ActiveEnergyDeliveredIntoLoad64, ActiveEnergyReceivedOutofLoad64, ActiveEnergyDeliveredPlussReceived64, ActiveEnergyDeliveredDelReceived64 "
            "FROM readings_pm2xxx WHERE is_read=0 LIMIT 5;";

        sqlite3_prepare_v2(dbPM, sqlPM, -1, &stmt, nullptr); 

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            int64_t ts = sqlite3_column_int64(stmt, 1) * 1000;
             // 🔑 Wh สะสมจากมิเตอร์
            int64_t currentWh =
                (int64_t)sqlite3_column_double(stmt, 68);

             // 🔥 คำนวณ delta
             EnergyResult energy = calcEnergyFromWh(dbPM, currentWh);

            if (energy.delta_kWh > 0) {
                 JsonDocument energyDoc;
                energyDoc.set("energy/second(kWh)", energy.delta_kWh);

                int64_t now_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

                tb.sendTelemetry(now_ms, energyDoc);

                  const char* sqlInsert =
                    "INSERT INTO energy_delta (timestamp, delta_kwh) "
                    "VALUES (?, ?);";

                sqlite3_stmt* stmtIns = nullptr;
                sqlite3_prepare_v2(dbPM, sqlInsert, -1, &stmtIns, nullptr);

                sqlite3_bind_int64(stmtIns, 1, now_ms / 1000);   // เก็บเป็นวินาที
                sqlite3_bind_double(stmtIns, 2, energy.delta_kWh);

                sqlite3_step(stmtIns);
                sqlite3_finalize(stmtIns);
            }
                if (newHour) {
                    const char* sql =
                        "SELECT SUM(delta_kwh) "
                        "FROM energy_delta "
                        "WHERE strftime('%Y-%m-%d %H', timestamp, 'unixepoch', 'localtime') = "
                        "strftime('%Y-%m-%d %H', 'now', '-1 hour', 'localtime');";
                
                    sqlite3_stmt* stmt = nullptr;
                    sqlite3_prepare_v2(dbPM, sql, -1, &stmt, nullptr);
                
                    double hourly_kwh = 0.0;
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        hourly_kwh = sqlite3_column_double(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                
                    // 👉 ส่งขึ้น ThingsBoard
                    JsonDocument energyHourdoc;
                    energyHourdoc.set("energy/hour(kWh)", hourly_kwh);
                
                    int64_t ts =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
                        
                    tb.sendTelemetry(ts, energyHourdoc);
                        
                    std::cout << "🕐 Hourly energy = "
                              << hourly_kwh << " kWh\n";
                }
                if (newHour) {
                const char* sql =
                    "SELECT SUM(delta_kwh) "
                    "FROM energy_delta "
                    "WHERE strftime('%Y-%m-%d %H', timestamp, 'unixepoch', 'localtime') = "
                    "strftime('%Y-%m-%d %H', 'now', '-1 hour', 'localtime');";
                            
                sqlite3_stmt* stmt = nullptr;
                sqlite3_prepare_v2(dbPM, sql, -1, &stmt, nullptr);
                            
                double hourly_kwh = 0.0;
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    hourly_kwh = sqlite3_column_double(stmt, 0);
                }
                sqlite3_finalize(stmt);
            
                // ✅ INSERT ค่า "รายชั่วโมง" ลง DB (isread = 1)
                const char* sqlInsert =
                    "INSERT INTO energy_delta (timestamp, delta_kWh_hour, isread) "
                    "VALUES (strftime('%s','now'), ?, 1);";
            
                sqlite3_stmt* stmtIns = nullptr;
                sqlite3_prepare_v2(dbPM, sqlInsert, -1, &stmtIns, nullptr);
                sqlite3_bind_double(stmtIns, 1, hourly_kwh);
                sqlite3_step(stmtIns);
                sqlite3_finalize(stmtIns);
            
                std::cout << "🕐 Hourly energy saved = "
                          << hourly_kwh << " kWh\n";
            }

            

          JsonDocument doc;
            doc.set("unit_id_iPM2xxx", sqlite3_column_int(stmt, 2));
                    
            doc.set("voltageA(V)_iPM2xxx", sqlite3_column_double(stmt, 3));
            doc.set("voltageB(V)_iPM2xxx", sqlite3_column_double(stmt, 4));
            doc.set("voltageC(V)_iPM2xxx", sqlite3_column_double(stmt, 5));
            doc.set("voltageAvg(V)_iPM2xxx", sqlite3_column_double(stmt, 6));
                    
            doc.set("currentA(A)_iPM2xxx", sqlite3_column_double(stmt, 7));
            doc.set("currentB(A)_iPM2xxx", sqlite3_column_double(stmt, 8));
            doc.set("currentC(A)_iPM2xxx", sqlite3_column_double(stmt, 9));
            doc.set("currentAvg(A)_iPM2xxx", sqlite3_column_double(stmt,10));
                    
            doc.set("activePowerTotal(W)_iPM2xxx", sqlite3_column_double(stmt,11));
            doc.set("frequency(Hz)_iPM2xxx", sqlite3_column_double(stmt,12));
            doc.set("totalEnergy(kWh)_iPM2xxx", sqlite3_column_double(stmt,13));
            doc.set("ActiveEnergyDeliveredIntoLoad(kWh)_iPM2xxx", sqlite3_column_double(stmt,14));
                    
            doc.set("currentUnbalanceA(%)_iPM2xxx", sqlite3_column_double(stmt,15));
            doc.set("currentUnbalanceB(%)_iPM2xxx", sqlite3_column_double(stmt,16));
            doc.set("currentUnbalanceC(%)_iPM2xxx", sqlite3_column_double(stmt,17));
            doc.set("currentUnbalanceWorst(%)_iPM2xxx", sqlite3_column_double(stmt,18));

            doc.set("ActiveEnergyReceived_OutofLoad(kWh)_iPM2xxx", sqlite3_column_double(stmt,19));
            doc.set("ActiveEnergyDeliveredPlussReceived(kWh)_iPM2xxx", sqlite3_column_double(stmt,20));
            doc.set("ActiveEnergyDeliveredDelReceived(kWh)_iPM2xxx", sqlite3_column_double(stmt,21));
                    
            doc.set("ReactiveEnergyDelivered(kVARh)_iPM2xxx", sqlite3_column_double(stmt,22));
            doc.set("ReactiveEnergyReceived(kVARh)_iPM2xxx", sqlite3_column_double(stmt,23));
            doc.set("ReactiveEnergyDeliveredPlussReceived(kVARh)_iPM2xxx", sqlite3_column_double(stmt,24));
            doc.set("ReactiveEnergyDeliveredDelReceived(kVARh)_iPM2xxx", sqlite3_column_double(stmt,25));
                    
            doc.set("ApparentEnergyDelivered(kVAh)_iPM2xxx", sqlite3_column_double(stmt,26));
            doc.set("ApparentEnergyReceived(kVAh)_iPM2xxx", sqlite3_column_double(stmt,27));
            doc.set("ApparentEnergyDeliveredPlussReceived(kVAh)_iPM2xxx", sqlite3_column_double(stmt,28));
            doc.set("ApparentEnergyDeliveredDelReceived(kVAh)_iPM2xxx", sqlite3_column_double(stmt,29));
                    
            doc.set("ActivePowerA(kW)_iPM2xxx", sqlite3_column_double(stmt,30));
            doc.set("ActivePowerB(kW)_iPM2xxx", sqlite3_column_double(stmt,31));
            doc.set("ActivePowerC(kW)_iPM2xxx", sqlite3_column_double(stmt,32));
                    
            doc.set("ReactivePowerA(kVAR)_iPM2xxx", sqlite3_column_double(stmt,33));
            doc.set("ReactivePowerB(kVAR)_iPM2xxx", sqlite3_column_double(stmt,34));
            doc.set("ReactivePowerC(kVAR)_iPM2xxx", sqlite3_column_double(stmt,35));

            doc.set("ApparentPowerA(kVA)_iPM2xxx", sqlite3_column_double(stmt,36));
            doc.set("ApparentPowerB(kVA)_iPM2xxx", sqlite3_column_double(stmt,37));
            doc.set("ApparentPowerC(kVA)_iPM2xxx", sqlite3_column_double(stmt,38));
                    
            doc.set("PowerFactorA(%)_iPM2xxx", sqlite3_column_double(stmt,39));
            doc.set("PowerFactorB(%)_iPM2xxx", sqlite3_column_double(stmt,40));
            doc.set("PowerFactorC(%)_iPM2xxx", sqlite3_column_double(stmt,41));
                    
            doc.set("PowerDemandMethod_iPM2xxx", sqlite3_column_int(stmt,42));
            doc.set("PowerDemandIntervalDuration_iPM2xxx", sqlite3_column_int(stmt,43));
            doc.set("PowerDemandSubintervalDuration_iPM2xxx", sqlite3_column_int(stmt,44));
            doc.set("PowerDemandElapsedTimeInInterval_iPM2xxx", sqlite3_column_int(stmt,45));
            doc.set("PowerDemandElapsedTimeInSubinterval_iPM2xxx", sqlite3_column_int(stmt,46));

            doc.set("CurrentDemandMethod_iPM2xxx", sqlite3_column_int(stmt,47));
            doc.set("CurrentDemandIntervalDuration_iPM2xxx", sqlite3_column_int(stmt,48));
            doc.set("CurrentDemandElapsedTimein_iPM2xxx", sqlite3_column_int(stmt,49));
            doc.set("CurrentDemandSubintervalDuration_iPM2xxx", sqlite3_column_int(stmt,50));
            doc.set("CurrentDemandElapsedTimeinInterval_iPM2xxx", sqlite3_column_int(stmt,51));
            doc.set("VoltageAB(V)_iPM2xxx", sqlite3_column_double(stmt,52));
            doc.set("VoltageBC(V)_iPM2xxx", sqlite3_column_double(stmt,53));
            doc.set("VoltageCA(V)_iPM2xxx", sqlite3_column_double(stmt,54));
            doc.set("VoltageLLAvg(V)_iPM2xxx", sqlite3_column_double(stmt,55));

            doc.set("VoltageUnbalanceAB(%)_iPM2xxx", sqlite3_column_double(stmt,56));;
            doc.set("VoltageUnbalanceBC(%)_iPM2xxx", sqlite3_column_double(stmt,57));;
            doc.set("VoltageUnbalanceCA(%)_iPM2xxx", sqlite3_column_double(stmt,58));;
            doc.set("VoltageUnbalanceLLWorst(%)_iPM2xxx", sqlite3_column_double(stmt,59));;

            doc.set("VoltageUnbalanceAN(%)_iPM2xxx", sqlite3_column_double(stmt,60));;
            doc.set("VoltageUnbalanceBN(%)_iPM2xxx", sqlite3_column_double(stmt,61));;
            doc.set("VoltageUnbalanceCN(%)_iPM2xxx", sqlite3_column_double(stmt,62));;
            doc.set("VoltageUnbalanceLNWorst(%)_iPM2xxx", sqlite3_column_double(stmt,63));;

            doc.set("DisplacementPowerFactorA_iPM2xxx", sqlite3_column_double(stmt,64));;
            doc.set("DisplacementPowerFactorB_iPM2xxx", sqlite3_column_double(stmt,65));;
            doc.set("DisplacementPowerFactorC_iPM2xxx", sqlite3_column_double(stmt,66));;
            doc.set("DisplacementPowerFactorTotal_iPM2xxx", sqlite3_column_double(stmt,67));;

            
            doc.set("ActiveEnergyDeliveredIntoLoad64(Wh)_iPM2xxx", sqlite3_column_double(stmt,68));;
            doc.set("ActiveEnergyReceivedOutofLoad64(Wh)_iPM2xxx", sqlite3_column_double(stmt,69));;
            doc.set("ActiveEnergyDeliveredPlussReceived64(Wh)_iPM2xxx", sqlite3_column_double(stmt,70));;
            doc.set("ActiveEnergyDeliveredDelReceived64(Wh)_iPM2xxx", sqlite3_column_double(stmt,71));;

            tb.sendTelemetry(ts, doc);

            sqlite3_exec(
                dbPM,
                ("UPDATE readings_pm2xxx SET is_read=1 WHERE id=" +
                 std::to_string(id)).c_str(),
                nullptr, nullptr, nullptr);

            std::cout << "Sent iPM2xxx id=" << id << "\n";
            std::cout << "⚡ Delta Energy = "
                    << energy.delta_kWh << " kWh\n";
        }
        sqlite3_finalize(stmt);
        
        std::this_thread::sleep_for(
            std::chrono::seconds(SEND_INTERVAL_SEC));
    }
}
