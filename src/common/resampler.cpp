/**
 * @file resampler.cpp
 * @brief Polyphase FIR resampler implementation
 * 
 * @author Alex Pennington, AAM402/KY4OLB
 * @date December 2024
 * @license MIT
 */

#include "pal/resampler.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace pal {

Resampler::Resampler(int ratio, int taps_per_phase)
    : ratio_(ratio)
    , taps_per_phase_(taps_per_phase)
    , total_taps_(ratio * taps_per_phase)
    , coeffs_(total_taps_)
    , history_(total_taps_, 0.0f)
    , history_pos_(0)
{
    design_filter();
}

void Resampler::design_filter() {
    // Design lowpass filter: Fc = 0.8 * (Fs_low / 2) / Fs_high
    // For 48kHz -> 8kHz: Fc = 0.8 * 4000 / 48000 = 0.0667
    // Using slightly lower cutoff for better stopband rejection
    float fc = 0.45f / ratio_;  // Normalized cutoff frequency
    int M = total_taps_ - 1;
    
    // Windowed sinc filter design
    float sum = 0.0f;
    for (int i = 0; i < total_taps_; i++) {
        float n = static_cast<float>(i) - M / 2.0f;
        
        // Sinc function
        float sinc;
        if (std::abs(n) < 1e-6f) {
            sinc = 2.0f * fc;
        } else {
            sinc = std::sin(2.0f * M_PI * fc * n) / (M_PI * n);
        }
        
        // Hamming window
        float window = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / M);
        
        coeffs_[i] = sinc * window;
        sum += coeffs_[i];
    }
    
    // Normalize for unity gain at DC
    for (auto& c : coeffs_) {
        c /= sum;
    }
}

float Resampler::apply_filter() const {
    float sum = 0.0f;
    for (int i = 0; i < total_taps_; i++) {
        size_t idx = (history_pos_ + i) % total_taps_;
        sum += history_[idx] * coeffs_[i];
    }
    return sum;
}

size_t Resampler::decimate(const float* input, size_t input_count, float* output) {
    size_t output_count = 0;
    
    for (size_t i = 0; i < input_count; i++) {
        // Add sample to circular history buffer
        history_[history_pos_] = input[i];
        history_pos_ = (history_pos_ + 1) % total_taps_;
        
        // Output every ratio_ samples
        if (((i + 1) % ratio_) == 0) {
            output[output_count++] = apply_filter();
        }
    }
    
    return output_count;
}

size_t Resampler::interpolate(const float* input, size_t input_count, float* output) {
    size_t output_count = 0;
    
    for (size_t i = 0; i < input_count; i++) {
        // For each input sample, produce ratio_ output samples
        for (int phase = 0; phase < ratio_; phase++) {
            // Insert input sample at phase 0, zeros elsewhere
            float sample = (phase == 0) ? input[i] * ratio_ : 0.0f;
            
            history_[history_pos_] = sample;
            history_pos_ = (history_pos_ + 1) % total_taps_;
            
            output[output_count++] = apply_filter();
        }
    }
    
    return output_count;
}

void Resampler::reset() {
    std::fill(history_.begin(), history_.end(), 0.0f);
    history_pos_ = 0;
}

} // namespace pal
