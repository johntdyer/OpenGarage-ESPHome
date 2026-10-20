// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <Arduino.h>
#include <cstdint>

namespace esphome::opengarage {
// Owner-selected warning/sound preferences, loaded before App.setup() by generic
// setup. A muted warning keeps its countdown and Cancel window; a zero warning
// time dispatches at the next service (Security+ still waits up to 1 s for an
// idle bus slot). Recovery threshold beeps and the audible IP report are always on.
struct AudioPolicy {
  static inline bool warning_muted{false};
  static inline bool startup_muted{false};
  static inline uint32_t default_warning_ms{5000};  // YAML warning_time.
  static inline int32_t warning_ms{-1};             // Owner override; -1 = default.
  static inline uint16_t warning_hz{1000};          // Stock 1 kHz.
  static inline int32_t lockout_ms{-1};              // Owner Open/Close cooldown; -1 = YAML lockout_time.
  static inline uint32_t default_lockout_ms{30000};
  static uint32_t effective_warning_ms() { return warning_ms < 0 ? default_warning_ms : uint32_t(warning_ms); }
};

inline void warning_burst() {
  if (!AudioPolicy::warning_muted) tone(13, AudioPolicy::warning_hz, 500);  // Stock 500 ms envelope.
}
}  // namespace esphome::opengarage
