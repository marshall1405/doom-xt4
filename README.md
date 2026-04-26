# doom-xt4

A DOOM-inspired raycaster running on the XTeink X4 (ESP32-C3, ~380 KB RAM, 800×480 e-ink display).

Not a real DOOM port — a Wolfenstein-style raycaster engine using DOOM sprites, built specifically around the constraints of an e-ink display.

---

## Hardware

| Component | Detail |
|---|---|
| MCU | ESP32-C3 |
| RAM | ~380 KB usable |
| Display | SSD1677 e-ink, 800×480, 1-bit mono |
| Input | 7-button layout (D-pad + confirm/back/power) |
| Storage | SD card (FAT32) |

## Controls

| Button | Action |
|---|---|
| Up | Move forward |
| Down | Move backward |
| Left | Turn left |
| Right | Turn right |
| Confirm | Shoot |
| Power | Power off (blank screen + deep sleep) |

## Building

Requires PlatformIO. Build and flash:

```bash
pio run --target upload
```

The device must be in bootloader mode to upload: hold BOOT, plug in USB, release BOOT.

---

## Projects & Licenses
All of these very heavily used and should be checked out. Huge thanks to the developers!

### open-x4-epaper/community-sdk
Hardware abstraction layer for the XTeink X4 — EInkDisplay, InputManager, SDCardManager drivers.
→ https://github.com/open-x4-epaper/community-sdk

### Lode's Raycasting Tutorial
All raycasting code in this project (DDA wall projection, Z-buffer, sprite projection) is derived solely from Lode Vandevenne's tutorial. No third-party raycaster code was used.
→ https://lodev.org/cgtutor/raycasting.html

### Freedoom
The zombie soldier sprite (`sprites/poss[a-d]5.png`) and pistol sprite (`sprites/pisga0.png`) were downloaded from Freedoom, resized to 32×32, and converted to 1-bit C arrays. Freedoom is a free content replacement for DOOM released under an open license that permits redistribution.
→ https://github.com/freedoom/freedoom
→ https://freedoom.github.io
