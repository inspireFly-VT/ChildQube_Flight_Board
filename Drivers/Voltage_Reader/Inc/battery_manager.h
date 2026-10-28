// battery_manager.h

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include "main.h"
#include "stm32l4xx_hal.h"   // ← change this, gives you ALL HAL types
#include "stm32l4xx_hal_i2c.h"
#include <stdint.h>

// ── Charge state from CHARGE_STAT bits ──────────────────────────
typedef enum {
    CHG_NOT_CHARGING    = 0,
    CHG_TRICKLE         = 1,
    CHG_PRECHARGE       = 2,
    CHG_FAST_CC         = 3,
    CHG_TAPER_CV        = 4,
    CHG_TOPOFF          = 6,
    CHG_DONE            = 7
} ChargeState;

// ── TS (thermistor) status ────────────────────────────────────────
typedef enum {
    TS_NORMAL = 0,
    TS_WARM   = 1,
    TS_COOL   = 2,
    TS_COLD   = 3,
    TS_HOT    = 4
} TS_Status;

// ── Fault flags from REG 0x24 ────────────────────────────────────
typedef struct {
    uint8_t input_undervoltage;   // VAC_UV_STAT
    uint8_t input_overvoltage;    // VAC_OV_STAT
    uint8_t battery_overcurrent;  // IBAT_OCP_STAT
    uint8_t battery_overvoltage;  // VBAT_OV_STAT
    uint8_t thermal_shutdown;     // TSHUT_STAT
    uint8_t safety_timer_expired; // CHG_TMR_STAT
    uint8_t drv_fault;            // DRV_OKZ_STAT
} BQ_Faults;

// ── Full battery status snapshot ─────────────────────────────────
typedef struct {
    int      vbat_adc_mv;      // Battery voltage from BQ ADC (mV)
    int      vbat_raw_mv;      // Battery voltage from STM32 ADC (mV)
    int      vac_mv;           // Input voltage (mV)
    int      ibat_ma;          // Charge/discharge current (mA, signed)
    int      iac_ma;           // Input current (mA)
    int      ts_percent_x10;   // e.g. 734 means 73.4%
    uint8_t  power_good;       // 1 = input source valid
    uint8_t  watchdog_expired; // 1 = chip in default mode
    ChargeState charge_state;
    TS_Status   ts_status;
    BQ_Faults   faults;
} BatteryStatus;

// ── Function declarations ─────────────────────────────────────────
int read_battery_voltage(ADC_HandleTypeDef *hadc1);

ChargeState read_charge_state(I2C_HandleTypeDef *hi2c);

#endif
