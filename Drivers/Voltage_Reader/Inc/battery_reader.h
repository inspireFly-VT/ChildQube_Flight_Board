/**
 * battery_reader.h - Reads battery voltage, current and charge state
 *
 * Board: inspireFly battery board, U8 (BQ25756ERRVR = BQ25756E), on I2C2
 * (PB13 SCL / PB14 SDA). Register addresses from TI BQ25756E datasheet,
 * section 7.5.
 *
 * The battery is read through the charger chip (BQ25756E), the only part on
 * the board that can measure it. This code does NOT change any charging
 * settings. It only:
 *   - checks the part ID
 *   - turns on the charger's built-in ADC
 *   - reads battery voltage, battery current and charge state
 */
#ifndef BATTERY_READER_H
#define BATTERY_READER_H

#include "main.h"
#include <stdint.h>

/* BQ25756E 7-bit address is 0x6A (the non-E BQ25756 is 0x6B).
 * Shifted left for STM32 HAL. */
#define CHARGER_I2C_ADDR        (0x6AU << 1)

/* Charge state, REG0x21 CHARGE_STAT[2:0] */
typedef enum {
    CHARGER_NOT_CHARGING = 0,
    CHARGER_TRICKLE      = 1,
    CHARGER_PRECHARGE    = 2,
    CHARGER_FAST_CC      = 3,
    CHARGER_TAPER_CV     = 4,
    CHARGER_RESERVED     = 5,
    CHARGER_TOPOFF       = 6,
    CHARGER_DONE         = 7
} ChargerState;

/* Call once after MX_I2C2_Init(). Returns HAL_ERROR if the chip isn't found. */
HAL_StatusTypeDef BatteryReader_Init(I2C_HandleTypeDef *hi2c);

/* Battery voltage measured by the charger's ADC (REG0x33), in millivolts.
 * This is the voltage at BAT+ (SRN pin). */
HAL_StatusTypeDef BatteryReader_ReadVoltage_mV(I2C_HandleTypeDef *hi2c, uint32_t *mv);

/* Battery current in mA (REG0x2F). Positive = charging, negative = discharging. */
HAL_StatusTypeDef BatteryReader_ReadCurrent_mA(I2C_HandleTypeDef *hi2c, int32_t *ma);

/* Charge state (REG0x21). */
HAL_StatusTypeDef BatteryReader_ReadState(I2C_HandleTypeDef *hi2c, ChargerState *st);

/* Power-good flag (REG0x22 bit 7): 1 = charger input is valid. */
HAL_StatusTypeDef BatteryReader_IsChargerPluggedIn(I2C_HandleTypeDef *hi2c, uint8_t *pg);

/* Human-readable name for a charge state. */
const char *BatteryReader_StateName(ChargerState st);

#endif /* BATTERY_READER_H */
