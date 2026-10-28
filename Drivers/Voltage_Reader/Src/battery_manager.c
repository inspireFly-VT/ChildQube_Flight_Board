// battery_manager.c  (skeleton)

#include "../../Voltage_Reader/Inc/battery_manager.h"

#include <stdio.h>

// ── BQ25756E I2C address ──────────────────────────────────────────
#define BQ_I2C_ADDR   (0x6A << 1)

// ── Register addresses ────────────────────────────────────────────
#define REG_CHARGER_STATUS_1   0x21
#define REG_CHARGER_STATUS_2   0x22
#define REG_CHARGER_STATUS_3   0x23
#define REG_FAULT_STATUS       0x24

int read_battery_voltage(ADC_HandleTypeDef *hadc1) {
    int sum = 0;

    for (int i = 0; i < 10; i++) {
        HAL_ADC_Start(hadc1);
        HAL_ADC_PollForConversion(hadc1, HAL_MAX_DELAY);
        sum += HAL_ADC_GetValue(hadc1);
    }

    int adc_val = sum / 10;

    // Convert to millivolts
    int voltage_mv = (adc_val * 3300) / 4095;

    voltage_mv *= 2;  // adjust for voltage divider

    return voltage_mv;
}

ChargeState read_charge_state(I2C_HandleTypeDef *hi2c) {
    uint8_t reg_val = 0;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,           // pointer to your I2C handle
        BQ_I2C_ADDR,    // device address
        REG_CHARGER_STATUS_1,  // 0x21 - register to read
        I2C_MEMADD_SIZE_8BIT,  // register address is 1 byte
        &reg_val,       // buffer to store result
        1,              // read 1 byte
        HAL_MAX_DELAY   // timeout
    );

    if (status != HAL_OK) {
        // I2C transaction failed - handle error
        //return CHG_NOT_CHARGING;
    	uint32_t err = HAL_I2C_GetError(hi2c);  // HAL_I2C_ERROR_AF = NACK, TIMEOUT = no response/clock
    	return CHG_NOT_CHARGING;                // consider a separate CHG_COMM_ERROR state
    }

    // CHARGE_STAT is bits [2:0] of register 0x21
    uint8_t charge_bits = reg_val & 0x07;

    return (ChargeState)charge_bits;
}
