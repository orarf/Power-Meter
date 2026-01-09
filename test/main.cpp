#include "Read_iA9MEM15.h"
#include "Read_iPM2xxx.h"
#include "env.h"
#include "ThingsBoardClient.h"

#include <sqlite3.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

constexpr int SEND_INTERVAL_SEC = 60;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TB_TOKEN>\n";
        return 1;
    }

    std::string token = argv[1];
    ThingsBoardClient tb(token, "thingsboard.tricommtha.com", 1883);
    tb.connect();

    std::cout << "Connected to ThingsBoard\n";

    sqlite3 *dbA9 = nullptr;
    sqlite3 *dbPM = nullptr;

    sqlite3_open("/workspace/PanelServerPAS600/iA9MEM15.db", &dbA9);
    sqlite3_open("/workspace/PanelServerPAS600/iPM2xxx.db", &dbPM);

    while (true) {
        // ====== Read Modbus ======
        Read_iA9MEM15({100,101,102}, "192.168.100.28", 502);
        Read_iPM2xxx({1}, "192.168.100.28", 502);

        // ====== iA9MEM15 → ThingsBoard ======
        const char *sqlA9 =
            "SELECT id, timestamp, unit_id, voltage_an, current_a, "
            "total_active_power, total_energy "
            "FROM readings WHERE is_read=0 LIMIT 5;";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(dbA9, sqlA9, -1, &stmt, nullptr) == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id        = sqlite3_column_int(stmt, 0);
                int64_t ts    = sqlite3_column_int64(stmt, 1) * 1000; // ms
                int unit_id   = sqlite3_column_int(stmt, 2);

                JsonDocument doc;
                doc.set("unit_id", unit_id);
                doc.set("voltage", (int64_t)sqlite3_column_double(stmt, 3));
                doc.set("current", (int64_t)sqlite3_column_double(stmt, 4));
                doc.set("power",   (int64_t)sqlite3_column_double(stmt, 5));
                doc.set("energy",  (int64_t)sqlite3_column_double(stmt, 6));

                tb.sendTelemetry(ts, doc);

                std::string upd =
                    "UPDATE readings SET is_read=1 WHERE id=" +
                    std::to_string(id);
                sqlite3_exec(dbA9, upd.c_str(), nullptr, nullptr, nullptr);

                std::cout << "Sent iA9MEM15 row id=" << id << "\n";
            }
            sqlite3_finalize(stmt);
        }

        // ====== iPM2xxx → ThingsBoard ======
        const char *sqlPM =
            "SELECT id, timestamp, unit_id, voltage_a, voltage_b, voltage_c, voltage_avg, current_a, current_b, current_c, current_avg, "
            "active_power_total, total_energy " 
            "FROM readings_pm2xxx WHERE is_read=0 LIMIT 5;";

        if (sqlite3_prepare_v2(dbPM, sqlPM, -1, &stmt, nullptr) == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id        = sqlite3_column_int(stmt, 0);
                int64_t ts    = sqlite3_column_int64(stmt, 1) * 1000;
                int unit_id   = sqlite3_column_int(stmt, 2);

                JsonDocument doc;
                doc.set("unit_id", unit_id);
                doc.set("voltage", (int64_t)sqlite3_column_double(stmt, 3));
                doc.set("currentA(A)", (int64_t)sqlite3_column_double(stmt, 4));
                doc.set("currentB(A)", (int64_t)sqlite3_column_double(stmt, 5));
                doc.set("currentC(A)", (int64_t)sqlite3_column_double(stmt, 6));
                doc.set("currentAvg(A)", (int64_t)sqlite3_column_double(stmt, 7));
                doc.set("power",   (int64_t)sqlite3_column_double(stmt, 8));
                doc.set("energy",  (int64_t)sqlite3_column_double(stmt, 9));

                tb.sendTelemetry(ts, doc);

                std::string upd =
                    "UPDATE readings_pm2xxx SET is_read=1 WHERE id=" +
                    std::to_string(id);
                sqlite3_exec(dbPM, upd.c_str(), nullptr, nullptr, nullptr);

                std::cout << "Sent iPM2xxx row id=" << id << "\n";
            }
            sqlite3_finalize(stmt);
        }

        std::cout << "Waiting " << SEND_INTERVAL_SEC << " sec...\n";
        std::this_thread::sleep_for(
            std::chrono::seconds(SEND_INTERVAL_SEC));
    }

    sqlite3_close(dbA9);
    sqlite3_close(dbPM);
    tb.disconnect();
}
