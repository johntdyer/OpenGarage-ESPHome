// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <Arduino.h>

namespace esphome::opengarage {
// Owner-selected sound preferences, loaded before App.setup() by generic setup.
// Muting a door warning keeps its full countdown and Cancel window; only GPIO13
// stays silent. Recovery threshold beeps and the audible IP report are always on.
struct AudioPolicy {
  static inline bool warning_muted{false};
  static inline bool startup_muted{false};
};

inline void warning_burst() {
  if (!AudioPolicy::warning_muted) tone(13, 1000, 500);  // Stock 1 kHz, 500 ms envelope.
}
}  // namespace esphome::opengarage
