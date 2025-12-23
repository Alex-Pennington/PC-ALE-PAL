/**
 * @file test_resampler.cpp
 * @brief Unit tests for Resampler class
 * 
 * @author Alex Pennington, AAM402/KY4OLB
 * @date December 2024
 */

#include "pal/resampler.h"
#include <iostream>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Simple test framework
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "... "; \
    tests_run++; \
    try { name(); tests_passed++; std::cout << "PASSED\n"; } \
    catch (const std::exception& e) { std::cout << "FAILED: " << e.what() << "\n"; } \
} while(0)

#define ASSERT(cond) if (!(cond)) throw std::runtime_error("Assertion failed: " #cond)
#define ASSERT_NEAR(a, b, tol) if (std::abs((a) - (b)) > (tol)) \
    throw std::runtime_error("Assertion failed: " #a " != " #b)

// Generate sine wave
std::vector<float> generate_sine(float freq, float sample_rate, size_t count) {
    std::vector<float> samples(count);
    for (size_t i = 0; i < count; i++) {
        samples[i] = std::sin(2.0f * M_PI * freq * i / sample_rate);
    }
    return samples;
}

// Measure frequency content using simple DFT at target frequency
float measure_frequency_power(const float* samples, size_t count, 
                               float target_freq, float sample_rate) {
    float real = 0, imag = 0;
    for (size_t i = 0; i < count; i++) {
        float phase = 2.0f * M_PI * target_freq * i / sample_rate;
        real += samples[i] * std::cos(phase);
        imag += samples[i] * std::sin(phase);
    }
    return std::sqrt(real * real + imag * imag) / count;
}

TEST(test_decimate_preserves_frequency) {
    pal::Resampler resampler(6);  // 48kHz -> 8kHz
    
    // Generate 1kHz sine at 48kHz (well within 4kHz Nyquist for 8kHz)
    auto input = generate_sine(1000.0f, 48000.0f, 4800);  // 100ms
    std::vector<float> output(input.size() / 6 + 16);
    
    size_t out_count = resampler.decimate(input.data(), input.size(), output.data());
    
    // Should have ~800 samples (100ms at 8kHz)
    ASSERT(out_count >= 790 && out_count <= 810);
    
    // Measure 1kHz content in output (at 8kHz sample rate)
    float power = measure_frequency_power(output.data(), out_count, 1000.0f, 8000.0f);
    
    // Should have strong 1kHz content
    ASSERT(power > 0.3f);  // Reasonably strong
}

TEST(test_interpolate_preserves_frequency) {
    pal::Resampler resampler(6);  // 8kHz -> 48kHz
    
    // Generate 1kHz sine at 8kHz
    auto input = generate_sine(1000.0f, 8000.0f, 800);  // 100ms
    std::vector<float> output(input.size() * 6 + 96);
    
    size_t out_count = resampler.interpolate(input.data(), input.size(), output.data());
    
    // Should have ~4800 samples (100ms at 48kHz)
    ASSERT(out_count >= 4790 && out_count <= 4810);
    
    // Measure 1kHz content in output (at 48kHz sample rate)
    float power = measure_frequency_power(output.data(), out_count, 1000.0f, 48000.0f);
    
    // Should have strong 1kHz content
    ASSERT(power > 0.3f);
}

TEST(test_decimate_rejects_alias) {
    pal::Resampler resampler(6);  // 48kHz -> 8kHz
    
    // Generate 5kHz sine at 48kHz (above 4kHz Nyquist for 8kHz - should be filtered)
    auto input = generate_sine(5000.0f, 48000.0f, 4800);
    std::vector<float> output(input.size() / 6 + 16);
    
    size_t out_count = resampler.decimate(input.data(), input.size(), output.data());
    
    // Measure 5kHz content - but at 8kHz sample rate, this would alias
    // The filter should have removed most of it
    float power_at_3k = measure_frequency_power(output.data(), out_count, 3000.0f, 8000.0f);
    
    // Should have weak alias content (good filter = low aliasing)
    ASSERT(power_at_3k < 0.1f);
}

TEST(test_roundtrip) {
    pal::Resampler dec_resampler(6);
    pal::Resampler int_resampler(6);
    
    // Generate 1kHz sine at 48kHz
    auto input = generate_sine(1000.0f, 48000.0f, 4800);
    std::vector<float> decimated(input.size() / 6 + 16);
    std::vector<float> restored(input.size() + 96);
    
    // Decimate 48k -> 8k
    size_t dec_count = dec_resampler.decimate(input.data(), input.size(), decimated.data());
    
    // Interpolate 8k -> 48k
    size_t int_count = int_resampler.interpolate(decimated.data(), dec_count, restored.data());
    
    // Compare original and restored - skip edges due to filter delay
    size_t skip = 100;
    float max_error = 0;
    for (size_t i = skip; i < std::min(input.size(), int_count) - skip; i++) {
        float error = std::abs(input[i] - restored[i]);
        max_error = std::max(max_error, error);
    }
    
    // Should reconstruct reasonably well
    ASSERT(max_error < 0.2f);  // Within 20% (filter ripple + delay mismatch)
}

TEST(test_reset_clears_history) {
    pal::Resampler resampler(6);
    
    // Process some samples
    auto input1 = generate_sine(1000.0f, 48000.0f, 480);
    std::vector<float> output1(100);
    resampler.decimate(input1.data(), input1.size(), output1.data());
    
    // Reset
    resampler.reset();
    
    // Process DC (zeros) - should get zeros out immediately after reset
    std::vector<float> zeros(480, 0.0f);
    std::vector<float> output2(100);
    size_t count = resampler.decimate(zeros.data(), zeros.size(), output2.data());
    
    // After reset + zeros input, output should be near zero
    float max_val = 0;
    for (size_t i = 10; i < count; i++) {  // Skip initial transient
        max_val = std::max(max_val, std::abs(output2[i]));
    }
    ASSERT(max_val < 0.01f);
}

int main() {
    std::cout << "=== Resampler Unit Tests ===\n\n";
    
    RUN_TEST(test_decimate_preserves_frequency);
    RUN_TEST(test_interpolate_preserves_frequency);
    RUN_TEST(test_decimate_rejects_alias);
    RUN_TEST(test_roundtrip);
    RUN_TEST(test_reset_clears_history);
    
    std::cout << "\n=== Results: " << tests_passed << "/" << tests_run << " passed ===\n";
    
    return (tests_passed == tests_run) ? 0 : 1;
}
