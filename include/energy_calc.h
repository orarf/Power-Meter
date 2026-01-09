#ifndef ENERGY_CALC_H
#define ENERGY_CALC_H

#include <sqlite3.h>
#include <cstdint>

struct EnergyResult {
    double delta_kWh;
};

EnergyResult calcEnergyFromWh(
    sqlite3* db,
    int64_t current_wh
);

#endif
