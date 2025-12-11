#include "maxim_algo.h"
#include <math.h>

// Simple peak detection and SpO2 calculation logic
void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length,
                                            uint32_t *pun_red_buffer, int32_t *pn_spo2,
                                            int8_t *pch_spo2_valid, int32_t *pn_heart_rate,
                                            int8_t *pch_hr_valid)
{
    uint32_t un_ir_mean = 0, un_red_mean = 0;
    int32_t i;
    
    // 1. Calculate DC (Direct Component) - i.e. the average
    for (i = 0; i < n_ir_buffer_length; i++) {
        un_ir_mean += pun_ir_buffer[i];
        un_red_mean += pun_red_buffer[i];
    }
    un_ir_mean /= n_ir_buffer_length;
    un_red_mean /= n_ir_buffer_length;

    // 2. Remove DC, calculate AC (Alternating Component) and perform simple peak detection
    // For simplicity, zero-crossing or threshold detection is used here for heart rate calculation
    // The actual Maxim algorithm uses a sliding window and correlation analysis
    
    // --- Simple Signal Quality Check ---
    if (un_ir_mean < 10000) { // IR light is too weak, indicating no finger
        *pn_spo2 = -999;
        *pch_spo2_valid = 0;
        *pn_heart_rate = -999;
        *pch_hr_valid = 0;
        return;
    }

    // --- Calculate AC Component Amplitude (Max - Min) ---
    uint32_t ir_max = 0, ir_min = 0xFFFFFFFF;
    uint32_t red_max = 0, red_min = 0xFFFFFFFF;

    for (i = 0; i < n_ir_buffer_length; i++) {
        if (pun_ir_buffer[i] > ir_max) ir_max = pun_ir_buffer[i];
        if (pun_ir_buffer[i] < ir_min) ir_min = pun_ir_buffer[i];
        if (pun_red_buffer[i] > red_max) red_max = pun_red_buffer[i];
        if (pun_red_buffer[i] < red_min) red_min = pun_red_buffer[i];
    }

    float n_ac_ir = ir_max - ir_min;
    float n_ac_red = red_max - red_min;

    // --- Calculate SpO2 (Based on R value) ---
    // Formula: R = (AC_red / DC_red) / (AC_ir / DC_ir)
    float n_ratio_average = (n_ac_red / un_red_mean) / (n_ac_ir / un_ir_mean);
    
    // Empirical formula: SpO2 = -45.060 * R * R + 30.354 * R + 94.845
    float spo2_calc = -45.060 * n_ratio_average * n_ratio_average + 30.354 * n_ratio_average + 94.845;
    
    if (spo2_calc > 100) spo2_calc = 100;
    if (spo2_calc < 70) spo2_calc = 70; // Lower limit depends on the scenario

    *pn_spo2 = (int32_t)spo2_calc;
    *pch_spo2_valid = 1;

    // --- Calculate Heart Rate (Simple peak counting) ---
    int32_t n_peaks = 0;
    int32_t threshold = un_ir_mean; // Simple threshold using the average
    bool above = false;

    for (i = 0; i < n_ir_buffer_length; i++) {
        if (pun_ir_buffer[i] > threshold && !above) {
            above = true;
            n_peaks++;
        } else if (pun_ir_buffer[i] < threshold) {
            above = false;
        }
    }
    
    // Sampling duration (seconds) = n_ir_buffer_length / 100Hz
    // HR = (Peaks / Seconds) * 60
    float duration_sec = (float)n_ir_buffer_length / 100.0f; // Assuming 100Hz
    *pn_heart_rate = (int32_t)((n_peaks / duration_sec) * 60.0f);
    *pch_hr_valid = 1;
}