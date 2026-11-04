/**
 * battery_reader.c - Reads battery voltage, current and charge state
 * See battery_reader.h for details.
 */
#include "../Inc/battery_reader.h"

/* ---- Registers (TI BQ25756E datasheet, section 7.5) ---- */
#define REG_CHARGER_STATUS_1    0x21U   /* CHARGE_STAT[2:0] */
#define REG_CHARGER_STATUS_2    0x22U   /* PG_STAT bit 7 */
#define REG_ADC_CONTROL         0x2BU   /* ADC_EN bit 7, ADC_RATE bit 6, ADC_SAMPLE[5:4] */
#define REG_IBAT_ADC            0x2FU   /* 16-bit, 2's complement, 2 mA/LSB (5 mOhm sense) */
#define REG_VBAT_ADC            0x33U   /* 16-bit, 2 mV/LSB */
#define REG_PART_INFO           0x3DU   /* PART_NUM[6:3] */

#define ADC_EN                  (1U << 7)
#define ADC_CONTINUOUS_15BIT    (ADC_EN)    /* ADC_RATE=0 (continuous), ADC_SAMPLE=00 (15-bit) */

/* The BQ25756E datasheet lists PART_NUM as "011 - BQ25756E" but gives a reset
 * value of 0x2, so accept either. */
#define PART_NUM_A              0x2U
#define PART_NUM_B              0x3U

#define I2C_TIMEOUT_MS          100U

/* ---- Low-level helpers ---- */
static HAL_StatusTypeDef rd8(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *val)
{
    return HAL_I2C_Mem_Read(hi2c, CHARGER_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                            val, 1, I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef wr8(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val)
{
    return HAL_I2C_Mem_Write(hi2c, CHARGER_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &val, 1, I2C_TIMEOUT_MS);
}

/* 16-bit registers are little endian: low byte at reg, high byte at reg+1 */
static HAL_StatusTypeDef rd16(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t *val)
{
    uint8_t b[2];
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(hi2c, CHARGER_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                                           b, 2, I2C_TIMEOUT_MS);
    if (s == HAL_OK) {
        *val = (uint16_t)((uint16_t)b[1] << 8 | b[0]);
    }
    return s;
}

/* Make sure the ADC is running. The charger resets its registers if its
 * watchdog expires or it loses power, so re-enable it whenever it's off. */
static HAL_StatusTypeDef ensure_adc_on(I2C_HandleTypeDef *hi2c)
{
    uint8_t ctrl;
    HAL_StatusTypeDef s = rd8(hi2c, REG_ADC_CONTROL, &ctrl);
    if (s != HAL_OK) return s;
    if ((ctrl & ADC_EN) == 0U) {
        s = wr8(hi2c, REG_ADC_CONTROL, ADC_CONTINUOUS_15BIT);
        HAL_Delay(200);   /* let one full conversion cycle finish */
    }
    return s;
}

/* ---- Public API ---- */
HAL_StatusTypeDef BatteryReader_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t v;
    HAL_StatusTypeDef s;

    /* 1. Is the chip there, and is it really a BQ25756E? */
    s = rd8(hi2c, REG_PART_INFO, &v);
    if (s != HAL_OK) return s;
    v = (uint8_t)((v >> 3) & 0x0FU);
    if (v != PART_NUM_A && v != PART_NUM_B) return HAL_ERROR;

    /* 2. Turn the ADC on, continuous conversion, 15-bit resolution. */
    return wr8(hi2c, REG_ADC_CONTROL, ADC_CONTINUOUS_15BIT);
}

HAL_StatusTypeDef BatteryReader_ReadVoltage_mV(I2C_HandleTypeDef *hi2c, uint32_t *mv)
{
    uint16_t raw;
    HAL_StatusTypeDef s = ensure_adc_on(hi2c);
    if (s != HAL_OK) return s;
    s = rd16(hi2c, REG_VBAT_ADC, &raw);
    if (s == HAL_OK) *mv = (uint32_t)(raw & 0x7FFFU) * 2U;
    return s;
}

HAL_StatusTypeDef BatteryReader_ReadCurrent_mA(I2C_HandleTypeDef *hi2c, int32_t *ma)
{
    uint16_t raw;
    HAL_StatusTypeDef s = ensure_adc_on(hi2c);
    if (s != HAL_OK) return s;
    s = rd16(hi2c, REG_IBAT_ADC, &raw);
    if (s == HAL_OK) *ma = (int32_t)(int16_t)raw * 2;
    return s;
}

HAL_StatusTypeDef BatteryReader_ReadState(I2C_HandleTypeDef *hi2c, ChargerState *st)
{
    uint8_t v;
    HAL_StatusTypeDef s = rd8(hi2c, REG_CHARGER_STATUS_1, &v);
    if (s == HAL_OK) *st = (ChargerState)(v & 0x07U);
    return s;
}

HAL_StatusTypeDef BatteryReader_IsChargerPluggedIn(I2C_HandleTypeDef *hi2c, uint8_t *pg)
{
    uint8_t v;
    HAL_StatusTypeDef s = rd8(hi2c, REG_CHARGER_STATUS_2, &v);
    if (s == HAL_OK) *pg = (uint8_t)((v >> 7) & 1U);
    return s;
}

const char *BatteryReader_StateName(ChargerState st)
{
    switch (st) {
    case CHARGER_NOT_CHARGING: return "Not charging";
    case CHARGER_TRICKLE:      return "Trickle charge";
    case CHARGER_PRECHARGE:    return "Pre-charge";
    case CHARGER_FAST_CC:      return "Fast charge (CC)";
    case CHARGER_TAPER_CV:     return "Taper charge (CV)";
    case CHARGER_TOPOFF:       return "Top-off";
    case CHARGER_DONE:         return "Charge done";
    default:                  return "Reserved";
    }
}
