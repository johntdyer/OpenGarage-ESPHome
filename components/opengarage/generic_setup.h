// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "esphome/core/defines.h"
#ifdef USE_OPENGARAGE_GENERIC_SETUP
#include "audio_policy.h"
#include "device_settings.h"
#include "generic_credentials.h"
#include "saved_wifi_reset.h"
#include "opengarage_component.h"
#include "esphome/components/api/api_server.h"
#include "esphome/components/esphome/ota/ota_esphome.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/web_server/web_server.h"
#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome::opengarage {
class GenericSetup;
// Owner warning/sound preference. On = stock behavior (flag clear). Never
// optimistic: the parent persists first and republishes the stored value.
class SettingSwitch : public switch_::Switch {
 public:
  SettingSwitch(GenericSetup *parent, uint32_t off_flag) : parent_(parent), flag_(off_flag) {}
 protected:
  void write_state(bool state) override;
  GenericSetup *parent_;
  uint32_t flag_;
};

// Owner warning time (s), tone (Hz) or Open/Close cooldown (s). Same
// persist-then-publish contract.
enum class SettingKind : uint8_t { WARNING_TIME, WARNING_TONE, COOLDOWN };
class SettingNumber : public number::Number {
 public:
  SettingNumber(GenericSetup *parent, SettingKind kind) : parent_(parent), kind_(kind) {}
 protected:
  void control(float value) override;
  GenericSetup *parent_;
  SettingKind kind_;
};

class GenericSetup : public Component, public AsyncWebHandler, public ButtonRecovery {
 public:
  GenericSetup(OpenGarageComponent *parent, api::APIServer *api, esphome::ESPHomeOTAComponent *ota,
               web_server::WebServer *web)
      : parent_(parent), api_(api), ota_(ota), web_(web) {}
  void load_credentials();  // Codegen: after mode/threshold allocation, before App.setup().
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::BUS + 1.0f; }
  void set_status_sensor(text_sensor::TextSensor *sensor) { status_ = sensor; }
  void set_setting_switches(SettingSwitch *warning, SettingSwitch *startup) {
    warning_switch_ = warning; startup_switch_ = startup;
  }
  void set_setting_numbers(SettingNumber *time, SettingNumber *tone, SettingNumber *cooldown) {
    time_number_ = time; tone_number_ = tone; cooldown_number_ = cooldown;
  }
  void request_setting(uint32_t off_flag, bool on);
  void request_number(SettingKind kind, float value);
  bool canHandle(AsyncWebServerRequest *request) const override;
  void handleRequest(AsyncWebServerRequest *request) override;
  // ESPAsyncWebServer otherwise discards URL-encoded POST fields before dispatch.
  bool isRequestHandlerTrivial() const override { return false; }
  void button_hold(bool factory) override;
  void button_report_ip() override;
  void button_release(bool factory) override;
 protected:
  static constexpr uint32_t WINDOW_MS = 600000;  // Initial open AP, ten minutes per power-up.
  bool trusted_host_(AsyncWebServerRequest *request) const;
  bool initial_allowed_(AsyncWebServerRequest *request) const;
  bool authorize_(AsyncWebServerRequest *request);
  bool mutation_allowed_(AsyncWebServerRequest *request) const;
  void send_(AsyncWebServerRequest *request, int code, const char *type, const std::string &body);
  void set_auth_();
  void fail_();
  bool open_dashboard_route_(AsyncWebServerRequest *request) const;
  void forward_dashboard_(AsyncWebServerRequest *request);
  bool save_settings_(const DeviceSettings &next);
  void publish_switches_();
  void apply_policy_();
  void handle_settings_post_(AsyncWebServerRequest *request);
  OpenGarageComponent *parent_;
  api::APIServer *api_;
  esphome::ESPHomeOTAComponent *ota_;
  web_server::WebServer *web_;
  SettingSwitch *warning_switch_{nullptr}, *startup_switch_{nullptr};
  SettingNumber *time_number_{nullptr}, *tone_number_{nullptr}, *cooldown_number_{nullptr};
  DeviceSettingsSlot settings_slot_;
  DeviceSettings settings_{};       // Saved record; name edits apply after restart.
  char active_name_[32]{}, active_friendly_[40]{};  // Stable App name storage for this boot.
  bool open_dashboard_{false}, tail_checked_{false};
  text_sensor::TextSensor *status_{nullptr};
  ESPPreferenceObject preference_;
  ESPPreferenceObject setup_tune_preference_; // RTC only; never shifts the flash credential/Wi-Fi slots.
  static constexpr uint32_t SETUP_TUNE_MARKER = 0x4F47544EU;
  bool setup_tune_pending_{false}, ap_announced_{false}, station_announced_{false};
  SavedWiFiReset wifi_reset_;
  GenericCredentials credentials_{};
  std::string auth_password_;  // Stable c_str until set_auth_ refreshes the middleware pointers.
  std::string token_;
  bool configured_{false};
  bool failed_{false};
  bool expired_{false};
  uint32_t started_ms_{0};
  uint32_t new_client_id_{0};
  uint32_t prepared_client_id_{0};
  bool recovery_inhibited_{false}, recovery_committed_{false}, wifi_recovery_{false};
};
}  // namespace esphome::opengarage
#endif
