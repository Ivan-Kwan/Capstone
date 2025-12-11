#ifndef MAXIM_ALGO_H
#define MAXIM_ALGO_H

#include <stdint.h>
#include <stdbool.h>

// Algorithm buffer size (must be equal to or larger than BATCH_SIZE)
#define ALGO_BUFFER_SIZE 400

/**
 * @brief Maxim Official Algorithm Interface
 * * @param pun_ir_buffer  [Input] IR data array
 * @param n_ir_buffer_length [Input] Data length (usually 100~400)
 * @param pun_red_buffer [Input] Red data array
 * @param pn_spo2        [Output] Calculated SpO2 value
 * @param pch_spo2_valid [Output] SpO2 validity (1=valid, 0=invalid)
 * @param pn_heart_rate  [Output] Calculated Heart Rate value
 * @param pch_hr_valid   [Output] Heart Rate validity (1=valid, 0=invalid)
 */
void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length,
                                            uint32_t *pun_red_buffer, int32_t *pn_spo2,
                                            int8_t *pch_spo2_valid, int32_t *pn_heart_rate,
                                            int8_t *pch_hr_valid);

#endif