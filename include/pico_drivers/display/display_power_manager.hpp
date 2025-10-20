#ifndef PICO_DRIVERS_DISPLAY_DISPLAY_POWER_MANAGER_HPP_
#define PICO_DRIVERS_DISPLAY_DISPLAY_POWER_MANAGER_HPP_

#include <cstdint>

class DisplayPowerManager {
  public:
    void reset(uint8_t initial_contrast) {
        power_save_enabled_ = false;
        dimmed_ = false;
        contrast_persist_suspended_ = false;
        current_contrast_ = initial_contrast;
        previous_contrast_ = initial_contrast;
        fade_restore_contrast_ = initial_contrast;
    }

    void set_power_save_enabled(bool enable) { power_save_enabled_ = enable; }
    bool power_save_enabled() const { return power_save_enabled_; }

    void set_dimmed(bool enable) { dimmed_ = enable; }
    bool dimmed() const { return dimmed_; }

    void suspend_contrast_persist(bool suspend) { contrast_persist_suspended_ = suspend; }
    bool contrast_persist_suspended() const { return contrast_persist_suspended_; }

    void on_contrast_written(uint8_t contrast) {
        current_contrast_ = contrast;
        if (!dimmed_ && !contrast_persist_suspended_) {
            previous_contrast_ = contrast;
        }
        if (!contrast_persist_suspended_ && !dimmed_) {
            fade_restore_contrast_ = contrast;
        }
    }

    uint8_t current_contrast() const { return current_contrast_; }
    uint8_t previous_contrast() const { return previous_contrast_; }
    void set_previous_contrast(uint8_t value) { previous_contrast_ = value; }

    uint8_t fade_restore_contrast() const { return fade_restore_contrast_; }
    void set_fade_restore_contrast(uint8_t value) { fade_restore_contrast_ = value; }

  private:
    bool power_save_enabled_ = false;
    bool dimmed_ = false;
    bool contrast_persist_suspended_ = false;
    uint8_t current_contrast_ = 0xFF;
    uint8_t previous_contrast_ = 0xFF;
    uint8_t fade_restore_contrast_ = 0xFF;
};

#endif // PICO_DRIVERS_DISPLAY_DISPLAY_POWER_MANAGER_HPP_
