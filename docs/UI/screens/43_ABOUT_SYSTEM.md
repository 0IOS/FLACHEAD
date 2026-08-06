# 43_ABOUT_SYSTEM.md

# FLACHEAD UI — About System Screen Specification

## 1. Purpose

The About System Screen provides technical information about the FLACHEAD device, software, and connected hardware.

It acts as the identity and diagnostics page of the device.

It displays:

* FLACHEAD version
* hardware information
* audio hardware information
* storage information
* system status

Primary principle:

> The user should understand what their device is running without exposing unnecessary complexity.

---

# 2. Design goals

The About System Screen must be:

* informative
* lightweight
* readable
* useful for troubleshooting
* safe for non-technical users

---

# 3. Dependencies

Required systems:

* Configuration Service
* Hardware Detection Service
* DSP Manager
* Storage Manager
* System Information Service
* Update Manager (future)
* Theme Engine
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="x5m8q2"
Hardware

↓

System Information Service

↓

About Screen

↓

User
```

---

The screen only displays information.

It does not:

* modify hardware
* change configuration
* perform updates directly

---

# 5. Screen layout

Portrait layout:

```text id="q8m3x5"
+----------------------+
| < Back               |
|                      |
| FLACHEAD             |
|                      |
| Version              |
| v1.0.0               |
|                      |
| Hardware             |
| Raspberry Pi Zero W  |
|                      |
| Audio                |
| TANCHJIM BUNNY DSP   |
|                      |
| Storage              |
| 64GB SD Card         |
|                      |
| Diagnostics          |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Device identity section

Displays:

* device name
* firmware version
* build information

Example:

```text id="m7x3q9"
FLACHEAD

Firmware:
1.0.0

Build:
2026.08
```

---

# 7. Hardware information

Displays:

## Main board

Example:

```text id="v5m8x3"
Board:

Raspberry Pi Zero W
```

---

## Display

Example:

```text id="p6m3x8"
Display:

2.8" TFT

Resolution:

320x240
```

---

## Storage

Example:

```text id="c8m4x5"
Storage:

64GB SD Card
```

---

# 8. Audio hardware information

Displays:

```text id="w7m2x4"
Output:

TANCHJIM BUNNY DSP


Connection:

USB Audio
```

---

Additional:

* supported formats
* active output mode
* driver information

---

# 9. Software information

Displays:

* FLACHEAD version
* UI version
* audio engine version
* database version

Example:

```text id="n4m8x6"
UI:

1.0.0


Audio Engine:

1.0.0
```

---

# 10. Diagnostics section

Provides quick status overview.

Example:

```text id="r5m8x2"
System Status

✓ Audio

✓ Storage

✓ Database

✓ Display
```

---

# 11. Advanced diagnostics

Future option:

```text id="k6m3x9"
Detailed Diagnostics
```

May include:

* CPU usage
* RAM usage
* temperature
* logs

---

# 12. Physical control support

UP/DOWN:

Navigate sections.

SELECT:

Open details.

BACK:

Return.

HOME:

Launcher.

---

# 13. Theme behavior

About System uses:

```text id="b8m4x3"
System Theme
```

---

No album-based colors.

Reason:

This is a system utility screen.

---

# 14. Animations

Allowed:

* simple transitions
* status indicator changes

Duration:

100-200ms

---

Avoid:

* unnecessary effects
* animated logos consuming resources

---

# 15. Performance requirements

Targets:

Opening:

<200ms

Information loading:

asynchronous

Memory:

minimal

---

The About screen must remain lightweight because it may be used during troubleshooting.

---

# 16. Error handling

Unknown hardware:

Display:

```text id="z7m4x2"
Unknown Device
```

---

Unavailable information:

Display:

```text id="h5m8x6"
Not Available
```

---

Hardware detection failure:

Do not crash.

---

# 17. Acceptance criteria

Implementation is complete when:

* version information displays
* hardware information displays
* DSP information displays
* diagnostics work
* screen remains responsive

---

# 18. Architectural recommendations

Required service:

```text id="d6m3x8"
System Information Service

├── Device Info

├── Software Info

├── Hardware Info

├── Audio Info

└── Diagnostics
```

---

Required components:

```text id="e8m4x6"
InfoRow

SectionHeader

StatusIndicator

ScrollableList
```

---

# 19. Future improvements

Possible additions:

* firmware update screen
* backup/restore settings
* export diagnostics
* developer mode
* hardware tests

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep this screen read-only.
* Gather information through services.
* Avoid expensive hardware queries every frame.
* Cache static information.
* Keep diagnostics optional.

The About System screen should make FLACHEAD feel like a complete hardware product, similar to a premium dedicated audio player.
