# OpenGarage for ESPHome

> **This is a personal fork of [OpenGarage/OpenGarage-ESPHome](https://github.com/OpenGarage/OpenGarage-ESPHome).**
> It adds owner settings that the stock OpenGarage firmware let you control but the upstream ESPHome firmware does not (see [upstream issue #2](https://github.com/OpenGarage/OpenGarage-ESPHome/issues/2)). Upstream does not plan to merge these changes, so they are maintained here. This fork's `main` is upstream plus the changes below; upstream updates are merged in periodically.

## Fork Changes

All changes are opt-in: the defaults match upstream, and nothing changes until you edit a setting. Settings are saved on the device and survive power loss, reboots, firmware updates and Wi-Fi reset. Factory reset clears them. Configuration entities apply immediately; a door request already counting down keeps its timing.

**Home Assistant / web dashboard (Configuration)**

| Setting | Range / default | Purpose |
| --- | --- | --- |
| Door Warning Sound | on / off | Silence the buzzer before door moves; the delay still runs |
| Door Warning Time | 0–30 s, default 5 | Length of the pre-move countdown; 0 sends the command immediately |
| Door Warning Tone | 200–5000 Hz in 50 Hz steps, default 1000 | Buzzer pitch |
| Door Command Cooldown | 5–120 s, default 30 | How long Open/Close are refused after a door command; Toggle is unaffected |
| Startup Sounds | on / off | Skip the boot/connect tunes |

**Setup page (`/og/setup` → Device Settings)**

- **Hostname and display name:** rename the device. The hostname sets its `.local` address, DHCP name and recovery AP name. Saving restarts the device.
- **Admin password:** replace the generated password with your own (8–63 characters). It applies to the web pages, firmware upload, ESPHome OTA and the recovery AP. The HA encryption key does not change.
- **Open dashboard:** optionally use the main controls page without a password on your home network. The setup page, firmware upload, ESPHome OTA and the HA API stay protected, and door commands are accepted only from the device's own page.

Details are in [Controls and Operation](docs/operation.md) and [Device Settings](docs/install.md#device-settings).

**Safety:** a shorter, silent or zero warning removes the audible alert before the door moves, and a cooldown shorter than your door's travel time lets Open/Close act on a stale position. Keep a warning whenever someone may be near the door or it is out of sight, and don't use unwarned closing for unattended automations.

**Storage:** the new settings live in a fixed, checksummed slot at the end of the ESP8266 preference sector, so existing Wi-Fi, HA key and calibration records are not moved by an update. If you install upstream 0.10.26 or older afterwards, first set a password of at least 12 characters; older firmware treats shorter ones as unprovisioned.

**Building:** this fork has no prebuilt releases. Build it yourself (see [Building and Contributing](docs/development.md)):

```sh
git clone https://github.com/johntdyer/OpenGarage-ESPHome.git && cd OpenGarage-ESPHome
python3.12 -m venv .venv && .venv/bin/pip install -r requirements.txt
make compile PYTHON=.venv/bin/python
```

Upload `firmware/.esphome/build/opengarage/.pioenvs/opengarage/firmware.bin` through `/og/setup` → Firmware Update. Saved credentials, Wi-Fi and calibration are kept.

**Keeping up with upstream:** `git fetch upstream && git merge upstream/main`, then rebuild. If upstream changes its pinned ESPHome version, recheck the internals this fork relies on (listed in [Building and Contributing](docs/development.md#pinned-framework-contracts)) before flashing.

---

*The rest of this README is from upstream.*

ESPHome-based firmware for OpenGarage (OG) with local Home Assistant (HA) integration. Supports **OpenGarage v2.0–v2.2 and v2.3+ boards with 4 MB flash**. Older 2 MB boards are not supported.

**Experimental firmware for user testing.** Basic operation has been manually tested, but broad opener compatibility and long-term reliability have not been established. Read the [known issues and safety limits](docs/known-issues.md).

<img src="docs/images/home-assistant-dashboard.jpg" alt="Home Assistant dashboard with OpenGarage door, light and remote-lock controls alongside door and vehicle sensors" width="800">

Example using the [optional Home Assistant cards](docs/dashboard.md). A [built-in web interface](docs/install.md#device-web-interface) is also available.

## Start Here

To install: download the release `.bin`, upload it to OG, configure Wi-Fi through its setup AP, and add the discovered ESPHome device to Home Assistant. No compilation, MQTT broker, HACS or ESPHome Device Builder is required to use a prebuilt image. This firmware replaces the stock UI and stock cloud integrations.

- [Installation and HA Pairing](docs/install.md)
- [Controls and Operation](docs/operation.md)
- [Updates and Recovery](docs/recovery.md)
- [Optional HA Cards](docs/dashboard.md)
- [Known Issues](docs/known-issues.md)
- [Building and Contributing](docs/development.md)

## Features and Hardware

The v2.0–v2.2 hardware uses dry-contact control. On v2.3+, select None / Security+ 1.0 / Security+ 2.0 in software and restart. Hardware detection does not automatically detect the opener protocol or establish wiring compatibility.

Features include door controls, distance-based vehicle sensing, editable distance thresholds, stock-style buzzer warnings, audible IP reporting, Wi-Fi/factory reset gestures, and Security+ light, remote-lock and reported obstruction entities. Keep the opener's original controls and safety devices operational; Toggle is not an emergency Stop.

## Source and License

Project code is GPL-3.0-or-later. See [LICENSE](LICENSE) and [VENDOR.md](VENDOR.md) for dependency provenance and retained third-party notices.
