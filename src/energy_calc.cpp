#include "energy_calc.h"
#include <iostream>

EnergyResult calcEnergyFromWh(sqlite3* db, int64_t current_wh)
{
    EnergyResult result{0.0};

    int64_t prev_wh = 0;

    /* ===== อ่านค่า prev_wh ===== */
    const char* sqlGet =
        "SELECT prev_wh FROM energy_state WHERE id = 1;";

    sqlite3_stmt* stmtGet = nullptr;
    sqlite3_prepare_v2(db, sqlGet, -1, &stmtGet, nullptr);

    if (sqlite3_step(stmtGet) == SQLITE_ROW) {
        prev_wh = sqlite3_column_int64(stmtGet, 0);
    }
    sqlite3_finalize(stmtGet);

    /* ===== รอบแรก ยังไม่คิด ===== */
    if (prev_wh == 0 || current_wh <= prev_wh) {
        const char* sqlInit =
            "UPDATE energy_state "
            "SET prev_wh = ?, updated_at = strftime('%s','now') "
            "WHERE id = 1;";
        sqlite3_stmt* stmtInit = nullptr;
        sqlite3_prepare_v2(db, sqlInit, -1, &stmtInit, nullptr);
        sqlite3_bind_int64(stmtInit, 1, current_wh);
        sqlite3_step(stmtInit);
        sqlite3_finalize(stmtInit);
        return result;
    }

    /* ===== คำนวณหน่วยจริง ===== */
    int64_t delta_wh = current_wh - prev_wh;
    result.delta_kWh = delta_wh / 1000.0;

    /* ===== อัปเดต prev ===== */
    const char* sqlUpdate =
        "UPDATE energy_state "
        "SET prev_wh = ?, updated_at = strftime('%s','now') "
        "WHERE id = 1;";
    sqlite3_stmt* stmtUpd = nullptr;
    sqlite3_prepare_v2(db, sqlUpdate, -1, &stmtUpd, nullptr);
    sqlite3_bind_int64(stmtUpd, 1, current_wh);
    sqlite3_step(stmtUpd);
    sqlite3_finalize(stmtUpd);

    return result;
}// end of calcEnergyFromWh
