# 02_TOUCHSCREEN_CALIBRATION.md

# FLACHEAD UI — Touchscreen Calibration Specification

## 1. Purpose

This document defines how FLACHEAD handles touchscreen calibration.

Because FLACHEAD uses a small 2.8" TFT touchscreen, accurate touch mapping is critical for reliable interaction.

The calibration system manages:

* coordinate alignment
* touch accuracy
* calibration storage
* recovery from incorrect calibration

Primary principle:

> Touch input should feel precise and predictable even on low-cost hardware.

---

# 2. Design goals

The calibration system must be:

* simple
* reliable
* persistent
* hardware-independent
* recoverable

---

# 3. Calibration architecture

Data flow:

```text id="x7m4q9"
Touch Hardware

↓

Touch Driver

↓

Calibration Layer

↓

Input Manager

↓

UI Coordinates
```

---

The UI must only receive calibrated coordinates.

---

# 4. Coordinate systems

FLACHEAD uses three coordinate spaces:

```text id="m5x8q3"
1. Hardware Coordinates

2. Calibrated Coordinates

3. UI Coordinates
```

---

Example:

```text id="v8m3x5"
Raw Touch:

(120,850)


↓

Calibration


↓

UI:

(100,200)
```

---

# 5. Calibration process

User flow:

```text id="c6m4x9"
Settings

↓

Display

↓

Touch Calibration

↓

Calibration Points

↓

Save
```

---

# 6. Calibration points

Recommended:

4-point calibration.

Points:

```text id="r7m3x8"
Top Left

Top Right

Bottom Left

Bottom Right
```

---

Avoid excessive points.

Reason:

Small display.

---

# 7. Calibration screen

Layout:

```text id="n8m4x6"
+----------------------+
|                      |
|          +           |
|                      |
|                      |
|                      |
|                 +    |
|                      |
|                      |
|     +                |
|                      |
|                +     |
|                      |
+----------------------+
```

---

Instructions:

```text id="w5m3x7"
Touch each marker
```

---

# 8. Calibration data

Stored in:

```text id="a7m4x8"
Configuration Service
```

---

Example:

```text id="p6m8x2"
touch_x_scale

touch_y_scale

offset_x

offset_y
```

---

# 9. Applying calibration

Every touch event:

```text id="u5m3x9"
Raw Input

↓

Calibration Matrix

↓

UI Coordinate

↓

Gesture System
```

---

# 10. Rotation handling

FLACHEAD is portrait-only.

Supported:

```text id="k8m4x5"
Portrait
```

---

No dynamic rotation.

---

Reason:

* simpler
* faster
* fewer calibration problems

---

# 11. Calibration validation

After calibration:

Show test screen.

Example:

```text id="d7m3x8"
Touch all corners

Confirm accuracy
```

---

If incorrect:

Retry.

---

# 12. Touch accuracy requirements

Target:

```text id="h5m8x2"
±5 pixels
```

---

Important for:

* buttons
* sliders
* list rows

---

# 13. Automatic recovery

If invalid calibration detected:

Fallback:

```text id="s6m4x9"
Default Calibration
```

---

Allow user recalibration.

---

# 14. Physical control fallback

Touchscreen failure:

FLACHEAD remains usable through:

* navigation buttons
* playback buttons
* volume dial

---

# 15. Performance requirements

Calibration processing:

Must be:

* lightweight
* calculated once
* cached

---

Do not transform coordinates repeatedly with expensive calculations.

---

# 16. Error handling

Touch driver unavailable:

Display:

```text id="b8m3x5"
Touchscreen unavailable

Use physical controls
```

---

Calibration corruption:

Reset safely.

---

# 17. Acceptance criteria

Implementation is complete when:

* touch coordinates align correctly
* calibration survives reboot
* incorrect calibration can be repaired
* physical controls remain functional

---

# 18. Architectural recommendations

Required module:

```text id="d8m4x6"
Touch Calibration Manager

├── Read Calibration

├── Apply Transformation

├── Save Calibration

├── Reset Calibration

└── Validate Input
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep calibration separate from gesture detection.
* Store calibration permanently.
* Never hardcode screen coordinates.
* Test with real hardware.
* Provide a physical-control fallback.

A reliable touchscreen is important, but FLACHEAD should always remain a hardware music player first.
