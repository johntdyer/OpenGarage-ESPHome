// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "esphome/core/application.h"
#include "esphome/core/preferences.h"
#include "esphome/components/esp8266/preference_backend.h"
#include <cstdint>
#include <cstring>

namespace esphome::opengarage {
// Owner settings added after the first public release. ESP8266 flash preferences
// are allocation ordered, and any new make_preference() before Wi-Fi/API would
// shift their saved slots on OTA. This record instead aliases a FIXED slot at the
// tail of the 128-word sector, far above the ~66 words allocated today. Its own
// checksum seed rejects stale or foreign data; writes are refused if dynamic
// allocation ever reaches the tail. Factory reset erases it; Wi-Fi reset keeps it.
struct DeviceSettings {
  static constexpr uint32_t MAGIC = 0x4F475301U;  // OG owner settings, format 1.
  static constexpr uint32_t TYPE = 0x4F475302U;   // Preference checksum seed.
  static constexpr uint32_t WARNING_MUTED = 1U, STARTUP_MUTED = 2U, OPEN_DASHBOARD = 4U;
  static constexpr uint32_t KNOWN_FLAGS = WARNING_MUTED | STARTUP_MUTED | OPEN_DASHBOARD;
  static constexpr uint32_t MAX_WARNING_S = 30, MIN_TONE_HZ = 200, MAX_TONE_HZ = 5000;
  uint32_t magic{MAGIC};
  // Bits 0-7 flags; 8-15 warning seconds + 1 (0 = YAML default); 16-28 tone Hz (0 = 1 kHz).
  uint32_t flags{0};
  char hostname[32]{};  // Empty: compiled MAC-suffixed name.
  char friendly[40]{};  // Empty: compiled friendly name.

  bool flag(uint32_t bit) const { return (flags & bit) != 0; }
  void set_flag(uint32_t bit, bool on) { flags = on ? (flags | bit) : (flags & ~bit); }
  int32_t warning_s() const { const uint32_t code = (flags >> 8) & 0xFFU; return code ? int32_t(code - 1) : -1; }
  void set_warning_s(int32_t s) { flags = (flags & ~0xFF00U) | (uint32_t(s < 0 ? 0 : s + 1) << 8); }
  uint32_t tone_hz() const { return flags >> 16; }  // 0 = stock.
  void set_tone_hz(uint32_t hz) { flags = (flags & 0xFFFFU) | (hz << 16); }
  // DNS label usable by mDNS, DHCP and the ESP8266 32-byte hostname buffer.
  static bool hostname_valid(const char *name, size_t length) {
    if (length == 0 || length > 31 || name[0] == '-' || name[length - 1] == '-') return false;
    for (size_t i = 0; i < length; ++i) {
      const char c = name[i];
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) return false;
    }
    return true;
  }
  // Printable ASCII without characters needing JSON/HTML escaping.
  static bool friendly_valid(const char *name, size_t length) {
    if (length > 39) return false;
    for (size_t i = 0; i < length; ++i) {
      const unsigned char c = name[i];
      if (c < 32 || c > 126 || c == '"' || c == '\\' || c == '<' || c == '>' || c == '&') return false;
    }
    return true;
  }
  bool valid() const {
    const char *h_end = static_cast<const char *>(std::memchr(hostname, 0, sizeof(hostname)));
    const char *f_end = static_cast<const char *>(std::memchr(friendly, 0, sizeof(friendly)));
    if (magic != MAGIC || (flags & 0xFFU & ~KNOWN_FLAGS) || !h_end || !f_end) return false;
    if (warning_s() > int32_t(MAX_WARNING_S) || (tone_hz() != 0 && (tone_hz() < MIN_TONE_HZ || tone_hz() > MAX_TONE_HZ)))
      return false;
    return (h_end == hostname || hostname_valid(hostname, h_end - hostname)) && friendly_valid(friendly, f_end - friendly);
  }
};
static_assert(sizeof(DeviceSettings) == 80, "Owner settings tail slot layout must remain stable");

class DeviceSettingsSlot {
 public:
  static constexpr uint8_t WORDS = sizeof(DeviceSettings) / 4;
  static constexpr uint16_t FLASH_WORDS = 128;  // restore_from_flash: true sector size.
  static constexpr uint16_t OFFSET = FLASH_WORDS - (WORDS + 1);
  DeviceSettingsSlot() {
    backend_.offset = OFFSET;
    backend_.length_words = WORDS;
    backend_.type = DeviceSettings::TYPE;
    backend_.in_flash = true;
  }
  bool load(DeviceSettings *settings) {
    DeviceSettings loaded{};
    if (!backend_.load(reinterpret_cast<uint8_t *>(&loaded), sizeof(loaded)) || !loaded.valid()) return false;
    *settings = loaded;
    return true;
  }
  // Dynamic allocations grow upward from zero. Never write into one of them.
  bool clear_of_allocations() const {
    return global_preferences != nullptr && global_preferences->current_flash_offset <= OFFSET;
  }
  // RAM cache only; used to roll back a staged record after a failed sync.
  bool stage(const DeviceSettings &settings) {
    return settings.valid() && clear_of_allocations() &&
        backend_.save(reinterpret_cast<const uint8_t *>(&settings), sizeof(settings));
  }
  bool save(const DeviceSettings &settings) { return stage(settings) && global_preferences->sync(); }

 protected:
  esp8266::ESP8266PreferenceBackend backend_;
};

// ESPHome 2026.8.2 has no runtime name setter. Before App.setup(), point the
// protected name fields at stable storage so Wi-Fi/DHCP, mDNS, the AP SSID, the
// API and the web title all see the owner's choice. Recheck on upgrades.
struct AppIdentity : Application {
  static void set(const char *name, const char *friendly) {
    if (name) App.*(&AppIdentity::name_) = StringRef(name, std::strlen(name));
    if (friendly) App.*(&AppIdentity::friendly_name_) = StringRef(friendly, std::strlen(friendly));
  }
};
}  // namespace esphome::opengarage
